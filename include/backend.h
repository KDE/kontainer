#ifndef BACKEND_H
#define BACKEND_H

#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QMap>
#include <QObject>
#include <QProcess>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QString>
#include <QStringList>

class Backend : public QObject
{
    Q_OBJECT
public:
    explicit Backend(QObject *parent = nullptr);

    // Container operations
    QList<QMap<QString, QString>> getContainers() const;
    QString createContainer(const QString &name, const QString &image, const QString &home = "", bool init = false, const QStringList &volumes = QStringList());
    QString deleteContainer(const QString &name);
    void enterContainer(const QString &name, const QString &terminal);
    void upgradeContainer(const QString &name, const QString &terminal);
    void upgradeAllContainers(const QString &terminal);
    void executeInTerminal(const QString &terminal, const QString &command);
    // App operations
    QStringList getAvailableApps(const QString &containerName);
    QStringList getExportedApps(const QString &containerName);
    QString exportApp(const QString &appName, const QString &containerName);
    QString unexportApp(const QString &appName, const QString &containerName);
    QString getContainerDistro(const QString &containerName) const;

    // Image operations
    QList<QMap<QString, QString>> getAvailableImages();
    QList<QMap<QString, QString>> searchImages(const QString &query);

signals:
    void assembleFinished(const QString &result);

public slots:
    void assembleContainer(const QString &iniFile);
    void installDebPackage(const QString &terminal, const QString &containerName, const QString &filePath);
    void installRpmPackage(const QString &terminal, const QString &containerName, const QString &filePath);
    void installArchPackage(const QString &terminal, const QString &containerName, const QString &filePath);

private:
    QString runCommand(const QStringList &command) const;
    QString parseDistroFromImage(const QString &imageUrl) const;
    QString getDistroIcon(const QString &distroName) const;

    const QStringList DISTROS = {"alma",       "alpine", "amazon",     "arch",       "bazzite",   "blackarch",   "bluefin",  "bookworm",   "bullseye",
                                 "buster",     "centos", "chainguard", "clearlinux", "crystal",   "debian",      "deepin",   "fedora",     "gentoo",
                                 "kali",       "leap",   "linuxmint",  "mageia",     "neon",      "neurodebian", "opensuse", "oracle",     "plasma",
                                 "powershell", "redhat", "rhel",       "rocky",      "slackware", "steamos",     "toolbox",  "tumbleweed", "ubi",
                                 "ublue",      "ubuntu", "vanilla",    "void",       "wheezy",    "wolfi"};

    QMap<QString, QString> distroIconMap = {// Official distros
                                            {"alma", "almalinux.svg"},
                                            {"alpine", "alpine.svg"},
                                            //{"amazon", "amazon.svg"},
                                            {"arch", "archlinux.svg"},
                                            {"blackarch", "archlinux.svg"},
                                            {"centos", "centos.svg"},
                                            //{"clearlinux", "clearlinux.svg"},
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
                                            {"void", "void.svg"},
                                            {"wolfi", "wolfi.svg"},

                                            // Variants and derivatives
                                            {"bazzite", "fedora.svg"},
                                            {"bluefin", "ubuntu.svg"},
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
                                            {"ublue", "tux.svg"}, // Could use fedora.svg or create custom
                                            {"crystal", "crystal.svg"},

                                            // Default fallbacks
                                            {"unknown", "tux.svg"}};
};

#endif // BACKEND_H
