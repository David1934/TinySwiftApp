# 编译器配置
CROSS_COMPILE_DIR = /home/david/rk_build2025
CROSS_COMPILE = $(CROSS_COMPILE_DIR)/buildroot/output/rockchip_rk3568_swift/host/bin/aarch64-buildroot-linux-gnu-

CC       := $(CROSS_COMPILE)gcc
CXX      := $(CROSS_COMPILE)g++

TARGET = TinySwiftApp
CXXFLAGS = -std=c++17 -O3 -flto -fno-exceptions -Wall -Wextra
LDFLAGS = -flto -Wl,--gc-sections

# 基础源文件列表
BASE_SRCS = \
    main.cpp \
    dtof_main.cpp \
    utils.cpp \
    adaps_dtof.cpp \
    misc_device.cpp \
    v4l2.cpp

# 基础头文件列表（作为依赖项，确保头文件修改时重新编译）
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
# 追加嵌入式链接选项（rpath）
LDFLAGS += -Wl,-rpath,/vendor/lib64/
# 追加嵌入式专用库（-L. 对应pro文件的 -L$$PWD）
LIBS += -L. -ladaps_swift_decode -lpthread

# 生成目标文件列表（.cpp 转 .o）
OBJS = $(BASE_SRCS:.cpp=.o)

# 编译选项中加入宏定义
CXXFLAGS += $(DEFINES)

# 默认目标：编译生成可执行文件
all: $(TARGET)

# 链接目标文件生成可执行文件
$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS) $(LIBS)

# 编译规则：每个 .cpp 文件生成对应的 .o 文件
%.o: %.cpp $(BASE_HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 清理目标：删除目标文件
clean:
	rm -f $(OBJS)

# 伪目标声明（避免与同名文件冲突）
.PHONY: all clean