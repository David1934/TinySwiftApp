cmake_minimum_required(VERSION 3.10)
project(TinySwiftApp VERSION 1.0 LANGUAGES CXX)

# ========================== 交叉编译配置 ==========================
# 对应 Makefile 中的 CROSS_COMPILE 前缀
set(CROSS_COMPILE_DIR "/home/david/rk_build2025")
set(CROSS_COMPILE_PREFIX "${CROSS_COMPILE_DIR}/buildroot/output/rockchip_rk3568_swift/host/bin/aarch64-buildroot-linux-gnu-")

# 设置交叉编译的系统和编译器（CMake 交叉编译核心配置）
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)
set(CMAKE_C_COMPILER "${CROSS_COMPILE_PREFIX}gcc")
set(CMAKE_CXX_COMPILER "${CROSS_COMPILE_PREFIX}g++")

# 配置C++标准（对应 pro 文件的 C++17）
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 编译模式（对应 pro 文件的 release）
set(CMAKE_BUILD_TYPE Release)

# 基础源文件列表
set(BASE_SRCS
    main.cpp
    dtof_main.cpp
    utils.cpp
    adaps_dtof.cpp
    misc_device.cpp
    v4l2.cpp
)

# 基础头文件列表
set(BASE_HEADERS
    dtof_main.h
    common.h
    depthmapwrapper.h
    utils.h
    misc_device.h
    adaps_dtof.h
    adaps_dtof_uapi.h
    v4l2.h
)

# 生成可执行文件
add_executable(${PROJECT_NAME}
    ${BASE_SRCS}
    ${BASE_HEADERS}  # 头文件加入便于IDE识别，不影响编译
)

# 编译选项（对应 pro 文件的 QMAKE_CXXFLAGS_RELEASE）
target_compile_options(${PROJECT_NAME} PRIVATE
    -O3
    -flto
    -fno-exceptions
    -Wall
    -Wextra
)

# 基础宏定义
target_compile_definitions(${PROJECT_NAME} PRIVATE
    CONFIG_VIDEO_ADS6401
    ENABLE_POINTCLOUD_OUTPUT
)

# 链接选项（对应 pro 文件的 QMAKE_LFLAGS_RELEASE）
target_link_options(${PROJECT_NAME} PRIVATE
    -flto
    -Wl,--gc-sections
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    pthread
)

# 搜索当前目录下的库（对应 pro 文件的 -L$$PWD）
target_link_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE
    adaps_swift_decode
)

# 头文件搜索路径（当前目录）
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)