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

#ifndef PCIE_REG_READ_H
#define PCIE_REG_READ_H

#include "pcie_common_api.h"

#define MAX_PCIE_MODULE_NAME_LEN 20

struct pcie_reg_read_req_para {
	uint32_t port_id;
	uint32_t module_id;
	uint32_t offset;
};

struct pcie_module_table {
	char module_name[MAX_PCIE_MODULE_NAME_LEN];
	uint32_t module_id;
};

enum pcie_module_id_list {
	AP_IOB_TX_REG_ID = 0,
	AP_IOB_RX_REG_ID = 1,
	AP_P2P_REG_ID = 2,
	AP_APAT_REG_ID = 3,
	AP_GLOBAL_REG_ID = 4,
	PCIPC_REG_ID = 5,
	AP_MCTP_REG_ID = 6,
	AP_ENGINE_REG_ID = 7,
	AP_INT_REG_ID = 8,
	AP_DMA_REG_ID = 9,
	TOP_REG_ID = 10,
	CORE_GLOBAL_REG_ID = 11,
	DL_REG_ID = 12,
	MAC_REG_ID = 13,
	TL_REG_ID = 14,
	TL_CORE_REG_ID = 15,
	TL_CORE_PF_REG_ID = 16,
	TL_CORE_DFX_REG_ID = 17,
	CFGSPACE_ID = 18,
	CXL_RCRB_ID = 19,
	PCS_GLB_REG_ID = 20,
	PCS_LANE_REG_ID = 21,
};

int pcie_reg_read(uint32_t port_id, uint32_t module_id, uint32_t offset);
int pcie_read_name2module_id(const char *module_name, uint32_t *module_id);

#endif /* PCIE_REG_READ_H */
