// src/packagemanager.h
#ifndef PACKAGEMANAGER_H
#define PACKAGEMANAGER_H

#include <QString>

class PackageManager {
public:
    static QString getDistroFromImage(const QString &image) {
        QString lowerImage = image.toLower();
        if (lowerImage.contains("debian") || lowerImage.contains("ubuntu") || lowerImage.contains("mint")) {
            return "deb";
        } else if (lowerImage.contains("fedora") || lowerImage.contains("rhel") || lowerImage.contains("centos")) {
            return "rpm";
        } else if (lowerImage.contains("opensuse") || lowerImage.contains("suse")) {
            return "rpm";
        } else if (lowerImage.contains("arch") || lowerImage.contains("manjaro") || lowerImage.contains("endeavouros")) {
            return "arch";
        }
        return "";
    }
};

#endif // PACKAGEMANAGER_H
