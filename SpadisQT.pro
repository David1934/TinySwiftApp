#-------------------------------------------------
#
# Project created by QtCreator 2020-05-08T12:37:46
#
#-------------------------------------------------

QT += core gui
# CONFIG+= console
CONFIG += debug
QMAKE_CXXFLAGS += -g
QMAKE_LFLAGS += -rdynamic
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = SpadisQT
TEMPLATE = app

DEFINES += RUN_ON_ROCKCHIP

SOURCES +=\
        globalapplication.cpp \
        main.cpp \
        mainwindow.cpp \
        FrameProcessThread.cpp \
        utils.cpp \
        v4l2.cpp

HEADERS  += mainwindow.h \
        common.h \
        depthmapwrapper.h \
        globalapplication.h \
        FrameProcessThread.h \
        utils.h \
        v4l2.h

contains(DEFINES, RUN_ON_ROCKCHIP) {
    DEFINES += RUN_ON_RK3568
    DEFINES += CONFIG_VIDEO_ADS6401
#    DEFINES += CONFIG_ADAPS_SWIFT_FLOOD

    QMAKE_LFLAGS += -Wl,-rpath,/vendor/lib64/
    SOURCES += adaps_dtof.cpp
    HEADERS  += adaps_dtof.h
    HEADERS  += rk-camera-module.h
    LIBS += -L$$PWD -ladaps_swift_decode
}

FORMS    += mainwindow.ui
