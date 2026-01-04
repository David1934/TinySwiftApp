#include <poll.h>

#include "dtof_main.h"

DToF_Main::DToF_Main(struct sensor_params sensor_param)
{
    int ret = 0;
    int ret_4_start_stream = 0;
#if defined(EEPROM_PARAMETER_FETCH_DEMO)
    float* rgbLensIntrinsicData;          // 8xsizeof(float)
    float* common_extrinsicData;          // 7xsizeof(float)
#endif

    memset(&sns_param, 0, sizeof(struct sensor_params));
    sns_param.work_mode = sensor_param.work_mode;
    sns_param.env_type = sensor_param.env_type;
    sns_param.measure_type = sensor_param.measure_type;
    sns_param.framerate_type = sensor_param.framerate_type;
    sns_param.power_mode = sensor_param.power_mode;
    sns_param.to_dump_frame_cnt = sensor_param.to_dump_frame_cnt;
    sns_param.roi_sram_rolling = sensor_param.roi_sram_rolling;
    sns_param.module_kernel_type = sensor_param.module_kernel_type;

    frame_process_thread_id = PTHREAD_INVALID;
    dumped_frame_cnt = 0;
    dump_spot_statistics_times = Utils::get_env_var_intvalue(ENV_VAR_DUMP_SPOT_STATISTICS_TIMES);
    dump_ptm_frame_headinfo_times = Utils::get_env_var_intvalue(ENV_VAR_DUMP_PTM_FRAME_HEADINFO_TIMES);

    misc_dev = Misc_Device::getInstance();
    misc_dev->set_module_kernel_type(sns_param.module_kernel_type);
    misc_dev->set_roi_sram_rolling(sns_param.roi_sram_rolling);
    utils = new Utils();
    v4l2 = new V4L2(sns_param);
    v4l2->Get_frame_size_4_curr_wkmode(&sns_param.raw_width, &sns_param.raw_height, &sns_param.out_frm_width, &sns_param.out_frm_height);
    DBG_INFO( "raw_width: %d raw_height: %d, env_type: %d, measure_type: %d, framerate_type: %d\n",
        sns_param.raw_width, sns_param.raw_height, sns_param.env_type, sns_param.measure_type, sns_param.framerate_type);
    adaps_dtof = new ADAPS_DTOF(sns_param);

    ret = v4l2->V4l2_initilize();
    if (ret < 0)
    {
        DBG_ERROR("Fail to v4l2->V4l2_initilize(),ret:%d, errno: %s (%d)...", ret,
            strerror(errno), errno);
        return;
    }
    else {
        ret_4_start_stream = v4l2->Start_streaming(); // need this before adaps_dtof->adaps_dtof_initilize() to get the real exposure_period ptm_fine_exposure_value in initParams()
        if (ret_4_start_stream < 0)
        {
            DBG_ERROR("Fail to v4l2->V4l2_initilize(),ret:%d, errno: %s (%d)...", ret_4_start_stream,
                strerror(errno), errno);
            return ;
        }

        if (NULL_POINTER == adaps_dtof)
        {
            DBG_ERROR("adaps_dtof is NULL" );
            return ;
        }

        ret = adaps_dtof->adaps_dtof_initilize();
        if (0 > ret)
        {
            DBG_ERROR("Fail to adaps_dtof->adaps_dtof_initilize(),ret:%d, errno: %s (%d)...", 
                ret, strerror(errno), errno);
            return ;
        }
        else {
            depth_buffer_size = sizeof(u16)*sns_param.out_frm_width*sns_param.out_frm_height;

            depth_buffer = (u16 *)malloc(depth_buffer_size);
            if (NULL_POINTER != depth_buffer)
            {
                memset(depth_buffer, 0, depth_buffer_size);
            }

#if ALGO_LIB_VERSION_CODE >= VERSION_HEX_VALUE(3, 5, 6) && defined(ENABLE_POINTCLOUD_OUTPUT)
            out_pcloud_buffer_size = sizeof(pc_pkt_t)*sns_param.out_frm_width*sns_param.out_frm_height;

            out_pcloud_buffer = (pc_pkt_t *)malloc(out_pcloud_buffer_size);
            if (NULL_POINTER != out_pcloud_buffer)
            {
                memset(out_pcloud_buffer, 0, out_pcloud_buffer_size);
            }
#endif

        }
    }

#if defined(EEPROM_PARAMETER_FETCH_DEMO)
    // 获取EEPROM参数前, 需要先执行Misc_Device类的构造函数和read_dtof_module_static_data()函数
    p_eeprominfo = misc_dev->get_dtof_calib_eeprom_param();
    rgbLensIntrinsicData  = reinterpret_cast<float*>(p_eeprominfo + BIG_FOV_MODULE_EEPROM_RGB_INTRINSIC_OFFSET);
    common_extrinsicData  = reinterpret_cast<float*>(p_eeprominfo + BIG_FOV_MODULE_EEPROM_COMMON_EXTRINSIC_OFFSET);
#endif

    pthread_create(&frame_process_thread_id, NULL, FrameProcessThread, (void*)this);
}

DToF_Main::~DToF_Main()
{
    // 等待线程结束
    pthread_join(frame_process_thread_id, NULL);

    if (v4l2)
    {
        v4l2->Stop_streaming();
        v4l2->V4l2_close();
    }

    if (NULL_POINTER != adaps_dtof)
    {
        delete adaps_dtof;
        adaps_dtof = NULL_POINTER;
    }

    if (NULL_POINTER != v4l2)
    {
        delete v4l2;
        v4l2 = NULL_POINTER;
    }

    if (NULL_POINTER != utils)
    {
        delete utils;
        utils = NULL_POINTER;
    }

    if (NULL_POINTER != depth_buffer)
    {
        free(depth_buffer);
        depth_buffer = NULL_POINTER;
    }

#if ALGO_LIB_VERSION_CODE >= VERSION_HEX_VALUE(3, 5, 6) && defined(ENABLE_POINTCLOUD_OUTPUT)
    if (NULL_POINTER != out_pcloud_buffer)
    {
        free(out_pcloud_buffer);
        out_pcloud_buffer = NULL_POINTER;
    }
#endif
}

bool DToF_Main::frame_decode(   struct frame_decode_param *param)
{
    int decodeRet = 0;
    static int run_times = 0;

    unsigned int frm_sequence = param->frm_sequence;
    void *frm_rawdata = param->frm_buffer;
    int buf_len = param->frm_buf_len;
    enum frame_data_type frm_type = param->frm_type;

    switch (frm_type) {
        case FDATA_TYPE_DTOF_RAW_GRAYSCALE:
            if (adaps_dtof)
            {
                run_times++;

                if ((sns_param.to_dump_frame_cnt > 0) && (dumped_frame_cnt < sns_param.to_dump_frame_cnt))
                {
                    if (true == Utils::is_env_var_true(ENV_VAR_SAVE_FRAME_RAW_DATA_ENABLE))
                    {
                        utils->save_frame(frm_sequence,frm_rawdata,buf_len,
                            sns_param.raw_width, sns_param.raw_height,
                            FDATA_TYPE_DTOF_RAW_GRAYSCALE);
                    }
                }

                decodeRet = adaps_dtof->dtof_frame_decode(frm_sequence, (unsigned char *)frm_rawdata, buf_len, depth_buffer, NULL_POINTER, sns_param.work_mode);
                if (0 == decodeRet)
                {
                    if ((sns_param.to_dump_frame_cnt > 0) && (dumped_frame_cnt < sns_param.to_dump_frame_cnt))
                    {
                        if (true == Utils::is_env_var_true(ENV_VAR_SAVE_FRAME_DEPTH16_ENABLE))
                        {
                            utils->save_frame(frm_sequence,depth_buffer,depth_buffer_size,
                                sns_param.out_frm_width, sns_param.out_frm_height,
                                FDATA_TYPE_DTOF_DECODED_GRAYSCALE);
                        }
                    }

                }
                else {
                    DBG_ERROR("dtof_frame_decode() return %d , to_dump_frame_cnt: %d...", decodeRet, sns_param.to_dump_frame_cnt);
                }
            }
            break;

        case FDATA_TYPE_DTOF_RAW_DEPTH:
            if (adaps_dtof)
            {
                if (frm_sequence < dump_ptm_frame_headinfo_times && 0 != dump_ptm_frame_headinfo_times)
                {
                    adaps_dtof->dump_frame_headinfo(frm_sequence, (unsigned char *)frm_rawdata, buf_len, sns_param.work_mode);
                }

                if ((sns_param.to_dump_frame_cnt > 0) && (dumped_frame_cnt < sns_param.to_dump_frame_cnt))
                {
                    if (true == Utils::is_env_var_true(ENV_VAR_SAVE_FRAME_RAW_DATA_ENABLE))
                    {
                        utils->save_frame(frm_sequence,frm_rawdata,buf_len,
                            sns_param.raw_width, sns_param.raw_height,
                            FDATA_TYPE_DTOF_RAW_DEPTH);
                    }
                }

                decodeRet = adaps_dtof->dtof_frame_decode(
                    frm_sequence,
                    (unsigned char *)frm_rawdata,
                    buf_len,
                    depth_buffer,
#if ALGO_LIB_VERSION_CODE >= VERSION_HEX_VALUE(3, 5, 6) && defined(ENABLE_POINTCLOUD_OUTPUT)
                    out_pcloud_buffer,
#else
                    NULL_POINTER,
#endif
                    sns_param.work_mode);

                if (0 == decodeRet)
                {
                    outputed_frame_cnt++;

                    if (frm_sequence < dump_spot_statistics_times && 0 != dump_spot_statistics_times)
                    {
                        adaps_dtof->dumpSpotCount(depth_buffer, sns_param.out_frm_width, sns_param.out_frm_height, frm_sequence, outputed_frame_cnt, decodeRet, __LINE__);
                    }
                    adaps_dtof->depthMapDump(depth_buffer, sns_param.out_frm_width, sns_param.out_frm_height, outputed_frame_cnt, __LINE__);

                    if ((sns_param.to_dump_frame_cnt > 0) && (dumped_frame_cnt < sns_param.to_dump_frame_cnt))
                    {
                        if (true == Utils::is_env_var_true(ENV_VAR_SAVE_FRAME_DEPTH16_ENABLE))
                        {
                            utils->save_frame(frm_sequence, depth_buffer, depth_buffer_size,
                                sns_param.out_frm_width, sns_param.out_frm_height,
                                FDATA_TYPE_DTOF_DECODED_DEPTH16);
                        }

#if ALGO_LIB_VERSION_CODE >= VERSION_HEX_VALUE(3, 5, 6) && defined(ENABLE_POINTCLOUD_OUTPUT)
                        if (true == Utils::is_env_var_true(ENV_VAR_SAVE_FRAME_POINTCLOUD_ENABLE))
                        {
                            utils->save_frame(frm_sequence, out_pcloud_buffer, out_pcloud_buffer_size,
                                sns_param.out_frm_width, sns_param.out_frm_height,
                                FDATA_TYPE_DTOF_DECODED_POINT_CLOUD);
                        }
#endif

                        if (Utils::is_env_var_true(ENV_VAR_SAVE_DEPTH_TXT_ENABLE))
                        {
                            utils->save_depth_txt_file(depth_buffer, frm_sequence, depth_buffer_size, sns_param.out_frm_width, sns_param.out_frm_height);
                        }
                    }

                }
                else {
                    //DBG_ERROR("dtof_frame_decode() return %d , frm_sequence: %d, adaps_dtof: %p...", decodeRet, frm_sequence, adaps_dtof);
                }

            }
            else {
                DBG_ERROR("adaps_dtof is NULL");
            }
            break;

        default:
            DBG_ERROR("Unsupported input frame type (%d)...", frm_type);
            decodeRet = -EINVAL;
            break;
    }
    if (0 == decodeRet)
    {
        if (NULL_POINTER != depth_buffer)
        {
            memset(depth_buffer, 0, depth_buffer_size);
        }

    }
    if (sns_param.to_dump_frame_cnt > 0)
    {
        dumped_frame_cnt++;
    }

    return true;
}

void* DToF_Main::FrameProcessThread(void* arg)
{
    int ret = 0;
    struct frame_decode_param param;
    // 将 void* 转换为当前类指针（this 指针）
    DToF_Main* this_instance = static_cast<DToF_Main*>(arg);
    int fd = this_instance->v4l2->Get_videodev_fd();

    // 使用poll监听设备fd，设置100ms超时，避免ioctl无限阻塞
    struct pollfd fds[1];
    fds[0].fd = fd;
    fds[0].events = POLLIN; // 等待数据可读

    while (PTHREAD_INVALID != this_instance->frame_process_thread_id)
    {
         ret = poll(fds, 1, POLL_TIMEOUT);
         if (ret <= 0)
         {
             if (ret < 0)
             {
                 DBG_ERROR("Fail to poll, errno: %s (%d)...", strerror(errno), errno);
                 break;
             }
         }
         else {
             // 确认有数据可读，再调用DQBUF
             if (fds[0].revents & POLLIN)
             {
                 ret = this_instance->v4l2->Capture_frame(&param);

                 if(ret < 0)
                 {
                     DBG_ERROR("Fail to Capture_frame, ret: %d...", ret);
                     break;
                 }
                 else {
                     if (true == Utils::is_env_var_true(ENV_VAR_BUFFER_FULLY_ZERO_CHECK))
                     {
                         int offset = 256; // For PTM mode, The first 256 bytes are always not full-zero, even some bugs happen.
                         if (FDATA_TYPE_DTOF_RAW_GRAYSCALE == param.frm_type)
                         {
                            offset = 0;
                         }
                         if (true == this_instance->utils->buffer_is_fully_same(static_cast<unsigned char*>(param.frm_buffer) + offset, param.frm_buf_len - offset, 0))
                         {
                             DBG_NOTICE("%s() frame[%d] buffer (offset:%d, length:%d) are fully zero!!!\n",  __FUNCTION__, param.frm_sequence, offset, param.frm_buf_len - offset);
                         }
                     }

                     this_instance->frame_decode(&param);
                     // if already capture expected frames, try to quit.
                     if ((this_instance->sns_param.to_dump_frame_cnt > 0) && (this_instance->dumped_frame_cnt >= this_instance->sns_param.to_dump_frame_cnt))
                     {
                         DBG_NOTICE("----frame dump done, exiting-----to_dump_frame_cnt: %d, dumped_frame_cnt: %d\n", this_instance->sns_param.to_dump_frame_cnt, this_instance->dumped_frame_cnt);
                         break;
                     }
                 }
             }
         }

        usleep(FRAME_PROCESS_THREAD_INTERVAL_US);

    }

    return NULL_POINTER;
}

void DToF_Main::stopFrameProcessThread()
{
    pthread_t tid;

    if (frame_process_thread_id != PTHREAD_INVALID){
        tid = frame_process_thread_id;
        frame_process_thread_id = PTHREAD_INVALID;
        pthread_cancel(tid);
    }
}

void DToF_Main::stop()
{
    stopFrameProcessThread();
}


