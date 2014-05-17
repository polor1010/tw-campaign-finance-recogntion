#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QPainter>
#include <QDebug>
#include <QImage>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    isLoadImage = true;
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

    if(isLoadImage)
    {
        painter.drawImage(0,20,image);
    }
}

void MainWindow::getLines()
{

    filePath = QFileDialog::getOpenFileName(this, tr("Open File"),
                                                     "",
                                                     tr("Files (*.*)"));

    regionRecognition.setDeskew(true);
    regionRecognition.getRegions(filePath);
    regionRecognition.getRegionRecognitions(filePath);

    int fileNameLength = filePath.length();
    QString savePath = filePath.replace(fileNameLength-4,4,".json");
    qDebug() << savePath;
    regionRecognition.writeJsonFile(savePath);

    update();
}



void MainWindow::processImage()
{

    filePath = QFileDialog::getOpenFileName(this, tr("Open Image"),
                                                     "",
                                                     tr("Files (*.*)"));

    jSonFilePath = QFileDialog::getOpenFileName(this, tr("Open Json File"),
                                                     "",
                                                     tr("Files (*.*)"));


    regionRecognition.readJsonFile(jSonFilePath);
    regionRecognition.getRegionRecognitions(filePath);
    regionRecognition.writeJsonFile("result.json");

    image.load(filePath);

    qDebug() << filePath << jSonFilePath;


}


void MainWindow::processRegionImage()
{
    filePath = QFileDialog::getOpenFileName(this, tr("Open File"),
                                                     "",
                                                     tr("Files (*.*)"));
    QMessageBox qmb;
    qmb.setText(filePath);
    //qmb.exec();

    image.load(filePath);
    isLoadImage = true;

    qDebug() << image.width() << image.height();
    qDebug() << image.bytesPerLine();
    qDebug() << image.bitPlaneCount();

    regionRecognition.processImageChinese(image);

    regionRecognition.drawInfo(image);
    //int fileNameLength = filePath.length();
    //qDebug()<<filePath.insert(fileNameLength-4,"_R");
    //image.save(filePath);

    //saveRegions();
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

    regionRecognition.setDeskew(true);

    for (int i = 0; i < list.size(); ++i)  {
        QFileInfo fileInfo = list.at(i);
        filePath = fileInfo.filePath();
        //image.load(filePath);

        regionRecognition.getRegions(filePath);
        regionRecognition.getRegionRecognitions(filePath);

        int fileNameLength = filePath.length();
        QString savePath = filePath.replace(fileNameLength-4,4,".json");
        qDebug() << savePath;
        regionRecognition.writeJsonFile(savePath);
    }

}
