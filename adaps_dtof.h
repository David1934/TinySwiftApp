#ifndef ADAPS_DTOF_H
#define ADAPS_DTOF_H

#include <fcntl.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/mman.h>
#include <linux/videodev2.h>
#include <linux/media.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <QDebug>
#include <QDateTime>
#include "v4l2.h"

const int DEPTH_MASK = 0x1FFF; // For depth16 format, the low 13 bits is distance
const int COLOR_HIGH = 9000;
const int COLOR_MEDIUM = 9000;
const int COLOR_MAP_HIGH = COLOR_MEDIUM;

const int RANGE_MIN = 30;
const int RANGE_MAX = 8192;

#define MAX_DEPTH_OUTPUT_FORMATS 3
#define CHIP_TEMPERATURE_THRESHOLD                 10.0
#define CHIP_TEMPERATURE_MAX_THRESHOLD             80.0

/* Minimum and maximum macros */
#define MAX(a,b)  (((a) > (b)) ? (a) : (b))
#define MIN(a,b)  (((a) < (b)) ? (a) : (b))

struct BGRColor
{
    u8 Blue;
    u8 Green;
    u8 Red;
};

typedef void* CHILIBRARYHANDLE;
typedef DepthMapWrapper*(*CREATEDEPTHMAPWRAPPER)(
      WrapperDepthInitInputParams  initInputParams,
      WrapperDepthInitOutputParams initOutputParams);
typedef void(*DESTROYDEPTHMAPWRAPPER)(
      DepthMapWrapper* pDepthMapWrapper);
typedef bool(*PROCESSFRAME)(
      DepthMapWrapper*       pDepthMapWrapper,
      WrapperDepthInput      in_image,
      WrapperDepthCamConfig* wrapper_depth_map_config,
      uint32_t                 num_outputs,
      WrapperDepthOutput     outputs[]);

class ADAPS_DTOF : public QObject
{
Q_OBJECT

public:
    ADAPS_DTOF(struct sensor_params params, V4L2 *v4l2);
    ~ADAPS_DTOF();

    int initilize();
    void ConvertDepthToColoredMap(u16 depth16_buffer[], u8 depth_colored_map[], int outImgWidth, int outImgHeight);
    void ConvertGreyscaleToColoredMap(u16 depth16_buffer[], u8 depth_colored_map[], int outImgWidth, int outImgHeight);
    int dtof_decode(unsigned char *frm_rawdata , u16 depth16_buffer[], enum sensor_workmode swk);
    void release();

private:
    V4L2 *m_v4l2;
    struct sensor_params m_sns_param;
    uint64_t m_exposure_time;
    int32_t  m_sensitivity;
    struct adaps_get_eeprom *p_eeprominfo;

    struct BGRColor m_basic_colors[5];
    u16 m_LimitedMaxDistance;
    u16 m_rangeHigh;
    u16 m_rangeLow;
    CHILIBRARYHANDLE       m_hDepthLib;
    CREATEDEPTHMAPWRAPPER  m_createDepthMapWrapper;
    DESTROYDEPTHMAPWRAPPER m_destroyDepthMapWrapper;
    PROCESSFRAME           m_processFrame;
    DepthMapWrapper*      m_DepthMapWrapper;
    bool m_conversionLibInited;

    int GetAdapsTofEepromInfo(uint8_t* pRawData, uint32_t rawDataSize, SetWrapperParam* setparam);
    void initParams(WrapperDepthInitInputParams  *     initInputParams,WrapperDepthInitOutputParams      *initOutputParams);
    void PrepareFrameParam(WrapperDepthCamConfig *wrapper_depth_map_config);
    u8 normalizeGreyscale(u16 range);
    void Distance_2_BGRColor(int bucketNum, float bucketSize, u16 distance, struct BGRColor *destColor);
};

#endif // ADAPS_DTOF_H

