#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <stdint.h>

#define POLYNOMIAL 0xEDB88320

class Utils
{

public:
    Utils();
    ~Utils();
    uint32_t crc32(uint32_t initial, const unsigned char *buf, size_t len);

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
    uint32_t m_crc32_table[256];
    void generate_crc32_table();
};

#endif // UTILS_H
