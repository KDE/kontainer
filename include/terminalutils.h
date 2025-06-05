#pragma once
#include <QMap>
#include <QString>

inline QMap<QString, QString> getTerminalIconMap() {
    return {
    {"gnome-terminal", "gnome-terminal"},
    {"xfce4-terminal", "xfce4-terminal"},
    {"xterm", "xterm"},
    {"konsole", "utilities-terminal"},
    {"org.kde.konsole", "utilities-terminal"}, // Flatpak version
    {"kgx", "kgx"}, // GNOME Console
    {"tilix", "tilix"},
    {"alacritty", "alacritty"},
    {"kitty", "kitty"},
    {"terminator", "terminator"},
    {"urxvt", "urxvt"},
    {"lxterminal", "lxterminal"},
    {"eterm", "eterm"},
    {"st", "st"},
    {"wezterm", "wezterm"},
    {"konsole.wrapper", "utilities-terminal"}
    };
}
