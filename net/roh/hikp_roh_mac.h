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

#ifndef __HIKP_ROH_MAC_H__
#define __HIKP_ROH_MAC_H__

#include "tool_lib.h"
#include "hikp_net_lib.h"

#define MAX_CRD_SIZE 20

#define ROH_MAC_ROH_PORT_ID_OFFSET 4
#define ROH_MAC_DIR_OFFSET 5
#define ROH_MAC_CAM_OFFSET 32
#define ROH_MAC_LRH_LB_OFFSET 16
#define ROH_MAC_LRH_VNI_0_OFFSET 24
#define ROH_MAC_LRH_VNI_1_OFFSET 8
#define ROH_MAC_LRH_SEID_OFFSET 8
#define ROH_MAC_LRH_TTL_OFFSET 24
#define ROH_MAC_LRH_SL_OFFSET 28
#define ROH_MAC_LRH_FECN_OFFSET 1
#define ROH_MAC_LRH_RM_OFFSET 3
#define ROH_MAC_LRH_PLENGTH_OFFSET 5
#define ROH_MAC_LRH_NLP_OFFSET 19
#define ROH_MAC_LRH_CFG_OFFSET 24
#define ROH_MAC_LRH_VL_OFFSET 29
#define ROH_MAC_LRH_REG_MATCH_1 0
#define ROH_MAC_LRH_REG_MATCH_2 1
#define ROH_MAC_LRH_REG_MATCH_3 2
#define ROH_MAC_LRH_REG_MATCH_4 3
#define NUM_ROWS_HOLDING_CRDS 10
#define NUM_ROWS_INIT_CRDS 12
#define NUM_ROWS_TEMP_CRDS 20
#define NUM_REGS 4

#define MAX_CAM_SIZE 32
#define CMD_SHOW_MAC_TYPE_FLAG HI_BIT(31)
#define CMD_SHOW_CAM_FLAG HI_BIT(30)
#define CMD_SHOW_CREDIT_CNT HI_BIT(29)
#define COMMANDER_ERR_MAX_STRING 128
#define ROH_LINK_0 0
#define ROH_LINK_1 1
#define BLOCK_SIZE 20
#define DMAC_CONVERT_ENABLE_MASK 0x1
#define SMAC_CONVERT_ENABLE_MASK 0x10

enum print_crd {
	HOLDING_CRD = 0,
	INIT_CRD = 1,
	TEMP_CRD = 2,
};

struct roh_mac_param {
	struct tool_target target;
	uint32_t flag;
};

struct roh_mac_req_para {
	struct bdf_t bdf;
	uint8_t crd_type;
	uint32_t cam_block_index;
};

struct cam_table_entry_t {
	unsigned int eid;
	unsigned long mac;
};

union cut_reg {
	uint32_t value;
	uint16_t cut[2];
};

struct roh_mac_get_type {
	uint8_t mac_type;
};

struct roh_mac_cam_caps {
	uint32_t convert_enable;
	uint32_t cam_convert_enable;
};

struct roh_mac_cam_reg_num {
	uint32_t cam_reg_num;
};

struct roh_mac_cam_table {
	uint32_t cam_eid[20];
	uint32_t cam_mac_low32[20];
	uint32_t cam_mac_high16[20];
};

struct roh_mac_credit_data {
	uint32_t cut_reg_value[MAX_CRD_SIZE];
};

#endif /* __HIKP_ROH_MAC_H__ */
