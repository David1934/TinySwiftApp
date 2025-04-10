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
#include "depthmapwrapper.h"

/* https://developer.android.com/reference/android/graphics/ImageFormat#DEPTH16
*
* Android dense depth image format.
* Each pixel is 16 bits, representing a depth ranging measurement from a depth camera or similar sensor. The 16-bit sample consists of a confidence value and the actual ranging measurement.
* The confidence value is an estimate of correctness for this sample. It is encoded in the 3 most significant bits of the sample, with a value of 0 representing 100% confidence, a value of 1
* representing 0% confidence, a value of 2 representing 1/7, a value of 3 representing 2/7, and so on.
*
*/

// BUT, to extend the maximum measurement distance from 8.192 meters to 16.384 meters, 
// we have modified depth16 format. The lower 14 bits represent the actual distance, and the higher 2 bits represent the confidence level.
const int CONFIDENCE_MASK = 0x3;
const int DEPTH_MASK = 0x3FFF; // For depth16 format, the low 14 bits is distance
const int DEPTH_BITS = 14;

const int COLOR_HIGH = 9000;
const int COLOR_MEDIUM = 9000;
const int COLOR_MAP_HIGH = COLOR_MEDIUM;

const int RANGE_MIN = 30;
const int RANGE_MAX = 8192;

#define MAX_DEPTH_OUTPUT_FORMATS 3
#define CHIP_TEMPERATURE_MIN_THRESHOLD             15.0
#define CHIP_TEMPERATURE_MAX_THRESHOLD             90.0

/* Minimum and maximum macros */
#define MAX(a,b)  (((a) > (b)) ? (a) : (b))
#define MIN(a,b)  (((a) < (b)) ? (a) : (b))

enum depth_confidence_level{
    ANDROID_CONF_HIGH = 0,
    ANDROID_CONF_LOW = 1,
    ANDROID_CONF_MEDIUM = 3
};

struct BGRColor
{
    u8 Blue;
    u8 Green;
    u8 Red;
};

#if 0
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
#endif

class ADAPS_DTOF : public QObject
{
Q_OBJECT

public:
    ADAPS_DTOF(struct sensor_params params, V4L2 *v4l2);
    ~ADAPS_DTOF();

    int adaps_dtof_initilize();
    void GetDepth4watchSpot(const u16 depth16_buffer[], const int outImgWidth, const int outImgHeight, u8 x, u8 y, u16 *distance, u8 *confidence);
    void ConvertDepthToColoredMap(const u16 depth16_buffer[], u8 depth_colored_map[], u8 depth_confidence_map[], const int outImgWidth, const int outImgHeight);
    void ConvertGreyscaleToColoredMap(u16 depth16_buffer[], u8 depth_colored_map[], int outImgWidth, int outImgHeight);
    int dtof_frame_decode(unsigned char *frm_rawdata, int buf_len, u16 depth16_buffer[], enum sensor_workmode swk);
    void adaps_dtof_release();
#if defined(ENABLE_DYNAMICALLY_UPDATE_ROI_SRAM_CONTENT)
    int DepthBufferMerge(u16 merged_depth16_buffer[], const u16 to_merge_depth16_buffer[], int outImgWidth, int outImgHeight);
#endif

private:
    V4L2 *m_v4l2;
    struct sensor_params m_sns_param;
    uint64_t m_exposure_time;
    int32_t  m_sensitivity;
    swift_eeprom_data_t *p_eeprominfo;
    SetWrapperParam set_param;

    struct BGRColor m_basic_colors[5];
    u16 m_LimitedMaxDistance;
    u16 m_rangeHigh;
    u16 m_rangeLow;
#if 1
    void *m_handlerDepthLib;
    char m_DepthLibversion[32];
    char m_DepthLibConfigXmlPath[128];
#else
    CHILIBRARYHANDLE       m_hDepthLib;
    CREATEDEPTHMAPWRAPPER  m_createDepthMapWrapper;
    DESTROYDEPTHMAPWRAPPER m_destroyDepthMapWrapper;
    PROCESSFRAME           m_processFrame;
    DepthMapWrapper*      m_DepthMapWrapper;
#endif
    bool m_conversionLibInited;
    uint32_t m_decoded_frame_cnt;

#if defined(ENABLE_DYNAMICALLY_UPDATE_ROI_SRAM_CONTENT)
    uint8_t cur_calib_sram_data_group_idx;
    bool trace_calib_sram_switch;
#endif

    int FillSetWrapperParamFromEepromInfo(uint8_t* pEEPROMData, SetWrapperParam* setparam);
    void initParams(WrapperDepthInitInputParams  *     initInputParams,WrapperDepthInitOutputParams      *initOutputParams);
    void PrepareFrameParam(WrapperDepthCamConfig *wrapper_depth_map_config);
    u8 normalizeGreyscale(u16 range);
    void Distance_2_BGRColor(int bucketNum, float bucketSize, u16 distance, struct BGRColor *destColor);
};

#endif // ADAPS_DTOF_H

