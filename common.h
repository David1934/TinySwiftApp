#ifndef COMMON_H
#define COMMON_H

#include <QObject>
#include "rk-camera-module.h"
#include "depthmapwrapper.h"
#include <QString>

#define DEBUG                       1
#define FRAME_INTERVAL              1   // unit is ms
#define APP_NAME                    "QSTCamera"
#define APP_VERSION                 "v1.0_build20230729a"

#define DEFAULT_SENSOR_TYPE         SENSOR_TYPE_RGB
#define DEFAULT_WORK_MODE           WK_RGB_YUYV
#define DEFAULT_SAVE_FRAME_CNT      1

#define DBG_ERROR(fmt, args ...)						\
        qCritical("ERR: <%s> %d " fmt "\n",				\
			__func__, __LINE__, ##args)

#define DBG_INFO(fmt, args ...) 									\
	if (DEBUG) {						\
        qCritical("INFO: <%s> %d " fmt "\n",				\
			__func__, __LINE__, ##args);				\
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
