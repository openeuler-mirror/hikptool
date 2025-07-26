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

#ifndef HIKP_ROCE_EXT_COMMON_H
#define HIKP_ROCE_EXT_COMMON_H

#include "hikp_net_lib.h"

#define ROCE_MAX_REG_NUM (NET_MAX_REQ_DATA_NUM - 1)
#define ROCE_MAX_U64_REG_NUM 18

#define ROCE_HIKP_CAEP_REG_NUM_EXT ROCE_MAX_REG_NUM
#define ROCE_HIKP_GMV_REG_NUM_EXT ROCE_MAX_REG_NUM
#define ROCE_HIKP_MDB_REG_NUM_EXT ROCE_MAX_REG_NUM
#define ROCE_HIKP_PKT_REG_NUM_EXT ROCE_MAX_REG_NUM
#define ROCE_HIKP_QMM_REG_NUM_EXT ROCE_MAX_REG_NUM
#define ROCE_HIKP_SCC_REG_NUM_EXT ROCE_MAX_REG_NUM
#define ROCE_HIKP_TIMER_REG_NUM_EXT ROCE_MAX_REG_NUM
#define ROCE_HIKP_TRP_REG_NUM_EXT ROCE_MAX_REG_NUM
#define ROCE_HIKP_TSP_REG_NUM_EXT ROCE_MAX_REG_NUM
#define ROCE_HIKP_RST_REG_NUM ROCE_MAX_REG_NUM
#define ROCE_HIKP_GLOBAL_CFG_REG_NUM ROCE_MAX_REG_NUM
#define ROCE_HIKP_BOND_REG_NUM ROCE_MAX_REG_NUM
#define ROCE_HIKP_DFX_STA_NUM_EXT ROCE_MAX_U64_REG_NUM

#define ROCE_HIKP_DATA_U64_FLAG 1 << 0

struct roce_ext_head {
	uint8_t total_block_num;
	uint8_t cur_block_num;
	uint8_t flags;
	uint8_t reserved;
};

struct roce_ext_res_param {
	struct roce_ext_head head;
	uint32_t reg_data[0];
};

struct roce_ext_res_data_u64 {
	uint32_t offset[ROCE_MAX_U64_REG_NUM];
	uint64_t data[ROCE_MAX_U64_REG_NUM];
	uint32_t rsv[4];
};

struct roce_ext_res_param_u64 {
	struct roce_ext_head head;
	uint32_t rsv;
	struct roce_ext_res_data_u64 reg_data;
};

struct reg_data {
	uint32_t *offset;
	union {
		void *data;
		uint32_t *data_u32;
		uint64_t *data_u64;
	};
};

struct roce_ext_reg_name {
	const char **reg_name;
	uint8_t arr_len;
};

struct roce_ext_res_output {
	struct roce_ext_head res_head;
	struct reg_data reg;
	uint32_t per_val_size;
	struct roce_ext_reg_name reg_name;
};

void hikp_roce_ext_execute(struct major_cmd_ctrl *self,
			   enum roce_cmd_type cmd_type,
			   int (*get_data)(struct hikp_cmd_ret **cmd_ret,
					   uint32_t block_id,
					   struct roce_ext_reg_name *reg_name));

#endif /* HIKP_ROCE_EXT_COMMON_H */
