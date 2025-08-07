// SPDX-License-Identifier: GPL-2.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2025 Hadi Chokr <hadichokr@icloud.com>

#include "mainwindow.h"
#include <main.h>
#include "appflags.h"

bool g_noTerminal = false;

int main(int argc, char *argv[])
{
    for (int i = 1; i < argc; ++i) {
        if (QString(argv[i]) == "--no-terminal") {
            g_noTerminal = true;
        }
    }

    KLocalizedString::setApplicationDomain("kontainer");
    QApplication app(argc, argv);
    app.setStyle(QStyleFactory::create("Breeze"));

    MainWindow window;
    window.show();

    return app.exec();
}
