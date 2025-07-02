/*
 * Copyright (c) 2025 Hisilicon Technologies Co., Ltd.
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

#ifndef HIKP_ROCE_DFX_STA_H
#define HIKP_ROCE_DFX_STA_H

#include "hikp_roce_ext_common.h"

struct cmd_roce_dfx_sta_param_t {
	uint8_t reset_flag;
	struct tool_target target;
};

struct roce_dfx_sta_req_param {
	struct bdf_t bdf;
	uint32_t block_id;
	uint8_t reset_flag;
};

int hikp_roce_set_dfx_sta_bdf(char *nic_name);
void hikp_roce_dfx_sta_execute(struct major_cmd_ctrl *self);

#endif /* HIKP_ROCE_DFX_STA_H */
