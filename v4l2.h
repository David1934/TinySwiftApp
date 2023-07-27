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

#define ENTITY_NAME_4_DTOF_SENSOR   "m00_b_ads6401 7-005e"

#define MEDIA_DEVNAME_4_DTOF_SENSOR "/dev/media2"
#define VIDEO_DEV_4_DTOF_SENSOR     "/dev/video22"
#define PIXELFORMAT_4_DTOF_SENSOR   V4L2_PIX_FMT_SBGGR8
#define BUFFER_COUNT_4_DTOF_SENSOR  6

#define MEDIA_DEVNAME_4_RGB_SENSOR  "/dev/media0"
#define VIDEO_DEV_4_RGB_SENSOR      "/dev/video0"
#define VIDEO_DEV_4_RGB_RK35XX      "/dev/video55"
#define BUFFER_COUNT_4_RGB_SENSOR   5
#define RGB_IMAGE_CHANEL            3

#define DEV_NODE_LEN                32
#define FMT_NUM_PLANES              1
#define DATA_SAVE_PATH              "/sdcard/" // "/home/david/"

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

    void nv12_2_rgb(unsigned char *nv12, unsigned char *rgb, int width, int height);
    void yuyv_2_rgb(unsigned char *yuyv, unsigned char *rgb, int width, int height);
    bool Initilize(void);
    bool Start_streaming(void);
    bool Capture_frame();
    void Stop_streaming(void);
    void Close(void);
#if defined(USE_CALLBACK_4_NEW_FRAME_PROCESS)
    void Set_new_frame_callback(bool(*func) (unsigned int frm_sequence, void *frm_buf, int frm_len, struct timeval frm_timestamp, enum frame_data_type ftype));
#endif
    void Get_output_frame_size(int *in_width, int *in_height, int *out_width, int *out_height);
    int adaps_readTemperatureOfDtofSubdev(float *temperature);
    void* adaps_getEEPROMData(void);

private:
    enum frame_data_type frm_type;
    int fd;
    struct buffer *buffers;

    struct v4l2_requestbuffers	req_bufs;
    enum	v4l2_buf_type	buf_type;
    unsigned int	pixel_format;

    char        sensor_sd_name[DEV_NODE_LEN];
    char        media_dev[DEV_NODE_LEN];
    char        video_dev[DEV_NODE_LEN];
    struct sensor_params tof_param;
    int         frame_buffer_count;
    struct sensor_data *sensordata;

    char        sd_devnode_4_dtof[DEV_NODE_LEN];
    int         fd_4_dtof;
    struct adaps_get_eeprom *p_eeprominfo;

    int init();
    int adaps_readEEPROMData(void);
    bool save_eeprom(void *buf, int len);
    int adaps_setParam4DtofSubdev(void);
    bool alloc_buffers(void);
    void free_buffers(void);
    int get_devnode_from_sysfs(struct media_entity_desc *entity_desc, char *p_devname);
    int get_subdev_node_4_sensor();
#if defined(USE_CALLBACK_4_NEW_FRAME_PROCESS)
    bool(*m_new_frame_cb)(unsigned int frm_sequence, void *frm_buf, int frm_len, struct timeval frm_timestamp, enum frame_data_type ftype);
#else

signals:
    void new_frame_process(unsigned int frm_sequence, void *frm_buf, int frm_len, struct timeval frm_timestamp, enum frame_data_type ftype);
#endif
};

#endif // V4L2_H
