#include "backend.h"
#include "packagemanager.h"
#include "terminalutils.h"

Backend::Backend(QObject *parent)
    : QObject(parent)
{
    // Check if we're running as a Flatpak
    m_isFlatpak = QFile::exists("/.flatpak-info");
}

QString Backend::runCommand(const QStringList &command) const
{
    QProcess process;
    QStringList actualCommand;

    if (m_isFlatpak) {
        // Prepend with flatpak-spawn if running as Flatpak
        actualCommand << "flatpak-spawn" << "--host" << command;
    } else {
        actualCommand = command;
    }

    process.start(actualCommand[0], actualCommand.mid(1));
    if (!process.waitForFinished(60000)) {
        return "Error: Command timed out";
    }
    return process.readAllStandardOutput().trimmed();
}

QString Backend::parseDistroFromImage(const QString &imageUrl) const
{
    QString image = imageUrl.toLower();

    // First check hardcoded URL mappings (exact matches)
    static const QMap<QString, QString> hardcodedMappings = {{"ghcr.io/vanilla-os/vso:main", "vso"},
                                                             {"docker.io/blackarchlinux/blackarch:latest", "blackarch"},
                                                             {"cgr.dev/chainguard/wolfi-base", "wolfi"}};

    // Check for exact matches first
    for (auto it = hardcodedMappings.constBegin(); it != hardcodedMappings.constEnd(); ++it) {
        if (image.startsWith(it.key())) {
            return it.value();
        }
    }

    // Ordered list of patterns to try (most specific to most generic)
    QVector<QPair<QString, QString>> patterns = {
        // 1. Explicit toolbox patterns (ubi9/toolbox, ubuntu-toolbox, etc.)
        {"(^|/)([a-z]+)-?toolbox(:|$)", "$2"}, // ubuntu-toolbox -> ubuntu
        {"(^|/)ubi([0-9]+)/toolbox(:|$)", "rhel"}, // ubi9/toolbox -> rhel
        {"(^|/)([a-z]+)/toolbox(:|$)", "$2"}, // fedora/toolbox -> fedora

        // 2. Versioned distro names (ubuntu:22.04, rockylinux:9)
        {"(^|/)([a-z]+)[.:-]?([0-9]{1,2}\\.?[0-9]{0,2})(:|$)", "$2"}, // ubuntu22.04 -> ubuntu

        // 3. Standard distro names in path
        {"(^|/)(alma|alpine|amazon|arch|centos|debian|fedora|rocky|rhel|ubuntu)(:|/|$)", "$2"},

        // 4. Common abbreviations and aliases
        {"(^|/)(rh|redhat)(:|/|$)", "rhel"},
        {"(^|/)ubi([0-9]?)(:|/|$)", "rhel"},
        {"(^|/)nd[0-9]+(:|/|$)", "neurodebian"},

        // 5. Substring fallback (least specific)
        {"([a-z]+)(-|_)?(linux|os)", "$1"} // something-linux -> something
    };

    // Try each pattern in order
    for (const auto &[pattern, replacement] : patterns) {
        QRegularExpression rx(pattern);
        QRegularExpressionMatch match = rx.match(image);
        if (match.hasMatch()) {
            QString matchedDistro = match.captured(replacement == "$2" ? 2 : 1);

            // Validate the matched distro against our known list
            for (const QString &distro : DISTROS) {
                if (matchedDistro.contains(distro)) {
                    return distro;
                }
            }
        }
    }

    // Final fallback: check for any distro name as substring
    for (const QString &distro : DISTROS) {
        if (image.contains(distro)) {
            return distro;
        }
    }

    return "unknown";
}

QString Backend::getContainerDistro(const QString &containerName) const
{
    if (containerName.isEmpty())
        return "";

    QList<QMap<QString, QString>> containers = getContainers();
    for (const auto &container : containers) {
        if (container["name"] == containerName) {
            return PackageManager::getDistroFromImage(container["image"]);
        }
    }
    return "";
}

QList<QMap<QString, QString>> Backend::getContainers() const
{
    QString output = runCommand({"distrobox", "list", "--no-color"});
    QStringList lines = output.split('\n', Qt::SkipEmptyParts);
    if (lines.isEmpty())
        return {};

    QStringList headers;
    for (const QString &col : lines[0].split('|', Qt::SkipEmptyParts)) {
        headers << col.trimmed();
    }

    QList<QMap<QString, QString>> containers;
    for (int i = 1; i < lines.size(); ++i) {
        QStringList parts;
        for (const QString &col : lines[i].split('|', Qt::SkipEmptyParts)) {
            parts << col.trimmed();
        }

        QMap<QString, QString> container;
        container["name"] = parts[headers.indexOf("NAME")];
        container["image"] = parts[headers.indexOf("IMAGE")];
        container["distro"] = parseDistroFromImage(container["image"]);
        container["status"] = parts[headers.indexOf("STATUS")];
        container["id"] = parts[headers.indexOf("ID")];
        container["icon"] = getDistroIcon(container["distro"]);

        containers << container;
    }
    return containers;
}

QString Backend::createContainer(const QString &name, const QString &image, const QString &home, bool init, const QStringList &volumes)
{
    QStringList args = {"distrobox", "create", "-n", name, "-i", image, "-Y"};
    if (init)
        args << "--init" << "--additional-packages" << "systemd";
    if (!home.isEmpty())
        args << "--home" << home;
    for (const QString &v : volumes) {
        args << "--volume" << v;
    }
    return runCommand(args);
}

QString Backend::deleteContainer(const QString &name)
{
    return runCommand({"distrobox", "rm", name, "--force"});
}

void Backend::executeInTerminal(const QString &terminal, const QString &command)
{
    auto terminalInfoMap = getTerminalInfoMap();
    if (!terminalInfoMap.contains(terminal)) {
        qWarning() << "Unknown terminal:" << terminal;
        QProcess::startDetached("xterm", {"-e", command});
        return;
    }

    TerminalInfo info = terminalInfoMap[terminal];
    QStringList args = info.commandFormat;
    QString executable = terminal;

    // Replace placeholders
    for (int i = 0; i < args.size(); ++i) {
        args[i].replace("$terminal", terminal);
        args[i].replace("$command", command);
    }

    // For Flatpak terminals, we use flatpak as the executable
    if (isFlatpakTerminal(terminal)) {
        executable = "flatpak";
    }

    if (m_isFlatpak) {
        // If we're running as Flatpak, we need to use flatpak-spawn to launch the terminal
        QStringList flatpakArgs = {"flatpak-spawn", "--host", executable};
        flatpakArgs.append(args);

        if (!QStandardPaths::findExecutable("flatpak-spawn").isEmpty()) {
            bool success = QProcess::startDetached("flatpak-spawn", flatpakArgs.mid(1));
            if (!success) {
                qWarning() << "Failed to start terminal" << executable << "with args" << args;
                QProcess::startDetached("flatpak-spawn", {"--host", "xterm", "-e", command});
            }
        } else {
            QProcess::startDetached("flatpak-spawn", {"--host", "xterm", "-e", command});
        }
    } else {
        if (!QStandardPaths::findExecutable(executable).isEmpty()) {
            bool success = QProcess::startDetached(executable, args);
            if (!success) {
                qWarning() << "Failed to start terminal" << executable << "with args" << args;
                QProcess::startDetached("xterm", {"-e", command});
            }
        } else {
            QProcess::startDetached("xterm", {"-e", command});
        }
    }
}

void Backend::assembleContainer(const QString &iniFile)
{
    QProcess *process = new QProcess(this);
    connect(process, &QProcess::finished, this, [this, process](int exitCode) {
        QString result = process->readAllStandardOutput();
        if (exitCode != 0) {
            result = "Error: " + process->readAllStandardError();
        }
        emit assembleFinished(result);
        process->deleteLater();
    });

    if (m_isFlatpak) {
        process->start("flatpak-spawn", {"--host", "distrobox", "assemble", "create", "--file", iniFile});
    } else {
        process->start("distrobox", {"assemble", "create", "--file", iniFile});
    }
}

void Backend::enterContainer(const QString &name, const QString &terminal)
{
    executeInTerminal(terminal, QString("distrobox enter %1").arg(name));
}

void Backend::upgradeContainer(const QString &name, const QString &terminal)
{
    executeInTerminal(terminal, QString("distrobox-upgrade %1").arg(name));
}

void Backend::upgradeAllContainers(const QString &terminal)
{
    executeInTerminal(terminal, "distrobox-upgrade --all");
}

void Backend::installDebPackage(const QString &terminal, const QString &containerName, const QString &filePath)
{
    QString command = QString("sudo apt install -y %1").arg(filePath);
    executeInTerminal(terminal, QString("distrobox enter %1 -- %2").arg(containerName).arg(command));
}

void Backend::installRpmPackage(const QString &terminal, const QString &containerName, const QString &filePath)
{
    QString command = QString("sudo dnf install -y %1").arg(filePath);
    executeInTerminal(terminal, QString("distrobox enter %1 -- %2").arg(containerName).arg(command));
}

void Backend::installArchPackage(const QString &terminal, const QString &containerName, const QString &filePath)
{
    QString command = QString("sudo pacman -U --noconfirm %1").arg(filePath);
    executeInTerminal(terminal, QString("distrobox enter %1 -- %2").arg(containerName).arg(command));
}

QStringList Backend::getAvailableApps(const QString &containerName)
{
    QString output = runCommand({"distrobox", "enter", containerName, "--", "find", "/usr/share/applications", "-name", "*.desktop"});
    QStringList apps;
    for (const QString &line : output.split('\n', Qt::SkipEmptyParts)) {
        apps << line.split('/').last().replace(".desktop", "");
    }
    return apps;
}

QStringList Backend::getExportedApps(const QString &containerName)
{
    QStringList apps;
    QDir dir(QDir::homePath() + "/.local/share/applications");
    QString prefix = containerName + "-";

    for (const QFileInfo &file : dir.entryInfoList({prefix + "*.desktop"}, QDir::Files)) {
        apps << file.baseName().mid(prefix.length());
    }
    return apps;
}

QString Backend::exportApp(const QString &appName, const QString &containerName)
{
    return runCommand({"distrobox", "enter", containerName, "--", "distrobox-export", "--app", appName});
}

QString Backend::unexportApp(const QString &appName, const QString &containerName)
{
    return runCommand({"distrobox", "enter", containerName, "--", "distrobox-export", "--app", appName, "--delete"});
}

QList<QMap<QString, QString>> Backend::getAvailableImages()
{
    QList<QMap<QString, QString>> images;
    QString output = runCommand({"distrobox", "create", "-C"});

    for (const QString &line : output.split('\n', Qt::SkipEmptyParts)) {
        QMap<QString, QString> image;
        QString trimmed = line.trimmed();
        image["line"] = trimmed;
        image["url"] = trimmed;
        image["name"] = trimmed.split('/').last();
        image["distro"] = parseDistroFromImage(trimmed);
        image["icon"] = getDistroIcon(image["distro"]);
        images.append(image);
    }

    return images;
}

QString Backend::getDistroIcon(const QString &distroName) const
{
    QString lowerDistro = distroName.toLower();

    // First try exact match
    if (distroIconMap.contains(lowerDistro)) {
        return ":/icons/" + distroIconMap[lowerDistro];
    }

    // Then try partial matches
    for (const QString &key : distroIconMap.keys()) {
        if (lowerDistro.contains(key)) {
            return ":/icons/" + distroIconMap[key];
        }
    }

    // Default icon (tux)
    return ":/icons/tux.svg";
}

QList<QMap<QString, QString>> Backend::searchImages(const QString &query)
{
    QList<QMap<QString, QString>> allImages = getAvailableImages();
    QList<QMap<QString, QString>> filteredImages;

    for (const auto &image : allImages) {
        if (image["name"].contains(query, Qt::CaseInsensitive) || image["distro"].contains(query, Qt::CaseInsensitive)
            || image["url"].contains(query, Qt::CaseInsensitive)) {
            filteredImages.append(image);
        }
    }

    return filteredImages;
}
