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

#ifndef HIKP_ROCE_GMV_H
#define HIKP_ROCE_GMV_H

#include "hikp_roce_ext_common.h"

#define ROCE_HIKP_GMV_REG_NUM 7
#define ROCE_HIKP_GMV_REG_SWICTH 2
#define ROCE_MAX_HIKPTOOL_GMV 256
struct cmd_roce_gmv_param {
	struct tool_target target;
	uint32_t gmv_index;
};

struct roce_gmv_req_para {
	struct bdf_t bdf;
	uint32_t gmv_index;
};

struct roce_gmv_rsp_data {
	uint32_t reg_offset[ROCE_HIKP_GMV_REG_NUM];
	uint32_t reg_data[ROCE_HIKP_GMV_REG_NUM];
};

enum roce_gmv_cmd_type {
	GMV_SHOW = 0x0,
};

int hikp_roce_set_gmv_bdf(char *nic_name);
void hikp_roce_set_gmv_index(uint32_t gmv_index);
void hikp_roce_gmv_execute(struct major_cmd_ctrl *self);

#endif /* HIKP_ROCE_GMV_H */
