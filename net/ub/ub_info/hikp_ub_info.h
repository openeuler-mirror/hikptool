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

#ifndef HIKP_UB_INFO_H
#define HIKP_UB_INFO_H

#include "hikp_net_lib.h"

enum ub_info_sub_cmd_type {
	UB_BASIC_INFO_DUMP = 0,
};

struct ub_info_param {
	struct tool_target target;
};

struct ub_info_req_para {
	struct bdf_t bdf;
};

struct ub_info_rsp {
	uint32_t cloud_mode;
	uint32_t pf_drv_type;
	uint32_t vf_drv_type;
};

#endif /* HIKP_UB_INFO_H */
