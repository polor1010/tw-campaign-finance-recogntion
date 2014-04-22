#ifndef IMAGEPROCESS_H
#define IMAGEPROCESS_H

#include <vector>
#include <QtGlobal>

//#define uchar unsigned char
using namespace std;



typedef struct{
    int		size;
    int		cx, cy;
    int		label;
    int		left, right, top, bottom;
    bool	IsRegion;
}REGION_ENTRY;


vector<REGION_ENTRY> ConnectedComponent(int *buffer8, int width, int height, int minSize = 10);
void SecondPass(int *bufferINT , int width , int height , int labelCounter , int *parent ,  vector<REGION_ENTRY> &regionList , int minSize );
void Union(int x, int y, int *p);

void getMaxRegion(uchar *buffer8 , int width , int height );
vector<REGION_ENTRY> getCharPosition( uchar *buffer8 , int width , int height );
void noiseRemove(uchar *buffer8 , int width , int height );
void trans2RGB(uchar *buffer8 , int width , int height, int widthEff , uchar *buffer32 );
void trans2Gray(uchar *buffer32 , int width , int height, int widthEff , uchar *buffer8 );

#endif // IMAGEPROCESS_H
