#include "v4l2.h"

#define CLEAR(x) memset(&(x), 0, sizeof(x))

V4L2::V4L2(struct sensor_params params)
{
    init();
    memcpy(&tof_param, &params, sizeof(struct sensor_params));

    if (NULL != sensordata[params.work_mode].sensor_subdev)
    {
        memcpy(sensor_sd_name, sensordata[params.work_mode].sensor_subdev, DEV_NODE_LEN);
    }
    else {
        memset(sensor_sd_name, 0, DEV_NODE_LEN);
    }
    memcpy(media_dev, sensordata[params.work_mode].media_devnode, DEV_NODE_LEN);
    memcpy(video_dev, sensordata[params.work_mode].video_devnode, DEV_NODE_LEN);
    tof_param.raw_width = sensordata[params.work_mode].raw_w;
    tof_param.raw_height = sensordata[params.work_mode].raw_h;
    pixel_format = sensordata[params.work_mode].pixfmt;
    frame_buffer_count = sensordata[params.work_mode].frm_buf_cnt;
    tof_param.sensor_type = sensordata[params.work_mode].stype;
    frm_type = sensordata[params.work_mode].ftype;
    tof_param.out_frm_width = sensordata[params.work_mode].out_frm_width;
    tof_param.out_frm_height = sensordata[params.work_mode].out_frm_height;

#if defined(DEBUG)
    DBG_INFO("sensor_sd_name: %s", sensor_sd_name);
    DBG_INFO("media_dev: %s", media_dev);
    DBG_INFO("video_dev: %s", video_dev);
    DBG_INFO("preset width: %d", tof_param.raw_width);
    DBG_INFO("preset height: %d", tof_param.raw_height);
    DBG_INFO("preset pixel_format: 0x%x", pixel_format);
    DBG_INFO("preset frame_buffer_count: %d", frame_buffer_count);
    DBG_INFO("preset sensor_type: %d", tof_param.sensor_type);
#endif
}


V4L2::~V4L2()
{
    free(sensordata);
}

int V4L2::init()
{
    sensordata = (struct sensor_data *)malloc(sizeof(struct sensor_data)*WK_COUNT);

    sensordata[WK_DTOF_PHR] = {
        ENTITY_NAME_4_DTOF_SENSOR,
        MEDIA_DEVNAME_4_DTOF_SENSOR,
        VIDEO_DEV_4_DTOF_SENSOR,
        1032,
        32,
        210,
        160,
        PIXELFORMAT_4_DTOF_SENSOR,
        BUFFER_COUNT_4_DTOF_SENSOR,
        SENSOR_TYPE_DTOF,
        FDATA_TYPE_DTOF_DEPTH
    };

    sensordata[WK_DTOF_PCM] = {
        ENTITY_NAME_4_DTOF_SENSOR,
        MEDIA_DEVNAME_4_DTOF_SENSOR,
        VIDEO_DEV_4_DTOF_SENSOR,
        2560,
        32,
        210,
        160,
        PIXELFORMAT_4_DTOF_SENSOR,
        BUFFER_COUNT_4_DTOF_SENSOR,
        SENSOR_TYPE_DTOF,
        FDATA_TYPE_DTOF_GRAYSCALE
    };

    sensordata[WK_DTOF_FHR] = {
        ENTITY_NAME_4_DTOF_SENSOR,
        MEDIA_DEVNAME_4_DTOF_SENSOR,
        VIDEO_DEV_4_DTOF_SENSOR,
        4104,
        32,
        210,
        160,
        PIXELFORMAT_4_DTOF_SENSOR,
        BUFFER_COUNT_4_DTOF_SENSOR,
        SENSOR_TYPE_DTOF,
        FDATA_TYPE_DTOF_DEPTH
    };

    sensordata[WK_RGB_NV12] = {
        NULL,
        MEDIA_DEVNAME_4_RGB_SENSOR,
        VIDEO_DEV_4_RGB_RK35XX,
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
        return ret;
    }

#if defined(DEBUG)
    DBG_INFO("searching sensor %s...",sensor_sd_name);
    DBG_INFO("enum entities...");
    DBG_INFO("major:minor   id      entity_name       device node");
    DBG_INFO("-------------------------------------------------------");
#endif
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
#if defined(DEBUG)
        DBG_INFO("   %2d:%2d      %2d    %16s   %s", entity_desc.v4l.major, entity_desc.v4l.minor, id,entity_desc.name,cur_devnode);
#endif
        if(strcmp(sensor_sd_name, entity_desc.name) == 0)
        {
            strcpy(sd_devnode_4_dtof, cur_devnode);
#if defined(DEBUG)
            DBG_INFO("sd_devnode_4_dtof: %s", sd_devnode_4_dtof);
#endif
            ret = 0;
            break;
        }
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
#if defined(DEBUG)
        DBG_INFO("buffer[%d].start: %p, length: %d", i, buffers[i].start, buffers[i].length);
#endif

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

    for (i = 0; i < req_bufs.count; ++i) {
        if (-1 == munmap(buffers[i].start, buffers[i].length)) {
            DBG_ERROR("Fail to munmap, errno: %s (%d)...", 
                strerror(errno), errno);
            return;
        }
    }

    free(buffers);
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

void* V4L2::adaps_getEEPROMData(void)
{
    return p_eeprominfo;
}

int V4L2::adaps_readEEPROMData(void)
{
    int ret = 0;
    if (SENSOR_TYPE_DTOF == tof_param.sensor_type)
    {
        p_eeprominfo = (struct adaps_get_eeprom *)malloc(sizeof(struct adaps_get_eeprom));
        if (-1 == ioctl(fd_4_dtof, ADAPS_GET_EEPROM, p_eeprominfo)) {
            DBG_ERROR("Fail to read eeprom of dtof sub device, errno: %s (%d)...", 
                   strerror(errno), errno);
            ret = -1;
            free(p_eeprominfo);
            p_eeprominfo = NULL;
        }else
        {     
            DBG_INFO("adaps_get_eeprom  adaps_get_eeprom.pRawData=%x  sizeof swift_eeprom_data_t=%ld in user space  \n   ",
                    p_eeprominfo->pRawData,
                    sizeof(swift_eeprom_data_t));
            //save_eeprom(p_eeprominfo->pRawData, sizeof(swift_eeprom_data_t));
        }
    }

    return ret;
}

int V4L2::adaps_setParam4DtofSubdev(void)
{
    int ret = 0;
    struct adaps_set_param_in_config set_inconfig;
    set_inconfig.env_type = tof_param.env_type;
    set_inconfig.measure_type = tof_param.measure_type;
    set_inconfig.framerate_type = AdapsFramerateType30FPS;
    set_inconfig.vcselzonecount_type = AdapsVcselZoneCount4;

    if (SENSOR_TYPE_DTOF == tof_param.sensor_type)
    {
        if (-1 == ioctl(fd_4_dtof, ADAPS_SET_PARAM_IN_CONFIG, &set_inconfig)) {
            DBG_ERROR("Fail to set param for dtof sub device, errno: %s (%d)...", 
                   strerror(errno), errno);
            ret = -1;
        }else
        {     
            DBG_INFO("adaps_set_param_in_config set_inconfig.env_type=%d   set_inconfig.measure_type=%d  set_inconfig.framerate_type=%d  \n   ",
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

    if (SENSOR_TYPE_DTOF == tof_param.sensor_type)
    {
        if (-1 == ioctl(fd_4_dtof, ADAPS_GET_PARAM_PERFRAME, &perframe)) {
            DBG_ERROR("Fail to read per frame param from dtof sub device, errno: %s (%d)...", 
                   strerror(errno), errno);
            ret = -1;
        }else
        {     
            float temp1 = perframe.internal_temperature%100;
            float temp2 = perframe.internal_temperature/100;
            
            *temperature = temp2+temp1/100;
        }
    }

    return ret;
}

bool V4L2::Initilize(void)
{
    int ret = 0;
    struct v4l2_capability	cap;
    struct v4l2_format      fmt;
    struct v4l2_frmsizeenum frmsize;
    struct v4l2_fmtdesc fmtdesc;

    if (SENSOR_TYPE_DTOF == tof_param.sensor_type)
    {
        ret = get_subdev_node_4_sensor();
        if (ret < 0)
        {
            DBG_ERROR("Fail to get subdev node for dtof sensor...");
            return false;
        }

        if ((fd_4_dtof = open(sd_devnode_4_dtof, O_RDWR)) == -1)
        {
            DBG_ERROR("Fail to open device %s , errno: %s (%d)...", 
                sd_devnode_4_dtof, strerror(errno), errno);
            return false;
        }
    }

    if ((fd = open(video_dev, O_RDWR)) == -1)
    {
        DBG_ERROR("Fail to open device %s , errno: %s (%d)...", 
            video_dev, strerror(errno), errno);
        return false;
    }

    if (ioctl(fd, VIDIOC_QUERYCAP, &cap) == -1)
    {
        DBG_ERROR("Fail to query device capabilities, errno: %s (%d)...", 
            strerror(errno), errno);
        return false;
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
        return false;
    }

    if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
        DBG_ERROR("this device seems not support streaming, capabilities: 0x%x...",
            cap.capabilities);
        return false;
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

    DBG_INFO("VIDIOC_S_FMT %d X %d, pixel_format: 0x%x, raw_width:%d, raw_height:%d...\n",
    tof_param.raw_width, tof_param.raw_height, pixel_format, tof_param.raw_width, tof_param.raw_height);
    CLEAR(fmt);
    fmt.type            = buf_type;
    fmt.fmt.pix.pixelformat = pixel_format;
    fmt.fmt.pix.width  = tof_param.raw_width;
    fmt.fmt.pix.height   = tof_param.raw_height;
    //fmt.fmt.pix.field   = V4L2_FIELD_INTERLACED;
    //fmt.fmt.pix.quantization = V4L2_QUANTIZATION_FULL_RANGE;

    if (ioctl(fd, VIDIOC_S_FMT, &fmt) == -1) {
        DBG_ERROR("Fail to set format, errno: %s (%d)...", 
            strerror(errno), errno);
        return false;
    }

    if (ioctl(fd, VIDIOC_G_FMT, &fmt) == -1) //重新读取 结构体，以确认完成设置
    {
        DBG_ERROR("Fail to get format , errno: %s (%d)...", 
            strerror(errno), errno);
        return false;
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
        return false;
    }

    if (ioctl(fd, VIDIOC_G_PARM, &setfps) == -1) //重新读取结构体，以确认完成设置
    {
        qDebug() << "Unable to get fps";
        return false;
    }
    else {
        qDebug() << "fps:\t" << setfps.parm.capture.timeperframe.denominator / setfps.parm.capture.timeperframe.numerator;
    }
#endif

    if (false == alloc_buffers())
    {
        return false;
    }

    if (SENSOR_TYPE_DTOF == tof_param.sensor_type)
    {
        if (-1 == adaps_readEEPROMData())
        {
            return false;
        }
        
        if (-1 == adaps_setParam4DtofSubdev())
        {
            return false;
        }
    }

#if defined(DEBUG)
    DBG_INFO("init dev %s [OK]", video_dev);
#endif
    return true;
}

bool V4L2::Start_streaming(void)
{
    if (-1 == ioctl(fd, VIDIOC_STREAMON, &buf_type))
    {
        DBG_ERROR("Fail to stream_on, errno: %s (%d)...", 
            strerror(errno), errno);
        return false;
    }

    return true;
}

bool V4L2::Capture_frame()
{
    struct v4l2_buffer  v4l2_buf;
    struct v4l2_plane v4l2_planes[FMT_NUM_PLANES];
    int bytesused;

    CLEAR(v4l2_buf);

    v4l2_buf.type = buf_type;
    v4l2_buf.memory = V4L2_MEMORY_MMAP;

    CLEAR(v4l2_planes);
    if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == buf_type) {
        v4l2_buf.m.planes = v4l2_planes;
        v4l2_buf.length = FMT_NUM_PLANES;
    }

    if (ioctl(fd, VIDIOC_DQBUF, &v4l2_buf) == -1) {
        DBG_ERROR("Fail to dequeue buffer, errno: %s (%d)...", 
            strerror(errno), errno);
        return false;
    }

    if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == buf_type)
        bytesused = v4l2_buf.m.planes[0].bytesused;
    else
        bytesused = v4l2_buf.bytesused;

#if 0 //defined(DEBUG)
    long timestamp_ms = 1000 * v4l2_buf.timestamp.tv_sec + v4l2_buf.timestamp.tv_usec / 1000;
    DBG_INFO("after VIDIOC_DQBUF--buff index=%d  -bytesused=%d, buffers[v4l2_buf.index].length:%d---timestamp=%ld-",
        v4l2_buf.index,bytesused, buffers[v4l2_buf.index].length, timestamp_ms);
#endif

    emit new_frame_process(v4l2_buf.sequence, buffers[v4l2_buf.index].start, bytesused, v4l2_buf.timestamp, frm_type);

#if 0 //defined(DEBUG)
    DBG_INFO("before VIDIOC_QBUF--buff index=%d  buf_type=0x%x, V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE:0x%x-",
        v4l2_buf.index, buf_type, V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE);
#endif
    if (-1 == ioctl(fd, VIDIOC_QBUF, &v4l2_buf)) {
        DBG_ERROR("Fail to queue buffer, errno: %s (%d)...", 
            strerror(errno), errno);
        return false;
    }

    return true;
}

void V4L2::Stop_streaming(void)
{
    if (-1 == ioctl(fd, VIDIOC_STREAMOFF, &buf_type)) {
        DBG_ERROR("Fail to stream off, errno: %s (%d)...", 
            strerror(errno), errno);
    }
    return;
}

void V4L2::Close(void)
{
    if (SENSOR_TYPE_DTOF == tof_param.sensor_type)
    {
        free(p_eeprominfo);
        p_eeprominfo = NULL;
        if (-1 == close(fd_4_dtof)) {
            DBG_ERROR("Fail to close device, errno: %s (%d)...", 
                strerror(errno), errno);
            return;
        }
    }

    free_buffers();
    if (-1 == close(fd)) {
        DBG_ERROR("Fail to close device, errno: %s (%d)...", 
            strerror(errno), errno);
        return;
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

void V4L2::Get_output_frame_size(int *in_width, int *in_height, int *out_width, int *out_height)
{
    *in_width = tof_param.raw_width;
    *in_height = tof_param.raw_height;
    *out_width = tof_param.out_frm_width;
    *out_height = tof_param.out_frm_height;
}

