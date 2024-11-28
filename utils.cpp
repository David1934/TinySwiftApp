#include "utils.h"

Utils::Utils()
{
    currentIndex = 0;
    generate_crc32_table();
    replay_file_path = REPLAY_DATA_FILE_PATH;  // 替换为实际的目录路径
    QString fileExtension = REPLAY_DATA_FILE_EXT_NAME;  // 替换为实际的文件后缀名

    loadFiles(replay_file_path, fileExtension);
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

// 加载指定目录下特定后缀名的文件到文件列表
void Utils::loadFiles(const QString &directoryPath, const QString &fileExtension) {
    QDir dir(directoryPath);
    QStringList filters;
    filters << "*" + fileExtension;
    fileList = dir.entryList(filters, QDir::Files);
}

// 从文件列表中获取下一个文件内容加载到buffer，循环加载
QByteArray Utils::loadNextFileToBuffer() {
    if (fileList.isEmpty()) {
        return QByteArray();
    }

    QString currentFileName = fileList[currentIndex];
    QString fullFilePath = replay_file_path + "/" + currentFileName;
    QFile file(fullFilePath);
    if (file.open(QIODevice::ReadOnly)) {
        QByteArray buffer = file.readAll();
        file.close();

        // 更新索引，实现循环
        currentIndex = (currentIndex + 1) % fileList.size();
        return buffer;
    }

    return QByteArray();
}

bool Utils::is_replay_data_exist()
{
    bool ret;

    if (fileList.isEmpty())
    {
        ret = false;
    }
    else
    {
        ret = true;
    }

    return ret;
}

void Utils::test_pattern_generate(unsigned char *write_buf, int len, int ptn_idx)
{
    int i;

    switch (ptn_idx)
    {
        case ETP_FULL_00:
            memset(write_buf,0,len);
            break;

        case ETP_FULL_55:
            memset(write_buf,0x55,len);
            break;

        case ETP_FULL_AA:
            memset(write_buf,0xAA,len);
            break;

        case ETP_FULL_FF:
            memset(write_buf,0xFF,len);
            break;

        case ETP_00_TO_FF:
            for (i=0; i<len; i++)
            {
                write_buf[i] = (unsigned char) (i & 0xFF);
            }
            break;

        case ETP_FF_TO_00:
            for (i=0; i<len; i++)
            {
                write_buf[i] = (unsigned char) (0xFF - (i & 0xFF));
            }
            break;

        default:
            break;
    }
}

void Utils::hexdump(const unsigned char * buf, int buf_len, const char * title)
{
    int             i, j, k;
    const int       NoPerLine = 16;
    char            line_buf[100];
    int             line_len;

    if (NULL == buf) {
        DBG_ERROR("Null pointer");
        return;
    }

    if (NULL != title) {
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

        DBG_PRINTK("[%4d] %s\n", i * NoPerLine, line_buf);
    }
}

