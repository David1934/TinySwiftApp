#ifndef __DEPTHMAP_WRAPPER_H__
#define __DEPTHMAP_WRAPPER_H__

/**=============================================================================

Copyright (c) 2017-2021 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================**/


//==============================================================================
// Included modules
//==============================================================================
#include <stdint.h>
#include <memory>


//==============================================================================
// MACROS
//==============================================================================
#ifdef _MSC_VER
#define CP_DLL_PUBLIC __declspec(dllexport)
#else
#define CP_DLL_PUBLIC __attribute__ ((visibility ("default")))
#endif

#define IMG_UNUSED(x) (void)(x)


typedef unsigned char      uint8_t;
typedef unsigned int        uint32_t;


//==============================================================================
// DECLARATIONS
//==============================================================================
#define ADAPS_SPARSE_POINT_POSITION_DATA_SIZE 960
#define AdapsAlgoLibVersionLength  32

struct AdapsSparsePointPositionData
{
	uint32_t x_pos[ADAPS_SPARSE_POINT_POSITION_DATA_SIZE];
	uint32_t y_pos[ADAPS_SPARSE_POINT_POSITION_DATA_SIZE];
	uint32_t hist[ADAPS_SPARSE_POINT_POSITION_DATA_SIZE];
};

typedef struct ADAPS_MIRROR_FRAME_SET
  {
      uint8_t mirror_x;
      uint8_t mirror_y;
  }AdapsMirrorFrameSet;

struct AdapsAdvisedType
{
	uint8_t AdvisedMeasurementType;
	uint8_t AdvisedEnvironmentType;
};

typedef struct pc_pack {
    float X;
    float Y;
    float Z;
    float c;
} pc_pkt_t;

typedef enum {
    WRAPPER_CAM_FORMAT_NONE,
    WRAPPER_CAM_FORMAT_IR,
    WRAPPER_CAM_FORMAT_DEPTH16,
    WRAPPER_CAM_FORMAT_DEPTH_POINT_CLOUD,
    WRAPPER_CAM_FORMAT_MAX,
} WrapperDepthFormat;

typedef enum {
    DEPTH_OUT_NORMAL,       ///< No change
    DEPTH_OUT_MIRROR,       ///< Mirror(horizontal)
    DEPTH_OUT_FLIP,         ///< Flip(vertical)
    DEPTH_OUT_MIRROR_FLIP,  ///< Mirror/Flip(h/v)
} RotateConfig;

typedef struct {
    int32_t  bitsPerPixel;
    uint32_t strideBytes;
    uint32_t sliceHeight;
    size_t   planeSize;
} WrapperDepthFormatParams;

//begin: add by hzt 2021-12-6 for adaps control
typedef struct {
    AdapsEnvironmentType env_type_in;
    AdapsMeasurementType measure_type_in;
    float laser_realtime_tempe;
    AdapsEnvironmentType *advised_env_type_out;
    AdapsMeasurementType *advised_measure_type_out;
    int32_t focutPoint[2];// 0 is x,1 is y
} AdapsParamAndOutForProcessEveryFrame;
//end: add by hzt 2021-12-6 for adaps control

typedef enum {
    WRAPPER_EXP_MODE_ME,
    WRAPPER_EXP_MODE_AE,
    WRAPPER_EXP_MODE_NONE
} WrapperDepthExpModeType;

typedef struct {
    float min_fps;
    float max_fps;
    float video_min_fps;
    float video_max_fps;
} WrapperDepthCamFpsRange;

typedef struct {
    int64_t in_depth_map_me_val;
    int64_t in_out_depth_map_ae_val;
    int64_t in_depth_map_frame_num;
    int64_t in_out_depth_map_timestamp;
    int64_t out_depth_map_laser_strength_val;
    WrapperDepthExpModeType in_depth_map_exp_mode;
    WrapperDepthCamFpsRange in_depth_map_fps_range;
	AdapsParamAndOutForProcessEveryFrame frame_parameters;
} WrapperDepthCamConfig;

typedef struct {
    WrapperDepthFormat format;
    WrapperDepthFormatParams formatParams;
	struct AdapsSparsePointPositionData sPPData;
    union
    {
        uint8_t*  out_depth_image;
        pc_pkt_t* out_pcloud_image;
    };
    int32_t out_image_fd;
    union
    {
        uint32_t  out_image_length;
        uint32_t* count_pt_cloud;
    };
} WrapperDepthOutput;

typedef struct {
    WrapperDepthFormatParams formatParams;
    const int8_t* in_image;
    int32_t in_image_fd;
} WrapperDepthInput;

//begin: add by hzt 2021-12-6 for adaps control
typedef struct {
    uint8_t work_mode;
    bool compose_subframe;
    bool expand_pixel;
    bool walkerror;
    AdapsMirrorFrameSet mirror_frame;
    float* adapsLensIntrinsicData;          // 9xsizeof(float)
    float* adapsSpodOffsetData;             // 4x240xsizeof(float)
    float* accurateSpotPosData;             // 4x240xsizeof(float)x2
    uint8_t ptm_fine_exposure_value;        // fine exposure value, 0 - 255
    uint8_t exposure_period;                // exposure_period, 0 - 255
    float cali_ref_tempe[2];  //[0] for indoor, [1] for outdoor
    float cali_ref_depth[2];  //[0] for indoor, [1] for outdoor
    AdapsEnvironmentType env_type;  // value 0-->indoor, value 1 -->outdoor
    AdapsMeasurementType measure_type; //value 0-->normal distance, 1-->short distance
    uint8_t *proximity_hist; //256 bytes for eeprom
    // TODO - after v1.2.0
    uint8_t *OutAlgoVersion;  // OutAlgoVersion[AdapsAlgoVersionLength];
    uint8_t zone_cnt;
} SetWrapperParam;

typedef struct {
    int32_t         width;
    int32_t         height;
    int32_t         dm_width;
    int32_t         dm_height;
    uint8_t         is_secure;
    uint8_t*        pRawData;
    uint32_t        rawDataSize;
    RotateConfig    rotateConfig;
    uint32_t        outputPlaneCount;
    uint32_t        outputPlaneFormat[WRAPPER_CAM_FORMAT_MAX];
	SetWrapperParam setparam;
} WrapperDepthInitInputParams;

typedef struct {
    uint64_t* exposure_time;
    int32_t*  sensitivity;
} WrapperDepthInitOutputParams;
//end: add by hzt 2021-12-6 for adaps control

//==============================================================================
// API CLASS
//==============================================================================
class CP_DLL_PUBLIC DepthMapWrapper
{
public:
    DepthMapWrapper(
        int32_t width,
        int32_t height,
        int32_t dm_width,
        int32_t dm_height,
        uint8_t is_secure,
        uint8_t* pRawData,
        uint32_t rawDataSize,
        SetWrapperParam set_param,
        uint64_t* exposure_time,
        int32_t* sensitivity);
    
    bool processFrame(
        WrapperDepthInput in_image,
        WrapperDepthCamConfig *wrapper_depth_map_config,
        uint32_t num_outputs,
        WrapperDepthOutput outputs[]);

    ~DepthMapWrapper() ;

    // For future use
    static unsigned int getWrapperVersion();

private:
    struct DepthWrapperData;
    DepthWrapperData *pDepthData;

    DepthMapWrapper(); // No default constructor
    DepthMapWrapper( const DepthMapWrapper& ); // No copy constructor
    DepthMapWrapper& operator=( const DepthMapWrapper& ); // No copy assignment operator
};

// Class factories
#ifdef __cplusplus
extern "C" {
#endif
CP_DLL_PUBLIC
DepthMapWrapper* DepthMapWrapperCreate(
    WrapperDepthInitInputParams  inputParams,
    WrapperDepthInitOutputParams outputParams);

CP_DLL_PUBLIC
bool DepthMapWrapperProcessFrame(
    DepthMapWrapper* pDepthMapWrapper,
    WrapperDepthInput in_image,
    WrapperDepthCamConfig *wrapper_depth_map_config,
    uint32_t num_outputs,
    WrapperDepthOutput outputs[]);

CP_DLL_PUBLIC
void DepthMapWrapperDestroy(DepthMapWrapper* pDepthMapWrapper);
#ifdef __cplusplus
}
#endif

#endif //__DEPTHMAP_WRAPPER_H__

