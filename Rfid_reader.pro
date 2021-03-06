#-------------------------------------------------
#
# Project created by QtCreator 2018-05-11T22:30:27
#
#-------------------------------------------------

QT       += core gui network sql

CONFIG += console

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Rfid_reader
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    rfid_impinj.cpp \
    logwindow.cpp

HEADERS  += mainwindow.h \
    rfid_impinj.h \
    rfid_impinj_cmd.h \
    logwindow.h

FORMS    += mainwindow.ui

RESOURCES += \
    rfid.qrc

RC_ICONS = finishline.ico
