/* SPDX-License-Identifier: ((GPL-2.0+ WITH Linux-syscall-note) OR MIT) */
/*
 * Rockchip module information
 * Copyright (C) 2018-2019 Rockchip Electronics Co., Ltd.
 */

#ifndef _UAPI_RKMODULE_CAMERA_H
#define _UAPI_RKMODULE_CAMERA_H

#include <linux/types.h>

//#include "rk_isp20_hw.h"
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

#define RKMODULE_API_VERSION		KERNEL_VERSION(0, 1, 0x2)

/* using for rk3588 dual isp unite */
#define RKMOUDLE_UNITE_EXTEND_PIXEL	128
/* using for rv1109 and rv1126 */
#define RKMODULE_EXTEND_LINE		24

#define RKMODULE_NAME_LEN		32
#define RKMODULE_LSCDATA_LEN		289

#define RKMODULE_MAX_VC_CH		4

#define RKMODULE_PADF_GAINMAP_LEN	1024
#define RKMODULE_PDAF_DCCMAP_LEN	256
#define RKMODULE_AF_OTP_MAX_LEN		3

#define RKMODULE_CAMERA_MODULE_INDEX	"rockchip,camera-module-index"
#define RKMODULE_CAMERA_MODULE_FACING	"rockchip,camera-module-facing"
#define RKMODULE_CAMERA_MODULE_NAME	"rockchip,camera-module-name"
#define RKMODULE_CAMERA_LENS_NAME	"rockchip,camera-module-lens-name"

#define RKMODULE_CAMERA_SYNC_MODE	"rockchip,camera-module-sync-mode"
#define RKMODULE_INTERNAL_MASTER_MODE	"internal_master"
#define RKMODULE_EXTERNAL_MASTER_MODE	"external_master"
#define RKMODULE_SLAVE_MODE		"slave"

/* BT.656 & BT.1120 multi channel
 * On which channels it can send video data
 * related with struct rkmodule_bt656_mbus_info
 */
#define RKMODULE_CAMERA_BT656_ID_EN_BITS_1		(0x1)
#define RKMODULE_CAMERA_BT656_ID_EN_BITS_2		(0x3)
#define RKMODULE_CAMERA_BT656_ID_EN_BITS_3		(0x7)
#define RKMODULE_CAMERA_BT656_ID_EN_BITS_4		(0xf)
#define RKMODULE_CAMERA_BT656_PARSE_ID_LSB		BIT(0)
#define RKMODULE_CAMERA_BT656_PARSE_ID_MSB		BIT(1)
#define RKMODULE_CAMERA_BT656_CHANNEL_0			BIT(2)
#define RKMODULE_CAMERA_BT656_CHANNEL_1			BIT(3)
#define RKMODULE_CAMERA_BT656_CHANNEL_2			BIT(4)
#define RKMODULE_CAMERA_BT656_CHANNEL_3			BIT(5)
#define RKMODULE_CAMERA_BT656_CHANNELS			(RKMODULE_CAMERA_BT656_CHANNEL_0 | \
							 RKMODULE_CAMERA_BT656_CHANNEL_1 | \
							 RKMODULE_CAMERA_BT656_CHANNEL_2 | \
							 RKMODULE_CAMERA_BT656_CHANNEL_3)

#define DPHY_MAX_LANE					4

#define RKMODULE_GET_MODULE_INFO	\
	_IOR('V', BASE_VIDIOC_PRIVATE + 0, struct rkmodule_inf)

#define RKMODULE_AWB_CFG	\
	_IOW('V', BASE_VIDIOC_PRIVATE + 1, struct rkmodule_awb_cfg)

#define RKMODULE_AF_CFG	\
	_IOW('V', BASE_VIDIOC_PRIVATE + 2, struct rkmodule_af_cfg)

#define RKMODULE_LSC_CFG	\
	_IOW('V', BASE_VIDIOC_PRIVATE + 3, struct rkmodule_lsc_cfg)

#define RKMODULE_GET_HDR_CFG	\
	_IOR('V', BASE_VIDIOC_PRIVATE + 4, struct rkmodule_hdr_cfg)

#define RKMODULE_SET_HDR_CFG	\
	_IOW('V', BASE_VIDIOC_PRIVATE + 5, struct rkmodule_hdr_cfg)

#define RKMODULE_SET_CONVERSION_GAIN	\
	_IOW('V', BASE_VIDIOC_PRIVATE + 6, __u32)

#define RKMODULE_GET_LVDS_CFG	\
	_IOR('V', BASE_VIDIOC_PRIVATE + 7, struct rkmodule_lvds_cfg)

#define RKMODULE_SET_DPCC_CFG	\
	_IOW('V', BASE_VIDIOC_PRIVATE + 8, struct rkmodule_dpcc_cfg)

#define RKMODULE_GET_NR_SWITCH_THRESHOLD	\
	_IOR('V', BASE_VIDIOC_PRIVATE + 9, struct rkmodule_nr_switch_threshold)

#define RKMODULE_SET_QUICK_STREAM	\
	_IOW('V', BASE_VIDIOC_PRIVATE + 10, __u32)

#define RKMODULE_GET_BT656_INTF_TYPE	\
	_IOR('V', BASE_VIDIOC_PRIVATE + 11, __u32)

#define RKMODULE_GET_VC_FMT_INFO \
    _IOR('V', BASE_VIDIOC_PRIVATE + 12, struct rkmodule_vc_fmt_info)

#define RKMODULE_GET_VC_HOTPLUG_INFO \
    _IOR('V', BASE_VIDIOC_PRIVATE + 13, struct rkmodule_vc_hotplug_info)

#define RKMODULE_GET_START_STREAM_SEQ	\
	_IOR('V', BASE_VIDIOC_PRIVATE + 14, __u32)

#define RKMODULE_GET_VICAP_RST_INFO	\
	_IOR('V', BASE_VIDIOC_PRIVATE + 15, struct rkmodule_vicap_reset_info)

#define RKMODULE_SET_VICAP_RST_INFO	\
	_IOW('V', BASE_VIDIOC_PRIVATE + 16, struct rkmodule_vicap_reset_info)

#define RKMODULE_GET_BT656_MBUS_INFO	\
	_IOR('V', BASE_VIDIOC_PRIVATE + 17, struct rkmodule_bt656_mbus_info)

#define RKMODULE_GET_DCG_RATIO	\
	_IOR('V', BASE_VIDIOC_PRIVATE + 18, struct rkmodule_dcg_ratio)

#define RKMODULE_GET_SONY_BRL	\
	_IOR('V', BASE_VIDIOC_PRIVATE + 19, __u32)

#define RKMODULE_GET_CHANNEL_INFO	\
	_IOWR('V', BASE_VIDIOC_PRIVATE + 20, struct rkmodule_channel_info)

#define RKMODULE_GET_SYNC_MODE       \
	_IOR('V', BASE_VIDIOC_PRIVATE + 21, __u32)

#define RKMODULE_SET_SYNC_MODE       \
	_IOW('V', BASE_VIDIOC_PRIVATE + 22, __u32)

#define RKMODULE_SET_MCLK       \
	_IOW('V', BASE_VIDIOC_PRIVATE + 23, struct rkmodule_mclk_data)

#define RKMODULE_SET_LINK_FREQ       \
	_IOW('V', BASE_VIDIOC_PRIVATE + 24, __s64)

#define RKMODULE_SET_BUS_CONFIG       \
	_IOW('V', BASE_VIDIOC_PRIVATE + 25, struct rkmodule_bus_config)

#define RKMODULE_GET_BUS_CONFIG       \
	_IOR('V', BASE_VIDIOC_PRIVATE + 26, struct rkmodule_bus_config)

#define RKMODULE_SET_REGISTER       \
	_IOW('V', BASE_VIDIOC_PRIVATE + 27, struct rkmodule_reg)

#define RKMODULE_SYNC_I2CDEV       \
        _IOW('V', BASE_VIDIOC_PRIVATE + 28, __u8)

#define RKMODULE_SYNC_I2CDEV_COMPLETE       \
        _IOW('V', BASE_VIDIOC_PRIVATE + 29, __u8)

#define RKMODULE_SET_DEV_INFO       \
	_IOW('V', BASE_VIDIOC_PRIVATE + 30, struct rkmodule_dev_info)

#define RKMODULE_SET_CSI_DPHY_PARAM       \
	_IOW('V', BASE_VIDIOC_PRIVATE + 31, struct rkmodule_csi_dphy_param)

#define RKMODULE_GET_CSI_DPHY_PARAM       \
	_IOWR('V', BASE_VIDIOC_PRIVATE + 32, struct rkmodule_csi_dphy_param)

#define RKMODULE_GET_CSI_DSI_INFO       \
	_IOWR('V', BASE_VIDIOC_PRIVATE + 33, __u32)






//theses code are change for swift -start
//for swif sensor
//because BASE_VIDIOC_PRIVATE is 192  the  192-255 is private
//, system no use 192+36 =228,   so we use the last 

#define ADAPS_SET_PARAM_IN_CONFIG       \
	_IOW('V', BASE_VIDIOC_PRIVATE + (255-BASE_VIDIOC_PRIVATE), struct adaps_set_param_in_config)

#define ADAPS_SET_PARAM_IN_RUNTIME       \
	_IOW('V', BASE_VIDIOC_PRIVATE + (255-BASE_VIDIOC_PRIVATE-1), struct adaps_set_param_in_runtime)

#define ADAPS_GET_PARAM_PERFRAME       \
	_IOR('V', BASE_VIDIOC_PRIVATE + (255-BASE_VIDIOC_PRIVATE-2), struct adaps_get_param_perframe)

#define ADAPS_GET_EEPROM       \
	_IOR('V', BASE_VIDIOC_PRIVATE + (255-BASE_VIDIOC_PRIVATE-3), struct adaps_get_eeprom)

//theses code are change for swift -end


struct rkmodule_i2cdev_info {
	u8 slave_addr;
} __attribute__ ((packed));

struct rkmodule_dev_info {
	union {
		struct rkmodule_i2cdev_info i2c_dev;
		u32 reserved[8];
	};
} __attribute__ ((packed));

/* csi0/csi1 phy support full/split mode */
enum rkmodule_phy_mode {
	PHY_FULL_MODE,
	PHY_SPLIT_01,
	PHY_SPLIT_23,
};

struct rkmodule_mipi_lvds_bus {
	__u32 bus_type;
	__u32 lanes;
	__u32 phy_mode;
};

struct rkmodule_bus_config {
	union {
		struct rkmodule_mipi_lvds_bus bus;
		__u32 reserved[32];
	};
} __attribute__ ((packed));

struct rkmodule_reg {
    __u64 num_regs;
    __u64 preg_addr;
    __u64 preg_value;
    __u64 preg_addr_bytes;
    __u64 preg_value_bytes;
} __attribute__ ((packed));

/**
 * struct rkmodule_base_inf - module base information
 *
 */
struct rkmodule_base_inf {
	char sensor[RKMODULE_NAME_LEN];
	char module[RKMODULE_NAME_LEN];
	char lens[RKMODULE_NAME_LEN];
} __attribute__ ((packed));

/**
 * struct rkmodule_fac_inf - module factory information
 *
 */
struct rkmodule_fac_inf {
	__u32 flag;

	char module[RKMODULE_NAME_LEN];
	char lens[RKMODULE_NAME_LEN];
	__u32 year;
	__u32 month;
	__u32 day;
} __attribute__ ((packed));

/**
 * struct rkmodule_awb_inf - module awb information
 *
 */
struct rkmodule_awb_inf {
	__u32 flag;

	__u32 r_value;
	__u32 b_value;
	__u32 gr_value;
	__u32 gb_value;

	__u32 golden_r_value;
	__u32 golden_b_value;
	__u32 golden_gr_value;
	__u32 golden_gb_value;
} __attribute__ ((packed));

/**
 * struct rkmodule_lsc_inf - module lsc information
 *
 */
struct rkmodule_lsc_inf {
	__u32 flag;

	__u16 lsc_w;
	__u16 lsc_h;
	__u16 decimal_bits;

	__u16 lsc_r[RKMODULE_LSCDATA_LEN];
	__u16 lsc_b[RKMODULE_LSCDATA_LEN];
	__u16 lsc_gr[RKMODULE_LSCDATA_LEN];
	__u16 lsc_gb[RKMODULE_LSCDATA_LEN];

	__u16 width;
	__u16 height;
	__u16 table_size;
} __attribute__ ((packed));

/**
 * enum rkmodule_af_dir - enum of module af otp direction
 */
enum rkmodele_af_otp_dir {
	AF_OTP_DIR_HORIZONTAL = 0,
	AF_OTP_DIR_UP = 1,
	AF_OTP_DIR_DOWN = 2,
};

/**
 * struct rkmodule_af_otp - module af otp in one direction
 */
struct rkmodule_af_otp {
	__u32 vcm_start;
	__u32 vcm_end;
	__u32 vcm_dir;
};

/**
 * struct rkmodule_af_inf - module af information
 *
 */
struct rkmodule_af_inf {
	__u32 flag;
	__u32 dir_cnt;
	struct rkmodule_af_otp af_otp[RKMODULE_AF_OTP_MAX_LEN];
} __attribute__ ((packed));

/**
 * struct rkmodule_pdaf_inf - module pdaf information
 *
 */
struct rkmodule_pdaf_inf {
	__u32 flag;

	__u32 gainmap_width;
	__u32 gainmap_height;
	__u32 dccmap_width;
	__u32 dccmap_height;
	__u32 dcc_mode;
	__u32 dcc_dir;
	__u16 gainmap[RKMODULE_PADF_GAINMAP_LEN];
	__u16 dccmap[RKMODULE_PDAF_DCCMAP_LEN];
} __attribute__ ((packed));

/**
 * struct rkmodule_otp_module_inf - otp module info
 *
 */
struct rkmodule_otp_module_inf {
	__u32 flag;
	__u8 vendor[8];
	__u32 module_id;
	__u16 version;
	__u16 full_width;
	__u16 full_height;
	__u8 supplier_id;
	__u8 year;
	__u8 mouth;
	__u8 day;
	__u8 sensor_id;
	__u8 lens_id;
	__u8 vcm_id;
	__u8 drv_id;
	__u8 flip;
} __attribute__ ((packed));

/**
 * struct rkmodule_inf - module information
 *
 */
struct rkmodule_inf {
	struct rkmodule_base_inf base;
	struct rkmodule_fac_inf fac;
	struct rkmodule_awb_inf awb;
	struct rkmodule_lsc_inf lsc;
	struct rkmodule_af_inf af;
	struct rkmodule_pdaf_inf pdaf;
	struct rkmodule_otp_module_inf module_inf;
} __attribute__ ((packed));

/**
 * struct rkmodule_awb_inf - module awb information
 *
 */
struct rkmodule_awb_cfg {
	__u32 enable;
	__u32 golden_r_value;
	__u32 golden_b_value;
	__u32 golden_gr_value;
	__u32 golden_gb_value;
} __attribute__ ((packed));

/**
 * struct rkmodule_af_cfg
 *
 */
struct rkmodule_af_cfg {
	__u32 enable;
	__u32 vcm_start;
	__u32 vcm_end;
	__u32 vcm_dir;
} __attribute__ ((packed));

/**
 * struct rkmodule_lsc_cfg
 *
 */
struct rkmodule_lsc_cfg {
	__u32 enable;
} __attribute__ ((packed));

/**
 * NO_HDR: linear mode
 * HDR_X2: hdr two frame or line mode
 * HDR_X3: hdr three or line mode
 * HDR_COMPR: linearised and compressed data for hdr
 */
enum rkmodule_hdr_mode {
	NO_HDR = 0,
	HDR_X2 = 5,
	HDR_X3 = 6,
	HDR_COMPR,
};

enum rkmodule_hdr_compr_segment {
	HDR_COMPR_SEGMENT_4 = 4,
	HDR_COMPR_SEGMENT_12 = 12,
	HDR_COMPR_SEGMENT_16 = 16,
};

/* rkmodule_hdr_compr
 * linearised and compressed data for hdr: data_src = K * data_compr + XX
 *
 * bit: bit of src data, max 20 bit.
 * segment: linear segment, support 4, 6 or 16.
 * k_shift: left shift bit of slop amplification factor, 2^k_shift, [0 15].
 * slope_k: K * 2^k_shift.
 * data_src_shitf: left shift bit of source data, data_src = 2^data_src_shitf
 * data_compr: compressed data.
 */
struct rkmodule_hdr_compr {
	enum rkmodule_hdr_compr_segment segment;
	__u8 bit;
	__u8 k_shift;
	__u8 data_src_shitf[HDR_COMPR_SEGMENT_16];
	__u16 data_compr[HDR_COMPR_SEGMENT_16];
	__u32 slope_k[HDR_COMPR_SEGMENT_16];
};

/**
 * HDR_NORMAL_VC: hdr frame with diff virtual channels
 * HDR_LINE_CNT: hdr frame with line counter
 * HDR_ID_CODE: hdr frame with identification code
 */
enum hdr_esp_mode {
	HDR_NORMAL_VC = 0,
	HDR_LINE_CNT,
	HDR_ID_CODE,
};

/*
 * CSI/DSI input select IOCTL
 */
enum rkmodule_csi_dsi_seq {
	RKMODULE_CSI_INPUT = 0,
	RKMODULE_DSI_INPUT,
};

/**
 * lcnt: line counter
 *     padnum: the pixels of padding row
 *     padpix: the payload of padding
 * idcd: identification code
 *     efpix: identification code of Effective line
 *     obpix: identification code of OB line
 */
struct rkmodule_hdr_esp {
	enum hdr_esp_mode mode;
	union {
		struct {
			__u32 padnum;
			__u32 padpix;
		} lcnt;
		struct {
			__u32 efpix;
			__u32 obpix;
		} idcd;
	} val;
};

struct rkmodule_hdr_cfg {
	__u32 hdr_mode;
	struct rkmodule_hdr_esp esp;
	struct rkmodule_hdr_compr compr;
} __attribute__ ((packed));

/* sensor lvds sync code
 * sav: start of active video codes
 * eav: end of active video codes
 */
struct rkmodule_sync_code {
	__u16 sav;
	__u16 eav;
};

/* sensor lvds difference sync code mode
 * LS_FIRST: valid line ls-le or sav-eav
 *	   invalid line fs-fe or sav-eav
 * FS_FIRST: valid line fs-le
 *	   invalid line ls-fe
 * ls: line start
 * le: line end
 * fs: frame start
 * fe: frame end
 * SONY_DOL_HDR_1: sony dol hdr pattern 1
 * SONY_DOL_HDR_2: sony dol hdr pattern 2
 */
enum rkmodule_lvds_mode {
	LS_FIRST = 0,
	FS_FIRST,
	SONY_DOL_HDR_1,
	SONY_DOL_HDR_2
};

/* sync code of different frame type (hdr or linear) for lvds
 * act: valid line sync code
 * blk: invalid line sync code
 */
struct rkmodule_lvds_frm_sync_code {
	struct rkmodule_sync_code act;
	struct rkmodule_sync_code blk;
};

/* sync code for lvds of sensor
 * odd_sync_code: sync code of odd frame id for lvds of sony sensor
 * even_sync_code: sync code of even frame id for lvds of sony sensor
 */
struct rkmodule_lvds_frame_sync_code {
	struct rkmodule_lvds_frm_sync_code odd_sync_code;
	struct rkmodule_lvds_frm_sync_code even_sync_code;
};

/* lvds sync code category of sensor for different operation */
enum rkmodule_lvds_sync_code_group {
	LVDS_CODE_GRP_LINEAR = 0x0,
	LVDS_CODE_GRP_LONG,
	LVDS_CODE_GRP_MEDIUM,
	LVDS_CODE_GRP_SHORT,
	LVDS_CODE_GRP_MAX
};

/* struct rkmodule_lvds_cfg
 * frm_sync_code[index]:
 *  index == LVDS_CODE_GRP_LONG:
 *    sync code for frame of linear mode or for long frame of hdr mode
 *  index == LVDS_CODE_GRP_MEDIUM:
 *    sync code for medium long frame of hdr mode
 *  index == LVDS_CODE_GRP_SHOR:
 *    sync code for short long frame of hdr mode
 */
struct rkmodule_lvds_cfg {
	enum rkmodule_lvds_mode mode;
	struct rkmodule_lvds_frame_sync_code frm_sync_code[LVDS_CODE_GRP_MAX];
} __attribute__ ((packed));

/**
 * struct rkmodule_dpcc_cfg
 * enable: 0 -> disable dpcc, 1 -> enable multiple,
 *         2 -> enable single, 3 -> enable all;
 * cur_single_dpcc: the strength of single dpcc;
 * cur_multiple_dpcc: the strength of multiple dpcc;
 * total_dpcc: the max strength;
 */
struct rkmodule_dpcc_cfg {
	__u32 enable;
	__u32 cur_single_dpcc;
	__u32 cur_multiple_dpcc;
	__u32 total_dpcc;
} __attribute__ ((packed));

/**
 * nr switch by gain
 * direct: 0 -> up_thres LSNR to HSNR, 1 -> up_thres HSNR to LSNR
 * up_thres: threshold of nr change from low gain to high gain
 * down_thres: threshold of nr change from high gain to low gain;
 * div_coeff: Coefficients converted from float to int
 */
struct rkmodule_nr_switch_threshold {
	__u32 direct;
	__u32 up_thres;
	__u32 down_thres;
	__u32 div_coeff;
} __attribute__ ((packed));

/**
 * enum rkmodule_bt656_intf_type
 * to support sony bt656 raw
 */
enum rkmodule_bt656_intf_type {
	BT656_STD_RAW = 0,
	BT656_SONY_RAW,
};

/**
 * struct rkmodule_vc_fmt_info - virtual channels fmt info
 *
 */
struct rkmodule_vc_fmt_info {
	__u32 width[RKMODULE_MAX_VC_CH];
	__u32 height[RKMODULE_MAX_VC_CH];
	__u32 fps[RKMODULE_MAX_VC_CH];
} __attribute__ ((packed));

/**
 * struct rkmodule_vc_hotplug_info - virtual channels hotplug status info
 * detect_status: hotplug status
 *     bit 0~3 means channels id, value : 0 -> plug out, 1 -> plug in.
 */
struct rkmodule_vc_hotplug_info {
	__u8 detect_status;
} __attribute__ ((packed));


/* sensor start stream sequence
 * RKMODULE_START_STREAM_DEFAULT: by default
 * RKMODULE_START_STREAM_BEHIND : sensor start stream should be behind the controller
 * RKMODULE_START_STREAM_FRONT  : sensor start stream should be in front of the controller
 */
enum rkmodule_start_stream_seq {
	RKMODULE_START_STREAM_DEFAULT = 0,
	RKMODULE_START_STREAM_BEHIND,
	RKMODULE_START_STREAM_FRONT,
};

/*
 * the causation to do cif reset work
 */
enum rkmodule_reset_src {
	RKCIF_RESET_SRC_NON = 0x0,
	RKCIF_RESET_SRC_ERR_CSI2,
	RKCIF_RESET_SRC_ERR_LVDS,
	RKICF_RESET_SRC_ERR_CUTOFF,
	RKCIF_RESET_SRC_ERR_HOTPLUG,
	RKCIF_RESET_SRC_ERR_APP,
};

struct rkmodule_vicap_reset_info {
	__u32 is_reset;
	enum rkmodule_reset_src src;
} __attribute__ ((packed));

struct rkmodule_bt656_mbus_info {
	__u32 flags;
	__u32 id_en_bits;
} __attribute__ ((packed));

/* DCG ratio (float) = integer + decimal / div_coeff */
struct rkmodule_dcg_ratio {
	__u32 integer;
	__u32 decimal;
	__u32 div_coeff;
};

struct rkmodule_channel_info {
	__u32 index;
	__u32 vc;
	__u32 width;
	__u32 height;
	__u32 bus_fmt;
	__u32 data_type;
	__u32 data_bit;
} __attribute__ ((packed));

/*
 * link to vicap
 * linear mode: pad0~pad3 for id0~id3;
 *
 * HDR_X2: id0 fiexd to vc0 for long frame
 *         id1 fixed to vc1 for short frame;
 *         id2~id3 reserved, can config by PAD2~PAD3
 *
 * HDR_X3: id0 fiexd to vc0 for long frame
 *         id1 fixed to vc1 for middle frame
 *         id2 fixed to vc2 for short frame;
 *         id3 reserved, can config by PAD3
 *
 * link to isp, the connection relationship is as follows
 */
enum rkmodule_max_pad {
	PAD0, /* link to isp */
	PAD1, /* link to csi wr0 | hdr x2:L x3:M */
	PAD2, /* link to csi wr1 | hdr      x3:L */
	PAD3, /* link to csi wr2 | hdr x2:M x3:S */
	PAD_MAX,
};

/*
 * sensor exposure sync mode
 */
enum rkmodule_sync_mode {
	NO_SYNC_MODE = 0,
	EXTERNAL_MASTER_MODE,
	INTERNAL_MASTER_MODE,
	SLAVE_MODE,
};

struct rkmodule_mclk_data {
	u32 enable;
	u32 mclk_index;
	u32 mclk_rate;
	u32 reserved[8];
};

/*
 * csi dphy param
 * lp_vol_ref -> Reference voltage-645mV for LP  Function control pin
 * for rk3588 dcphy
 * 3'b000 : 605mV
 * 3'b001 : 625mV
 * 3'b010 : 635mV
 * 3'b011 : 645mV
 * 3'b100 : 655mV
 * 3'b101 : 665mV
 * 3'b110 : 685mV
 * 3'b111 : 725mV
 *
 * lp_hys_sw -> LP-RX Hysteresis Level Control
 * for rk3588 dcphy
 * 2'b00=45mV
 * 2'b01=65mV
 * 2'b10=85mV
 * 2'b11=100mV
 *
 * lp_escclk_pol_sel -> LP ESCCLK Polarity sel
 * for rk3588 dcphy
 * 1'b0: normal
 * 1'b1: swap ,Increase 1ns delay
 *
 * skew_data_cal_clk -> Skew Calibration Manual Data Fine Delay Control Register
 * for rk3588 dcphy
 * BIT[4:0] 30ps a step
 *
 * clk_hs_term_sel/data_hs_term_sel -> HS-RX Termination Impedance Control
 * for rk3588 dcphy
 * 3b'000 : 102Ω
 * 3b'001 : 99.1Ω
 * 3b'010 : 96.6Ω (default)
 * 3b'011 : 94.1Ω
 * 3b'100 : 113Ω
 * 3b'101 : 110Ω
 * 3b'110 : 107Ω
 * 3b'111 : 104Ω
 */

enum csi2_dphy_vendor {
	PHY_VENDOR_INNO = 0x0,
	PHY_VENDOR_SAMSUNG = 0x01,
};

struct rkmodule_csi_dphy_param {
	u32 vendor;
	u32 lp_vol_ref;
	u32 lp_hys_sw[DPHY_MAX_LANE];
	u32 lp_escclk_pol_sel[DPHY_MAX_LANE];
	u32 skew_data_cal_clk[DPHY_MAX_LANE];
	u32 clk_hs_term_sel;
	u32 data_hs_term_sel[DPHY_MAX_LANE];
	u32 reserved[32];
};

//theses code are change for swift -start
//for swift code
//for swif sensor
enum adaps_work_mode {
    ADAPS_PTM_PHR_MODE   = 0, 
    ADAPS_PCM_MODE = 1,
    ADAPS_PTM_FHR_MODE   = 2,    
//    ADAPS_PTM_DEBUG_PHR_MODE = 3,
//    ADAPS_PTM_DEBUG_FHR_MODE=4,
    ADAPS_MODE_MAX,
};
typedef enum
{
    AdapsFramerateTypeUninitilized,
    AdapsFramerateType15FPS,
    AdapsFramerateType30FPS,
} AdapsFramerateType;
typedef enum
{
    AdapsVcselZoneCountUninitilized,
    AdapsVcselZoneCount1,
    AdapsVcselZoneCount4 = 4,
} AdapsVcselZoneCountType;
typedef enum
{
    AdapsDataTypeUninitilized,
    AdapsDataTypeDepth,
    AdapsDataTypeGrayscale,
} AdapsDataType;// output data format


typedef enum
{
    AdapsMeasurementTypeUninitilized,
    AdapsMeasurementTypeNormal,
    AdapsMeasurementTypeShort,
    AdapsMeasurementTypeFull
} AdapsMeasurementType;

typedef enum {
    AdapsEnvTypeUninitilized,
    AdapsEnvTypeIndoor,
    AdapsEnvTypeOutdoor,
} AdapsEnvironmentType;

typedef enum {
    AdapsPowerModeUninitilized,
    AdapsPowerModeNormal,
    AdapsPowerModeDiv3,
} AdapsPowerMode;

typedef enum {
    AdapsSensorNotChange,       // the role of tof sensor will be determined by the register configure of ads6401_sensor.xml
    AdapsSensorForceAsMaster,
    AdapsSensorForceAsSlave
} AdapsSensorForceRole;

typedef enum {
    AdapsVcselModeUninitilized,
    AdapsVcselModeOn,
    AdapsVcselModeOff,
} AdapsVcselMode;

 


// There are two group Calibration SRAM for spod address, every group has 4 calibration registers.
#define TOF_SRAM_ZONE_NUM                           8
#define CALIB_SRAM_REG_SIZE                         512 //480     //unit is bytes

#define SWIFT_DEVICE_NAME_LENGTH            64
#define SWIFT_SPOT_POSITION_SIZE            (CALIB_SRAM_REG_SIZE*TOF_SRAM_ZONE_NUM)  //512 * 4 * 2
#define SWIFT_SPOT_BOX                      1920  //960 * 2
#define SWIFT_OFFSET_SIZE                   960

#define OFFSET(structure, member) ((uintptr_t)&((structure*)0)->member)
#define MEMBER_SIZE(structure, member) sizeof(((structure*)0)->member)

#pragma pack(4)
typedef struct SwiftEepromData
{
    // In Swift EEPROM, one page has 64 bytes.
    // Page 1
    char            deviceName[SWIFT_DEVICE_NAME_LENGTH];
    // Page 2 - 65
    unsigned char   sramData[SWIFT_SPOT_POSITION_SIZE];
    // Page 66
    float           intrinsic[9];     // 36 bytes
    float           offset;           // 4 bytes
    __u32        refSpadSelection; // 4 bytes
    __u32        driverChannelOffset[4];  // 16 bytes
    __u32        distanceTemperatureCoefficient;
    // Page 67 - 186
    float           spotPos[SWIFT_SPOT_BOX]; // 7680 bytes, 120 pages
    // Page 187 - 246
    float           spotOffset[SWIFT_OFFSET_SIZE]; // 3840 bytes, 60 pages
    // Page 247
    __u32        tdcDelay[16]; // 8 bytes
    // Page 248
    float           indoorCalibTemperature; // Calibration temperature.
    float           indoorCalibRefDistance; // Calibration reference distance.
    float           outdoorCalibTemperature; // Calibration temperature.
    float           outdoorCalibRefDistance; // Calibration reference distance.
    float           calibrationInfo[12]; // reserved
    // Page 249 - 252 
    uint8_t         pxyHistogram[248]; // pxy histogram
    float           pxyDepth;          // pxy depth
    float           pxyNumberOfPulse;  // pxy number of pulse
    // Page 253
    uint16_t        markedPixels[32];  // 16 hot pixels, 16 dead pixels.
    // Page 254
    char            moduleInfo[64]; // adaps calibration info.
    // Page 255
    char            reservedPage255[SWIFT_DEVICE_NAME_LENGTH];
    // Page 256
    __u32            totalChecksum;
    __u32            sramDataChecksum;
    __u32            spotPositionChecksum;
    __u32            spotOffsetChecksum;
    __u32            proximityChecksum;
}swift_eeprom_data_t;


#define  AD4001_EEPROM_VERSION_INFO_OFFSET          OFFSET(swift_eeprom_data_t, deviceName)               /// 0, 0x00
#define  AD4001_EEPROM_VERSION_INFO_SIZE            MEMBER_SIZE(swift_eeprom_data_t, deviceName)

// please refer to data struct 'swift_eeprom_data_t' in SpadisPC
#define  AD4001_EEPROM_SPODPOS_OFFSET               OFFSET(swift_eeprom_data_t, sramData)               /// 64, 0x40
#define  AD4001_EEPROM_SPODPOS_SIZE                 MEMBER_SIZE(swift_eeprom_data_t, sramData)

#define  AD4001_EEPROM_INTRINSIC_OFFSET             OFFSET(swift_eeprom_data_t, intrinsic)              /// 4160 0x1040
#define  AD4001_EEPROM_INTRINSIC_SIZE               MEMBER_SIZE(swift_eeprom_data_t, intrinsic)         /// 9xsizeof(float)

#define  AD4001_EEPROM_ACCURATESPODPOS_OFFSET       OFFSET(swift_eeprom_data_t, spotPos)        /// 4224 0x1080
#define  AD4001_EEPROM_ACCURATESPODPOS_SIZE         MEMBER_SIZE(swift_eeprom_data_t, spotPos)   /// 4x240x2xsizeof(float)=1920x4=7680

#define  AD4001_EEPROM_SPODOFFSET_OFFSET            OFFSET(swift_eeprom_data_t, spotOffset)             /// 11904 0x2e80
#define  AD4001_EEPROM_SPODOFFSET_SIZE              MEMBER_SIZE(swift_eeprom_data_t, spotOffset)        /// 4x240xsizeof(float)=960x4=3840

#define  AD4001_EEPROM_TDCDELAY_OFFSET              OFFSET(swift_eeprom_data_t, tdcDelay)               /// 15744 0x3d80 
#define  AD4001_EEPROM_TDCDELAY_SIZE                MEMBER_SIZE(swift_eeprom_data_t, tdcDelay)          /// 2xsizeof(uint32_t)=16x4=64

#define  AD4001_EEPROM_INDOOR_CALIBTEMPERATURE_OFFSET      OFFSET(swift_eeprom_data_t, indoorCalibTemperature)               /// 15808 0x3dc0
#define  AD4001_EEPROM_INDOOR_CALIBTEMPERATURE_SIZE        MEMBER_SIZE(swift_eeprom_data_t, indoorCalibTemperature)          /// 1xsizeof(float)=1x4=4

#define  AD4001_EEPROM_INDOOR_CALIBREFDISTANCE_OFFSET      OFFSET(swift_eeprom_data_t, indoorCalibRefDistance)               /// 15812 0x3dc4
#define  AD4001_EEPROM_INDOOR_CALIBREFDISTANCE_SIZE        MEMBER_SIZE(swift_eeprom_data_t, indoorCalibRefDistance)          /// 1xsizeof(float)=1x4=4

#define  AD4001_EEPROM_OUTDOOR_CALIBTEMPERATURE_OFFSET      OFFSET(swift_eeprom_data_t, outdoorCalibTemperature)               /// 15816
#define  AD4001_EEPROM_OUTDOOR_CALIBTEMPERATURE_SIZE        MEMBER_SIZE(swift_eeprom_data_t, outdoorCalibTemperature)          /// 1xsizeof(float)=1x4=4

#define  AD4001_EEPROM_OUTDOOR_CALIBREFDISTANCE_OFFSET      OFFSET(swift_eeprom_data_t, outdoorCalibRefDistance)               /// 15820
#define  AD4001_EEPROM_OUTDOOR_CALIBREFDISTANCE_SIZE        MEMBER_SIZE(swift_eeprom_data_t, outdoorCalibRefDistance)          /// 1xsizeof(float)=1x4=4

#define  AD4001_EEPROM_PROX_HISTOGRAM_OFFSET        OFFSET(swift_eeprom_data_t, pxyHistogram)               /// 15824+12x(sizeof(float))=15872
#define  AD4001_EEPROM_PROX_HISTOGRAM_SIZE          MEMBER_SIZE(swift_eeprom_data_t, pxyHistogram)          /// 256

#define  AD4001_EEPROM_TOTAL_CHECKSUM_OFFSET        OFFSET(swift_eeprom_data_t, totalChecksum)
#define  AD4001_EEPROM_TOTAL_CHECKSUM_SIZE          MEMBER_SIZE(swift_eeprom_data_t, totalChecksum)

#pragma pack()




struct adaps_set_param_in_config{
    AdapsEnvironmentType env_type;
    AdapsMeasurementType measure_type;
//    AdapsPowerMode  powermode;
    AdapsFramerateType framerate_type;
    AdapsVcselZoneCountType vcselzonecount_type;
 };
 
struct adaps_set_param_in_runtime{
   AdapsEnvironmentType env_type;
   AdapsMeasurementType measure_type;
   AdapsVcselMode vcsel_mode;
   bool env_valid;
   bool measure_valid;
   bool vcsel_valid;
   
};

struct adaps_get_param_perframe{
    //float internal_temperature;
    __u32 internal_temperature; //since kernel doesn't use float type, the temperate is a expanded integer value (x100)
    __u8 laser_exposure_period;
    __u8 fine_exposure_time; 
};

#pragma pack(4)
 struct adaps_get_eeprom{
    __u8 pRawData[sizeof(swift_eeprom_data_t)];
    __u32 rawDataSize;//may be not need
 };
#pragma pack()


//for swift code
//theses code are change for swift -end



#endif /* _UAPI_RKMODULE_CAMERA_H */
