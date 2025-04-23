#ifndef MISC_DEVICE_H
#define MISC_DEVICE_H

#if defined(RUN_ON_EMBEDDED_LINUX)

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
//#include <QDebug>
#include <QDateTime>

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
    Misc_Device();
    ~Misc_Device();

    int read_dtof_runtime_status_param(float *temperature);
    void* get_dtof_calib_eeprom_param(void);
    void* get_dtof_exposure_param(void);
    void* get_dtof_runtime_status_param(void);
    int write_device_register(register_op_data_t *reg);
    int read_device_register(register_op_data_t *reg);
    int read_dtof_exposure_param(void);
    int write_dtof_initial_param(struct adaps_dtof_intial_param *param);
    int get_dtof_module_static_data(void **pp_module_static_data, void **pp_calib_data_buffer, uint32_t *calib_data_size);
    int parse_script_from_buffer(UINT8 workMode, const uint32_t script_buf_size, const uint8_t* script_buf, const uint32_t blkwrite_reg_count, const uint8_t* blkwrite_reg_data);

private:

    void* mmap_buffer_base;
    u32 mmap_buffer_max_size;
    sem_t misc_semLock;

    char        devnode_4_misc[DEV_NODE_LEN];
    int         fd_4_misc;
    swift_eeprom_data_t *p_eeprominfo;
    struct adaps_dtof_module_static_data module_static_data;
    struct adaps_dtof_runtime_status_param last_runtime_status_param;
    struct adaps_dtof_exposure_param exposureParam;
    int eeprom_data_size;
    u8* mapped_eeprom_calib_data_buffer;
    u8* mapped_script_sensor_settings;
    uint16_t sensor_reg_setting_cnt;
    uint16_t vcsel_reg_setting_cnt;
#if (ADS6401_MODULE_SPOT == SWIFT_MODULE_TYPE)
    u8* mapped_script_vcsel_opn7020_settings;
#else
    u8* mapped_script_vcsel_PhotonIC5015_settings;
#endif
    u8* mapped_roi_sram_data[ZONE_COUNT_PER_SRAM_GROUP * CALIB_SRAM_GROUP_COUNT];
    uint16_t roi_sram_size[ZONE_COUNT_PER_SRAM_GROUP * CALIB_SRAM_GROUP_COUNT];

    int read_dtof_module_static_data(void);
    int check_crc32_4_dtof_calib_eeprom_param(void);
    bool save_dtof_calib_eeprom_param(void *buf, int len);
    bool check_crc_4_eeprom_item(uint8_t *pEEPROMData, uint32_t offset, uint32_t length, uint8_t savedCRC, const char *tag);
    int get_next_line(const uint8_t *buffer, size_t buffer_len, size_t *pos, char *line, size_t line_len);
    int LoadItemsFromBuffer(const uint32_t ulBufferLen, const uint8_t* pucBuffer, ScriptItem* items, uint32_t* number);
    int parse_items(const uint32_t ulItemsCount, const ScriptItem *pstrItems, const uint32_t blkwrite_reg_count, const uint8_t* blkwrite_reg_data);
    int write_external_config_script(external_config_script_param_t *param);
    int copy_roisram_data(const u8 roi_reg_addr, const u8 roi_reg_size, const uint32_t blkwrite_reg_count, const uint8_t* blkwrite_reg_data);

};

#endif // RUN_ON_EMBEDDED_LINUX

#endif // MISC_DEVICE_H


