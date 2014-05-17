#include "mainwindow.h"
#include <QApplication>
#include <imagespliter.h>

int main(int argc, char *argv[])
{
    /*
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
    */

    QString imagePath = argv[1];

    ImageSpliter imagespliter;
    imagespliter.getTables(imagePath);

}
