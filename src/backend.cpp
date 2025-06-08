#include "backend.h"
#include "terminalutils.h"

#include <QProcess>
#include <QDir>
#include <QFileInfo>
#include <QRegularExpression>
#include <QDebug>

Backend::Backend(QObject *parent) : QObject(parent) {}

QString Backend::runCommand(const QStringList &command)
{
    QProcess process;
    process.start(command[0], command.mid(1));
    if (!process.waitForFinished(60000)) {
        return "Error: Command timed out";
    }
    return process.readAllStandardOutput().trimmed();
}

QString Backend::parseDistroFromImage(const QString &imageUrl)
{
    QString last = imageUrl.split('/').last().toLower();
    for (const QString &d : DISTROS) {
        if (last.contains(d)) return d;
    }
    return "unknown";
}

QList<QMap<QString, QString>> Backend::getContainers()
{
    QString output = runCommand({"distrobox", "list", "--no-color"});
    QStringList lines = output.split('\n', Qt::SkipEmptyParts);
    if (lines.isEmpty()) return {};

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

        containers << container;
    }
    return containers;
}

QString Backend::createContainer(const QString &name, const QString &image,
                                const QString &home, bool init,
                                const QStringList &volumes)
{
    QStringList args = {"distrobox", "create", "-n", name, "-i", image, "-Y"};
    if (init) args << "--init" << "--additional-packages" << "systemd";
    if (!home.isEmpty()) args << "--home" << home;
    for (const QString &v : volumes) {
        args << "--volume" << v;
    }
    return runCommand(args);
}

QString Backend::deleteContainer(const QString &name)
{
    return runCommand({"distrobox", "rm", name, "--force"});
}

void Backend::executeInTerminal(const QString &terminal, const QString &command) {
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

    process->start("distrobox", {"assemble", "create", "--file", iniFile});
}

void Backend::enterContainer(const QString &name, const QString &terminal) {
    executeInTerminal(terminal, QString("distrobox enter %1").arg(name));
}

void Backend::upgradeContainer(const QString &name, const QString &terminal) {
    executeInTerminal(terminal, QString("distrobox-upgrade %1").arg(name));
}

void Backend::upgradeAllContainers(const QString &terminal) {
    executeInTerminal(terminal, "distrobox-upgrade --all");
}

QStringList Backend::getAvailableApps(const QString &containerName)
{
    QString output = runCommand({
        "distrobox", "enter", containerName, "--",
        "find", "/usr/share/applications", "-name", "*.desktop"
    });
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
    return runCommand({
        "distrobox", "enter", containerName, "--",
        "distrobox-export", "--app", appName
    });
}

QString Backend::unexportApp(const QString &appName, const QString &containerName)
{
    return runCommand({
        "distrobox", "enter", containerName, "--",
        "distrobox-export", "--app", appName, "--delete"
    });
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
        images.append(image);
    }

    return images;
}

QList<QMap<QString, QString>> Backend::searchImages(const QString &query)
{
    QList<QMap<QString, QString>> allImages = getAvailableImages();
    QList<QMap<QString, QString>> filteredImages;

    for (const auto &image : allImages) {
        if (image["name"].contains(query, Qt::CaseInsensitive) ||
            image["distro"].contains(query, Qt::CaseInsensitive) ||
            image["url"].contains(query, Qt::CaseInsensitive)) {
            filteredImages.append(image);
        }
    }

    return filteredImages;
}
