# 基础配置
QT       -= gui            # 禁用GUI模块
QT       += core  # 仅链接必要模块
CONFIG   += c++17 release  # 启用C++17和发布模式
TARGET    = SpadisQT_console   # 输出名称
TEMPLATE  = app

# 编译器优化标志（Linux/GCC示例）
QMAKE_CXXFLAGS_RELEASE += -O3 -flto -fno-exceptions
QMAKE_LFLAGS_RELEASE   += -flto -Wl,--gc-sections

DEFINES += RUN_ON_EMBEDDED_LINUX

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

LIBS += -lssl -lcrypto -lz

contains(DEFINES, RUN_ON_EMBEDDED_LINUX) {
    DEFINES += RUN_ON_RK3568
    DEFINES += CONFIG_VIDEO_ADS6401
#    DEFINES += CONFIG_ADAPS_SWIFT_FLOOD
#    DEFINES += ENABLE_DYNAMICALLY_UPDATE_ROI_SRAM_CONTENT
    DEFINES += NO_UI_APPLICATION

    QMAKE_LFLAGS += -Wl,-rpath,/vendor/lib64/
    SOURCES += adaps_dtof.cpp
    SOURCES += host_comm.cpp
    SOURCES += misc_device.cpp
    HEADERS += misc_device.h
    HEADERS += host_comm.h
    HEADERS  += adaps_dtof.h
    HEADERS += adaps_dtof_uapi.h
    LIBS += -L$$PWD -ladaps_swift_decode -lAdapsSender
}
