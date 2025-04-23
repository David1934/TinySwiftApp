#include "adaps_dtof.h"
#include "utils.h"
#if defined(ENABLE_DYNAMICALLY_UPDATE_ROI_SRAM_CONTENT)
#include "ads6401_roi_sram_test_data.h"
#endif
#include <globalapplication.h>

ADAPS_DTOF::ADAPS_DTOF(struct sensor_params params)
{
    memcpy(&m_sns_param, &params, sizeof(struct sensor_params));

    m_basic_colors[4] = {255, 0, 0};
    m_basic_colors[3] = {255, 255, 0};
    m_basic_colors[2] = {0, 255, 0};
    m_basic_colors[1] = {0, 255, 255};
    m_basic_colors[0] = {0, 0, 255};

    m_LimitedMaxDistance = COLOR_MAP_HIGH;
    m_rangeHigh = COLOR_MAP_HIGH;
    m_rangeLow = RANGE_MIN;
    m_conversionLibInited = false;
    m_decoded_frame_cnt = 0;
#if defined(ENABLE_DYNAMICALLY_UPDATE_ROI_SRAM_CONTENT)
    cur_calib_sram_data_group_idx = 0;
    trace_calib_sram_switch = Utils::is_env_var_true(ENV_VAR_TRACE_ROI_SRAM_SWITCH);
#endif
    p_misc_device = qApp->get_misc_dev_instance();
}


ADAPS_DTOF::~ADAPS_DTOF()
{
    p_misc_device = NULL_POINTER;
}

int ADAPS_DTOF::hexdump_param(void* param_ptr, int param_size, const char *param_name, int callline)
{
    char temp_string[128];
    Utils *utils;

    if (true == Utils::is_env_var_true(ENV_VAR_DUMP_ALGO_LIB_IO_PARAM))
    {
        utils = new Utils();
        sprintf(temp_string, "-----dump %s parameter, size: %d from Line: %d-----", param_name, param_size, callline);
        utils->hexdump((unsigned char *) param_ptr, param_size, temp_string);
        delete utils;
    }

    return 0;
}

u8 ADAPS_DTOF::normalizeGreyscale(u16 range) {
    u16 normalized = range - RANGE_MIN;
    // Clamp to min/max
    normalized = MAX(RANGE_MIN, normalized);
    normalized = MIN(RANGE_MAX, normalized);
    // Normalize to 0 to 255
    normalized = normalized - RANGE_MIN;
    normalized = (normalized * 255) / (RANGE_MAX - RANGE_MIN);
    return (u8) normalized;
}

int ADAPS_DTOF::FillSetWrapperParamFromEepromInfo(uint8_t* pEEPROMData, SetWrapperParam* setparam) {
    int result = 0;
    Utils *utils;
    int dump_roi_sram_size = 0;

    if (NULL_POINTER == pEEPROMData) {
        DBG_ERROR( "pEEPROMData is NULL for EEPROMInfo");
        return -1;
    }

#if 0
    DBG_NOTICE("sizeof(swift_eeprom_data_t)=%ld", sizeof(swift_eeprom_data_t));
    DBG_NOTICE("AD4001_EEPROM_VERSION_INFO_OFFSET: %d\n", AD4001_EEPROM_VERSION_INFO_OFFSET);
    DBG_NOTICE("AD4001_EEPROM_MODULE_INFO_OFFSET: %d\n", AD4001_EEPROM_MODULE_INFO_OFFSET);
    DBG_NOTICE("AD4001_EEPROM_INTRINSIC_OFFSET: %d\n", AD4001_EEPROM_INTRINSIC_OFFSET);
    DBG_NOTICE("AD4001_EEPROM_INTRINSIC_SIZE: %d\n", AD4001_EEPROM_INTRINSIC_SIZE);

    utils = new Utils();
    utils->hexdump((unsigned char *) pRawData, 180, "1st 180 bytes of EEPROM");
    delete utils;
#endif

    setparam->adapsLensIntrinsicData  = reinterpret_cast<float*>(pEEPROMData + AD4001_EEPROM_INTRINSIC_OFFSET);
    if (Utils::is_env_var_true(ENV_VAR_DUMP_LENS_INTRINSIC))
    {
        for (int i = 0; i < 9; i++)
        {
            DBG_NOTICE("intrinsic Parameters[%d] = %f", i, setparam->adapsLensIntrinsicData[i]);
        }
    }

#if (ADS6401_MODULE_SPOT == SWIFT_MODULE_TYPE)
    setparam->adapsSpodOffsetData     = reinterpret_cast<float*>(pEEPROMData + AD4001_EEPROM_SPOTOFFSET_OFFSET);
    setparam->proximity_hist          = pEEPROMData + AD4001_EEPROM_PROX_HISTOGRAM_OFFSET;
    setparam->accurateSpotPosData     = reinterpret_cast<float*>(pEEPROMData + AD4001_EEPROM_ACCURATESPODPOS_OFFSET);
#else
    setparam->adapsSpodOffsetData     = reinterpret_cast<float*>(pEEPROMData + AD4001_EEPROM_SPOTOFFSET_OFFSET+ONE_SPOD_OFFSET_BYTE_SIZE);  //pointer to offset0
    setparam->proximity_hist          = NULL_POINTER;
    setparam->accurateSpotPosData     = NULL_POINTER;
#endif

#if defined(ENABLE_DYNAMICALLY_UPDATE_ROI_SRAM_CONTENT)
    setparam->spot_cali_data          = multiple_roi_sram_test_data;
#else
    setparam->spot_cali_data          = pEEPROMData + AD4001_EEPROM_ROISRAM_DATA_OFFSET;
#endif

    dump_roi_sram_size = Utils::get_env_var_intvalue(ENV_VAR_DUMP_ROI_SRAM_SIZE);
    if (dump_roi_sram_size)
    {
        utils = new Utils();
        utils->hexdump((unsigned char *) setparam->spot_cali_data, dump_roi_sram_size, "ROI SRAM data dump to Algo lib");
        delete utils;
    }

    setparam->cali_ref_tempe[AdapsEnvTypeIndoor - 1]  = *reinterpret_cast<float*>(pEEPROMData + AD4001_EEPROM_INDOOR_CALIBTEMPERATURE_OFFSET);
    setparam->cali_ref_tempe[AdapsEnvTypeOutdoor - 1] = *reinterpret_cast<float*>(pEEPROMData + AD4001_EEPROM_OUTDOOR_CALIBTEMPERATURE_OFFSET);
    setparam->cali_ref_depth[AdapsEnvTypeIndoor - 1]  = *reinterpret_cast<float*>(pEEPROMData + AD4001_EEPROM_INDOOR_CALIBREFDISTANCE_OFFSET);
    setparam->cali_ref_depth[AdapsEnvTypeOutdoor - 1] = *reinterpret_cast<float*>(pEEPROMData + AD4001_EEPROM_OUTDOOR_CALIBREFDISTANCE_OFFSET);

#if (ADS6401_MODULE_SPOT == SWIFT_MODULE_TYPE)
    if (true == Utils::is_env_var_true(ENV_VAR_DISABLE_WALK_ERROR))
    {
    }
    else {
        setparam->calibrationInfo = pEEPROMData + AD4001_EEPROM_VERSION_INFO_OFFSET;
        setparam->walk_error_para_list = pEEPROMData + AD4001_EEPROM_WALK_ERROR_OFFSET;
    }
#endif

#ifdef WINDOWS_BUILD
    setparam->dump_data =false;
#endif

    return result;
}

void ADAPS_DTOF::initParams(WrapperDepthInitInputParams* initInputParams,WrapperDepthInitOutputParams* initOutputParams)
{
    struct adaps_dtof_exposure_param *p_exposure_param;
    Host_Communication *host_comm = Host_Communication::getInstance();

    if (NULL_POINTER == p_misc_device)
    {
        DBG_ERROR("p_misc_device is NULL");
        return;
    }

    memset(&set_param, 0, sizeof(SetWrapperParam));
    set_param.work_mode = static_cast<int>(m_sns_param.work_mode);

#if defined(ENABLE_DYNAMICALLY_UPDATE_ROI_SRAM_CONTENT)
    set_param.expand_pixel = false;
#else
    if (true == qApp->is_capture_req_from_host() && NULL_POINTER != host_comm)
    {
        set_param.expand_pixel = host_comm->get_req_expand_pixel();
    }
    else {
        set_param.expand_pixel = false;
    }
#endif

    if (true == qApp->is_capture_req_from_host() && NULL_POINTER != host_comm)
    {
        set_param.compose_subframe = host_comm->get_req_compose_subframe();
    }
    else {
        set_param.compose_subframe = true;
    }

    if (true == qApp->is_capture_req_from_host() && NULL_POINTER != host_comm)
    {
        set_param.mirror_frame.mirror_x = host_comm->get_req_mirror_x();
    }
    else {
        set_param.mirror_frame.mirror_x = false;
    }

    if (true == qApp->is_capture_req_from_host() && NULL_POINTER != host_comm)
    {
        set_param.mirror_frame.mirror_y = host_comm->get_req_mirror_y();
    }
    else {
        set_param.mirror_frame.mirror_y = false;
    }

    if (true == qApp->is_capture_req_from_host() && NULL_POINTER != host_comm)
    {
        set_param.walkerror = (0 != host_comm->get_req_walkerror_version());
    }
    else {
        set_param.walkerror = true;
    }

    // always allow to check environment variable to change these setting for debug.
    if (true == Utils::is_env_var_true(ENV_VAR_ENABLE_EXPAND_PIXEL))
    {
        set_param.expand_pixel = true;
    }

    if (true == Utils::is_env_var_true(ENV_VAR_DISABLE_COMPOSE_SUBFRAME))
    {
        set_param.compose_subframe = false;
    }

    if (true == Utils::is_env_var_true(ENV_VAR_MIRROR_X_ENABLE))
    {
        set_param.mirror_frame.mirror_x = true;
    }

    if (true == Utils::is_env_var_true(ENV_VAR_MIRROR_Y_ENABLE))
    {
        set_param.mirror_frame.mirror_y = true;
    }

    if (true == Utils::is_env_var_true(ENV_VAR_DISABLE_WALK_ERROR))
    {
        set_param.walkerror = false;
    }

    set_param.peak_index = 1;
    set_param.env_type = m_sns_param.env_type;
    set_param.measure_type = m_sns_param.measure_type;

    DepthMapWrapperGetVersion(m_DepthLibversion);
    set_param.OutAlgoVersion = (uint8_t*)m_DepthLibversion;
    DBG_NOTICE("depth algo lib version: %s", m_DepthLibversion);

    p_eeprominfo = (swift_eeprom_data_t *) p_misc_device->get_dtof_calib_eeprom_param();
    if (NULL_POINTER == p_eeprominfo) {
        DBG_ERROR("p_eeprominfo is NULL for EEPROMInfo");
        return ;
    }
    FillSetWrapperParamFromEepromInfo((uint8_t* ) p_eeprominfo, &set_param);

    p_exposure_param = (struct adaps_dtof_exposure_param *) p_misc_device->get_dtof_exposure_param();
    if (NULL_POINTER == p_exposure_param) {
        DBG_ERROR("p_exposure_param is NULL");
        return ;
    }
    set_param.exposure_period = p_exposure_param->exposure_period;
    set_param.ptm_fine_exposure_value = p_exposure_param->ptm_fine_exposure_value;

    DBG_INFO("ptm fine exposure value: 0x%x\n", set_param.ptm_fine_exposure_value);
    DBG_INFO("exposure period value: 0x%x\n", set_param.exposure_period);
    DBG_INFO("indoor calib temperature: %f, outdoor: %f\n",
           set_param.cali_ref_tempe[0], set_param.cali_ref_tempe[1]);
    DBG_INFO("indoor calib ref depth: %f, outdoor: %f\n",
           set_param.cali_ref_depth[0], set_param.cali_ref_depth[1]);

    initInputParams->width = m_sns_param.raw_width;
    initInputParams->height = m_sns_param.raw_height;
    initInputParams->dm_width = m_sns_param.out_frm_width;
    initInputParams->dm_height = m_sns_param.out_frm_height;

    initInputParams->pRawData = (uint8_t*) p_eeprominfo;
    initInputParams->rawDataSize = sizeof(swift_eeprom_data_t);
    initInputParams->configFilePath = m_DepthLibConfigXmlPath;
    sprintf(m_DepthLibConfigXmlPath, "%s", DEPTH_LIB_CONFIG_PATH);
    initInputParams->setparam = set_param;

    initOutputParams->exposure_time = &m_exposure_time;
    initOutputParams->sensitivity = &m_sensitivity;

    DBG_INFO("initParams set_param.work_mode=%d set_param.env_type=%d  set_param.measure_type=%d\n",
           set_param.work_mode,set_param.env_type,set_param.measure_type);
    DBG_INFO("initParams set_param.compose_subframe=%d set_param.expand_pixel=%d set_param.mirror_frame.mirror_x=%d  set_param.mirror_frame.mirror_y=%d\n",
           set_param.compose_subframe,set_param.expand_pixel,set_param.mirror_frame.mirror_x,set_param.mirror_frame.mirror_y);
    DBG_INFO("initParams set success\n");
}

int ADAPS_DTOF::adaps_dtof_initilize()
{
    int result = 0;
    WrapperDepthInitInputParams     initInputParams                 = {};
    WrapperDepthInitOutputParams    initOutputParams;

    initParams(&initInputParams,&initOutputParams);

    hexdump_param(&initInputParams, sizeof(WrapperDepthInitInputParams), "initInputParams", __LINE__);
    result = DepthMapWrapperCreate(&m_handlerDepthLib, initInputParams, initOutputParams);
    if (!m_handlerDepthLib || result < 0) {
        DBG_ERROR("Error creating depth map wrapper, result: %d, m_handlerDepthLib: %p", result, m_handlerDepthLib);
        return result;
    }

#if 0 //(ADS6401_MODULE_SPOT == SWIFT_MODULE_TYPE)
    CircleForMask circleForMask;
    circleForMask.CircleMaskCenterX = OUTPUT_WIDTH_4_DTOF_SENSOR;
    circleForMask.CircleMaskCenterY = OUTPUT_HEIGHT_4_DTOF_SENSOR;
    circleForMask.CircleMaskR = 0.0f;

    DepthMapWrapperSetCircleMask(m_handlerDepthLib,circleForMask);
#endif
    DBG_INFO("Adaps depth lib initialize okay.");

    m_conversionLibInited = true;

    return result;
}

void ADAPS_DTOF::adaps_dtof_release()
{
    if (NULL_POINTER != m_handlerDepthLib)
    {
        DepthMapWrapperDestroy(m_handlerDepthLib);
        DBG_INFO("Adaps depth lib destroy okay.");
        m_handlerDepthLib = NULL_POINTER;
    }
}

void ADAPS_DTOF::PrepareFrameParam(WrapperDepthCamConfig *wrapper_depth_map_config)
{
    float  t = 0.0f;

    if (-1 == p_misc_device->read_dtof_runtime_status_param(&t)) {
        DBG_ERROR("Fail to read temperature, errno: %s (%d)...", 
            strerror(errno), errno);
        return;
    }

    if (t < CHIP_TEMPERATURE_MIN_THRESHOLD) {// Eg. m_currToFTemperature in camxsensornode.cpp is not set since temperature read failure.
        DBG_ERROR("too low temperature: %f", t);
        wrapper_depth_map_config->frame_parameters.laser_realtime_tempe = CHIP_TEMPERATURE_MIN_THRESHOLD;
    }
    else if (t > CHIP_TEMPERATURE_MAX_THRESHOLD) {// Eg. m_currToFTemperature in camxsensornode.cpp is not set since temperature read failure.
        DBG_ERROR("too high temperature: %f", t);
        wrapper_depth_map_config->frame_parameters.laser_realtime_tempe = CHIP_TEMPERATURE_MAX_THRESHOLD;
    }
    else {
        wrapper_depth_map_config->frame_parameters.laser_realtime_tempe = t;
    }
    //wrapper_depth_map_config->frame_parameters.laser_realtime_tempe = CHIP_TEMPERATURE_MAX_THRESHOLD;

    //DBG_INFO( "adapsChipTemperature: %f\n " ,wrapper_depth_map_config->frame_parameters.laser_realtime_tempe);

    wrapper_depth_map_config->frame_parameters.measure_type_in = (AdapsMeasurementType) m_sns_param.measure_type;
    //DBG_INFO("AdapsMeasurementType: %d \n" , wrapper_depth_map_config->frame_parameters.measure_type_in);

    wrapper_depth_map_config->frame_parameters.focutPoint[0] = 1;
    wrapper_depth_map_config->frame_parameters.focutPoint[1] = 2; //need to get this value
    /*DBG_INFO("Adaps FocusPoint:x %d y %d \n", 
                wrapper_depth_map_config->frame_parameters.focutPoint[0],
                wrapper_depth_map_config->frame_parameters.focutPoint[1]);*/

    wrapper_depth_map_config->frame_parameters.env_type_in = (AdapsEnvironmentType) m_sns_param.env_type;
   // DBG_INFO("AdapsEnvironmentType: %d \n" , wrapper_depth_map_config->frame_parameters.env_type_in);
#if defined(ENABLE_DYNAMICALLY_UPDATE_ROI_SRAM_CONTENT)
    wrapper_depth_map_config->frame_parameters.calib_sram_data_group_idx = cur_calib_sram_data_group_idx;
#endif

    wrapper_depth_map_config->frame_parameters.advised_env_type_out     = &m_sns_param.advisedEnvType;
    wrapper_depth_map_config->frame_parameters.advised_measure_type_out = &m_sns_param.advisedMeasureType;

    wrapper_depth_map_config->frame_parameters.focusRoi.FocusLeftTopX = 0;
    wrapper_depth_map_config->frame_parameters.focusRoi.FocusLeftTopY = 0;
    wrapper_depth_map_config->frame_parameters.focusRoi.FocusRightBottomX = m_sns_param.out_frm_width;
    wrapper_depth_map_config->frame_parameters.focusRoi.FocusRightBottomY = m_sns_param.out_frm_height;
}

void ADAPS_DTOF::Distance_2_BGRColor(int bucketNum, float bucketSize, u16 distance, struct BGRColor *destColor)
{
    float scale = (float)((distance -  bucketNum * bucketSize) / bucketSize);
    destColor->Blue = (u8)(m_basic_colors[bucketNum].Blue + (m_basic_colors[bucketNum+1].Blue - m_basic_colors[bucketNum].Blue) * scale);
    destColor->Green = (u8)(m_basic_colors[bucketNum].Green + (m_basic_colors[bucketNum+1].Green - m_basic_colors[bucketNum].Green) * scale);
    destColor->Red = (u8)(m_basic_colors[bucketNum].Red + (m_basic_colors[bucketNum+1].Red - m_basic_colors[bucketNum].Red) * scale);
}

#if defined(ENABLE_DYNAMICALLY_UPDATE_ROI_SRAM_CONTENT)
int ADAPS_DTOF::DepthBufferMerge(u16 merged_depth16_buffer[], const u16 to_merge_depth16_buffer[], int outImgWidth, int outImgHeight)
{
    int index;
    int  i, j;
    u8 to_merge_confidence;
    u16 merged_depth16, to_merge_depth16;
    u16 to_merge_distance;
    int new_added_spot_count = 0;

    if (NULL_POINTER == merged_depth16_buffer) {
        DBG_ERROR( "merged_depth16_buffer is NULL");
        return -1;
    }

    if (NULL_POINTER == to_merge_depth16_buffer) {
        DBG_ERROR( "to_merge_depth16_buffer is NULL");
        return -1;
    }

    for (j = 0; j < outImgHeight; j++)
    {
        for (i = 0; i < outImgWidth; i++)
        {
            index = j * outImgWidth + i;
            to_merge_depth16 = to_merge_depth16_buffer[index];
            to_merge_distance = to_merge_depth16 & DEPTH_MASK;

            if (0 != to_merge_distance)
            {
                merged_depth16 = merged_depth16_buffer[index];
                to_merge_confidence = (to_merge_depth16 >> DEPTH_BITS) & CONFIDENCE_MASK;

                if ((ANDROID_CONF_LOW != to_merge_confidence) && (0 == merged_depth16))
                {
                    merged_depth16_buffer[index] = to_merge_depth16;
                    new_added_spot_count++;
                }
                else {
                    if (0 != merged_depth16)
                    {
                       // DBG_INFO("--- frm_cnt: %d, Spot(%d, %d) exists in merged frame already, merged_depth: 0x%04x, to_merge_depth: 0x%04x...", m_decoded_frame_cnt, i, j, merged_depth16, to_merge_depth16);
                    }
                    else {
                       // DBG_NOTICE("--- frm_cnt: %d, Spot(%d, %d) has too low confidence, to_merge_depth: 0x%04x...", m_decoded_frame_cnt, i, j, to_merge_depth16);
                    }
                }
            }
        }
    }

    if (trace_calib_sram_switch)
    {
        DBG_NOTICE("### frame_%d new_added_spot_count: %d", m_decoded_frame_cnt, new_added_spot_count);
    }
    return new_added_spot_count;
}
#endif

void ADAPS_DTOF::GetDepth4watchSpot(const u16 depth16_buffer[], const int outImgWidth, u8 x, u8 y, u16 *distance, u8 *confidence)
{
    int rawImgIdx;
    rawImgIdx = y * outImgWidth + x;

    *distance = depth16_buffer[rawImgIdx] & DEPTH_MASK;
    *confidence = (depth16_buffer[rawImgIdx] >> DEPTH_BITS) & CONFIDENCE_MASK;
}

void ADAPS_DTOF::ConvertDepthToColoredMap(const u16 depth16_buffer[], u8 depth_colored_map[], u8 depth_confidence_map[], const int outImgWidth, const int outImgHeight)
{
    int rawImgIdx;
    struct BGRColor bgrColor;
    u16 distance;
    u8 confidence;
    float bucketSize;
    int bucketNum;
    int  i, j;
    int non_zero_depth_count = 0;

    for (j = 0; j < outImgHeight; j++) {
        for (i = 0; i < outImgWidth; i++) {
            rawImgIdx = j * outImgWidth + i;
            distance = depth16_buffer[rawImgIdx] & DEPTH_MASK;
            confidence = (depth16_buffer[rawImgIdx] >> DEPTH_BITS) & CONFIDENCE_MASK;

#if 1
                double maxDepth = 30.0; // private static double maxDepth = 30.0; from line of SpadisPC\spadisApp\Models\GenerateColorMap.cs
                double distance_db = (double) (distance_db / 1000.0);

                float rangeLow = qApp->get_RealDistanceMinMappedRange();
                float rangeHigh = qApp->get_RealDistanceMaxMappedRange();

                if (distance_db > maxDepth) distance_db = maxDepth; // If the value is out of the range
                if (distance_db < 0.0) distance_db = 0.0;

                bucketSize = (rangeHigh - rangeLow) / 4.0f;
                if (bucketSize <= 0.001f)
                {
                    bgrColor.Red = m_basic_colors[0].Red;
                    bgrColor.Green = m_basic_colors[0].Green;
                    bgrColor.Blue = m_basic_colors[0].Blue;
                }
                else
                {
                    // Seperate 1023 in 4 buckets, each one has 256 number
                    bucketNum = (int)((distance_db - rangeLow) / bucketSize);

                    if (distance_db > rangeLow && bucketNum <= 3)
                    {
                        Distance_2_BGRColor(bucketNum, bucketSize, (distance_db - m_rangeLow), &bgrColor);
                    }
                    else if (distance_db == 0.0) // Show as black
                    {
                        bgrColor.Red = 0;
                        bgrColor.Green = 0;
                        bgrColor.Blue = 0;
                    }
                    else if (distance_db <= rangeLow) // Show as colors[0]
                    {
                        bgrColor.Red = m_basic_colors[0].Red;
                        bgrColor.Green = m_basic_colors[0].Green;
                        bgrColor.Blue = m_basic_colors[0].Blue;
                    }
                    else // Show as colors[4]
                    {
                        bgrColor.Red = m_basic_colors[4].Red;
                        bgrColor.Green = m_basic_colors[4].Green;
                        bgrColor.Blue = m_basic_colors[4].Blue;
                    }
                }
#else
            if (distance > m_LimitedMaxDistance)
            {
                distance = m_LimitedMaxDistance; // If the value is out of the range
            }

            bucketSize = (m_rangeHigh - m_rangeLow) / 4.0f;
            bucketNum = (int)((distance - m_rangeLow) / bucketSize);

            if (distance > m_rangeLow && bucketNum <= 3)
            {
                Distance_2_BGRColor(bucketNum, bucketSize, (distance - m_rangeLow), &bgrColor);
            }
            else if (distance == 0) { // Show as black
                bgrColor.Red = 0;
                bgrColor.Green = 0;
                bgrColor.Blue = 0;
            }
            else if (distance <= m_rangeLow)
            { // Show as colors[0]
                bgrColor.Red = m_basic_colors[0].Red;
                bgrColor.Green = m_basic_colors[0].Green;
                bgrColor.Blue = m_basic_colors[0].Blue;

                if (Utils::is_env_var_true(ENV_VAR_DUMP_SPOT_DEPTH))
                {
                    DBG_NOTICE("### frm_cnt: %d,Spot(%d, %d) distance: %d mm, confidence: %d, non_zero_depth_count: %d, bgrColor(%d, %d, %d)",
                        m_decoded_frame_cnt, i, j, distance, confidence, non_zero_depth_count, bgrColor.Blue, bgrColor.Green, bgrColor.Red);
                }
            }
            else { // Show as colors[4]
                bgrColor.Red = m_basic_colors[4].Red;
                bgrColor.Green = m_basic_colors[4].Green;
                bgrColor.Blue = m_basic_colors[4].Blue;

                if (Utils::is_env_var_true(ENV_VAR_DUMP_SPOT_DEPTH))
                {
                    DBG_NOTICE("--- frm_cnt: %d, Spot(%d, %d) distance: %d mm, confidence: %d, non_zero_depth_count: %d", m_decoded_frame_cnt, i, j, distance, confidence, non_zero_depth_count);
                }
            }
#endif

            depth_colored_map[rawImgIdx * 3 + 0] = bgrColor.Red;
            depth_colored_map[rawImgIdx * 3 + 1] = bgrColor.Green;
            depth_colored_map[rawImgIdx * 3 + 2] = bgrColor.Blue;

            if (0 != distance)
            {
                non_zero_depth_count++;

                if (ANDROID_CONF_LOW == confidence)
                {
                    depth_confidence_map[rawImgIdx * 3 + 0] = 0xFF; //Red;
                    depth_confidence_map[rawImgIdx * 3 + 1] = 0x00; //Green;
                    depth_confidence_map[rawImgIdx * 3 + 2] = 0x00; //Blue;
                }
                else if (ANDROID_CONF_HIGH == confidence)
                {
                    depth_confidence_map[rawImgIdx * 3 + 0] = 0x00; //Red;
                    depth_confidence_map[rawImgIdx * 3 + 1] = 0xFF; //Green;
                    depth_confidence_map[rawImgIdx * 3 + 2] = 0x00; //Blue;
                }
                else {
                    if (Utils::is_env_var_true(ENV_VAR_DUMP_MID_CONF_ENABLE))
                    {
                        DBG_NOTICE("frm_cnt: %d, non_zero_depth_count: %d, Spot(%d, %d) depth16: 0x%x, distance: %d mm, confidence: %d", 
                            m_decoded_frame_cnt, non_zero_depth_count, i, j, depth16_buffer[rawImgIdx], distance, confidence);
                    }
                    depth_confidence_map[rawImgIdx * 3 + 0] = 0x00; //Red;
                    depth_confidence_map[rawImgIdx * 3 + 1] = 0x00; //Green;
                    depth_confidence_map[rawImgIdx * 3 + 2] = 0xFF; //Blue;
                }
            }
            else {
                // show BLACK for those spots whose distance is 0.
                depth_confidence_map[rawImgIdx * 3 + 0] = 0x00; //Red;
                depth_confidence_map[rawImgIdx * 3 + 1] = 0x00; //Green;
                depth_confidence_map[rawImgIdx * 3 + 2] = 0x00; //Blue;
            }
        }
    }

#if defined(ENABLE_DYNAMICALLY_UPDATE_ROI_SRAM_CONTENT)
    if (trace_calib_sram_switch)
    {
        DBG_NOTICE("### frame_%d non_zero_depth_count: %d, set_param.expand_pixel: %d", m_decoded_frame_cnt, non_zero_depth_count, set_param.expand_pixel);
    }
    else {
        DBG_INFO("### frame_%d non_zero_depth_count: %d, set_param.expand_pixel: %d", m_decoded_frame_cnt, non_zero_depth_count, set_param.expand_pixel);
    }
#else
    DBG_INFO("### frame_%d non_zero_depth_count: %d, set_param.expand_pixel: %d", m_decoded_frame_cnt, non_zero_depth_count, set_param.expand_pixel);
#endif
}

void ADAPS_DTOF::ConvertGreyscaleToColoredMap(u16 depth16_buffer[], u8 greyscale_colored_map[], int outImgWidth, int outImgHeight)
{
#if 1
    int i;
    u16 value;
    double scale;
    double range;
    double imgValue;
    int maxColor = 255;
    u16 GrayScaleMinMappedRange = qApp->get_GrayScaleMinMappedRange();
    u16 GrayScaleMaxMappedRange = qApp->get_GrayScaleMaxMappedRange();

    range = GrayScaleMaxMappedRange - GrayScaleMinMappedRange;
    
    for (i = 0; i < outImgHeight * outImgWidth; i++)
    {
        value = depth16_buffer[i];
        if (value <= GrayScaleMinMappedRange)
        {
            imgValue = 0;
        }
        else if (value >= GrayScaleMaxMappedRange)
        {
            imgValue = maxColor;
        }
        else
        {
            scale = (value - GrayScaleMinMappedRange) / range;
            imgValue = scale * maxColor;
        }
        greyscale_colored_map[i * 3] = (u8)imgValue;
        greyscale_colored_map[i * 3 + 1] = (u8)imgValue;
        greyscale_colored_map[i * 3 + 2] = (u8)imgValue;
    }
#else
    int rawImgIdx;
    int  i, j, rgb_index = 0;
    u16 distance;
    u8 greyColor;

    for (j = 0; j < outImgHeight; j++) {
        for (i = 0; i < outImgWidth; i++) {
            rawImgIdx = j * outImgWidth + i;
            distance = depth16_buffer[rawImgIdx] & DEPTH_MASK;
            greyColor = normalizeGreyscale(distance);

            greyscale_colored_map[rgb_index * 3 + 0] = greyColor;
            greyscale_colored_map[rgb_index * 3 + 1] = greyColor;
            greyscale_colored_map[rgb_index * 3 + 2] = greyColor;
            rgb_index++;

        }
    }
#endif
}

int ADAPS_DTOF::dtof_frame_decode(unsigned char *frm_rawdata, int frm_rawdata_size, u16 depth16_buffer[], enum sensor_workmode swk)
{
    WrapperDepthCamConfig config;
    int result=0;
    bool done = false;
    uint32_t req_output_stream_cnt = 0;
    // Host_Communication *host_comm = Host_Communication::getInstance();

    Q_UNUSED(swk);
    WrapperDepthOutput outputs[MAX_DEPTH_OUTPUT_FORMATS];
    WrapperDepthInput depthInput;

    if (false == m_conversionLibInited)
    {
        DBG_ERROR("ConversionLib Init Fail \n");
        return -1;
    }

    if (NULL_POINTER == p_misc_device)
    {
        DBG_ERROR("p_misc_device is NULL");
        return -1;
    }

#if 0
    if (host_comm)
    {
        if ((0 != (host_comm->get_req_out_data_type() & FRAME_DECODED_DEPTH16)) && (0 != (host_comm->get_req_out_data_type() & FRAME_DECODED_POINT_CLOUD)))
        {
            outputs[0].format                    = WRAPPER_CAM_FORMAT_DEPTH16;
            outputs[0].out_image_length          = m_sns_param.out_frm_width*m_sns_param.out_frm_height*sizeof(u16);
            outputs[0].formatParams.bitsPerPixel = 16;
            outputs[0].formatParams.strideBytes  = m_sns_param.out_frm_width;
            outputs[0].formatParams.sliceHeight  = m_sns_param.out_frm_height;
            outputs[0].out_depth_image = (uint8_t*) depth16_buffer;

            outputs[1].format                    = WRAPPER_CAM_FORMAT_DEPTH_POINT_CLOUD;
            outputs[1].count_pt_cloud          = m_sns_param.out_frm_width*m_sns_param.out_frm_height*sizeof(u16);
            outputs[1].formatParams.bitsPerPixel = 16;
            outputs[1].formatParams.strideBytes  = m_sns_param.out_frm_width*2;
            outputs[1].formatParams.sliceHeight  = m_sns_param.out_frm_height;
            outputs[1].out_pcloud_image = m_out_put_pointcloud_buffer;

            req_output_stream_cnt = 2;
        }
        else if (0 != (host_comm->get_req_out_data_type() & FRAME_DECODED_DEPTH16))
        {
            outputs[0].format                    = WRAPPER_CAM_FORMAT_DEPTH16;
            outputs[0].out_image_length          = m_sns_param.out_frm_width*m_sns_param.out_frm_height*sizeof(u16);
            outputs[0].formatParams.bitsPerPixel = 16;
            outputs[0].formatParams.strideBytes  = m_sns_param.out_frm_width;
            outputs[0].formatParams.sliceHeight  = m_sns_param.out_frm_height;
            outputs[0].out_depth_image = (uint8_t*) depth16_buffer;

            req_output_stream_cnt = 1;
        }
        else if (0 != (host_comm->get_req_out_data_type() & FRAME_DECODED_POINT_CLOUD))
        {
            outputs[0].format                    = WRAPPER_CAM_FORMAT_DEPTH_POINT_CLOUD;
            outputs[0].out_image_length          = m_sns_param.out_frm_width*m_sns_param.out_frm_height*sizeof(u16);
            outputs[0].formatParams.bitsPerPixel = 16;
            outputs[0].formatParams.strideBytes  = m_sns_param.out_frm_width;
            outputs[0].formatParams.sliceHeight  = m_sns_param.out_frm_height;
            outputs[0].out_depth_image = (uint8_t*) depth16_buffer;
        
            req_output_stream_cnt = 1;
        }
    }
    else 
#endif
    {
        outputs[0].format                    = WRAPPER_CAM_FORMAT_DEPTH16;
        outputs[0].out_image_length          = m_sns_param.out_frm_width*m_sns_param.out_frm_height*sizeof(u16);
        outputs[0].formatParams.bitsPerPixel = 16;
        outputs[0].formatParams.strideBytes  = m_sns_param.out_frm_width;
        outputs[0].formatParams.sliceHeight  = m_sns_param.out_frm_height;
        outputs[0].out_depth_image = (uint8_t*) depth16_buffer;
        req_output_stream_cnt = 1;
    }

    depthInput.in_image    = (const int8_t*)frm_rawdata;
    depthInput.formatParams.bitsPerPixel = 8;
    depthInput.formatParams.strideBytes  = m_sns_param.raw_width;
    depthInput.formatParams.sliceHeight  = m_sns_param.raw_height;
    depthInput.in_image_size    = frm_rawdata_size;
    //DBG_INFO( "raw_width: %d raw_height: %d out_width: %d out_height: %d\n", m_sns_param.raw_width, m_sns_param.raw_height, m_sns_param.out_frm_width, m_sns_param.out_frm_height);

    PrepareFrameParam(&config);

    //BOOL disableAlgo = CamX::OsUtils::GetPropertyBool("debug.adaps.disableAlgo", false);
    bool disableAlgo =false;

    if (false == disableAlgo)
    {
        if (0 == m_decoded_frame_cnt)
        {
            hexdump_param(&depthInput, sizeof(WrapperDepthInput), "depthInput", __LINE__);
            hexdump_param(&config, sizeof(WrapperDepthCamConfig), "config", __LINE__);
            hexdump_param(&outputs, sizeof(WrapperDepthOutput), "output0", __LINE__);
        }

        done = DepthMapWrapperProcessFrame(m_handlerDepthLib,
                                    depthInput,
                                    &config,
                                    req_output_stream_cnt,
                                    outputs);
        m_decoded_frame_cnt++;

#if defined(ENABLE_DYNAMICALLY_UPDATE_ROI_SRAM_CONTENT)
        if (trace_calib_sram_switch)
        {
            DBG_NOTICE( "sram_data_group_idx: %d done: %d m_decoded_frame_cnt: %d", config.frame_parameters.calib_sram_data_group_idx, done, m_decoded_frame_cnt);
        }
#endif
    }
    else {
        static  uint32_t count_g=0;
        count_g++;
        if(count_g%4 == 0)
            done = 1;
        else
            done = 0;

       DBG_INFO( " bypass the algo done=%d \n",done);
    }

    if (true == done)
    {
        result = 0;

#if defined(ENABLE_DYNAMICALLY_UPDATE_ROI_SRAM_CONTENT)
        cur_calib_sram_data_group_idx++;
        if (cur_calib_sram_data_group_idx >= MAX_CALIB_SRAM_DATA_GROUP_CNT)
        {
            cur_calib_sram_data_group_idx = 0;
            result = 0;
        }
        else {
            if (WK_DTOF_PCM != m_sns_param.work_mode)
            {
                result = -1; // there are more sub-frames to be merged,so return -1 here.
            }
        }
#endif
    }
    else {
        //DBG_ERROR("DepthMapWrapperProcessFrame() return false\n");
        result = -1;
    }

    return result;
}

