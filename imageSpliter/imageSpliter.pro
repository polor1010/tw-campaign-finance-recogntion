#-------------------------------------------------
#
# Project created by QtCreator 2014-05-17T00:38:18
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG-=app_bundle
TARGET = imageSpliter
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    ../imageprocess.cpp \
    imagespliter.cpp

HEADERS  += mainwindow.h \
    ../imageprocess.h \
    imagespliter.h

FORMS    += mainwindow.ui
