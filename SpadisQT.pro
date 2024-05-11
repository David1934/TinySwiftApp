#-------------------------------------------------
#
# Project created by QtCreator 2020-05-08T12:37:46
#
#-------------------------------------------------

QT       += core gui
# CONFIG+= console
# CONFIG += debug
# QMAKE_CXXFLAGS += -g
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = SpadisQT
TEMPLATE = app

LIBS += -L$$PWD -ladaps_swift_decode
QMAKE_LFLAGS += -Wl,-rpath,/vendor/lib64/

SOURCES +=\
        globalapplication.cpp \
        main.cpp \
        mainwindow.cpp \
        adaps_dtof.cpp \
        majorimageprocessingthread.cpp \
        utils.cpp \
        v4l2.cpp

HEADERS  += mainwindow.h \
        adaps_dtof.h \
        common.h \
        depthmapwrapper.h \
        globalapplication.h \
        majorimageprocessingthread.h \
        rk-camera-module.h \
        v4l2.h

FORMS    += mainwindow.ui
