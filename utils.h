#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdint.h>

#define POLYNOMIAL 0xEDB88320

class Utils
{

public:
    Utils();
    ~Utils();
    uint32_t crc32(uint32_t initial, const unsigned char *buf, size_t len);

private:
    uint32_t m_crc32_table[256];
    void generate_crc32_table();
};

#endif // UTILS_H
