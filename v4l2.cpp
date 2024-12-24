#include <sys/sysmacros.h>
#include <linux/v4l2-subdev.h>

#include "v4l2.h"
#include "utils.h"

#define CLEAR(x) memset(&(x), 0, sizeof(x))

V4L2::V4L2(struct sensor_params params)
{
    p_eeprominfo = NULL;
    buffers = NULL;
    sensordata = NULL;
    fd_4_dtof = 0;
    fd = 0;
    firstFrameTimeUsec = 0;
    rxFrameCnt = 0;
    fps = 0;
    streamed_timeUs = 0;
    last_temperature = 0;
    init();
    memcpy(&snr_param, &params, sizeof(struct sensor_params));

    if (NULL != sensordata[params.work_mode].sensor_subdev)
    {
        memcpy(sensor_sd_name, sensordata[params.work_mode].sensor_subdev, DEV_NODE_LEN);
    }
    else {
        memset(sensor_sd_name, 0, DEV_NODE_LEN);
    }
    memcpy(media_dev, sensordata[params.work_mode].media_devnode, DEV_NODE_LEN);
    memcpy(video_dev, sensordata[params.work_mode].video_devnode, DEV_NODE_LEN);
    snr_param.raw_width = sensordata[params.work_mode].raw_w;
    snr_param.raw_height = sensordata[params.work_mode].raw_h;
    pixel_format = sensordata[params.work_mode].pixfmt;
    frame_buffer_count = sensordata[params.work_mode].frm_buf_cnt;
    snr_param.sensor_type = sensordata[params.work_mode].stype;
    frm_type = sensordata[params.work_mode].ftype;
    snr_param.out_frm_width = sensordata[params.work_mode].out_frm_width;
    snr_param.out_frm_height = sensordata[params.work_mode].out_frm_height;

    DBG_INFO("sensor_sd_name: %s", sensor_sd_name);
    DBG_INFO("media_dev: %s", media_dev);
    DBG_INFO("video_dev: %s", video_dev);
    DBG_INFO("preset width: %d", snr_param.raw_width);
    DBG_INFO("preset height: %d", snr_param.raw_height);
    DBG_INFO("preset pixel_format: 0x%x", pixel_format);
    DBG_INFO("preset frame_buffer_count: %d", frame_buffer_count);
    DBG_INFO("preset sensor_type: %d", snr_param.sensor_type);
}


V4L2::~V4L2()
{
    if (NULL != sensordata)
    {
        free(sensordata);
        sensordata = NULL;
    }
}

void V4L2::mode_switch(struct sensor_params params)
{
    memcpy(&snr_param, &params, sizeof(struct sensor_params));

    if (NULL != sensordata[params.work_mode].sensor_subdev)
    {
        memcpy(sensor_sd_name, sensordata[params.work_mode].sensor_subdev, DEV_NODE_LEN);
    }
    else {
        memset(sensor_sd_name, 0, DEV_NODE_LEN);
    }
    memcpy(media_dev, sensordata[params.work_mode].media_devnode, DEV_NODE_LEN);
    memcpy(video_dev, sensordata[params.work_mode].video_devnode, DEV_NODE_LEN);
    snr_param.raw_width = sensordata[params.work_mode].raw_w;
    snr_param.raw_height = sensordata[params.work_mode].raw_h;
    pixel_format = sensordata[params.work_mode].pixfmt;
    frame_buffer_count = sensordata[params.work_mode].frm_buf_cnt;
    snr_param.sensor_type = sensordata[params.work_mode].stype;
    frm_type = sensordata[params.work_mode].ftype;
    snr_param.out_frm_width = sensordata[params.work_mode].out_frm_width;
    snr_param.out_frm_height = sensordata[params.work_mode].out_frm_height;
}

int V4L2::init()
{
    if (NULL == sensordata)
    {
        sensordata = (struct sensor_data *)malloc(sizeof(struct sensor_data)*WK_COUNT);
    }

    sensordata[WK_DTOF_PHR] = {
        ENTITY_NAME_4_DTOF_SENSOR,
        MEDIA_DEVNAME_4_DTOF_SENSOR,
        VIDEO_DEV_4_DTOF_SENSOR,
        1032,
        MIPI_RAW_HEIGHT_4_DTOF_SENSOR,
        OUTPUT_WIDTH_4_DTOF_SENSOR,
        OUTPUT_HEIGHT_4_DTOF_SENSOR,
        PIXELFORMAT_4_DTOF_SENSOR,
        BUFFER_COUNT_4_DTOF_SENSOR,
        SENSOR_TYPE_DTOF,
        FDATA_TYPE_DTOF_RAW_DEPTH
    };

    sensordata[WK_DTOF_PCM] = {
        ENTITY_NAME_4_DTOF_SENSOR,
        MEDIA_DEVNAME_4_DTOF_SENSOR,
        VIDEO_DEV_4_DTOF_SENSOR,
        2560,
        MIPI_RAW_HEIGHT_4_DTOF_SENSOR,
        OUTPUT_WIDTH_4_DTOF_SENSOR,
        OUTPUT_HEIGHT_4_DTOF_SENSOR,
        PIXELFORMAT_4_DTOF_SENSOR,
        BUFFER_COUNT_4_DTOF_SENSOR,
        SENSOR_TYPE_DTOF,
        FDATA_TYPE_DTOF_RAW_GRAYSCALE
    };

    sensordata[WK_DTOF_FHR] = {
        ENTITY_NAME_4_DTOF_SENSOR,
        MEDIA_DEVNAME_4_DTOF_SENSOR,
        VIDEO_DEV_4_DTOF_SENSOR,
        4104,
        MIPI_RAW_HEIGHT_4_DTOF_SENSOR,
        OUTPUT_WIDTH_4_DTOF_SENSOR,
        OUTPUT_HEIGHT_4_DTOF_SENSOR,
        PIXELFORMAT_4_DTOF_SENSOR,
        BUFFER_COUNT_4_DTOF_SENSOR,
        SENSOR_TYPE_DTOF,
        FDATA_TYPE_DTOF_RAW_DEPTH
    };

    sensordata[WK_RGB_NV12] = {
        NULL,
        MEDIA_DEVNAME_4_RGB_SENSOR,
        VIDEO_DEV_4_RGB_RK3588,
        640,
        480,
        640,
        480,
        V4L2_PIX_FMT_NV12,
        BUFFER_COUNT_4_RGB_SENSOR,
        SENSOR_TYPE_RGB,
        FDATA_TYPE_NV12
    };

    sensordata[WK_RGB_YUYV] = {
        NULL,
        MEDIA_DEVNAME_4_RGB_SENSOR,
        VIDEO_DEV_4_RGB_SENSOR,
        640,
        480,
        640,
        480,
        V4L2_PIX_FMT_YUYV,
        BUFFER_COUNT_4_RGB_SENSOR,
        SENSOR_TYPE_RGB,
        FDATA_TYPE_YUYV
    };

    return 0;
}

int V4L2::get_devnode_from_sysfs(struct media_entity_desc *entity_desc, char *p_devname)
{
    struct stat devstat;
    char devname[32];
    char sysname[32];
    char target[1024];
    char *p;
    int ret;

    sprintf(sysname, "/sys/dev/char/%u:%u", entity_desc->v4l.major,
        entity_desc->v4l.minor);
    ret = readlink(sysname, target, sizeof(target) - 1);
    if (ret < 0)
        return -errno;

    target[ret] = '\0';
    p = strrchr(target, '/');
    if (p == NULL)
        return -EINVAL;

    sprintf(devname, "/dev/%s", p + 1);
    ret = stat(devname, &devstat);
    if (ret < 0)
        return -errno;

    /* Sanity check: udev might have reordered the device nodes.
     * Make sure the major/minor match. We should really use
     * libudev.
     */
    if (major(devstat.st_rdev) == entity_desc->v4l.major &&
        minor(devstat.st_rdev) == entity_desc->v4l.minor)
        strcpy(p_devname, devname);

    return 0;
}

int V4L2::get_subdev_node_4_sensor()
{
    struct media_entity_desc entity_desc;
    uint32_t id = 0;
    int ret = -EINVAL;
    char cur_devnode[DEV_NODE_LEN];
    int fp;

    fp = open(media_dev, O_RDWR);
    if (fp < 0)
    {
        DBG_ERROR("Fail to open media device %s, errno: %s (%d)...", 
            media_dev, strerror(errno), errno);
        return 0 - __LINE__;
    }

    DBG_INFO("searching sensor %s...",sensor_sd_name);
    DBG_INFO("enum entities...");
    DBG_INFO("major:minor   id      entity_name       device node");
    DBG_INFO("-------------------------------------------------------");

    memset(&entity_desc, 0, sizeof(entity_desc));
    while(1)
    {
        entity_desc.id = id|MEDIA_ENT_ID_FLAG_NEXT;
        ret = ioctl(fp, MEDIA_IOC_ENUM_ENTITIES, &entity_desc);
        if(ret)
        {
            break;
        }

        id = entity_desc.id;
        get_devnode_from_sysfs(&entity_desc,cur_devnode);
        DBG_INFO("   %2d:%2d      %2d    %16s   %s", entity_desc.v4l.major, entity_desc.v4l.minor, id,entity_desc.name,cur_devnode);
        if(strcmp(sensor_sd_name, entity_desc.name) == 0)
        {
            strcpy(sd_devnode_4_dtof, cur_devnode);
            DBG_INFO("sd_devnode_4_dtof: %s", sd_devnode_4_dtof);
            ret = 0;
            break;
        }
    }

    return ret;
}

int V4L2::Set_param_4_sensor_sub_device(int raw_w_4_curr_wkmode, int raw_h_4_curr_wkmode)
{
    int ret = 0;
    struct v4l2_subdev_format sensorFmt;

    memset(&sensorFmt, 0, sizeof(sensorFmt));
    sensorFmt.pad           = 0;
    sensorFmt.which         = V4L2_SUBDEV_FORMAT_ACTIVE;
    sensorFmt.format.width  = raw_w_4_curr_wkmode;
    sensorFmt.format.height = raw_h_4_curr_wkmode;
    DBG_INFO("--VIDIOC_SUBDEV_S_FMT--fd_4_dtof:%d, sd_devnode_4_dtof:%s, width:%d, height:%d", fd_4_dtof, sd_devnode_4_dtof, raw_w_4_curr_wkmode, raw_h_4_curr_wkmode);

    ret = xioctl(fd_4_dtof, VIDIOC_SUBDEV_S_FMT, &sensorFmt);
    if (-1 == ret) {
        DBG_ERROR("Fail to set format for dtof sub device, errno: %s (%d)...", 
               strerror(errno), errno);
    }

    return ret;
}

bool V4L2::alloc_buffers(void)
{
    unsigned int i;

    req_bufs.count           = frame_buffer_count;
    req_bufs.type            = buf_type;
    req_bufs.memory          = V4L2_MEMORY_MMAP;

    if (ioctl(fd, VIDIOC_REQBUFS, &req_bufs) == -1) {
        DBG_ERROR("Fail to request buffer , buf_type:%d errno: %s (%d)...", 
            buf_type, strerror(errno), errno);
        return false;
    }

    buffers             = (buffer *) malloc(req_bufs.count * sizeof(struct buffer));

    if (!buffers) {
        DBG_ERROR("Out of memory");
        return false;
    }

    for (i = 0; i < req_bufs.count; i++)
    {
        struct v4l2_buffer	v4l2_buf;
        struct v4l2_plane v4l2_planes[FMT_NUM_PLANES];
        CLEAR(v4l2_planes);
        
        v4l2_buf.type = buf_type;
        v4l2_buf.memory = V4L2_MEMORY_MMAP;
        v4l2_buf.index          = i;
        
        if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == buf_type) {
            v4l2_buf.m.planes = v4l2_planes;
            v4l2_buf.length = FMT_NUM_PLANES;
        }

        if (-1 == ioctl (fd, VIDIOC_QUERYBUF, &v4l2_buf)) {
            DBG_ERROR("Fail to query buffer, errno: %s (%d)...", 
                strerror(errno), errno);
            return false;
        }
        
        if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == buf_type) {
            buffers[i].length = v4l2_buf.m.planes[0].length;
            buffers[i].start = mmap(NULL /* start anywhere */,
                     v4l2_buf.m.planes[0].length,
                     PROT_READ | PROT_WRITE /* required */,
                     MAP_SHARED /* recommended */,
                     fd, v4l2_buf.m.planes[0].m.mem_offset);
        } else {
            buffers[i].length = v4l2_buf.length;
            buffers[i].start = mmap (NULL, v4l2_buf.length,
                                      PROT_READ | PROT_WRITE,
                                      MAP_SHARED,
                                      fd, v4l2_buf.m.offset);
        }

        if (MAP_FAILED == buffers[i].start) {
            DBG_ERROR("Fail to mmap, errno: %s (%d)...", 
                strerror(errno), errno);
            return false;
        }
        DBG_INFO("buffer[%d].start: %p, length: %d", i, buffers[i].start, buffers[i].length);

        v4l2_buf.type = buf_type;
        v4l2_buf.memory = V4L2_MEMORY_MMAP;
        v4l2_buf.index = i;
        
        if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == buf_type) {
            v4l2_buf.m.planes = v4l2_planes;
            v4l2_buf.length = FMT_NUM_PLANES;
        }
        if (-1 == ioctl(fd, VIDIOC_QBUF, &v4l2_buf)) {
            DBG_ERROR("Fail to queue buffer, errno: %s (%d)...", 
                strerror(errno), errno);
            return false;
        }

    }

    return true;
}

void V4L2::free_buffers(void)
{
    unsigned int    i;

    if (NULL != buffers)
    {
        for (i = 0; i < req_bufs.count; ++i) {
            if (-1 == munmap(buffers[i].start, buffers[i].length)) {
                DBG_ERROR("Fail to munmap, errno: %s (%d)...",
                    strerror(errno), errno);
                return;
            }
        }

        free(buffers);
        buffers = NULL;
    }
}

bool V4L2::save_eeprom(void *buf, int len)
{
    QDateTime       localTime = QDateTime::currentDateTime();
    QString         currentTime = localTime.toString("yyyyMMddhhmmss");
    char *          LocalTimeStr = (char *) currentTime.toStdString().c_str();
    char *          filename = new char[50];
    sprintf(filename,"%s%s.eeprom",DATA_SAVE_PATH,LocalTimeStr);
    FILE *fp = fopen(filename, "wb");

    if (fp == NULL) {
        DBG_ERROR("Fail to create file %s , errno: %s (%d)...", 
            filename, strerror(errno), errno);
        return false;
    }

    fwrite(buf, len, 1, fp);
    delete[] filename;
    fclose(fp);

    return true;
}

#if defined(RUN_ON_ROCKCHIP)
void* V4L2::adaps_getExposureParam(void)
{
    return &snr_param.exposureParam;
}

int V4L2::adaps_readExposureParam(void)
{
    int ret = 0;
    struct adaps_get_exposure_param param;
    memset(&param, 0, sizeof(param));

    if (SENSOR_TYPE_DTOF == snr_param.sensor_type)
    {
        if (-1 == ioctl(fd_4_dtof, ADAPS_GET_EXPOSURE_PARAM, &param)) {
            DBG_ERROR("Fail to get exposure param from dtof sub device, errno: %s (%d)...", 
                   strerror(errno), errno);
            ret = -1;
        }else
        {     
            snr_param.exposureParam.laser_exposure_period = param.laser_exposure_period;
            snr_param.exposureParam.fine_exposure_time = param.fine_exposure_time;
        }
    }

    return ret;
}
#endif

void* V4L2::adaps_getEEPROMData(void)
{
    return p_eeprominfo;
}

#if defined(RUN_ON_ROCKCHIP)
#if (ADS6401_MODDULE_SPOT == SWIFT_MODULE_TYPE)
int V4L2::adaps_chkEEPROMChecksum(void)
{
    int ret = 0;

    uint32_t saved_crc32 = 0;
    swift_eeprom_data_t *p_swift_eeprom_data;
    p_swift_eeprom_data = (swift_eeprom_data_t *)p_eeprominfo->pRawData;
    if (Utils::is_env_var_true(ENV_VAR_SAVE_EEPROM_ENABLE))
    {
        save_eeprom(p_eeprominfo->pRawData, sizeof(swift_eeprom_data_t));
    }

    Utils *utils = new Utils();
    uint32_t calc_crc32 = utils->crc32(0, (const unsigned char *)p_eeprominfo->pRawData, AD4001_EEPROM_TOTAL_CHECKSUM_OFFSET - AD4001_EEPROM_VERSION_INFO_OFFSET);
    delete utils;
    saved_crc32 = p_swift_eeprom_data->totalChecksum;
    if (calc_crc32 == saved_crc32)
    {
        DBG_INFO("EEPROM crc32 matched!!! sizeof(swift_eeprom_data_t)=%ld, length before totalChecksum:%ld,calc_crc32:0x%x,saved_crc32:0x%x",
                sizeof(swift_eeprom_data_t),
                AD4001_EEPROM_TOTAL_CHECKSUM_OFFSET - AD4001_EEPROM_VERSION_INFO_OFFSET,
                calc_crc32,
                saved_crc32);
    }
    else {
        DBG_ERROR("EEPROM crc32 mismatched!!! eeprominfo->pRawData: %p, sizeof(swift_eeprom_data_t)=%ld, length before totalChecksum:%ld,calc_crc32:0x%x,saved_crc32:0x%x",
                p_eeprominfo->pRawData,
                sizeof(swift_eeprom_data_t),
                AD4001_EEPROM_TOTAL_CHECKSUM_OFFSET - AD4001_EEPROM_VERSION_INFO_OFFSET,
                calc_crc32,
                saved_crc32);
        ret = -1;
    }

    return ret;
}

#else

int V4L2::adaps_chkEEPROMChecksum(void)
{
    int i = 0,ret=0;
    uint32_t read_crc32 = 0;
    uint32_t calc_crc32 = 0;
    swift_eeprom_data_t *p_swift_eeprom_data;
    p_swift_eeprom_data = (swift_eeprom_data_t *)p_eeprominfo->pRawData;
    if (Utils::is_env_var_true(ENV_VAR_SAVE_EEPROM_ENABLE))
    {
        save_eeprom(p_eeprominfo->pRawData, sizeof(swift_eeprom_data_t));
    }

    Utils *utils = new Utils();

    //do page 0 crc
    calc_crc32 = utils->crc32(0, (const unsigned char *) p_eeprominfo->pRawData, AD4001_EEPROM_VERSION_INFO_SIZE + AD4001_EEPROM_SN_INFO_SIZE);
    if(p_swift_eeprom_data->Crc32Pg0 == calc_crc32)
    {
        DBG_INFO("EEPROM Crc32Pg0 matched!!! sizeof(swift_eeprom_data_t)=%ld, calc_crc32:0x%x",
                sizeof(swift_eeprom_data_t),
                calc_crc32);
        //hex_data_dump((const u8 *) sensor->eeprom_data, 64, "eeprom data head");
    }
    else {
        DBG_ERROR("EEPROM Crc32Pg0 MISMATCHED!!! AD4001_EEPROM_ROISRAM_DATA_OFFSET=%ld, calc_crc32:0x%x,read Crc32Pg0:0x%x",
                AD4001_EEPROM_ROISRAM_DATA_OFFSET,
                calc_crc32,
                p_swift_eeprom_data->Crc32Pg0);
        //hex_data_dump((const u8 *) sensor->eeprom_data, 64, "eeprom data head");
        ret = -1;
        //goto check_exit;
    }

    //do page 1 crc
    calc_crc32 = utils->crc32(0, (const unsigned char *) p_eeprominfo->pRawData + AD4001_EEPROM_MODULE_INFO_OFFSET, AD4001_EEPROM_MODULE_INFO_SIZE);

    if(p_swift_eeprom_data->Crc32Pg1 == calc_crc32)
    {
        DBG_INFO("EEPROM Crc32Pg1 matched!!! calc_crc32:0x%x",
                calc_crc32);
        //hex_data_dump((const u8 *) sensor->eeprom_data, 64, "eeprom data head");
    }
    else {
        DBG_ERROR("EEPROM Crc32Pg1 MISMATCHED!!! calc_crc32:0x%x,read Crc32Pg1:0x%x",
                calc_crc32,
                p_swift_eeprom_data->Crc32Pg1);
        ret = -1;
        //goto check_exit;
    }

    //do page 2 crc
    calc_crc32 = utils->crc32(0, (const unsigned char *) p_eeprominfo->pRawData + AD4001_EEPROM_TDCDELAY_OFFSET, AD4001_EEPROM_TDCDELAY_SIZE+AD4001_EEPROM_INTRINSIC_SIZE+AD4001_EEPROM_INDOOR_CALIBTEMPERATURE_SIZE+AD4001_EEPROM_OUTDOOR_CALIBTEMPERATURE_SIZE+AD4001_EEPROM_INDOOR_CALIBREFDISTANCE_SIZE+AD4001_EEPROM_OUTDOOR_CALIBREFDISTANCE_SIZE);
    if(p_swift_eeprom_data->Crc32Pg2 == calc_crc32)
    {
        DBG_INFO("EEPROM Crc32Pg2 matched!!! calc_crc32:0x%x",
                calc_crc32);
        //hex_data_dump((const u8 *) sensor->eeprom_data, 64, "eeprom data head");
    }
    else {
        DBG_ERROR("EEPROM Crc32Pg2 MISMATCHED!!! calc_crc32:0x%x,read Crc32Pg2:0x%x",
                calc_crc32,
                p_swift_eeprom_data->Crc32Pg2);
        ret = -1;
        //goto check_exit;
    }

    //do sram data crc for every zone
    for ( i = 0; i < ZONE_COUNT_PER_SRAM_GROUP * CALIB_SRAM_GROUP_COUNT; ++i)
    {
        calc_crc32 = utils->crc32(0, (const unsigned char *) p_eeprominfo->pRawData + AD4001_EEPROM_ROISRAM_DATA_OFFSET + i*SRAM_ZONE_OCUPPY_SPACE, SRAM_ZONE_VALID_DATA_LENGTH);
        read_crc32 = *(uint32_t*)(p_eeprominfo->pRawData + AD4001_EEPROM_ROISRAM_DATA_OFFSET + i*SRAM_ZONE_OCUPPY_SPACE + SRAM_ZONE_VALID_DATA_LENGTH);
        if (calc_crc32 != read_crc32)
        {
            DBG_ERROR("SRAM %d CRC checksum MISMATCHED, calc_crc32=0x%x   read_crc32=0x%x\n", i, calc_crc32, read_crc32);
            ret = -1;
            //goto check_exit;
        }
        else
        {
            DBG_INFO("SRAM %d CRC checksum matched!!! sizeof(swift_eeprom_data_t)=%ld, calc_crc32:0x%x",
                i,
                sizeof(swift_eeprom_data_t),
                calc_crc32);
        }

        //reset the crc in the eeprom to 0,because the algo will use these data
        //*(uint32_t*)(p_eeprominfo->pRawData + i*SRAM_ZONE_OCUPPY_SPACE + SRAM_ZONE_VALID_DATA_LENGTH)=0;
    }

    //do offset crc 
    for ( i = 0; i < 8; ++i)
    {
        calc_crc32 = utils->crc32(0, (unsigned char*)(p_swift_eeprom_data->spotOffset)+i*OFFSET_VALID_DATA_LENGTH, OFFSET_VALID_DATA_LENGTH);
        if (calc_crc32 != p_swift_eeprom_data->Crc32Offset[i])
        {
            DBG_ERROR("offset %d CRC checksum MISMATCHED, read_crc=0x%x, calc_crc32:0x%x\n", i, p_swift_eeprom_data->Crc32Offset[i], calc_crc32);
            ret = -1;
            //goto check_exit;
        }
        else
        {
            DBG_INFO("offset %d CRC checksum matched!!! calc_crc32:0x%x", i, calc_crc32);
        }
    }

check_exit:
    delete utils;

    return ret;
}
#endif

int V4L2::adaps_readEEPROMData(void)
{
    int ret = 0;

    if (SENSOR_TYPE_DTOF == snr_param.sensor_type)
    {
        p_eeprominfo = (struct adaps_get_eeprom *)malloc(sizeof(struct adaps_get_eeprom));
        if (-1 == ioctl(fd_4_dtof, ADAPS_GET_EEPROM, p_eeprominfo)) {
            DBG_ERROR("Fail to read eeprom of dtof sub device(%d, %s), errno: %s (%d), ADAPS_GET_EEPROM:0x%lx, sizeof(struct adaps_get_eeprom):%ld, sizeof(swift_eeprom_data_t): %ld...", fd_4_dtof, sd_devnode_4_dtof,
                   strerror(errno), errno, ADAPS_GET_EEPROM, sizeof(struct adaps_get_eeprom), sizeof(swift_eeprom_data_t));
            ret = -1;
            if (NULL != p_eeprominfo)
            {
                free(p_eeprominfo);
                p_eeprominfo = NULL;
            }
        }else
        {
            if (false == Utils::is_env_var_true(ENV_VAR_SKIP_EEPROM_CRC_CHK))
            {
                ret = adaps_chkEEPROMChecksum();
                ret = 0; // skip eeprom crc mismatch now, since there are some modules whose crc is mismatched.
            }
        }
    }

    return ret;
}

int V4L2::adaps_setParam4DtofSubdev(void)
{
    int ret = 0;
    struct adaps_set_param_in_config set_inconfig;
    set_inconfig.env_type = snr_param.env_type;
    set_inconfig.measure_type = snr_param.measure_type;
    set_inconfig.framerate_type = DEFAULT_DTOF_FRAMERATE;
    set_inconfig.vcselzonecount_type = AdapsVcselZoneCount4;

    if (SENSOR_TYPE_DTOF == snr_param.sensor_type)
    {
        if (-1 == ioctl(fd_4_dtof, ADAPS_SET_PARAM_IN_CONFIG, &set_inconfig)) {
            DBG_ERROR("Fail to set param for dtof sub device, errno: %s (%d)...", 
                   strerror(errno), errno);
            ret = -1;
        }else
        {     
            DBG_INFO("adaps_set_param_in_config set_inconfig.env_type=%d   set_inconfig.measure_type=%d  set_inconfig.framerate_type=%d     ",
                set_inconfig.env_type,
                set_inconfig.measure_type,
                set_inconfig.framerate_type);
        }
    }

    return ret;
}

int V4L2::adaps_readTemperatureOfDtofSubdev(float *temperature)
{
    int ret = 0;
    struct adaps_get_param_perframe perframe;
    memset(&perframe,0,sizeof(perframe));

    if (SENSOR_TYPE_DTOF == snr_param.sensor_type)
    {
        if (-1 == ioctl(fd_4_dtof, ADAPS_GET_PARAM_PERFRAME, &perframe)) {
            DBG_ERROR("Fail to read per frame param from dtof sub device, errno: %s (%d)...", 
                   strerror(errno), errno);
            ret = -1;
        }else
        {     
            last_temperature = perframe.internal_temperature;
            last_expected_vop_abs_x100 = perframe.expected_vop_abs_x100;
            last_expected_pvdd_x100 = perframe.expected_pvdd_x100;

            *temperature = (float) ((double)perframe.internal_temperature /(double)100.0f);
            DBG_INFO("internal_temperature: %d, temperature: %f\n", perframe.internal_temperature, *temperature);
        }
    }

    return ret;
}
#endif

int V4L2::V4l2_initilize(void)
{
    int ret = 0;
    struct v4l2_capability	cap;
    struct v4l2_format      fmt;
//    struct v4l2_frmsizeenum frmsize;
//    struct v4l2_fmtdesc fmtdesc;

    if (SENSOR_TYPE_DTOF == snr_param.sensor_type)
    {
        ret = get_subdev_node_4_sensor();
        if (ret < 0)
        {
            DBG_ERROR("Fail to get subdev node for dtof sensor...");
            return 0 - __LINE__;
        }

        if ((fd_4_dtof = open(sd_devnode_4_dtof, O_RDWR)) == -1)
        {
            DBG_ERROR("Fail to open device %s , errno: %s (%d)...", 
                sd_devnode_4_dtof, strerror(errno), errno);
            return 0 - __LINE__;
        }
    }

    if ((fd = open(video_dev, O_RDWR)) == -1)
    {
        DBG_ERROR("Fail to open device %s , errno: %s (%d)...", 
            video_dev, strerror(errno), errno);
        fd = 0;
        return 0 - __LINE__;
    }

    if (ioctl(fd, VIDIOC_QUERYCAP, &cap) == -1)
    {
        DBG_ERROR("Fail to query device capabilities, errno: %s (%d)...", 
            strerror(errno), errno);
        return 0 - __LINE__;
    }
    else
    {
        DBG_INFO("driver:\t\t%s", cap.driver);
        DBG_INFO("card:\t\t%s", cap.card);
        DBG_INFO("bus_info:\t%s", cap.bus_info);
        DBG_INFO("version:\t0x%x", cap.version);
        DBG_INFO("capabilities:\t0x%x", cap.capabilities);
        DBG_INFO("device_caps:\t0x%x", cap.capabilities);
    }

    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) &&
            !(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE)) {
        DBG_ERROR("this device seems not support video capture, capabilities: 0x%x...",
            cap.capabilities);
        return 0 - __LINE__;
    }

    if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
        DBG_ERROR("this device seems not support streaming, capabilities: 0x%x...",
            cap.capabilities);
        return 0 - __LINE__;
    }

    if (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) {
        buf_type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        DBG_INFO("Buffer type:\tV4L2_BUF_TYPE_VIDEO_CAPTURE");
    } else if (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE) {
        buf_type                = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
        DBG_INFO("Buffer type:\tV4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE");
    }

#if 0
    fmtdesc.index       = 0;
    fmtdesc.type        = buf_type;
    DBG_INFO("Support formats:");
    while (ioctl(fd, VIDIOC_ENUM_FMT, &fmtdesc) != -1) {
        DBG_INFO("\t%d\t%s\t\tfourcc:0x%x(%c%c%c%c)",
            fmtdesc.index + 1, 
            fmtdesc.description, 
            fmtdesc.pixelformat,
            fmtdesc.pixelformat & 0xff,
            fmtdesc.pixelformat >> 8 & 0xff,
            fmtdesc.pixelformat >> 16 & 0xff,
            fmtdesc.pixelformat >> 24 & 0xff
            );
        frmsize.pixel_format = fmtdesc.pixelformat;
        frmsize.index       = 0;
        
        while (ioctl(fd, VIDIOC_ENUM_FRAMESIZES, &frmsize) >= 0) {
            if (frmsize.type == V4L2_FRMSIZE_TYPE_DISCRETE) {
                DBG_INFO("\t\tDISCRETE resolution[%d]: %d X %d",frmsize.index, frmsize.discrete.width, frmsize.discrete.height);
            }
            else if (frmsize.type == V4L2_FRMSIZE_TYPE_STEPWISE) {
                DBG_INFO("\t\tSTEPWISE resolution[%d]: %d X %d",frmsize.index, frmsize.stepwise.max_width, frmsize.stepwise.max_height);
            }
        
            frmsize.index++;
        }
        fmtdesc.index++;
    }
#endif

    // when config swift work mode, need TDC delay min/max need to know which environment, which measurement is used, so I move this the following lines before set sensor fmt;
#if defined(RUN_ON_ROCKCHIP)
    if (SENSOR_TYPE_DTOF == snr_param.sensor_type)
    {
        if (0 > adaps_readEEPROMData())
        {
#if !defined(IGNORE_REAL_COMMUNICATION)
            return 0 - __LINE__;
#endif
        }

        if (0 > adaps_setParam4DtofSubdev())
        {
            return 0 - __LINE__;
        }
    }

#if !defined(VIDIOC_S_FMT_INCLUDE_VIDIOC_SUBDEV_S_FMT)
    if (0 != fd_4_dtof)
    {
        ret = Set_param_4_sensor_sub_device(snr_param.raw_width, snr_param.raw_height);
        if (0 > ret) {
            DBG_ERROR("Fail to Set_param_4_sensor_sub_device, errno: %s (%d)...", 
                   strerror(errno), errno);
            return ret;
        }
    }
#endif
#endif

    DBG_INFO("VIDIOC_S_FMT %d X %d, pixel_format: 0x%x, raw_width:%d, raw_height:%d...\n",
    snr_param.raw_width, snr_param.raw_height, pixel_format, snr_param.raw_width, snr_param.raw_height);
    CLEAR(fmt);
    fmt.type            = buf_type;
    fmt.fmt.pix.pixelformat = pixel_format;
    fmt.fmt.pix.width  = snr_param.raw_width;
    fmt.fmt.pix.height   = snr_param.raw_height;
    //fmt.fmt.pix.field   = V4L2_FIELD_INTERLACED;
    //fmt.fmt.pix.quantization = V4L2_QUANTIZATION_FULL_RANGE;

    if (ioctl(fd, VIDIOC_S_FMT, &fmt) == -1) {
        DBG_ERROR("Fail to set format for dev: %s (%d), errno: %s (%d)...", video_dev, fd,
            strerror(errno), errno);
        return 0 - __LINE__;
    }

    if (ioctl(fd, VIDIOC_G_FMT, &fmt) == -1) //重新读取 结构体，以确认完成设置
    {
        DBG_ERROR("Fail to get format , errno: %s (%d)...", 
            strerror(errno), errno);
        return 0 - __LINE__;
    }
    else {
        DBG_INFO("fmt.type:\t%d", fmt.type);
        DBG_INFO("pix.width:\t%d", fmt.fmt.pix.width);
        DBG_INFO("pix.height:\t%d", fmt.fmt.pix.height);
        DBG_INFO("pix.field:\t%d", fmt.fmt.pix.field);
    }

#if 0
    setfps.type         = buf_type; // 设置摄像头的帧率，这里一般设置为 30fps
    setfps.parm.capture.timeperframe.denominator = 30; //fps=30/1=30
    setfps.parm.capture.timeperframe.numerator = 1;

    if (ioctl(fd, VIDIOC_S_PARM, &setfps) == -1) {
        qDebug("Unable to set fps on <%s> line:%d, errno: %s (%d)...", __FUNCTION__, __LINE__, strerror(errno), errno);
        return 0 - errno;
    }

    if (ioctl(fd, VIDIOC_G_PARM, &setfps) == -1) //重新读取结构体，以确认完成设置
    {
        qDebug() << "Unable to get fps";
        return 0 - errno;
    }
    else {
        qDebug() << "fps:\t" << setfps.parm.capture.timeperframe.denominator / setfps.parm.capture.timeperframe.numerator;
    }
#endif

#if defined(RUN_ON_ROCKCHIP)
    if (SENSOR_TYPE_DTOF == snr_param.sensor_type)
    {
        if (0 > adaps_readExposureParam())
        {
            return 0 - __LINE__;
        }
    }
#endif

    if (false == alloc_buffers())
    {
        return 0 - __LINE__;
    }

    DBG_INFO("init dev %s [OK]", video_dev);

    return 0;
}

int V4L2::Start_streaming(void)
{
    firstFrameTimeUsec = 0;
    rxFrameCnt = 0;
    fps = 0;
    streamed_timeUs = 0;
    if (-1 == ioctl(fd, VIDIOC_STREAMON, &buf_type))
    {
        DBG_ERROR("Fail to stream_on, errno: %s (%d)...", 
            strerror(errno), errno);
        return 0 - __LINE__;
    }
    DBG_INFO("Start to streaming for dev %s", video_dev);

    return 0;
}

int V4L2::Capture_frame()
{
    struct v4l2_buffer  v4l2_buf;
    struct v4l2_plane v4l2_planes[FMT_NUM_PLANES];
    int bytesused;
    struct timeval tv;
    long currTimeUsec;
    status_params1 param1;

    CLEAR(v4l2_buf);

    v4l2_buf.type = buf_type;
    v4l2_buf.memory = V4L2_MEMORY_MMAP;

    CLEAR(v4l2_planes);
    if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == buf_type) {
        v4l2_buf.m.planes = v4l2_planes;
        v4l2_buf.length = FMT_NUM_PLANES;
    }

    if (0 == fd || ioctl(fd, VIDIOC_DQBUF, &v4l2_buf) == -1) {
        DBG_ERROR("Fail to dequeue buffer, fd: %d errno: %s (%d)...", fd,
            strerror(errno), errno);
        return 0 - __LINE__;
    }

    if (v4l2_buf.flags & V4L2_BUF_FLAG_ERROR) {
        u64 rxTimeUsec;
        rxTimeUsec = v4l2_buf.timestamp.tv_sec*1000000 + v4l2_buf.timestamp.tv_usec;

        DBG_ERROR("Error (flags:0x%x) in dequeue frame#%d timestampUs: %lld buffer...", 
            v4l2_buf.flags, v4l2_buf.sequence, rxTimeUsec);
        goto error_exit;
    }

    if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == buf_type)
        bytesused = v4l2_buf.m.planes[0].bytesused;
    else
        bytesused = v4l2_buf.bytesused;

    gettimeofday(&tv,NULL);
    rxFrameCnt++;
    DBG_INFO("Rx %ld frames for dev %s", rxFrameCnt, video_dev);
    if (0 == firstFrameTimeUsec)
    {
        firstFrameTimeUsec = tv.tv_sec*1000000 + tv.tv_usec;

        switch (pixel_format) {
            case V4L2_PIX_FMT_YUYV:
                bits_per_pixel = 12;
                break;

            case V4L2_PIX_FMT_NV12:
                bits_per_pixel = 12;
            break;

            case V4L2_PIX_FMT_SBGGR8:
            default:
                bits_per_pixel = 8;
                break;
        }
        total_bytes_per_line = bytesused/snr_param.raw_height;
        payload_bytes_per_line = (snr_param.raw_width * bits_per_pixel)/8;
        padding_bytes_per_line = total_bytes_per_line - payload_bytes_per_line;
        DBG_NOTICE("\n------frame raw size: %d X %d, bits_per_pixel: %d, payload_bytes_per_line: %d, total_bytes_per_line: %d, padding_bytes_per_line: %d---\n",
            snr_param.raw_width, snr_param.raw_height,
            bits_per_pixel, payload_bytes_per_line, total_bytes_per_line, padding_bytes_per_line);
    }
    else {
        currTimeUsec = tv.tv_sec*1000000 + tv.tv_usec;
        streamed_timeUs = (currTimeUsec - firstFrameTimeUsec);
        fps = (rxFrameCnt * 1000000) / streamed_timeUs;
    }

    param1.fps = fps;
    param1.streamed_time_us = streamed_timeUs;
#if defined(RUN_ON_ROCKCHIP)
    param1.curr_temperature = last_temperature;
    param1.curr_exp_vop_abs = last_expected_vop_abs_x100;
    param1.curr_exp_pvdd = last_expected_pvdd_x100;
#endif
    emit update_info(param1);

    emit new_frame_process(v4l2_buf.sequence, buffers[v4l2_buf.index].start, bytesused, v4l2_buf.timestamp, frm_type, total_bytes_per_line);

error_exit:
    if (0 == fd || -1 == ioctl(fd, VIDIOC_QBUF, &v4l2_buf)) {
        DBG_ERROR("Fail to queue buffer, errno: %s (%d)...", 
            strerror(errno), errno);
        return 0 - __LINE__;
    }

    return 0;
}

void V4L2::Stop_streaming(void)
{
    int streamed_second = streamed_timeUs/1000000;

    if (rxFrameCnt <= 0) // not started yet.
        return;

    DBG_NOTICE("\n------Test Statistics------streamed: %02d:%02d:%02d, rxFrameCnt: %ld, fps: %d, frame raw size: %d X %d---\n",
        streamed_second/3600,streamed_second/60,streamed_second%60,
        rxFrameCnt, fps, 
        snr_param.raw_width, snr_param.raw_height);

    if (-1 == ioctl(fd, VIDIOC_STREAMOFF, &buf_type)) {
        DBG_ERROR("Fail to stream off, errno: %s (%d)...", 
            strerror(errno), errno);
    }
    return;
}

void V4L2::Close(void)
{
    if (SENSOR_TYPE_DTOF == snr_param.sensor_type)
    {
        if (NULL != p_eeprominfo)
        {
            free(p_eeprominfo);
            p_eeprominfo = NULL;
        }
        if (-1 == close(fd_4_dtof)) {
            DBG_ERROR("Fail to close device %d (%s), errno: %s (%d)...", fd_4_dtof, sd_devnode_4_dtof,
                strerror(errno), errno);
            return;
        }
    }

    free_buffers();
    if (0 != fd)
    {
        if (-1 == close(fd)) {
            DBG_ERROR("Fail to close device, errno: %s (%d)...", 
                strerror(errno), errno);
            return;
        }
        fd = 0;
    }

    return;
}

void V4L2::nv12_2_rgb(unsigned char *nv12 , unsigned char *rgb, int width , int height)
{
    const int nv_start = width * height;
    int  i, j, index = 0, rgb_index = 0;
    unsigned char y, u, v;
    int r, g, b, nv_index = 0;

    for (i = 0; i < height; i++) {
        for (j = 0; j < width; j++){
            nv_index = i / 2 * width + j - j % 2;

            y = nv12[rgb_index];
            u = nv12[nv_start + nv_index];
            v = nv12[nv_start + nv_index + 1];

            r = y + (140 * (v - 128)) / 100;
            g = y - (34 * (u - 128)) / 100 - (71 * (v - 128)) / 100;
            b = y + (177 * (u - 128)) / 100;

            if (r > 255)   r = 255;
            if (g > 255)   g = 255;
            if (b > 255)   b = 255;
            if (r < 0)     r = 0;
            if (g < 0)     g = 0;
            if (b < 0)     b = 0;

            index = rgb_index ;
            rgb[index * 3 + 0] = r;
            rgb[index * 3 + 1] = g;
            rgb[index * 3 + 2] = b;
            rgb_index++;
        }
    }
    return;
}


void V4L2::yuyv_2_rgb(unsigned char *yuyv, unsigned char *rgb, int width, int height) {
    int i, j;
    int y0, u, y1, v;
    int r, g, b;

    for (i = 0, j = 0; i < (width * height) * 2; i+=4, j+=6) {
        y0 = yuyv[i];
        u = yuyv[i + 1] - 128;
        y1 = yuyv[i + 2];
        v = yuyv[i + 3] - 128;

        r = y0 + 1.370705 * v;
        g = y0 - 0.698001 * v - 0.337633 * u;
        b = y0 + 1.732446 * u;

        /* Ensure RGB values are within range */
        rgb[j] = (r > 255) ? 255 : ((r < 0) ? 0 : r);
        rgb[j + 1] = (g > 255) ? 255 : ((g < 0) ? 0 : g);
        rgb[j + 2] = (b > 255) ? 255 : ((b < 0) ? 0 : b);

        r = y1 + 1.370705 * v;
        g = y1 - 0.698001 * v - 0.337633 * u;
        b = y1 + 1.732446 * u;

        rgb[j + 3] = (r > 255) ? 255 : ((r < 0) ? 0 : r);
        rgb[j + 4] = (g > 255) ? 255 : ((g < 0) ? 0 : g);
        rgb[j + 5] = (b > 255) ? 255 : ((b < 0) ? 0 : b);
    }
}

void V4L2::Get_frame_size_4_curr_wkmode(int *in_width, int *in_height, int *out_width, int *out_height)
{
    *in_width = snr_param.raw_width;
    *in_height = snr_param.raw_height;
    *out_width = snr_param.out_frm_width;
    *out_height = snr_param.out_frm_height;
}

