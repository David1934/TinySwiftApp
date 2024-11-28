#include <mainwindow.h>
#include <globalapplication.h>

#include "majorimageprocessingthread.h"
#include "utils.h"

MajorImageProcessingThread::MajorImageProcessingThread()
{
    stopped = true;
    majorindex = -1;
    rgb_buffer = NULL;
    depth_buffer = NULL;
    sns_param.sensor_type = qApp->get_sensor_type();
    sns_param.save_frame_cnt = qApp->get_save_cnt();

    if (SENSOR_TYPE_DTOF == sns_param.sensor_type)
    {
        sns_param.work_mode = qApp->get_wk_mode();
        sns_param.env_type = AdapsEnvTypeIndoor;
        if (WK_DTOF_FHR == sns_param.work_mode)
        {
            sns_param.measure_type = AdapsMeasurementTypeFull;
        }
        else {
            sns_param.measure_type = AdapsMeasurementTypeNormal;
        }

        utils = new Utils();
        v4l2 = new V4L2(sns_param);
        v4l2->Get_frame_size_4_curr_wkmode(&sns_param.raw_width, &sns_param.raw_height, &sns_param.out_frm_width, &sns_param.out_frm_height);
        DBG_INFO( "raw_width: %d raw_height: %d FRAME_INTERVAL: %d ms\n", sns_param.raw_width, sns_param.raw_height, FRAME_INTERVAL);
        adaps_dtof = new ADAPS_DTOF(sns_param, v4l2);
    }
    else {
        sns_param.work_mode = qApp->get_wk_mode();
        v4l2 = new V4L2(sns_param);
        v4l2->Get_frame_size_4_curr_wkmode(&sns_param.raw_width, &sns_param.raw_height, &sns_param.out_frm_width, &sns_param.out_frm_height);
        adaps_dtof = NULL;
    }
    connect(v4l2, SIGNAL(new_frame_process(unsigned int, void *, int, struct timeval, enum frame_data_type, int)),
            this, SLOT(new_frame_handle(unsigned int, void *, int, struct timeval, enum frame_data_type, int)), Qt::DirectConnection);

    connect(v4l2, SIGNAL(update_info(int, unsigned long)),  this, SLOT(info_update(int, unsigned long)));
    connect(this, SIGNAL(threadLoopExit()), this, SLOT(onThreadLoopExit()));
}

void MajorImageProcessingThread::set_skip_frame_process(bool val)
{
    skip_frame_process = val;

    return;
}

void MajorImageProcessingThread::mode_switch(QString mode_type)
{
    if(!mode_type.compare("RGB"))
    {
        sns_param.sensor_type = SENSOR_TYPE_RGB;
        sns_param.work_mode = WKMODE_4_RGB_SENSOR;
        sns_param.env_type = AdapsEnvTypeUninitilized;
        sns_param.measure_type = AdapsMeasurementTypeUninitilized;
        v4l2->mode_switch(sns_param);
        v4l2->Get_frame_size_4_curr_wkmode(&sns_param.raw_width, &sns_param.raw_height, &sns_param.out_frm_width, &sns_param.out_frm_height);
        if (NULL != adaps_dtof) adaps_dtof->release();
    }
    else
    {
        sns_param.sensor_type = SENSOR_TYPE_DTOF;
        sns_param.env_type = AdapsEnvTypeIndoor;
        if(!mode_type.compare("PHR"))
        {
            sns_param.work_mode = WK_DTOF_PHR;
        }
        else if(!mode_type.compare("PCM"))
        {
            sns_param.work_mode = WK_DTOF_PCM;
        }
        else if(!mode_type.compare("FHR"))
        {
            sns_param.work_mode = WK_DTOF_FHR;
        }

        if (WK_DTOF_FHR == sns_param.work_mode)
        {
            sns_param.measure_type = AdapsMeasurementTypeFull;
        }
        else {
            sns_param.measure_type = AdapsMeasurementTypeNormal;
        }

        v4l2->mode_switch(sns_param);
        v4l2->Get_frame_size_4_curr_wkmode(&sns_param.raw_width, &sns_param.raw_height, &sns_param.out_frm_width, &sns_param.out_frm_height);
        if (NULL != adaps_dtof)
        {
            adaps_dtof->mode_switch(sns_param, v4l2);
        }
    }

}

void MajorImageProcessingThread::save_depth_txt_file(void *frm_buf,unsigned int frm_sequence,int frm_len)
{
    Q_UNUSED(frm_sequence);
    QDateTime       localTime = QDateTime::currentDateTime();
    QString         currentTime = localTime.toString("yyyyMMddhhmmss");
    char *          LocalTimeStr = (char *) currentTime.toStdString().c_str();

    char path[50]={0};
    int16_t *p_temp=(int16_t*)frm_buf;

    sprintf(path,"%sframe%03d_%s_%d_depth16%s",DATA_SAVE_PATH,frm_sequence, LocalTimeStr, frm_len, ".txt");
    FILE*fp = NULL;
    fp=fopen(path, "w+");
    if (fp == NULL)
    {
        DBG_INFO("fopen output file %s failed!\n",  path);
        return;
    }
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
    DBG_INFO("Save depth file %s success!\n",  path);

}

bool MajorImageProcessingThread::save_frame(unsigned int frm_sequence, void *frm_buf, int buf_size, int frm_w, int frm_h, struct timeval frm_timestamp, enum frame_data_type ftype)
{
    const char extName[FDATA_TYPE_COUNT][24]   = {
                                    ".nv12",
                                    ".yuyv",
                                    ".rgb888",
                                    ".raw_grayscale",
                                    ".raw_depth",
                                    ".decoded_grayscale",
                                    ".decoded_depth16"
                                };

    Q_UNUSED(frm_timestamp);

    QDateTime       localTime = QDateTime::currentDateTime();
    QString         currentTime = localTime.toString("yyyyMMddhhmmss");
    char *          LocalTimeStr = (char *) currentTime.toStdString().c_str();
    char *          filename = new char[128];

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

bool MajorImageProcessingThread::info_update(int fps, unsigned long streamed_time)
{
    emit update_runtime_display(fps, streamed_time);

    return true;
}

bool MajorImageProcessingThread::new_frame_handle(unsigned int frm_sequence, void *frm_rawdata, int buf_len, struct timeval frm_timestamp, enum frame_data_type ftype, int total_bytes_per_line)
{
    int decodeRet = 0;
    int test_pattern_index = 0;
    static int run_times = 0;

    Q_UNUSED(frm_sequence);
    Q_UNUSED(frm_timestamp);
    Q_UNUSED(buf_len);

#if 0
    DBG_INFO("skipFrameProcess:%d, frm_sequence:%d, buf_len:%d", skip_frame_process, frm_sequence, buf_len);
#endif

    switch (ftype) {
        case FDATA_TYPE_DTOF_RAW_GRAYSCALE:
            if (adaps_dtof)
            {
                test_pattern_index = Utils::get_env_var_intvalue(ENV_VAR_TEST_PATTERN_TYPE);
                if (0 == run_times)
                {
                    DBG_NOTICE("test_pattern_index: %d, test_pattern_buffer_length: %d, work_mode: %d, to_capture_frame_cnt: %d...\n",
                        test_pattern_index, buf_len, sns_param.work_mode, qApp->get_save_cnt());
                }

                if ((test_pattern_index >= ETP_00_TO_FF) && (test_pattern_index <= ETP_FULL_FF))
                {
                    utils->test_pattern_generate((unsigned char *) frm_rawdata, buf_len, test_pattern_index);
                    if (0 == run_times)
                    {
                        utils->hexdump((unsigned char *) frm_rawdata, 64, "1st 64 bytes of TEST PATTERN");
                    }
                }
                run_times++;

                if (sns_param.save_frame_cnt > 0)
                {
                    if (true == Utils::is_env_var_true(ENV_VAR_SAVE_FRAME_ENABLE))
                    {
                        save_frame(frm_sequence,frm_rawdata,buf_len,
                            sns_param.raw_width, sns_param.raw_height,
                            frm_timestamp, FDATA_TYPE_DTOF_RAW_GRAYSCALE);
                    }
                }

                if (skip_frame_process)
                {
                    decodeRet = -EINVAL;
                }
                else {
                    decodeRet = adaps_dtof->dtof_decode((unsigned char *)frm_rawdata,depth_buffer,sns_param.work_mode);
                }

                if (0 == decodeRet)
                {
                    if (sns_param.save_frame_cnt > 0)
                    {
                        if (true == Utils::is_env_var_true(ENV_VAR_SAVE_FRAME_ENABLE))
                        {
                            save_frame(frm_sequence,depth_buffer,sns_param.out_frm_width*sns_param.out_frm_height*sizeof(u16),
                                sns_param.out_frm_width, sns_param.out_frm_height,
                                frm_timestamp, FDATA_TYPE_DTOF_DECODED_GRAYSCALE);
                        }
                    }
                    adaps_dtof->ConvertGreyscaleToColoredMap(depth_buffer,rgb_buffer,sns_param.out_frm_width,sns_param.out_frm_height);
                    if (sns_param.save_frame_cnt > 0)
                    {
                        if (true == Utils::is_env_var_true(ENV_VAR_SAVE_FRAME_ENABLE))
                        {
                            save_frame(frm_sequence,rgb_buffer,sns_param.out_frm_width*sns_param.out_frm_height*RGB_IMAGE_CHANEL,
                                sns_param.out_frm_width, sns_param.out_frm_height,
                                frm_timestamp, FDATA_TYPE_RGB888);
                        }
                    }
                }
                else {
                    DBG_ERROR("dtof_decode() return %d , errno: %s (%d), save_frame_cnt: %d...", decodeRet, strerror(errno), errno, sns_param.save_frame_cnt);
                }
            }
            break;

        case FDATA_TYPE_DTOF_RAW_DEPTH:
            if (adaps_dtof)
            {
                //DBG_INFO("frm_sequence:%d, buf_len:%d, save_frame_cnt:%d", frm_sequence, buf_len, sns_param.save_frame_cnt);
                if (true == utils->is_replay_data_exist())
                {
                    QByteArray buffer = utils->loadNextFileToBuffer();
                    if (buffer.isEmpty()) {
                    }
                    else {
                        std::string stdStr = buffer.toStdString();
                        memcpy(frm_rawdata, (void *) stdStr.c_str(), buf_len);
                        //frm_rawdata = (void *) stdStr.c_str();
                    }
                }

                if (sns_param.save_frame_cnt > 0)
                {
                    if (true == Utils::is_env_var_true(ENV_VAR_SAVE_FRAME_ENABLE))
                    {
#if 0
                        save_frame(frm_sequence,frm_rawdata,buf_len,
                            sns_param.raw_width, sns_param.raw_height,
                            frm_timestamp, FDATA_TYPE_DTOF_RAW_DEPTH);
#else
                        // swift test pattern output, the first 2 lines/packets have some variable values, so I'm ignoring them.
                        save_frame(frm_sequence,
                            frm_rawdata + total_bytes_per_line *2,
                            buf_len - total_bytes_per_line *2,
                            sns_param.raw_width, sns_param.raw_height,
                            frm_timestamp, FDATA_TYPE_DTOF_RAW_DEPTH);
#endif
                    }
                }

                if (skip_frame_process)
                {
                    decodeRet = -EINVAL;
                }
                else {
                    decodeRet = adaps_dtof->dtof_decode((unsigned char *)frm_rawdata,depth_buffer,sns_param.work_mode);
                }

                if (0 == decodeRet)
                {
                    if (sns_param.save_frame_cnt > 0)
                    {
                        if (true == Utils::is_env_var_true(ENV_VAR_SAVE_FRAME_ENABLE))
                        {
                            save_frame(frm_sequence,depth_buffer,sns_param.out_frm_width*sns_param.out_frm_height*sizeof(u16),
                                sns_param.out_frm_width, sns_param.out_frm_height,
                                frm_timestamp, FDATA_TYPE_DTOF_DECODED_DEPTH16);
                        }

                        if (Utils::is_env_var_true(ENV_VAR_SAVE_DEPTH_TXT_ENABLE))
                        {
                            save_depth_txt_file(depth_buffer, frm_sequence, sns_param.out_frm_width*sns_param.out_frm_height*sizeof(u16));
                        }
                    }
                    adaps_dtof->ConvertDepthToColoredMap(depth_buffer,rgb_buffer,sns_param.out_frm_width,sns_param.out_frm_height);
                    if (sns_param.save_frame_cnt > 0)
                    {
                        if (true == Utils::is_env_var_true(ENV_VAR_SAVE_FRAME_ENABLE))
                        {
                            save_frame(frm_sequence,rgb_buffer,sns_param.out_frm_width*sns_param.out_frm_height*RGB_IMAGE_CHANEL,
                                sns_param.out_frm_width, sns_param.out_frm_height,
                                frm_timestamp, FDATA_TYPE_RGB888);
                        }
                    }
                }
                else {
                    //DBG_ERROR("dtof_decode() return %d , errno: %s (%d), save_frame_cnt: %d...", decodeRet, strerror(errno), errno, sns_param.save_frame_cnt);
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
            decodeRet = -EINVAL;
            break;
    }
    if (0 == decodeRet)
    {
        QImage img = QImage(rgb_buffer, sns_param.out_frm_width, sns_param.out_frm_height, sns_param.out_frm_width*RGB_IMAGE_CHANEL,QImage::Format_RGB888);
        emit newFrameReady4Display(img);
    }
    if (sns_param.save_frame_cnt > 0)
    {
        sns_param.save_frame_cnt--;
    }

    return true;
}

void MajorImageProcessingThread::onThreadLoopExit()
{
    if (v4l2)
    {
        v4l2->Stop_streaming();
        v4l2->Close();
    }
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

    emit threadEnd(stop_req_code);
}

void MajorImageProcessingThread::stop(int stop_request_code)
{
    stop_req_code = stop_request_code;
    if(!stopped)
    {
        stopped = true;
    }
    else {
        emit threadLoopExit();
    }
}

int MajorImageProcessingThread::init(int index)
{
    int ret = 0;
    ret = v4l2->V4l2_initilize();
    if (ret < 0)
    {
        //ui->mainlabel->clear();
        //ui->mainlabel->setText("No camera detected.");
        DBG_ERROR("Fail to v4l2->V4l2_initilize(),ret:%d, errno: %s (%d)...", ret,
            strerror(errno), errno);
        return ret;
    }
    else {
        if (SENSOR_TYPE_DTOF == sns_param.sensor_type)
        {
            if (NULL == adaps_dtof)
            {
                DBG_ERROR("adaps_dtof is NULL" );
                return 0 - __LINE__;
            }

            ret = adaps_dtof->adaps_dtof_initilize();
            if (0 > ret)
            {
                //ui->mainlabel->clear();
                //ui->mainlabel->setText("No camera detected.");
                DBG_ERROR("Fail to adaps_dtof->adaps_dtof_initilize(),ret:%d, errno: %s (%d)...", 
                    ret, strerror(errno), errno);
                return 0 - __LINE__;
            }
            else {
                //ui->mainlabel->clear();
                //ui->mainlabel->setText("Camera is ready for use.");
                depth_buffer = (u16 *)malloc(sizeof(u16)*sns_param.out_frm_width*sns_param.out_frm_height);
            }
        }
        rgb_buffer = (unsigned char *)malloc(sizeof(unsigned char)*sns_param.out_frm_width*sns_param.out_frm_height*RGB_IMAGE_CHANEL);
        ret = v4l2->Start_streaming();
        if (0 == ret)
        {
            stopped = false;
            majorindex = index;
        }
    }

    return ret;
}

void MajorImageProcessingThread::run()
{
    int ret = 0;
    if(majorindex != -1)
    {
        while(!stopped)
        {
            msleep(FRAME_INTERVAL);

            if(!stopped)
            {
                ret=v4l2->Capture_frame();
            }

            if(ret < 0)
            {
                stopped=true;
                //stop(STOP_REQUEST_QUIT);
            }
            else {
                if ((0 != qApp->get_save_cnt()) && (0 == sns_param.save_frame_cnt)) // if already capture expected frames, try to quit.
                {
                    stop(STOP_REQUEST_STOP);
                }
            }
        }

        emit threadLoopExit();
    }
}

