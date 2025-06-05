#pragma once
#include <QMap>
#include <QString>

inline QMap<QString, QString> getTerminalIconMap() {
    return {
        {"gnome-terminal", "gnome-terminal"},
        {"konsole", "utilities-terminal"},
        {"xfce4-terminal", "xfce4-terminal"},
        {"xterm", "xterm"}
    };
}
