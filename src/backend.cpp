// SPDX-License-Identifier: GPL-2.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2025 Hadi Chokr <hadichokr@icloud.com>

#include "backend.h"
#include "packagemanager.h"
#include "terminalutils.h"
#include <cstdint>
#include <mainwindow.h>

Backend::Backend(MainWindow *mainWindow, QObject *parent)
    : QObject(parent)
    , m_mainWindow(mainWindow)
{
    m_isFlatpak = QFile::exists("/.flatpak-info");
    m_preferredBackend = m_mainWindow->preferredBackend;
    validatePreferredBackend();
}

void Backend::setPreferredBackend(const QString &backend)
{
    m_preferredBackend = backend;
    validatePreferredBackend();
}

void Backend::validatePreferredBackend()
{
    QStringList backends = availableBackends();

    if (backends.isEmpty()) {
        qWarning() << "No container backend found!";
        m_preferredBackend.clear();
    } else if (backends.size() == 1) {
        m_preferredBackend = backends.first();
        qDebug() << "Only one backend available, using:" << m_preferredBackend;
    } else {
        if (!backends.contains(m_preferredBackend)) {
            m_preferredBackend = backends.first();
            qDebug() << "Preferred backend not available, fallback to:" << m_preferredBackend;
        } else {
            qDebug() << "Using preferred backend:" << m_preferredBackend;
        }
    }

    // Keep main window in sync
    if (m_mainWindow) {
        m_mainWindow->preferredBackend = m_preferredBackend;
    }
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
        return i18n("Error: Command timed out");
    }
    return process.readAllStandardOutput().trimmed();
}

QString Backend::parseDistroFromImage(const QString &imageUrl) const
{
    QString image = imageUrl.toLower();

    // First check hardcoded URL mappings
    static const QMap<QString, QString> hardcodedMappings = {{"registry.fedoraproject.org/fedora-toolbox", "fedora"},
                                                             {"registry.fedoraproject.org/f", "fedora"},
                                                             {"ghcr.io/vanilla-os/vso:main", "vso"},
                                                             {"docker.io/blackarchlinux/blackarch:latest", "blackarch"},
                                                             {"cgr.dev/chainguard/wolfi-base", "wolfi"}};

    // Check exact matches first
    for (auto it = hardcodedMappings.constBegin(); it != hardcodedMappings.constEnd(); ++it) {
        if (image.startsWith(it.key())) {
            return it.value();
        }
    }

    // Ordered patterns (most specific to most generic)
    QVector<QPair<QString, QString>> patterns = {// Toolbox-specific patterns
                                                 {"(^|/)f([0-9]+)(/|:|$)", "fedora"}, // f38 -> fedora
                                                 {"(^|/)fedora-toolbox(:|$)", "fedora"}, // fedora-toolbox
                                                 {"(^|/)ubi([0-9]+)(/|:|$)", "rhel"}, // ubi9 -> rhel
                                                 {"(^|/)rhel-toolbox(:|$)", "rhel"}, // rhel-toolbox

                                                 // Versioned distro names
                                                 {"(^|/)([a-z]+)[.:-]?([0-9]{1,2}\\.?[0-9]{0,2})(:|$)", "$2"},

                                                 // Standard distro names
                                                 {"(^|/)(alma|alpine|amazon|arch|centos|debian|fedora|rocky|rhel|ubuntu)(:|/|$)", "$2"},

                                                 // Common abbreviations
                                                 {"(^|/)(rh|redhat)(:|/|$)", "rhel"},
                                                 {"(^|/)ubi([0-9]?)(:|/|$)", "rhel"},

                                                 // Substring fallback
                                                 {"([a-z]+)(-|_)?(linux|os)", "$1"}};

    // Try each pattern
    for (const auto &[pattern, replacement] : patterns) {
        QRegularExpression rx(pattern);
        QRegularExpressionMatch match = rx.match(image);
        if (match.hasMatch()) {
            QString matched;
            if (replacement.startsWith('$')) {
                int group = replacement.mid(1).toInt();
                matched = match.captured(group);
            } else {
                matched = replacement;
            }

            // Validate against known distros
            for (const QString &distro : DISTROS) {
                if (matched.contains(distro)) {
                    return distro;
                }
            }
        }
    }

    // Final fallback
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

QStringList Backend::availableBackends() const
{
    QStringList result;

    auto isAvailable = [this](const QString &name) -> bool {
        if (m_isFlatpak) {
            QProcess whichProcess;
            whichProcess.start("flatpak-spawn", {"--host", "which", name});
            whichProcess.waitForFinished(500);
            return (whichProcess.exitCode() == 0);
        } else {
            return !QStandardPaths::findExecutable(name).isEmpty();
        }
    };

    if (isAvailable("distrobox"))
        result << "distrobox";
    if (isAvailable("toolbox"))
        result << "toolbox";

    return result;
}

QStringList Backend::getAvailableTerminals() const
{
    QStringList availableTerminals;
    auto terminalInfoMap = getTerminalInfoMap();

    for (const auto &[term, info] : terminalInfoMap.asKeyValueRange()) {
        if (!isFlatpakTerminal(term)) {
            bool terminalAvailable = false;
            if (m_isFlatpak) {
                QProcess whichProcess;
                whichProcess.start("flatpak-spawn", {"--host", "which", term});
                whichProcess.waitForFinished(500);
                terminalAvailable = (whichProcess.exitCode() == 0);
            } else {
                terminalAvailable = !QStandardPaths::findExecutable(term).isEmpty();
            }
            if (terminalAvailable) {
                availableTerminals.append(term);
            }
        }
    }

    // Check Flatpak terminals
    for (const auto &[term, info] : terminalInfoMap.asKeyValueRange()) {
        if (isFlatpakTerminal(term)) {
            bool terminalAvailable = false;
            if (m_isFlatpak) {
                QProcess process;
                process.start("flatpak-spawn", {"--host", "flatpak", "list", "--app", "--columns=application"});
                if (process.waitForFinished(500) && process.readAllStandardOutput().contains(term.toUtf8())) {
                    terminalAvailable = true;
                }
            } else {
                QProcess process;
                process.start("flatpak", {"list", "--app", "--columns=application"});
                if (process.waitForFinished(500) && process.readAllStandardOutput().contains(term.toUtf8())) {
                    terminalAvailable = true;
                }
            }
            if (terminalAvailable) {
                availableTerminals.append(term);
            }
        }
    }

    return availableTerminals;
}

QList<QMap<QString, QString>> Backend::getContainers() const
{
    QString output;
    if (m_preferredBackend == "distrobox") {
        output = runCommand({"distrobox", "list", "--no-color"});
    } else if (m_preferredBackend == "toolbox") {
        output = runCommand({"toolbox", "list", "-c"});
    } else {
        return {};
    }

    QStringList lines = output.split('\n', Qt::SkipEmptyParts);
    if (lines.isEmpty())
        return {};

    QList<QMap<QString, QString>> containers;

    if (m_preferredBackend == "distrobox") {
        // Pipe-separated table (with header)
        QStringList headers;
        for (const QString &col : lines[0].split('|', Qt::SkipEmptyParts)) {
            headers << col.trimmed();
        }

        for (int i = 1; i < lines.size(); ++i) {
            QStringList parts;
            for (const QString &col : lines[i].split('|', Qt::SkipEmptyParts)) {
                parts << col.trimmed();
            }

            if (parts.size() < headers.size())
                continue;

            QMap<QString, QString> container;
            container["id"] = parts[headers.indexOf("ID")];
            container["name"] = parts[headers.indexOf("NAME")];
            container["status"] = parts[headers.indexOf("STATUS")];
            container["image"] = parts[headers.indexOf("IMAGE")];
            container["distro"] = parseDistroFromImage(container["image"]);
            container["icon"] = getDistroIcon(container["distro"]);

            containers << container;
        }
    } else if (m_preferredBackend == "toolbox") {
        // Toolbox format: ID NAME CREATED STATUS IMAGE
        // Example: "2a16a2f1137c  fed  About an hour ago  created  registry.fedoraproject.org/fedora-toolbox:latest"

        for (int i = 1; i < lines.size(); ++i) {
            QString line = lines[i].trimmed();

            // Split on two or more spaces to handle the column formatting
            QStringList parts = line.split(QRegularExpression("\\s{2,}"), Qt::SkipEmptyParts);

            // Need at least ID, NAME, and IMAGE (some columns might be missing)
            if (parts.size() < 3)
                continue;

            QMap<QString, QString> container;

            // First part is always ID
            container["id"] = parts[0].trimmed();

            // Second part is NAME (might be followed by CREATED/STATUS)
            container["name"] = parts[1].trimmed();

            // Last part is always IMAGE
            container["image"] = parts.last().trimmed();

            // Set default status
            container["status"] = "running";

            container["distro"] = parseDistroFromImage(container["image"]);
            container["icon"] = getDistroIcon(container["distro"]);

            containers << container;
        }
    }
    return containers;
}

QString Backend::createContainer(const QString &name, const QString &image, const QString &home, bool init, const QStringList &volumes)
{
    QStringList args;
    if (m_preferredBackend == "distrobox") {
        args = {"distrobox", "create", "-n", name, "-i", image, "-Y"};
        if (init)
            args << "--init" << "--additional-packages" << "systemd";
        if (!home.isEmpty())
            args << "--home" << home;
        for (const QString &v : volumes) {
            args << "--volume" << v;
        }
    } else if (m_preferredBackend == "toolbox") {
        args = {"toolbox", "create", "-c", name, "-i", image, "-y"};
    } else {
        return i18n("Error: Failed to create Container.");
    }
    return runCommand(args);
}

QString Backend::deleteContainer(const QString &name)
{
    if (m_preferredBackend == "distrobox") {
        return runCommand({"distrobox", "rm", name, "--force"});
    } else if (m_preferredBackend == "toolbox") {
        return runCommand({"toolbox", "rm", name, "--force"});
    } else {
        return i18n("Error: Failed to delete Container.");
    }
}

void Backend::executeInTerminal(const QString &terminal, const QString &command)
{
    auto terminalInfoMap = getTerminalInfoMap();
    if (!terminalInfoMap.contains(terminal)) {
        qWarning() << i18n("Unknown terminal:") << terminal;
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

        // Check if terminal exists on host using flatpak-spawn --host which
        QProcess whichProcess;
        whichProcess.start("flatpak-spawn", {"--host", "which", executable});
        whichProcess.waitForFinished();

        if (whichProcess.exitCode() == 0) {
            bool success = QProcess::startDetached("flatpak-spawn", flatpakArgs.mid(1));
            if (!success) {
                qWarning() << i18n("Failed to start terminal") << executable << i18n("with args") << args;
                QProcess::startDetached("flatpak-spawn", {"--host", "xterm", "-e", command});
            }
        } else {
            QProcess::startDetached("flatpak-spawn", {"--host", "xterm", "-e", command});
        }
    } else {
        if (!QStandardPaths::findExecutable(executable).isEmpty()) {
            bool success = QProcess::startDetached(executable, args);
            if (!success) {
                qWarning() << i18n("Failed to start terminal") << executable << i18n("with args") << args;
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
            result = i18n("Error: %1", process->readAllStandardError());
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
    if (m_preferredBackend == "distrobox") {
        executeInTerminal(terminal, QString("distrobox enter %1").arg(name));
    } else if (m_preferredBackend == "toolbox") {
        executeInTerminal(terminal, QString("toolbox enter %1").arg(name));
    } else {
        // Bruh
    }
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
    if (m_preferredBackend == "distrobox") {
        QString fullCmd = QString("distrobox enter %1 -- %2").arg(containerName, command);
        qDebug() << "[installDebPackage] Using distrobox with command:" << fullCmd;
        executeInTerminal(terminal, fullCmd);
    } else if (m_preferredBackend == "toolbox") {
        QString fullCmd = QString("toolbox run -c %1 sh -c \"%2\"").arg(containerName, command);
        qDebug() << "[installDebPackage] Using toolbox with command:" << fullCmd;
        executeInTerminal(terminal, fullCmd);
    } else {
        qWarning() << "[installDebPackage] Unknown backend:" << m_preferredBackend;
    }
}

void Backend::installRpmPackage(const QString &terminal, const QString &containerName, const QString &filePath)
{
    QString command = QString("sudo dnf install -y %1").arg(filePath);
    if (m_preferredBackend == "distrobox") {
        QString fullCmd = QString("distrobox enter %1 -- %2").arg(containerName, command);
        qDebug() << "[installRpmPackage] Using distrobox with command:" << fullCmd;
        executeInTerminal(terminal, fullCmd);
    } else if (m_preferredBackend == "toolbox") {
        QString fullCmd = QString("toolbox run -c %1 sh -c \"%2\"").arg(containerName, command);
        qDebug() << "[installRpmPackage] Using toolbox with command:" << fullCmd;
        executeInTerminal(terminal, fullCmd);
    } else {
        qWarning() << "[installRpmPackage] Unknown backend:" << m_preferredBackend;
    }
}

void Backend::installArchPackage(const QString &terminal, const QString &containerName, const QString &filePath)
{
    QString command = QString("sudo pacman -U --noconfirm %1").arg(filePath);
    if (m_preferredBackend == "distrobox") {
        QString fullCmd = QString("distrobox enter %1 -- %2").arg(containerName, command);
        qDebug() << "[installArchPackage] Using distrobox with command:" << fullCmd;
        executeInTerminal(terminal, fullCmd);
    } else if (m_preferredBackend == "toolbox") {
        QString fullCmd = QString("toolbox run -c %1 sh -c \"%2\"").arg(containerName, command);
        qDebug() << "[installArchPackage] Using toolbox with command:" << fullCmd;
        executeInTerminal(terminal, fullCmd);
    } else {
        qWarning() << "[installArchPackage] Unknown backend:" << m_preferredBackend;
    }
}

QStringList Backend::getAvailableApps(const QString &containerName)
{
    QString output;
    if (m_preferredBackend == "distrobox") {
        output = runCommand({"distrobox",
                             "enter",
                             containerName,
                             "--",
                             "sh",
                             "-c",
                             "find /usr/share/applications -name '*.desktop' ! -exec grep -q '^NoDisplay=true' {} \\; -print"});
    } else if (m_preferredBackend == "toolbox") {
        output = runCommand({"toolbox",
                             "run",
                             "-c",
                             containerName,
                             "sh",
                             "-c",
                             "find /usr/share/applications -name '*.desktop' ! -exec grep -q '^NoDisplay=true' {} \\; -print"});
    } else {
        // I dont know
    }

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
        QString fileName = file.fileName(); // e.g. "mycontainer-org.kde.kcalc.desktop"
        QString appId = fileName.mid(prefix.length());
        appId.chop(QStringLiteral(".desktop").length());
        apps << appId; // e.g. "org.kde.kcalc"
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

void Backend::installDebPackageNoTerminal(const QString &containerName, const QString &filePath)
{
    qDebug() << "[installDebPackageNoTerminal] Installing" << filePath << "in container" << containerName << "(Flatpak:" << m_isFlatpak << ")";

    QProcess *process = new QProcess(this);
    QStringList args;

    if (m_isFlatpak) {
        args << "flatpak-spawn" << "--host" << "distrobox" << "enter" << containerName << "--" << "sudo" << "apt" << "install" << "-y" << filePath;
    } else {
        args << "distrobox" << "enter" << containerName << "--" << "sudo" << "apt" << "install" << "-y" << filePath;
    }

    qDebug() << "[installDebPackageNoTerminal] Command:" << args;

    connect(process, &QProcess::finished, this, [this, process](int exitCode) {
        QString stdoutData = process->readAllStandardOutput();
        QString stderrData = process->readAllStandardError();

        qDebug() << "[installDebPackageNoTerminal] Exit code:" << exitCode;
        qDebug() << "[installDebPackageNoTerminal] Stdout:" << stdoutData;
        qDebug() << "[installDebPackageNoTerminal] Stderr:" << stderrData;

        QString result = stdoutData;
        if (exitCode != 0) {
            result += "\nError: " + stderrData;
        }

        emit debInstallFinished(result);
        process->deleteLater();
    });

    process->start(args[0], args.mid(1));
}

void Backend::installRpmPackageNoTerminal(const QString &containerName, const QString &filePath)
{
    qDebug() << "[installRpmPackageNoTerminal] Installing" << filePath << "in container" << containerName << "(Flatpak:" << m_isFlatpak << ")";

    QProcess *process = new QProcess(this);
    QStringList args;

    if (m_isFlatpak) {
        args << "flatpak-spawn" << "--host" << "distrobox" << "enter" << containerName << "--" << "sudo" << "dnf" << "install" << "-y" << filePath;
    } else {
        args << "distrobox" << "enter" << containerName << "--" << "sudo" << "dnf" << "install" << "-y" << filePath;
    }

    qDebug() << "[installRpmPackageNoTerminal] Command:" << args;

    connect(process, &QProcess::finished, this, [this, process](int exitCode) {
        QString stdoutData = process->readAllStandardOutput();
        QString stderrData = process->readAllStandardError();

        qDebug() << "[installRpmPackageNoTerminal] Exit code:" << exitCode;
        qDebug() << "[installRpmPackageNoTerminal] Stdout:" << stdoutData;
        qDebug() << "[installRpmPackageNoTerminal] Stderr:" << stderrData;

        QString result = stdoutData;
        if (exitCode != 0) {
            result += "\nError: " + stderrData;
        }

        emit rpmInstallFinished(result);
        process->deleteLater();
    });

    process->start(args[0], args.mid(1));
}

void Backend::installArchPackageNoTerminal(const QString &containerName, const QString &filePath)
{
    qDebug() << "[installArchPackageNoTerminal] Installing" << filePath << "in container" << containerName << "(Flatpak:" << m_isFlatpak << ")";

    QProcess *process = new QProcess(this);
    QStringList args;

    if (m_isFlatpak) {
        args << "flatpak-spawn" << "--host" << "distrobox" << "enter" << containerName << "--" << "sudo" << "pacman" << "-U" << "--noconfirm" << filePath;
    } else {
        args << "distrobox" << "enter" << containerName << "--" << "sudo" << "pacman" << "-U" << "--noconfirm" << filePath;
    }

    qDebug() << "[installArchPackageNoTerminal] Command:" << args;

    connect(process, &QProcess::finished, this, [this, process](int exitCode) {
        QString stdoutData = process->readAllStandardOutput();
        QString stderrData = process->readAllStandardError();

        qDebug() << "[installArchPackageNoTerminal] Exit code:" << exitCode;
        qDebug() << "[installArchPackageNoTerminal] Stdout:" << stdoutData;
        qDebug() << "[installArchPackageNoTerminal] Stderr:" << stderrData;

        QString result = stdoutData;
        if (exitCode != 0) {
            result += "\nError: " + stderrData;
        }

        emit archInstallFinished(result);
        process->deleteLater();
    });

    process->start(args[0], args.mid(1));
}

void Backend::upgradeContainerNoTerminal(const QString &containerName)
{
    qDebug() << "[upgradeContainerNoTerminal] Upgrading container" << containerName << "(Flatpak:" << m_isFlatpak << ")";

    QProcess *process = new QProcess(this);
    QStringList args;

    if (m_isFlatpak) {
        args << "flatpak-spawn" << "--host" << "distrobox-upgrade" << containerName;
    } else {
        args << "distrobox-upgrade" << containerName;
    }

    qDebug() << "[upgradeContainerNoTerminal] Command:" << args;

    connect(process, &QProcess::finished, this, [this, process](int exitCode) {
        QString stdoutData = process->readAllStandardOutput();
        QString stderrData = process->readAllStandardError();

        qDebug() << "[upgradeContainerNoTerminal] Exit code:" << exitCode;
        qDebug() << "[upgradeContainerNoTerminal] Stdout:" << stdoutData;
        qDebug() << "[upgradeContainerNoTerminal] Stderr:" << stderrData;

        QString result = stdoutData;
        if (exitCode != 0) {
            result += "\nError: " + stderrData;
        }

        emit upgradeFinished(result);
        process->deleteLater();
    });

    process->start(args[0], args.mid(1));
}

void Backend::upgradeAllContainersNoTerminal()
{
    qDebug() << "[upgradeAllContainersNoTerminal] Upgrading all containers (Flatpak:" << m_isFlatpak << ")";

    QProcess *process = new QProcess(this);
    QStringList args;

    if (m_isFlatpak) {
        args << "flatpak-spawn" << "--host" << "distrobox-upgrade" << "--all";
    } else {
        args << "distrobox-upgrade" << "--all";
    }

    qDebug() << "[upgradeAllContainersNoTerminal] Command:" << args;

    connect(process, &QProcess::finished, this, [this, process](int exitCode) {
        QString stdoutData = process->readAllStandardOutput();
        QString stderrData = process->readAllStandardError();

        qDebug() << "[upgradeAllContainersNoTerminal] Exit code:" << exitCode;
        qDebug() << "[upgradeAllContainersNoTerminal] Stdout:" << stdoutData;
        qDebug() << "[upgradeAllContainersNoTerminal] Stderr:" << stderrData;

        QString result = stdoutData;
        if (exitCode != 0) {
            result += "\nError: " + stderrData;
        }

        emit upgradeAllFinished(result);
        process->deleteLater();
    });

    process->start(args[0], args.mid(1));
}
