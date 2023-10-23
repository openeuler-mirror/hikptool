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

#ifndef HIKP_UB_CRD_H
#define HIKP_UB_CRD_H

#include "hikp_net_lib.h"

#define MAX_CRD_SIZE 20
#define NUM_ROWS_INIT_CRDS 6
#define NUM_ROWS_TEMP_CRDS 12

enum ub_crd_sub_cmd_type {
	UB_CRD_INFO_DUMP = 0,
};

union cut_reg {
	uint32_t value;
	uint16_t cut[2];
};

struct ub_crd_param {
	struct tool_target target;
};

struct ub_crd_req_para {
	struct bdf_t bdf;
};

struct ub_crd_rsp {
	uint32_t cut_reg_value[MAX_CRD_SIZE];
};

#endif /* HIKP_UB_CRD_H */
