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

#ifndef __HIKP_ROCE_EXT_COMMON_H__
#define __HIKP_ROCE_EXT_COMMON_H__

#include "hikp_net_lib.h"

#define ROCE_MAX_REG_NUM (NET_MAX_REQ_DATA_NUM - 1)

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

struct roce_ext_head {
	uint8_t total_block_num;
	uint8_t cur_block_num;
	uint16_t reserved;
};

struct roce_ext_res_param {
	struct roce_ext_head head;
	uint32_t reg_data[0];
};

struct reg_data {
	uint32_t *offset;
	uint32_t *data;
};

void hikp_roce_ext_execute(struct major_cmd_ctrl *self,
			   enum roce_cmd_type cmd_type,
			   int (*get_data)(struct hikp_cmd_ret **cmd_ret,
					   uint32_t block_id));

#endif /* __HIKP_ROCE_EXT_COMMON_H__ */
