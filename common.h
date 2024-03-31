#ifndef COMMON_H
#define COMMON_H

#include <QObject>
#include "rk-camera-module.h"
#include "depthmapwrapper.h"
#include <QString>

#define DEBUG                       1
#define FRAME_INTERVAL              1   // unit is ms
#define APP_NAME                    "SpadisQT"
#define APP_VERSION                 "v1.0_build20240331a"

#if defined(RUN_ON_RK3588)
#define WKMODE_4_RGB_SENSOR         WK_RGB_NV12    // On DELL notebook, it is WK_MODE_YUYV, on Apple notebookm it is WK_MODE_NV12?

#define DEFAULT_SENSOR_TYPE         SENSOR_TYPE_DTOF
#define DEFAULT_WORK_MODE           WK_DTOF_PHR
#else
#define WKMODE_4_RGB_SENSOR         WK_RGB_YUYV    // On DELL notebook, it is WK_MODE_YUYV, on Apple notebookm it is WK_MODE_NV12?

#define DEFAULT_SENSOR_TYPE         SENSOR_TYPE_RGB
#define DEFAULT_WORK_MODE           WKMODE_4_RGB_SENSOR
#endif

#define DEFAULT_SAVE_FRAME_CNT      1

#define RTCTIME_DISPLAY_FMT         "hh:mm:ss"  // "yyyy/MM/dd hh:mm:ss"

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

#define DBG_ERROR(fmt, args ...)                                                                    \
    do {                                                                                            \
        struct timespec ts;                                                                         \
        clock_gettime(CLOCK_REALTIME, &ts);                                                         \
        struct tm tm = *localtime(&ts.tv_sec);                                                      \
        char timestamp[120];                                                                        \
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", &tm);                           \
        qCritical("[%s.%03ld.%03ld.%03ld] ERROR: <%s> %d " fmt "\n", timestamp, ts.tv_nsec/1000000, \
            (ts.tv_nsec/1000)%1000, ts.tv_nsec%1000, __func__, __LINE__, ##args);                   \
    }while(0)

#define DBG_INFO(fmt, args ...)                                                                     \
    if (DEBUG) {                                                                                    \
        struct timespec ts;                                                                         \
        clock_gettime(CLOCK_REALTIME, &ts);                                                         \
        struct tm tm = *localtime(&ts.tv_sec);                                                      \
        char timestamp[120];                                                                        \
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", &tm);                           \
        qCritical("[%s.%03ld.%03ld.%03ld] INFO: <%s> %d " fmt "\n", timestamp, ts.tv_nsec/1000000,  \
            (ts.tv_nsec/1000)%1000, ts.tv_nsec%1000, __func__, __LINE__, ##args);                   \
    }

typedef unsigned char u8;
typedef unsigned short u16;

enum sensortype{
    SENSOR_TYPE_RGB,
    SENSOR_TYPE_DTOF
};

enum frame_data_type{
    FDATA_TYPE_NV12,
    FDATA_TYPE_YUYV,
    FDATA_TYPE_RGB,
    FDATA_TYPE_DTOF_GRAYSCALE,
    FDATA_TYPE_DTOF_DEPTH,
    FDATA_TYPE_DTOF_DEPTH16,
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
};

#endif // COMMON_H

