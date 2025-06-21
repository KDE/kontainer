#ifndef TOOLBOXIMAGES_H
#define TOOLBOXIMAGES_H

#include <QList>
#include <QMap>
#include <QString>

// Each map has keys: "distro", "version", "image"
static const QList<QMap<QString, QString>> toolboxImages = {
    {{"distro", "AlmaLinux"}, {"version", "8"}, {"image", "quay.io/toolbx-images/almalinux-toolbox:8"}},
    {{"distro", "AlmaLinux"}, {"version", "9"}, {"image", "quay.io/toolbx-images/almalinux-toolbox:9"}},
    {{"distro", "AlmaLinux"}, {"version", "latest"}, {"image", "quay.io/toolbx-images/almalinux-toolbox:latest"}},

    {{"distro", "Alpine"}, {"version", "3.16"}, {"image", "quay.io/toolbx-images/alpine-toolbox:3.16"}},
    {{"distro", "Alpine"}, {"version", "3.17"}, {"image", "quay.io/toolbx-images/alpine-toolbox:3.17"}},
    {{"distro", "Alpine"}, {"version", "3.18"}, {"image", "quay.io/toolbx-images/alpine-toolbox:3.18"}},
    {{"distro", "Alpine"}, {"version", "3.19"}, {"image", "quay.io/toolbx-images/alpine-toolbox:3.19"}},
    {{"distro", "Alpine"}, {"version", "3.20"}, {"image", "quay.io/toolbx-images/alpine-toolbox:3.20"}},
    {{"distro", "Alpine"}, {"version", "edge"}, {"image", "quay.io/toolbx-images/alpine-toolbox:edge"}},
    {{"distro", "Alpine"}, {"version", "latest"}, {"image", "quay.io/toolbx-images/alpine-toolbox:latest"}},

    {{"distro", "AmazonLinux"}, {"version", "2"}, {"image", "quay.io/toolbx-images/amazonlinux-toolbox:2"}},
    {{"distro", "AmazonLinux"}, {"version", "2023"}, {"image", "quay.io/toolbx-images/amazonlinux-toolbox:2023"}},
    {{"distro", "AmazonLinux"}, {"version", "latest"}, {"image", "quay.io/toolbx-images/amazonlinux-toolbox:latest"}},

    {{"distro", "Archlinux"}, {"version", "latest"}, {"image", "quay.io/toolbx/arch-toolbox:latest"}},

    {{"distro", "Bazzite Arch"}, {"version", "latest"}, {"image", "ghcr.io/ublue-os/bazzite-arch:latest"}},
    {{"distro", "Bazzite Arch GNOME"}, {"version", "latest"}, {"image", "ghcr.io/ublue-os/bazzite-arch-gnome:latest"}},

    {{"distro", "CentOS"}, {"version", "stream8"}, {"image", "quay.io/toolbx-images/centos-toolbox:stream8"}},
    {{"distro", "CentOS"}, {"version", "stream9"}, {"image", "quay.io/toolbx-images/centos-toolbox:stream9"}},
    {{"distro", "CentOS"}, {"version", "latest"}, {"image", "quay.io/toolbx-images/centos-toolbox:latest"}},

    {{"distro", "Debian"}, {"version", "10"}, {"image", "quay.io/toolbx-images/debian-toolbox:10"}},
    {{"distro", "Debian"}, {"version", "11"}, {"image", "quay.io/toolbx-images/debian-toolbox:11"}},
    {{"distro", "Debian"}, {"version", "12"}, {"image", "quay.io/toolbx-images/debian-toolbox:12"}},
    {{"distro", "Debian"}, {"version", "testing"}, {"image", "quay.io/toolbx-images/debian-toolbox:testing"}},
    {{"distro", "Debian"}, {"version", "unstable"}, {"image", "quay.io/toolbx-images/debian-toolbox:unstable"}},
    {{"distro", "Debian"}, {"version", "latest"}, {"image", "quay.io/toolbx-images/debian-toolbox:latest"}},

    {{"distro", "Fedora"}, {"version", "37"}, {"image", "registry.fedoraproject.org/fedora-toolbox:37"}},
    {{"distro", "Fedora"}, {"version", "38"}, {"image", "registry.fedoraproject.org/fedora-toolbox:38"}},
    {{"distro", "Fedora"}, {"version", "39"}, {"image", "registry.fedoraproject.org/fedora-toolbox:39"}},
    {{"distro", "Fedora"}, {"version", "40"}, {"image", "registry.fedoraproject.org/fedora-toolbox:40"}},
    {{"distro", "Fedora"}, {"version", "41"}, {"image", "quay.io/fedora/fedora-toolbox:41"}},
    {{"distro", "Fedora"}, {"version", "rawhide"}, {"image", "quay.io/fedora/fedora-toolbox:rawhide"}},

    {{"distro", "openSUSE"}, {"version", "latest"}, {"image", "registry.opensuse.org/opensuse/distrobox:latest"}},

    {{"distro", "RedHat"}, {"version", "8"}, {"image", "registry.access.redhat.com/ubi8/toolbox"}},
    {{"distro", "RedHat"}, {"version", "9"}, {"image", "registry.access.redhat.com/ubi9/toolbox"}},

    {{"distro", "Rocky Linux"}, {"version", "8"}, {"image", "quay.io/toolbx-images/rockylinux-toolbox:8"}},
    {{"distro", "Rocky Linux"}, {"version", "9"}, {"image", "quay.io/toolbx-images/rockylinux-toolbox:9"}},
    {{"distro", "Rocky Linux"}, {"version", "latest"}, {"image", "quay.io/toolbx-images/rockylinux-toolbox:latest"}},

    {{"distro", "Ubuntu"}, {"version", "16.04"}, {"image", "quay.io/toolbx/ubuntu-toolbox:16.04"}},
    {{"distro", "Ubuntu"}, {"version", "18.04"}, {"image", "quay.io/toolbx/ubuntu-toolbox:18.04"}},
    {{"distro", "Ubuntu"}, {"version", "20.04"}, {"image", "quay.io/toolbx/ubuntu-toolbox:20.04"}},
    {{"distro", "Ubuntu"}, {"version", "22.04"}, {"image", "quay.io/toolbx/ubuntu-toolbox:22.04"}},
    {{"distro", "Ubuntu"}, {"version", "24.04"}, {"image", "quay.io/toolbx/ubuntu-toolbox:24.04"}},
    {{"distro", "Ubuntu"}, {"version", "latest"}, {"image", "quay.io/toolbx/ubuntu-toolbox:latest"}},

    {{"distro", "Chainguard Wolfi"}, {"version", "latest"}, {"image", "quay.io/toolbx-images/wolfi-toolbox:latest"}}};

#endif // TOOLBOXIMAGES_H
