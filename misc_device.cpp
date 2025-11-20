#include <sys/sysmacros.h>

#include "misc_device.h"
#include "utils.h"

#define CLEAR(x)                            memset(&(x), 0, sizeof(x))

#define REG_NULL                            0xFF
#define REG16_NULL                          0xFFFF

// 静态成员变量定义（必须！）
Misc_Device* Misc_Device::instance = NULL_POINTER;

Misc_Device* Misc_Device::getInstance() {
    if (!instance) {
        instance = new Misc_Device();
    }
    return instance;
}

Misc_Device::Misc_Device()
{
    p_spot_module_eeprom = NULL_POINTER;
    p_flood_module_eeprom = NULL_POINTER;
    p_bigfov_module_eeprom = NULL_POINTER;
    mmap_buffer_base = NULL_POINTER;
    mapped_eeprom_data_buffer = NULL_POINTER;
    mapped_script_sensor_settings = NULL_POINTER;
    mapped_script_vcsel_settings = NULL_POINTER;
//    sensor_reg_setting_cnt = 0;
//    vcsel_reg_setting_cnt = 0;
    // mmap_buffer_max_size should be a multiple of PAGE_SIZE (4096, for Linux kernel memory management)
    mmap_buffer_max_size = MMAP_BUFFER_MAX_SIZE_4_WHOLE_EEPROM_DATA
        + REG_SETTING_BUF_MAX_SIZE_PER_SEG
        + REG_SETTING_BUF_MAX_SIZE_PER_SEG
        + (PER_CALIB_SRAM_ZONE_SIZE * ZONE_COUNT_PER_SRAM_GROUP * MAX_CALIB_SRAM_ROTATION_GROUP_CNT);
    memset((void *) &last_runtime_status_param, 0, sizeof(struct adaps_dtof_runtime_status_param));
    fd_4_misc = 0;
    memset(devnode_4_misc, 0, DEV_NODE_LEN);
    sprintf(devnode_4_misc, "%s_misc", VIDEO_DEV_4_MISC_DEVICE);

    sem_init(&misc_semLock, 0 ,1);

    if ((fd_4_misc = open(devnode_4_misc, O_RDWR)) == -1)
    {
        DBG_ERROR("Fail to open device %s , errno: %s (%d)...", 
            devnode_4_misc, strerror(errno), errno);
        return;
    }

    if (0 != fd_4_misc)
    {
        mmap_buffer_base = mmap(NULL_POINTER, mmap_buffer_max_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd_4_misc, 0);
        if (mmap_buffer_base == MAP_FAILED) {
            DBG_ERROR("Failed to mmap buffer, mmap_buffer_max_size: %d...", mmap_buffer_max_size);
            return;
        }
        mapped_eeprom_data_buffer = (u8* ) mmap_buffer_base;
        mapped_script_sensor_settings = (u8*)(mapped_eeprom_data_buffer + MMAP_BUFFER_MAX_SIZE_4_WHOLE_EEPROM_DATA);
        mapped_script_vcsel_settings = (u8* ) (mapped_script_sensor_settings + ROI_SRAM_BUF_MAX_SIZE);
        mapped_roi_sram_data = mapped_script_vcsel_settings + REG_SETTING_BUF_MAX_SIZE_PER_SEG;
        //qApp->set_mmap_address_4_loaded_roisram(mapped_roi_sram_data);

        if (0 > read_dtof_module_static_data())
        {
            DBG_ERROR("Failed to read module static data");
            return;
        }

    }
}


Misc_Device::~Misc_Device()
{
    if (NULL_POINTER != mmap_buffer_base)
    {
        if (munmap(mmap_buffer_base, mmap_buffer_max_size)) {
            DBG_ERROR("Failed to unmap buffer");
        }
        mmap_buffer_base = NULL_POINTER;
    }

    if (NULL_POINTER != p_spot_module_eeprom)
    {
        p_spot_module_eeprom = NULL_POINTER;
    }
    if (NULL_POINTER != p_flood_module_eeprom)
    {
        p_flood_module_eeprom = NULL_POINTER;
    }
    if (NULL_POINTER != p_bigfov_module_eeprom)
    {
        p_bigfov_module_eeprom = NULL_POINTER;
    }

    if ((0 != fd_4_misc) && (-1 == close(fd_4_misc))) {
        DBG_ERROR("Fail to close device %d (%s), errno: %s (%d)...", fd_4_misc, devnode_4_misc,
            strerror(errno), errno);
        return;
    }
    sem_destroy(&misc_semLock);
}

void* Misc_Device::get_dtof_exposure_param(void)
{
    return &exposureParam;
}

int Misc_Device::read_dtof_exposure_param(void)
{
    int ret = 0;
    struct adaps_dtof_exposure_param param;
    memset(&param, 0, sizeof(param));

    if (-1 == misc_ioctl(fd_4_misc, ADAPS_GET_DTOF_EXPOSURE_PARAM, &param)) {
        DBG_ERROR("Fail to get exposure param from dtof sensor device, errno: %s (%d)...", 
               strerror(errno), errno);
        ret = -1;
    }
    else {     
        exposureParam.exposure_period = param.exposure_period;
        exposureParam.ptm_coarse_exposure_value = param.ptm_coarse_exposure_value;
        exposureParam.ptm_fine_exposure_value = param.ptm_fine_exposure_value;
        exposureParam.pcm_gray_exposure_value = param.pcm_gray_exposure_value;
        DBG_INFO("exposure_period: 0x%02x, ptm_coarse_exposure_value: 0x%02x, ptm_fine_exposure_value: 0x%02x, pcm_gray_exposure_value: 0x%02x",
            param.exposure_period, param.ptm_coarse_exposure_value, param.ptm_fine_exposure_value,param.pcm_gray_exposure_value);
    }

    return ret;
}

void* Misc_Device::get_dtof_calib_eeprom_param(void)
{
    if (false == module_static_data.ready)
    {
        if (0 > read_dtof_module_static_data())
        {
            DBG_ERROR("Failed to read module static data");
        }
    }

    if (ADS6401_MODULE_SPOT == module_static_data.module_type)
    {
        return p_spot_module_eeprom;
    }
    else if (ADS6401_MODULE_SMALL_FLOOD == module_static_data.module_type) {
        return p_flood_module_eeprom;
    }
    else {
        return p_bigfov_module_eeprom;
    }

}

int Misc_Device::read_dtof_module_static_data(void)
{
    int ret = 0;

    if (-1 == misc_ioctl(fd_4_misc, ADAPS_GET_DTOF_MODULE_STATIC_DATA, &module_static_data)) {
        DBG_ERROR("Fail to read module_static_data of dtof misc device(%d, %s), ioctl cmd: 0x%lx errno: %s (%d)...",
            fd_4_misc, devnode_4_misc, ADAPS_GET_DTOF_MODULE_STATIC_DATA, strerror(errno), errno);
        ret = -1;
    }
    else {
        DBG_NOTICE("module_type: 0x%x, ready: %d", module_static_data.module_type, module_static_data.ready);
        if (module_static_data.ready)
        {
            uint8_t *pEEPROMData;
            int eeprom_data_size;

            module_type = module_static_data.module_type;

            if (ADS6401_MODULE_SPOT == module_static_data.module_type)
            {
                p_spot_module_eeprom = (swift_spot_module_eeprom_data_t *) mapped_eeprom_data_buffer;
                // set default anchor Offset (in case no host_comm) for spot module, may be changed from PC SpadisApp.
                anchor_rowOffset = 0;
                anchor_colOffset = 1;

                pEEPROMData = (uint8_t *) p_spot_module_eeprom;
                eeprom_data_size = sizeof(swift_spot_module_eeprom_data_t);
            }
            else if (ADS6401_MODULE_SMALL_FLOOD == module_static_data.module_type) {
                p_flood_module_eeprom = (swift_flood_module_eeprom_data_t *) mapped_eeprom_data_buffer;
                // non-spot module does not need anchor preprocess
                anchor_rowOffset = 0;
                anchor_colOffset = 0;

                pEEPROMData = (uint8_t *) p_flood_module_eeprom;
                eeprom_data_size = sizeof(swift_flood_module_eeprom_data_t);
            }
            else {
                p_bigfov_module_eeprom = (swift_eeprom_v2_data_t *) mapped_eeprom_data_buffer;
                anchor_rowOffset = 0;
                anchor_colOffset = 0;

                pEEPROMData = (uint8_t *) p_bigfov_module_eeprom;
                eeprom_data_size = sizeof(swift_eeprom_v2_data_t);
            }

            if (Utils::is_env_var_true(ENV_VAR_SAVE_EEPROM_ENABLE))
            {
                Utils *utils = new Utils();
                utils->save_dtof_eeprom_calib_data_2_file(pEEPROMData, eeprom_data_size);
                delete utils;
            }

        }
    }

    return ret;
}

int Misc_Device::get_dtof_module_static_data(void **pp_module_static_data, void **pp_eeprom_data_buffer, uint32_t *eeprom_data_size)
{
    *pp_module_static_data = (void *) &module_static_data;
    *pp_eeprom_data_buffer = mapped_eeprom_data_buffer;
    if (ADS6401_MODULE_SPOT == module_static_data.module_type)
    {
        *eeprom_data_size = sizeof(swift_spot_module_eeprom_data_t);
    }
    else if (ADS6401_MODULE_SMALL_FLOOD == module_static_data.module_type) {
        *eeprom_data_size = sizeof(swift_flood_module_eeprom_data_t);
    }
    else {
        *eeprom_data_size = sizeof(swift_eeprom_v2_data_t);
    }

    return 0;
}

int Misc_Device::write_dtof_initial_param(struct adaps_dtof_intial_param *param)
{
    int ret = 0;

    if (-1 == misc_ioctl(fd_4_misc, ADAPS_SET_DTOF_INITIAL_PARAM, param)) {
        DBG_ERROR("Fail to set initial param for dtof sensor device, errno: %s (%d)...", 
               strerror(errno), errno);
        ret = -1;
    }
    else {
        DBG_INFO("dtof_intial_param env_type=%d measure_type=%d framerate_type=%d     ",
            param->env_type,
            param->measure_type,
            param->framerate_type);
    }

    return ret;
}

void* Misc_Device::get_dtof_runtime_status_param(void)
{
    return &last_runtime_status_param;
}

int Misc_Device::read_dtof_runtime_status_param(struct adaps_dtof_runtime_status_param **status_param)
{
    int ret = 0;
    struct adaps_dtof_runtime_status_param param;
    memset(&param,0,sizeof(param));

    if (-1 == misc_ioctl(fd_4_misc, ADAPS_GET_DTOF_RUNTIME_STATUS_PARAM, &param)) {
        DBG_ERROR("Fail to get runtime status param from dtof sensor device, errno: %s (%d)...", 
               strerror(errno), errno);
        ret = -1;
    }else
    {     
        last_runtime_status_param.inside_temperature_x100 = param.inside_temperature_x100;
        last_runtime_status_param.expected_vop_abs_x100 = param.expected_vop_abs_x100;
        last_runtime_status_param.expected_pvdd_x100 = param.expected_pvdd_x100;
        *status_param = &last_runtime_status_param;
    }

    return ret;
}

int Misc_Device::get_dtof_inside_temperature(float *temperature)
{
    int ret = 0;

    *temperature = (float) ((double)last_runtime_status_param.inside_temperature_x100 /(double)100.0f);

    return ret;
}

u32 Misc_Device::get_module_type()
{
    return module_type;
}

bool Misc_Device::is_roi_sram_rolling()
{
    return roi_sram_rolling;
}

int Misc_Device::set_roi_sram_rolling(bool val)
{
    int ret = 0;

    roi_sram_rolling = val;

    return ret;
}

UINT8 Misc_Device::get_module_kernel_type()
{
    return module_kernel_type;
}

int Misc_Device::set_module_kernel_type(UINT8 value)
{
    int ret = 0;

    module_kernel_type = value;

    return ret;
}

int Misc_Device::get_anchorOffset(UINT8 *rowOffset, UINT8 *colOffset)
{
    int ret = 0;

    *colOffset = anchor_colOffset;
    *rowOffset = anchor_rowOffset;

    return ret;
}


