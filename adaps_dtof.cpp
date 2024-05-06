#include "adaps_dtof.h"
#include <dlfcn.h>

ADAPS_DTOF::ADAPS_DTOF(struct sensor_params params, V4L2 *v4l2)
{
    memcpy(&m_sns_param, &params, sizeof(struct sensor_params));

    m_basic_colors[0] = {255, 0, 0};
    m_basic_colors[1] = {255, 255, 0};
    m_basic_colors[2] = {0, 255, 0};
    m_basic_colors[3] = {0, 255, 255};
    m_basic_colors[4] = {0, 0, 255};

    m_LimitedMaxDistance = COLOR_MAP_HIGH;
    m_rangeHigh = COLOR_MAP_HIGH;
    m_rangeLow = RANGE_MIN;
    m_conversionLibInited = false;
    m_v4l2 = v4l2;
}


ADAPS_DTOF::~ADAPS_DTOF()
{
    m_v4l2 = NULL;
}

void ADAPS_DTOF::mode_switch(struct sensor_params params, V4L2 *v4l2)
{
    memcpy(&m_sns_param, &params, sizeof(struct sensor_params));
    m_conversionLibInited = false;
    m_v4l2 = v4l2;
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

int ADAPS_DTOF::GetAdapsTofEepromInfo(uint8_t* pRawData, uint32_t rawDataSize, SetWrapperParam* setparam) {
    int result = 0;

    if (NULL == pRawData) {
        DBG_ERROR( "pRawData is NULL for EEPROMInfo");
        return -1;
    }

    if (rawDataSize < sizeof(swift_eeprom_data_t)) {
        DBG_ERROR("rawDataSize %d is illegal, The total size is %ld"
            , rawDataSize, sizeof(swift_eeprom_data_t));
        return -1;
    }

    setparam->adapsLensIntrinsicData  = reinterpret_cast<float*>(pRawData + AD4001_EEPROM_INTRINSIC_OFFSET);
    setparam->adapsSpodOffsetData     = reinterpret_cast<float*>(pRawData + AD4001_EEPROM_SPODOFFSET_OFFSET);
    setparam->proximity_hist          = pRawData + AD4001_EEPROM_PROX_HISTOGRAM_OFFSET;
    setparam->accurateSpotPosData     = reinterpret_cast<float*>(pRawData + AD4001_EEPROM_ACCURATESPODPOS_OFFSET);

    setparam->cali_ref_tempe[AdapsEnvTypeIndoor - 1]  = *reinterpret_cast<float*>(pRawData + AD4001_EEPROM_INDOOR_CALIBTEMPERATURE_OFFSET);
    setparam->cali_ref_tempe[AdapsEnvTypeOutdoor - 1] = *reinterpret_cast<float*>(pRawData + AD4001_EEPROM_OUTDOOR_CALIBTEMPERATURE_OFFSET);
    setparam->cali_ref_depth[AdapsEnvTypeIndoor - 1]  = *reinterpret_cast<float*>(pRawData + AD4001_EEPROM_INDOOR_CALIBREFDISTANCE_OFFSET);
    setparam->cali_ref_depth[AdapsEnvTypeOutdoor - 1] = *reinterpret_cast<float*>(pRawData + AD4001_EEPROM_OUTDOOR_CALIBREFDISTANCE_OFFSET);

    return result;
}

void ADAPS_DTOF::initParams(WrapperDepthInitInputParams  *     initInputParams,WrapperDepthInitOutputParams      *initOutputParams)
{
    SetWrapperParam set_param;
    unsigned char version[32];

    memset(&set_param, 0, sizeof(SetWrapperParam));
    set_param.work_mode = static_cast<int>(m_sns_param.work_mode);
    set_param.compose_subframe = true;
    set_param.expand_pixel = true;
    set_param.mirror_frame.mirror_x = false;
    set_param.mirror_frame.mirror_y = false;
    set_param.walkerror=1;  
    set_param.env_type = m_sns_param.env_type;
    set_param.measure_type = m_sns_param.measure_type;

    set_param.OutAlgoVersion = (uint8_t*)version;

    p_eeprominfo = (struct adaps_get_eeprom *) m_v4l2->adaps_getEEPROMData();
    if (NULL == p_eeprominfo) {
        DBG_ERROR( "p_eeprominfo is NULL for EEPROMInfo");
        return ;
    }
    GetAdapsTofEepromInfo(p_eeprominfo->pRawData, p_eeprominfo->rawDataSize, &set_param);


    DBG_INFO("ptm fine exposure value: %d\n", set_param.ptm_fine_exposure_value);
    DBG_INFO("exposure period value: %d\n", set_param.exposure_period);
    DBG_INFO("indoor calib temeprature: %f, outdoor: %f\n",
           set_param.cali_ref_tempe[0], set_param.cali_ref_tempe[1]);
    DBG_INFO("indoor calib ref depth: %f, outdoor: %f\n",
           set_param.cali_ref_depth[0], set_param.cali_ref_depth[1]);

    initInputParams->width = m_sns_param.raw_width;
    initInputParams->height = m_sns_param.raw_height;
    initInputParams->dm_width = m_sns_param.out_frm_width;
    initInputParams->dm_height = m_sns_param.out_frm_height;
    initInputParams->is_secure = false;

    //need to check the below code   attention
    initInputParams->pRawData = p_eeprominfo->pRawData;
    initInputParams->rawDataSize = p_eeprominfo->rawDataSize;
    
    initInputParams->setparam = set_param;

    initOutputParams->exposure_time = &m_exposure_time;
    initOutputParams->sensitivity = &m_sensitivity;

    //DBG_INFO("outputParams set_param.work_mode=%d  set_param.compose_subframe=%d set_param.expand_pixel=%d set_param.env_type=%d  set_param.measure_type=%d\n",
           //set_param.work_mode,set_param.compose_subframe,set_param.expand_pixel,set_param.env_type,set_param.measure_type);
    DBG_INFO("outputParams set success\n");
}

int ADAPS_DTOF::initilize()
{
    int result = 0;
    WrapperDepthInitInputParams     initInputParams                 = {};
    WrapperDepthInitOutputParams    initOutputParams;
    const unsigned int bind_flags = RTLD_NOW | RTLD_LOCAL;
    const char *libFullName="/usr/lib/libadaps_swift_decode.so";

    m_hDepthLib = dlopen(libFullName, bind_flags);

    if (NULL == m_hDepthLib)
    {
        DBG_ERROR("Error Loading Library: %s, error:%s\n", libFullName, dlerror());
        return 0 - __LINE__;
    }

    if (NULL != m_hDepthLib)
    {
        m_createDepthMapWrapper = reinterpret_cast<CREATEDEPTHMAPWRAPPER>(
             dlsym(m_hDepthLib, "DepthMapWrapperCreate"));
        m_destroyDepthMapWrapper = reinterpret_cast<DESTROYDEPTHMAPWRAPPER>(
             dlsym(m_hDepthLib, "DepthMapWrapperDestroy"));
        m_processFrame = reinterpret_cast<PROCESSFRAME>(dlsym(m_hDepthLib,
                    "DepthMapWrapperProcessFrame"));
        if (NULL == m_createDepthMapWrapper || NULL == m_destroyDepthMapWrapper || NULL == m_processFrame)
        {
            DBG_ERROR("Error getting function address(es) from library: %s", libFullName);
            result = 0 - __LINE__;
        }
        else
        {   
            initParams(&initInputParams,&initOutputParams);
            m_DepthMapWrapper    = m_createDepthMapWrapper(initInputParams, initOutputParams);
            if (NULL == m_DepthMapWrapper)
            {
                 DBG_ERROR("Error creating depth map wrapper \n");
                 result = 0 - __LINE__;
            }
            else
            {
                m_conversionLibInited = true;
                DBG_INFO( "Successfully creating DepthMapWrapper %p \n",m_DepthMapWrapper);
             }
         }
    }
    else
    {
        DBG_ERROR( "Error loading lib %s \n", libFullName);
        result = 0 - __LINE__;
    }
    return result;
}

void ADAPS_DTOF::release()
{
    if (NULL != m_DepthMapWrapper)
    {
        m_destroyDepthMapWrapper(m_DepthMapWrapper);
        m_DepthMapWrapper = NULL;

    }

    if (NULL != m_hDepthLib)
    {
        int result = 0;
        if (0 != dlclose(m_hDepthLib))
        {
            result = -1;
        }

        if (0 != result)
        {
            DBG_ERROR( "Failed to dlclose lib");
        }
        m_hDepthLib = NULL;
    }
}

void ADAPS_DTOF::PrepareFrameParam(WrapperDepthCamConfig *wrapper_depth_map_config)
{
    float  t = 0.0f;

    if (-1 == m_v4l2->adaps_readTemperatureOfDtofSubdev(&t)) {
        DBG_ERROR("Fail to read temperature, errno: %s (%d)...", 
            strerror(errno), errno);
        return;
    }

    if (t < CHIP_TEMPERATURE_THRESHOLD) {// Eg. m_currToFTemperature in camxsensornode.cpp is not set since temperature read failure.
        wrapper_depth_map_config->frame_parameters.laser_realtime_tempe = CHIP_TEMPERATURE_THRESHOLD;
    } else {
        wrapper_depth_map_config->frame_parameters.laser_realtime_tempe = t;
    }

    //DBG_INFO( "PrepareFrameParam adapsChipTemperature: %f\n " ,wrapper_depth_map_config->frame_parameters.laser_realtime_tempe);

    wrapper_depth_map_config->frame_parameters.measure_type_in = (AdapsMeasurementType) m_sns_param.measure_type;    
    //DBG_INFO("PrepareFrameParam AdapsMeasurementType: %d \n" , wrapper_depth_map_config->frame_parameters.measure_type_in);
            
    wrapper_depth_map_config->frame_parameters.focutPoint[0] = 1;
    wrapper_depth_map_config->frame_parameters.focutPoint[1] = 2; //need to get this value
    /*DBG_INFO("PrepareFrameParam Adaps FocusPoint:x %d y %d \n", 
                wrapper_depth_map_config->frame_parameters.focutPoint[0], 
                wrapper_depth_map_config->frame_parameters.focutPoint[1]);*/

    wrapper_depth_map_config->frame_parameters.env_type_in = (AdapsEnvironmentType) m_sns_param.env_type;
   // DBG_INFO("PrepareFrameParam AdapsEnvironmentType: %d \n" , wrapper_depth_map_config->frame_parameters.env_type_in);

    wrapper_depth_map_config->frame_parameters.advised_env_type_out     = &m_sns_param.advisedEnvType;
    wrapper_depth_map_config->frame_parameters.advised_measure_type_out = &m_sns_param.advisedMeasureType;
}

void ADAPS_DTOF::Distance_2_BGRColor(int bucketNum, float bucketSize, u16 distance, struct BGRColor *destColor)
{
    float scale = (float)((distance -  bucketNum * bucketSize) / bucketSize);
    destColor->Blue = (u8)(m_basic_colors[bucketNum].Blue + (m_basic_colors[bucketNum+1].Blue - m_basic_colors[bucketNum].Blue) * scale);
    destColor->Green = (u8)(m_basic_colors[bucketNum].Green + (m_basic_colors[bucketNum+1].Green - m_basic_colors[bucketNum].Green) * scale);
    destColor->Red = (u8)(m_basic_colors[bucketNum].Red + (m_basic_colors[bucketNum+1].Red - m_basic_colors[bucketNum].Red) * scale);
}

void ADAPS_DTOF::ConvertDepthToColoredMap(u16 depth16_buffer[], u8 depth_colored_map[], int outImgWidth, int outImgHeight)
{
    int rawImgIdx;
    struct BGRColor bgrColor;
    int  i, j, rgb_index = 0;

    for (j = 0; j < outImgHeight; j++) {
        for (i = 0; i < outImgWidth; i++) {
            rawImgIdx = j * outImgWidth + i;
            u16 distance = depth16_buffer[rawImgIdx] & DEPTH_MASK;

            if (distance > m_LimitedMaxDistance) distance = m_LimitedMaxDistance; // If the value is out of the range

            float bucketSize = (m_rangeHigh - m_rangeLow) / 4.0f;
            int bucketNum = (int)((distance - m_rangeLow) / bucketSize);
            
            if (distance > m_rangeLow && bucketNum <= 3) {
                Distance_2_BGRColor(bucketNum, bucketSize, (distance - m_rangeLow), &bgrColor);
            }
            else if (distance == 0.0) { // Show as black
                bgrColor.Red = 0;
                bgrColor.Green = 0;
                bgrColor.Blue = 0;
            }
            else if (distance <= m_rangeLow) { // Show as colors[0]
                bgrColor.Red = m_basic_colors[0].Red;
                bgrColor.Green = m_basic_colors[0].Green;
                bgrColor.Blue = m_basic_colors[0].Blue;
            }
            else { // Show as colors[4]
                bgrColor.Red = m_basic_colors[4].Red;
                bgrColor.Green = m_basic_colors[4].Green;
                bgrColor.Blue = m_basic_colors[4].Blue;
            }
            depth_colored_map[rgb_index * 3 + 0] = bgrColor.Red;
            depth_colored_map[rgb_index * 3 + 1] = bgrColor.Green;
            depth_colored_map[rgb_index * 3 + 2] = bgrColor.Blue;
            rgb_index++;
        }
    }
}

void ADAPS_DTOF::ConvertGreyscaleToColoredMap(u16 depth16_buffer[], u8 depth_colored_map[], int outImgWidth, int outImgHeight)
{
    int rawImgIdx;
    int  i, j, rgb_index = 0;

    for (j = 0; j < outImgHeight; j++) {
        for (i = 0; i < outImgWidth; i++) {
            rawImgIdx = j * outImgWidth + i;
            u16 distance = depth16_buffer[rawImgIdx] & DEPTH_MASK;
            u8 greyColor = normalizeGreyscale(distance);

            depth_colored_map[rgb_index * 3 + 0] = greyColor;
            depth_colored_map[rgb_index * 3 + 1] = greyColor;
            depth_colored_map[rgb_index * 3 + 2] = greyColor;
            rgb_index++;
        }
    }
}

int ADAPS_DTOF::dtof_decode(unsigned char *frm_rawdata , u16 depth16_buffer[], enum sensor_workmode swk)
{
    WrapperDepthCamConfig config;
    int result=0;

    Q_UNUSED(swk);
    WrapperDepthOutput outputs[MAX_DEPTH_OUTPUT_FORMATS];
    WrapperDepthInput depthInput;

    if (false == m_conversionLibInited)
    {
        DBG_ERROR("ConversionLib Init Fail \n");
        return -1;
    }
    outputs[0].format                    = WRAPPER_CAM_FORMAT_DEPTH16;
    outputs[0].out_image_length          = m_sns_param.out_frm_width*m_sns_param.out_frm_height*sizeof(u16);
    outputs[0].formatParams.bitsPerPixel = 16;
    outputs[0].formatParams.strideBytes  = m_sns_param.out_frm_width;
    outputs[0].formatParams.sliceHeight  = m_sns_param.out_frm_height;                    
    outputs[0].out_depth_image = (uint8_t*) depth16_buffer;
#if 0
    outputs[1].format                    = WRAPPER_CAM_FORMAT_DEPTH16;
    outputs[1].out_image_length          = m_sns_param.out_frm_width*m_sns_param.out_frm_height*sizeof(u16);
    outputs[1].formatParams.bitsPerPixel = 16;
    outputs[1].formatParams.strideBytes  = m_sns_param.out_frm_width*2;
    outputs[1].formatParams.sliceHeight  = m_sns_param.out_frm_height;                    
    outputs[1].out_depth_image = m_out_put_pointcloud_buffer;
#endif

    depthInput.in_image    = (const int8_t*)frm_rawdata;
    depthInput.formatParams.bitsPerPixel =8;
    depthInput.formatParams.strideBytes  = m_sns_param.raw_width;
    depthInput.formatParams.sliceHeight  = m_sns_param.raw_height;
    //DBG_INFO( "raw_width: %d raw_height: %d out_width: %d out_height: %d\n", m_sns_param.raw_width, m_sns_param.raw_height, m_sns_param.out_frm_width, m_sns_param.out_frm_height);

    //depthInput.formatParams.planeSize    = pProcessRequestInfo->phInputBuffer[0]->planeSize[0];

    uint32_t iResult = 0;
    //for (uint32_t index = 0; index < pProcessRequestInfo->phInputBuffer[0]->imageCount; index++) 
    {
        PrepareFrameParam(&config);

        //BOOL disableAlgo = CamX::OsUtils::GetPropertyBool("debug.adaps.disableAlgo", false);
        bool disableAlgo =false;

        if (false == disableAlgo) 
        {
            iResult += m_processFrame(m_DepthMapWrapper,
                                  depthInput,
                                  &config,
                                  1, //2,
                                  outputs);

            //DBG_INFO( "depthInput: image , pAddr=%p, iResult: %d \n", depthInput.in_image, iResult);
        }else
        {
            static  uint32_t count_g=0;
            count_g++;
            if(count_g%4 == 0)
                iResult = 1;
            else
                iResult = 0;
               
           DBG_INFO( " bypass the algo result=%d \n",iResult);
        }
    }

    result = ((1 == iResult) || (4 == iResult)) ? 0 : -1;
    return result;
}

