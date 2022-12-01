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

#ifndef __PCIE_LINK_LTSSM_H_
#define __PCIE_LINK_LTSSM_H_

#include "pcie_common_api.h"

#define TRACE_STR_NUM 0x20
#define TRACER_DEPTH 65

struct pcie_ltssm_num_string {
	int ltssm;
	char ltssm_c[TRACE_STR_NUM];
};

struct pcie_trace_req_para {
	uint32_t port_id;
	uint32_t trace_mode;
};

union pcie_link_info {
	/* Define the struct bits */
	struct {
		uint32_t mac_cur_link_width : 6;     /* [5..0]  */
		uint32_t reserved_0 : 2;             /* [7..6]  */
		uint32_t mac_cur_link_speed : 4;     /* [11..8]  */
		uint32_t reserved_1 : 4;             /* [15..12]  */
		uint32_t mac_link_up : 1;            /* [16]  */
		uint32_t reserved_2 : 2;             /* [18..17]  */
		uint32_t lane_reverse : 1;           /* [19]  */
		uint32_t reserved_3 : 4;             /* [23..20]  */
		uint32_t mac_ltssm_st : 6;           /* [29..24]  */
		uint32_t reserved_4 : 2;             /* [31..30]  */
	} bits;

	uint32_t u32;
};

int pcie_ltssm_trace_show(uint32_t port_id);
int pcie_ltssm_trace_clear(uint32_t port_id);
int pcie_ltssm_trace_mode_set(uint32_t port_id, uint32_t mode);
int pcie_ltssm_link_status_get(uint32_t port_id);

#endif
