/*
 * Copyright (c) 2026 Hisilicon Technologies Co., Ltd.
 * Hikptool is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 *
 * See the Mulan PSL v2 for more details.
 */
#ifndef HIKP_OPTICAL_DOM_H
#define HIKP_OPTICAL_DOM_H

#include <stdint.h>
#include <stdbool.h>
#include "tool_lib.h"

enum optical_dom_cmd {
	OPTICAL_DOM_CMD_DIAG = 1,
};

enum optical_dom_sub_cmd {
	OPTICAL_DOM_SUBCMD_GET_DATA = 1,
};

#define OPTICAL_DOM_MAX_LANES		8

#define OPTICAL_DOM_PAGE_00H		0
#define OPTICAL_DOM_PAGE_01H		1
#define OPTICAL_DOM_PAGE_02H		2
#define OPTICAL_DOM_PAGE_11H		17
#define OPTICAL_DOM_PAGE_14H		20
#define OPTICAL_DOM_BANK_0		0

#define VENDOR_NAME_LEN			16
#define VENDOR_PN_LEN			16
#define VENDOR_SN_LEN			16
#define DATE_CODE_LEN			8
#define VENDOR_REV_LEN			2

#define OPTICAL_DOM_HEADER_SIZE		132
#define OPTICAL_DOM_SLOT_SIZE		132
#define OPTICAL_DOM_PAGE_DATA_SIZE	128
#define OPTICAL_DOM_DATA_BLK_SIZE	232

enum sff_identifier {
	ID_UNKNOWN		= 0x0,
	ID_SFP			= 0x3,  /* SFP/SFP+/SFP28 */
	ID_QSFP			= 0xC,  /* QSFP */
	ID_QSFP_PLUS		= 0xD,  /* QSFP+ */
	ID_QSFP28		= 0x11, /* QSFP28 */
	ID_QSFP_DD		= 0x18, /* QSFP-DD 8X */
	ID_OSFP			= 0x19, /* OSFP 8X */
	ID_SFP_DD		= 0x1A, /* SFP-DD 2X */
	ID_QSFP_PLUS_CMIS	= 0x1E, /* QSFP+ with CMIS */
	ID_SFP_DD_CMIS		= 0x1F, /* SFP-DD with CMIS */
	ID_SFP_PLUS_CMIS	= 0x20, /* SFP+ with CMIS */
};

enum optical_dom_media_type {
	MEDIA_TYPE_UNDEFINED	= 0,
	MEDIA_TYPE_MMF		= 1,
	MEDIA_TYPE_SMF		= 2,
	MEDIA_TYPE_PASSIVE_COPPER = 3,
	MEDIA_TYPE_ACTIVE_CABLE	= 4,
	MEDIA_TYPE_BASE_T	= 5,
	MEDIA_TYPE_RSVD,
};

#define LANE_DATA_INVALID_FFFF		65535

#define DUMP_ANNOTATION_PAGE_LOWER	255
#define DUMP_BYTES_PER_ROW		16

struct dump_annotation {
	uint8_t page;
	uint8_t page_row_base;
	const char *label;
};

/* --- Protocol length constants (based on CMIS specification) --- */
#define DOM_CIS_VENDOR_OUI_LEN		3        /* OUI field size in bytes */

/* --- Register offsets (based on CMIS specification) --- */

/* Lower Page */
#define DOM_CIS_IDENT_OFFSET			0
#define DOM_CIS_CUR_TEMP_OFFSET			14
#define DOM_CIS_CUR_VCC_OFFSET			16
#define DOM_CIS_MODULE_TYPE_OFFSET		85

/* Page 00h upper (CMIS byte address; use DOM_CIS_SLOT_IDX) */
#define DOM_CIS_VENDOR_NAME_START		129
#define DOM_CIS_VENDOR_OUI_OFFSET		145
#define DOM_CIS_VENDOR_PN_START			148
#define DOM_CIS_VENDOR_REV_START		164
#define DOM_CIS_VENDOR_SN_START			166
#define DOM_CIS_DATE_YEAR_OFFSET		182
#define DOM_CIS_PWR_CLASS_OFFSET		200
#define DOM_CIS_PWR_MAX_OFFSET			201

/* Page 01h upper (CMIS byte address; use DOM_CIS_SLOT_IDX) */
#define DOM_CIS_SMF_LEN_OFFSET			132
#define DOM_CIS_OM5_LEN_OFFSET			133
#define DOM_CIS_OM4_LEN_OFFSET			134
#define DOM_CIS_OM3_LEN_OFFSET			135
#define DOM_CIS_OM2_LEN_OFFSET			136
#define DOM_CIS_NOM_WAVE_MSB			138
#define DOM_CIS_WAVE_TOL_MSB			140
#define DOM_CIS_DIAG_TYPE_OFFSET		151
#define DOM_CIS_DIAG_CHAN_ADVER_OFFSET		160
#define DOM_CIS_SMF_LEN_MUL_MSK			0xC0
#define DOM_CIS_SMF_LEN_VAL_MSK			0x3F
#define DOM_CIS_TX_BIAS_MUL_MSK			0x18
#define DOM_CIS_TX_BIAS_MUL_1X			0x00
#define DOM_CIS_TX_BIAS_MUL_2X			0x08
#define DOM_CIS_TX_BIAS_MUL_4X			0x10

/* Page 02h upper (CMIS byte address; use DOM_CIS_SLOT_IDX) */
#define DOM_CIS_TEMP_HALRM_OFFSET		128
#define DOM_CIS_TEMP_LALRM_OFFSET		130
#define DOM_CIS_TEMP_HWARN_OFFSET		132
#define DOM_CIS_TEMP_LWARN_OFFSET		134
#define DOM_CIS_VCC_HALRM_OFFSET		136
#define DOM_CIS_VCC_LALRM_OFFSET		138
#define DOM_CIS_VCC_HWARN_OFFSET		140
#define DOM_CIS_VCC_LWARN_OFFSET		142
#define DOM_CIS_TX_PWR_HALRM_OFFSET		176
#define DOM_CIS_TX_PWR_LALRM_OFFSET		178
#define DOM_CIS_TX_PWR_HWARN_OFFSET		180
#define DOM_CIS_TX_PWR_LWARN_OFFSET		182
#define DOM_CIS_TX_BIAS_HALRM_OFFSET		184
#define DOM_CIS_TX_BIAS_LALRM_OFFSET		186
#define DOM_CIS_TX_BIAS_HWARN_OFFSET		188
#define DOM_CIS_TX_BIAS_LWARN_OFFSET		190
#define DOM_CIS_RX_PWR_HALRM_OFFSET		192
#define DOM_CIS_RX_PWR_LALRM_OFFSET		194
#define DOM_CIS_RX_PWR_HWARN_OFFSET		196
#define DOM_CIS_RX_PWR_LWARN_OFFSET		198

/* Page 11h lane flags (CMIS byte address; use DOM_CIS_SLOT_IDX) */
#define DOM_CIS_TX_LOS_OFFSET			136
#define DOM_CIS_TX_LOL_OFFSET			137
#define DOM_CIS_RX_LOS_OFFSET			147
#define DOM_CIS_RX_LOL_OFFSET			148

/* Page 11h lane monitors (CMIS byte address; use DOM_CIS_SLOT_IDX) */
#define DOM_CIS_TX_PWR_OFFSET			154
#define DOM_CIS_TX_BIAS_OFFSET			170
#define DOM_CIS_RX_PWR_OFFSET			186

/* --- Firmware request/response structures --- */

enum optical_dom_module_status {
	MODULE_STATUS_OK			= 0,
	MODULE_STATUS_EIO			= 5,   /* I2C or access error */
	MODULE_STATUS_ENODEV			= 19,  /* Optical not present */
	MODULE_STATUS_EINVAL			= 22,  /* Invalid argument */
	MODULE_STATUS_EOPNOTSUPP		= 95,  /* Port not supported */
	MODULE_STATUS_CHIP_DIE_ERR		= 241,
	MODULE_STATUS_BLK_ID_ERR		= 242,
	MODULE_STATUS_READ_FAILED		= 243,
	MODULE_STATUS_UVB_FAILED		= 244,
};

struct optical_dom_req_data {
	uint8_t  chip_id;
	uint8_t  port_id;
	uint8_t  die_id;
	uint8_t  rsvd_0;
	uint32_t block_id;
	uint32_t rsvd_1[4];
};

struct optical_dom_blk_rsp {
	uint16_t total_blk_num;
	uint16_t cur_blk_size;
	uint8_t  ret_code;
	uint8_t  rsvd[3];
	uint8_t  data[OPTICAL_DOM_DATA_BLK_SIZE];
};

struct optical_dom_blk_ctrl {
	uint32_t blk_id;
	uint32_t resp_blk_size;
	uint32_t total_blk_num;
};

#pragma pack(push, 1)

struct opti_dynamic_slot {
	uint8_t  page_num;
	uint8_t  bank_num;
	uint16_t reserved;
	uint8_t  data[OPTICAL_DOM_PAGE_DATA_SIZE];
};

struct opti_dynamic_container {
	uint8_t  hw_version;
	uint8_t  total_slots_count;
	uint8_t  rsvd[2];
	uint8_t  lower_page[OPTICAL_DOM_PAGE_DATA_SIZE];
	struct opti_dynamic_slot slots[];
};

#pragma pack(pop)

/* Convert CMIS byte address to slot-data array index */
#define DOM_CIS_SLOT_IDX(cmis_byte) ((cmis_byte) - 128)

/* Big-endian 16-bit read from raw byte pointer; caller casts result as needed */
#define DOM_U16_BE_AT(ptr, off) (((ptr)[(off)] << 8) | (ptr)[(off) + 1])

/* Little-endian 16-bit read from raw byte pointer; caller casts result as needed */
#define DOM_U16_LE_AT(ptr, off) ((ptr)[(off)] | ((ptr)[(off) + 1] << 8))

#define OPTICAL_DOM_PG_LOWER	HI_BIT(0)
#define OPTICAL_DOM_PG_00H	HI_BIT(1)
#define OPTICAL_DOM_PG_01H	HI_BIT(2)
#define OPTICAL_DOM_PG_02H	HI_BIT(3)
#define OPTICAL_DOM_PG_11H	HI_BIT(4)
#define OPTICAL_DOM_PG_14H	HI_BIT(5)

struct optical_dom_parse_data {
	uint32_t page_present;
	char	vendor_name[VENDOR_NAME_LEN + 1];
	char	vendor_pn[VENDOR_PN_LEN + 1];
	char	vendor_sn[VENDOR_SN_LEN + 1];
	char	vendor_rev[VENDOR_REV_LEN + 1];
	char	date_code[DATE_CODE_LEN + 1];
	uint8_t	identifier;
	uint8_t	media_type;
	uint8_t	host_lane_count;
	uint8_t	media_lane_count;
	double	temperature;
	double	voltage;
	double	max_power;
	uint8_t	power_class;
	uint8_t	bias_mul;
	uint8_t	vendor_oui[3];
	double	smf_length_km;
	double	om5_length_m;
	double	om4_length_m;
	double	om3_length_m;
	double	om2_length_m;
	double	nom_wavelength_nm;
	double	wavelength_tol_nm;
	double	tx_power_dbm[OPTICAL_DOM_MAX_LANES];
	double	rx_power_dbm[OPTICAL_DOM_MAX_LANES];
	double	tx_bias_ma[OPTICAL_DOM_MAX_LANES];
	double	host_snr[OPTICAL_DOM_MAX_LANES];
	double	media_snr[OPTICAL_DOM_MAX_LANES];
	uint8_t	rx_los_mask;
	uint8_t	tx_los_mask;
	uint8_t	rx_lol_mask;
	uint8_t	tx_lol_mask;
	double	temp_high_alarm;
	double	temp_low_alarm;
	double	temp_high_warn;
	double	temp_low_warn;
	double	volt_high_alarm;
	double	volt_low_alarm;
	double	volt_high_warn;
	double	volt_low_warn;
	double	tx_bias_high_alarm;
	double	tx_bias_low_alarm;
	double	tx_bias_high_warn;
	double	tx_bias_low_warn;
	double	tx_power_high_alarm;
	double	tx_power_low_alarm;
	double	tx_power_high_warn;
	double	tx_power_low_warn;
	double	rx_power_high_alarm;
	double	rx_power_low_alarm;
	double	rx_power_high_warn;
	double	rx_power_low_warn;
};

#define OPTICAL_DOM_PARAM_CHIP		HI_BIT(0)
#define OPTICAL_DOM_PARAM_PORT		HI_BIT(1)
#define OPTICAL_DOM_PARAM_DIE		HI_BIT(2)
#define OPTICAL_DOM_PARAM_RAW		HI_BIT(3)
#define OPTICAL_DOM_PARAM_MANDATORY \
	(OPTICAL_DOM_PARAM_CHIP | OPTICAL_DOM_PARAM_PORT | OPTICAL_DOM_PARAM_DIE)

struct optical_dom_param {
	uint8_t	chip_id;
	uint8_t	port_id;
	uint8_t	die_id;
	uint32_t param_mask;
};

#endif /* HIKP_OPTICAL_DOM_H */
