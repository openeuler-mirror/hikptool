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

#ifndef __PCIE_REG_DUMP_H_
#define __PCIE_REG_DUMP_H_

#include "pcie_common_api.h"

#define PCIE_REG_NAME_LEN 60
#define MAX_STR_LEN 80
#define PCIE_DUMPREG_LOGFILE_NAME "pcie_dumpreg"

enum pcie_dump_level {
	DUMP_GLOBAL_LEVEL = 1,
	DUMP_PORT_LEVEL = 2,
};

enum pcie_dump_log_version {
	ORIGINAL_VERSION = 0,
};

struct pcie_dumpreg_info {
	uint32_t val;
	char name[PCIE_REG_NAME_LEN];
};

struct pcie_dump_req_para {
	uint32_t port_id;
	uint32_t level;
};

int pcie_dumpreg_do_dump(uint32_t port_id, uint32_t dump_level);

#endif
