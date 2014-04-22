#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include "imageprocess.h"

using namespace std;

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
    bool isloadImage;
    QImage image;
    vector<REGION_ENTRY> charPositions;

public slots:
    void openImage();
    void processImage();
    void getFolderFiles();
};



#endif // MAINWINDOW_H
