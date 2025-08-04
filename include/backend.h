// SPDX-License-Identifier: GPL-2.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2025 Hadi Chokr <hadichokr@icloud.com>

#pragma once

#include <KLocalizedString>
#include <KTerminalLauncherJob>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QFuture>
#include <QMap>
#include <QObject>
#include <QProcess>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QString>
#include <QStringList>
#include <QtConcurrent/QtConcurrent>
#include <algorithm>

class Backend : public QObject
{
    Q_OBJECT
public:
    explicit Backend(QObject *parent = nullptr);
    QStringList availableBackends() const;
    void setPreferredBackend(const QString &backend);
    bool isTerminalJobPossible() const;

    // Container operations
    QString
    createContainer(const QString &name, const QString &image, const QString &home = QString(), bool init = false, const QStringList &volumes = QStringList());
    QString deleteContainer(const QString &name);
    void enterContainer(const QString &name);
    void upgradeContainer(const QString &name);
    void upgradeAllContainers();
    void executeInTerminal(const QString &command);
    void installDebPackageNoTerminal(const QString &containerName, const QString &filePath);
    void installRpmPackageNoTerminal(const QString &containerName, const QString &filePath);
    void installArchPackageNoTerminal(const QString &containerName, const QString &filePath);
    void upgradeContainerNoTerminal(const QString &containerName);
    void upgradeAllContainersNoTerminal();
    // App operations
    QStringList getAvailableApps(const QString &containerName);
    QStringList getExportedApps(const QString &containerName);
    QString exportApp(const QString &appName, const QString &containerName);
    QString unexportApp(const QString &appName, const QString &containerName);
    QString getContainerDistro(const QString &containerName) const;
    QString preferredBackend() const;
    void checkTerminaljob();

    // Image operations
    QList<QMap<QString, QString>> getAvailableImages();
    QList<QMap<QString, QString>> searchImages(const QString &query);

signals:
    void assembleFinished(const QString &result);
    void debInstallFinished(const QString &output);
    void rpmInstallFinished(const QString &output);
    void archInstallFinished(const QString &output);
    void upgradeFinished(const QString &output);
    void upgradeAllFinished(const QString &output);
    void packageInstallFinished(const QString &signalName, const QString &result);
    void outputReceived(const QString &output);
    void containerCreationStarted();
    void containerOutput(const QString &output);
    void containerCreationFinished(bool success, const QString &message);
    void availableBackendsChanged(const QStringList &backends);
    void containersFetched(const QList<QMap<QString, QString>> &containers);

public slots:
    void assembleContainer(const QString &iniFile);
    void installDebPackage(const QString &containerName, const QString &filePath);
    void installRpmPackage(const QString &containerName, const QString &filePath);
    void installArchPackage(const QString &containerName, const QString &filePath);
    void fetchContainersAsync();

private:
    QString resolveBinaryPath(const QString &binary);
    QString runCommand(const QStringList &command) const;
    QString parseDistroFromImage(const QString &imageUrl) const;
    QString getDistroIcon(const QString &distroName) const;
    bool m_isFlatpak = false;
    QString m_preferredBackend;
    void checkAvailableBackends();
    void validatePreferredBackend();
    QString getDistroFromToolboxImage(const QString &image) const;
    void installPackageNoTerminal(const QString &containerName, const QString &filePath, const QString &packageCommand, const QString &signalName);
    void handlePackageInstallFinished(QProcess *process, int exitCode, const QString &signalName);
    QStringList buildToolboxCommand(const QString &containerName, const QString &command);
    QStringList buildDistroboxCommand(const QString &containerName, const QString &command);
    QProcess *m_createProcess = nullptr;
    QStringList m_cachedBackends;
    QList<QMap<QString, QString>> m_currentContainers;
    bool m_isTerminalJobPossible;

    const QStringList DISTROS = {"alma",     "alpine",     "amazon", "amazonlinux", "arch",       "bazzite",   "blackarch",   "bluefin",  "bookworm",
                                 "bullseye", "buster",     "centos", "chainguard",  "clearlinux", "crystal",   "debian",      "deepin",   "fedora",
                                 "gentoo",   "kali",       "leap",   "linuxmint",   "mageia",     "neon",      "neurodebian", "opensuse", "oracle",
                                 "plasma",   "powershell", "redhat", "rhel",        "rocky",      "slackware", "steamos",     "toolbox",  "tumbleweed",
                                 "ubi",      "ublue",      "ubuntu", "vanilla",     "vso",        "void",      "wheezy",      "wolfi"};

    QMap<QString, QString> distroIconMap = {// Official distros
                                            {"alma", "almalinux.svg"},
                                            {"alpine", "alpine.svg"},
                                            {"amazon", "amazonlinux.svg"},
                                            {"amazonlinux", "amazonlinux.svg"},
                                            {"arch", "archlinux.svg"},
                                            {"blackarch", "blackarchlinux.svg"},
                                            {"centos", "centos.svg"},
                                            {"clearlinux", "clearlinux.svg"},
                                            {"debian", "debian.svg"},
                                            {"deepin", "deepin.svg"},
                                            {"fedora", "fedora.svg"},
                                            {"gentoo", "gentoo.svg"},
                                            {"kali", "kali-linux.svg"},
                                            {"linuxmint", "linuxmint.svg"},
                                            {"mageia", "mageia.svg"},
                                            {"neon", "kde-neon.svg"},
                                            {"neurodebian", "debian.svg"},
                                            {"opensuse", "opensuse.svg"},
                                            {"oracle", "oracle.svg"},
                                            {"redhat", "redhat.svg"},
                                            {"rhel", "redhat.svg"},
                                            {"rocky", "rocky-linux.svg"},
                                            {"slackware", "slackware.svg"},
                                            {"steamos", "steamos.svg"},
                                            {"ubuntu", "ubuntu.svg"},
                                            {"vanilla", "vanilla.svg"},
                                            {"vso", "vanilla.svg"},
                                            {"void", "void.svg"},
                                            {"wolfi", "wolfi.svg"},

                                            // Variants and derivatives
                                            {"bazzite", "bazzite.svg"},
                                            {"bluefin", "ublue.svg"},
                                            {"bullseye", "debian.svg"},
                                            {"buster", "debian.svg"},
                                            {"bookworm", "debian.svg"},
                                            {"leap", "opensuse.svg"},
                                            {"tumbleweed", "tumbleweed.svg"},
                                            {"plasma", "kde-neon.svg"},
                                            {"powershell", "tux.svg"}, // Generic icon
                                            {"toolbox", "tux.svg"}, // Generic icon
                                            {"ubi", "redhat.svg"}, // Red Hat Universal Base Image
                                            {"neurodebian", "debian.svg"},

                                            // Cloud/container specific
                                            {"chainguard", "tux.svg"},
                                            {"ublue", "ublue.svg"}, // Could use fedora.svg or create custom
                                            {"crystal", "crystal.svg"},

                                            // Default fallbacks
                                            {"unknown", "tux.svg"}};
};
