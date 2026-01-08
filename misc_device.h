#ifndef MISC_DEVICE_H
#define MISC_DEVICE_H


#include <fcntl.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/mman.h>
#include <linux/videodev2.h>
#include <linux/media.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <semaphore.h>

#include "common.h"

#define DEV_NODE_LEN                            32

#define HEX_NUMBER                              16
#define DEC_NUMBER                              10
#define ADS6401_I2C_ADDR_IN_SCRIPT              0x5E
#define ADS6401_I2C_ADDR2_IN_SCRIPT             0x4A
#define VCSEL_OPN7020_I2C_ADDR_IN_SCRIPT        0x31

#define MCUCTRL_I2C_ADDR_4_FLOOD_IN_SCRIPT      0x60

#define SCRIPT_LINE_LENGTH                      256
#define ADS6401_REG_ADDR_4_WORK_START           0xAB

typedef enum {
    I2C_Write,
    I2C_Read,
    MIPI_Read,
    Swift_Delay,
    Comment,
    Block_Write,
    Block_Read,
    Slave_Addr
} ScriptType;

typedef struct {
    ScriptType     type;
    int            i2c_addr; // or sleep time in ms for delay command
    int            reg_addr;
    int            reg_val;
    char           note[200];
} ScriptItem;



class Misc_Device
{

public:
    // 删除拷贝构造和赋值操作（防止复制单例）
    Misc_Device(const Misc_Device&) = delete;
    Misc_Device& operator=(const Misc_Device&) = delete;
    ~Misc_Device();

    // 获取单例实例
    static Misc_Device* getInstance();
    u32 get_module_type();
    bool is_roi_sram_rolling();
    int set_roi_sram_rolling(bool val);
    UINT8 get_module_kernel_type();
    int set_module_kernel_type(UINT8 value);
    int get_anchorOffset(UINT8 *rowOffset, UINT8 *colOffset);

    int read_dtof_runtime_status_param(struct adaps_dtof_runtime_status_param **status_param);
    int get_dtof_inside_temperature(float *temperature);
    void* get_dtof_calib_eeprom_param(void);
    void* get_dtof_exposure_param(void);
    void* get_dtof_runtime_status_param(void);
    int read_dtof_exposure_param(void);
    int write_dtof_initial_param(struct adaps_dtof_intial_param *param);
    int get_dtof_module_static_data(void **pp_module_static_data, void **pp_eeprom_data_buffer, uint32_t *eeprom_data_size);

private:
    static Misc_Device* instance; // 静态成员变量声明
    Misc_Device(); // 私有构造函数（防止外部实例化）
    static void destroyInstance();

    u32 module_type = 0;
    bool roi_sram_rolling = false;
    UINT8 module_kernel_type = 0;
    UINT8 anchor_rowOffset = 0;
    UINT8 anchor_colOffset = 0;

    void* mmap_buffer_base;
    u32 mmap_buffer_max_size;
    sem_t misc_semLock;

    char        devnode_4_misc[DEV_NODE_LEN];
    int         fd_4_misc;
    swift_spot_module_eeprom_data_t *p_spot_module_eeprom;
    swift_flood_module_eeprom_data_t *p_flood_module_eeprom;
    swift_eeprom_v2_data_t *p_bigfov_module_eeprom;
    struct adaps_dtof_module_static_data module_static_data;
    struct adaps_dtof_runtime_status_param last_runtime_status_param;
    struct adaps_dtof_exposure_param exposureParam;
    u8* mapped_eeprom_data_buffer;
    u8* mapped_script_sensor_settings;
//    uint16_t sensor_reg_setting_cnt;
//    uint16_t vcsel_reg_setting_cnt;
    u8* mapped_script_vcsel_settings;           // opn7020 for spot module, PhotonIC5015 for flood module
    u8* mapped_roi_sram_data;

    int read_dtof_module_static_data(void);
    int dump_eeprom_data(u8* pEEPROM_Data);

};


#endif // MISC_DEVICE_H


