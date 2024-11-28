#ifndef MajorImageProcessingThread_H
#define MajorImageProcessingThread_H

#include <QThread>
#include <QImage>
#include <QDebug>
#include"v4l2.h"
#include "adaps_dtof.h"
#include"utils.h"

class MajorImageProcessingThread : public QThread
{
    Q_OBJECT
public:
    MajorImageProcessingThread();

    QImage majorImage;
    void stop(int stop_request_code);
    void mode_switch(QString sensortype);
    int init(int index);
    void set_skip_frame_process(bool val);

protected:
    void run();

private:
    volatile int majorindex;
    volatile bool stopped;
    volatile bool skip_frame_process;
    struct sensor_params sns_param;

    ADAPS_DTOF *adaps_dtof;
    Utils *utils;
    V4L2 *v4l2;
    u16 *depth_buffer;
    unsigned char *rgb_buffer;
    int stop_req_code;
    bool save_frame(unsigned int frm_sequence, void *frm_buf, int buf_size, int frm_w, int frm_h, struct timeval frm_timestamp, enum frame_data_type);
    void save_depth_txt_file(void *frm_buf,unsigned int frm_sequence,int frm_len);

private slots:
    bool new_frame_handle(unsigned int frm_sequence, void *frm_buf, int buf_len, struct timeval frm_timestamp, enum frame_data_type, int total_bytes_per_line);
    bool info_update(int fps, unsigned long streamed_time);
    void onThreadLoopExit();

signals:
    void newFrameReady4Display(QImage image);
    bool update_runtime_display(int fps, unsigned long streamed_time);
    void threadLoopExit();
    void threadEnd(int exit_request_code);
};

#endif // MajorImageProcessingThread_H
