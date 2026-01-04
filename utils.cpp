#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <cinttypes>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

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

/**
 * @brief Check if a directory exists and is writable
 * @param dir_path Path to the directory
 * @return 0: Exists and writable; -1: Does not exist; -2: Exists but not writable; -3: Not a directory; Other negative values: System call error
 */
int Utils::check_dir_exist_and_writable(const char *dir_path)
{
    if (dir_path == NULL) {
        DBG_ERROR("Error: Directory path can't be blank\n");
        return -4;
    }

    struct stat stat_buf;
    int ret = stat(dir_path, &stat_buf);

    // Check if directory exists
    if (ret == -1) {
        DBG_ERROR("Directory <%s> does not exist", dir_path);
        return -1;  // Directory does not exist
    }

    // Verify it's a directory (not a file)
    if (!S_ISDIR(stat_buf.st_mode)) {
        DBG_ERROR("Error: <%s> is not a directory\n", dir_path);
        return -3;
    }

    // Check write permission for current process
    ret = access(dir_path, W_OK);
    if (ret == -1) {
        DBG_ERROR("Directory <%s> is not writable", dir_path);
        return -2;  // Exists but no write permission
    }

    // All checks passed
    return 0;
}

/**
 * @brief Check if a file/path exists (does not distinguish file types)
 * @param file_path Path to the file
 * @return 0: Exists; -1: Does not exist; Other negative values: System call error
 */
int Utils::check_file_exist(const char *file_path)
{
    if (file_path == NULL) {
        DBG_ERROR("Error: File path can't be blank\n");
        return -2;
    }

    // Use F_OK to only check existence (no permission check)
    int ret = access(file_path, F_OK);
    if (ret == -1) {
        DBG_ERROR("File %s does not exist", file_path);
        return -1;
    }

    return 0;
}

/**
 * @brief Enhanced check: Verify if path is a regular file and exists
 * @param file_path Path to the file
 * @return 0: Regular file and exists; -1: Does not exist; -2: Not a regular file; Other negative values: System call error
 */
int Utils::check_regular_file_exist(const char *file_path)
{
    if (file_path == NULL) {
        DBG_ERROR("Error: File path is a null pointer\n");
        return -3;
    }

    struct stat stat_buf;
    int ret = stat(file_path, &stat_buf);
    if (ret == -1) {
        DBG_ERROR("stat file failed");
        return -1;  // Does not exist
    }

    // Check if it's a regular file (exclude directories, symlinks, devices, etc.)
    if (!S_ISREG(stat_buf.st_mode)) {
        DBG_ERROR("Error: %s is not a regular file\n", file_path);
        return -2;
    }

    return 0;
}


