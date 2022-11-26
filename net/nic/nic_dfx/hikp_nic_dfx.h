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

#ifndef __HIKP_NIC_DFX_H__
#define __HIKP_NIC_DFX_H__

#include "hikp_net_lib.h"

enum nic_dfx_cmd_type {
	SSU_DFX_REG_DUMP = 0,
	IGU_EGU_DFX_REG_DUMP = 1,
	PPP_DFX_REG_DUMP = 2,
	NCSI_DFX_REG_DUMP = 3,
	BIOS_COMM_DFX_REG_DUMP = 4,
	RCB_DFX_REG_DUMP = 5,
	TXDMA_DFX_REG_DUMP = 6,
	MASTER_DFX_REG_DUMP = 7,
	INVALID_MODULE = 0xFFFFFFFF,
};

enum nic_dfx_reg_type {
	INCORRECT_REG_TYPE = 0,
	TYPE_32_RX_ERROR_STATS = 1,
	TYPE_32_RX_DROP_STATS = 2,
	TYPE_32_RX_NORMAL_STATS = 3,
	TYPE_32_TX_ERROR_STATS = 4,
	TYPE_32_TX_DROP_STATS = 5,
	TYPE_32_TX_NORMAL_STATS = 6,
	TYPE_32_RX_PORT_ERROR_STATS = 7,
	TYPE_32_RX_PORT_DROP_STATS = 8,
	TYPE_32_RX_PORT_NORMAL_STATS = 9,
	TYPE_32_TX_PORT_ERROR_STATS = 10,
	TYPE_32_TX_PORT_DROP_STATS = 11,
	TYPE_32_TX_PORT_NORMAL_STATS = 12,
	TYPE_32_ERROR_STATUS = 13,
	TYPE_32_RUNNING_STATUS = 14,
	TYPE_32_CFG_STATUS = 15,
	TYPE_32_PORT_ERROR_STATUS = 16,
	TYPE_32_PORT_RUNNING_STATUS = 17,
	TYPE_32_PORT_CFG_STATUS = 18,
	TYPE_32_COMM_STATS = 19,
	TYPE_32_COMM_DROP_STATS = 20,
	TYPE_32_COMM_ERROR_STATS = 21,
	TYPE_64_RX_ERROR_STATS = 30,
	TYPE_64_RX_DROP_STATS = 31,
	TYPE_64_RX_NORMAL_STATS = 32,
	TYPE_64_TX_ERROR_STATS = 33,
	TYPE_64_TX_DROP_STATS = 34,
	TYPE_64_TX_NORMAL_STATS = 35,
	TYPE_64_RX_PORT_ERROR_STATS = 36,
	TYPE_64_RX_PORT_DROP_STATS = 37,
	TYPE_64_RX_PORT_NORMAL_STATS = 38,
	TYPE_64_TX_PORT_ERROR_STATS = 39,
	TYPE_64_TX_PORT_DROP_STATS = 40,
	TYPE_64_TX_PORT_NORMAL_STATS = 41,
	TYPE_64_COMM_STATS = 42,
	TYPE_64_COMM_DROP_STATS = 43,
	TYPE_64_COMM_ERROR_STATS = 44,
	TYPE_64_TX_PF_ERROR_STATS = 45,
	TYPE_64_TX_PF_DROP_STATS = 46,
	TYPE_64_TX_PF_NORMAL_STATS = 47,
	TYPE_INVALID = 255,
};

#define MAX_MODULE_NAME_LEN 20
struct dfx_module_cmd {
	uint8_t module_name[MAX_MODULE_NAME_LEN];
	uint32_t sub_cmd_code;
};

#define MAX_TYPE_NAME_LEN 40

enum nic_dfx_reg_width {
	WIDTH_32_BIT = 32,
	WIDTH_64_BIT = 64,
};

struct dfx_type_parse {
	uint8_t type_id;
	uint8_t bit_width;
	uint8_t type_name[MAX_TYPE_NAME_LEN];
};

struct nic_dfx_type_head {
	uint8_t type_id;
	uint8_t bit_width;
	uint8_t reg_num;
	uint8_t flag;
};

struct nic_dfx_req_para {
	struct bdf_t bdf;
	uint8_t block_id;
};

struct nic_dfx_param {
	struct tool_target target;
	uint32_t sub_cmd_code;
	uint8_t module_idx;
	uint8_t flag;
};

#define MODULE_SET_FLAG 0x1

#define MAX_DFX_DATA_NUM 59

/*********************************************************
 * All registers are returned as key-value pairs, and divided
 * into three groups of data.
 * 1. 32bit regs: R0 bit0~bit15: offset, R1 bit0~bit31: value
 * 2. 64bit regs: R0 bit0~bit15: offset, R0 bit16~bit31 high16 value, R1 bit0~bit31: low32 value
 *********************************************************/
#define DFX_REG_VALUE_OFF 16
#define DFX_REG_VALUE_MASK 0xFFFF
#define DFX_REG_ADDR_MASK 0xFFFF

#define WORD_NUM_PER_REG 2
#define BIT_NUM_OF_WORD 32
struct nic_dfx_rsp_head_t {
	uint8_t total_blk_num;
	uint8_t total_type_num;
	uint8_t cur_blk_size;
	uint8_t rsvd;
};

struct nic_dfx_rsp_t {
	struct nic_dfx_rsp_head_t rsp_head;
	uint32_t reg_data[MAX_DFX_DATA_NUM];
};

#endif
