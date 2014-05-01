#include "imageprocess.h"
#include <QDebug>



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

    for( y = 0 ; y < height ; y++ , pBuffer32 += widthEff , pBuffer8 += width )
    {
        for( x = 0 , k = 0 ; x < width ; x++ , k += 4)
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

    for( y = 0 ; y < height ; y++ , pBuffer32 += widthEff , pBuffer8 += width )
    {
        for( x = 0 , k = 0 ; x < width ; x++ , k += 4)
        {
           pBuffer32[k] = pBuffer32[k+1] = pBuffer32[k+2] = pBuffer8[x];
        }
    }

}

void ImageProcess::horizontalEdge(unsigned char *buffer8,int width , int height )
{

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

/**
     * 記錄文字位置, 如果文字 寬度>高度 則認定為非文字
     *
     * @param unsigned char *buffer8
     * @param int   width
     * @param int   height
     * @return vector<REGION_ENTRY> 記錄文字位置的 region vector
     */
vector<REGION_ENTRY> ImageProcess::getCharPosition( unsigned char *buffer8 , int width , int height )
{
   vector<REGION_ENTRY> charPositions;
    int i;

    int *bufferINT8 = new int[width*height];
    for( i = 0 ; i < width * height ; i++ )
    {
        bufferINT8[i] = 255 - buffer8[i];
    }

    charPositions = connectedComponent(bufferINT8,width,height,height*height/128);//128為大約估算值,沒意義

    for( i = 0 ; i < charPositions.size() ; i++ )
    {
            int width = abs(charPositions.at(i).right - charPositions.at(i).left) + 1;
            int height = abs(charPositions.at(i).bottom - charPositions.at(i).top) + 1;

            if( width > height )
            {
                charPositions.at(i).IsRegion = false;
            }

            //qDebug() << i << charPositions.at(i).IsRegion << charPositions.at(i).size << width * height;
    }

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
        secondPass(bufferTemp ,  width ,  height , index , parent,  regionList , minSize );

    SAFE_RELEASE(bufferTemp);
    SAFE_RELEASE(parent);

    return regionList;
}

void ImageProcess::secondPass(int *bufferINT , int width , int height , int labelCounter , int *parent,  vector<REGION_ENTRY> &regionList , int minSize )
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
    //pBuffer8 = buffer8;
    //index = k;

    for( i = 0 ; i < height ; i++ , pBufferTemp += width )//, pBuffer8 += width
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

                   // pBuffer8[j] = parent[a];
                }
               //else
                   // pBuffer8[j] = 0;
            }
            //else
                //pBuffer8[j] = 0;
        }
    }

    for( i = 0 ; i < regionList.size() ; i++ )
    {
        regionList.at(k).cx /= regionList.at(k).size;
        regionList.at(k).cy /= regionList.at(k).size;
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
