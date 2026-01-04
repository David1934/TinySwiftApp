#ifndef DTOF_MAIN_H
#define DTOF_MAIN_H


#include <fcntl.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <semaphore.h>
#include <pthread.h>

#include "common.h"
#include"utils.h"
#include"v4l2.h"
#include "adaps_dtof.h"
#include"misc_device.h"

#define PTHREAD_INVALID 0

class DToF_Main
{

public:
    DToF_Main(struct sensor_params sensor_param);
    ~DToF_Main();
    void stop();

    // static 互斥锁（保护共享变量，static 确保全局唯一）
    static pthread_mutex_t mutex;

private:
    volatile uint32_t dump_spot_statistics_times;
    volatile uint32_t dump_ptm_frame_headinfo_times;
    char *expected_md5_string;

    struct sensor_params sns_param;
#if ALGO_LIB_VERSION_CODE >= VERSION_HEX_VALUE(3, 5, 6) && defined(ENABLE_POINTCLOUD_OUTPUT)
    pc_pkt_t* out_pcloud_buffer;
    u32 out_pcloud_buffer_size;
#endif
    u16 *depth_buffer;
    u32 depth_buffer_size;
    uint32_t outputed_frame_cnt;
    uint32_t dumped_frame_cnt;

    Misc_Device *misc_dev;
    Utils *utils = NULL_POINTER;
    V4L2 *v4l2 = NULL_POINTER;
    ADAPS_DTOF *adaps_dtof = NULL_POINTER;

#if defined(EEPROM_PARAMETER_FETCH_DEMO)
    void* p_eeprominfo = NULL_POINTER;
#endif

    pthread_t frame_process_thread_id = PTHREAD_INVALID;

    static void* FrameProcessThread(void* arg);
    void stopFrameProcessThread();

    bool frame_decode(   struct frame_decode_param *param);
};

#endif // DTOF_MAIN_H
