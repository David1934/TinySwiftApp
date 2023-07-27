#include "majorimageprocessingthread.h"

MajorImageProcessingThread::MajorImageProcessingThread()
{
    stopped = false;
    majorindex = -1;
    sns_param.sensor_type = SELECTED_SENSOR_TYPE;
    sns_param.save_frame_cnt = SAVE_FRAME_CNT;

    if (SENSOR_TYPE_DTOF == sns_param.sensor_type)
    {
        sns_param.work_mode = SELECTED_WORK_MODE;
        sns_param.env_type = AdapsEnvTypeIndoor;
        sns_param.measure_type = AdapsMeasurementTypeNormal;
        v4l2 = new V4L2(sns_param);
        v4l2->Get_output_frame_size(&sns_param.raw_width, &sns_param.raw_height, &sns_param.out_frm_width, &sns_param.out_frm_height);
        DBG_INFO( "raw_width: %d raw_height: %d FRAME_INTERVAL: %d ms\n", sns_param.raw_width, sns_param.raw_height, FRAME_INTERVAL);
        adaps_dtof = new ADAPS_DTOF(sns_param, v4l2);
    }
    else {
        sns_param.work_mode = SELECTED_WORK_MODE;
        v4l2 = new V4L2(sns_param);
        v4l2->Get_output_frame_size(&sns_param.raw_width, &sns_param.raw_height, &sns_param.out_frm_width, &sns_param.out_frm_height);
        adaps_dtof = NULL;
    }
#if defined(USE_CALLBACK_4_NEW_FRAME_PROCESS)
    v4l2->Set_new_frame_callback(save_frame);
#else
    //connect(v4l2, SIGNAL(new_frame_process(unsigned int, void *, int, struct timeval, enum frame_data_type)),
    //    this, SLOT(save_frame(unsigned int, void *, int, struct timeval, enum frame_data_type)), Qt::DirectConnection);
    connect(v4l2, SIGNAL(new_frame_process(unsigned int, void *, int, struct timeval, enum frame_data_type)),
            this, SLOT(new_frame_handle(unsigned int, void *, int, struct timeval, enum frame_data_type)), Qt::DirectConnection);
#endif
}

bool MajorImageProcessingThread::save_frame(unsigned int frm_sequence, void *frm_buf, int frm_len, struct timeval frm_timestamp, enum frame_data_type ftype)
{
    const char extName[FDATA_TYPE_COUNT][16]   = {
                                    ".nv12",
                                    ".yuyv",
                                    ".rgb",
                                    ".gray_raw",
                                    ".depth_raw",
                                    ".depth16"
                                };

    Q_UNUSED(frm_sequence);
    Q_UNUSED(frm_timestamp);

    QDateTime       localTime = QDateTime::currentDateTime();
    QString         currentTime = localTime.toString("yyyyMMddhhmmss");
    char *          LocalTimeStr = (char *) currentTime.toStdString().c_str();
    char *          filename = new char[50];
    sprintf(filename,"%sframe%03d_%s_%d%s", DATA_SAVE_PATH, frm_sequence, LocalTimeStr, frm_len, extName[ftype]);
    FILE *fp = fopen(filename, "wb");

    if (fp == NULL) {
        DBG_ERROR("Fail to create file %s , errno: %s (%d)...", 
            filename, strerror(errno), errno);
        return false;
    }

    fwrite(frm_buf, frm_len, 1, fp);
    delete[] filename;
    fclose(fp);

    return true;
}

bool MajorImageProcessingThread::new_frame_handle(unsigned int frm_sequence, void *frm_rawdata, int buf_len, struct timeval frm_timestamp, enum frame_data_type ftype)
{
    int decodeRet;

    Q_UNUSED(frm_sequence);
    Q_UNUSED(frm_timestamp);
    Q_UNUSED(buf_len);
#if 0 //defined(DEBUG)
    DBG_INFO("frm_sequence:%d, buf_len:%d, out_frm_width:%d, out_frm_height:%d", frm_sequence, buf_len, sns_param.out_frm_width, sns_param.out_frm_height);
#endif

    switch (ftype) {
        case FDATA_TYPE_DTOF_GRAYSCALE:
            if (adaps_dtof)
            {
                if (sns_param.save_frame_cnt > 0)
                {
                    save_frame(frm_sequence,frm_rawdata,buf_len,frm_timestamp, FDATA_TYPE_DTOF_GRAYSCALE);
                }
                decodeRet = adaps_dtof->dtof_decode((unsigned char *)frm_rawdata,depth_buffer,WK_DTOF_PCM);
                if (0 == decodeRet)
                {
                    if (sns_param.save_frame_cnt > 0)
                    {
                        save_frame(frm_sequence,depth_buffer,sns_param.out_frm_width*sns_param.out_frm_height*sizeof(u16),frm_timestamp, FDATA_TYPE_DTOF_DEPTH16);
                    }
                    adaps_dtof->ConvertGreyscaleToColoredMap(depth_buffer,rgb_buffer,sns_param.out_frm_width,sns_param.out_frm_height);
                    if (sns_param.save_frame_cnt > 0)
                    {
                        save_frame(frm_sequence,rgb_buffer,sns_param.out_frm_width*sns_param.out_frm_height*RGB_IMAGE_CHANEL,frm_timestamp, FDATA_TYPE_RGB);
                    }
                }
                else {
                    return false; // don't show the frame until a complete image frame is decoded.
                }
            }
            break;

        case FDATA_TYPE_DTOF_DEPTH:
            if (adaps_dtof)
            {
                //DBG_INFO("frm_sequence:%d, buf_len:%d, save_frame_cnt:%d", frm_sequence, buf_len, sns_param.save_frame_cnt);
                if (sns_param.save_frame_cnt > 0)
                {
                    save_frame(frm_sequence,frm_rawdata,buf_len,frm_timestamp, FDATA_TYPE_DTOF_DEPTH);
                }
#if 1
                decodeRet = adaps_dtof->dtof_decode((unsigned char *)frm_rawdata,depth_buffer,WK_DTOF_PHR);
                if (0 == decodeRet)
                {
                    if (sns_param.save_frame_cnt > 0)
                    {
                        save_frame(frm_sequence,depth_buffer,sns_param.out_frm_width*sns_param.out_frm_height*sizeof(u16),frm_timestamp, FDATA_TYPE_DTOF_DEPTH16);
                    }
                    adaps_dtof->ConvertDepthToColoredMap(depth_buffer,rgb_buffer,sns_param.out_frm_width,sns_param.out_frm_height);
                    if (sns_param.save_frame_cnt > 0)
                    {
                        save_frame(frm_sequence,rgb_buffer,sns_param.out_frm_width*sns_param.out_frm_height*RGB_IMAGE_CHANEL,frm_timestamp, FDATA_TYPE_RGB);
                    }
                }
                else {
                    return false; // don't show the frame until a complete image frame is decoded.
                }
#endif
            }
            break;

        case FDATA_TYPE_YUYV:
            if (v4l2)
            {
                v4l2->yuyv_2_rgb((unsigned char *)frm_rawdata,rgb_buffer,sns_param.out_frm_width,sns_param.out_frm_height);
            }
            break;

        case FDATA_TYPE_NV12:
            if (v4l2)
            {
                v4l2->nv12_2_rgb((unsigned char *)frm_rawdata,rgb_buffer,sns_param.out_frm_width,sns_param.out_frm_height);
            }
        break;

        default:
            DBG_ERROR("Unsupported input frame type (%d)...", ftype);
            break;
    }
    if (sns_param.save_frame_cnt > 0)
    {
        sns_param.save_frame_cnt--;
    }
    QImage img = QImage(rgb_buffer, sns_param.out_frm_width, sns_param.out_frm_height, QImage::Format_RGB888);
    emit SendMajorImageProcessing(img);

    return true;
}

void MajorImageProcessingThread::stop()
{
    stopped = true;
    v4l2->Stop_streaming();
    v4l2->Close();
    if (SENSOR_TYPE_DTOF == sns_param.sensor_type)
    {
        adaps_dtof->release();
        free(depth_buffer);
    }
    free(rgb_buffer);
}

void MajorImageProcessingThread::init(int index)
{
    bool ret = 0;
    stopped = false;
    majorindex = index;
    ret = v4l2->Initilize();
    if (false == ret)
    {
        //ui->mainlabel->clear();
        //ui->mainlabel->setText("No camera detected.");
        DBG_ERROR("Fail to v4l2->Initilize(), errno: %s (%d)...", 
            strerror(errno), errno);
        return;
    }
    else {
        if (SENSOR_TYPE_DTOF == sns_param.sensor_type)
        {
            if (-1 == adaps_dtof->initilize())
            {
                //ui->mainlabel->clear();
                //ui->mainlabel->setText("No camera detected.");
                DBG_ERROR("Fail to adaps_dtof->initilize(), errno: %s (%d)...", 
                    strerror(errno), errno);
                return;
            }
            else {
                //ui->mainlabel->clear();
                //ui->mainlabel->setText("Camera is ready for use.");
                depth_buffer = (u16 *)malloc(sizeof(u16)*sns_param.out_frm_width*sns_param.out_frm_height);
            }
        }
        rgb_buffer = (unsigned char *)malloc(sizeof(unsigned char)*sns_param.out_frm_width*sns_param.out_frm_height*RGB_IMAGE_CHANEL);
        ret = v4l2->Start_streaming();
    }
}

void MajorImageProcessingThread::run()
{
    if(majorindex != -1)
    {
        while(!stopped)
        {
            msleep(FRAME_INTERVAL);

            v4l2->Capture_frame();
        }
    }
}

