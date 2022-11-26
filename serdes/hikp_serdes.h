/*
 * Copyright (c) 2022 Hisilicon Technologies Co., Ltd.
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

#ifndef __HIKP_SERDES_H__
#define __HIKP_SERDES_H__

#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <linux/if.h>
#include "tool_lib.h"

#define __SERDES_DESC(x) 1

enum serdes_cmd_type_e {
	SERDES_SYS_RESERVE0        = 0,
	SERDES_SYS_RESERVE1        = 1,
	SERDES_SYS_RESERVE2        = 2,
	SERDES_HILINK_INIT         = 3,
	SERDES_NORMAL_ADAPT        = 4,
	SERDES_DATA_RATE_SWITCH    = 5,
	SERDES_GET_DS_POWER_STATE  = 6,
	SERDES_SET_DS_POWER_DOWN   = 7,
	SERDES_MACRO_POWER_UP      = 8,
	SERDES_MACRO_POWER_DOWN    = 9,
	SERDES_PLL_POWER_DOWN      = 10,
	SERDES_LANE_RESET          = 11,
	SERDES_MCU_RESET           = 12,
	SERDES_GET_TX_CFG          = 13,
	SERDES_SET_TX_CFG          = 14,
	SERDES_GET_RX_CFG          = 15,
	SERDES_SET_RX_CFG          = 16,
	SERDES_CDR_CFG             = 17,
	SERDES_RX_ADAPT            = 18,
	SERDES_GET_VOLTAGE         = 19,
	SERDES_COUPLE_SEL          = 20,
	SERDES_GET_IMPEDANCE       = 21,
	SERDES_LOOPBACK            = 22,
	SERDES_PRBS                = 23,
	SERDES_FOUR_EYE_SCAN       = 24,
	SERDES_FULL_EYE_SCAN       = 25,
	SERDES_GET_SNR             = 26,
	SERDES_SSC                 = 27,
	SERDES_GET_PN              = 28,
	SERDES_SET_PN              = 29,
	SERDES_GET_BITORDER        = 30,
	SERDES_SET_BITORDER        = 31,
	SERDES_KEY_INFO            = 32,
	SERDES_DPM_INFO            = 33,
	SERDES_REG_READ_BY_ADDR    = 34,
	SERDES_REG_READ_BY_NAME    = 35,
	SERDES_REG_WRITE_BY_ADDR   = 36,
	SERDES_REG_WRITE_BY_NAME   = 37,
	SERDES_DUMP_REG            = 38,
	SERDES_GET_ADAPT_LOG       = 39,
	SERDES_GET_PCIE_LINK_LOG   = 40,
	SERDES_GREENBOX            = 41,
	SERDES_GET_FW_VERSION      = 42,
	SERDES_FW_LOAD             = 43,
	SERDES_GET_VERSION         = 44,
	SERDES_TYPE_NUM
};

enum hilink_dump_type_e {
	HILINK_SERDES_REG_CS = 0,
	HILINK_SERDES_REG_DS,
	HILINK_SERDES_REG_CSDS,
	HILINK_SERDES_REG_RAM,
	HILINK_SUBCTRL_REG,
	HILINK_DUMP_TYPE_END
};

enum hilink_use_mode_e {
	HILINK_USE_MODE_NORMAL = 0,
	HILINK_USE_MODE_PCIE,
	HILINK_USE_MODE_SATA,
	HILINK_USE_MODE_SAS,
	HILINK_USE_MODE_HCCS,
	HILINK_USE_MODE_ETH,
	HILINK_USE_MODE_FC,
	HILINK_USE_MODE_CXL,
	HILINK_USE_MODE_ROH,
	HILINK_USE_MODE_ETH_ROH, /* lane0~3:roh, lane4~7:eth */
	HILINK_USE_MODE_ROH_ETH, /* lane0~3:eth, lane4~7:roh */
	HILINK_USE_MODE_UBN,
	HILINK_USE_MODE_END
};

enum hilink_ssc_type_e {
	HILINK_NO_SSC = 0,
	HILINK_SINGLE_SSC,
	HILINK_MULTI_SSC_FROM_INSIDE,
	HILINK_MULTI_SSC_FROM_SOUTH,
	HILINK_MULTI_SSC_FROM_NORTH,
	HILINK_MULTI_SSC_FROM_WEST,
	HILINK_MULTI_SSC_FROM_EAST,
	HILINK_SSC_TYPE_END
};

struct cmd_serdes_param {
	uint8_t chip_id;
	uint8_t macro_id;
	uint8_t start_sds_id;
	uint8_t sds_num;
	uint8_t val;
	uint8_t sub_cmd;
	uint8_t rsvd1;
	uint8_t rsvd2;
};

struct hilink_cmd_general {
	uint32_t chip_id      : 16;
	uint32_t macro_id     : 16;
	uint32_t start_sds_id : 16;
	uint32_t sds_num      : 16;
};

#define CMD_ARRAY_BUF_SIZE 64

struct hilink_cmd_in {
	struct hilink_cmd_general cmd_para;

	uint32_t cmd_type : 8;
	uint32_t sub_cmd  : 8;
	uint32_t rw       : 1;         /* 0: read, 1: write */
	uint32_t rsvd     : 15;

	uint32_t val;
	union {
		char field[CMD_ARRAY_BUF_SIZE];
		uint32_t addr;
		void *ex_param; /* Extended parameters */
	};
};

struct hilink_cmd_out {
	uint32_t str_len;                     /* out_str length */
	uint32_t result_offset;
	uint32_t type;                        /* 0:data; 1:string */
	unsigned int ret_val;
	char *out_str;
};

struct hilink_tx_param {
	int8_t fir_pre3;   /* -16~15 */
	int8_t fir_pre2;   /* -16~15 */
	int8_t fir_pre1;   /* -30~30 */
	uint8_t fir_main;   /* 0~63   */
	int8_t fir_post1;  /* -30~30 */
	int8_t fir_post2;  /* -16~15 */
	uint8_t swing;      /* H60: hswing_en; Other: swing */
	int8_t rsv;
};

#define HILINK_SERDES_RX_PARA_COUNT 18
struct hilink_rx_param {
	uint8_t data[HILINK_SERDES_RX_PARA_COUNT];
	uint8_t hilink_ip; /* defined by HILINK_IP_E */
	uint8_t rsvd;
};

#define HILINK_SERDES_RX_TAP_COUNT 24
struct hilink_serdes_rx_tap {
	int16_t tap_value[HILINK_SERDES_RX_TAP_COUNT];
	uint8_t hilink_ip; /* defined by HILINK_IP_E */
	uint8_t rsvd_0;
	uint16_t rsvd_1;
};

struct hilink_4p_eye_result {
	int32_t bottom;
	int32_t top;
	int32_t left;
	int32_t right;
};

struct hilink_detail_info {
	struct hilink_tx_param tx_cfg;
	uint8_t alos_status;
	uint8_t loopback_type;
	struct hilink_rx_param rx_ctle_cfg;
	struct hilink_serdes_rx_tap rx_tap_cfg;
	struct hilink_4p_eye_result eye_diagram;
	uint32_t snr;
};

struct hilink_brief_info {
	uint32_t tx_cs_sel        : 1;   /* [0] */
	uint32_t rx_cs_sel        : 1;   /* [1] */
	uint32_t tx_pn            : 1;   /* [2] */
	uint32_t rx_pn            : 1;   /* [3] */
	uint32_t tx_power         : 1;   /* [4] */
	uint32_t rx_power         : 1;   /* [5] */
	uint32_t refclk_sel       : 1;   /* [6] */
	uint32_t rsvd_0           : 9;   /* [7:15]  */
	uint32_t usemode          : 8;   /* [16:23] */
	uint32_t ssc_type         : 8;   /* [24:31] */
	uint32_t tx_data_rate_mhz;
	uint32_t rx_data_rate_mhz;
	uint32_t rsvd_1;
};

#endif
