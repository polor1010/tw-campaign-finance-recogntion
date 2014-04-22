#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QImage>
#include <QPainter>
#include <QDebug>
#include <QDir>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    isloadImage = false;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

    if(isloadImage)
    {
        painter.drawImage(0,0,image);
    }

}

void MainWindow::openImage()
{
    filePath = QFileDialog::getOpenFileName(this, tr("Open File"),
                                                     "",
                                                     tr("Files (*.*)"));
    QMessageBox qmb;
    qmb.setText(filePath);
    //qmb.exec();

    image.load(filePath);
    isloadImage = true;

    qDebug() << image.width() << image.height();
    qDebug() << image.bytesPerLine();
    qDebug() << image.bitPlaneCount();

    update();

}



void MainWindow::processImage()
{
    int i;
    int height = image.height();
    int width = image.width();
    int widthEff = image.bytesPerLine();

    uchar *pBuffer = image.bits();

    uchar *buffer8 = new uchar[width*height];
    memset(buffer8,0,sizeof(uchar)*width*height);

    trans2Gray(pBuffer,width,height,widthEff,buffer8);
    noiseRemove(buffer8,width,height);
    noiseRemove(buffer8,width,height);
    getMaxRegion(buffer8,width,height);
    charPositions = getCharPosition(buffer8,width,height);
    qDebug() << "char"<< charPositions.size();
    trans2RGB(buffer8,width,height,widthEff,image.bits());

    QPainter painter;
    painter.begin(&image);
    painter.setPen(QPen(QColor(Qt::red)));
    //p.setBrush(QBrush(QColor(Qt::color0), Qt::NoBrush));

    for( i = 0 ; i <  charPositions.size() ; i++ )
    {
        painter.drawRect(QRect(QPoint( charPositions[i].left,charPositions[i].top),QPoint( charPositions[i].right,charPositions[i].bottom)));
    }//p.drawRect(QRect(3,3,50,50));
    painter.end();


    if(buffer8)
        delete []buffer8;

    int fileNameLength = filePath.length();
    qDebug()<<filePath.insert(fileNameLength-4,"_R");
    image.save(filePath);
    update();
}

void MainWindow::getFolderFiles()
{

    QString fileName = QFileDialog::getExistingDirectory();;// = dialog.getOpenFileName();
    qDebug()<<fileName;

    QDir dir;
    dir.setPath(fileName);
    dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);

    QStringList nameFilter;
    nameFilter << "*.png" << "*.jpg" << "*.gif";
    QFileInfoList list = dir.entryInfoList(nameFilter, QDir::Files);
    //qDebug()<<"Bytes Filename";

    for (int i = 0; i < list.size(); ++i)  {
        QFileInfo fileInfo = list.at(i);
        filePath = fileInfo.filePath();
        image.load(filePath);

        processImage();
        //int fileNameLength = fileInfo.filePath().length();
        //qDebug()<<fileInfo.filePath().insert(fileNameLength-4,"_R");
    }

}
