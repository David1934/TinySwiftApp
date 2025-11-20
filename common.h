#ifndef COMMON_H
#define COMMON_H

#include <cstring>
#include <stdlib.h>
#include <cstdio>
#include <time.h>

#include "adaps_types.h"
#include "adaps_dtof_uapi.h"

#define MAX_CALIB_SRAM_DATA_GROUP_CNT           9

#define VERSION_MAJOR                           1
#define VERSION_MINOR                           0
#define VERSION_REVISION                        0
#define LAST_MODIFIED_TIME                      "20251130A"

#define DEFAULT_DTOF_FRAMERATE                  AdapsFramerateType30FPS // AdapsFramerateType60FPS

#define DEPTH_LIB_DATA_DUMP_PATH                "./RawData/DumpData/"
#define DEPTH_LIB_CONFIG_PATH                   "/vendor/etc/camera/adapsdepthsettings.xml"
#define DATA_SAVE_PATH                          "/tmp/" // "/sdcard/"
#define DEFAULT_SAVE_FRAME_CNT                  0
#define RTCTIME_DISPLAY_FMT                     "hh:mm:ss"  // "yyyy/MM/dd hh:mm:ss"
#define FRAME_PROCESS_THREAD_INTERVAL_US        10   // unit is us

// query the video node by the command 'v4l2-ctl --list-devices' or 'media-ctl -p -d /dev/media0'


#define PIXELFORMAT_4_DTOF_SENSOR               V4L2_PIX_FMT_SBGGR8

#define VIDEO_DEV_4_MISC_DEVICE                 "/dev/ads6401"

#define MEDIA_DEVNAME_4_DTOF_SENSOR             "/dev/media0"
#define VIDEO_DEV_4_DTOF_SENSOR                 "/dev/video0"
#define ENTITY_NAME_4_DTOF_SENSOR               "m00_b_ads6401 4-005e"  // "m01_b_ads6401 8-005e"
//#define VIDIOC_S_FMT_INCLUDE_VIDIOC_SUBDEV_S_FMT    // On rk3588 Linux platform, when call ioctl(fd, VIDIOC_S_FMT, &fmt), it will set format for sensor too.

#define DEFAULT_TIMER_TEST_TIMES                0
#define TIMER_TEST_INTERVAL                     5   // unit is second
#define FRAME_INTERVAL_4_PROGRESS_REPORT        500   // every X frames, report a progress

#define WAIT_TIME_4_THREAD_EXIT                 10  // unit is  milliseconds

#define DEFAULT_ENVIRONMENT_TYPE            AdapsEnvTypeIndoor
#define DEFAULT_MEASUREMENT_TYPE            AdapsMeasurementTypeFull

#define SPOT_MODULE_TYPE_NAME               "Spot"
#define FLOOD_MODULE_TYPE_NAME              "Flood"
#define BIG_FOV_FLOOD_MODULE_TYPE_NAME      "Big_FoV_Flood"
#define BIG_FOV_FLOOD_V2_MODULE_TYPE_NAME   "Big_FoV_Flood_V2"

#define DEFAULT_WORK_MODE                       WK_UNINITIALIZED

#define DEBUG_PRO
#define ENV_VAR_SAVE_EEPROM_ENABLE              "save_eeprom_enable"
#define ENV_VAR_SAVE_DEPTH_TXT_ENABLE           "save_depth_txt_enable"
#define ENV_VAR_SAVE_FRAME_ENABLE               "save_frame_enable"
#define ENV_VAR_SKIP_FRAME_DECODE               "skip_frame_decode"
#define ENV_VAR_ENABLE_EXPAND_PIXEL             "enable_expand_pixel"      // processed in adaps decode algo lib
#define ENV_VAR_DISABLE_COMPOSE_SUBFRAME        "disable_compose_subframe"  // processed in adaps decode algo lib
#define ENV_VAR_MIRROR_X_ENABLE                 "mirror_x_enable"
#define ENV_VAR_MIRROR_Y_ENABLE                 "mirror_y_enable"
#define ENV_VAR_DISABLE_WALK_ERROR              "disable_walk_error"      // processed in adaps decode algo lib
#define ENV_VAR_ROI_SRAM_COORDINATES_CHECK      "roi_sram_coordinates_check"
#define ENV_VAR_FRAME_DROP_CHECK_ENABLE         "frame_drop_check_enable"
#define ENV_VAR_BUFFER_FULLY_ZERO_CHECK         "buffer_fully_zero_check"

#define ENV_VAR_FORCE_COARSE_EXPOSURE           "force_coarseExposure"
#define ENV_VAR_FORCE_FINE_EXPOSURE             "force_fineExposure"
#define ENV_VAR_FORCE_GRAY_EXPOSURE             "force_grayExposure"
#define ENV_VAR_FORCE_LASEREXPOSUREPERIOD       "force_laserExposurePeriod"
#define ENV_VAR_FORCE_FRAMERATE_FPS             "force_framerate_fps"
#define ENV_VAR_FORCE_ROI_SRAM_SIZE             "force_roi_sram_size"

#define ENV_VAR_DBGINFO_ENABLE                  "debug_info_enable"
#define ENV_VAR_TRACE_ALGO_LIB_DECODE_COSTTIME  "trace_algo_lib_decode_costtime"

#define ENV_VAR_DUMP_LENS_INTRINSIC             "dump_lens_intrinsic"
#define ENV_VAR_DUMP_ROI_SRAM_SIZE              "dump_roi_sram_size"
#define ENV_VAR_DUMP_DEPTH_MAP_FRAME_ITVL       "dump_depth_map_frame_interval"
#define ENV_VAR_DUMP_PARSING_SCRIPT_ITEMS       "dump_parsing_script_items"
#define ENV_VAR_DUMP_SPOT_STATISTICS_TIMES      "dump_spot_statistics_times"
#define ENV_VAR_DUMP_PTM_FRAME_HEADINFO_TIMES   "dump_ptm_frame_headinfo_times"
#define ENV_VAR_DUMP_WALKERROR_PARAM_COUNT      "dump_walkerror_param_count"
#define ENV_VAR_DUMP_OFFSETDATA_PARAM_COUNT     "dump_offsetdata_param_count"
#define ENV_VAR_DUMP_CALIB_REFERENCE_DISTANCE   "dump_calib_reference_distance"
#define ENV_VAR_ENABLE_ALGO_LIB_DUMP_DATA       "enable_algo_lib_dump_data"

#define __tostr(x)                          #x
#define __stringify(x)                      __tostr(x)

#define VERSION_STRING                      __stringify(VERSION_MAJOR) "."  \
            __stringify(VERSION_MINOR) "."  \
            __stringify(VERSION_REVISION)

#define APP_NAME                        "TinySwiftApp"

#define APP_VERSION_CODE                 (VERSION_MAJOR << 16 | VERSION_MINOR << 8 | VERSION_REVISION)

#define APP_VERSION                      VERSION_STRING "_LM" LAST_MODIFIED_TIME

#define NULL_POINTER                            nullptr

inline const char* get_filename(const char* path) {
    const char* file = strrchr(path, '/');
    if (!file) {
        file = strrchr(path, '\\');
    }
    return file ? file + 1 : path;
}

inline bool env_var_is_true(const char *var_name)
{
    const char *env_var_value = getenv(var_name);

    if (env_var_value != NULL_POINTER && strcmp(env_var_value, "true") == 0) {
        return true;
    }
    return false;
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
        printf("<%s-%s() %d> " fmt "\n",                                                         \
            get_filename(__FILE__), __func__, __LINE__, ##args);                                   \
    }while(0)

#define DBG_ERROR(fmt, args ...)                                                                    \
    do {                                                                                            \
        struct timespec ts;                                                                         \
        clock_gettime(CLOCK_REALTIME, &ts);                                                         \
        struct tm tm = *localtime(&ts.tv_sec);                                                      \
        char timestamp[120];                                                                        \
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", &tm);                           \
        printf("[%s.%03ld.%03ld.%03ld] ERROR: <%s-%s() %d> " fmt "\n", timestamp, ts.tv_nsec/1000000,    \
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
        printf("[%s] NOTICE: <%s-%s() %d> " fmt "\n", timestamp, get_filename(__FILE__), __func__, __LINE__, ##args);                \
    }while(0)

#define DBG_INFO(fmt, args ...)                                                                 \
    if (env_var_is_true(ENV_VAR_DBGINFO_ENABLE)) {                                                \
        struct timespec ts;                                                                     \
        clock_gettime(CLOCK_REALTIME, &ts);                                                     \
        struct tm tm = *localtime(&ts.tv_sec);                                                  \
        char timestamp[120];                                                                    \
        sprintf(timestamp, "%4d-%02d-%02d %02d:%02d:%02d.%03ld.%03ld.%03ld", tm.tm_year+1900,   \
            tm.tm_mon+1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec,                          \
            ts.tv_nsec/1000000, (ts.tv_nsec/1000)%1000, ts.tv_nsec%1000);                       \
        printf("[%s] INFO: <%s-%s() %d> " fmt "\n", timestamp, get_filename(__FILE__), __func__, __LINE__, ##args);                \
    }
#else
#define DBG_PRINTK      printf
#define DBG_ERROR       printf
#define DBG_NOTICE      printf
#define DBG_INFO        printf
#endif

#define errno_debug(fmt)            DBG_ERROR("%s error %d, %s\n", (fmt), errno, strerror(errno))

typedef unsigned char u8;
typedef unsigned short u16;
typedef signed short s16;
typedef unsigned int u32;
typedef unsigned long long u64;

enum frame_data_type{
    FDATA_TYPE_DTOF_RAW_GRAYSCALE,
    FDATA_TYPE_DTOF_RAW_DEPTH,
    FDATA_TYPE_DTOF_DECODED_GRAYSCALE,
    FDATA_TYPE_DTOF_DECODED_DEPTH16,
    FDATA_TYPE_DTOF_DECODED_POINT_CLOUD,
    FDATA_TYPE_COUNT
};

enum sensor_workmode{
    WK_DTOF_PHR,
    WK_DTOF_PCM,
    WK_DTOF_FHR,
    WK_UNINITIALIZED,
    WK_COUNT = WK_UNINITIALIZED,
};

struct sensor_params
{
    enum sensor_workmode work_mode;
    uint32_t    to_dump_frame_cnt;
    bool        roi_sram_rolling;
    int         raw_width;
    int         raw_height;
    int         out_frm_width;
    int         out_frm_height;
    AdapsEnvironmentType env_type;
    AdapsMeasurementType measure_type;
    AdapsFramerateType framerate_type;
    AdapsPowerMode  power_mode;
    AdapsEnvironmentType advisedEnvType;
    AdapsMeasurementType advisedMeasureType;
    struct adaps_dtof_exposure_param exposureParam;
    UINT8 module_kernel_type;
};

#endif // COMMON_H

