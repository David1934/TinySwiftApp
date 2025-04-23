#include <cstdio>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <semaphore.h>
#include <string.h>

#include "common.h"
#include "utils.h"
#include "globalapplication.h"
#include "host_comm.h"

#ifndef SENDER_LOG_FUNC
#define SENDER_LOG_FUNC         printf
#endif

#define LOG_ERROR               DBG_ERROR
#define LOG_INFO                DBG_NOTICE
#define LOG_DEBUG               DBG_NOTICE
#define LOG_WARN                DBG_NOTICE


// 静态成员变量定义（必须！）
Host_Communication* Host_Communication::instance = NULL_POINTER;

Host_Communication* Host_Communication::getInstance() {
    if (!instance) {
        instance = new Host_Communication();
    }
    return instance;
}

// 私有构造函数
Host_Communication::Host_Communication() {
    txRawdataFrameCnt = 0;
    connected = false;
    backuped_script_buffer = NULL_POINTER;
    backuped_script_buffer_size = 0;
    backuped_blkwrite_reg_data = NULL_POINTER;
    backuped_blkwrite_reg_count = 0;
    txRawdataFrameCnt = 0;
    txDepth16FrameCnt = 0;
    firstRawdataFrameTimeUsec = 0;
    firstDepth16FrameTimeUsec = 0;
    adaps_sender_init();
}

Host_Communication::~Host_Communication()
{
    LOG_DEBUG("%s() total txRawdataFrameCnt: %ld.\n", __FUNCTION__, txRawdataFrameCnt);
    sender_destroy();
    if (NULL_POINTER != backuped_script_buffer)
    {
        free(backuped_script_buffer);
        backuped_script_buffer = NULL_POINTER;
        backuped_script_buffer_size = 0;
    }

    if (NULL_POINTER != backuped_blkwrite_reg_data)
    {
        free(backuped_blkwrite_reg_data);
        backuped_blkwrite_reg_data = NULL_POINTER;
        backuped_blkwrite_reg_count = 0;
    }

    LOG_DEBUG("%s() total txRawdataFrameCnt: %ld..\n", __FUNCTION__, txRawdataFrameCnt);

    delete instance;
    instance = NULL_POINTER;
}

UINT8 Host_Communication::get_req_out_data_type()
{
    return backuped_capture_req_param.req_out_data_type;
}

BOOLEAN Host_Communication::get_req_compose_subframe()
{
    return backuped_capture_req_param.compose_subframe;
}

BOOLEAN Host_Communication::get_req_expand_pixel()
{
    return backuped_capture_req_param.expand_pixel;
}

BOOLEAN Host_Communication::get_req_mirror_x()
{
    return backuped_capture_req_param.mirror_x;
}

BOOLEAN Host_Communication::get_req_mirror_y()
{
    return backuped_capture_req_param.mirror_y;
}

UINT8 Host_Communication::get_req_walkerror_version()
{
    return backuped_capture_req_param.walkerror_version;
}

int Host_Communication::dump_buffer_data(void* dump_buf, const char *buffer_name, int callline)
{
    int dump_size = 0;
    char temp_string[128];
    Utils *utils;

    dump_size = Utils::get_env_var_intvalue(ENV_VAR_DUMP_COMM_BUFFER_SIZE);
    if (dump_size)
    {
        utils = new Utils();
        sprintf(temp_string, "-----dump %s buffer, size: %d from Line: %d-----", buffer_name, dump_size, callline);
        utils->hexdump((unsigned char *) dump_buf, dump_size, temp_string);
        delete utils;
    }

    return 0;
}

int Host_Communication::report_error_msg(UINT16 responsed_cmd, UINT16 err_code, char *err_msg, int err_msg_length)
{
    void *async_buf;
    CommandData_t* pCmdData = NULL_POINTER;
    error_report_param_t *pErrReportParam;
    uint32_t ulBufSize = sizeof(CommandData_t) + sizeof(error_report_param_t);

    if (false == qApp->is_capture_req_from_host())
    {
        return 0;
    }

    if (false == connected || false == sender_is_working()) {
        LOG_ERROR("It seems host is not connected yet or disconnected, connected: %d.\n", connected);
        return -1;
    }

    if (err_msg_length > ERROR_MSG_MAX_LENGTH)
    {
        LOG_ERROR("Error message is too long.\n");
        return -1;
    }

    async_buf = sender_async_buf_alloc(ulBufSize);
    if (NULL_POINTER == async_buf) {
        LOG_ERROR("async alloc buf fail: %u.\n", ulBufSize);
        return -1;
    }

    pCmdData = (CommandData_t*) async_buf;

    pCmdData->cmd = CMD_DEVICE_SIDE_REPORT_ERROR;
    pErrReportParam  = (error_report_param_t *) pCmdData->param;
    pErrReportParam->responsed_cmd = responsed_cmd;
    pErrReportParam->err_code = err_code;
    memcpy(pErrReportParam->err_msg, err_msg, err_msg_length);

    int result = sender_async_send_msg(async_buf, ulBufSize, 0);

    if (result < 0) {
        //sender_async_buf_free(pstrCmdData);
        LOG_ERROR("async send raw data fail: %u.\n", ulBufSize);
        return result;
    }

    LOG_DEBUG("%s(%s) sucessfully!\n", __FUNCTION__, err_msg);

    return 0;
}

int Host_Communication::dump_frame_param(frame_buffer_param_t *pDataBufferParam)
{
    LOG_DEBUG("UINT8            data_type = 0x%x;               // refer to enum swift_data_type of this .h file", pDataBufferParam->data_type);
    LOG_DEBUG("UINT8            work_mode = %d;                 // refer to swift_workmode_t of adaps_types.h", pDataBufferParam->work_mode);
    LOG_DEBUG("UINT16           frm_width = %d;", pDataBufferParam->frm_width);
    LOG_DEBUG("UINT16           frm_height = %d;", pDataBufferParam->frm_height);
    LOG_DEBUG("UINT16           padding_bytes_per_line = %d;    // for PCM/FHR/PHR raw data, rockchip 4352 - 4104 for FHR", pDataBufferParam->padding_bytes_per_line);
    LOG_DEBUG("UINT8            env_type = %d;                  // refer to AdapsEnvironmentType of adaps_types.h", pDataBufferParam->env_type);
    LOG_DEBUG("UINT8            measure_type = %d;              // refer to AdapsMeasurementType of adaps_types.h", pDataBufferParam->measure_type);
    LOG_DEBUG("UINT8            framerate_type = %d;            // refer to AdapsFramerateType of adaps_types.h", pDataBufferParam->framerate_type);
    LOG_DEBUG("UINT8            power_mode = %d;                // refer to AdapsPowerMode of adaps_types.h", pDataBufferParam->power_mode);

    LOG_DEBUG("UINT32           curr_pvdd = %d;                 // the integer part of the PVDD voltage multiplied by 100", pDataBufferParam->curr_pvdd);
    LOG_DEBUG("UINT32           curr_vop_abs = %d;              // the integer part of the absolute value of the VOP voltage multiplied by 100", pDataBufferParam->curr_vop_abs);
    LOG_DEBUG("UINT32           curr_inside_temperature = %d;   // the integer part of the current temperature (in degrees Celsius) multiplied by 100", pDataBufferParam->curr_inside_temperature);
    LOG_DEBUG("UINT8            ptm_coarse_exposure_value = %d; //ptm_coarse_exposure_value, register configure value", pDataBufferParam->ptm_coarse_exposure_value);
    LOG_DEBUG("UINT8            ptm_fine_exposure_value = %d;   //ptm_fine_exposure_value, register configure value", pDataBufferParam->ptm_fine_exposure_value);
    LOG_DEBUG("UINT8            pcm_gray_exposure_value = %d;   //pcm_gray_exposure_value, register configure value", pDataBufferParam->pcm_gray_exposure_value);
    LOG_DEBUG("UINT8            exposure_period = %d;           //laser_exposure_period, register configure value", pDataBufferParam->exposure_period);
    LOG_DEBUG("UINT64           frame_sequence = %ld;", pDataBufferParam->frame_sequence);
    LOG_DEBUG("UINT64           frame_timestamp_us = %ld;", pDataBufferParam->frame_timestamp_us);
    LOG_DEBUG("UINT16           mipi_rx_fps = %d;", pDataBufferParam->mipi_rx_fps);
    LOG_DEBUG("UINT8            roi_data_index = %d;", pDataBufferParam->roi_data_index);
    LOG_DEBUG("UINT32           buffer_size = %d;               // the size for the following buffer, unit is byte", pDataBufferParam->buffer_size);
    LOG_DEBUG("sizeof(frame_buffer_param_t) = %ld", sizeof(frame_buffer_param_t));
    LOG_DEBUG("------------------------------------------------------");

    return 0;
}

int Host_Communication::report_frame_depth16_data(void* pFrameData, uint32_t frameData_size, frame_buffer_param_t *pFrmBufParam)
{
    int dump_times = 0;
    int result;
    void *async_buf;
    CommandData_t* pCmdData = NULL_POINTER;
    frame_buffer_param_t *pDataBufferParam;
    uint32_t ulCmdDataLen = sizeof(CommandData_t) + sizeof(frame_buffer_param_t);
    uint32_t ulBufSize = ulCmdDataLen + frameData_size;
    struct timeval tv;
    long currTimeUsec;
    unsigned long streamed_timeUs;
    int txDepth16Frame_fps;

    if (false == qApp->is_capture_req_from_host())
    {
        return 0;
    }

    if (0 == (get_req_out_data_type() & FRAME_DECODED_DEPTH16))
    {
        return 0;
    }

    if (false == connected || false == sender_is_working()) {
        LOG_ERROR("It seems host is not connected yet or disconnected, connected: %d.\n", connected);
        return -1;
    }

    async_buf = sender_async_buf_alloc(ulBufSize);
    if (NULL_POINTER == async_buf) {
        LOG_ERROR("async alloc buf fail: %u.\n", ulBufSize);
        return -1;
    }

    pCmdData = (CommandData_t*) async_buf;

    pCmdData->cmd = CMD_DEVICE_SIDE_REPORT_FRAME_DEPTH16_DATA;
    pDataBufferParam  = (frame_buffer_param_t *) pCmdData->param;
    memcpy(pDataBufferParam, pFrmBufParam, sizeof(frame_buffer_param_t));
    pDataBufferParam->buffer_size = frameData_size;
    memcpy(pDataBufferParam->buffer, pFrameData, frameData_size);

    dump_times = Utils::get_env_var_intvalue(ENV_VAR_DUMP_FRAME_PARAM_TIMES);
    if (0 != dump_times && txDepth16FrameCnt < static_cast<unsigned long>(dump_times))
    {
        dump_frame_param(pDataBufferParam);
    }

    result = sender_async_send_msg(async_buf, ulBufSize, 0);
    gettimeofday(&tv,NULL_POINTER);

    if (result < 0) {
        //sender_async_buf_free(pstrCmdData);
        LOG_ERROR("async send raw data fail: %u.\n", ulBufSize);
        return result;
    }

    txDepth16FrameCnt++;

    if (0 == firstDepth16FrameTimeUsec)
    {
        firstDepth16FrameTimeUsec = tv.tv_sec*1000000 + tv.tv_usec;
    }
    else {
        currTimeUsec = tv.tv_sec*1000000 + tv.tv_usec;
        streamed_timeUs = (currTimeUsec - firstDepth16FrameTimeUsec);
        txDepth16Frame_fps = (txDepth16FrameCnt * 1000000) / streamed_timeUs;
    }

    if (1 == txDepth16FrameCnt % FRAME_INTERVAL_4_PROGRESS_REPORT)
    {
        LOG_DEBUG("%s() sucessfully, mipi_rx_fps = %d fps, txDepth16Frame_fps = %d fps, txDepth16FrameCnt: %ld, frame_sequence: %ld, sizeof(CommandData_t): %ld, ulCmdDataLen: %d, frameData_size: %d, ulBufSize: %d.\n", 
            __FUNCTION__, pDataBufferParam->mipi_rx_fps, txDepth16Frame_fps, txDepth16FrameCnt, pDataBufferParam->frame_sequence, sizeof(CommandData_t), ulCmdDataLen, frameData_size, ulBufSize);
    }

    return 0;
}

int Host_Communication::report_frame_raw_data(void* pFrameData, uint32_t frameData_size, frame_buffer_param_t *pFrmBufParam)
{
    int dump_times = 0;
    int result;
    void *async_buf;
    CommandData_t* pCmdData = NULL_POINTER;
    frame_buffer_param_t *pDataBufferParam;
    uint32_t ulCmdDataLen = sizeof(CommandData_t) + sizeof(frame_buffer_param_t);
    uint32_t ulBufSize = ulCmdDataLen + frameData_size;
    struct timeval tv;
    long currTimeUsec;
    unsigned long streamed_timeUs;
    int txRawdataFrame_fps;

    if (false == qApp->is_capture_req_from_host())
    {
        return 0;
    }

    if (0 == (get_req_out_data_type() & FRAME_RAW_DATA))
    {
        return 0;
    }

    if (false == connected || false == sender_is_working()) {
        LOG_ERROR("It seems host is not connected yet or disconnected, connected: %d.\n", connected);
        return -1;
    }

    async_buf = sender_async_buf_alloc(ulBufSize);
    if (NULL_POINTER == async_buf) {
        LOG_ERROR("async alloc buf fail: %u.\n", ulBufSize);
        return -1;
    }

    pCmdData = (CommandData_t*) async_buf;

    pCmdData->cmd = CMD_DEVICE_SIDE_REPORT_FRAME_RAW_DATA;
    pDataBufferParam  = (frame_buffer_param_t *) pCmdData->param;
    memcpy(pDataBufferParam, pFrmBufParam, sizeof(frame_buffer_param_t));
    pDataBufferParam->buffer_size = frameData_size;
    memcpy(pDataBufferParam->buffer, pFrameData, frameData_size);

    dump_times = Utils::get_env_var_intvalue(ENV_VAR_DUMP_FRAME_PARAM_TIMES);
    if (0 != dump_times && txRawdataFrameCnt < static_cast<unsigned long>(dump_times))
    {
        dump_frame_param(pDataBufferParam);
    }

    result = sender_async_send_msg(async_buf, ulBufSize, 0);
    gettimeofday(&tv,NULL_POINTER);

    if (result < 0) {
        //sender_async_buf_free(pstrCmdData);
        LOG_ERROR("async send raw data fail: %u.\n", ulBufSize);
        return result;
    }

    txRawdataFrameCnt++;

    if (0 == firstRawdataFrameTimeUsec)
    {
        firstRawdataFrameTimeUsec = tv.tv_sec*1000000 + tv.tv_usec;
    }
    else {
        currTimeUsec = tv.tv_sec*1000000 + tv.tv_usec;
        streamed_timeUs = (currTimeUsec - firstRawdataFrameTimeUsec);
        txRawdataFrame_fps = (txRawdataFrameCnt * 1000000) / streamed_timeUs;
    }

    if (1 == txRawdataFrameCnt % FRAME_INTERVAL_4_PROGRESS_REPORT)
    {
        LOG_DEBUG("%s() sucessfully, mipi_rx_fps = %d fps, txRawdataFrame_fps = %d fps, txRawdataFrameCnt: %ld, frame_sequence: %ld, sizeof(CommandData_t): %ld, ulCmdDataLen: %d, frameData_size: %d, ulBufSize: %d.\n", 
            __FUNCTION__, pDataBufferParam->mipi_rx_fps, txRawdataFrame_fps, txRawdataFrameCnt, pDataBufferParam->frame_sequence, sizeof(CommandData_t), ulCmdDataLen, frameData_size, ulBufSize);
    }

    return 0;
}

int Host_Communication::dump_module_static_data(module_static_data_t *pStaticDataParam)
{
    CHAR                    serialNumber[SENSOR_SN_LENGTH+1] = {0};

    memcpy(serialNumber, pStaticDataParam->serialNumber, SENSOR_SN_LENGTH);

    LOG_DEBUG("UINT8            data_type = 0x%x;               // refer to enum swift_data_type of this .h file", pStaticDataParam->data_type);
    LOG_DEBUG("UINT32           module_type = 0x%x;             // refer to ADS6401_MODULE_SPOT and ADS6401_MODULE_FLOOD of adaps_types.h file", pStaticDataParam->module_type);
    LOG_DEBUG("UINT32           eeprom_capacity = %d;           // unit is byte", pStaticDataParam->eeprom_capacity);
    LOG_DEBUG("UINT16           otp_vbe25 = 0x%04x;", pStaticDataParam->otp_vbe25);
    LOG_DEBUG("UINT16           otp_vbd = 0x%04x;", pStaticDataParam->otp_vbd);
    LOG_DEBUG("UINT16           otp_adc_vref = 0x%04x;", pStaticDataParam->otp_adc_vref);
    LOG_DEBUG("CHAR             serialNumber = [%s];", serialNumber);
    LOG_DEBUG("UINT32           calib_data_size = %d;           // unit is byte", pStaticDataParam->calib_data_size);
    LOG_DEBUG("sizeof(module_static_data_t) = %ld", sizeof(module_static_data_t));

    return 0;
}

int Host_Communication::report_module_static_data()
{
    uint32_t ulBufSize;
    void *async_buf;
    void* pEEPROM_buffer;
    uint32_t EEPROM_size;
    void *module_static_data_addr;
    struct adaps_dtof_module_static_data *module_static_data;
    module_static_data_t *pStaticDataParam;
    CommandData_t* pCmdData;
    uint32_t ulCmdDataLen = sizeof(CommandData_t) + sizeof(module_static_data_t);

    if (false == connected || false == sender_is_working()) {
        LOG_ERROR("It seems host is not connected yet or disconnected, connected: %d.\n", connected);
        return -1;
    }

    p_misc_device = qApp->get_misc_dev_instance();

    if (NULL_POINTER == p_misc_device)
    {
        DBG_ERROR("p_misc_device is NULL, p_misc_device:%p, qApp: %p, qApp->get_misc_dev_instance(): %p", p_misc_device, qApp, qApp->get_misc_dev_instance());
        return -1;
    }

    p_misc_device->get_dtof_module_static_data(&module_static_data_addr, &pEEPROM_buffer, &EEPROM_size);
    module_static_data = (struct adaps_dtof_module_static_data *) module_static_data_addr;
    ulBufSize = ulCmdDataLen + EEPROM_size;

    async_buf = sender_async_buf_alloc(ulBufSize);
    if (NULL_POINTER == async_buf) {
        LOG_ERROR("async alloc buf fail: %u.\n", ulBufSize);
        return -1;
    }

    pCmdData = (CommandData_t*) async_buf;

    pCmdData->cmd = CMD_DEVICE_SIDE_REPORT_MODULE_STATIC_DATA;
    pStaticDataParam  = (module_static_data_t *) pCmdData->param;
    pStaticDataParam->module_type = module_static_data->module_type;
    pStaticDataParam->eeprom_capacity = module_static_data->eeprom_capacity;
    pStaticDataParam->otp_vbe25 = module_static_data->otp_vbe25;
    pStaticDataParam->otp_vbd = module_static_data->otp_vbd;
    pStaticDataParam->otp_adc_vref = module_static_data->otp_adc_vref;
    memcpy(pStaticDataParam->serialNumber, module_static_data->serialNumber, SENSOR_SN_LENGTH);
    pStaticDataParam->data_type = MODULE_STATIC_DATA;
    pStaticDataParam->calib_data_size = EEPROM_size;
    memcpy(pStaticDataParam->calib_data, pEEPROM_buffer, EEPROM_size);
    //dump_buffer_data(pStaticDataParam->calib_data, "module_static_data", __LINE__);
    if (true == Utils::is_env_var_true(ENV_VAR_DUMP_MODULE_STATIC_DATA))
    {
        dump_module_static_data(pStaticDataParam);
    }

    int result = sender_async_send_msg(async_buf, ulBufSize, 0);

    if (result < 0) {
        //sender_async_buf_free(pstrCmdData);
        LOG_ERROR("async send module calibrate data fail: %u.\n", ulBufSize);
        return result;
    }
    LOG_DEBUG("%s() sucessfully.\n", __FUNCTION__);

    return 0;
}

int Host_Communication::dump_capture_req_param(capture_req_param_t* pCaptureReqParam)
{
    LOG_DEBUG("UINT8            work_mode = %d;             // refer to swift_workmode_t of adaps_types.h", pCaptureReqParam->work_mode);
    LOG_DEBUG("UINT8            env_type = %d;              // refer to AdapsEnvironmentType of adaps_types.h", pCaptureReqParam->env_type);
    LOG_DEBUG("UINT8            measure_type = %d;          // refer to AdapsMeasurementType of adaps_types.h", pCaptureReqParam->measure_type);
    LOG_DEBUG("UINT8            framerate_type = %d;        // refer to AdapsFramerateType of adaps_types.h", pCaptureReqParam->framerate_type);
    LOG_DEBUG("UINT8            power_mode = %d;            // refer to AdapsPowerMode of adaps_types.h", pCaptureReqParam->power_mode);
    LOG_DEBUG("UINT8            walkerror_version = %d;     // refer to ???", pCaptureReqParam->walkerror_version);
    LOG_DEBUG("UINT8            req_out_data_type = %d;     // refer to enum swift_data_type of this .h file", pCaptureReqParam->req_out_data_type);
    LOG_DEBUG("BOOLEAN          compose_subframe = %d;", pCaptureReqParam->compose_subframe);
    LOG_DEBUG("BOOLEAN          expand_pixel = %d;", pCaptureReqParam->expand_pixel);
    LOG_DEBUG("BOOLEAN          mirror_x = %d;", pCaptureReqParam->mirror_x);
    LOG_DEBUG("BOOLEAN          mirror_y = %d;", pCaptureReqParam->mirror_y);
    LOG_DEBUG("BOOLEAN          laserEnable = %d;", pCaptureReqParam->laserEnable);
    LOG_DEBUG("BOOLEAN          vopAdjustEnable = %d;", pCaptureReqParam->vopAdjustEnable);
    LOG_DEBUG("UINT8            rowSearchingRange = %d;", pCaptureReqParam->rowSearchingRange);
    LOG_DEBUG("UINT8            colSearchingRange = %d;", pCaptureReqParam->colSearchingRange);
    LOG_DEBUG("UINT8            rowOffset = %d;", pCaptureReqParam->rowOffset);
    LOG_DEBUG("UINT8            colOffset = %d;", pCaptureReqParam->colOffset);
    LOG_DEBUG("expose_param_t   expose_param = (0x%02x, 0x%02x, 0x%02x);", 
            pCaptureReqParam->expose_param.coarseExposure, pCaptureReqParam->expose_param.fineExposure, pCaptureReqParam->expose_param.grayExposure);
    LOG_DEBUG("BOOLEAN          script_loaded = %d;", pCaptureReqParam->script_loaded);
    LOG_DEBUG("UINT32           script_size = %d;           // set to 0 if script_loaded == false", pCaptureReqParam->script_size);
    LOG_DEBUG("UINT8            blkwrite_reg_count = %d;    // set to 0 if no block_write register is included in script file", pCaptureReqParam->blkwrite_reg_count);
    LOG_DEBUG("sizeof(capture_req_param_t) = %ld", sizeof(capture_req_param_t));

    return 0;
}

void Host_Communication::swift_set_colormap_range(CommandData_t* pCmdData, uint32_t rxDataLen)
{
    colormap_range_param_t* pColormapRangeParam;

    if (rxDataLen < (sizeof(CommandData_t) + sizeof(colormap_range_param_t)))
    {
        LOG_ERROR("swift_set_colormap_range: rxDataLen %d is too short for CMD_HOST_SIDE_SET_COLORMAP_RANGE_PARAM.\n", rxDataLen);
        return;
    }

    pColormapRangeParam = (colormap_range_param_t*) pCmdData->param;

    qApp->set_GrayScaleMinMappedRange(pColormapRangeParam->GrayScaleMinMappedRange);
    qApp->set_GrayScaleMaxMappedRange(pColormapRangeParam->GrayScaleMaxMappedRange);

    qApp->set_RealDistanceMinMappedRange(pColormapRangeParam->RealDistanceMinMappedRange);
    qApp->set_RealDistanceMaxMappedRange(pColormapRangeParam->RealDistanceMaxMappedRange);
}

void Host_Communication::swift_start_capture(CommandData_t* pCmdData, uint32_t rxDataLen)
{
    capture_req_param_t* pCaptureReqParam;

    if (rxDataLen < (sizeof(CommandData_t) + sizeof(capture_req_param_t)))
    {
        LOG_ERROR("swift_start_capture: rxDataLen %d is too short for CMD_HOST_SIDE_START_CAPTURE.\n", rxDataLen);
        return;
    }

    pCaptureReqParam = (capture_req_param_t*) pCmdData->param;
    LOG_DEBUG("------req_out_data_type: 0x%x-----\n", pCaptureReqParam->req_out_data_type);
    //pCaptureReqParam->req_out_data_type = FRAME_RAW_DATA | FRAME_DECODED_DEPTH16;
    if (true == Utils::is_env_var_true(ENV_VAR_DUMP_CAPTURE_REQ_PARAM))
    {
        dump_capture_req_param(pCaptureReqParam);
    }

    qApp->set_capture_req_from_host(true);
    memcpy(&backuped_capture_req_param, pCaptureReqParam, sizeof(capture_req_param_t));
    emit set_capture_options(pCaptureReqParam);
    //LOG_ERROR("param.env_type %d.\n", pCaptureReqParam->env_type);
    //pCaptureReqParam->env_type = AdapsEnvTypeIndoor;

    if (pCaptureReqParam->script_loaded && pCaptureReqParam->script_size)
    {
        backuped_wkmode = pCaptureReqParam->work_mode;
        backuped_script_buffer_size = pCaptureReqParam->script_size;

        if (NULL_POINTER == backuped_script_buffer)
        {
            backuped_script_buffer = (u8 *) malloc(backuped_script_buffer_size);
            LOG_DEBUG("------backuped_script_buffer_size: %d, backuped_script_buffer: %p-----\n", backuped_script_buffer_size, backuped_script_buffer);
            if (NULL_POINTER == backuped_script_buffer) {
                DBG_ERROR("Fail to malloc for backup_script_buffer.\n");
                return ;
            }
        }

        memcpy(backuped_script_buffer, pCaptureReqParam->script_buffer, backuped_script_buffer_size);

        backuped_blkwrite_reg_count = pCaptureReqParam->blkwrite_reg_count;
        
        if (NULL_POINTER == backuped_blkwrite_reg_data)
        {
            backuped_blkwrite_reg_data = (u8 *) malloc(backuped_blkwrite_reg_count * sizeof(blkwrite_reg_data_t));
            if (NULL_POINTER == backuped_blkwrite_reg_data) {
                DBG_ERROR("Fail to malloc for backuped_blkwrite_reg_data.\n");
                return ;
            }
        }
        
        memcpy(backuped_blkwrite_reg_data, &pCaptureReqParam->script_buffer[backuped_script_buffer_size], backuped_blkwrite_reg_count * sizeof(blkwrite_reg_data_t));
    }
    else {
        backuped_script_buffer_size = 0;
    }

    emit start_capture();
}

void Host_Communication::get_backuped_script_buffer_info(UINT8 *workmode, UINT8 ** script_buffer, uint32_t *script_buffer_size, UINT8 ** blkwrite_reg_data, uint32_t *blkwrite_reg_count)
{
    *workmode = backuped_wkmode;
    *script_buffer_size = backuped_script_buffer_size;
    *script_buffer = backuped_script_buffer;
    *blkwrite_reg_count = backuped_blkwrite_reg_count;
    *blkwrite_reg_data = backuped_blkwrite_reg_data;
}

void Host_Communication::read_device_register(UINT16 cmd, CommandData_t* pCmdData, uint32_t rxDataLen)
{
    int ret = 0;
    register_op_data_t *register_op_data;

    LOG_INFO("--- receive report sensor register event -----\n");

    if (rxDataLen != (sizeof(CommandData_t) + sizeof(register_op_data_t)))
    {
        LOG_ERROR("dataLen(%u) is illegal.\n", rxDataLen);
        return;
    }

    register_op_data = (register_op_data_t*) pCmdData->param;
    p_misc_device = qApp->get_misc_dev_instance();
    if (NULL_POINTER == p_misc_device)
    {
        DBG_ERROR("p_misc_device is NULL");
        return;
    }

    ret = p_misc_device->read_device_register(register_op_data);
    if (0 > ret)
    {
        char err_msg[] = "Fail to read register, please check the device side's log for more details.";
        report_error_msg(cmd, CMD_DEVICE_SIDE_ERROR_READ_REGISTER, err_msg, strlen(err_msg));
        return;
    }

    if (CMD_HOST_SIDE_GET_VCSLDRV_OP7020_REGISTER == cmd)
    {
        pCmdData->cmd       = CMD_DEVICE_SIDE_REPORT_VCSLDRV_OP7020_REGISTER;
    }
    else {
        pCmdData->cmd       = CMD_DEVICE_SIDE_REPORT_SENSOR_REGISTER;
    }

    LOG_WARN("get sensor register[0x%x] = 0x%x.\n"
        , register_op_data->reg_addr, register_op_data->reg_val);

    ret = sender_send_msg((void *)pCmdData, (sizeof(CommandData_t) + sizeof(register_op_data_t)));
    if (ret) {
        LOG_ERROR("call sender_send_msg: %u fail: %u.\n", pCmdData->cmd, ret);
    }

    return;
}

void Host_Communication::write_device_register(UINT16 cmd, CommandData_t* pCmdData, uint32_t rxDataLen)
{
    int ret = 0;
    register_op_data_t *register_op_data;

    LOG_INFO("--- receive write sensor register event -----\n");

    if (rxDataLen != (sizeof(CommandData_t) + sizeof(register_op_data_t)))
    {
        LOG_ERROR("dataLen(%u) is illegal.\n", rxDataLen);
        return;
    }

    register_op_data = (register_op_data_t*) pCmdData->param;
    p_misc_device = qApp->get_misc_dev_instance();
    if (NULL_POINTER == p_misc_device)
    {
        DBG_ERROR("p_misc_device is NULL_POINTER");
        return;
    }

    ret = p_misc_device->write_device_register(register_op_data);
    if (0 > ret)
    {
        char err_msg[] = "Fail to write register, please check the device side's log for more details.";
        report_error_msg(cmd, CMD_DEVICE_SIDE_ERROR_WRITE_REGISTER, err_msg, strlen(err_msg));
    }

    return;
}

void Host_Communication::swift_event_process(void* pRXData, uint32_t rxDataLen)
{
    V4L2 *v4l2 = qApp->get_v4l2_instance();

    if (rxDataLen < sizeof(CommandData_t)) {
        LOG_ERROR("swift_event_process: rxDataLen %d is too short.\n", rxDataLen);
        return;
    }

    CommandData_t* pCmdData = (CommandData_t*)pRXData;
    LOG_DEBUG("swift_event_process: cmd = 0x%x, rxDataLen %d.\n", pCmdData->cmd, rxDataLen);
    switch (pCmdData->cmd)
    {
        case CMD_HOST_SIDE_GET_MODULE_STATIC_DATA:
            report_module_static_data();
            break;

        case CMD_HOST_SIDE_SET_COLORMAP_RANGE_PARAM:
            swift_set_colormap_range(pCmdData, rxDataLen);
            break;

        case CMD_HOST_SIDE_START_CAPTURE:
            swift_start_capture(pCmdData, rxDataLen);
            break;

        case CMD_HOST_SIDE_STOP_CAPTURE:
            if (true != Utils::is_env_var_true(ENV_VAR_DEVELOP_DEBUGGING))
            {
                //stop_capture_event();
                emit stop_capture();
            }
            break;

        case CMD_HOST_SIDE_GET_SENSOR_REGISTER:
        case CMD_HOST_SIDE_GET_VCSLDRV_OP7020_REGISTER:
            if (NULL_POINTER == v4l2)
            {
                DBG_ERROR("v4l2 is NULL");
                return;
            }

            if (true == v4l2->get_power_on_state())
            {
                read_device_register(pCmdData->cmd, pCmdData, rxDataLen);
            } else {
                LOG_WARN("cmd %u: sensor is power off.\n", pCmdData->cmd);
            }
            break;

        case CMD_HOST_SIDE_SET_SENSOR_REGISTER:
        case CMD_HOST_SIDE_SET_VCSLDRV_OP7020_REGISTER:
            if (NULL_POINTER == v4l2)
            {
                DBG_ERROR("v4l2 is NULL");
                return;
            }

            if (true == v4l2->get_power_on_state())
            {
                write_device_register(pCmdData->cmd, pCmdData, rxDataLen);
            } else {
                LOG_WARN("cmd %u: sensor is power off.\n", pCmdData->cmd);
            }
            break;

        default:
            LOG_ERROR("Unknown swift_event_process: cmd %d.\n", pCmdData->cmd);
            break;
    }

    return;
}

void Host_Communication::swift_sender_disconnected()
{
    LOG_WARN("sender is disconnected now.\n");

    if (true != Utils::is_env_var_true(ENV_VAR_DEVELOP_DEBUGGING))
    {
        emit stop_capture();
        qApp->set_capture_req_from_host(false);
        connected = false;
    }

    return;
}

void Host_Communication::swift_sender_connected()
{
    LOG_WARN("sender connect success.\n");

    connected = true;

    return;
}

int Host_Communication::swift_sender_callback(SenderEventId_t id, void* arg_ptr, uint32_t arg_u32, ...)
{
    Host_Communication* obj = instance;
    //LOG_DEBUG("swift_sender_callback_server: id = %d.\n", id);

    switch (id)
    {
    case SENDER_EVT_CONNECTED:
        obj->swift_sender_connected();
        break;

    case SENDER_EVT_DISCONNECTED:
        obj->swift_sender_disconnected();
        break;

    case SENDER_EVT_RECEIVED_MSG:
        obj->swift_event_process(arg_ptr, arg_u32);
        break;

    default:
        LOG_ERROR("Unknown sender evt:%d\n", id);
        break;
    }

    return 0;
}

int Host_Communication::adaps_sender_init()
{
    int ret = 0;
    sender_init_param_t init_param;

    sender_log_config(SENDER_LOG_WARN, (sender_log_output_func_t)SENDER_LOG_FUNC);

    memset(&init_param, 0, sizeof(init_param));

    init_param.host_or_device = SENDER_RUN_AS_DEVICE;
    init_param.tcp_port = TCP_SERVER_PORT;
    init_param.callback = swift_sender_callback;

    ret = sender_init(&init_param);
    LOG_DEBUG("sender init ret %d, sender lib version: %s\n", ret, sender_get_version_str());
    if (ret != 0) {
        LOG_ERROR("sender init failed, ret %d\n", ret);
        return -1;
    }

    return ret;
}

