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

#ifndef HIKP_ROCE_BOND_H
#define HIKP_ROCE_BOND_H

#include "hikp_roce_ext_common.h"

struct cmd_roce_bond_param {
	struct tool_target target;
	uint32_t sub_cmd;
};

struct roce_bond_req_param {
	struct bdf_t bdf;
	uint32_t block_id;
};

#endif /* HIKP_ROCE_BOND_H */
