#include "regionrecognition.h"
#include <QImage>
#include <QPainter>
#include <QDebug>
#include <QDir>
#include <QVariant>
#include <QJsonValue>
#include <QJsonDocument>
#include <QJsonObject>
#include <QVariantMap>
#include <QJsonArray>
#include <math.h>


RegionRecognition::RegionRecognition()
{
    skew = 0.0;
    isDeskew = true;
    isMergeImage = false;
    isDenoise = false;
    isInverse = false;
    isLoadDataBase = loadPatterns();
}
/**
     * 輸出 JSON FILE
     *
     * @param QString outFilePath
     * @return void
     */
void RegionRecognition::writeJsonFile(QString outFilePath)
{
    QFile file(outFilePath);

    //qDebug()<<"writeJson";

    int i;
    QString content;
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)){
        //file.write("{\"recognitions\":[");

        content += "{\"theta\":"+QString::number(skew) +",";

        if(isInverse)
            content += "\"inverse\":1,";
        else
            content += "\"inverse\":0,";

        content += "\"recognitions\":[";

        for( i = 0 ; i < tableRegions.size(); i++ )
        {
            content += "{";
            content += "\"row\":" + QString::number(tableRegions.at(i).row) + ",";
            content += "\"column\":" + QString::number(tableRegions.at(i).column) + ",";
            content += "\"left_top\":[" + QString::number(tableRegions.at(i).left) +","+ QString::number(tableRegions.at(i).top) + "],";
            content += "\"right_down\":[" + QString::number(tableRegions.at(i).right) +","+ QString::number(tableRegions.at(i).bottom) + "],";
            content += "\"result\":\"" +tableRegions.at(i).result+"\"";

            if( i < tableRegions.size()-1 )//tableRegions.size() -1 )
                content += "},";
            else
                content += "}";
        }

        content += "]}";

        //qDebug() << content ;
        file.write(content.toUtf8());
    }
    file.close();
    tableRegions.clear();
}

/**
     * 讀取 JSON file 並用 tableRegions 記錄
     *
     * @param QString filePath
     * @return bool 是否讀取成功
     */
bool RegionRecognition::readJsonFile(QString filePath)
{
    tableRegions.clear();

    QFile file(filePath);
    if(!file.exists()){
        qDebug() << "load jason file error"<< filePath;
        return false;
    }

    QString content;
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)){
        QTextStream stream(&file);

        content = file.readAll();
    }
    file.close();

    QJsonObject conetentAll = QJsonDocument::fromJson(content.toUtf8()).object();

    QJsonObject root = conetentAll.value(QString("data")).toObject();
    //qDebug() <<"parser " << root;
    QJsonObject imageFile = root.value(QString("meta")).toObject();
    //qDebug() << imageFile;
    //int width = imageFile.value(QString("pic_width")).toInt();
    //int height = imageFile.value(QString("pic_height")).toInt();
    //qDebug() << imageFile.value(QString("pic_height")).toString();
    //qDebug() << imageFile.value(QString("reverse")).toInt();
    isInverse = imageFile.value(QString("reverse")).toInt();
    //qDebug() << "isInverse" << isInverse;


    QJsonArray table = root.value(QString("tables")).toArray();

    //qDebug() << table[0];

    int y,x;
    for( y = 0 ; y < table.size() ; y++ )
    {
        QJsonArray rows = table[y].toArray();
        for( x = 0 ; x < rows.size() ; x++ )
        {
            TABLE_REGION region;
            region.left = rows[x].toObject().value(QString("left_top")).toArray()[0].toInt();
            region.top = rows[x].toObject().value(QString("left_top")).toArray()[1].toInt();
            region.right = rows[x].toObject().value(QString("right_down")).toArray()[0].toInt();
            region.bottom = rows[x].toObject().value(QString("right_down")).toArray()[1].toInt();
            region.column = x+1;
            region.row = y+1;
            //qDebug() << region.left << region.top ;
            tableRegions.push_back(region);
        }
    }

    if( table.size() == 0 )
    {
        qDebug() << "parser json file error " << filePath;
        return false;
    }
    else
    {
        return true;
    }
}

void RegionRecognition::calibrateDateResult(QString &date)
{
    int size = date.size();
    if( date.at(size-3) != '/' )
    {
        date.insert(size-2,'/');
    }

    size = date.size();
    if( date.at(size-6) != '/' )
    {
        date.insert(size-5,'/');
    }

}

/**
     * 讀取 image file 並根據 tableRegions 記錄的位置做文字辨識 (目前只辨識 row 0:序號 1:日期 6:支出金額)
     *
     * @param QString filePath
     * @return bool 影像是否讀取成功
     */
bool RegionRecognition::getRegionRecognitions(QString filePath)
{
    if( !isLoadDataBase )
    {
        qDebug() << "database not load";
        return false;
    }

    QImage image;
    if( !image.load(filePath) )
    {
        qDebug() << "image load error";
        return false;
    }

    if( skew != 0.0 )
    {
        QMatrix matrix;
        matrix.rotate(-skew);
        image = image.transformed(matrix);
    }

    if( isInverse )
    {
        QMatrix matrix;
        matrix.rotate(180);
        image = image.transformed(matrix);
    }

    unsigned char *buffer24 = image.bits();

    int x,y,k,i;

    int widthEff = image.bytesPerLine();
    int bytePerPixel = widthEff / image.width();

    for( i = 0 ; i < tableRegions.size() ; i++ )
    {
        int width = abs(tableRegions.at(i).left-tableRegions.at(i).right) + 1;
        int height = abs(tableRegions.at(i).top-tableRegions.at(i).bottom) + 1;
        int posX = tableRegions.at(i).left;
        int posY = tableRegions.at(i).top;

        int column = tableRegions.at(i).column;
        int row = tableRegions.at(i).row;

        QImage regionImage = QImage(width,height,QImage::Format_ARGB32);
        unsigned char *regionBuffer = regionImage.bits();
        unsigned char *pRegionBuffer = regionBuffer;
        int regionWidthEff = regionImage.bytesPerLine();

        unsigned char *pBuffer24 = buffer24 + posY * widthEff + posX * bytePerPixel;
        for( y = 0 ; y < height ; y++ , pBuffer24 += widthEff , pRegionBuffer += regionWidthEff )
        {
            for( x = 0 , k = 0; x < width ; x++ , k += 4)
            {
                pRegionBuffer[k] =
                pRegionBuffer[k+1] =
                pRegionBuffer[k+2] = pBuffer24[x*bytePerPixel];
            }
        }

        //只辨識 row 0:序號 1:日期 6:支出金額
        if( row > 1  && (column == 1 || column == 2|| column == 6 || column == 7))
        //if( row > 1  && (column == 4))
        {
            if( patterns.size() > 0 )
            {
                tableRegions.at(i).result = processImage(regionImage);

                if( column == 2 )//處理日期辨識結果 有時候 '/' 會沒有辨識到
                    calibrateDateResult(tableRegions.at(i).result);
            }

#ifdef DEBUG_SAVE_IMAGE
            static int counter = 0;

            drawInfo(regionImage);
            QPainter painter;
            painter.begin(&regionImage);
            painter.setPen(QPen(QColor(Qt::red)));
            painter.drawText(10,15,tableRegions.at(i).result);
            painter.end();
/**/
            //QString::number(counter) +
            QString filePath = QDir::currentPath() + "/"+ QString::number(row)+"_"+QString::number(column)+".jpg";
            qDebug() << filePath;
            regionImage.save(filePath);
            counter++;
#endif
        }
        else
        {
            /*
            if( row > 1 )
            {
                QString filePath = QDir::currentPath() + "/"+ QString::number(row)+"_"+QString::number(column)+".jpg";
                qDebug() << filePath;
                regionImage.save(filePath);
            }
            */
        }
    }
/*
    image = image.convertToFormat(QImage::Format_ARGB32);
    drawTableRegionInfo(image);
    QString savePath = filePath.insert(filePath.size()-4,"_R");
    image.save(savePath);
    qDebug() << savePath;
*/

    return true;
}


void RegionRecognition::drawTableRegionInfo(QImage &image)
{
    int i;

    QPainter painter;
    painter.begin(&image);
    painter.setFont(QFont("Chicago", 18));
    for( i = 0 ; i < tableRegions.size() ; i++ )
    {
        int left = tableRegions.at(i).left;
        int top = tableRegions.at(i).top;
        int right = tableRegions.at(i).right;
        int bottom = tableRegions.at(i).bottom;
        int column = tableRegions.at(i).column;
        int row = tableRegions.at(i).row;
        QString str = QString("(%1,%2) %3").arg(column).arg(row).arg(tableRegions.at(i).result);
        painter.drawText( left , top - 16 , str );
        painter.drawLine( left , top , right , bottom );

    }
    painter.end();

}

/**
     * 將 buffer 轉成 QImage 並儲存
     *
     * @param unsigned char *buffer8
     * @param int   width
     * @param int   height
     * @return void
     */
void RegionRecognition::trans2Image(unsigned char *buffer8,int width,int height)
{
    static int counter = 0;
    QImage *charImage = new QImage(width,height,QImage::Format_ARGB32);
    unsigned char *pCharImageBuffer = charImage->bits();
    int charWidthEff = charImage->bytesPerLine();

    unsigned char *pBuffer8 = buffer8;

    int y,x,k;

    for( y = 0 ; y < height ; y++ , pCharImageBuffer += charWidthEff , pBuffer8 += width )
    {
        for( x = 0 , k = 0; x < width ; x++ , k += 4 )
        {
            pCharImageBuffer[k] = pCharImageBuffer[k+1] = pCharImageBuffer[k+2] = pBuffer8[x];
        }
    }

    //charImage->save(QDir::currentPath()+QString::number(counter)+".bmp","BMP");

    counter++;
}



/**
     * 根據 charPositions 得知影像中文字位置, 並作辨識
     *
     * @param unsigned char *buffer8
     * @param int   width
     * @return void
     */
 void RegionRecognition::getRecognitions(unsigned char *buffer8,int width)
 {
     int x,y,i;

     int charCounter = 0;
     for( i = 0 ; i < charPositions.size() ; i++ )
     {
         if( charPositions.at(i).IsRegion )
         {
             int left = charPositions.at(i).left;
             int right = charPositions.at(i).right;
             int top = charPositions.at(i).top;
             int bottom = charPositions.at(i).bottom;

             unsigned char *pBuffer8 = buffer8 + top * width + left;
             int subWidth = abs(left-right)+1;
             int subHeight = abs(top-bottom)+1;

             unsigned char *charBuffer = new unsigned char[subWidth*subHeight];
             memset(charBuffer,0,sizeof(unsigned char)*subWidth*subHeight);

             unsigned char *pCharBuffer = charBuffer;
             for( y = 0 ; y < subHeight ; y++ , pBuffer8 += width , pCharBuffer += subWidth )
             {
                 for( x = 0 ; x < subWidth ; x++ )
                 {
                     pCharBuffer[x] = pBuffer8[x];
                 }
             }

             unsigned char *patternBuffer = new unsigned char[PATTERN_WIDTH*PATTERN_HEIGHT];
             memset(patternBuffer,0,sizeof(unsigned char)*PATTERN_WIDTH*PATTERN_HEIGHT);

             float ratioX = (float)(subWidth ) / (PATTERN_WIDTH - 4);
             float ratioY = (float)(subHeight) / (PATTERN_HEIGHT - 4);

             unsigned char *pPatternBuffer = patternBuffer + PATTERN_WIDTH * 2 + 2;
             for( y = 0 ; y < PATTERN_HEIGHT - 4 ; y++ , pPatternBuffer += PATTERN_WIDTH )
             {
                 for( x = 0 ; x < PATTERN_WIDTH - 4 ; x++ )
                 {
                     int posX = x * ratioX;
                     int posY = y * ratioY;

                     //inverse for distance transform
                     pPatternBuffer[x] = 255 - charBuffer[posY * subWidth + posX];
                 }
             }

             unsigned char *distanceBuffer = new unsigned char[PATTERN_WIDTH*PATTERN_HEIGHT];
             memset(distanceBuffer,255,sizeof(unsigned char)*PATTERN_WIDTH*PATTERN_HEIGHT);
             chamferDistance(patternBuffer,PATTERN_WIDTH,PATTERN_HEIGHT,distanceBuffer,128,DISTANCE_MAX,DISTANCE_MIN);
             memcpy(patternBuffer,distanceBuffer,sizeof(unsigned char)*PATTERN_WIDTH*PATTERN_HEIGHT);
             trans2Image(charBuffer,subWidth,subHeight);

             float wordRatio = (float)subWidth/subHeight;
             if( wordRatio < 0.35 )
             {
                 charPositions.at(i).result = '1';
             }
             else
             {
                 charPositions.at(i).result = recognition(patternBuffer);
             }

             //qDebug() << "result " << charPositions.at(i).result << "ratio " << (float)subWidth/subHeight;

#ifdef DEBUG_SAVE_IMAGE
             trans2Image(patternBuffer,PATTERN_WIDTH,PATTERN_HEIGHT);
#endif
             SAFE_RELEASE(charBuffer);
             SAFE_RELEASE(patternBuffer);

             charCounter++;
         }
     }

 }

 /**
      * 輸入 region 影像並作辨識
      *
      * @param QImage image
      * @return QString 辨識結果
      */
 QString RegionRecognition::processImageChinese(QImage &image)
 {
     charPositions.clear();
     //isSpace = false;

     int height = image.height();
     int width = image.width();
     int widthEff = image.bytesPerLine();
     //qDebug() << widthEff << width;
     unsigned char *pBuffer = image.bits();

     unsigned char *buffer8 = new uchar[width*height];
     memset(buffer8,0,sizeof(uchar)*width*height);

     trans2Gray(pBuffer,width,height,widthEff,buffer8);

     if(isDenoise)
     {
         noiseRemove(buffer8,width,height);
         noiseRemove(buffer8,width,height);
     }

     getWordBounderH(buffer8,width,height);
     //charPositions = getCharPosition(buffer8,width,height);
     //qDebug() << "char size" << charPositions.size();
     //trans2RGB(buffer8,width,height,widthEff,image.bits());

     //getRecognitions(buffer8,width);

     SAFE_RELEASE(buffer8);

     return sortCharPosition();
 }

 /**
      * 輸入 region 影像並作辨識
      *
      * @param QImage image
      * @return QString 辨識結果
      */
 QString RegionRecognition::processImage(QImage &image)
 {
     charPositions.clear();
     //isSpace = false;

     int height = image.height();
     int width = image.width();
     int widthEff = image.bytesPerLine();

     unsigned char *pBuffer = image.bits();

     unsigned char *buffer8 = new uchar[width*height];
     memset(buffer8,0,sizeof(uchar)*width*height);

     trans2Gray(pBuffer,width,height,widthEff,buffer8);

     if(isDenoise)
     {
        noiseRemove(buffer8,width,height);
        noiseRemove(buffer8,width,height);
     }

     getMaxRegion(buffer8,width,height);

     int *horizontal = new int[width];
     //memset(horizontal,0,sizeof(int)*width);

     int top,bottom;
     charPositions = getCharPosition(buffer8,width,height,horizontal,top,bottom);
     //qDebug() << "char size" << charPositions.size();
     trans2RGB(buffer8,width,height,widthEff,image.bits());
/*
     int x,y,k;
     for( y = 0 ; y < height ; y++ , pBuffer += widthEff )
     {
         for( x = 0 , k = 0 ; x < width ; x++ , k += 4 )
         {
             if( horizontal[x] == 0 || y == top || y == bottom)//
             //if(  y == top || y == bottom)//horizontal[x] == 0 ||

             {
                 pBuffer[k] = 0;
                 pBuffer[k+1] = 0;
                 pBuffer[k+2] = 255;

             }

         }
     }
*/
     getRecognitions(buffer8,width);

     SAFE_RELEASE(buffer8);


     return sortCharPosition();
 }

 /**
      * 計算影像的歪斜角度,並校正
      *
      * @param QImage image
      * @return void
      */
 void RegionRecognition::skewCalibration(QImage &image)
 {
     int height = image.height();
     int width = image.width();
     int widthEff = image.bytesPerLine();

     unsigned char *pBuffer = image.bits();

     unsigned char *buffer8 = new unsigned char[width*height];
     memset(buffer8,0,sizeof(unsigned char)*width*height);

     trans2Gray(pBuffer,width,height,widthEff,buffer8);

     unsigned char *bufferHorizontal8 = new unsigned char[width*height];
     memcpy(bufferHorizontal8,buffer8,sizeof(unsigned char)*width*height);

     vector<TABLE_LINE> horizontalLines;
     skew = horizontalLineFilter(bufferHorizontal8,width,height,horizontalLines);

     qDebug() << "width/height: " << image.width() << image.height() << "skew "<< skew;

     QMatrix matrix;
     matrix.rotate(-skew);
     image = image.transformed(matrix);
#ifdef DEBUG_SAVE_IMAGE
     image.save("skewCalibration.jpg");
#endif
     //trans2RGB(bufferHorizontal8,width,height,widthEff,image.bits());

     SAFE_RELEASE(bufferHorizontal8);
     SAFE_RELEASE(buffer8);
 }


/**
     * 擷取 table 中的每個欄位位置
     *
     * @param QImage image
     * @return void
     */
bool RegionRecognition::getRegionPosition(QImage &image)
{
    int i;
    int height = image.height();
    int width = image.width();
    int widthEff = image.bytesPerLine();

    unsigned char *pBuffer = image.bits();

    unsigned char *buffer8 = new unsigned char[width*height];
    memset(buffer8,255,sizeof(unsigned char)*width*height);

    trans2Gray(pBuffer,width,height,widthEff,buffer8);

    int *bufferINT = new int[width*height];
    memset(bufferINT,0,sizeof(int)*width*height);

    for( i = 0 ; i < width * height ; i++ )
    {
        bufferINT[i] = buffer8[i];
    }

    vector<REGION_ENTRY> regions = connectedComponent(bufferINT,width,height,50);
    std::sort(regions.begin(), regions.end(),regionCompareSize);

    if( regions.size() > 1 )
    {
        vector<Point> LeftTopPoints;
        getCornerPoints( regions , LeftTopPoints , LEFT_TOP );
        sortCorner(LeftTopPoints);

        vector<Point> RightBottomPoints;
        getCornerPoints( regions , RightBottomPoints , RIGHT_BOTTOM );
        sortCorner(RightBottomPoints);

        int rbSize = RightBottomPoints.size();
        int ltSize = LeftTopPoints.size();
        isInverse = false;
        if( (rbSize == ltSize)  && (ltSize > 0) )
        {
            int top = LeftTopPoints.at(0).y;
            int bottom = RightBottomPoints.at(RightBottomPoints.size()-1).y;
            if( rbSize < 189 )
            {
                 if( top > (height - bottom) )
                 {
                     top = (height - bottom) ;
                 }
                 else
                 {
                     bottom = height - top;
                 }
            }

            isInverse = isReverse( buffer8,width,height,top,bottom);

            qDebug() << "isInverse:" << isInverse;// << top << bottom << top << height-bottom;

            if( isInverse )
            {
                QMatrix matrix;
                matrix.rotate(180);
                image = image.transformed(matrix);
                reverseCornerPoints(LeftTopPoints,RightBottomPoints,width,height);
            }

            for( i = 0 ; i < rbSize ; i++ )
            {
                TABLE_REGION region;
                region.left = LeftTopPoints.at(i).x;
                region.top = LeftTopPoints.at(i).y;
                region.right = RightBottomPoints.at(i).x;
                region.bottom = RightBottomPoints.at(i).y;
                region.column = (i) % TABLE_COLUMN + 1;
                region.row = (i) / TABLE_COLUMN + 1;
                tableRegions.push_back(region);
            }
/*
            for( i = 0 ; i < width * height ; i++ )
            {
                buffer8[i] = bufferINT[i] ;
            }
            trans2RGB(buffer8,width,height,widthEff,image.bits());
            image.save("result.jpg");
*/
        }
        else
        {
            qDebug() << "get corner point fail.." << "getLeftTopPoints: "<<ltSize<<"getRightBottomPoints: "<<rbSize;
        }
    }
    else
    {
        qDebug() << "get region error, region size = " << regions.size();
    }
    //qDebug() << "finish getRegions";

    SAFE_RELEASE(bufferINT);
    SAFE_RELEASE(buffer8);

    return true;
}




/**
     * 移除 table 中的文字
     *
     * @param QImage image
     * @return void
     */
void RegionRecognition::removeRegionChar(QImage &image)
{
      int x,y,i;
      int height = image.height();
      int width = image.width();
      int widthEff = image.bytesPerLine();

      unsigned char *pBuffer = image.bits();

      unsigned char *buffer8 = new unsigned char[width*height];
      memset(buffer8,255,sizeof(unsigned char)*width*height);

      trans2Gray(pBuffer,width,height,widthEff,buffer8);

      int *bufferINT = new int[width*height];
      memset(bufferINT,0,sizeof(int)*width*height);

      for( i = 0 ; i < width * height ; i++ )
      {
          bufferINT[i] = buffer8[i];
      }

      vector<REGION_ENTRY> regions = connectedComponent(bufferINT,width,height,50);

      for( i = 0 ; i < regions.size() ; i++ )
      {
          int startX = regions.at(i).left;
          int startY = regions.at(i).top;
          int subWidth = abs(regions.at(i).left-regions.at(i).right)+1;
          int subHeight = abs(regions.at(i).top-regions.at(i).bottom)+1;

          if( subWidth < width / 2 )
          {
              unsigned char *pBuffer8 = buffer8 + ( startY + subHeight / 5)* width + startX;

              for( y =  subHeight / 6 ; y < subHeight -  subHeight / 6 ; y++ , pBuffer8 += width )
              {
                  for( x = 0 ; x < subWidth ; x++ )
                  {
                      pBuffer8[x] = 255;
                  }
              }
          }
      }

      SAFE_RELEASE(bufferINT);
      SAFE_RELEASE(buffer8);
}

/**
     * 將 table 的左上右下的點座標, 做 inverse
     *
     * @param vector<Point> &ltPoints
     * @param vector<Point> &rbPoints
     * @param int width
     * @param int height
     * @return QString 辨識結果
     */
void RegionRecognition::reverseCornerPoints(vector<Point> &ltPoints, vector<Point> &rbPoints, int width, int height)
{
    int i;

    vector<Point> ltPointsTemp;
    vector<Point> rbPointsTemp;

    for( i = 0 ; i < ltPoints.size() ; i++ )
    {
        ltPointsTemp.push_back( ltPoints.at(ltPoints.size()-i-1) );
        rbPointsTemp.push_back( rbPoints.at(rbPoints.size()-i-1) );
    }

    for( i = 0 ; i < ltPoints.size() ; i++ )
    {
        rbPoints.at(i).x = width - ltPointsTemp.at(i).x;
        rbPoints.at(i).y = height - ltPointsTemp.at(i).y;

        ltPoints.at(i).x = width - rbPointsTemp.at(i).x;
        ltPoints.at(i).y = height - rbPointsTemp.at(i).y;
    }

}

/**
     * 畫偵測到的欄位位置,(左上點畫到右下,有些圖bbp只有1所以沒有設定顏色)
     *
     * @param QImage &image
     * @param vector<Point> &ltPoints
     * @param vector<Point> &rbPoints
     * @return void
     */
void RegionRecognition::drawRegionInfo(QImage &image , vector<Point> &ltPoints, vector<Point> &rbPoints)
{
    int i;

    QPainter painter;
    painter.begin(&image);
    //painter.setPen(QPen(QBrush(),5));

    if( ltPoints.size() == rbPoints.size() )
    {
        for( i = 0 ; i <  ltPoints.size() ; i++ )
        {
            int column = (i) % TABLE_COLUMN + 1;
            int row = (i) / TABLE_COLUMN + 1;
            QString str = QString("(%1,%2)").arg(column).arg(row);
            painter.drawText( ltPoints.at(i).x ,ltPoints.at(i).y ,str );
            painter.drawLine( ltPoints.at(i).x ,ltPoints.at(i).y , rbPoints.at(i).x , rbPoints.at(i).y );
        }
    }
    painter.end();

}

/**
     * 先做 line detection(垂直水平分開做) 再計算 cross points
     *
     * @param QImage &image
     * @return void
     */
void RegionRecognition::getLineCrossPoints(QImage &image)
{
    int i,j;
    int height = image.height();
    int width = image.width();
    int widthEff = image.bytesPerLine();

    unsigned char *pBuffer = image.bits();

    unsigned char *buffer8 = new unsigned char[width*height];
    memset(buffer8,255,sizeof(unsigned char)*width*height);

    trans2Gray(pBuffer,width,height,widthEff,buffer8);

    unsigned char *bufferVertical8 = new unsigned char[width*height];
    memcpy(bufferVertical8,buffer8,sizeof(unsigned char)*width*height);

    unsigned char *bufferHorizontal8 = new unsigned char[width*height];
    memcpy(bufferHorizontal8,buffer8,sizeof(unsigned char)*width*height);

    vector<TABLE_LINE> verticalLines;
    vector<TABLE_LINE> horizontalLines;

    vertivalLineFilter(bufferVertical8,width,height,verticalLines);
    //horizontalLineFilter(bufferHorizontal8,width,height,horizontalLines);

    qDebug() << "filter finish..";


    vector<Point> crossPoints;
    for( i = 0 ; i < verticalLines.size() ; i++ )
    {
        for( j = 0 ; j < horizontalLines.size() ; j++ )
        {
            if( fabs(verticalLines.at(i).a) < 0.2 && fabs(horizontalLines.at(j).a) < 0.2 )
            {
                Point point;
                point.x = verticalLines.at(i).b;
                point.y = horizontalLines.at(j).b;

                if( point.x > 0 && point.x < width && point.y > 0 && point.y < height )
                {
                    crossPoints.push_back(point);
                   // qDebug() << "CrossPoints : " << point.x << point.y << horizontalLines.at(j).a << horizontalLines.at(j).b;
                    //qDebug() << verticalLines.at(i).a << verticalLines.at(i).b;
                 }
            }
        }
    }

     memset(buffer8,0,sizeof(unsigned char)*width*height);

     for( i = 0 ; i < crossPoints.size() ; i++ )
     {
         int x = crossPoints.at(i).x;
         int y = crossPoints.at(i).y;

         buffer8[y*width+x] = 255;
     }


     for( i  = 0 ; i < width * height ; i++ )
     {
         int diff = abs(bufferVertical8[i] - bufferHorizontal8[i]);
         if( diff == 0 )
         {
             //bufferVertical8[i] = bufferHorizontal8[i] = 0;
         }

         buffer8[i] = (bufferVertical8[i]);// | bufferHorizontal8[i]) ;
     }

     trans2RGB(buffer8,width,height,widthEff,image.bits());

     SAFE_RELEASE(bufferVertical8);
     SAFE_RELEASE(bufferHorizontal8);
     SAFE_RELEASE(buffer8);

     image.save("test3.jpg");
}

/**
     * 先做影像角度校正, 再找出欄位的左上和右下的座標點
     *
     * @param QImage &image
     * @return void
     */
bool RegionRecognition::getRegions(QString filePath)
{
    QImage image;
    if( !image.load(filePath) )
    {
        qDebug() << "image load error";
        return false;
    }

    if( isDeskew )
        skewCalibration(image);

    return getRegionPosition(image);
}


 /**
      * 將 charPositions 裡的文字從左到右排序, 並回傳結果
      *
      * @return QString 文字排序結果
      */
 QString RegionRecognition::sortCharPosition()
 {
     int i,j;

     QString result;
     for( i = 0 ; i < charPositions.size() ; i++ )
     {
         for( j = i ; j < charPositions.size() ; j++ )
         {
             if( charPositions.at(i).left > charPositions.at(j).left )
             {
                 REGION_ENTRY region = charPositions.at(i);
                 charPositions.at(i) = charPositions.at(j);
                 charPositions.at(j) = region;
             }
         }
     }

     for( i = 0 ; i < charPositions.size() ; i++ )
     {
         if( charPositions.at(i).IsRegion )
         {
             if( charPositions.at(i).result == 'a')
             {
                 result += '/';
             }
             else
             {
                result += charPositions.at(i).result;
             }
         }
     }
     if( charPositions.size() == 0 )
     {
         result = "none";
     }

     return result;
 }

 /**
      * 將辨識資訊(文字位置,辨識結果)畫到影像中
      *
      * @param QImage image
      * @return void
      */
 void RegionRecognition::drawInfo(QImage &image)
 {
     int i;

     QPainter painter;
     painter.begin(&image);
     //painter.drawImage(0,0,image);
     painter.setPen(QPen(QColor(Qt::red)));

     if( charPositions.size() == 0 )
     {
         painter.drawText(20,30,"none");
     }

     for( i = 0 ; i <  charPositions.size() ; i++ )
     {
         if( charPositions.at(i).IsRegion )
         {
             painter.drawRect(QRect(QPoint( charPositions[i].left,charPositions[i].top),QPoint( charPositions[i].right,charPositions[i].bottom)));
             QString result(charPositions[i].result);
             if(result == "a")
                 result = "/";
             painter.drawText( charPositions[i].left,charPositions[i].top,result);
         }
         //charPositions.at(i).result;
     }//p.drawRect(QRect(3,3,50,50));
     painter.end();

 }

 /**
      * 讀取辨識資料庫的圖, 並將 buffer 存到 patterns 裡
      *
      * @return bool 是否讀取成功
      */
 bool RegionRecognition::loadPatterns()
 {
     QDir dir;
     QDir::current().path();
     //qDebug() <<  QDir::current().path()+"/words/";
     dir.setPath( QDir::currentPath() + "/database/");
     dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);

     QStringList nameFilter;
     nameFilter << "*.png" << "*.jpg" << "*.gif"<< "*.bmp";
     QFileInfoList list = dir.entryInfoList(nameFilter, QDir::Files);
     //qDebug()<<"Bytes Filename";

     for (int i = 0; i < list.size(); ++i)  {
         QFileInfo fileInfo = list.at(i);
         //filePath = ;
         QImage image;
         image.load(fileInfo.filePath());

         setPattern(image.bits(),image.width(),image.height(),image.bytesPerLine(),fileInfo.fileName().at(0).unicode());
         //processImage();
         //int fileNameLength = fileInfo.filePath().length();
         //qDebug()<<fileInfo.filePath().insert(fileNameLength-4,"_R");
     }
     if( list.size() > 0 )
     {
        //qDebug() << "load database";
        return true;
     }
     else
     {
         qDebug() << "load database fail (path?)";
         return false;
     }
 }

 /**
      * 辨識文字, （buffer8 都必須為 PATTERN_HEIGHT * PATTERN_WIDTH 大小, 否則會出錯）
      *
      * @param unsigned char *buffer8
      * @return char 辨識結果
      */
 char RegionRecognition::recognition(unsigned char *buffer8)
 {
     int i,y,x;
     int sumTemp = PATTERN_WIDTH * PATTERN_HEIGHT * 255;
     int index = -1;
     int diffSum = 0;

     for( i = 0 ; i < patterns.size() ; i++ )
     {
         unsigned char *pBuffer8 = buffer8;
         unsigned char *pPatternBuffer8 = patterns.at(i).buffer8;

         diffSum = 0;
         for( y = 0 ; y < PATTERN_HEIGHT ; y++ , pPatternBuffer8 += PATTERN_WIDTH , pBuffer8 += PATTERN_WIDTH )
         {
             for( x = 0 ; x < PATTERN_WIDTH ; x++ )
             {
                 diffSum += abs(pPatternBuffer8[x]-pBuffer8[x]);
             }
         }

         if( diffSum < sumTemp )
         {
             sumTemp = diffSum;
             index = i;
         }
         //qDebug() << diffSum;
     }

     return patterns.at(index).word;
 }

 /**
      * 將影像做 charmer distance transform 並存到 patterns 裡
      *
      * @param unsigned char *buffer32
      * @param int   width
      * @param int   height
      * @param int   widthEff
      * @param char word
      * @return void
      */
 void RegionRecognition::setPattern(unsigned char *buffer32,int width,int height,int widthEff,char word)
 {
     OCR_PATTERN pattern;
     pattern.word = word;
     pattern.buffer8 = new unsigned char[PATTERN_WIDTH*PATTERN_HEIGHT];
     unsigned char *distanceBuffer = new unsigned char[PATTERN_WIDTH*PATTERN_HEIGHT];

     unsigned char *pBuffer32 = buffer32;
     unsigned char *pBuffer8 = pattern.buffer8;
     int x,y,k;

     for( y = 0 ; y < height ; y++ , pBuffer32 += widthEff , pBuffer8 += PATTERN_WIDTH )
     {
         for( x = 0 , k = 0 ; x < width ; x++ , k += 4 )
         {
             //inverse for distance transform
             pBuffer8[x] = 255 - pBuffer32[k];
         }
     }

     chamferDistance(pattern.buffer8,PATTERN_WIDTH,PATTERN_HEIGHT,distanceBuffer,128,DISTANCE_MAX,DISTANCE_MIN);
     memcpy(pattern.buffer8,distanceBuffer,sizeof(unsigned char)*PATTERN_WIDTH*PATTERN_HEIGHT);

     patterns.push_back(pattern);

     SAFE_RELEASE(distanceBuffer);

     //qDebug() << "patterns add." << patterns.size() << patterns.at(patterns.size()-1).word ;
 }

 void RegionRecognition::saveWord()
 {


 }
