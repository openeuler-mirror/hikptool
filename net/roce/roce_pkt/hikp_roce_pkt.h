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

#ifndef __HIKP_ROCE_PKT_H__
#define __HIKP_ROCE_PKT_H__

#include "hikp_roce_ext_common.h"

#define ROCE_HIKP_PKT_REG_NUM 29
#define PKT_PER_REG_NUM 2

struct cmd_roce_pkt_param_t {
	uint8_t reset_flag;
	struct tool_target target;
};

struct roce_pkt_req_param {
	struct bdf_t bdf;
	uint8_t reset_flag;
};

struct roce_pkt_res {
	uint32_t offset[ROCE_HIKP_PKT_REG_NUM];
	uint32_t data[ROCE_HIKP_PKT_REG_NUM];
};

struct roce_pkt_res_param {
	uint32_t total_block_num;
	struct roce_pkt_res reg_data;
};

#endif /* __HIKP_ROCE_PKT_H__ */
