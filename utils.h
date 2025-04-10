#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <stdint.h>
#include <QDir>
#include <QFile>
#include <QByteArray>
#include <iostream>
#include "common.h"

#define POLYNOMIAL 0xEDB88320
#define REPLAY_DATA_FILE_PATH                   "/root"
#define REPLAY_DATA_FILE_EXT_NAME               ".raw_depth"

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
    uint32_t crc32(uint32_t initial, const unsigned char *buf, size_t len);
    unsigned char CRC8Calculate(const unsigned char buffer[], int len);
    QByteArray loadNextFileToBuffer();
    bool is_replay_data_exist();
    void test_pattern_generate(unsigned char *write_buf, int len, int ptn_idx);
    void hexdump(const unsigned char * buf, int buf_len, const char * title);
    void nv12_2_rgb(unsigned char *nv12, unsigned char *rgb, int width, int height);
    void yuyv_2_rgb(unsigned char *yuyv, unsigned char *rgb, int width, int height);
    bool IsASCII(const char c);
    void GetRgb4watchPoint(const u8 rgb_buffer[], const int outImgWidth, const int outImgHeight, u8 x, u8 y, u8 *r, u8 *g, u8 *b);
    void GetPidTid(const char *callfunc, const int callline);
    int MD5Check4Buffer(const unsigned char* buffer, int size, const char *expected_md5_string, const char *call_func, int call_line);

    static bool is_env_var_true(const char *var_name)
    {
        const char *env_var_value = getenv(var_name);
    
        if (env_var_value != NULL && strcmp(env_var_value, "true") == 0) {
            return true;
        }
        return false;
    }

    static int get_env_var_intvalue(const char *var_name)
    {
        int ret = 0;
        const char *env_var_value = getenv(var_name);
    
        if (env_var_value != NULL) {
            ret = atoi(env_var_value);
        }
    
        return ret;
    }

    static char * get_env_var_stringvalue(const char *var_name)
    {
        return getenv(var_name);
    }

private:
    QStringList fileList;
    QString replay_file_path;
    int currentIndex;
    uint32_t m_crc32_table[256];
    void generate_crc32_table();
    void loadFiles(const QString &directoryPath, const QString &fileExtension);
    unsigned char hexCharToValue(char c);
    void hexStringToByteArray(const char* hexString, unsigned char* byteArray, int byteArrayLength);
};

#endif // UTILS_H
