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

#ifndef SDMA_DUMP_REG_H
#define SDMA_DUMP_REG_H

#include "sdma_tools_include.h"

struct sdma_dump_req_para {
	uint32_t chip_id;
	uint32_t die_id;
	uint32_t chn_id;
};

int sdma_dev_check(void);
int sdma_reg_dump(struct tool_sdma_cmd *cmd);

#endif /* SDMA_DUMP_REG_H */
