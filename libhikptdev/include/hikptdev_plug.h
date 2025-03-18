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

#ifndef HIKPTDEV_PLUG_H
#define HIKPTDEV_PLUG_H

#include <stdint.h>

struct hikp_cmd_header {
	uint32_t version;
	uint32_t mod_code;
	uint32_t cmd_code;
	uint32_t sub_cmd_code;
};

struct hikp_cmd_ret {
	uint32_t status;
	uint32_t version;
	uint32_t rsp_data_num;
	uint32_t rsp_data[0];
};

/* Module code */
enum cmd_module_type {
	PCIE_MOD = 0,
	SERDES_MOD = 1,
	NIC_MOD = 2,
	SOCIP_MOD = 3,
	ROCE_MOD = 4,
	ROH_MOD = 5,
	SAS_MOD = 6,
	SATA_MOD = 7,
	MAC_MOD = 8,
	DPDK_MOD = 9,
	CXL_MOD = 10,
	UB_MOD = 11,
	HCCS_MOD = 16,
	SDMA_MOD = 17
};

void hikp_unlock(void);
void hikp_cmd_init(struct hikp_cmd_header *req_header, uint32_t mod_code, uint32_t cmd_code,
		   uint32_t sub_cmd_code);
struct hikp_cmd_ret *hikp_cmd_alloc(struct hikp_cmd_header *req_header,
				    const void *req_data, uint32_t req_size);
void hikp_cmd_free(struct hikp_cmd_ret **cmd_ret);
int hikp_dev_init(void);
void hikp_dev_uninit(void);
int hikp_rsp_normal_check(const struct hikp_cmd_ret *cmd_ret);
int hikp_rsp_normal_check_with_version(const struct hikp_cmd_ret *cmd_ret, uint32_t version);

#endif /* HIKPTDEV_PLUG_H */
