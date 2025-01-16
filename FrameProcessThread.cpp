#include <mainwindow.h>
#include <globalapplication.h>

#include "FrameProcessThread.h"
#include "utils.h"

FrameProcessThread::FrameProcessThread()
{
    stopped = true;
    majorindex = -1;
    rgb_buffer = NULL;
    depth_buffer = NULL;
#if defined(ENABLE_DYNAMICALLY_UPDATE_ROI_SRAM_CONTENT)
    merged_depth_buffer = NULL;
#endif
    v4l2 = NULL;
    utils = NULL;
    sns_param.sensor_type = qApp->get_sensor_type();
    sns_param.save_frame_cnt = qApp->get_save_cnt();
    skip_frame_process = Utils::is_env_var_true(ENV_VAR_SKIP_FRAME_PROCESS);

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
#if defined(RUN_ON_ROCKCHIP)
        adaps_dtof = new ADAPS_DTOF(sns_param, v4l2);
#endif
    }
    else {
        sns_param.work_mode = qApp->get_wk_mode();
        v4l2 = new V4L2(sns_param);
        v4l2->Get_frame_size_4_curr_wkmode(&sns_param.raw_width, &sns_param.raw_height, &sns_param.out_frm_width, &sns_param.out_frm_height);
#if defined(RUN_ON_ROCKCHIP)
        adaps_dtof = NULL;
#endif
    }
    connect(v4l2, SIGNAL(new_frame_process(unsigned int, void *, int, struct timeval, enum frame_data_type, int)),
            this, SLOT(new_frame_handle(unsigned int, void *, int, struct timeval, enum frame_data_type, int)), Qt::DirectConnection);

    qRegisterMetaType<status_params1>("status_params1");
    connect(v4l2, SIGNAL(update_info(status_params1)),  this, SLOT(info_update(status_params1)));
    connect(this, SIGNAL(threadLoopExit()), this, SLOT(onThreadLoopExit()));
}

FrameProcessThread::~FrameProcessThread()
{
    if (NULL != utils)
    {
        delete utils;
    }

#if defined(RUN_ON_ROCKCHIP)
    if (NULL != depth_buffer)
    {
        free(depth_buffer);
        depth_buffer = NULL;
    }

#if defined(ENABLE_DYNAMICALLY_UPDATE_ROI_SRAM_CONTENT)
    if (NULL != merged_depth_buffer)
    {
        free(merged_depth_buffer);
        merged_depth_buffer = NULL;
    }
#endif

    if (NULL != adaps_dtof)
    {
        delete adaps_dtof;
    }
#endif

    if (NULL != rgb_buffer)
    {
        free(rgb_buffer);
        rgb_buffer = NULL;
    }

    if (NULL != v4l2)
    {
        delete v4l2;
    }
}

void FrameProcessThread::mode_switch(QString mode_type)
{
    if(!mode_type.compare("RGB"))
    {
        sns_param.sensor_type = SENSOR_TYPE_RGB;
        sns_param.work_mode = WKMODE_4_RGB_SENSOR;
        sns_param.env_type = AdapsEnvTypeUninitilized;
        sns_param.measure_type = AdapsMeasurementTypeUninitilized;
        v4l2->V4l2_mode_switch(sns_param);
        v4l2->Get_frame_size_4_curr_wkmode(&sns_param.raw_width, &sns_param.raw_height, &sns_param.out_frm_width, &sns_param.out_frm_height);
#if defined(RUN_ON_ROCKCHIP)
        if (NULL != adaps_dtof)
        {
            adaps_dtof->adaps_dtof_release();
        }
#endif
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

        v4l2->V4l2_mode_switch(sns_param);
        v4l2->Get_frame_size_4_curr_wkmode(&sns_param.raw_width, &sns_param.raw_height, &sns_param.out_frm_width, &sns_param.out_frm_height);
#if defined(RUN_ON_ROCKCHIP)
        if (NULL != adaps_dtof)
        {
            adaps_dtof->mode_switch(sns_param, v4l2);
        }
#endif
    }

}

void FrameProcessThread::save_depth_txt_file(void *frm_buf,unsigned int frm_sequence,int frm_len)
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

bool FrameProcessThread::save_frame(unsigned int frm_sequence, void *frm_buf, int buf_size, int frm_w, int frm_h, struct timeval frm_timestamp, enum frame_data_type ftype)
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

bool FrameProcessThread::info_update(status_params1 param1)
{
    status_params2 param2;

    param2.sensor_type = sns_param.sensor_type;
    param2.mipi_rx_fps = param1.mipi_rx_fps;
    param2.streamed_time_us = param1.streamed_time_us;
    param2.work_mode = sns_param.work_mode;
#if defined(RUN_ON_ROCKCHIP)
    param2.curr_temperature = param1.curr_temperature;
    param2.curr_exp_vop_abs = param1.curr_exp_vop_abs;
    param2.curr_exp_pvdd = param1.curr_exp_pvdd;
    param2.env_type = sns_param.env_type;
    param2.measure_type = sns_param.measure_type;
#endif

    emit update_runtime_display(param2);

    return true;
}

bool FrameProcessThread::new_frame_handle(unsigned int frm_sequence, void *frm_rawdata, int buf_len, struct timeval frm_timestamp, enum frame_data_type ftype, int total_bytes_per_line)
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
#if defined(RUN_ON_ROCKCHIP)
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
                    int i;
                    decodeRet = 0;

                    for (i = 0; i < sns_param.out_frm_width*sns_param.out_frm_height; i++)
                    {
                        rgb_buffer[i*3] = 0x0;     // R component, fill with min value 0x0
                        rgb_buffer[i*3 + 1] = 0x0;  // G component, fill with min value 0x00
                        rgb_buffer[i*3 + 2] = 0xFF;  // B component, fill with max value 0xFF
                    }
                }
                else {
                    decodeRet = adaps_dtof->dtof_frame_decode((unsigned char *)frm_rawdata,depth_buffer,sns_param.work_mode);
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
                        DBG_ERROR("dtof_frame_decode() return %d , errno: %s (%d), save_frame_cnt: %d...", decodeRet, strerror(errno), errno, sns_param.save_frame_cnt);
                    }
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
                    int i;
                    decodeRet = 0;

                    for (i = 0; i < sns_param.out_frm_width*sns_param.out_frm_height; i++)
                    {
                        rgb_buffer[i*3] = 0xFF;     // R component, fill with max value 0xFF
                        rgb_buffer[i*3 + 1] = 0x0;  // G component, fill with min value 0x00
                        rgb_buffer[i*3 + 2] = 0x0;  // B component, fill with min value 0x00
                    }
                }
                else {
                    decodeRet = adaps_dtof->dtof_frame_decode((unsigned char *)frm_rawdata, depth_buffer, sns_param.work_mode);
#if defined(ENABLE_DYNAMICALLY_UPDATE_ROI_SRAM_CONTENT)
                    int new_added_spot_count = adaps_dtof->DepthBufferMerge(merged_depth_buffer, depth_buffer, sns_param.out_frm_width, sns_param.out_frm_height);

                    if (0 == decodeRet)
                    {
                        if (sns_param.save_frame_cnt > 0)
                        {
                            if (true == Utils::is_env_var_true(ENV_VAR_SAVE_FRAME_ENABLE))
                            {
                                save_frame(frm_sequence,merged_depth_buffer,sns_param.out_frm_width*sns_param.out_frm_height*sizeof(u16),
                                    sns_param.out_frm_width, sns_param.out_frm_height,
                                    frm_timestamp, FDATA_TYPE_DTOF_DECODED_DEPTH16);
                            }

                            if (Utils::is_env_var_true(ENV_VAR_SAVE_DEPTH_TXT_ENABLE))
                            {
                                save_depth_txt_file(merged_depth_buffer, frm_sequence, sns_param.out_frm_width*sns_param.out_frm_height*sizeof(u16));
                            }
                        }
                        adaps_dtof->ConvertDepthToColoredMap(merged_depth_buffer,rgb_buffer,sns_param.out_frm_width,sns_param.out_frm_height);
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
                        //DBG_ERROR("dtof_frame_decode() return %d , errno: %s (%d), save_frame_cnt: %d...", decodeRet, strerror(errno), errno, sns_param.save_frame_cnt);
                    }
#else
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
                        //DBG_ERROR("dtof_frame_decode() return %d , errno: %s (%d), save_frame_cnt: %d...", decodeRet, strerror(errno), errno, sns_param.save_frame_cnt);
                    }
#endif
                }
            }
            break;
#endif

        case FDATA_TYPE_YUYV:
            if (v4l2)
            {
                utils->yuyv_2_rgb((unsigned char *)frm_rawdata,rgb_buffer,sns_param.out_frm_width,sns_param.out_frm_height);
            }
            break;

        case FDATA_TYPE_NV12:
            if (v4l2)
            {
                utils->nv12_2_rgb((unsigned char *)frm_rawdata,rgb_buffer,sns_param.out_frm_width,sns_param.out_frm_height);
            }
        break;

        default:
            DBG_ERROR("Unsupported input frame type (%d)...", ftype);
            decodeRet = -EINVAL;
            break;
    }
    if (0 == decodeRet)
    {
#if defined(ENABLE_DYNAMICALLY_UPDATE_ROI_SRAM_CONTENT)
        if (NULL != merged_depth_buffer)
        {
            memset(merged_depth_buffer, 0, sizeof(u16)*sns_param.out_frm_width*sns_param.out_frm_height);
        }
#endif
        if (NULL != depth_buffer)
        {
            memset(depth_buffer, 0, sizeof(u16)*sns_param.out_frm_width*sns_param.out_frm_height);
        }
        QImage img = QImage(rgb_buffer, sns_param.out_frm_width, sns_param.out_frm_height, sns_param.out_frm_width*RGB_IMAGE_CHANEL,QImage::Format_RGB888);
        emit newFrameReady4Display(img);
    }
    if (sns_param.save_frame_cnt > 0)
    {
        sns_param.save_frame_cnt--;
    }

    return true;
}

void FrameProcessThread::onThreadLoopExit()
{
    if (v4l2)
    {
        v4l2->Stop_streaming();
        v4l2->V4l2_close();
    }
    if (SENSOR_TYPE_DTOF == sns_param.sensor_type)
    {
/*
        if (adaps_dtof)
        {
            adaps_dtof->adaps_dtof_release();
        }
*/
        if (NULL != depth_buffer)
        {
            free(depth_buffer);
            depth_buffer = NULL;
        }
#if defined(ENABLE_DYNAMICALLY_UPDATE_ROI_SRAM_CONTENT)
        if (NULL != merged_depth_buffer)
        {
            free(merged_depth_buffer);
            merged_depth_buffer = NULL;
        }
#endif
    }
    if (NULL != rgb_buffer)
    {
        free(rgb_buffer);
        rgb_buffer = NULL;
    }

    emit threadEnd(stop_req_code);
}

void FrameProcessThread::stop(int stop_request_code)
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

bool FrameProcessThread::isSleeping()
{
    return sleeping;
}

int FrameProcessThread::init(int index)
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
#if defined(RUN_ON_ROCKCHIP)
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
                if (NULL != depth_buffer)
                {
                    memset(depth_buffer, 0, sizeof(u16)*sns_param.out_frm_width*sns_param.out_frm_height);
                }

#if defined(ENABLE_DYNAMICALLY_UPDATE_ROI_SRAM_CONTENT)
                merged_depth_buffer = (u16 *)malloc(sizeof(u16)*sns_param.out_frm_width*sns_param.out_frm_height);
                if (NULL != merged_depth_buffer)
                {
                    memset(merged_depth_buffer, 0, sizeof(u16)*sns_param.out_frm_width*sns_param.out_frm_height);
                }
#endif
            }
#endif
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

void FrameProcessThread::run()
{
    int ret = 0;
    if(majorindex != -1)
    {
        while(!stopped)
        {
            if (this->isInterruptionRequested()) {
                 DBG_NOTICE("Thread is interrupted, cleaning up...");
                 break;
             }

            if(!stopped)
            {
                ret=v4l2->Capture_frame();
                sleeping = true;
                QThread::msleep(FRAME_INTERVAL);
                sleeping = false;
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
                    //break;
                }
            }
        }

        emit threadLoopExit();
    }
}

