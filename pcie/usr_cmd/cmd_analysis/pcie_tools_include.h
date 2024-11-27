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

#ifndef PCIE_TOOLS_INCLUDE_H
#define PCIE_TOOLS_INCLUDE_H

#include "hikptdev_plug.h"
#include "tool_lib.h"

#define MAX_PARA_LENTH 10

struct tool_pcie_cmd {
	uint32_t cmd_type;
	uint32_t chip_id;
	uint32_t port_id;
	uint32_t trace_mode_val;
	uint32_t dump_level_val;
	uint32_t read_offset_val;
	uint32_t read_module_val;
};

#endif /* PCIE_TOOLS_INCLUDE_H */
