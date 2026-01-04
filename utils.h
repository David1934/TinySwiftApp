#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <stdint.h>
#include <iostream>

#include <linux/reboot.h>
#include <sys/reboot.h>
#include <unistd.h>  // 引入 getuid() 函数的声明


#include "common.h"

#define POLYNOMIAL 0xEDB88320

enum test_pattern
{
    ETP_00_TO_FF = 1,
    ETP_FF_TO_00,
    ETP_FULL_00,
    ETP_FULL_55,
    ETP_FULL_AA,
    ETP_FULL_FF,
    ETP_CNT
};
    
class Utils
{

public:
    Utils();
    ~Utils();
    void hexdump(const unsigned char * buf, int buf_len, const char * title);
    bool save_binary_file(const char *filename, const void *buffer, size_t size, const char *call_func, unsigned int call_line);
    bool save_frame(unsigned int frm_sequence, void *frm_buf, int buf_size, int frm_w, int frm_h, enum frame_data_type);
    void save_depth_txt_file(void *frm_buf,unsigned int frm_sequence,int frm_len, int w, int h);
    bool save_dtof_eeprom_calib_data_2_file(void *buf, int len);
    bool buffer_is_fully_same(const unsigned char *buffer, int len, unsigned char val);
    int check_dir_exist_and_writable(const char *dir_path);
    int check_file_exist(const char *file_path);
    int check_regular_file_exist(const char *file_path);

    static bool is_env_var_true(const char *var_name)
    {
        const char *env_var_value = getenv(var_name);
    
        if (env_var_value != NULL_POINTER && strcmp(env_var_value, "true") == 0) {
            return true;
        }
        return false;
    }

    static int get_env_var_intvalue(const char *var_name)
    {
        int ret = 0;
        const char *env_var_value = getenv(var_name);
    
        if (env_var_value != NULL_POINTER) {
            ret = atoi(env_var_value);
        }
    
        return ret;
    }

    static char * get_env_var_stringvalue(const char *var_name)
    {
        return getenv(var_name);
    }

private:
    bool IsASCII(const unsigned char c);
    std::string getCurrentDateTime();

};

#endif // UTILS_H
