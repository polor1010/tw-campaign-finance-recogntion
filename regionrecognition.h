#ifndef REGIONRECOGNITION_H
#define REGIONRECOGNITION_H

#include <QString>
#include <QImage>
#include "imageprocess.h"

//#define DEBUG_SAVE_IMAGE

using namespace std;

typedef struct{
    char word;
    unsigned char *buffer8;
}OCR_PATTERN;

typedef struct{
    int top;
    int left;
    int bottom;
    int right;
    int row;
    int column;
    QString result;
}TABLE_REGION;

class RegionRecognition : public ImageProcess
{

public:
    RegionRecognition();

    bool readJsonFile(QString filePath);
    void writeJsonFile(QString filePath);
    bool getRegionRecognitions(QString filePath);

    QString processImageChinese(QImage &image);
    QString processImage(QImage image);
    void drawInfo(QImage &image);

private:
    QString sortCharPosition();
    char recognition(unsigned char *buffer8);
    void setPattern(unsigned char *buffer32, int width, int height, int widthEff, char word);
    void getRecognitions(unsigned char *buffer8, int width);
    bool loadPatterns();
    void saveWord();
    void trans2Image(unsigned char *buffer8,int width,int height);

private:
    vector<OCR_PATTERN> patterns;
    vector<REGION_ENTRY> charPositions;
    vector<TABLE_REGION> tableRegions;
    QImage image;
    bool isLoadDataBase;
    bool isInverse;

};



#endif // REGIONRECOGNITION_H
