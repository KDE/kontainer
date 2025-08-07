// SPDX-License-Identifier: GPL-2.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2025 Hadi Chokr <hadichokr@icloud.com>

#include "backend.h"
#include "appflags.h"
#include "packagemanager.h"
#include <mainwindow.h>
#include <toolboximages.h>

Backend::Backend(QObject *parent)
    : QObject(parent)
{
    m_isFlatpak = QFile::exists("/.flatpak-info");

    QSettings settings;
    m_preferredBackend = settings.value("container/backend", "distrobox").toString();

    connect(this, &Backend::containersFetched, this, [this](const QList<QMap<QString, QString>> &containers) {
        m_currentContainers = containers;
    });

    checkAvailableBackends();

    checkTerminaljob();
}

// Getter and technically setter
bool Backend::isTerminalJobPossible() {
    checkTerminaljob();
    return m_isTerminalJobPossible;
}

void Backend::checkTerminaljob()
{
    if (g_noTerminal) {
        qDebug() << "Terminal job check skipped due to --no-terminal flag";
        m_isTerminalJobPossible = false;
        return;
    }

    KTerminalLauncherJob job(QStringLiteral("true"));
    m_isTerminalJobPossible = job.prepare();
}

void Backend::checkAvailableBackends()
{
    auto isAvailable = [this](const QString &name) -> QFuture<bool> {
        return QtConcurrent::run([this, name]() -> bool {
            if (m_isFlatpak) {
                QProcess whichProcess;
                whichProcess.start("flatpak-spawn", {"--host", "which", name});
                if (!whichProcess.waitForStarted()) {
                    return false;
                }
                QEventLoop loop;
                QObject::connect(&whichProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), &loop, &QEventLoop::quit);
                loop.exec();
                return (whichProcess.exitCode() == 0);
            } else {
                return !QStandardPaths::findExecutable(name).isEmpty();
            }
        });
    };

    const QList<QString> backends = {"distrobox", "toolbox"};
    QList<QFuture<bool>> futures;

    for (const auto &backend : backends) {
        futures.append(isAvailable(backend));
    }

    // Create a combined future that waits for all checks to complete
    auto combinedFuture = QtFuture::whenAll(futures.begin(), futures.end());

    combinedFuture.then(this, [this, backends](const QList<QFuture<bool>> &results) {
        QList<QString> availableBackends;
        for (int i = 0; i < results.size(); ++i) {
            if (results[i].result()) {
                availableBackends.append(backends[i]);
            }
        }
        m_cachedBackends = availableBackends;
        validatePreferredBackend();
        Q_EMIT availableBackendsChanged(m_cachedBackends);
    });
}

QStringList Backend::availableBackends() const
{
    return m_cachedBackends;
}

QString Backend::preferredBackend() const
{
    return m_preferredBackend;
}

void Backend::setPreferredBackend(const QString &backend)
{
    if (m_preferredBackend != backend && m_cachedBackends.contains(backend)) {
        m_preferredBackend = backend;
        QSettings settings;
        settings.setValue("container/backend", backend);
    }
}

void Backend::validatePreferredBackend()
{
    if (!m_cachedBackends.contains(m_preferredBackend) && !m_cachedBackends.isEmpty()) {
        m_preferredBackend = m_cachedBackends.first();
        QSettings settings;
        settings.setValue("container/backend", m_preferredBackend);
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

QString Backend::getContainerDistro(const QString &containerName) const
{
    if (containerName.isEmpty())
        return "";

    for (const auto &container : m_currentContainers) {
        if (container["name"] == containerName) {
            return PackageManager::getDistroFromImage(container["image"]);
        }
    }
    return "";
}

void Backend::fetchContainersAsync()
{
    auto *process = new QProcess(this);
    process->setProcessChannelMode(QProcess::MergedChannels);

    QStringList command;
    if (m_preferredBackend == "distrobox") {
        command = {"distrobox", "list", "--no-color"};
    } else if (m_preferredBackend == "toolbox") {
        command = {"toolbox", "list", "-c"};
    } else {
        emit containersFetched({});
        return;
    }

    // Apply Flatpak wrapper if needed
    QStringList actualCommand;
    if (m_isFlatpak) {
        actualCommand << "flatpak-spawn" << "--host" << command;
    } else {
        actualCommand = command;
    }

    process->start(actualCommand[0], actualCommand.mid(1));

    connect(process, &QProcess::finished, this, [this, process](int exitCode, QProcess::ExitStatus exitStatus) {
        QList<QMap<QString, QString>> containers;

        if (exitStatus != QProcess::NormalExit || exitCode != 0) {
            emit containersFetched({});
            process->deleteLater();
            return;
        }

        QString output = QString::fromUtf8(process->readAllStandardOutput());
        QStringList lines = output.split('\n', Qt::SkipEmptyParts);

        if (lines.isEmpty()) {
            emit containersFetched({});
            process->deleteLater();
            return;
        }

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

                container["distro"] = getDistroFromToolboxImage(container["image"]);
                container["icon"] = getDistroIcon(container["distro"]);

                containers << container;
            }
        }

        emit containersFetched(containers);
        process->deleteLater();
    });

    // Handle error case
    connect(process, &QProcess::errorOccurred, this, [this, process](QProcess::ProcessError error) {
        qWarning() << "Failed to fetch containers:" << process->errorString();
        emit containersFetched({});
        process->deleteLater();
    });
}

QString Backend::createContainer(const QString &name, const QString &image, const QString &home, bool init, const QStringList &volumes)
{
    emit containerCreationStarted();

    m_createProcess = new QProcess(this);
    m_createProcess->setProcessChannelMode(QProcess::MergedChannels);

    QString result;
    bool success = false;
    QEventLoop loop;

    // Collect all output
    QString output;
    connect(m_createProcess, &QProcess::readyReadStandardOutput, this, [this, &output]() {
        QString text = QString::fromUtf8(m_createProcess->readAllStandardOutput());
        output += text;
        emit containerOutput(text);
    });

    connect(m_createProcess, &QProcess::readyReadStandardError, this, [this, &output]() {
        QString text = QString::fromUtf8(m_createProcess->readAllStandardError());
        output += text;
        emit containerOutput(text);
    });

    connect(m_createProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [&](int exitCode, QProcess::ExitStatus exitStatus) {
        success = (exitStatus == QProcess::NormalExit && exitCode == 0);
        output += QString::fromUtf8(m_createProcess->readAllStandardOutput());
        output += QString::fromUtf8(m_createProcess->readAllStandardError());
        loop.quit();
    });

    // Build args
    QStringList args;
    if (m_preferredBackend == "distrobox") {
        args = {m_isFlatpak ? "flatpak-spawn" : "distrobox"};
        if (m_isFlatpak)
            args << "--host" << "distrobox";
        args << "create" << "-n" << name << "-i" << image << "-Y";

        if (init)
            args << "--init" << "--additional-packages" << "systemd";
        if (!home.isEmpty())
            args << "--home" << home;
        for (const QString &v : volumes)
            args << "--volume" << v;

    } else if (m_preferredBackend == "toolbox") {
        args = {m_isFlatpak ? "flatpak-spawn" : "toolbox"};
        if (m_isFlatpak)
            args << "--host" << "toolbox";
        args << "create" << "-c" << name << "-i" << image << "-y";

    } else {
        return i18n("Error: No supported backend available");
    }

    // Start the process
    m_createProcess->start(args.first(), args.mid(1));
    if (!m_createProcess->waitForStarted()) {
        return i18n("Error: Failed to start container creation process");
    }

    loop.exec();

    QString message = success ? i18n("Container created successfully") : i18n("Container creation failed");
    emit containerCreationFinished(success, message + "\n\n" + output);

    m_createProcess->deleteLater();
    m_createProcess = nullptr;

    return success ? message : i18n("Error: ") + output;
}

QString Backend::deleteContainer(const QString &name)
{
    QString appsPath;

    if (m_preferredBackend == "distrobox") {
        return runCommand({"distrobox", "rm", name, "--force"});
    } else if (m_preferredBackend == "toolbox") {
        // First remove all exported desktop files for this container
        if (m_isFlatpak) {
            // Access host's ~/.local/share/applications manually
            QString home = qEnvironmentVariable("HOME");
            appsPath = home + "/.local/share/applications";
        } else {
            appsPath = QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation);
        }

        QDir appsDir(appsPath);
        QStringList exportedFiles = appsDir.entryList({"*-" + name + ".desktop"}, QDir::Files);

        for (const QString &file : exportedFiles) {
            QFile::remove(appsDir.filePath(file));
        }

        // Update desktop database after removal
        QString updateResult = runCommand({"update-desktop-database", appsDir.path()});
        if (updateResult.startsWith("Error:")) {
            qWarning() << "Failed to update desktop database:" << updateResult;
        }

        // Now remove the container
        QString result = runCommand({"toolbox", "rm", name, "--force"});
        if (result.startsWith("Error:")) {
            return i18nc("Error message when toolbox deletion fails", "Failed to delete Toolbox %1. Error: %2", name, result);
        }

        return i18nc("Inform the User that the Toolbox Deletion Completed since it returns no output on its own", "Toolbox %1 deleted successfully.", name);
    } else {
        return i18n("Error: Failed to delete Container.");
    }
}

QString Backend::resolveBinaryPath(const QString &binary)
{
    // Hack needed for distrobox as KTerminalLauncherJob doesnt seem to want to launch it without full path
    if (m_isFlatpak) {
        QProcess proc;
        proc.start("flatpak-spawn", {"--host", "which", binary});
        proc.waitForFinished();
        QString output = QString::fromUtf8(proc.readAllStandardOutput()).trimmed();
        return output.isEmpty() ? binary : output;
    } else {
        QString path = QStandardPaths::findExecutable(binary);
        return path.isEmpty() ? binary : path;
    }
}

void Backend::executeInTerminal(const QString &command)
{
    KTerminalLauncherJob *job = nullptr;
    if (m_isFlatpak) {
        job = new KTerminalLauncherJob("flatpak-spawn --host " + command);
    } else {
        job = new KTerminalLauncherJob(command);
    }
    job->start();
}

void Backend::enterContainer(const QString &name)
{
    if (m_preferredBackend == "distrobox") {
        QString bin = resolveBinaryPath("distrobox");
        executeInTerminal(bin + " enter " + name);
    } else if (m_preferredBackend == "toolbox") {
        QString bin = resolveBinaryPath("toolbox");
        executeInTerminal(bin + " enter " + name);
    }
}

void Backend::upgradeContainer(const QString &name)
{
    QString bin = resolveBinaryPath("distrobox-upgrade");
    executeInTerminal(bin + " " + name);
}

void Backend::upgradeAllContainers()
{
    QString bin = resolveBinaryPath("distrobox-upgrade");
    executeInTerminal(bin + " --all");
}

void Backend::installDebPackage(const QString &containerName, const QString &filePath)
{
    QString command = "sudo apt install -y " + filePath;
    if (m_preferredBackend == "distrobox") {
        QString bin = resolveBinaryPath("distrobox");
        executeInTerminal(bin + " enter " + containerName + " -- " + command);
    } else if (m_preferredBackend == "toolbox") {
        QString bin = resolveBinaryPath("toolbox");
        executeInTerminal(bin + " run -c " + containerName + " " + command);
    }
}

void Backend::installRpmPackage(const QString &containerName, const QString &filePath)
{
    QString command = "sudo dnf install -y " + filePath;
    if (m_preferredBackend == "distrobox") {
        QString bin = resolveBinaryPath("distrobox");
        executeInTerminal(bin + " enter " + containerName + " -- " + command);
    } else if (m_preferredBackend == "toolbox") {
        QString bin = resolveBinaryPath("toolbox");
        executeInTerminal(bin + " run -c " + containerName + " " + command);
    }
}

void Backend::installArchPackage(const QString &containerName, const QString &filePath)
{
    QString command = "sudo pacman -U --noconfirm " + filePath;
    if (m_preferredBackend == "distrobox") {
        QString bin = resolveBinaryPath("distrobox");
        executeInTerminal(bin + " enter " + containerName + " -- " + command);
    } else if (m_preferredBackend == "toolbox") {
        QString bin = resolveBinaryPath("toolbox");
        executeInTerminal(bin + " run -c " + containerName + " " + command);
    }
}

void Backend::assembleContainer(const QString &iniFile)
{
    QString bin = resolveBinaryPath("distrobox-assemble");
    QString command = QString("%1 create --file \"%2\"").arg(bin, iniFile);

    if (isTerminalJobPossible()) {
        executeInTerminal(command);
        return;
    }

    emit assembleStartedWithDialog();

    QProcess *process = new QProcess(this);

    connect(process, &QProcess::readyReadStandardOutput, this, [this, process]() {
        QString output = QString::fromLocal8Bit(process->readAllStandardOutput());
        emit assembleFinished(output);
    });

    connect(process, &QProcess::readyReadStandardError, this, [this, process]() {
        QString error = QString::fromLocal8Bit(process->readAllStandardError());
        emit assembleFinished(error);
    });

    connect(process, &QProcess::finished, process, &QProcess::deleteLater);

    if (m_isFlatpak) {
        process->start("flatpak-spawn", {"--host", "distrobox", "assemble", "create", "--file", iniFile});
    } else {
        process->start("distrobox", {"assemble", "create", "--file", iniFile});
    }
}

QString Backend::getDistroFromToolboxImage(const QString &image) const
{
    for (const auto &entry : toolboxImages) {
        if (entry.image == image) {
            return entry.distro;
        }
    }

    // Fallback: use regex-based parser for unknown URLs
    return parseDistroFromImage(image);
}

QList<QMap<QString, QString>> Backend::getAvailableImages()
{
    QList<QMap<QString, QString>> images;

    if (m_preferredBackend == "toolbox") {
        // Handle toolbox images
        for (const auto &entry : toolboxImages) {
            QMap<QString, QString> image;
            QString imageUrl = entry.image;
            QString distro = entry.distro;
            QString version = entry.version;

            image["url"] = imageUrl;
            image["name"] = imageUrl.split('/').last();
            image["distro"] = distro;
            image["version"] = version;
            image["icon"] = getDistroIcon(distro);
            image["display"] = QString("%1 %2").arg(distro, version);

            images.append(image);
        }
    } else {
        // Handle distrobox images
        QString output = runCommand({"distrobox", "create", "-C"});
        for (const QString &line : output.split('\n', Qt::SkipEmptyParts)) {
            QMap<QString, QString> image;
            QString trimmed = line.trimmed();
            image["url"] = trimmed;
            image["name"] = trimmed.split('/').last();
            image["distro"] = parseDistroFromImage(trimmed);
            image["icon"] = getDistroIcon(image["distro"]);
            image["display"] = trimmed;
            images.append(image);
        }
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

// Modified implementation:
void Backend::installPackageNoTerminal(const QString &containerName, const QString &filePath, const QString &packageCommand, const QString &signalName)
{
    qDebug().nospace() << "[installPackageNoTerminal] Installing " << filePath << " in container " << containerName << " (Flatpak:" << m_isFlatpak
                       << ", Backend:" << m_preferredBackend << ")";

    QProcess *process = new QProcess(this);
    QStringList args;
    QString fullCommand = QString("sudo %1 %2").arg(packageCommand, filePath);

    if (m_preferredBackend == "distrobox") {
        args = buildDistroboxCommand(containerName, fullCommand);
    } else if (m_preferredBackend == "toolbox") {
        args = buildToolboxCommand(containerName, fullCommand);
    } else {
        qWarning() << "[installPackageNoTerminal] Unknown backend:" << m_preferredBackend;
        emit packageInstallFinished(signalName, "Error: Unknown container backend");
        process->deleteLater();
        return;
    }

    qDebug() << "[installPackageNoTerminal] Command:" << args;

    // Connect real-time output signals
    connect(process, &QProcess::readyReadStandardOutput, [this, process]() {
        QString output = process->readAllStandardOutput();
        emit outputReceived(output);
    });

    connect(process, &QProcess::readyReadStandardError, [this, process]() {
        QString output = process->readAllStandardError();
        emit outputReceived(output);
    });

    connect(process, &QProcess::finished, this, [this, process, signalName](int exitCode) {
        handlePackageInstallFinished(process, exitCode, signalName);
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

    // Connect real-time output signals
    connect(process, &QProcess::readyReadStandardOutput, [this, process]() {
        QString output = process->readAllStandardOutput();
        emit outputReceived(output);
    });

    connect(process, &QProcess::readyReadStandardError, [this, process]() {
        QString output = process->readAllStandardError();
        emit outputReceived(output);
    });

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

    // Connect real-time output signals
    connect(process, &QProcess::readyReadStandardOutput, [this, process]() {
        QString output = process->readAllStandardOutput();
        emit outputReceived(output);
    });

    connect(process, &QProcess::readyReadStandardError, [this, process]() {
        QString output = process->readAllStandardError();
        emit outputReceived(output);
    });

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

QStringList Backend::buildDistroboxCommand(const QString &containerName, const QString &command)
{
    QStringList args;
    if (m_isFlatpak) {
        args << "flatpak-spawn" << "--host";
    }
    args << "distrobox" << "enter" << containerName << "--" << command.split(" ");
    return args;
}

QStringList Backend::buildToolboxCommand(const QString &containerName, const QString &command)
{
    QStringList args;
    if (m_isFlatpak) {
        args << "flatpak-spawn" << "--host";
    }
    args << "toolbox" << "run" << "-c" << containerName << "--";

    // Handle quoted paths properly
    QStringList commandParts = command.split(" ", Qt::SkipEmptyParts);
    for (const QString &part : commandParts) {
        args << (part.startsWith('/') ? QDir::toNativeSeparators(part) : part);
    }

    return args;
}

void Backend::handlePackageInstallFinished(QProcess *process, int exitCode, const QString &signalName)
{
    QString stdoutData = process->readAllStandardOutput();
    QString stderrData = process->readAllStandardError();

    qDebug() << "[handlePackageInstallFinished] Exit code:" << exitCode;
    qDebug() << "[handlePackageInstallFinished] Stdout:" << stdoutData;
    qDebug() << "[handlePackageInstallFinished] Stderr:" << stderrData;

    QString result = stdoutData;
    if (exitCode != 0) {
        result += "\nError: " + stderrData;
    }

    // Emit the specific signal based on signalName
    if (signalName == "debInstallFinished") {
        emit debInstallFinished(result);
    } else if (signalName == "rpmInstallFinished") {
        emit rpmInstallFinished(result);
    } else if (signalName == "archInstallFinished") {
        emit archInstallFinished(result);
    }

    // Also emit the generic signal
    emit packageInstallFinished(signalName, result);

    process->deleteLater();
}

void Backend::installDebPackageNoTerminal(const QString &containerName, const QString &filePath)
{
    installPackageNoTerminal(containerName, filePath, "apt install -y", "debInstallFinished");
}

void Backend::installRpmPackageNoTerminal(const QString &containerName, const QString &filePath)
{
    installPackageNoTerminal(containerName, filePath, "dnf install -y", "rpmInstallFinished");
}

void Backend::installArchPackageNoTerminal(const QString &containerName, const QString &filePath)
{
    installPackageNoTerminal(containerName, filePath, "pacman -U --noconfirm", "archInstallFinished");
}

QStringList Backend::getAvailableApps(const QString &containerName)
{
    // Find only valid .desktop files that are not NoDisplay=true
    QString findCmd =
    "find /usr/share/applications -type f -name '*.desktop' "
    "! -exec grep -q '^NoDisplay=true' {} \\; "
    "-exec test -f {} \\; "
    "-print";

    QString output;
    if (m_preferredBackend == "distrobox") {
        output = runCommand({
            "distrobox",
            "enter",
            containerName,
            "--",
            "sh",
            "--noprofile",
            "--norc",
            "-c",
            findCmd
        });
    } else if (m_preferredBackend == "toolbox") {
        output = runCommand({
            "toolbox",
            "run",
            "-c",
            containerName,
            "sh",
            "--noprofile",
            "--norc",
            "-c",
            findCmd
        });
    } else {
        return {};
    }

    QStringList apps;
    for (const QString &line : output.split('\n', Qt::SkipEmptyParts)) {
        if (QFileInfo(line).isFile() && line.endsWith(".desktop")) {
            apps << QFileInfo(line).baseName();
        }
    }

    return apps;
}


QStringList Backend::getExportedApps(const QString &containerName)
{
    QStringList apps;
    QString appsPath;

    if (m_isFlatpak) {
        appsPath = qEnvironmentVariable("HOME") + "/.local/share/applications";
    } else {
        appsPath = QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation);
    }

    QDir dir(appsPath);

    QStringList patterns;
    if (m_preferredBackend == "toolbox") {
        // Toolbox format: some-app-<container>.desktop
        patterns << QString("*-%1.desktop").arg(containerName);
    } else if (m_preferredBackend == "distrobox") {
        // Distrobox format: <container>-<app>.desktop
        patterns << QString("%1-*.desktop").arg(containerName);
    }

    for (const QFileInfo &file : dir.entryInfoList(patterns, QDir::Files)) {
        QString fileName = file.fileName(); // full filename with extension

        // Strip the ".desktop" suffix
        if (fileName.endsWith(".desktop")) {
            fileName.chop(8); // remove ".desktop"
        } else {
            continue; // not a .desktop file
        }

        QString appId;

        if (m_preferredBackend == "toolbox") {
            // remove trailing "-<containerName>"
            QString suffix = "-" + containerName;
            if (fileName.endsWith(suffix)) {
                appId = fileName.left(fileName.length() - suffix.length());
            }
        } else if (m_preferredBackend == "distrobox") {
            // remove leading "<containerName>-"
            QString prefix = containerName + "-";
            if (fileName.startsWith(prefix)) {
                appId = fileName.mid(prefix.length());
            }
        }

        if (!appId.isEmpty())
            apps << appId;
    }

    return apps;
}

QString Backend::exportApp(const QString &appName, const QString &containerName)
{
    if (m_preferredBackend == "distrobox") {
        QString desktopPath = "/usr/share/applications/" + appName + ".desktop";
        return runCommand({"distrobox", "enter", containerName, "--", "distrobox-export", "--app", desktopPath});
    } else {
        QString checkCmd = QString("toolbox run -c %1 which %2").arg(containerName, appName);
        if (runCommand({"sh", "-c", checkCmd}).isEmpty()) {
            return i18nc("Error message when application is not found in container", "Application %1 not found in container %2", appName, containerName);
        }

        QString desktopFile =
            runCommand({"toolbox", "run", "-c", containerName, "sh", "-c", QString("find /usr/share/applications -name '*%1*.desktop' | head -1").arg(appName)})
                .trimmed();

        if (desktopFile.isEmpty()) {
            return i18nc("Error message when desktop file is not found", "Could not find desktop file for %1 in container %2", appName, containerName);
        }

        QString desktopContent = runCommand({"toolbox", "run", "-c", containerName, "cat", desktopFile});

        QString appsPath;

        if (m_isFlatpak) {
            QString home = qEnvironmentVariable("HOME");
            appsPath = home + "/.local/share/applications";
        } else {
            appsPath = QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation);
        }

        QString exportedPath = appsPath + "/" + QFileInfo(desktopFile).completeBaseName() + "-" + containerName + ".desktop";

        QStringList lines = desktopContent.split('\n');
        QStringList newLines;
        bool hasNameTranslations = false;

        for (QString line : lines) {
            if (line.startsWith("Exec=")) {
                line = QString("Exec=toolbox run -c %1 %2").arg(containerName, line.mid(5));
            } else if (line.startsWith("Name=")) {
                line = QString("Name=%1 (on %2)").arg(line.mid(5), containerName);
            } else if (line.startsWith("Name[")) {
                hasNameTranslations = true;
            } else if (line.startsWith("GenericName=")) {
                line = QString("GenericName=%1 (on %2)").arg(line.mid(12), containerName);
            } else if (line.startsWith("TryExec=") || line == "DBusActivatable=true") {
                continue;
            }
            newLines << line;
        }

        if (hasNameTranslations) {
            for (int i = 0; i < newLines.size(); i++) {
                if (newLines[i].startsWith("Name[")) {
                    QString lang = newLines[i].section('[', 1).section(']', 0, 0);
                    QString value = newLines[i].section('=', 1);
                    newLines[i] = QString("Name[%1]=%2 (on %3)").arg(lang, value, containerName);
                }
            }
        }

        QFile file(exportedPath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            return i18nc("Error message when desktop file creation fails", "Failed to create desktop file: %1", exportedPath);
        }

        QTextStream out(&file);
        out << newLines.join('\n');
        file.close();

        runCommand({"update-desktop-database", appsPath});

        return i18nc("Success message after exporting application", "Successfully exported %1 from %2", appName, containerName);
    }
}

QString Backend::unexportApp(const QString &appName, const QString &containerName)
{
    QString appsPath;

    if (m_isFlatpak) {
        appsPath = qEnvironmentVariable("HOME") + "/.local/share/applications";
    } else {
        appsPath = QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation);
    }

    QString fileName;

    if (m_preferredBackend == "toolbox") {
        fileName = QString("%1-%2.desktop").arg(appName, containerName);
    } else if (m_preferredBackend == "distrobox") {
        fileName = QString("%1-%2.desktop").arg(containerName, appName);
    } else {
        fileName = QString("%1-%2.desktop").arg(appName, containerName);
    }

    QString exportedFile = appsPath + "/" + fileName;

    if (!QFile::exists(exportedFile)) {
        return i18nc("Error message when no exported app is found", "No exported application %1 found for container %2", appName, containerName);
    }

    if (!QFile::remove(exportedFile)) {
        return i18nc("Error message when desktop file removal fails", "Failed to remove desktop file: %1", exportedFile);
    }

    runCommand({"update-desktop-database", appsPath});

    return i18nc("Success message after unexporting application", "Successfully unexported %1 from %2", appName, containerName);
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
