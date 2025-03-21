#ifndef COMMON_H
#define COMMON_H

#include <QObject>
#include <QString>
#include <QMetaType>

#if defined(RUN_ON_ROCKCHIP)
//#define IGNORE_REAL_COMMUNICATION
#include "rk-camera-module.h"

#define MAX_CALIB_SRAM_DATA_GROUP_CNT                   9

#else

#include "adaps_types.h"
#include "adaps_dtof_uapi.h"
#endif

#define VERSION_MAJOR                           2
#define VERSION_MINOR                           2
#define VERSION_REVISION                        0
#define LAST_MODIFIED_TIME                      "20250326A"

#define DEFAULT_DTOF_FRAMERATE                  AdapsFramerateType30FPS // AdapsFramerateType60FPS

#define DEPTH_LIB_CONFIG_PATH                   "/vendor/etc/camera/adapsdepthsettings.xml"
#define DATA_SAVE_PATH                          "/tmp/" // "/sdcard/"
#define DEFAULT_SAVE_FRAME_CNT                  0
#define RTCTIME_DISPLAY_FMT                     "hh:mm:ss"  // "yyyy/MM/dd hh:mm:ss"
#define FRAME_INTERVAL_US                       10   // unit is us

// query the video node by the command 'v4l2-ctl --list-devices' or 'media-ctl -p -d /dev/media0'


#define PIXELFORMAT_4_DTOF_SENSOR               V4L2_PIX_FMT_SBGGR8

#define MEDIA_DEVNAME_4_RGB_SENSOR              "/dev/media0"
#define VIDEO_DEV_4_RGB_SENSOR                  "/dev/video0"
#define VIDEO_DEV_4_RGB_RK3588                  "/dev/video55"
#define VIDEO_DEV_4_MISC_DEVICE                 "/dev/ads6401"

#if defined(RUN_ON_RK3568)
    // for rk3568
    #define MEDIA_DEVNAME_4_DTOF_SENSOR             "/dev/media0"
    #define VIDEO_DEV_4_DTOF_SENSOR                 "/dev/video0"
    #define ENTITY_NAME_4_DTOF_SENSOR               "m00_b_ads6401 4-005e"
#else
    // for rk3588
    #define MEDIA_DEVNAME_4_DTOF_SENSOR             "/dev/media2"
    #define VIDEO_DEV_4_DTOF_SENSOR                 "/dev/video22"
    #define ENTITY_NAME_4_DTOF_SENSOR               "m00_b_ads6401 7-005e"
    #define VIDIOC_S_FMT_INCLUDE_VIDIOC_SUBDEV_S_FMT    // On rk3568 Linux platform, when call ioctl(fd, VIDIOC_S_FMT, &fmt), it will set format for sensor too.
#endif

#define DEFAULT_TIMER_TEST_TIMES                0
#define TIMER_TEST_INTERVAL                     5   // unit is second

#define WAIT_TIME_4_THREAD_EXIT                 10  // unit is  milliseconds

#if defined(RUN_ON_ROCKCHIP)
#define WKMODE_4_RGB_SENSOR                     WK_RGB_NV12    // On DELL notebook, it is WK_MODE_YUYV, on Apple notebook it is WK_MODE_NV12?

#define DEFAULT_SENSOR_TYPE                     SENSOR_TYPE_DTOF
#define DEFAULT_WORK_MODE                       WK_DTOF_FHR
    #if (ADS6401_MODDULE_SPOT == SWIFT_MODULE_TYPE)
        #define SWIFT_MODULE_TYPE_NAME          "Spot"
    #else
        #define SWIFT_MODULE_TYPE_NAME          "Flood"
    #endif
#else
#define WKMODE_4_RGB_SENSOR                     WK_RGB_YUYV    // On DELL notebook, it is WK_MODE_YUYV, on Apple notebook it is WK_MODE_NV12?

#define DEFAULT_SENSOR_TYPE                     SENSOR_TYPE_RGB
#define DEFAULT_WORK_MODE                       WKMODE_4_RGB_SENSOR
#endif

#define DEBUG_PRO
#define ENV_VAR_SAVE_EEPROM_ENABLE              "save_eeprom_enable"
#define ENV_VAR_SAVE_DEPTH_TXT_ENABLE           "save_depth_txt_enable"
#define ENV_VAR_SAVE_FRAME_ENABLE               "save_frame_enable"
#define ENV_VAR_DBGINFO_ENABLE                  "debug_info_enable"
#define ENV_VAR_SKIP_FRAME_PROCESS              "skip_frame_process"
#define ENV_VAR_SKIP_EEPROM_CRC_CHK             "skip_eeprom_crc_check"
#define ENV_VAR_DUMP_LENS_INTRINSIC             "dump_lens_intrinsic"
#define ENV_VAR_DISABLE_EXPAND_PIXEL            "disable_expand_pixel"      // processed in adaps decode algo lib
#define ENV_VAR_DISABLE_COMPOSE_SUBFRAME        "disable_compose_subframe"  // processed in adaps decode algo lib
#define ENV_VAR_DUMP_ROI_SRAM_SIZE              "dump_roi_sram_size"
#define ENV_VAR_TRACE_ROI_SRAM_SWITCH           "trace_roi_sram_switch"
#define ENV_VAR_DUMP_SPOT_DEPTH                 "dump_spot_depth"
#define ENV_VAR_MIRROR_X_ENABLE                 "mirror_x_enable"
#define ENV_VAR_MIRROR_Y_ENABLE                 "mirror_y_enable"
#define ENV_VAR_DISABLE_WALK_ERROR              "disable_walk_error"      // processed in adaps decode algo lib

#define __tostr(x)                          #x
#define __stringify(x)                      __tostr(x)

#define VERSION_STRING                      __stringify(VERSION_MAJOR) "."  \
            __stringify(VERSION_MINOR) "."  \
            __stringify(VERSION_REVISION)

#define APP_NAME                          "SpadisQT"
#define APP_VERSION_CODE                 (VERSION_MAJOR << 16 | VERSION_MINOR << 8 | VERSION_REVISION)

#if defined(RUN_ON_ROCKCHIP)
#define APP_VERSION                      VERSION_STRING "_LM" LAST_MODIFIED_TIME "_For_" SWIFT_MODULE_TYPE_NAME "_Module"
#else
#define APP_VERSION                      VERSION_STRING "_LM" LAST_MODIFIED_TIME
#endif

inline const char* get_filename(const char* path) {
    const char* file = strrchr(path, '/');
    if (!file) {
        file = strrchr(path, '\\');
    }
    return file ? file + 1 : path;
}

inline bool is_env_var_true(const char *var_name)
{
    const char *env_var_value = getenv(var_name);

    if (env_var_value != NULL && strcmp(env_var_value, "true") == 0) {
        return true;
    }
    return false;
}

inline int get_env_var_intvalue(const char *var_name)
{
    int ret = 0;
    const char *env_var_value = getenv(var_name);

    if (env_var_value != NULL) {
        ret = atoi(env_var_value);
    }

    return ret;
}

inline char *get_env_var_stringvalue(const char *var_name)
{
    return getenv(var_name);
}

#if defined(TRACE_IOCTL)
#define misc_ioctl(fh, request, arg) ({                    \
    int ret_val;                                       \
    do {                                               \
        printf("<%s> %d MISC_IOCTL(%d, 0x%x, 0x%x) \n", get_filename(__FILE__), __LINE__, fh, request, arg);   \
        ret_val = ioctl(fh, request, arg);             \
    } while (-1 == ret_val && EINTR == errno);         \
    ret_val;                                           \
})
#else
#define misc_ioctl                  ioctl
#endif

#if defined(DEBUG_PRO)
#define DBG_PRINTK(fmt, args ...)                                                                            \
    do {                                                                                                    \
        qCritical("<%s-%s() %d> " fmt "\n",                                                         \
            get_filename(__FILE__), __func__, __LINE__, ##args);                                   \
    }while(0)

#define DBG_ERROR(fmt, args ...)                                                                    \
    do {                                                                                            \
        struct timespec ts;                                                                         \
        clock_gettime(CLOCK_REALTIME, &ts);                                                         \
        struct tm tm = *localtime(&ts.tv_sec);                                                      \
        char timestamp[120];                                                                        \
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", &tm);                           \
        qCritical("[%s.%03ld.%03ld.%03ld] ERROR: <%s-%s() %d> " fmt "\n", timestamp, ts.tv_nsec/1000000,    \
            (ts.tv_nsec/1000)%1000, ts.tv_nsec%1000, get_filename(__FILE__), __func__, __LINE__, ##args);                   \
    }while(0)

#define DBG_NOTICE(fmt, args ...)                                           \
    do {                                                                                            \
        struct timespec ts;                                                                     \
        clock_gettime(CLOCK_REALTIME, &ts);                                                     \
        struct tm tm = *localtime(&ts.tv_sec);                                                  \
        char timestamp[120];                                                                    \
        sprintf(timestamp, "%4d-%02d-%02d %02d:%02d:%02d.%03ld.%03ld.%03ld", tm.tm_year+1900,   \
            tm.tm_mon+1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec,                          \
            ts.tv_nsec/1000000, (ts.tv_nsec/1000)%1000, ts.tv_nsec%1000);                       \
        qCritical("[%s] NOTICE: <%s-%s() %d> " fmt "\n", timestamp, get_filename(__FILE__), __func__, __LINE__, ##args);                \
    }while(0)

#define DBG_INFO(fmt, args ...)                                                                 \
    if (is_env_var_true(ENV_VAR_DBGINFO_ENABLE)) {                                                \
        struct timespec ts;                                                                     \
        clock_gettime(CLOCK_REALTIME, &ts);                                                     \
        struct tm tm = *localtime(&ts.tv_sec);                                                  \
        char timestamp[120];                                                                    \
        sprintf(timestamp, "%4d-%02d-%02d %02d:%02d:%02d.%03ld.%03ld.%03ld", tm.tm_year+1900,   \
            tm.tm_mon+1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec,                          \
            ts.tv_nsec/1000000, (ts.tv_nsec/1000)%1000, ts.tv_nsec%1000);                       \
        qCritical("[%s] INFO: <%s-%s() %d> " fmt "\n", timestamp, get_filename(__FILE__), __func__, __LINE__, ##args);                \
    }
#else
#define DBG_PRINTK      printf
#define DBG_ERROR       printf
#define DBG_NOTICE      printf
#define DBG_INFO        printf
#endif

#define LOG_ERROR                DBG_ERROR
#define LOG_INFO                DBG_INFO

typedef unsigned char u8;
typedef unsigned short u16;
typedef signed short s16;
typedef unsigned int u32;
typedef unsigned long long u64;

enum stop_request_code{
    STOP_REQUEST_RGB,
    STOP_REQUEST_PHR,
    STOP_REQUEST_PCM,
    STOP_REQUEST_FHR,
    STOP_REQUEST_STOP,
    STOP_REQUEST_QUIT,
};

enum sensortype{
    SENSOR_TYPE_RGB,
    SENSOR_TYPE_DTOF
};

enum frame_data_type{
    FDATA_TYPE_NV12,
    FDATA_TYPE_YUYV,
    FDATA_TYPE_RGB888,
    FDATA_TYPE_DTOF_RAW_GRAYSCALE,
    FDATA_TYPE_DTOF_RAW_DEPTH,
    FDATA_TYPE_DTOF_DECODED_GRAYSCALE,
    FDATA_TYPE_DTOF_DECODED_DEPTH16,
    FDATA_TYPE_COUNT
};

enum sensor_workmode{
    WK_DTOF_PHR,
    WK_DTOF_PCM,
    WK_DTOF_FHR,
    WK_RGB_NV12,
    WK_RGB_YUYV,
    WK_COUNT
};

struct sensor_params
{
    enum sensortype sensor_type;
    enum sensor_workmode work_mode;
    int         save_frame_cnt;
    int         raw_width;
    int         raw_height;
    int         out_frm_width;
    int         out_frm_height;
    AdapsEnvironmentType env_type;
    AdapsMeasurementType measure_type;
    AdapsEnvironmentType advisedEnvType;
    AdapsMeasurementType advisedMeasureType;
#if defined(RUN_ON_ROCKCHIP)
    struct adaps_dtof_exposure_param exposureParam;
#endif
};

struct status_params1
{
    int mipi_rx_fps;
    unsigned long streamed_time_us;
#if defined(RUN_ON_ROCKCHIP)
    unsigned int curr_temperature;
    unsigned int curr_exp_vop_abs;
    unsigned int curr_exp_pvdd;
#endif
};

struct status_params2
{
    int mipi_rx_fps;
    unsigned long streamed_time_us;
    unsigned int sensor_type;
    unsigned int work_mode;
#if defined(RUN_ON_ROCKCHIP)
    unsigned int curr_temperature;
    unsigned int curr_exp_vop_abs;
    unsigned int curr_exp_pvdd;
    unsigned int env_type;
    unsigned int measure_type;
#endif
};

Q_DECLARE_METATYPE(struct status_params1);
Q_DECLARE_METATYPE(struct status_params2);

#endif // COMMON_H

