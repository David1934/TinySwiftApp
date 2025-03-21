#ifndef V4L2_H
#define V4L2_H

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

#include "common.h"

#define BUFFER_COUNT_4_DTOF_SENSOR  6
#define OUTPUT_WIDTH_4_DTOF_SENSOR  210
#define OUTPUT_HEIGHT_4_DTOF_SENSOR 160

#define MIPI_RAW_HEIGHT_4_DTOF_SENSOR 32

#define BUFFER_COUNT_4_RGB_SENSOR   5
#define RGB_IMAGE_CHANEL            3

#define DEV_NODE_LEN                32
#define FMT_NUM_PLANES              1

#if defined(RUN_ON_ROCKCHIP)
#if (ADS6401_MODDULE_SPOT == SWIFT_MODULE_TYPE)
inline void EepromGetSwiftDeviceNumAddress(uint32_t* offset, uint32_t* length) {
    *offset = OFFSET(swift_eeprom_data_t, deviceName);
    *length = MEMBER_SIZE(swift_eeprom_data_t, deviceName);
}

inline void EepromGetSwiftSramDataAddress(uint32_t* offset, uint32_t* length) {
    *offset = OFFSET(swift_eeprom_data_t, sramData);
    *length = MEMBER_SIZE(swift_eeprom_data_t, sramData);
}

inline void EepromGetSwiftIntrinsicAddress(uint32_t* offset, uint32_t* length) {
    *offset = OFFSET(swift_eeprom_data_t, intrinsic);
    *length = MEMBER_SIZE(swift_eeprom_data_t, intrinsic);
}

inline void EepromGetSwiftSpotPosAddress(uint32_t* offset, uint32_t* length) {
    *offset = OFFSET(swift_eeprom_data_t, spotPos);
    *length = MEMBER_SIZE(swift_eeprom_data_t, spotPos);
}

inline void EepromGetSwiftOutDoorOffsetAddress(uint32_t* offset, uint32_t* length) {
    *offset = OFFSET(swift_eeprom_data_t, spotPos);
    *length = MEMBER_SIZE(swift_eeprom_data_t, spotPos) / 2;
}

inline void EepromGetSwiftSpotOffsetbAddress(uint32_t* offset, uint32_t* length) {
    *offset = OFFSET(swift_eeprom_data_t, spotPos) + SWIFT_OFFSET_SIZE * sizeof(float);
    *length = MEMBER_SIZE(swift_eeprom_data_t, spotPos) / 2;
}

inline void EepromGetSwiftSpotOffsetaAddress(uint32_t* offset, uint32_t* length) {
    *offset = OFFSET(swift_eeprom_data_t, spotOffset);
    *length = MEMBER_SIZE(swift_eeprom_data_t, spotOffset);
}

inline void EepromGetSwiftSpotOffsetAddress(uint32_t* offset, uint32_t* length) {
    *offset = OFFSET(swift_eeprom_data_t, spotOffset);
    *length = MEMBER_SIZE(swift_eeprom_data_t, spotOffset);
}

inline void EepromGetSwiftTdcDelayAddress(uint32_t* offset, uint32_t* length) {
    *offset = OFFSET(swift_eeprom_data_t, tdcDelay);
    *length = MEMBER_SIZE(swift_eeprom_data_t, tdcDelay);
}

inline void EepromGetSwiftRefDistanceAddress(uint32_t* offset, uint32_t* length) {
    *offset = OFFSET(swift_eeprom_data_t, indoorCalibTemperature);
    *length = MEMBER_SIZE(swift_eeprom_data_t, indoorCalibTemperature)
        + MEMBER_SIZE(swift_eeprom_data_t, indoorCalibRefDistance)
        + MEMBER_SIZE(swift_eeprom_data_t, outdoorCalibTemperature)
        + MEMBER_SIZE(swift_eeprom_data_t, outdoorCalibRefDistance)
        + MEMBER_SIZE(swift_eeprom_data_t, calibrationInfo);
}

inline void EepromGetSwiftCalibInfoAddress(uint32_t* offset, uint32_t* length) {
    *offset = OFFSET(swift_eeprom_data_t, indoorCalibTemperature);
    *length = MEMBER_SIZE(swift_eeprom_data_t, indoorCalibTemperature)
        + MEMBER_SIZE(swift_eeprom_data_t, indoorCalibRefDistance)
        + MEMBER_SIZE(swift_eeprom_data_t, outdoorCalibTemperature)
        + MEMBER_SIZE(swift_eeprom_data_t, outdoorCalibRefDistance);
}

inline void EepromGetSwiftPxyAddress(uint32_t* offset, uint32_t* length) {
    *offset = OFFSET(swift_eeprom_data_t, pxyHistogram);
    *length = MEMBER_SIZE(swift_eeprom_data_t, pxyHistogram)
        + MEMBER_SIZE(swift_eeprom_data_t, pxyDepth)
        + MEMBER_SIZE(swift_eeprom_data_t, pxyNumberOfPulse);
}

inline void EepromGetSwiftMarkedPixelAddress(uint32_t* offset, uint32_t* length) {
    *offset = OFFSET(swift_eeprom_data_t, markedPixels);
    *length = MEMBER_SIZE(swift_eeprom_data_t, markedPixels);
}

inline void EepromGetSwiftModuleInfoAddress(uint32_t* offset, uint32_t* length) {
    *offset = OFFSET(swift_eeprom_data_t, moduleInfo);
    *length = MEMBER_SIZE(swift_eeprom_data_t, moduleInfo);
}

inline void EepromGetSwiftWalkErrorAddress(uint32_t* offset, uint32_t* length) {
    *offset = OFFSET(swift_eeprom_data_t, WalkError);
    *length = MEMBER_SIZE(swift_eeprom_data_t, WalkError);
}

inline void EepromGetSwiftSpotEnergyAddress(uint32_t* offset, uint32_t* length) {
    *offset = OFFSET(swift_eeprom_data_t, SpotEnergy);
    *length = MEMBER_SIZE(swift_eeprom_data_t, SpotEnergy);
}

inline void EepromGetSwiftRawDepthAddress(uint32_t* offset, uint32_t* length) {
    *offset = OFFSET(swift_eeprom_data_t, RawDepthMean);
    *length = MEMBER_SIZE(swift_eeprom_data_t, RawDepthMean);
}

inline void EepromGetSwiftNoiseAddress(uint32_t* offset, uint32_t* length) {
    *offset = OFFSET(swift_eeprom_data_t, noise);
    *length = MEMBER_SIZE(swift_eeprom_data_t, noise);
}

inline void EepromGetSwiftChecksumAddress(uint32_t* offset, uint32_t* length) {
    *offset = OFFSET(swift_eeprom_data_t, checksum);
    *length = MEMBER_SIZE(swift_eeprom_data_t, checksum);
}
#endif
#endif

struct buffer
{
    void *start;
    unsigned int length;
};

struct sensor_data
{
    char *sensor_subdev;
    char *media_devnode;
    char *video_devnode;
    int raw_w;
    int raw_h;
    int out_frm_width;
    int out_frm_height;
    int pixfmt;
    int frm_buf_cnt; 
    enum sensortype stype;
    enum frame_data_type ftype;
};

class V4L2 : public QObject
{
Q_OBJECT

public:
    V4L2(struct sensor_params params);
    ~V4L2();

    int V4l2_initilize(void);
    int Start_streaming(void);
    int Capture_frame();
    void V4l2_mode_switch(struct sensor_params params);
    void Stop_streaming(void);
    void V4l2_close(void);
    void Get_frame_size_4_curr_wkmode(int *in_width, int *in_height, int *out_width, int *out_height);
#if defined(RUN_ON_ROCKCHIP)
    int V4l2_get_dtof_runtime_status_param(float *temperature);
    void* V4l2_get_dtof_calib_eeprom_param(void);
    void* V4l2_get_dtof_exposure_param(void);
#endif

private:
    enum frame_data_type frm_type;
    int fd;
    struct buffer *buffers;
    unsigned long firstFrameTimeUsec;
    unsigned long rxFrameCnt;
    int mipi_rx_fps;
    unsigned long streamed_timeUs;
    void *mapped_eeprom_buffer;
    int eeprom_data_size;

    struct v4l2_requestbuffers	req_bufs;
    enum	v4l2_buf_type	buf_type;
    unsigned int	pixel_format;
    int bits_per_pixel;
    int padding_bytes_per_line;
    int total_bytes_per_line;
    int payload_bytes_per_line;

    char        sensor_sd_name[DEV_NODE_LEN];
    char        media_dev[DEV_NODE_LEN];
    char        video_dev[DEV_NODE_LEN];
    struct sensor_params snr_param;
    int         frame_buffer_count;
    struct sensor_data *sensordata;

#if defined(RUN_ON_ROCKCHIP)
    char        sd_devnode_4_dtof[DEV_NODE_LEN];
    int         fd_4_dtof;
    char        devnode_4_misc[DEV_NODE_LEN];
    int         fd_4_misc;
    swift_eeprom_data_t *p_eeprominfo;
    unsigned int    last_temperature;
    unsigned int    last_expected_vop_abs_x100;
    unsigned int    last_expected_pvdd_x100;
#endif

    int init();
#if defined(RUN_ON_ROCKCHIP)
    int get_dtof_exposure_param(void);
    int get_dtof_calib_eeprom_param(void);
    int check_crc32_4_dtof_calib_eeprom_param(void);
    bool save_dtof_calib_eeprom_param(void *buf, int len);
    int set_dtof_initial_param(void);
    bool check_crc_4_eeprom_item(uint8_t *pEEPROMData, uint32_t offset, uint32_t length, uint8_t savedCRC, char *tag);
#endif
    bool alloc_buffers(void);
    void free_buffers(void);
    int get_devnode_from_sysfs(struct media_entity_desc *entity_desc, char *p_devname);
    int get_subdev_node_4_sensor();
    int set_param_4_sensor_sub_device(int raw_w_4_curr_wkmode, int raw_h_4_curr_wkmode);

signals:
    void new_frame_process(unsigned int frm_sequence, void *frm_buf, int frm_len, struct timeval frm_timestamp, enum frame_data_type ftype, int total_bytes_per_line);
    void update_info(status_params1 param1);
};

#endif // V4L2_H
