#ifndef IMAGEPROCESS_H
#define IMAGEPROCESS_H

#include <vector>

#define SAFE_RELEASE(x)  if(x) {delete []x; x=NULL;}

#define PATTERN_WIDTH 32
#define PATTERN_HEIGHT 64
#define DISTANCE_MAX 160
#define DISTANCE_MIN 30
#define MIN_CAHR_SIZE 80

using namespace std;

typedef struct{
    int		size;
    int		cx, cy;
    int		label;
    int		left, right, top, bottom;
    bool	IsRegion;
    char    result;
}REGION_ENTRY;

class ImageProcess
{

public:
    vector<REGION_ENTRY> connectedComponent(int *buffer8, int width, int height, int minSize = 10);
    void secondPass(int *bufferINT , int width , int height , int labelCounter , int *parent ,  vector<REGION_ENTRY> &regionList , int minSize);
    void connectedUnion(int x, int y, int *p);

    void chamferDistance(unsigned char *buffer8, int width, int height, unsigned char *resultBuffer8, int threshold, int maxValue, int minValue);
    void getMaxRegion(unsigned char *buffer8 , int width , int height );
    vector<REGION_ENTRY> getCharPosition(unsigned char *buffer8 , int width , int height );
    void noiseRemove(unsigned char *buffer8 , int width , int height );
    void horizontalEdge(unsigned char *buffer8,int width , int height );
    void trans2RGB(unsigned char *buffer8 , int width , int height, int widthEff , unsigned char *buffer32 );
    void trans2Gray(unsigned char *buffer32 , int width , int height, int widthEff , unsigned char *buffer8 );

};

#endif // IMAGEPROCESS_H
