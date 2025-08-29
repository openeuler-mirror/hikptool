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

#ifndef HIKP_ROCE_SCC_H
#define HIKP_ROCE_SCC_H

#include "hikp_roce_ext_common.h"

#define ROCE_HIKP_SCC_REG_NUM 29
#define MAX_SCC_MODULE_NAME_LEN 20
#define ROCE_SCC_HANDLE(x) \
	{                      \
		#x, x              \
	}

#define MAX_PARSE_NUM                       5
#define ROCEE_SCC_SCH_EN_MASK               GENMASK(24, 24)
#define ROCEE_TM_SCH_EN_MASK                GENMASK(23, 23)
#define ROCEE_SCH_UNIT_VALUE_MASK           GENMASK(22, 16)
#define ROCEE_SCC_WL_CFG_MASK               GENMASK(15, 12)
#define ROCEE_SCC_TOKEN_VALUE_MASK          GENMASK(11, 8)
#define ROCEE_SCC_SCH_EN_SHIFT              24
#define ROCEE_TM_SCH_EN_SHIFT               23
#define ROCEE_SCH_UNIT_VALUE_SHIFT          16
#define ROCEE_SCC_WL_CFG_SHIFT              12
#define ROCEE_SCC_TOKEN_VALUE_SHIFT         8

struct roce_scc_req_param {
	struct bdf_t bdf;
	uint8_t reset_flag;
	uint32_t block_id;
};

struct cmd_roce_scc_param_t {
	struct tool_target target;
	uint8_t reset_flag;
	uint32_t sub_cmd;
	uint8_t verbose_flag;
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
	SCC_COMMON = 1,
	DCQCN,
	DIP,
	HC3,
	LDCP,
	CFG,
};

int hikp_roce_set_scc_bdf(char *nic_name);
void hikp_roce_set_scc_submodule(uint32_t module);
void hikp_roce_scc_execute(struct major_cmd_ctrl *self);
void hikp_roce_set_scc_verbose_en(uint8_t verbose_en);

#endif /* HIKP_ROCE_SCC_H */
