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

#ifndef __HIKP_ROCE_SCC_H__
#define __HIKP_ROCE_SCC_H__

#include "hikp_net_lib.h"

#define ROCE_HIKP_SCC_REG_NUM 29
#define MAX_SCC_MODULE_NAME_LEN 20
#define ROCE_SCC_HANDLE(x) \
	{                      \
		#x, x              \
	}

struct roce_scc_req_param {
	struct bdf_t bdf;
	uint8_t reset_flag;
	uint32_t block_id;
};

struct cmd_roce_scc_param_t {
	struct tool_target target;
	uint8_t reset_flag;
	uint32_t sub_cmd;
};

struct roce_scc_head {
	uint8_t total_block_num;
	uint8_t cur_block_num;
	uint16_t rsvd;
};

struct roce_scc_res {
	uint32_t offset[ROCE_HIKP_SCC_REG_NUM];
	uint32_t data[ROCE_HIKP_SCC_REG_NUM];
};

struct roce_scc_res_param {
	uint32_t block_id;
	struct roce_scc_head head;
	struct roce_scc_res reg_data;
};

struct roce_scc_module {
	uint8_t module_name[MAX_SCC_MODULE_NAME_LEN];
	uint32_t sub_cmd_code;
};

enum roce_scc_type {
	COMMON = 1,
	DCQCN,
	DIP,
	HC3,
	LDCP,
	CFG,
};

#endif /* __HIKP_ROCE_SCC_H__ */
