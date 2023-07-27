#ifndef MajorImageProcessingThread_H
#define MajorImageProcessingThread_H

#include <QThread>
#include <QImage>
#include <QDebug>
#include"v4l2.h"
#include "adaps_dtof.h"

class MajorImageProcessingThread : public QThread
{
    Q_OBJECT
public:
    MajorImageProcessingThread();

    QImage majorImage;
    void stop();
    void init(int index);

protected:
    void run();

private:
    volatile int majorindex;
    volatile bool stopped;
    struct sensor_params sns_param;
    ADAPS_DTOF *adaps_dtof;
    V4L2 *v4l2;
    u16 *depth_buffer;
    unsigned char *rgb_buffer;
#if defined(USE_CALLBACK_4_NEW_FRAME_PROCESS)
    // 声明一个回调函数
    static bool save_frame(unsigned int frm_sequence, void *frm_buf, int frm_len, struct timeval frm_timestamp, enum frame_data_type);
#endif

private slots:
#if !defined(USE_CALLBACK_4_NEW_FRAME_PROCESS)
    bool save_frame(unsigned int frm_sequence, void *frm_buf, int frm_len, struct timeval frm_timestamp, enum frame_data_type);
    bool new_frame_handle(unsigned int frm_sequence, void *frm_buf, int buf_len, struct timeval frm_timestamp, enum frame_data_type);
#endif

signals:
    void SendMajorImageProcessing(QImage image);
};

#endif // MajorImageProcessingThread_H
