// SPDX-License-Identifier: GPL-2.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2025 Hadi Chokr <hadichokr@icloud.com>

#pragma once
#include <QMap>
#include <QString>
#include <QStringList>

struct TerminalInfo {
    QString icon;
    QStringList commandFormat;
};

inline QMap<QString, TerminalInfo> getTerminalInfoMap()
{
    return {// Regular terminals
            {"gnome-terminal", {"gnome-terminal", {"--", "$command"}}},
            {"xfce4-terminal", {"xfce4-terminal", {"--command=$command"}}},
            {"xterm", {"xterm", {"-e", "$command"}}},
            {"konsole", {"utilities-terminal", {"-e", "$command"}}},
            {"kgx", {"kgx", {"-e", "$command"}}},
            {"tilix", {"tilix", {"-e", "$command"}}},
            {"alacritty", {"alacritty", {"-e", "$command"}}},
            {"kitty", {"kitty", {"-e", "$command"}}},
            {"terminator", {"terminator", {"-e", "$command"}}},
            {"urxvt", {"urxvt", {"-e", "$command"}}},
            {"lxterminal", {"lxterminal", {"-e", "$command"}}},
            {"eterm", {"eterm", {"-e", "$command"}}},
            {"st", {"st", {"-e", "$command"}}},
            {"wezterm", {"wezterm", {"-e", "$command"}}},

            // Flatpak terminals
            {"org.contourterminal.Contour", {"contour", {"run", "$terminal", "--", "/bin/bash", "-c", "$command"}}},
            {"org.wezfurlong.wezterm", {"wezterm", {"run", "$terminal", "-e", "/bin/bash", "-c", "$command"}}},
            {"org.kde.konsole", {"utilities-terminal", {"run", "$terminal", "-e", "/bin/bash", "-c", "$command"}}}};
}

inline bool isFlatpakTerminal(const QString &terminal)
{
    return terminal.startsWith("org.") || terminal.startsWith("com.") || terminal.startsWith("app.");
}
