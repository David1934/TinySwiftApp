#ifndef HOST_COMMUNICATION_H
#define HOST_COMMUNICATION_H

#if defined(RUN_ON_EMBEDDED_LINUX)

#include <QObject>
#include <QString>
#include <QMetaType>

#include "adaps_types.h"
#include "adaps_sender.h"
#include "host_device_comm_types.h"
#include "misc_device.h"

Q_DECLARE_METATYPE(capture_req_param_t*);

class Host_Communication : public QObject
{
Q_OBJECT

public:
    // 删除拷贝构造和赋值操作（防止复制单例）
    Host_Communication(const Host_Communication&) = delete;
    Host_Communication& operator=(const Host_Communication&) = delete;
    ~Host_Communication();

    int report_frame_raw_data(void* pFrameData, uint32_t frameData_size, frame_buffer_param_t *pFrmBufParam);
    int report_frame_depth16_data(void* pFrameData, uint32_t frameData_size, frame_buffer_param_t *pFrmBufParam);
    void get_backuped_script_buffer_info(UINT8 *workmode, UINT8 ** script_buffer, uint32_t *script_buffer_size, UINT8 ** blkwrite_reg_data, uint32_t *blkwrite_reg_count);
    int report_error_msg(UINT16 responsed_cmd, UINT16 err_code, char *err_msg, int err_msg_length);
    UINT8 get_req_out_data_type();
    UINT8 get_req_walkerror_version();
    BOOLEAN get_req_compose_subframe();
    BOOLEAN get_req_expand_pixel();
    BOOLEAN get_req_mirror_x();
    BOOLEAN get_req_mirror_y();

    // 获取单例实例
    static Host_Communication* getInstance();

signals:
    void start_capture();
    void stop_capture();
    void set_capture_options(capture_req_param_t* param);

private slots:
    //void on_eeprom_readout(void* pEEPROM_buffer, int EEPROM_size);

private:
    static Host_Communication* instance; // 静态成员变量声明
    bool connected;
    UINT32 backuped_script_buffer_size;
    UINT8 *backuped_script_buffer;     // script buffer backup from CMD_HOST_SIDE_START_CAPTURE
    UINT8 backuped_wkmode;
    UINT32 backuped_blkwrite_reg_count;
    UINT8 *backuped_blkwrite_reg_data;     // blkwrite_reg_data backup from CMD_HOST_SIDE_START_CAPTURE
    capture_req_param_t backuped_capture_req_param;
    Misc_Device *p_misc_device;

    unsigned long txRawdataFrameCnt;
    unsigned long firstRawdataFrameTimeUsec;
    unsigned long txDepth16FrameCnt;
    unsigned long firstDepth16FrameTimeUsec;

    Host_Communication(); // 私有构造函数（防止外部实例化）

    static int swift_sender_callback(SenderEventId_t id, void* arg_ptr, uint32_t arg_u32, ...);

    void swift_start_capture(CommandData_t* pCmdData, uint32_t rxDataLen);
    void swift_set_colormap_range(CommandData_t* pCmdData, uint32_t rxDataLen);
    void read_device_register(UINT16 cmdType, CommandData_t* pCmdData, uint32_t rxDataLen);
    void write_device_register(UINT16 cmdType, CommandData_t* pCmdData, uint32_t rxDataLen);
    int report_module_static_data();

    void swift_event_process(void* pRXData, uint32_t rxDataLen);
    void swift_sender_disconnected(void);
    void swift_sender_connected(void);
    int adaps_sender_init(void);
    int dump_buffer_data(void* dump_buf, const char *buffer_name, int callline);
    int dump_capture_req_param(capture_req_param_t* pCaptureReqParam);
    int dump_frame_param(frame_buffer_param_t *pDataBufferParam);
    int dump_module_static_data(module_static_data_t *pStaticDataParam);

};

#endif // RUN_ON_EMBEDDED_LINUX

#endif // HOST_COMMUNICATION_H

