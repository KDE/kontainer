// include/packagemanager.h
#ifndef PACKAGEMANAGER_H
#define PACKAGEMANAGER_H

#include <QString>
#include <QStringList>

class PackageManager
{
public:
    static QString getDistroFromImage(const QString &image)
    {
        QString lowerImage = image.toLower();

        // Define package type mappings
        static const QMap<QString, QStringList> distroMappings = {{"deb", {"debian", "ubuntu", "mint", "vso", "popos", "kali"}},
                                                                  {"rpm", {"fedora", "rhel", "centos", "opensuse", "suse", "rockylinux"}},
                                                                  {"arch", {"arch", "manjaro", "endeavouros", "artix"}}};

        // Check each package type
        for (auto it = distroMappings.constBegin(); it != distroMappings.constEnd(); ++it) {
            const QString &pkgType = it.key();
            const QStringList &distros = it.value();

            for (const QString &distro : distros) {
                if (lowerImage.contains(distro)) {
                    return pkgType;
                }
            }
        }

        return "";
    }
};

#endif // PACKAGEMANAGER_H
