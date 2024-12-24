/*
 * Copyright (C) 2023-2024 Adaps Photonics Inc.
 */

#ifndef _ADAPS_DTOF_UAPI_H
#define _ADAPS_DTOF_UAPI_H

#if defined(RUN_ON_ROCKCHIP)
#include <linux/types.h>
#endif

#define ADAPS_DTOF_PRIVATE              0x1000

#define ADAPS_ROI_SRAM_DATA_CNT         2

#define RKMODULE_MAX_SENSOR_MODE        10
#define RKMODULE_MODE_NAME_MAX          8

#define SOC_PLATFORM_ROCKCHIP           0x0000
#define SOC_PLATFORM_RK3568             (SOC_PLATFORM_ROCKCHIP + 1)

#define SOC_PLATFORM_TI                 0x1000
#define SOC_PLATFORM_TI_AM62A           (SOC_PLATFORM_TI + 1)





#if !defined(BIT)
#define BIT(n)                  (1 << n)
#endif

#if 1 //def __KERNEL__
#if defined(CONFIG_VIDEO_ADS6311)  // FOR ADAPS_HAWK

struct rkmodule_sensor_mode {
    char modename[RKMODULE_MODE_NAME_MAX];
    __u32 image_width;
    __u32 image_height;
    __u16 userdata_width;
    __u16 userdata_height;
    __u32 image_datatype;
    __u32 userdata_datatype;
} __attribute__ ((packed));

struct rkmodule_sensor_module_inf {
    struct rkmodule_sensor_mode  sensor_mode[RKMODULE_MAX_SENSOR_MODE];
    __u8 mode_size;
} __attribute__ ((packed));

typedef struct {
    __u16 addr;
    __u16 val;
} reg16_data16_t;

typedef struct {
    __u16 addr;
    __u8 reserved;
    __u8 val;
} reg16_data8_t;

#if defined(NEWER_ADS6311_DRV_COWORK_WITH_OLDER_APP) // if use the new ads6311.c driver co-work with old user space App whose release version <= v3.2.0
typedef struct {
    __u32 workMode;
    __u16 subFrameNum;
    __u16 regSet1;
    __u16 regSet2;
    __u16 roiSram;
} enable_script_t;

#else

typedef struct {
    __u32 workMode;
    __u16 subFrameNum;
    __u16 regSet1;
    __u16 regSet2;
    __u16 roiSram[ADAPS_ROI_SRAM_DATA_CNT];
} enable_script_t;
#endif

typedef enum hawk_soc_board_version {
    HAWK_SOC_BOARD_VER_UNKNOWN = 0,
    HAWK_SOC_BOARD_VER_DEMO3   = SOC_PLATFORM_RK3568,       // 0x0001
    HAWK_SOC_BOARD_VER_MINI_DEMO4,

    HAWK_SOC_BOARD_VER_TI_AM62A   = SOC_PLATFORM_TI_AM62A,  // 0x1001
}hawk_soc_board_version_t;

typedef enum hawk_chip_eco_version {
    HAWK_CHIP_ECO_VER_UNKNOWN = 0,
    HAWK_CHIP_ECO_VER_NTO,  // NewTapeOut
    HAWK_CHIP_ECO_VER_ECO1,
    HAWK_CHIP_ECO_VER_ECO2,
    HAWK_CHIP_ECO_VER_ECO3,
}hawk_chip_eco_version_t;

typedef enum hawk_lens_type {
    HAWK_LENS_WIDEANGLE = 0,
    HAWK_LENS_TELEPHOTO,

    HAWK_LENS_COUNT
}hawk_lens_t;

typedef enum hawk_vcsel_op_code {
    HAWK_VCSEL_OP_UNININT = 0,
    HAWK_VCSEL_OP_START = 1,
    HAWK_VCSEL_OP_STOP = 2,
}hawk_vcsel_op_code_t;

enum hawk_rx_error_type_t {
    HAWK_RX_FSM_ALARM               = BIT(0),  // internal state-machine mismatch alarm
    HAWK_RX_WDT_ALARM               = BIT(1),  // watch-dog timer overflow alarm
    HAWK_RX_DV11_ALARM              = BIT(2),  // DV11 voltage abnormal alarm
    HAWK_RX_AV11_ALARM              = BIT(3),  // AV11 voltage abnormal alarm
    HAWK_RX_SV11_ALARM              = BIT(4),  // SV11 voltage abnormal alarm
    HAWK_RX_AV33_ALARM              = BIT(5),  // DV33 voltage abnormal alarm
    HAWK_RX_NOT_APPLIED6            = BIT(6),  // not applied
    HAWK_RX_NOT_APPLIED7            = BIT(7),  // not applied

    HAWK_RX_NOT_APPLIED8            = BIT(8),  // not applied
    HAWK_RX_NOT_APPLIED9            = BIT(9),  // not applied
    HAWK_RX_TRIGO0_ALARM            = BIT(10),  // triger o0 alarm
    HAWK_RX_TRIGO1_ALARM            = BIT(11),  // triger o1 alarm
    HAWK_RX_TRIGO2_ALARM            = BIT(12),  // triger o2 alarm
    HAWK_RX_TRIGO3_ALARM            = BIT(13),  // triger o3 alarm
    HAWK_RX_DRV_ERR_ALARM           = BIT(14),  // tx driver error alarm
    HAWK_RX_RESERVED                = BIT(15),  // reserved

    HAWK_RX_LOW_TEMP_OTP_DATA_ERR   = BIT(16),  // Low temperature otp parameter has some error
    HAWK_RX_NORMAL_OTP_DATA_ERR     = BIT(17),  // Normal otp parameter has some error
    HAWK_RX_HIGH_OTP_DATA_ERR       = BIT(18),  // High temperature otp parameter has some error
};

enum {
    HAWK_OTP_TEMPERATURE_PARAM_LOW,
    HAWK_OTP_TEMPERATURE_PARAM_ROOM,
    HAWK_OTP_TEMPERATURE_PARAM_HIGH,
    HAWK_OTP_TEMPERATURE_PARAM_COUNT,
};

enum hawk_otp_checksum_state{
    HAWK_OTP_CHECKSUM_UNSAVED,     // the value is full 0
    HAWK_OTP_CHECKSUM_MATCHED,
    HAWK_OTP_CHECKSUM_MISMATCHED,
};

struct hawk_otp_temperature_param
{
    __u8 chksum_state;              // refer to the enum hawk_otp_checksum_state
    __u16 vbe_code;
    __u16 vbr_voltage;              // value x10 mv
    __u8 temperature;               // low temperature use signed 8-bit integer, while room and high temperature use unsigned temperature
}__attribute__ ((packed));

struct hawk_otp_key_data
{
    u16 chip_eco_version;
    struct hawk_otp_temperature_param temperature_param[HAWK_OTP_TEMPERATURE_PARAM_COUNT];
}__attribute__ ((packed));

struct hawk_norflash_op_param
{
    __u32 op_code; //0:read,1:write
    __u32 offset;
    __u32 len;
}__attribute__ ((packed));

#define ADTOF_ENABLE_STREAM_NUM      \
    _IOW('T', ADAPS_DTOF_PRIVATE + 2, __u32 *)

#define ADTOF_ENABLE_SCRIPT_START      \
    _IOW('T', ADAPS_DTOF_PRIVATE + 3, enable_script_t *)

#define ADTOF_SET_VCSEL_CHANNEL      \
    _IOW('T', ADAPS_DTOF_PRIVATE + 4, __u16 *)

#define ADTOF_SET_SENSOR_REGISTER       \
    _IOW('T', ADAPS_DTOF_PRIVATE + 5, reg16_data8_t *)

#define ADTOF_GET_SENSOR_REGISTER       \
    _IOR('T', ADAPS_DTOF_PRIVATE + 6, reg16_data8_t *)

#define ADTOF_SET_MCU_REGISTER       \
    _IOW('T', ADAPS_DTOF_PRIVATE + 7, reg16_data16_t *)

#define ADTOF_GET_MCU_REGISTER       \
    _IOR('T', ADAPS_DTOF_PRIVATE + 8, reg16_data16_t *)

#define ADTOF_GET_DRIVER_VERSION      \
    _IOR('T', ADAPS_DTOF_PRIVATE + 9, __u32 *)

#define ADTOF_GET_MCU_RUNNING_FW_TYPE \
    _IOW('T', ADAPS_DTOF_PRIVATE + 10, __u16 *)             // 0x0A

#define ADTOF_GET_MCUFIRMWARE_VERSION \
    _IOR('T', ADAPS_DTOF_PRIVATE + 11, __u16 *)             // 0x0B

#define ADTOF_LET_MCU_ENTER_BOOTLOADER      \
    _IO('T', ADAPS_DTOF_PRIVATE + 12)                       // 0x0C

#define ADTOF_SET_MCU_FW_CRC32      \
    _IOW('T', ADAPS_DTOF_PRIVATE + 13, __u32 *)             // 0x0D

#define ADTOF_SET_MCU_FW_LENGTH      \
    _IOW('T', ADAPS_DTOF_PRIVATE + 14, __u32 *)             // 0x0E

#define ADTOF_GET_BLOCK_SIZE_4_FLASH_WRITE      \
    _IOR('T', ADAPS_DTOF_PRIVATE + 15, __u32 *)             // 0x0F

#define ADTOF_TRANSFER_DATA_WITH_OFFSET      \
    _IOW('T', ADAPS_DTOF_PRIVATE + 16, __u32 *)             // 0x10

#define ADTOF_GET_FLASH_WRITE_STATUS \
    _IOW('T', ADAPS_DTOF_PRIVATE + 17, __u16 *)             // 0x11

#define ADTOF_SET_FLASH_WRITE_STATUS \
    _IOW('T', ADAPS_DTOF_PRIVATE + 18, __u16 *)             // 0x12

#define ADTOF_GET_MCU_FW_UPGRADE_STATUS \
    _IOW('T', ADAPS_DTOF_PRIVATE + 19, __u16 *)             // 0x13

#define ADTOF_LET_MCU_REBOOT      \
    _IO('T', ADAPS_DTOF_PRIVATE + 20)                       // 0x14

#define ADTOF_GET_MCUHARDWARE_VERSION \
    _IOR('T', ADAPS_DTOF_PRIVATE + 21, __u16 *)             // 0x15

//for hawk sensor norflash data
#define ADTOF_NORFLASH_OPERATION       \
    _IOW('T', ADAPS_DTOF_PRIVATE + 22, struct hawk_norflash_op_param *)     // 0x16

#define ADTOF_VCSEL_OPERATION       \
    _IOW('T', ADAPS_DTOF_PRIVATE + 23, __u32 *)             // 0x17

#define ADTOF_SET_VOP_VOLTAGE      \
    _IOW('T', ADAPS_DTOF_PRIVATE + 24, __s16 *)             // 0x18

#define ADTOF_SET_PVDD_VOLTAGE      \
    _IOW('T', ADAPS_DTOF_PRIVATE + 25, __u16 *)             // 0x19

#define ADTOF_GET_TX_TEMPERATURE      \
    _IOR('T', ADAPS_DTOF_PRIVATE + 26, __u16 *)             // 0x1A

#define ADTOF_GET_RX_TEMPERATURE      \
    _IOR('T', ADAPS_DTOF_PRIVATE + 27, __s16 *)             // 0x1B

#define ADTOF_GET_HAWK_CHIP_ECO_VERSION      \
    _IOR('T', ADAPS_DTOF_PRIVATE + 28, __u16 *)             // 0x1C

#define ADTOF_GET_HAWK_CHIP_ASC_ALARM      \
    _IOR('T', ADAPS_DTOF_PRIVATE + 29, __u32 *)             // 0x1D

#define ADTOF_GET_HAWK_OTP_KEY_DATA       \
    _IOR('T', ADAPS_DTOF_PRIVATE + 30, struct hawk_otp_key_data *)          // 0x1E

#define ADTOF_SET_LENS_TYPE      \
    _IOW('T', ADAPS_DTOF_PRIVATE + 31, __u8 *)              // 0x1F

#define ADTOF_SWITCH_ROI_SRAM      \
    _IO('T', ADAPS_DTOF_PRIVATE + 32)                       // 0x20

#define ADTOF_GET_SOC_BOARD_VERSION      \
    _IOR('T', ADAPS_DTOF_PRIVATE + 33, __u16 *)             // 0x21

#endif // FOR ADAPS_HAWK






//==========================================================================
#if defined(CONFIG_VIDEO_ADS6401)  // FOR ADAPS_SWIFT

// built-in EEPROM P24C64E-C4H-MIR, VcselDriver OPN7020, and external VOP chip ADS5142
#define ADS6401_MODDULE_SPOT                0x6401A

// built-in EEPROM P24C256F-D4H-MIR, VcselDriver PhotonIC 5015, MCU HC32L110B6YA, 
// and external PWM based VOP chip TPS61170DRVR
#define ADS6401_MODDULE_FLOOD               0x6401B

#if defined(CONFIG_ADAPS_SWIFT_FLOOD)
    #define SWIFT_MODULE_TYPE               ADS6401_MODDULE_FLOOD
#else
    #define SWIFT_MODULE_TYPE               ADS6401_MODDULE_SPOT
#endif

#include "adaps_types.h"

// There are two group Calibration SRAM for spod address, every group has 4 calibration registers.
#define CALIB_SRAM_REG_BASE0                0xFB
#define CALIB_SRAM_REG_BASE1                0xF7
#define CSRU_SRAM_OFFSET_REG                0xC7

#define PER_ZONE_MAX_SPOT_COUNT             240

#define PER_CALIB_SRAM_ZONE_SIZE            512     //unit is bytes, actual use size is 480 bytes, the remaining 32 bytes is 0
#define ZONE_COUNT_PER_SRAM_GROUP           4
#define CALIB_SRAM_GROUP_COUNT              2

#define PER_ROISRAM_GROUP_SIZE              (PER_CALIB_SRAM_ZONE_SIZE * ZONE_COUNT_PER_SRAM_GROUP) //unit is bytes

#define ALL_ROISRAM_GROUP_SIZE              (PER_CALIB_SRAM_ZONE_SIZE * ZONE_COUNT_PER_SRAM_GROUP * CALIB_SRAM_GROUP_COUNT)

#define SWIFT_MAX_SPOT_COUNT                (PER_ZONE_MAX_SPOT_COUNT * ZONE_COUNT_PER_SRAM_GROUP * CALIB_SRAM_GROUP_COUNT)

#define SWIFT_DEVICE_NAME_LENGTH            64

#define OFFSET(structure, member)           ((uintptr_t)&((structure*)0)->member)
#define MEMBER_SIZE(structure, member)      sizeof(((structure*)0)->member)

#if (ADS6401_MODDULE_SPOT == SWIFT_MODULE_TYPE)
#define SWIFT_OFFSET_SIZE                   960
#pragma pack(4)
typedef struct SwiftEepromData
{
    // In Swift EEPROM, one page has 64 bytes.
    // Page 1
    char            deviceName[SWIFT_DEVICE_NAME_LENGTH];
    // Page 2 - 65
    unsigned char   sramData[ALL_ROISRAM_GROUP_SIZE];
    // Page 66
    float           intrinsic[9];     // 36 bytes
    float           offset;           // 4 bytes
    __u32           refSpadSelection; // 4 bytes
    __u32           driverChannelOffset[4];  // 16 bytes
    __u32           distanceTemperatureCoefficient;
    // Page 67 - 186
    float           spotPos[SWIFT_MAX_SPOT_COUNT]; // 7680 bytes, 120 pages
    // Page 187 - 246
    float           spotOffset[SWIFT_OFFSET_SIZE]; // 3840 bytes, 60 pages
    // Page 247
    __u32           tdcDelay[16]; // 8 bytes
    // Page 248
    float           indoorCalibTemperature; // Calibration temperature.
    float           indoorCalibRefDistance; // Calibration reference distance.
    float           outdoorCalibTemperature; // Calibration temperature.
    float           outdoorCalibRefDistance; // Calibration reference distance.
    float           calibrationInfo[12]; // reserved
    // Page 249 - 252 
    uint8_t         pxyHistogram[248]; // pxy histogram
    float           pxyDepth;          // pxy depth
    float           pxyNumberOfPulse;  // pxy number of pulse
    // Page 253
    uint16_t        markedPixels[32];  // 16 hot pixels, 16 dead pixels.
    // Page 254
    char            moduleInfo[64]; // adaps calibration info.
#if 1
    // Page 255
    char            reservedPage255[SWIFT_DEVICE_NAME_LENGTH];
    // Page 256
    __u32           totalChecksum;
    __u32           sramDataChecksum;
    __u32           spotPositionChecksum;
    __u32           spotOffsetChecksum;
    __u32           proximityChecksum;
#else // to be synced with SpadisPC
    //Page  255-256
    char            reserved1[MODULE_INFO_RESERVED_SIZE];
    //Page  257-622
    char            WalkError[SWIFT_WALKERROR_SIZE];
    //Page  623-736
    char            reserved2[WALKERROR_RESERVED_SIZE];
    //Page  737-796           
    float           SpotEnergy[SWIFT_SPOTENERGY_SIZE];
    //Page  796-1023
    char            noise[SWIFT_NOISE_SIZE];
    //Page  1024
    uint8_t         checksum[SWIFT_CHECKSUM_SIZE];
#endif
}swift_eeprom_data_t;

#define  AD4001_EEPROM_VERSION_INFO_OFFSET                  OFFSET(swift_eeprom_data_t, deviceName)               /// 0, 0x00
#define  AD4001_EEPROM_VERSION_INFO_SIZE                    MEMBER_SIZE(swift_eeprom_data_t, deviceName)

// please refer to data struct 'swift_eeprom_data_t' in SpadisPC
#define  AD4001_EEPROM_ROISRAM_DATA_OFFSET                  OFFSET(swift_eeprom_data_t, sramData)               /// 64, 0x40
#define  AD4001_EEPROM_ROISRAM_DATA_SIZE                    MEMBER_SIZE(swift_eeprom_data_t, sramData)

#define  AD4001_EEPROM_INTRINSIC_OFFSET                     OFFSET(swift_eeprom_data_t, intrinsic)              /// 4160 0x1040
#define  AD4001_EEPROM_INTRINSIC_SIZE                       MEMBER_SIZE(swift_eeprom_data_t, intrinsic)         /// 9xsizeof(float)

#define  AD4001_EEPROM_ACCURATESPODPOS_OFFSET               OFFSET(swift_eeprom_data_t, spotPos)        /// 4224 0x1080
#define  AD4001_EEPROM_ACCURATESPODPOS_SIZE                 MEMBER_SIZE(swift_eeprom_data_t, spotPos)   /// 4x240x2xsizeof(float)=1920x4=7680

#define  AD4001_EEPROM_SPOTOFFSET_OFFSET                    OFFSET(swift_eeprom_data_t, spotOffset)             /// 11904 0x2e80
#define  AD4001_EEPROM_SPOTOFFSET_SIZE                      MEMBER_SIZE(swift_eeprom_data_t, spotOffset)        /// 4x240xsizeof(float)=960x4=3840

#define  AD4001_EEPROM_TDCDELAY_OFFSET                      OFFSET(swift_eeprom_data_t, tdcDelay)               /// 15744 0x3d80 
#define  AD4001_EEPROM_TDCDELAY_SIZE                        MEMBER_SIZE(swift_eeprom_data_t, tdcDelay)          /// 2xsizeof(uint32_t)=16x4=64

#define  AD4001_EEPROM_INDOOR_CALIBTEMPERATURE_OFFSET       OFFSET(swift_eeprom_data_t, indoorCalibTemperature)               /// 15808 0x3dc0
#define  AD4001_EEPROM_INDOOR_CALIBTEMPERATURE_SIZE         MEMBER_SIZE(swift_eeprom_data_t, indoorCalibTemperature)          /// 1xsizeof(float)=1x4=4

#define  AD4001_EEPROM_INDOOR_CALIBREFDISTANCE_OFFSET       OFFSET(swift_eeprom_data_t, indoorCalibRefDistance)               /// 15812 0x3dc4
#define  AD4001_EEPROM_INDOOR_CALIBREFDISTANCE_SIZE         MEMBER_SIZE(swift_eeprom_data_t, indoorCalibRefDistance)          /// 1xsizeof(float)=1x4=4

#define  AD4001_EEPROM_OUTDOOR_CALIBTEMPERATURE_OFFSET      OFFSET(swift_eeprom_data_t, outdoorCalibTemperature)               /// 15816
#define  AD4001_EEPROM_OUTDOOR_CALIBTEMPERATURE_SIZE        MEMBER_SIZE(swift_eeprom_data_t, outdoorCalibTemperature)          /// 1xsizeof(float)=1x4=4

#define  AD4001_EEPROM_OUTDOOR_CALIBREFDISTANCE_OFFSET      OFFSET(swift_eeprom_data_t, outdoorCalibRefDistance)               /// 15820
#define  AD4001_EEPROM_OUTDOOR_CALIBREFDISTANCE_SIZE        MEMBER_SIZE(swift_eeprom_data_t, outdoorCalibRefDistance)          /// 1xsizeof(float)=1x4=4

#define  AD4001_EEPROM_PROX_HISTOGRAM_OFFSET                OFFSET(swift_eeprom_data_t, pxyHistogram)               /// 15824+12x(sizeof(float))=15872
#define  AD4001_EEPROM_PROX_HISTOGRAM_SIZE                  MEMBER_SIZE(swift_eeprom_data_t, pxyHistogram)          /// 256

#define  AD4001_EEPROM_TOTAL_CHECKSUM_OFFSET                OFFSET(swift_eeprom_data_t, totalChecksum)
#define  AD4001_EEPROM_TOTAL_CHECKSUM_SIZE                  MEMBER_SIZE(swift_eeprom_data_t, totalChecksum)

#pragma pack()

#else  // swift FLOOD module

#define SWIFT_OFFSET_SIZE                                   1920  //960 * 2

#define EEPROM_PAGE_SIZE                                    64
#define CRC32_SPACE                                         4
#define OFFSET_NUM                                          8

//page0
#define SWIFT_DEVICE_VERSION_LENGTH                         6
#define SWIFT_DEVICE_SN_LENGTH                              16
#define PAGE0_RESERVED_SIZE                                 (EEPROM_PAGE_SIZE - SWIFT_DEVICE_VERSION_LENGTH - SWIFT_DEVICE_SN_LENGTH - CRC32_SPACE)

//page1
#define SWIFT_DEVICE_MODULEINFO_LENGTH                      60

//page2

//sram data
#define SRAM_ZONE_OCUPPY_SPACE                              512
#define SRAM_ZONE_VALID_DATA_LENGTH                         480

//offset
#define OFFSET_VALID_DATA_LENGTH                            960


#pragma pack(1)
typedef struct SwiftEepromData
{
    // In Swift EEPROM, one page has 64 bytes.
    // Page 0
    char            Version[SWIFT_DEVICE_VERSION_LENGTH];//6
    char            SerialNumber[SWIFT_DEVICE_SN_LENGTH];//16
    unsigned int    Crc32Pg0;//4
    unsigned char   Pg0Reserved[PAGE0_RESERVED_SIZE];//38

    //page 1
    char            ModuleInfo[SWIFT_DEVICE_MODULEINFO_LENGTH];//60
    unsigned int    Crc32Pg1; //4

    //page2
    unsigned char   tdcDelay[2];
    float           intrinsic[9]; // 36 bytes
    float           indoorCalibTemperature; // Calibration temperature.
    float           outdoorCalibTemperature; // Calibration temperature.
    float           indoorCalibRefDistance; // Calibration reference distance.
    float           outdoorCalibRefDistance; // Calibration reference distance.
    unsigned int    Crc32Pg2;
    unsigned char   Pg2Reserved[64-2-36-16-4]; //64-2-36-16-4=6

    //page 3---66
    unsigned char   sramData[ALL_ROISRAM_GROUP_SIZE];// 512*8 ,BUT 480(datat)+4(crc) is valid for every 512 bytes

    //page 67-186
    float           spotOffset[SWIFT_OFFSET_SIZE]; // 64*60=3840 bytes = 960*4, 60 pages  for one offset.  here we have 2 offset???

    //page 187
    unsigned int    Crc32Offset[OFFSET_NUM];
    unsigned char   Pg187Reserved[64-32]; //64-32=32
}swift_eeprom_data_t;

#define  AD4001_EEPROM_VERSION_INFO_OFFSET                  OFFSET(swift_eeprom_data_t, Version)
#define  AD4001_EEPROM_VERSION_INFO_SIZE                    MEMBER_SIZE(swift_eeprom_data_t, Version)

#define  AD4001_EEPROM_SN_INFO_OFFSET                       OFFSET(swift_eeprom_data_t, SerialNumber)
#define  AD4001_EEPROM_SN_INFO_SIZE                         MEMBER_SIZE(swift_eeprom_data_t, SerialNumber)

#define  AD4001_EEPROM_MODULE_INFO_OFFSET                   OFFSET(swift_eeprom_data_t, ModuleInfo)
#define  AD4001_EEPROM_MODULE_INFO_SIZE                     MEMBER_SIZE(swift_eeprom_data_t, ModuleInfo)


// please refer to data struct 'swift_eeprom_data_t' in SpadisPC
#define  AD4001_EEPROM_ROISRAM_DATA_OFFSET                  OFFSET(swift_eeprom_data_t, sramData)
#define  AD4001_EEPROM_ROISRAM_DATA_SIZE                    MEMBER_SIZE(swift_eeprom_data_t, sramData)

#define  AD4001_EEPROM_INTRINSIC_OFFSET                     OFFSET(swift_eeprom_data_t, intrinsic)              /// 4160 0x1040
#define  AD4001_EEPROM_INTRINSIC_SIZE                       MEMBER_SIZE(swift_eeprom_data_t, intrinsic)         /// 9xsizeof(float)

//#define  AD4001_EEPROM_ACCURATESPODPOS_OFFSET       OFFSET(swift_eeprom_data_t, spotPos)
//#define  AD4001_EEPROM_ACCURATESPODPOS_SIZE         MEMBER_SIZE(swift_eeprom_data_t, spotPos)

#define  AD4001_EEPROM_SPOTOFFSET_OFFSET                    OFFSET(swift_eeprom_data_t, spotOffset)
#define  AD4001_EEPROM_SPOTOFFSET_SIZE                      MEMBER_SIZE(swift_eeprom_data_t, spotOffset)

#define  AD4001_EEPROM_TDCDELAY_OFFSET                      OFFSET(swift_eeprom_data_t, tdcDelay)
#define  AD4001_EEPROM_TDCDELAY_SIZE                        MEMBER_SIZE(swift_eeprom_data_t, tdcDelay)

#define  AD4001_EEPROM_INDOOR_CALIBTEMPERATURE_OFFSET       OFFSET(swift_eeprom_data_t, indoorCalibTemperature)
#define  AD4001_EEPROM_INDOOR_CALIBTEMPERATURE_SIZE         MEMBER_SIZE(swift_eeprom_data_t, indoorCalibTemperature)

#define  AD4001_EEPROM_INDOOR_CALIBREFDISTANCE_OFFSET       OFFSET(swift_eeprom_data_t, indoorCalibRefDistance)
#define  AD4001_EEPROM_INDOOR_CALIBREFDISTANCE_SIZE         MEMBER_SIZE(swift_eeprom_data_t, indoorCalibRefDistance)

#define  AD4001_EEPROM_OUTDOOR_CALIBTEMPERATURE_OFFSET      OFFSET(swift_eeprom_data_t, outdoorCalibTemperature)
#define  AD4001_EEPROM_OUTDOOR_CALIBTEMPERATURE_SIZE        MEMBER_SIZE(swift_eeprom_data_t, outdoorCalibTemperature)

#define  AD4001_EEPROM_OUTDOOR_CALIBREFDISTANCE_OFFSET      OFFSET(swift_eeprom_data_t, outdoorCalibRefDistance)
#define  AD4001_EEPROM_OUTDOOR_CALIBREFDISTANCE_SIZE        MEMBER_SIZE(swift_eeprom_data_t, outdoorCalibRefDistance)

#define ONE_SPOD_OFFSET_BYTE_SIZE                           3840//240*4*4

#pragma pack()
#endif


struct adaps_set_param_in_config{
    AdapsEnvironmentType env_type;
    AdapsMeasurementType measure_type;
//    AdapsPowerMode  powermode;
    AdapsFramerateType framerate_type;
    AdapsVcselZoneCountType vcselzonecount_type;
};

struct adaps_set_param_in_runtime{
   AdapsEnvironmentType env_type;
   AdapsMeasurementType measure_type;
   AdapsVcselMode vcsel_mode;
   bool env_valid;
   bool measure_valid;
   bool vcsel_valid;
   
};

struct adaps_get_exposure_param{
    __u8 laser_exposure_period;
    __u8 fine_exposure_time;
};

struct adaps_get_param_perframe{
    //float internal_temperature;
    __u32 internal_temperature; //since kernel doesn't use float type, the temperate is a expanded integer value (x100)
    __u32 expected_vop_abs_x100;
    __u32 expected_pvdd_x100;
};

#pragma pack(4)
 struct adaps_get_eeprom{
    __u8 pRawData[sizeof(swift_eeprom_data_t)];
    __u32 rawDataSize;//may be not need
};
#pragma pack()

#define ADAPS_SET_PARAM_IN_CONFIG       \
	_IOW('T', ADAPS_DTOF_PRIVATE + 2, struct adaps_set_param_in_config)

#define ADAPS_SET_PARAM_IN_RUNTIME       \
	_IOW('T', ADAPS_DTOF_PRIVATE + 3, struct adaps_set_param_in_runtime)

#define ADAPS_GET_PARAM_PERFRAME       \
	_IOR('T', ADAPS_DTOF_PRIVATE + 4, struct adaps_get_param_perframe)

#define ADAPS_GET_EEPROM       \
	_IOR('T', ADAPS_DTOF_PRIVATE + 5, struct adaps_get_eeprom)

#define ADAPS_GET_EXPOSURE_PARAM       \
	_IOR('T', ADAPS_DTOF_PRIVATE + 6, struct adaps_get_exposure_param)


#endif // FOR ADAPS_SWIFT
#endif


#endif /* _ADAPS_DTOF_UAPI_H */

