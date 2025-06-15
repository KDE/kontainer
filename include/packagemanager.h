// SPDX-License-Identifier: GPL-2.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2025 Hadi Chokr <hadichokr@icloud.com>

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
        static const QMap<QString, QStringList> distroMappings = {
            {"deb", {"debian", "ubuntu", "mint", "vso", "popos", "kali", "neon", "nd\\d+"}},
            {"rpm", {"fedora", "rhel", "centos", "opensuse", "suse", "rockylinux", "ubi\\d*", "almalinux", "mageia"}},
            {"arch", {"arch", "blackarch", "archlinux", "arch-toolbox", "arch-distrobox", "crystal"}}};

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
