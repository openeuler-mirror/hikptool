/*
 * Copyright (c) 2023 Hisilicon Technologies Co., Ltd.
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

#ifndef HIKP_UB_DFX_H
#define HIKP_UB_DFX_H

#include "hikp_net_lib.h"

#define MAX_DFX_DATA_NUM 59
#define MODULE_SET_FLAG 0x1

enum ub_dfx_cmd_type {
	LRB_DFX_REG_DUMP = 0,
	PFA_DFX_REG_DUMP = 1,
	PM_DFX_REG_DUMP = 2,
	INVALID_MODULE = 0xFFFFFFFF,
};

enum ub_dfx_reg_type {
	INCORRECT_REG_TYPE = 0,
	TYPE_32_STATS = 1,
	TYPE_32_RUNNING_STATUS = 2,
	TYPE_64_STATS = 3,
	TYPE_INVALID = 255,
};

#define MAX_TYPE_NAME_LEN 40

enum ub_dfx_reg_width {
	WIDTH_32_BIT = 32,
	WIDTH_64_BIT = 64,
};

struct dfx_type_parse {
	uint8_t type_id;
	uint8_t bit_width;
	uint8_t type_name[MAX_TYPE_NAME_LEN];
};

struct ub_dfx_param {
	struct tool_target target;
	uint32_t sub_cmd_code;
	uint8_t module_idx;
	uint8_t flag;
};

#define MAX_MODULE_NAME_LEN 20
struct dfx_module_cmd {
	uint8_t module_name[MAX_MODULE_NAME_LEN];
	uint32_t sub_cmd_code;
};

struct ub_dfx_req_para {
	struct bdf_t bdf;
	uint8_t block_id;
};

struct ub_dfx_type_head {
	uint8_t type_id;
	uint8_t bit_width;
	uint8_t reg_num;
	uint8_t flag;
};

struct ub_dfx_rsp_head {
	uint8_t total_blk_num;
	uint8_t total_type_num;
	uint8_t cur_blk_size;
	uint8_t rsvd;
};

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

struct ub_dfx_rsp {
	struct ub_dfx_rsp_head rsp_head;
	uint32_t reg_data[MAX_DFX_DATA_NUM];
};

#endif /* HIKP_UB_DFX_H */
