#ifndef ADAPS_TYPES_H
#define ADAPS_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


typedef int                 BOOL;
typedef char                CHAR;
typedef uint8_t             SBYTE;
typedef unsigned char       UCHAR;
typedef int                 INT;
typedef unsigned int        UINT;
typedef float               FLOAT;

typedef uint8_t             UINT8;
typedef int8_t              INT8;
typedef uint16_t            UINT16;
typedef int16_t             INT16;
typedef uint32_t            UINT32;
typedef int32_t             INT32;
typedef uint64_t            UINT64;
typedef int64_t             INT64;

#if !defined (TRUE)
#define TRUE                1
#endif // !defined (TRUE)

#if !defined (FALSE)
#define FALSE               0
#endif // !defined (FALSE)

#ifdef __cplusplus
}
#endif // __cplusplus



/* It seems useless now.
struct VcselZoneData
{
    UINT8 vcseZoneData[10];
};
*/

typedef struct ADAPS_MIRROR_FRAME_SET
{
    UINT8 mirror_x;
    UINT8 mirror_y;
}AdapsMirrorFrameSet;

// adaps tof operation mode
enum adaps_work_mode {
    ADAPS_PTM_PHR_MODE   = 0,
    ADAPS_PCM_MODE = 1,
    ADAPS_PTM_FHR_MODE   = 2,
    ADAPS_PTM_DEBUG_PHR_MODE = 3,
    ADAPS_PTM_DEBUG_FHR_MODE=4,
    ADAPS_MODE_MAX,
};

typedef enum
{
    AdapsFramerateTypeUninitilized,
    AdapsFramerateType15FPS,
    AdapsFramerateType25FPS,
    AdapsFramerateType30FPS,
    AdapsFramerateType60FPS,
} AdapsFramerateType;

typedef enum
{
    AdapsFrameModeUninitilized,
    AdapsFrameModeA,
    AdapsFrameModeB,
} AdapsFrameMode;

typedef enum
{
    AdapsVcselZoneCountUninitilized,
    AdapsVcselZoneCount1,
    AdapsVcselZoneCount4 = 4,
} AdapsVcselZoneCountType;

typedef enum
{
    AdapsApplyModeUninitilized,
    AdapsApplyModeFusion,
    AdapsApplyModeFocus,
} AdapsApplyMode;


typedef enum
{
    AdapsMeasurementTypeUninitilized,
    AdapsMeasurementTypeNormal,
    AdapsMeasurementTypeShort,
    AdapsMeasurementTypeFull,
} AdapsMeasurementType;

typedef enum {
    AdapsEnvTypeUninitilized,
    AdapsEnvTypeIndoor,
    AdapsEnvTypeOutdoor,
} AdapsEnvironmentType;

typedef enum {
    AdapsPowerModeUninitilized,
    AdapsPowerModeNormal,
    AdapsPowerModeDiv3,
} AdapsPowerMode;

typedef enum {
    AdapsVcselModeUninitilized,
    AdapsVcselModeOn,
    AdapsVcselModeOff,
} AdapsVcselMode;

struct AdapsAdvisedType
{
    UINT8 AdvisedMeasurementType;
    UINT8 AdvisedEnvironmentType;
};

typedef enum
{
    AdapsDataTypeUninitilized,
    AdapsDataTypeDepth,
    AdapsDataTypeGrayscale,
} AdapsDataType;// output data format

typedef enum {
    AdapsSensorNotChange,       // the role of tof sensor will be determined by the register configure of ads6401_sensor.xml
    AdapsSensorForceAsMaster,
    AdapsSensorForceAsSlave
} AdapsSensorForceRole;

#endif 

