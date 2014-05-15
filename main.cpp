#include "mainwindow.h"
#include <QApplication>

//#define GUI_MODE
#define UNIT_TEST

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
    if( argc < 2 )
    {
         printf("input error..\n");
         return 0;
    }
    QString imagePath = argv[1];

    RegionRecognition regionRecognition;

    int i;
    for( i = 2; i < argc ; i++ )
    {
        if( argv[i] == "-m" )
        {

        }
        if( argv[i] == "-d" )
        {
            regionRecognition.setDenoise(true);
        }
        if( argv[i] == "-s" )
        {
            regionRecognition.setDeskew(false);
        }
    }

    if( regionRecognition.getRegions(imagePath) )
    {
        if( regionRecognition.getRegionRecognitions(imagePath) )
        {
            printf("recognition finish : %s\n",imagePath.toLocal8Bit().data());

            int fileNameLength = imagePath.length();
            QString outJsonPath = imagePath.replace(fileNameLength-4,4,".json");
            regionRecognition.writeJsonFile(outJsonPath);

            printf("write json file : %s\n",outJsonPath.toLocal8Bit().data());
        }
    }
    else
    {
        printf("recognition error : %s\n",imagePath.toLocal8Bit().data());
    }

    return 0;
#endif

}
