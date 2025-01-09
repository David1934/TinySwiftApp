#ifndef FrameProcessThread_H
#define FrameProcessThread_H

#include <QThread>
#include <QImage>
#include <QDebug>
#include"v4l2.h"
#if defined(RUN_ON_ROCKCHIP)
#include "adaps_dtof.h"
#endif
#include"utils.h"

class FrameProcessThread : public QThread
{
    Q_OBJECT
public:
    FrameProcessThread();
    ~FrameProcessThread();

    QImage majorImage;
    void stop(int stop_request_code);
    void mode_switch(QString sensortype);
    int init(int index);
    bool isSleeping();

protected:
    void run();

private:
    volatile int majorindex;
    volatile bool stopped;
    volatile bool sleeping;
    volatile bool skip_frame_process;
    struct sensor_params sns_param;

#if defined(RUN_ON_ROCKCHIP)
    ADAPS_DTOF *adaps_dtof;
#endif
    Utils *utils;
    V4L2 *v4l2;
    u16 *depth_buffer;
    unsigned char *rgb_buffer;
    int stop_req_code;
    bool save_frame(unsigned int frm_sequence, void *frm_buf, int buf_size, int frm_w, int frm_h, struct timeval frm_timestamp, enum frame_data_type);
    void save_depth_txt_file(void *frm_buf,unsigned int frm_sequence,int frm_len);

private slots:
    bool new_frame_handle(unsigned int frm_sequence, void *frm_buf, int buf_len, struct timeval frm_timestamp, enum frame_data_type, int total_bytes_per_line);
    bool info_update(status_params1 param1);
    void onThreadLoopExit();

signals:
    void newFrameReady4Display(QImage image);
    bool update_runtime_display(status_params2 param2);
    void threadLoopExit();
    void threadEnd(int exit_request_code);
};

#endif // FrameProcessThread_H
