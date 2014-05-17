#include "imageprocess.h"
#include <math.h>
#include <QDebug>

#define LINE_WIDTH 10

bool regionCompareSize(const REGION_ENTRY &left, const REGION_ENTRY &right)
{
    return left.size > right.size;
}


bool tableLineCompareFitCounter(const TABLE_LINE &left, const TABLE_LINE &right)
{
    return left.fitCounter > right.fitCounter;
}

bool compareY(const Point &left, const Point &right)
{
    return left.y < right.y;
}

bool compareX(const Point &left, const Point &right)
{
    return left.x < right.x;
}


/**
     * 將 buffer 做 chamfer distance transform (white foreground black background)
     *
     * @param unsigned char *buffer8
     * @param int   width
     * @param int   height
     * @param unsigned char *resultBuffer8
     * @param int threshold
     * @param int maxValue
     * @param int minValue
     * @return void
     */
void ImageProcess::chamferDistance(unsigned char *buffer8, int width, int height, unsigned char *resultBuffer8, int threshold, int maxValue, int minValue)
{
    unsigned char *pResultBuffer8;
    int i, j,
        value,
        imageSize = height * width;

    memset(resultBuffer8, 0, sizeof(unsigned char) * imageSize);

    for( i = 0 ; i < imageSize ; i++ )
    {
        if( buffer8[i] <= threshold )
            resultBuffer8[i] = 255;
    }

    pResultBuffer8 = resultBuffer8 + width;
    for( i = 1 ; i < height ; i++, pResultBuffer8 += width )
    {
        for( j = 1 ; j < width - 1 ; j++ )
        {
            value = 255;

            if(value > pResultBuffer8[j])							value = pResultBuffer8[j];
            if(value > pResultBuffer8[j - 1] + minValue)				value = pResultBuffer8[j - 1] + minValue;
            if(value > pResultBuffer8[-width + j - 1] + maxValue)		value = pResultBuffer8[-width + j - 1] + maxValue;
            if(value > pResultBuffer8[-width + j] + minValue)			value = pResultBuffer8[-width + j] + minValue;
            if(value > pResultBuffer8[-width + j + 1] + maxValue)		value = pResultBuffer8[-width + j + 1] + maxValue;

            pResultBuffer8[j] = value;
        }
    }

    pResultBuffer8 = resultBuffer8 + ((height - 2) * width);
    for( i = height-1 ; i > 0 ; i--, pResultBuffer8 -= width )
    {
        for( j = width - 2 ; j > 0 ; j-- )
        {
            value = 255;

            if(value > pResultBuffer8[j])							value = pResultBuffer8[j];
            if(value > pResultBuffer8[j + 1] + minValue)				value = pResultBuffer8[j + 1] + minValue;
            if(value > pResultBuffer8[width + j + 1] + maxValue)		value = pResultBuffer8[width + j + 1] + maxValue;
            if(value > pResultBuffer8[width + j] + minValue)			value = pResultBuffer8[width + j] + minValue;
            if(value > pResultBuffer8[width + j - 1] + maxValue)		value = pResultBuffer8[width + j - 1] + maxValue;

            pResultBuffer8[j] = value;
        }
    }

    pResultBuffer8 = resultBuffer8;
    for( i = 0 ; i < height ; i++ , pResultBuffer8 += width )
        pResultBuffer8[0] = pResultBuffer8[width - 1] = 255;

    pResultBuffer8 = resultBuffer8 + (height - 1) * width;
    for( i = 0 ; i < width ; i++ )
        pResultBuffer8[i] = resultBuffer8[i] = 255;

}

/**
     * 將 buffer 中座標 (x,y) ~ (x-length,y) 的值設為 0
     *
     * @param unsigned char *buffer8
     * @param int   width
     * @param int x
     * @param int y
     * @param int length
     * @return void
     */
void ImageProcess::horizontalClear(unsigned char *buffer8 , int width ,int x, int y , int length )
{
    int i;

    unsigned char *pBuffer8 = buffer8 + y * width + x;

    for( i = 0 ; i <= length ; i++ )
        pBuffer8[i*width] = 0;

}

/**
     * 將 buffer 中座標 (x,y) ~ (x,y-length) 的值設為 0
     *
     * @param unsigned char *buffer8
     * @param int   width
     * @param int x
     * @param int y
     * @param int length
     * @return void
     */
void ImageProcess::verticalClear(unsigned char *buffer8 , int width ,int x, int y , int length )
{
    int i;

    //qDebug() << x << y;
    unsigned char *pBuffer8 = buffer8 + y * width + x;

    for( i = 0 ; i < length ; i++ )
        pBuffer8[i] = 0;
}

/**
     * 將偵測垂直線並回傳其傾斜角度
     *
     * @param unsigned char *edgeMap
     * @param unsigned char *centerMap
     * @param int   width
     * @param int   height
     * @param REGION_DIRECTION direction
     * @param vector<TABLE_LINE> &lines
     * @return float 影像傾斜(direction)角度
     */
float ImageProcess::getRegionLineFit( unsigned char *edgeMap,unsigned char *centerMap , int width , int height , REGION_DIRECTION direction , vector<TABLE_LINE> &lines)
{
    int x,y,i;

    int *bufferINT = new int[width*height];
    memset(bufferINT,0,sizeof(int)*width*height);

    for( i = 0 ; i < width * height ; i++ )
    {
        bufferINT[i] = edgeMap[i];
    }

    vector<REGION_ENTRY> regions = connectedComponent(bufferINT,width,height,20);
    //vector<TABLE_LINE> lines;

    for( i = 0 ; i < regions.size() ; i++ )
    {
        if( regions.at(i).IsRegion )
        {
            TABLE_LINE line;
            line.label = regions.at(i).label;
            line.fitCounter = 0;
            lines.push_back(line);
            //qDebug() << "region: " << i << regions.at(i).label;
        }
    }

    unsigned char *pCenterMap = centerMap + width + 1;
    int *pBufferINT = bufferINT + width + 1;

    for( y = 1 ; y < height - 1; y++ , pBufferINT += width , pCenterMap += width )
    {
        for( x = 1 ; x < width - 1 ; x++ )
        {
            if( pBufferINT[x] > 0 && pCenterMap[x] == 255 )
            {
                Point point;
                point.x = x;
                point.y = y;
                int label = pBufferINT[x];
                lines.at(label-1).centerPoints.push_back(point);
            }
        }
    }


    for( i = 0 ; i < lines.size() ; i++ )
    {
        lineFit( lines.at(i).centerPoints , lines.at(i).a , lines.at(i).b , direction );
       // if( lines.at(i).a == 0.0 )
        // qDebug() << "line " << i << lines.at(i).label << lines.at(i).centerPoints.size() << lines.at(i).a << lines.at(i).b  << lines.at(i).centerPoints.size();
    }

    int j,k;
    for( i = 0 ; i < lines.size() ; i++ )
    {
        vector<Point> fitPoints;
        for( j = i ; j < lines.size() ; j++)
        {
            if( (i != j) && (lines.at(i).centerPoints.size() > 10) )
            {
                float a = lines.at(i).a;
                float b = lines.at(i).b;

                for( k = 0 ; k < lines.at(j).centerPoints.size() ; k++ )
                {
                    int x = lines.at(j).centerPoints.at(k).x;
                    int y = lines.at(j).centerPoints.at(k).y;

                    if( a != 0.0 )
                    {
                        if( fabs(a * x + b - y) < 5.0 )
                        {
                            Point point;
                            point.x = x;
                            point.y = y;
                            lines.at(i).fitCounter++;
                           // fitPoints.push_back(point);
                            //lines.at(j).fitCounter--;
                        }
                    }
                    else
                    {
                        if( direction == V_DIR )
                        {
                            if( fabs(b-x) < 5.0 )
                            {
                                lines.at(i).fitCounter++;
                                //lines.at(j).fitCounter--;
                            }
                        }
                        else
                        {
                            if( fabs(a * x + b - y) < 5.0 )
                            {
                                lines.at(i).fitCounter++;
                                //lines.at(j).fitCounter--;
                            }
                        }
                    }
                }
            }
        }
        //for( m = 0 ; m < fitPoints.size() ; m++ )
         //   lines.at(i).centerPoints.push_back(fitPoints.at(m));

        lines.at(i).fitCounter += lines.at(i).centerPoints.size();
    }

    std::sort(lines.begin(), lines.end(),tableLineCompareFitCounter);

/*
    for( i = 0 ; i < 10 ; i++ )
    {
        //lineFit( lines.at(i).centerPoints , lines.at(i).a , lines.at(i).b , direction );

        qDebug() << "line " << i << lines.at(i).label << lines.at(i).centerPoints.size() << lines.at(i).a << lines.at(i).b << lines.at(i).fitCounter;
    }

    memset(edgeMap,0,sizeof(unsigned char)*width*height);

    unsigned char *pEdgeMap = edgeMap + width + 1;

    for( y = 1 ; y < height - 1; y++ , pEdgeMap += width )
    {
        for( x = 1 ; x < width - 1 ; x++ )
        {
            for( i = 0 ; i < lines.size() ; i++ )// lines.size()
            {
                if( fabs( lines.at(i).a ) < 0.2 )//&& lines.at(i).fitCounter > 50  ) //&& lines.at(i).a < -10.0 )
                {
                    float a = lines.at(i).a;
                    float b = lines.at(i).b;

                    if( a != 0.0 )
                    {
                        if( fabs(a * x + b - y) < 5.0 )
                        {
                            pEdgeMap[x] = 255;
                        }
                    }
                    else
                    {
                        if( direction == V_DIR )
                        {
                            if( fabs(b-x) < 5.0 )
                            {
                                pEdgeMap[x] = 255;
                            }
                        }
                        else
                        {
                            if( fabs(a * x + b - y) < 5.0 )
                            {
                                pEdgeMap[x] = 255;
                            }
                        }
                    }
                }
            }
        }
    }

    for( i = 0 ; i < width * height ; i++ )
    {
        edgeMap[i] = bufferINT[i] ;
    }

    for( i = 0 ;  i < 5 ; i++ )
    {
        qDebug() << i << lines.at(i).a << lines.at(i).b << lines.at(i).centerPoints.size();
        for( k = 0 ; k < lines.at(i).centerPoints.size() ; k++ )
        {
            int x = lines.at(i).centerPoints.at(k).x;
            int y = lines.at(i).centerPoints.at(k).y;

             edgeMap[y*width+x]= 255;
        }
    }
*/
    SAFE_RELEASE(bufferINT);

    if( lines.size() > 0 )
        return atan(lines.at(0).a) * 180 / 3.1415;
    else
        return 0.0;
}

/**
     * 將偵測垂直線並回傳其傾斜角度
     *
     * @param unsigned char *buffer8
     * @param int   width
     * @param int   height
     * @param int   widthEff
     * @param vector<TABLE_LINE> &lines
     * @return float 影像垂直傾斜角度
     */
float ImageProcess::vertivalLineFilter(unsigned char *buffer8 , int width , int height , vector<TABLE_LINE> &lines)
{
    int x,y,i;

    unsigned char *edgeMap = new unsigned char[width*height];
    memset(edgeMap,0,sizeof(unsigned char)*width*height);

    unsigned char *centerMap = new unsigned char[width*height];
    memset(centerMap,0,sizeof(unsigned char)*width*height);

    unsigned char *pCenterMap = centerMap + width + 1;
    unsigned char *pBuffer8 = buffer8 + width + 1;
    unsigned char *pEdgeMap = edgeMap + width + 1;

    for( y = 1 ; y < height - 1 ; y++ , pBuffer8 += width , pEdgeMap += width , pCenterMap += width )
    {
        for( x = 1 ; x < width - LINE_WIDTH ; x++ )
        {
            if( pBuffer8[x] < pBuffer8[x-1] )//possitive edge
            {
                pEdgeMap[x] = 255;
                bool isEnd = false;
                int length = 1;

                for( i = 1 ; i <= LINE_WIDTH ; i++ )
                {
                    length++;
                    unsigned char *pSubBuffer8 = pBuffer8 + x + i;
                    unsigned char *pSubEdgeMap = pEdgeMap + x + i;

                    if( pSubBuffer8[0] == 0 )
                        pSubEdgeMap[0] = 255;

                    if( pSubBuffer8[0] < pSubBuffer8[1] )//negative edge
                    {
                        isEnd = true;
                        break;
                    }
                }

                if( !isEnd )
                    verticalClear(edgeMap ,  width , x,  y , LINE_WIDTH );

                pCenterMap[x+length/2] = 255;
            }
        }
    }

    float skew =  getRegionLineFit(edgeMap,centerMap,width,height,V_DIR,lines);

    qDebug() << "vertivalLineFilter";


    for( i = 0 ; i < width * height ; i++ )
    {
        buffer8[i] = edgeMap[i];
    }

    SAFE_RELEASE(centerMap);
    SAFE_RELEASE(edgeMap);

    return skew;
}


/**
     * 利用一堆點去 fit 出線方程式
     *
     * @param vector<Point> pints
     * @param float &a
     * @param float &b
     * @param REGION_DIRECTION direction
     * @return void
     */
void ImageProcess::lineFit(vector<Point> pints , float &a,float &b , REGION_DIRECTION direction )
{
    int i;
    double t , sx = 0.0 , sy = 0.0 , st2 = 0.0 , sxoss = 0.0;
    a=0.0;

    int dataSize = pints.size();

    for ( i = 0 ; i < dataSize; i++ )
    {
        sx += pints.at(i).x;//Point[i].x;
        sy += pints.at(i).y;//Point[i].y;
    }

    sxoss = sx / dataSize;


    for( i = 0 ; i < dataSize ; i++ )
    {
        t = pints.at(i).x - sxoss;
        st2 += t * t;
        a+= t * pints.at(i).y;
    }

    if( st2 == 0 )//處理無限大問題, m = 0 標示為無限大狀況, 並不是實際上的斜率
    {
        a = 0;
    }
    else
    {
        a /= st2;
    }
    b = ( sy - sx * (a) ) / dataSize;
    //qDebug() << "fit " << a << b << dataSize;

    if( direction == V_DIR )//處理垂直斜率無限大問題
    {
        // add 2010.7.2
        if( fabs(a) < 0.2 )//|| fabs(a) > 50 )
        {
            a = 0.0;
            b = sxoss;
        }
    }

}

/**
     * 將偵測水平線並回傳其傾斜角度
     *
     * @param unsigned char *buffer8
     * @param int   width
     * @param int   height
     * @param int   widthEff
     * @param vector<TABLE_LINE> &lines
     * @return float 影像水平傾斜角度
     */
float ImageProcess::horizontalLineFilter(unsigned char *buffer8 , int width , int height, vector<TABLE_LINE> &lines)
{
    int x,y,i;

    unsigned char *edgeMap = new unsigned char[width*height];
    memset(edgeMap,0,sizeof(unsigned char)*width*height);

    unsigned char *centerMap = new unsigned char[width*height];
    memset(centerMap,0,sizeof(unsigned char)*width*height);

    unsigned char *pCenterMap = centerMap + width + 1;
    unsigned char *pBuffer8 = buffer8 + width + 1;
    unsigned char *pEdgeMap = edgeMap + width + 1;

    for( y = 1 ; y < height - 8 ; y++ , pBuffer8 += width , pEdgeMap += width , pCenterMap += width )
    {
        for( x = 1 ; x < width - 1 ; x++ )
        {
            if( pBuffer8[x] < pBuffer8[x-width] )//possitive edge
            {
                pEdgeMap[x] = 255;
                bool isEnd = false;

                int length = 1;
                for( i = 1 ; i <= LINE_WIDTH ; i++ )
                {
                    length++;
                    unsigned char *pSubBuffer8 = pBuffer8 + x + width * i;
                    unsigned char *pSubEdgeMap = pEdgeMap + x + width * i;

                    if( pSubBuffer8[0] == 0 )
                        pSubEdgeMap[0] = 255;

                    if( pSubBuffer8[0] < pSubBuffer8[width] )//negative edge
                    {
                        isEnd = true;
                        break;
                    }
                }

                if( !isEnd )
                    horizontalClear(edgeMap ,  width , x,  y ,  LINE_WIDTH );

                pCenterMap[x+length/2*width] = 255;
            }
        }
    }

    float skew = getRegionLineFit(edgeMap,centerMap,width,height,H_DIR,lines);

    //qDebug() << "horizontalLineFilter";
    for( i = 0 ; i < width * height ; i++ )
    {
        buffer8[i] = edgeMap[i];
    }

    //SAFE_RELEASE(bufferINT);
    SAFE_RELEASE(centerMap);
    SAFE_RELEASE(edgeMap);

    return skew;
}

/**
     * 利用 edge 數量辨識影像是否上下顛倒
     *
     * @param unsigned char *buffer8
     * @param int   width
     * @param int   height
     * @param int   top
     * @param int   bottom
     * @return bool 影像是否顛倒
     */
bool ImageProcess::isReverse(unsigned char *buffer8 , int width , int height , int top, int bottom)
{
    int x,y;

    //qDebug() << "top bottom "<< top << bottom;
    unsigned char *pBuffer8 = buffer8 + 1;

    int topEdgeSum = 0;
    int bottomEdgeSum = 0;

    for( y = 0 ; y < height ; y++ , pBuffer8 += width )
    {
        for( x = 1 ; x < width - 1 ; x++ )
        {
            int value = abs( pBuffer8[x-1] - pBuffer8[x+1] );
            if( y < top )
            {
                topEdgeSum += value;
            }

            if( y > bottom )
            {
                bottomEdgeSum += value;
            }
        }
    }

    //qDebug() << "bottomEdgeSum " << bottomEdgeSum;
    //qDebug() << "topEdgeSum " << topEdgeSum;

    return bottomEdgeSum > topEdgeSum;

}

/**
     * 將 points 先做y方向排序再做x方向排序
     *
     * @param vector<Point> &cornerPoints
     * @return void
     */
 void ImageProcess::sortCorner(vector<Point> &cornerPoints)
 {
     std::sort(cornerPoints.begin(), cornerPoints.end(),compareY);

     int i,j;
     int index = 0;
     vector<Point> points[TABLE_ROW];
     for( i = 0 ; i < cornerPoints.size() ; i++ )
     {
         points[index].push_back(cornerPoints.at(i));
         if( ((i + 1) % TABLE_COLUMN ) == 0)
         {
             //qDebug() << "compare" << index << i << rbPoints[index].size();
             std::sort(points[index].begin(), points[index].end(),compareX);
             index++;
         }
     }
     cornerPoints.clear();
     for( i = 0 ; i < index ; i++ )
     {
         for( j = 0 ; j < points[i].size() ; j++ )
         {
             cornerPoints.push_back(points[i].at(j));
         }
     }
 }

 /**
      * 將 points 做 merge, 距離在 25 pixel 內視為同一個 group
      * @param vector<REGION_ENTRY> &regions
      * @param vector<Point> &points
      * @param CORNER_TYPE type
      * @return void
      */
 void ImageProcess::getCornerPoints(vector<REGION_ENTRY> &regions, vector<Point> &points, CORNER_TYPE type)
 {
     int i,j;
     int end;
     float ratio = 0.0;
     for( i = 1 ; i < regions.size() - 1 ; i++ )
     {
         float ratinTemp = regions.at(i).size / regions.at(i+1).size;
         if(  ratinTemp > ratio )
         {
             ratio = ratinTemp;
             end = i;
         }
         //qDebug() << i << regions.at(i).left << regions.at(i).top << regions.at(i).size << end;
     }
     //qDebug() << "end: " << end;

     vector<Point> crossPoints;
     for( i = 1 ; i <= end ; i++ )
     {
         for( j = 1 ; j <= end ; j++ )
         {
             if( i != j )
             {
                 Point point;
                 if( type == LEFT_TOP )
                 {
                     point.x = regions.at(i).left;
                     point.y = regions.at(j).top;
                 }
                 else
                 {
                     point.x = regions.at(i).right;
                     point.y = regions.at(j).bottom;
                 }
                 crossPoints.push_back(point);
             }
         }
     }

     for( i = 0 ; i < crossPoints.size() ; i++ )
     {
         int count = 1;

         int x1 = crossPoints.at(i).x;
         int y1 = crossPoints.at(i).y;
         int sumX = x1;
         int sumY = y1;

         for( j = i + 1; j < crossPoints.size() ; j++ )
         {
             int x2 = crossPoints.at(j).x;
             int y2 = crossPoints.at(j).y;

             if( y2 > 0 && x2 > 0 )
             {
                 int distance = (x1 - x2)*(x1 - x2) + (y1 - y2)*(y1 - y2);

                 if( distance < 625 )
                 {
                     count++;
                     //if( x2 > x1 )
                        sumX += x2;
                     //if( y2 > y1 )
                        sumY += y2;
                     crossPoints.at(j).x = crossPoints.at(j).y =-9999;
                 }
             }
         }
         if( count > 1 )
         {
             Point point;
             point.x = sumX / count;
             point.y = sumY / count;
             points.push_back(point);
         }
     }

     if( type == LEFT_TOP )
        qDebug() << "getLeftTopPoints :"<< points.size();
     else
        qDebug() << "getRightBottomPoints :"<< points.size();

     sortCorner(points);
 }


/**
     * 將 32bit ARGB 影像轉換成 8 bits 灰階影像
     *
     * @param unsigned char *buffer32
     * @param int   width
     * @param int   height
     * @param int   widthEff
     * @param unsigned char *buffer8
     * @return void
     */
void ImageProcess::trans2Gray(unsigned char *buffer32 , int width , int height, int widthEff , unsigned char *buffer8 )
{
    int x,y,k;

    unsigned char *pBuffer32 = buffer32;
    unsigned char *pBuffer8 = buffer8;
    int bpp = widthEff / width;

    for( y = 0 ; y < height ; y++ , pBuffer32 += widthEff , pBuffer8 += width )
    {
        for( x = 0 , k = 0 ; x < width ; x++ , k += bpp)
        {
            if( pBuffer32[k] > 128)
                pBuffer8[x] = 255;//(pBuffer32[k] + pBuffer32[k+1] + pBuffer32[k+2]) / 3;
            else
                pBuffer8[x] = 0;
        }
    }

}

/**
     * 將 8 bits 灰階影像轉換成 32bit ARGB 影像
     *
     * @param unsigned char *buffer8
     * @param int   width
     * @param int   height
     * @param int   widthEff
     * @param unsigned char *buffer32
     * @return void
     */
void ImageProcess::trans2RGB(unsigned char *buffer8 , int width , int height, int widthEff , unsigned char *buffer32 )
{
    int x,y,k;

    unsigned char *pBuffer32 = buffer32;
    unsigned char *pBuffer8 = buffer8;
    int bpp = widthEff / width;

    //qDebug() << "bpp " << bpp;

    for( y = 0 ; y < height ; y++ , pBuffer32 += widthEff , pBuffer8 += width )
    {
        for( x = 0 , k = 0 ; x < width ; x++ , k += bpp )
        {
           //pBuffer32[k] = pBuffer32[k+1] = pBuffer32[k+2]  = pBuffer32[k+3] = pBuffer8[x];

            if( bpp == 4 || bpp == 3 )
            {
                pBuffer32[k] = pBuffer32[k+1] = pBuffer32[k+2]  = pBuffer32[k+3] = 255;
                   if( pBuffer8[x] < 255 && pBuffer8[x] > 0 )
                   {
                        pBuffer32[k] = pBuffer8[x]*11%255+31;
                        pBuffer32[k+1] = pBuffer8[x]*13%255+97;
                        pBuffer32[k+2] = pBuffer8[x]*31%255+43;
                   }
                   else
                   {
                      // pBuffer32[k] = pBuffer32[k+1] = pBuffer32[k+2]  = pBuffer32[k+3] = pBuffer8[x];
                   }
            }
            else
            {
                    pBuffer32[k] = pBuffer8[x];
            }

        }
    }

}


/**
     * 移除背景 noise
     *
     * @param unsigned char *buffer8
     * @param int   width
     * @param int   height
     * @return void
     */
void ImageProcess::noiseRemove(unsigned char *buffer8 , int width , int height )
{
    int x,y;

    unsigned char *bufferTemp = new unsigned char[width*height];

    memcpy(bufferTemp,buffer8,sizeof(unsigned char)*width*height);

    unsigned char *pBuffer8 = buffer8 + width + 1;
    unsigned char *pBufferTemp = bufferTemp + width + 1;

    for( y = 1 ; y < height - 1; y++ ,  pBuffer8 += width , pBufferTemp += width)
    {
        for( x = 1 ; x < width - 1; x++ )
        {
           if( pBuffer8[x] == 0 )
           {
               if( pBuffer8[x-1] == 255 &&  pBuffer8[x+1] == 255)
               {
                   pBufferTemp[x] = 255;
               }
               if( pBuffer8[x-width] == 255 &&  pBuffer8[x+width] == 255)
               {
                   pBufferTemp[x] = 255;
               }

           }
        }
    }

     memcpy(buffer8,bufferTemp,sizeof(unsigned char)*width*height);
}

/**
     * 將最大的面積 region 外的黑點移除
     *
     * @param unsigned char *buffer8
     * @param int   width
     * @param int   height
     * @return void
     */
void ImageProcess::getMaxRegion(unsigned char *buffer8 , int width , int height )
{
    int y,x;
    //get max size region
    vector<REGION_ENTRY> regionList;

    int i;
    int *bufferINT8 = new int[width*height];
    for( i = 0 ; i < width * height ; i++ )
    {
        bufferINT8[i] = buffer8[i];
    }

    regionList = connectedComponent(bufferINT8,width,height);

    int maxValue = -1;
    int label = -1;
    int index = -1;
    for( i = 0 ; i < regionList.size() ; i++ )
    {
        if( regionList.at(i).size > maxValue )
        {
            maxValue = regionList.at(i).size;
            label = regionList.at(i).label;
            index = i;
        }
        //qDebug() << i << regions[i].IsRegion << regions[i].c;
    }
    //qDebug() << index <<maxValue;
    //qDebug() << regions[index].top << regions[index].bottom;

    unsigned char *pBuffer8 = buffer8;

    for( y = 0 ; y < height ; y++ , pBuffer8 += width )
    {
        for( x = 0 ; x < width ; x++ )
        {
            if( x <= regionList.at(index).left || x >= regionList.at(index).right )
                pBuffer8[x] = 255;
            if( y <= regionList.at(index).top || y >= regionList.at(index).bottom )
                pBuffer8[x] = 255;
        }
    }

}

void ImageProcess::getWordBounderH(unsigned char *buffer8 , int width , int height)
{

    int x,y;

    unsigned char *pBuffer8 = buffer8;

    int *horizontalHistogram = new int[height];
    memset(horizontalHistogram,0,sizeof(int)*height);

    int center = (height + 1) / 2;
    int stepH = height/8;

    for( y = 0 ; y < height ; y++ , pBuffer8 += width )
    {
        for( x = 0 ; x < width ; x++ )
        {
            if( pBuffer8[x] == 0 )
            {
                horizontalHistogram[y]++;
            }
        }
    }

    int topBounder = center - stepH;
    float ratioTop = 0.0;
    int topIndex = 0;

    int botttomBounder = center + stepH;
    float ratioBottom = 0.0;
    int bottomIndex = 0;

    for( y = 0 ; y < height - 1 ; y++ )
    {
        if( y < topBounder )
        {
            float ratioT = (float)horizontalHistogram[y+1]/(float)horizontalHistogram[y];
            if( ratioT > ratioTop )
            {
                ratioTop = ratioT;
                topIndex = y;
            }

            //qDebug() << y << horizontalHistogram[y] << ratioT;
        }

        if( y > botttomBounder )
        {
            float ratioB = (float)horizontalHistogram[y]/(float)horizontalHistogram[y+1];
            if( ratioB > ratioBottom )
            {
                ratioBottom = ratioB;
                bottomIndex = y;
            }

            //qDebug() << y << horizontalHistogram[y] << ratioB;
        }
    }

    qDebug() << "top" << topIndex << "bottom" << bottomIndex;


    int *verticalHistogram = new int[width];
    memset(verticalHistogram,0,sizeof(int)*width);

    pBuffer8 = buffer8;

    for( y = 0 ; y < height ; y++ , pBuffer8 += width )
    {
        for( x = 0 ; x < width ; x++ )
        {
            if( y >= topIndex && y <= bottomIndex )
            {
                if( pBuffer8[x] == 0 )
                verticalHistogram[x]++;
            }
        }
    }

    for( x = 0 ; x < width ; x++ )
    {
        qDebug() << x << verticalHistogram[x];
    }

    SAFE_RELEASE(horizontalHistogram);
    SAFE_RELEASE(verticalHistogram);

}

/**
     * 記錄文字位置, 如果文字 寬度>高度 則認定為非文字
     *
     * @param unsigned char *buffer8
     * @param int   width
     * @param int   height
     * @return vector<REGION_ENTRY> 記錄文字位置的 region vector
     */
vector<REGION_ENTRY> ImageProcess::getCharPosition( unsigned char *buffer8 , int width , int height , int *horizontal , int &top , int &bottom )
{
   vector<REGION_ENTRY> charPositions;
    int i,x,y;

    int *bufferINT8 = new int[width*height];
    for( i = 0 ; i < width * height ; i++ )
    {
        bufferINT8[i] = 255 - buffer8[i];
    }

    //qDebug() << "filter size "<<height*height/1024;
    charPositions = connectedComponent(bufferINT8,width,height,height*height/128);//128為大約估算值,沒意義

    top = height;
    bottom = 0;
    int left = width;
    int right = 0;
    int index = -1;
    int maxArea = 0;
    for( i = 0 ; i < charPositions.size() ; i++ )
    {
            int wordWidth = abs(charPositions.at(i).right - charPositions.at(i).left) + 1;
            int wordHeight = abs(charPositions.at(i).bottom - charPositions.at(i).top) + 1;

            if( wordWidth > wordHeight || (wordHeight < height / 4) )
            {
                charPositions.at(i).IsRegion = false;
            }
            else
            {
                if( maxArea < charPositions.at(i).size )
                {
                    index = i;
                    maxArea = charPositions.at(i).size;
                }

                if( charPositions.at(i).left < left )
                    left = charPositions.at(i).left;
                if( charPositions.at(i).right > right )
                    right = charPositions.at(i).right;
            }

            //qDebug() << i << charPositions.at(i).IsRegion << charPositions.at(i).size << width * height;
    }

    if(index != -1)
    {
        top = charPositions.at(index).top;
        bottom = charPositions.at(index).bottom;
    }

    for( i = 0 ; i < width * height ; i++ )
    {
        //bufferINT8[i] = 255 - buffer8[i];
    }

    int *pBufferINT8 = bufferINT8 + width + 1;
    unsigned char *pBuffer8 = buffer8 + width  + 1;
    //int *horizontal = new int[width];
    memset(horizontal,0,sizeof(int)*width);
    int *vertical = new int[height];
    memset(vertical,0,sizeof(int)*height);

    for( y = 1 ; y < height - 1 ; y++ , pBufferINT8 += width , pBuffer8 += width )
    {
        for( x = 1 ; x < width - 1 ; x++ )
        {
            int index = pBufferINT8[x];
            if( index < charPositions.size() )
            {
                //目前有問題, 似乎label錯誤
                if( !charPositions.at(index).IsRegion )
                {
                    //pBuffer8[x] = 255;
                }
            }
            if( x < left || x > right )
            {
                pBuffer8[x] = 255;
                pBufferINT8[x] = 0;
            }

            if( y < top || y > bottom )
            {
                pBuffer8[x] = 255;
                pBufferINT8[x] = 0;
            }
            else
            {
                horizontal[x] += pBufferINT8[x];
                vertical[y] += pBuffer8[x] - pBuffer8[x-width];
            }

        }
    }

    for( i = 0 ; i < width * height ; i++ )
    {
        bufferINT8[i] = 255 - buffer8[i];
    }
    charPositions.clear();
    charPositions = connectedComponent(bufferINT8,width,height,height*height/128);//128為大約估算值,沒意義

    for( i = 0 ; i < charPositions.size() ; i++ )
    {
            int wordWidth = abs(charPositions.at(i).right - charPositions.at(i).left) + 1;
            int wordHeight = abs(charPositions.at(i).bottom - charPositions.at(i).top) + 1;

            if( wordWidth > wordHeight || (wordHeight < height / 4) )
            {
                charPositions.at(i).IsRegion = false;
            }
    }

    //qDebug() << top << bottom;
                /**/
    for( x = 0 ; x < width ; x++ )
    {
       // qDebug() << horizontal[x];
    }

    SAFE_RELEASE(bufferINT8);

    return charPositions;
}

/**
     * 將相鄰的pixel標示成同一個group,並且回傳記錄每個 group 資訊的 vector
     *
     * @param int   *buffer8
     * @param int   width
     * @param int   height
     * @param int   minSize
     * @return vector<REGION_ENTRY> 記錄每個 group 資訊的 vector
     */
vector<REGION_ENTRY> ImageProcess::connectedComponent(int *buffer8, int width, int height, int minSize )
{
    vector<REGION_ENTRY> regionList;
    int	i, j, k,index = 1;
    int  *pBufferTemp, *pBuffer8;

    // a b c
    // d x
    int a, b, c, d;

    if( !buffer8 )
        return regionList;

    int	*bufferTemp = new int[width * height];
    int	*parent = new int[width * height];

    // first pass
    pBufferTemp = bufferTemp + width;
    pBuffer8 = buffer8 + width;

    memset(bufferTemp, 0, sizeof(int) * width * height);
    memset(parent, 0, sizeof(int) * width * height);

    for( i = 1 ; i < height - 1 ; i++ , pBuffer8 += width, pBufferTemp += width )
    {
        for( j = 1 ; j < width -1  ; j++ )
        {
            if( pBuffer8[j] > 128 )
            {
                a = pBufferTemp[j - width - 1];
                b = pBufferTemp[j - width];
                c = pBufferTemp[j - width + 1];
                d = pBufferTemp[j - 1];

                if(a)
                {
                    pBufferTemp[j] = a;
                    if(b && a != b) connectedUnion(a, b, parent);
                    if(c && a != c) connectedUnion(a, c, parent);
                    if(d && a != d) connectedUnion(a, d, parent);
                }
                else if(b)
                {
                    pBufferTemp[j] = b;
                    if(c && b != c) connectedUnion(b, c, parent);
                    if(d && b != d) connectedUnion(b, d, parent);
                }
                else if(c)
                {
                    pBufferTemp[j] = c;
                    if(d && c != d) connectedUnion(c, d, parent);
                }
                else if(d != 0)
                    pBufferTemp[j] = d;
                else	// new label
                {
                    pBufferTemp[j] = index;
                    parent[index] = 0;
                    index++;
                }
            }
        }
    }

    // Processing equivalent class
    for( i = 1 ; i < index ; i++ )
    {
        if(parent[i])		// find parents
        {
            j = i;
            while(parent[j])
                j = parent[j];
            parent[i] = j;
        }
    }

    k = 1;
    for( i = 1 ; i < index ; i++ )
    {
        if(!parent[i])
        {
            parent[i] = k;
            k++;
        }
        else
        {
            j = parent[i];
            parent[i] = parent[j];
        }
    }
    index = k;

    if( index > 1 )
        secondPass(bufferTemp ,buffer8 ,  width ,  height , index , parent,  regionList , minSize );

    SAFE_RELEASE(bufferTemp);
    SAFE_RELEASE(parent);

    return regionList;
}

void ImageProcess::secondPass(int *bufferINT , int *buffer8 , int width , int height , int labelCounter , int *parent,  vector<REGION_ENTRY> &regionList , int minSize )
{
    int i,j,k;
    int *counter = new int[labelCounter];
    memset(counter,0,sizeof(int)*labelCounter);

    // relabeling
    int *pBufferTemp = bufferINT + width;
    for( i = 1 ; i < height - 1 ; i++ , pBufferTemp += width )
    {
        for( j = 1 ; j < width - 1 ; j++ )
        {
            if( pBufferTemp[j] > 0 )
            {
                int index = pBufferTemp[j];
                k = parent[index];
                counter[k]++;
                pBufferTemp[j] = k;
            }
        }
    }

    k = 0;
    for( i = 0 ; i < labelCounter ; i++ )
    {
        if( counter[i] > minSize )
        {
            REGION_ENTRY region;
            region.IsRegion = true;
            region.bottom = 0; region.top = height;
            region.size = 0; region.right = 0;
            region.cx = 0; region.cy = 0;
            region.left = width;
            region.label = k + 1;
            regionList.push_back(region);
            parent[i] = k + 1;
            k++;

            //qDebug() << "size :" << counter[i]  ;
        }
        else
            parent[i] = 0;
    }

    pBufferTemp = bufferINT;
    int *pBuffer8 = buffer8;
    //index = k;

    for( i = 0 ; i < height ; i++ , pBufferTemp += width, pBuffer8 += width )//
    {
        for( j = 0 ; j < width ; j++ )
        {
            if(pBufferTemp[j] > 0)
            {
                int index = pBufferTemp[j];
                if(counter[index] > minSize)
                {
                    k = parent[index] - 1;
                    regionList.at(k).size += 1;
                    regionList.at(k).cx += j;
                    regionList.at(k).cy += i;

                    if(regionList.at(k).top > i)
                        regionList.at(k).top = i;
                    if(regionList.at(k).bottom < i)
                        regionList.at(k).bottom = i;
                    if(regionList.at(k).right < j)
                        regionList.at(k).right = j;
                    if(regionList.at(k).left > j)
                        regionList.at(k).left = j;

                   pBuffer8[j] = parent[index];
               }
               else
                   pBuffer8[j] = 0;
            }
            else
                pBuffer8[j] = 0;
        }
    }

    for( i = 0 ; i < regionList.size() ; i++ )
    {
        regionList.at(i).cx /= regionList.at(i).size;
        regionList.at(i).cy /= regionList.at(i).size;
        //qDebug() << "c"<< i << regionList.at(i).left << regionList.at(i).top << regionList.at(i).right << regionList.at(i).bottom;
    }


    SAFE_RELEASE(counter);
}

void ImageProcess::connectedUnion(int x, int y, int *p)
{
    int j = x,
        k = y;

    while(p[j])	j = p[j];
    while(p[k])	k = p[k];

    if(j > k)	p[j] = k;
    if(j < k)	p[k] = j;
}
