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

#ifndef HIKP_ROCE_TSP_H
#define HIKP_ROCE_TSP_H

#include "hikp_roce_ext_common.h"

#define ROCE_HIKP_TSP_REG_NUM 29
#define TSP_PER_REG_NUM 2
#define MAX_TSP_MODULE_NAME_LEN 20
#define MAX_TSP_BANK_NUM 0x7
#define MAX_TGP_TMP_BANK_NUM 0x3
#define TSP_HANDLE(x)     \
	{                     \
		#x, x             \
	}

struct cmd_roce_tsp_param_t {
	uint32_t sub_cmd_code;
	uint32_t bank_id;
	uint8_t reset_flag;
	uint8_t bank_enter_flag;
	struct tool_target target;
};

struct roce_tsp_req_param {
	struct bdf_t bdf;
	uint8_t reset_flag;
	uint32_t bank_id;
};

struct roce_trp_res_data {
	uint32_t offset[ROCE_HIKP_TSP_REG_NUM];
	uint32_t data[ROCE_HIKP_TSP_REG_NUM];
};

struct roce_tsp_res_param {
	uint32_t total_block_num;
	struct roce_trp_res_data reg_data;
};

struct roce_tsp_module {
	uint8_t module_name[MAX_TSP_MODULE_NAME_LEN];
	uint32_t sub_cmd_code;
};

enum roce_tsp_sub_cmd_code {
	TSP_COMMON = 1,
	TDP,
	TGP_TMP,
};

int hikp_roce_set_tsp_bdf(char *nic_name);
void hikp_roce_set_tsp_bankid(uint32_t bank_id);
void hikp_roce_set_tsp_submodule(uint32_t module);
void hikp_roce_tsp_execute(struct major_cmd_ctrl *self);

#endif /* HIKP_ROCE_TSP_H */
