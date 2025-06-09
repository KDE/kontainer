#include "mainwindow.h"
#include <main.h>


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setStyle(QStyleFactory::create("Breeze"));

    MainWindow window;
    window.show();

    return app.exec();
}
