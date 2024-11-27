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

#ifndef HIKP_ROCE_TRP_H
#define HIKP_ROCE_TRP_H

#include "hikp_roce_ext_common.h"

#define TRP_DIV_NUM_T 4
#define ROCE_HIKP_TRP_REG_NUM 29
#define MAX_TRP_MODULE_NAME_LEN 20
#define ROCE_TRP_HANDLE(x) \
	{                      \
		#x, x              \
	}
#define TRP_MAX_BANK_NUM 0x3
#define PAYL_MAX_BANK_NUM 0x1
#define GAC_MAX_BANK_NUM 0x3
#define PER_TRP_DATA_NUM 2

struct roce_trp_req_param {
	struct bdf_t bdf;
	uint32_t bank_id;
	uint32_t block_id;
};

struct cmd_roce_trp_param_t {
	uint32_t sub_cmd;
	uint32_t bank_id;
	uint8_t bank_enter_flag;
	struct tool_target target;
};

struct roce_trp_head {
	uint8_t total_block_num;
	uint8_t cur_block_num;
	uint16_t rsvd;
};

struct roce_trp_res {
	uint32_t offset[ROCE_HIKP_TRP_REG_NUM];
	uint32_t data[ROCE_HIKP_TRP_REG_NUM];
};

struct roce_trp_res_param {
	uint32_t block_id;
	struct roce_trp_head head;
	struct roce_trp_res reg_data;
};

struct roce_trp_module {
	uint8_t module_name[MAX_TRP_MODULE_NAME_LEN];
	uint32_t sub_cmd_code;
};

enum roce_trp_type {
	TRP_RX = 1,
	GEN_AC,
	PAYL,
	COMMON,
};

#endif /* HIKP_ROCE_TRP_H */
