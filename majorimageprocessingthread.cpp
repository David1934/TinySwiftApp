#include <mainwindow.h>
#include <globalapplication.h>

#include "majorimageprocessingthread.h"
#include "utils.h"

MajorImageProcessingThread::MajorImageProcessingThread()
{
    stopped = false;
    majorindex = -1;
    rgb_buffer = NULL;
    depth_buffer = NULL;
    sns_param.sensor_type = qApp->get_sensor_type();
    sns_param.save_frame_cnt = qApp->get_save_cnt();

    if (SENSOR_TYPE_DTOF == sns_param.sensor_type)
    {
        sns_param.work_mode = qApp->get_wk_mode();
        sns_param.env_type = AdapsEnvTypeIndoor;
        sns_param.measure_type = AdapsMeasurementTypeNormal;
        v4l2 = new V4L2(sns_param);
        v4l2->Get_output_frame_size(&sns_param.raw_width, &sns_param.raw_height, &sns_param.out_frm_width, &sns_param.out_frm_height);
        DBG_INFO( "raw_width: %d raw_height: %d FRAME_INTERVAL: %d ms\n", sns_param.raw_width, sns_param.raw_height, FRAME_INTERVAL);
        adaps_dtof = new ADAPS_DTOF(sns_param, v4l2);
    }
    else {
        sns_param.work_mode = qApp->get_wk_mode();
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

void MajorImageProcessingThread::change(QString sensortype)
{
    if(!sensortype.compare("RGB"))
    {
        sns_param.sensor_type = SENSOR_TYPE_RGB;
        sns_param.work_mode = WK_RGB_NV12;
        sns_param.env_type = AdapsEnvTypeUninitilized;
        sns_param.measure_type = AdapsMeasurementTypeUninitilized;
        v4l2->change(sns_param);
        v4l2->Get_output_frame_size(&sns_param.raw_width, &sns_param.raw_height, &sns_param.out_frm_width, &sns_param.out_frm_height);
        if (NULL != adaps_dtof) adaps_dtof->release();
    }
    else
    {
        sns_param.sensor_type = SENSOR_TYPE_DTOF;
        sns_param.env_type = AdapsEnvTypeIndoor;
        sns_param.measure_type = AdapsMeasurementTypeNormal;
        if(!sensortype.compare("PHR"))
        {
            sns_param.work_mode = WK_DTOF_PHR;
        }
        else if(!sensortype.compare("PCM"))
        {
            sns_param.work_mode = WK_DTOF_PCM;
        }
        else if(!sensortype.compare("FHR"))
        {
            sns_param.work_mode = WK_DTOF_FHR;
        }
        v4l2->change(sns_param);
        v4l2->Get_output_frame_size(&sns_param.raw_width, &sns_param.raw_height, &sns_param.out_frm_width, &sns_param.out_frm_height);
        if (NULL != adaps_dtof) adaps_dtof->change(sns_param, v4l2);
    }

}

void MajorImageProcessingThread::save_depth(void *frm_buf,unsigned int frm_sequence,int frm_len)
{
    Q_UNUSED(frm_sequence);
    QDateTime       localTime = QDateTime::currentDateTime();
    QString         currentTime = localTime.toString("yyyyMMddhhmmss");
    char *          LocalTimeStr = (char *) currentTime.toStdString().c_str();

    char path[50]={0};
    int16_t *p_temp=(int16_t*)frm_buf;
    if (Utils::is_env_var_true(ENV_VAR_SAVE_DEPTH_ENABLE))
    {
        sprintf(path,"%sframe%03d_%s_%d%s",DATA_SAVE_PATH,frm_sequence, LocalTimeStr, frm_len, ".txt");
        FILE*fp = NULL;
        fp=fopen(path, "w+");
        if (fp == NULL)
        {
            DBG_INFO(" fopen output file %s failed!\n",  path);
            return;
        }
        printf(">\n");
        for (int i = 0; i < OUTPUT_HEIGHT_4_DTOF_SENSOR; i++)
        {
            int offset = i * OUTPUT_WIDTH_4_DTOF_SENSOR;
            for (int j = 0; j < OUTPUT_WIDTH_4_DTOF_SENSOR; j++)
            {
                fprintf(fp, "%6u ", (*(p_temp + offset + j))&0x1fff);  //do not printf high 3 bit confidence
            }
            fprintf(fp, "\n");
        }
        fflush(fp);
        fclose(fp);
   }

}

bool MajorImageProcessingThread::save_frame(unsigned int frm_sequence, void *frm_buf, int buf_size, int frm_w, int frm_h, struct timeval frm_timestamp, enum frame_data_type ftype)
{
    const char extName[FDATA_TYPE_COUNT][16]   = {
                                    ".nv12",
                                    ".yuyv",
                                    ".rgb",
                                    ".gray_raw",
                                    ".depth_raw",
                                    ".depth16"
                                };

    Q_UNUSED(frm_timestamp);

    QDateTime       localTime = QDateTime::currentDateTime();
    QString         currentTime = localTime.toString("yyyyMMddhhmmss");
    char *          LocalTimeStr = (char *) currentTime.toStdString().c_str();
    char *          filename = new char[128];

    if (false == Utils::is_env_var_true(ENV_VAR_SAVE_FRAME_ENABLE))
    {
        return false;
    }

    sprintf(filename,"%sframe%03d_%dx%d_%s_%d%s", DATA_SAVE_PATH, frm_sequence, frm_w, frm_h,LocalTimeStr, buf_size, extName[ftype]);
    FILE *fp = fopen(filename, "wb");

    if (fp == NULL) {
        DBG_ERROR("Fail to create file %s , errno: %s (%d)...", 
            filename, strerror(errno), errno);
        return false;
    }

    fwrite(frm_buf, buf_size, 1, fp);
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
                    save_frame(frm_sequence,frm_rawdata,buf_len,
                        sns_param.raw_width, sns_param.raw_height,
                        frm_timestamp, FDATA_TYPE_DTOF_GRAYSCALE);
                }
                decodeRet = adaps_dtof->dtof_decode((unsigned char *)frm_rawdata,depth_buffer,sns_param.work_mode);
                if (0 == decodeRet)
                {
                    if (sns_param.save_frame_cnt > 0)
                    {
                        save_frame(frm_sequence,depth_buffer,sns_param.out_frm_width*sns_param.out_frm_height*sizeof(u16),
                            sns_param.out_frm_width, sns_param.out_frm_height,
                            frm_timestamp, FDATA_TYPE_DTOF_DEPTH16);
                    }
                    adaps_dtof->ConvertGreyscaleToColoredMap(depth_buffer,rgb_buffer,sns_param.out_frm_width,sns_param.out_frm_height);
                    if (sns_param.save_frame_cnt > 0)
                    {
                        save_frame(frm_sequence,rgb_buffer,sns_param.out_frm_width*sns_param.out_frm_height*RGB_IMAGE_CHANEL,
                            sns_param.out_frm_width, sns_param.out_frm_height,
                            frm_timestamp, FDATA_TYPE_RGB);
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
                    save_frame(frm_sequence,frm_rawdata,buf_len,
                        sns_param.raw_width, sns_param.raw_height,
                        frm_timestamp, FDATA_TYPE_DTOF_DEPTH);
                }

                decodeRet = adaps_dtof->dtof_decode((unsigned char *)frm_rawdata,depth_buffer,sns_param.work_mode);
                if (0 == decodeRet)
                {
                    if (sns_param.save_frame_cnt > 0)
                    {
                        save_frame(frm_sequence,depth_buffer,sns_param.out_frm_width*sns_param.out_frm_height*sizeof(u16),
                            sns_param.out_frm_width, sns_param.out_frm_height,
                            frm_timestamp, FDATA_TYPE_DTOF_DEPTH16);
                    }
                    adaps_dtof->ConvertDepthToColoredMap(depth_buffer,rgb_buffer,sns_param.out_frm_width,sns_param.out_frm_height);
                    if (sns_param.save_frame_cnt > 0)
                    {
                        save_frame(frm_sequence,rgb_buffer,sns_param.out_frm_width*sns_param.out_frm_height*RGB_IMAGE_CHANEL,
                            sns_param.out_frm_width, sns_param.out_frm_height,
                            frm_timestamp, FDATA_TYPE_RGB);
                    }
                }
                else {
                    return false; // don't show the frame until a complete image frame is decoded.
                }
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
    QImage img = QImage(rgb_buffer, sns_param.out_frm_width, sns_param.out_frm_height, sns_param.out_frm_width*RGB_IMAGE_CHANEL,QImage::Format_RGB888);
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
/*
        if (adaps_dtof)
        {
            adaps_dtof->release();
        }
*/
        if (NULL != depth_buffer)
        {
            free(depth_buffer);
            depth_buffer = NULL;
        }
    }
    if (NULL != rgb_buffer)
    {
        free(rgb_buffer);
        rgb_buffer = NULL;
    }
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
            if ((NULL == adaps_dtof) || (-1 == adaps_dtof->initilize()))
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
    bool ret ;
    if(majorindex != -1)
    {
        while(!stopped)
        {
            msleep(FRAME_INTERVAL);

            if(!stopped)
            {
                ret=v4l2->Capture_frame();
            }
            if(ret==false)
            {
                stopped=true;
            }
        }
    }
}

