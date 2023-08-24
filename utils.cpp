#include "utils.h"

Utils::Utils()
{
    generate_crc32_table();
}

Utils::~Utils()
{
}

// 初始化crc32表
void Utils::generate_crc32_table() {
    for (uint32_t i = 0; i < 256; i++) {
        uint32_t crc = i;
        for (uint32_t j = 8; j > 0; j--) {
            if (crc & 1) {
                crc = (crc >> 1) ^ POLYNOMIAL;
            } else {
                crc >>= 1;
            }
        }
        m_crc32_table[i] = crc;
    }
}

// 计算数据的crc32值
uint32_t Utils::crc32(uint32_t initial, const unsigned char *buf, size_t len) {
    uint32_t c = initial ^ 0xFFFFFFFF;
    for (size_t i = 0; i < len; i++) {
        c = m_crc32_table[(c ^ buf[i]) & 0xFF] ^ (c >> 8);
    }
    return c ^ 0xFFFFFFFF;
}


