#include "mainwindow.h"
#include <QApplication>

//#define GUI_MODE
//#define UNIT_TEST

int main(int argc, char *argv[])
{

#ifdef UNIT_TEST

#endif

#ifdef GUI_MODE
    QApplication a(argc, argv);

    MainWindow w;
    w.show();

    return a.exec();
#else

    RegionRecognition regionRecognition;
    if( regionRecognition.readJsonFile(argv[2]) )
    {
        if( regionRecognition.getRegionRecognitions(argv[1]) )
        {
            regionRecognition.writeJsonFile(argv[3]);
            printf("recognition finish : %s\n",argv[1]);
        }
        else
        {
            printf("recognition error : %s\n",argv[1]);
        }
    }


    return 0;
#endif

}
