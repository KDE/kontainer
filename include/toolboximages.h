// SPDX-License-Identifier: GPL-2.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2025 Hadi Chokr <hadichokr@icloud.com>

#ifndef TOOLBOXIMAGES_H
#define TOOLBOXIMAGES_H

#include <QList>
#include <QMap>
#include <QString>

// Each map has keys: "distro", "version", "image"
static const QList<QMap<QString, QString>> toolboxImages = {
    {{"distro", "alma"}, {"version", "8"}, {"image", "quay.io/toolbx-images/almalinux-toolbox:8"}},
    {{"distro", "alma"}, {"version", "9"}, {"image", "quay.io/toolbx-images/almalinux-toolbox:9"}},
    {{"distro", "alma"}, {"version", "latest"}, {"image", "quay.io/toolbx-images/almalinux-toolbox:latest"}},

    {{"distro", "alpine"}, {"version", "3.16"}, {"image", "quay.io/toolbx-images/alpine-toolbox:3.16"}},
    {{"distro", "alpine"}, {"version", "3.17"}, {"image", "quay.io/toolbx-images/alpine-toolbox:3.17"}},
    {{"distro", "alpine"}, {"version", "3.18"}, {"image", "quay.io/toolbx-images/alpine-toolbox:3.18"}},
    {{"distro", "alpine"}, {"version", "3.19"}, {"image", "quay.io/toolbx-images/alpine-toolbox:3.19"}},
    {{"distro", "alpine"}, {"version", "3.20"}, {"image", "quay.io/toolbx-images/alpine-toolbox:3.20"}},
    {{"distro", "alpine"}, {"version", "edge"}, {"image", "quay.io/toolbx-images/alpine-toolbox:edge"}},
    {{"distro", "alpine"}, {"version", "latest"}, {"image", "quay.io/toolbx-images/alpine-toolbox:latest"}},

    {{"distro", "amazon"}, {"version", "2"}, {"image", "quay.io/toolbx-images/amazonlinux-toolbox:2"}},
    {{"distro", "amazon"}, {"version", "2023"}, {"image", "quay.io/toolbx-images/amazonlinux-toolbox:2023"}},
    {{"distro", "amazon"}, {"version", "latest"}, {"image", "quay.io/toolbx-images/amazonlinux-toolbox:latest"}},

    {{"distro", "arch"}, {"version", "latest"}, {"image", "quay.io/toolbx/arch-toolbox:latest"}},

    {{"distro", "bazzite"}, {"version", "latest"}, {"image", "ghcr.io/ublue-os/bazzite-arch:latest"}},
    {{"distro", "bazzite"}, {"version", "latest"}, {"image", "ghcr.io/ublue-os/bazzite-arch-gnome:latest"}},

    {{"distro", "centos"}, {"version", "stream8"}, {"image", "quay.io/toolbx-images/centos-toolbox:stream8"}},
    {{"distro", "centos"}, {"version", "stream9"}, {"image", "quay.io/toolbx-images/centos-toolbox:stream9"}},
    {{"distro", "centos"}, {"version", "latest"}, {"image", "quay.io/toolbx-images/centos-toolbox:latest"}},

    {{"distro", "debian"}, {"version", "10"}, {"image", "quay.io/toolbx-images/debian-toolbox:10"}},
    {{"distro", "debian"}, {"version", "11"}, {"image", "quay.io/toolbx-images/debian-toolbox:11"}},
    {{"distro", "debian"}, {"version", "12"}, {"image", "quay.io/toolbx-images/debian-toolbox:12"}},
    {{"distro", "debian"}, {"version", "testing"}, {"image", "quay.io/toolbx-images/debian-toolbox:testing"}},
    {{"distro", "debian"}, {"version", "unstable"}, {"image", "quay.io/toolbx-images/debian-toolbox:unstable"}},
    {{"distro", "debian"}, {"version", "latest"}, {"image", "quay.io/toolbx-images/debian-toolbox:latest"}},

    {{"distro", "fedora"}, {"version", "37"}, {"image", "registry.fedoraproject.org/fedora-toolbox:37"}},
    {{"distro", "fedora"}, {"version", "38"}, {"image", "registry.fedoraproject.org/fedora-toolbox:38"}},
    {{"distro", "fedora"}, {"version", "39"}, {"image", "registry.fedoraproject.org/fedora-toolbox:39"}},
    {{"distro", "fedora"}, {"version", "40"}, {"image", "registry.fedoraproject.org/fedora-toolbox:40"}},
    {{"distro", "fedora"}, {"version", "41"}, {"image", "quay.io/fedora/fedora-toolbox:41"}},
    {{"distro", "fedora"}, {"version", "latest"}, {"image", "registry.fedoraproject.org/fedora-toolbox:latest"}},
    {{"distro", "fedora"}, {"version", "rawhide"}, {"image", "quay.io/fedora/fedora-toolbox:rawhide"}},

    {{"distro", "opensuse"}, {"version", "latest"}, {"image", "registry.opensuse.org/opensuse/distrobox:latest"}},

    {{"distro", "redhat"}, {"version", "8"}, {"image", "registry.access.redhat.com/ubi8/toolbox"}},
    {{"distro", "redhat"}, {"version", "9"}, {"image", "registry.access.redhat.com/ubi9/toolbox"}},

    {{"distro", "rocky"}, {"version", "8"}, {"image", "quay.io/toolbx-images/rockylinux-toolbox:8"}},
    {{"distro", "rocky"}, {"version", "9"}, {"image", "quay.io/toolbx-images/rockylinux-toolbox:9"}},
    {{"distro", "rocky"}, {"version", "latest"}, {"image", "quay.io/toolbx-images/rockylinux-toolbox:latest"}},

    {{"distro", "Ubuntu"}, {"version", "16.04"}, {"image", "quay.io/toolbx/ubuntu-toolbox:16.04"}},
    {{"distro", "Ubuntu"}, {"version", "18.04"}, {"image", "quay.io/toolbx/ubuntu-toolbox:18.04"}},
    {{"distro", "Ubuntu"}, {"version", "20.04"}, {"image", "quay.io/toolbx/ubuntu-toolbox:20.04"}},
    {{"distro", "Ubuntu"}, {"version", "22.04"}, {"image", "quay.io/toolbx/ubuntu-toolbox:22.04"}},
    {{"distro", "Ubuntu"}, {"version", "24.04"}, {"image", "quay.io/toolbx/ubuntu-toolbox:24.04"}},
    {{"distro", "Ubuntu"}, {"version", "latest"}, {"image", "quay.io/toolbx/ubuntu-toolbox:latest"}},

    {{"distro", "wolfi"}, {"version", "latest"}, {"image", "quay.io/toolbx-images/wolfi-toolbox:latest"}}};

#endif // TOOLBOXIMAGES_H
