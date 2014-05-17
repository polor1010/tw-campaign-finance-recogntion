#include "imagespliter.h"

ImageSpliter::ImageSpliter()
{
}

void ImageSpliter::saveImage( unsigned char* buffer8,int width, int height , int index)
{
    int x,y,k;
    QImage regionImage = QImage(width,height,QImage::Format_ARGB32);
    unsigned char* regionBuffer = regionImage.bits();
    unsigned char* pRegionBuffer = regionBuffer;
    unsigned char* pBuffer8 = buffer8;
    int subEffWidth = regionImage.bytesPerLine();

    for( y = 0 ; y < height ; y++ , pRegionBuffer += subEffWidth , pBuffer8 += width )
    {
        for( x = 0 , k = 0; x < width ; x++ , k += 4 )
        {
            pRegionBuffer[k] = pRegionBuffer[k+1] = pRegionBuffer[k+2] = pRegionBuffer[k+3] = pBuffer8[x];
        }
    }

    QString saveName = fileName;
    int fileNameLength = fileName.size();
    QString filePath = QDir::currentPath() + "/" + saveName.insert(fileNameLength-4,"-"+QString::number(index));
    qDebug() << filePath;
    regionImage = regionImage.scaled(width*3,height*3);
    regionImage.save(filePath);

    FILE *file = fopen("split.cvs","a");
    fprintf(file,"%s\n",saveName.toLocal8Bit().data());
    fclose(file);
}

bool ImageSpliter::getTables(QString filePath)
{
     int splitLength = filePath.split('/').size();
     fileName = filePath.split('/').at(splitLength-1);
     //qDebug() << fileName;
     QImage image;
     if( !image.load(filePath) )
     {
         qDebug() << "image load error";
         return false;
     }

     int height = image.height();
     int width = image.width();
     int widthEff = image.bytesPerLine();
     //qDebug() << widthEff << width;
     unsigned char *pBuffer = image.bits();

     unsigned char *buffer8 = new uchar[width*height];
     memset(buffer8,0,sizeof(uchar)*width*height);

     int *bufferINT = new int[width*height];
     memset(buffer8,0,sizeof(int)*width*height);

     trans2Gray(pBuffer,width,height,widthEff,buffer8);

     OtsuThresholdRAW(buffer8,width,height);

     int i;
     for( i = 0 ; i < width * height ; i++ )
         bufferINT[i] = buffer8[i];

     vector<REGION_ENTRY> regions = connectedComponent(bufferINT,width,height,50);
     std::sort(regions.begin(), regions.end(),regionCompareSize);

     int end = 0;
     float ratio = 0.0;
     if( regions.size() > 12 )
     {
         for( i = 1 ; i < 12 ; i++ )
         {
             float ratinTemp = regions.at(i).size / regions.at(i+1).size;
             if(  ratinTemp > ratio )
             {
                 ratio = ratinTemp;
                 end = i;
             }
             //qDebug() << i << regions.at(i).left << regions.at(i).top << regions.at(i).size << end;
         }
     }
     //qDebug() << "end " << end;

     int x,y;
     for( i = 1 ; i <= end ; i++ )
     {
         int left = regions.at(i).left;
         int right = regions.at(i).right;
         int top = regions.at(i).top;
         int bottom = regions.at(i).bottom;

         unsigned char *pBuffer8 = buffer8 + top * width + left;
         int subWidth = abs(left-right)+1;
         int subHeight = abs(top-bottom)+1;

         unsigned char *tableBuffer = new unsigned char[subWidth*subHeight];
         memset(tableBuffer,0,sizeof(unsigned char)*subWidth*subHeight);

         unsigned char *pTableBuffer = tableBuffer;
         for( y = 0 ; y < subHeight ; y++ , pBuffer8 += width , pTableBuffer += subWidth )
         {
             for( x = 0 ; x < subWidth ; x++ )
             {
                 pTableBuffer[x] = pBuffer8[x];
             }
         }

         saveImage(tableBuffer,subWidth,subHeight,i);

         SAFE_RELEASE(tableBuffer);

     }

     SAFE_RELEASE(bufferINT);
     SAFE_RELEASE(buffer8);
 }


int ImageSpliter::OtsuThresholdRAW(unsigned char *Buffer, int Width , int Height)
{

     int i ,k;
     int Threshold = 128;
     int sum = Width * Height;
     int vmin , vmax;
     double wSum = 0.0;  //double
     double wSumTemp = 0.0; //double
     int t1 = 0 , t2 = 0;
     double m1 =0.0 , m2 = 0.0;//double
     double varianceTemp , variance;//double

     int Histogram[256];
     memset(Histogram,0,sizeof(int)*256);

     vmin = 255;
     vmax = 0;
     for( i = 0 ; i < Height * Width ; i++ )
         {
             Histogram[Buffer[i]]++;

             if(Buffer[i]>vmax)
                 vmax = Buffer[i];
             if(Buffer[i]< vmin)
                 vmin = Buffer[i];
         }


     for ( i = 0; i <= 255; i++)
     {
         wSum += (double) i *  (double)Histogram[i];

     }

     varianceTemp = -10.0;

     for( i = vmin ; i < vmax ; i++ )
     {
         //²Î­pt­È¥ª¥k¨âÃäªºÁ`­È
         t1 += Histogram[i];
         t2 = sum - t1;

         wSumTemp += (double) i * Histogram[i];


         if( t1 == 0 || t2 == 0 )
         {
             for( k = 0 ; k < Width * Height ; k++ )
             {
                 if( Buffer[k] > 128 )
                     Buffer[k] = 255;
                 else
                     Buffer[k] = 0;
             }

             return 128;
         }
         //¨âÃäªº½è¶q¤¤¤ß
         m1 = wSumTemp / t1;
         m2 = (wSum - wSumTemp) / t2;

         variance = (double) t1 *  (double) t2 * (m1 - m2) * (m1 - m2);

         if( variance > varianceTemp )
         {
             varianceTemp = variance;
             Threshold = i;
         }
     }

     for( i = 0 ; i < Width * Height ; i++ )
     {
         if( Buffer[i] > Threshold )
             Buffer[i] = 255;
         else
             Buffer[i] = 0;
     }


     return Threshold;
}
