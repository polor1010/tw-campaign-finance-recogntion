#ifndef IMAGEPROCESS_H
#define IMAGEPROCESS_H

#include <vector>

#define SAFE_RELEASE(x)  if(x) {delete []x; x=NULL;}

#define PATTERN_WIDTH 32
#define PATTERN_HEIGHT 64
#define DISTANCE_MAX 160
#define DISTANCE_MIN 30
#define MIN_CAHR_SIZE 80
#define TABLE_ROW 21
#define TABLE_COLUMN 9

using namespace std;

enum REGION_DIRECTION
{
    V_DIR,
    H_DIR
};

typedef struct{
    int x;
    int y;
}Point;

typedef struct{
    int		size;
    int		cx, cy;
    int		label;
    int		left, right, top, bottom;
    bool	IsRegion;
    char    result;
}REGION_ENTRY;

typedef struct{
    int label;
    float a;
    float b;
    int fitCounter;
    vector<Point>centerPoints;
}TABLE_LINE;

enum CORNER_TYPE
{
    LEFT_TOP,
    RIGHT_BOTTOM
};

class ImageProcess
{

public:
    vector<REGION_ENTRY> connectedComponent(int *buffer8  , int width, int height, int minSize = 10);
    void secondPass(int *bufferINT , int *buffer8, int width , int height , int labelCounter , int *parent ,  vector<REGION_ENTRY> &regionList , int minSize);
    void connectedUnion(int x, int y, int *p);

    void chamferDistance(unsigned char *buffer8, int width, int height, unsigned char *resultBuffer8, int threshold, int maxValue, int minValue);
    void getMaxRegion(unsigned char *buffer8 , int width , int height );
    vector<REGION_ENTRY> getCharPosition(unsigned char *buffer8 , int width , int height , int *horizontal  , int &top , int &bottom  );
    void noiseRemove(unsigned char *buffer8 , int width , int height );
    void horizontalEdge(unsigned char *buffer8,int width , int height );
    void trans2RGB(unsigned char *buffer8 , int width , int height, int widthEff , unsigned char *buffer32 );
    void trans2Gray(unsigned char *buffer32 , int width , int height, int widthEff , unsigned char *buffer8 );
    float vertivalLineFilter(unsigned char *buffer8 , int width , int height , vector<TABLE_LINE> &lines );
    void verticalClear(unsigned char *buffer8 , int width , int x, int y , int length );

    float getRegionLineFit(  unsigned char *edgeMap,unsigned char *centerMap , int width , int height , REGION_DIRECTION direction , vector<TABLE_LINE> &lines);
    float horizontalLineFilter(unsigned char *buffer8 , int width , int height , vector<TABLE_LINE> &lines);
    void horizontalClear(unsigned char *buffer8 , int width , int x, int y , int length );
    void lineFit(vector<Point> pints , float &a, float &b , REGION_DIRECTION direction);

    void getCornerPoints(vector<REGION_ENTRY> &regions,vector<Point> &points , CORNER_TYPE type );
    void sortCorner(vector<Point> &points);
    bool isReverse(unsigned char *buffer8 , int width , int height , int top, int bottom);
};

#endif // IMAGEPROCESS_H
