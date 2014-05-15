#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "regionrecognition.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void paintEvent(QPaintEvent *);

private:
    Ui::MainWindow *ui;

    QString filePath;
    QString jSonFilePath;
    bool isLoadImage;
    QImage image;
    RegionRecognition regionRecognition;

public slots:
    void processImage();
    void processRegionImage();
    void getFolderFiles();
    void getLines();

};



#endif // MAINWINDOW_H
