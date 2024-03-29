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

#ifndef __HIKP_ROCE_CAEP_H__
#define __HIKP_ROCE_CAEP_H__

#include "hikp_roce_ext_common.h"

#define ROCE_HIKP_CAEP_REG_NUM 29
#define PER_REG_NUM 2

struct cmd_roce_caep_param_t {
	struct tool_target target;
	uint32_t sub_cmd;
};

struct roce_caep_req_param {
	struct bdf_t bdf;
};

struct roce_caep_req_param_ext {
	struct roce_caep_req_param origin_param;
	uint32_t block_id;
};

struct roce_caep_res {
	uint32_t offset[ROCE_HIKP_CAEP_REG_NUM];
	uint32_t data[ROCE_HIKP_CAEP_REG_NUM];
};

struct roce_caep_res_param {
	uint32_t total_block_num;
	struct roce_caep_res reg_data;
};

enum roce_caep_cmd_type {
	CAEP_ORIGIN = 0,
	CAEP_EXT,
};

#endif /* __HIKP_ROCE_CAEP_H__ */
