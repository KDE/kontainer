#ifndef BACKEND_H
#define BACKEND_H

#include <QObject>
#include <QStringList>
#include <QMap>
#include <QStandardPaths>	

class Backend : public QObject
{
    Q_OBJECT
public:
    explicit Backend(QObject *parent = nullptr);

    // Container operations
    QList<QMap<QString, QString>> getContainers();
    QString createContainer(const QString &name, const QString &image, 
                          const QString &home = "", bool init = false,
                          const QStringList &volumes = QStringList());
    QString deleteContainer(const QString &name);
    void enterContainer(const QString &name);
    void upgradeContainer(const QString &name);
    void upgradeAllContainers();

    // App operations
    QStringList getAvailableApps(const QString &containerName);
    QStringList getExportedApps(const QString &containerName);
    QString exportApp(const QString &appName, const QString &containerName);
    QString unexportApp(const QString &appName, const QString &containerName);

    // Image operations
    QList<QMap<QString, QString>> getAvailableImages();
    QList<QMap<QString, QString>> searchImages(const QString &query);

private:
    QString runCommand(const QStringList &command);
    QString parseDistroFromImage(const QString &imageUrl);

    const QStringList DISTROS = {
        "alma", "alpine", "amazon", "bazzite", "arch", "centos", "clearlinux", 
        "crystal", "debian", "deepin", "fedora", "gentoo", "kali", "mageia", 
        "mint", "neon", "opensuse", "oracle", "redhat", "rhel", "rocky", 
        "slackware", "steamos", "ubuntu", "ublue", "vanilla", "void"
    };
};

#endif // BACKEND_H
