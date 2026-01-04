CROSS_COMPILE_DIR = /home/david/rk_build2025
CROSS_COMPILE = $(CROSS_COMPILE_DIR)/prebuilts/gcc/linux-x86/aarch64/gcc-arm-10.3-2021.07-x86_64-aarch64-none-linux-gnu/bin/aarch64-none-linux-gnu-

CC       := $(CROSS_COMPILE)gcc
CXX      := $(CROSS_COMPILE)g++

TARGET = TinySwiftApp
CXXFLAGS = -std=c++17 -O3 -flto -fno-exceptions -Wall -Wextra
LDFLAGS = -flto -Wl,--gc-sections

BASE_SRCS = \
    main.cpp \
    dtof_main.cpp \
    utils.cpp \
    adaps_dtof.cpp \
    misc_device.cpp \
    v4l2.cpp

BASE_HEADERS = \
    dtof_main.h \
    common.h \
    depthmapwrapper.h \
    utils.h \
    misc_device.h \
    adaps_dtof.h \
    adaps_dtof_uapi.h \
    v4l2.h

DEFINES += -DCONFIG_VIDEO_ADS6401 -DENABLE_POINTCLOUD_OUTPUT
LDFLAGS += -Wl,-rpath,/vendor/lib64/
LIBS += -L. -ladaps_swift_decode -lpthread

OBJS = $(BASE_SRCS:.cpp=.o)

CXXFLAGS += $(DEFINES)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS) $(LIBS)

%.o: %.cpp $(BASE_HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)

.PHONY: all clean