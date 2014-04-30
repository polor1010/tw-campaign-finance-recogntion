#-------------------------------------------------
#
# Project created by QtCreator 2014-04-20T00:05:51
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG-=app_bundle
TARGET = CharRecognition
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    imageprocess.cpp \
    regionrecognition.cpp

HEADERS  += mainwindow.h \
    imageprocess.h \
    regionrecognition.h

FORMS    += mainwindow.ui

OTHER_FILES +=
