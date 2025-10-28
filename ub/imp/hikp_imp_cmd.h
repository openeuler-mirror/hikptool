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

#ifndef HIKP_IMP_CMD_H
#define HIKP_IMP_CMD_H
#include "tool_lib.h"

#define PARAM_FUNC_MASK		HI_BIT(0)
#define PARAM_CHIP_MASK		HI_BIT(1)
#define PARAM_DIE_MASK		HI_BIT(2)

enum imp_func_type {
	IMP_FUNC_DUMP_LOG = 1,
};

struct imp_cmd_cfg {
	uint8_t func_type;
	uint8_t chip;
	uint8_t die;
	uint8_t rsvd;
	uint32_t param_mask;
};

#endif /* HIKP_IMP_CMD_H */
