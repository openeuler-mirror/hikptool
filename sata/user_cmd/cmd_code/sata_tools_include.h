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

#ifndef __SATA_TOOLS_INCLUDE_H_
#define __SATA_TOOLS_INCLUDE_H_

#include "hikptdev_plug.h"
#include "tool_lib.h"

#define MAX_PARA_LENTH 10

struct tool_sata_cmd {
	uint32_t sata_cmd_type;
	uint32_t phy_id;
	uint32_t chip_id;
	uint32_t die_id;
};

#endif
