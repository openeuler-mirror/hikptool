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

#ifndef PCIE_STATISTICS_H
#define PCIE_STATISTICS_H

#include "pcie_common_api.h"

#define GLOBAL_WIDTH_TABLE_SIZE 5
#define MAX_MACRO_ONEPORT 3

union mac_test_cnt {
	/* Define the struct bits */
	struct {
		unsigned int phy_lane_err_counter : 16; /* [15..0]  */
		unsigned int reserved_0 : 16;           /* [31..16]  */
	} bits;
	/* Define an unsigned member */
	unsigned int u32;
};

union mac_symbol_unlock_cnt {
	/* Define the struct bits */
	struct {
		unsigned int symbol_unlock_counter : 16; /* [15..0]  */
		unsigned int reserved_0 : 16;            /* [31..16]  */
	} bits;
	/* Define an unsigned member */
	unsigned int u32;
};

union mac_loop_link_data_err_cnt {
	/* Define the struct bits */
	struct {
		unsigned int loop_back_link_data_err_cnt : 16; /* [15..0]  */
		unsigned int reserved_0 : 16;                  /* [31..16]  */
	} bits;
	/* Define an unsigned member */
	unsigned int u32;
};

union mac_rx_err_cnt {
	/* Define the struct bits */
	struct {
		unsigned int pcs_rx_err_cnt : 16; /* [15..0]  */
		unsigned int reserved_0 : 16;     /* [31..16]  */
	} bits;
	/* Define an unsigned member */
	unsigned int u32;
};

union mac_framing_err_cnt {
	/* Define the struct bits */
	struct {
		unsigned int reg_framing_err_count : 16; /* [15..0]  */
		unsigned int resvered : 16;              /* [31..16]  */
	} bits;
	/* Define an unsigned member */
	unsigned int u32;
};

union dfx_lcrc_err_num {
	/* Define the struct bits */
	struct {
		unsigned int dl_lcrc_err_num : 8; /* [7..0]  */
		unsigned int reserved_0 : 24;     /* [31..8]  */
	} bits;
	/* Define an unsigned member */
	unsigned int u32;
};

union dfx_dcrc_err_num {
	/* Define the struct bits */
	struct {
		unsigned int dl_dcrc_err_num : 8; /* [7..0]  */
		unsigned int reserved_0 : 24;     /* [31..8]  */
	} bits;
	/* Define an unsigned member */
	unsigned int u32;
};

struct pcie_macro_info {
	uint32_t id;
	uint32_t lane_s;
	uint32_t lane_e;
};

struct pcie_info_distribution_pair {
	uint32_t port_id;
	uint32_t port_width;
	uint32_t ndie_id;
	uint32_t macro_num;
	struct pcie_macro_info macro_info[MAX_MACRO_ONEPORT];
};

struct pcie_port_info {
	uint32_t port_num;
	struct pcie_info_distribution_pair info_pair[0];
};

struct pcie_err_state {
	union mac_test_cnt test_cnt;
	union mac_symbol_unlock_cnt symbol_unlock_cnt;
	union mac_loop_link_data_err_cnt loop_link_data_err_cnt;
	uint32_t mac_int_status;
	union mac_rx_err_cnt rx_err_cnt;
	union mac_framing_err_cnt framing_err_cnt;
	union dfx_lcrc_err_num lcrc_err_num;
	union dfx_dcrc_err_num dcrc_err_num;
};

struct pcie_info_req_para {
	uint32_t interface_id;
};

int pcie_port_distribution_get(uint32_t chip_id);
int pcie_error_state_get(uint32_t port_id);
int pcie_error_state_clear(uint32_t port_id);

#endif /* PCIE_STATISTICS_H */
