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


RegionRecognition::RegionRecognition()
{
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
        content = "{\"recognitions\":[";

        for( i = 0 ; i < tableRegions.size(); i++ )//tableRegions.size()
        {
            content += "{";
            content += "\"row\":" + QString::number(tableRegions.at(i).row) + ",";
            content += "\"column\":" + QString::number(tableRegions.at(i).column) + ",";
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
    //QJsonObject imageFile = root.value(QString("file")).toObject();
    //int width = imageFile.value(QString("pic_width")).toInt();
    //int height = imageFile.value(QString("pic_height")).toInt();

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
        return false;
    }

    QImage image;
    if( !image.load(filePath) )
    {
        qDebug() << "image load error";
        return false;
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

        //只辨識 row 0:序號 1:日期 6:支出金額
        if( row > 1  && (column == 1 || column == 2 || column == 7))
        {
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

            if( patterns.size() > 0 )
                tableRegions.at(i).result = processImage(regionImage);

#ifdef DEBUG_SAVE_IMAGE
            static int counter = 0;

            drawInfo(regionImage);
            QPainter painter;
            painter.begin(&regionImage);
            painter.setPen(QPen(QColor(Qt::red)));
            painter.drawText(10,15,tableRegions.at(i).result);
            painter.end();

            QString filePath = QDir::currentPath() + "/"+QString::number(counter) + QString::number(row)+QString::number(column)+".jpg";
            qDebug() << filePath;
            regionImage.save(filePath);
            counter++;
#endif
        }
    }

    return true;
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
             trans2Image(patternBuffer,PATTERN_WIDTH,PATTERN_HEIGHT);

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
 QString RegionRecognition::processImage(QImage image)
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
     noiseRemove(buffer8,width,height);
     noiseRemove(buffer8,width,height);
     getMaxRegion(buffer8,width,height);

     charPositions = getCharPosition(buffer8,width,height);
     //qDebug() << "char size" << charPositions.size();
     trans2RGB(buffer8,width,height,widthEff,image.bits());

     getRecognitions(buffer8,width);

     SAFE_RELEASE(buffer8);


     return sortCharPosition();
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
