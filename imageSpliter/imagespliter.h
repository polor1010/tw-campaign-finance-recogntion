#ifndef IMAGESPLITER_H
#define IMAGESPLITER_H

#include "../imageprocess.h"
#include <QString>
#include <QImage>
#include <QDebug>
#include <QDir>

class ImageSpliter : public ImageProcess
{

public:
    ImageSpliter();
    bool getTables(QString filePath);
    int OtsuThresholdRAW(unsigned char *Buffer, int Width , int Height);

    void getRegions(unsigned char* buffer8 , int width , int height  , vector<REGION_ENTRY> &regions , int &end );
    void saveImage( unsigned char* buffer8,int width, int height , int index);
    QString fileName;
};

#endif // IMAGESPLITER_H
