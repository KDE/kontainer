#include "mainwindow.h"

#include <QApplication>
#include <QStyleFactory> // Wichtig für QStyleFactory

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Stil setzen
    app.setStyle(QStyleFactory::create("Breeze"));

    MainWindow window;
    window.show();

    return app.exec();
}
