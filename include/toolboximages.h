// SPDX-License-Identifier: GPL-2.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2025 Hadi Chokr <hadichokr@icloud.com>

#pragma once

#include <QList>
#include <QString>

struct ToolboxImage {
    QString distro;
    QString version;
    QString image;

    // Constructor for easy initialization
    ToolboxImage(const QString &d, const QString &v, const QString &i)
        : distro(d)
        , version(v)
        , image(i)
    {
    }
};

static const QList<ToolboxImage> toolboxImages = {{"alma", "8", "quay.io/toolbx-images/almalinux-toolbox:8"},
                                                  {"alma", "9", "quay.io/toolbx-images/almalinux-toolbox:9"},
                                                  {"alma", "10", "quay.io/toolbx-images/almalinux-toolbox:10"},
                                                  {"alma", "latest", "quay.io/toolbx-images/almalinux-toolbox:latest"},

                                                  {"alpine", "3.16", "quay.io/toolbx-images/alpine-toolbox:3.16"},
                                                  {"alpine", "3.17", "quay.io/toolbx-images/alpine-toolbox:3.17"},
                                                  {"alpine", "3.18", "quay.io/toolbx-images/alpine-toolbox:3.18"},
                                                  {"alpine", "3.19", "quay.io/toolbx-images/alpine-toolbox:3.19"},
                                                  {"alpine", "3.20", "quay.io/toolbx-images/alpine-toolbox:3.20"},
                                                  {"alpine", "edge", "quay.io/toolbx-images/alpine-toolbox:edge"},
                                                  {"alpine", "latest", "quay.io/toolbx-images/alpine-toolbox:latest"},

                                                  {"amazon", "2", "quay.io/toolbx-images/amazonlinux-toolbox:2"},
                                                  {"amazon", "2023", "quay.io/toolbx-images/amazonlinux-toolbox:2023"},
                                                  {"amazon", "latest", "quay.io/toolbx-images/amazonlinux-toolbox:latest"},

                                                  {"arch", "latest", "quay.io/toolbx/arch-toolbox:latest"},

                                                  {"bazzite", "latest-arch", "ghcr.io/ublue-os/bazzite-arch:latest"},
                                                  {"bazzite", "latest-arch-gnome", "ghcr.io/ublue-os/bazzite-arch-gnome:latest"},

                                                  {"centos", "stream8", "quay.io/toolbx-images/centos-toolbox:stream8"},
                                                  {"centos", "stream9", "quay.io/toolbx-images/centos-toolbox:stream9"},
                                                  {"centos", "stream10", "quay.io/toolbx-images/centos-toolbox:stream10"},
                                                  {"centos", "latest", "quay.io/toolbx-images/centos-toolbox:latest"},

                                                  {"debian", "10", "quay.io/toolbx-images/debian-toolbox:10"},
                                                  {"debian", "11", "quay.io/toolbx-images/debian-toolbox:11"},
                                                  {"debian", "12", "quay.io/toolbx-images/debian-toolbox:12"},
                                                  {"debian", "testing", "quay.io/toolbx-images/debian-toolbox:testing"},
                                                  {"debian", "unstable", "quay.io/toolbx-images/debian-toolbox:unstable"},
                                                  {"debian", "latest", "quay.io/toolbx-images/debian-toolbox:latest"},

                                                  {"fedora", "37", "registry.fedoraproject.org/fedora-toolbox:37"},
                                                  {"fedora", "38", "registry.fedoraproject.org/fedora-toolbox:38"},
                                                  {"fedora", "39", "registry.fedoraproject.org/fedora-toolbox:39"},
                                                  {"fedora", "40", "registry.fedoraproject.org/fedora-toolbox:40"},
                                                  {"fedora", "41", "quay.io/fedora/fedora-toolbox:41"},
                                                  {"fedora", "42", "quay.io/fedora/fedora-toolbox:42"},
                                                  {"fedora", "latest", "registry.fedoraproject.org/fedora-toolbox:latest"},
                                                  {"fedora", "rawhide", "quay.io/fedora/fedora-toolbox:rawhide"},

                                                  {"opensuse", "latest", "registry.opensuse.org/opensuse/distrobox:latest"},

                                                  {"redhat", "8", "registry.access.redhat.com/ubi8/toolbox"},
                                                  {"redhat", "9", "registry.access.redhat.com/ubi9/toolbox"},
                                                  {"redhat", "10", "registry.access.redhat.com/ubi10/toolbox"},

                                                  {"rocky", "8", "quay.io/toolbx-images/rockylinux-toolbox:8"},
                                                  {"rocky", "9", "quay.io/toolbx-images/rockylinux-toolbox:9"},
                                                  {"rocky", "latest", "quay.io/toolbx-images/rockylinux-toolbox:latest"},

                                                  {"ubuntu", "16.04", "quay.io/toolbx/ubuntu-toolbox:16.04"},
                                                  {"ubuntu", "18.04", "quay.io/toolbx/ubuntu-toolbox:18.04"},
                                                  {"ubuntu", "20.04", "quay.io/toolbx/ubuntu-toolbox:20.04"},
                                                  {"ubuntu", "22.04", "quay.io/toolbx/ubuntu-toolbox:22.04"},
                                                  {"ubuntu", "24.04", "quay.io/toolbx/ubuntu-toolbox:24.04"},
                                                  {"ubuntu", "latest", "quay.io/toolbx/ubuntu-toolbox:latest"},

                                                  {"wolfi", "latest", "quay.io/toolbx-images/wolfi-toolbox:latest"}};
