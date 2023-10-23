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

#ifndef __PCIE_COMMON_API_H_
#define __PCIE_COMMON_API_H_

#include <stdint.h>
#include <stdio.h>

#define CONTACT(x, y) x##y
#define STR_INTER(x) #x
#define STR(x) STR_INTER(x)

struct print_info {
	char *buff;
	size_t buff_size;
	size_t used_lenth;
};

struct pcie_comm_api {
	int (*ltssm_trace_show)(uint32_t port_id);
	int (*ltssm_trace_clear)(uint32_t port_id);
	int (*ltssm_trace_mode_set)(uint32_t port_id, uint32_t mode);
	int (*ltssm_link_information_get)(uint32_t port_id);
	int (*distribution_show)(uint32_t chip_id);
	int (*err_status_show)(uint32_t port_id);
	int (*err_status_clear)(uint32_t port_id);
	int (*reg_dump)(uint32_t port_id, uint32_t dump_level);
	int (*reg_read)(uint32_t port_id, uint32_t moudle_id, uint32_t offset);
	int (*pm_trace)(uint32_t port_id);
};

struct pcie_comm_api *pcie_get_comm_api(void);

#endif
