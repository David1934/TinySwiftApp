#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <pthread.h>
#include <cinttypes>

#include "utils.h"

Utils::Utils()
{
}

Utils::~Utils()
{
}

bool Utils::buffer_is_fully_same(const unsigned char *buffer, int len, unsigned char val)
{
    int i;

    for (i = 0; i < len ; i++)
    {
        if (buffer[i] != val)
            return false;
    }

    return true;
}

bool Utils::save_binary_file(const char *filename, const void *buffer, size_t size, const char *call_func, unsigned int call_line)
{
    FILE *fp = fopen(filename, "wb");

    if (fp == NULL_POINTER) {
        DBG_ERROR("Fail to create file %s , errno: %s (%d) call from <%s> Line: %d...", 
            filename, strerror(errno), errno, call_func, call_line);
        return false;
    }

    fwrite(buffer, size, 1, fp);
    fclose(fp);

    return true;
}

void Utils::save_depth_txt_file(void *frm_buf,unsigned int frm_sequence,int frm_len, int w, int h)
{
    const int DEPTH_MASK = 0x3FFF; // For depth16 format, the low 14 bits is distance
    std::string currentTime = getCurrentDateTime();
    char *          LocalTimeStr = (char *) currentTime.c_str();

    char path[50]={0};
    int16_t *p_temp=(int16_t*)frm_buf;

    sprintf(path,"%sframe%03d_%s_%d_depth%s",DATA_SAVE_PATH,frm_sequence, LocalTimeStr, frm_len, ".txt");
    FILE*fp = NULL_POINTER;
    fp=fopen(path, "w+");
    if (fp == NULL_POINTER)
    {
        DBG_ERROR("fopen output file %s failed!\n",  path);
        return;
    }
    for (int i = 0; i < h; i++)
    {
        int offset = i * w;
        for (int j = 0; j < w; j++)
        {
            fprintf(fp, "%6u ", (*(p_temp + offset + j)) & DEPTH_MASK);  //do not printf high confidence bits
        }
        fprintf(fp, "\n");
    }
    fflush(fp);
    fclose(fp);
    DBG_INFO("Save depth file %s success!\n",  path);

}

bool Utils::save_dtof_eeprom_calib_data_2_file(void *buf, int len)
{
    std::string currentTime = getCurrentDateTime();
    char *          LocalTimeStr = (char *) currentTime.c_str();
    char *          filename = new char[50];

    sprintf(filename, "%s%s.eeprom",DATA_SAVE_PATH,LocalTimeStr);
    Utils *utils = new Utils();
    utils->save_binary_file(filename, buf, 
        len,
        __FUNCTION__,
        __LINE__
        );
    delete utils;
    delete[] filename;

    return true;
}

bool Utils::save_frame(unsigned int frm_sequence, void *frm_buf, int buf_size, int frm_w, int frm_h, enum frame_data_type ftype)
{
    const char extName[FDATA_TYPE_COUNT][24]   = {
                                    ".raw_grayscale",
                                    ".raw_depth",
                                    ".decoded_grayscale",
                                    ".decoded_depth16",
                                    ".decoded_point_cloud"
                                };

    std::string currentTime = getCurrentDateTime();
    char *          LocalTimeStr = (char *) currentTime.c_str();
    char *          filename = new char[128];

    sprintf(filename, "%sframe%03d_%dx%d_%s_%d%s", DATA_SAVE_PATH, frm_sequence, frm_w, frm_h,LocalTimeStr, buf_size, extName[ftype]);
    save_binary_file(filename, frm_buf, 
        buf_size,
        __FUNCTION__,
        __LINE__
        );
    delete[] filename;

    return true;
}

bool Utils::IsASCII(const unsigned char c)
{
    bool ret = false;

    if (c >= 32 && c < 128)
        ret = true;

    return ret;
}

void Utils::hexdump(const unsigned char * buf, int buf_len, const char * title)
{
    int             i, j, k;
    const int       NoPerLine = 16;
    char            line_buf[100];
    int             line_len;

    if (NULL_POINTER == buf) {
        DBG_ERROR("Null pointer");
        return;
    }

    if (NULL_POINTER != title) {
        DBG_PRINTK("-------%s--------\n", title);
    }

    for (i = 0; i <= buf_len / NoPerLine; i++) {
        memset(line_buf, 0, sizeof(line_buf));
        line_len = 0;

        for (j = 0; j < NoPerLine; j++) {
            k = i * NoPerLine + j;

            if (k >= buf_len)
                break;

            line_len += sprintf(line_buf + line_len, "%02x,", buf[k]);
        }

        line_len += sprintf(line_buf + line_len, "%s", "    ");

        for (j = 0; j < NoPerLine; j++) {
            k = i * NoPerLine + j;

            if (k >= buf_len)
                break;

            if (IsASCII(buf[k]))
            {
                line_len += sprintf(line_buf + line_len, "%c", buf[k]);
            }
            else {
                line_len += sprintf(line_buf + line_len, "%c", '.');
            }
        }

        DBG_PRINTK("[%4d] %s\n", i * NoPerLine, line_buf);
    }
}

std::string Utils::getCurrentDateTime() {
    time_t now;
    struct tm timeinfo;
    char buffer[80];
    
    time(&now);
    localtime_r(&now, &timeinfo);
    
    strftime(buffer, sizeof(buffer), "%Y%m%d%H%M%S", &timeinfo);
    return std::string(buffer);
}

