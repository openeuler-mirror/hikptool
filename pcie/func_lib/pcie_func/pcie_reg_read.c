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

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "tool_lib.h"
#include "hikptdev_plug.h"
#include "os_common.h"
#include "pcie_common.h"
#include "pcie_reg_read.h"

static struct pcie_module_table g_module_table[] = {
	{"AP_IOB_TX_REG", AP_IOB_TX_REG_ID},
	{"AP_IOB_RX_REG", AP_IOB_RX_REG_ID},
	{"AP_P2P_REG", AP_P2P_REG_ID},
	{"AP_APAT_REG", AP_APAT_REG_ID},
	{"AP_GLOBAL_REG", AP_GLOBAL_REG_ID},
	{"PCIPC_REG", PCIPC_REG_ID},
	{"AP_MCTP_REG", AP_MCTP_REG_ID},
	{"AP_ENGINE_REG", AP_ENGINE_REG_ID},
	{"AP_DMA_REG", AP_DMA_REG_ID},
	{"TOP_REG", TOP_REG_ID},
	{"CORE_GLOBAL_REG", CORE_GLOBAL_REG_ID},
	{"DL_REG", DL_REG_ID},
	{"MAC_REG", MAC_REG_ID},
	{"TL_REG", TL_REG_ID},
	{"TL_CORE_REG", TL_CORE_REG_ID},
	{"TL_CORE_PF_REG", TL_CORE_PF_REG_ID},
	{"TL_CORE_DFX_REG", TL_CORE_DFX_REG_ID},
	{"CFGSPACE", CFGSPACE_ID},
	{"CXL_RCRB", CXL_RCRB_ID},
	{"PCS_GLB_REG", PCS_GLB_REG_ID},
	{"PCS_LANE_REG", PCS_LANE_REG_ID},
};

int pcie_read_name2module_id(const char *module_name, uint32_t *module_id)
{
	size_t i, num;

	if (module_name == NULL || module_id == NULL)
		return -EINVAL;

	num = HIKP_ARRAY_SIZE(g_module_table);
	for (i = 0; i < num; i++) {
		if (strcmp(module_name, g_module_table[i].module_name) == 0) {
			*module_id = g_module_table[i].module_id;
			return 0;
		}
	}

	return -EINVAL;
}

static int pcie_reg_read_result_show(const struct hikp_cmd_ret *cmd_ret)
{
	if (cmd_ret->rsp_data_num != 1) { /* 1 uint32_t data for reg read cmd */
		Err("pcie reg read data num check failed, num: %u.\n",
		    cmd_ret->rsp_data_num);
		return -EINVAL;
	}
	Info("RIGISTER VALUE[0x%08x].\n", cmd_ret->rsp_data[0]);

	return 0;
}

int pcie_reg_read(uint32_t port_id, uint32_t module_id, uint32_t offset)
{
	struct hikp_cmd_header req_header;
	struct hikp_cmd_ret *cmd_ret = NULL;
	struct pcie_reg_read_req_para req_data = { 0 };
	int ret = 0;

	req_data.port_id = port_id;
	req_data.module_id = module_id;
	req_data.offset = offset;
	hikp_cmd_init(&req_header, PCIE_MOD, PCIE_REGRD, REGRD_READ);
	cmd_ret = hikp_cmd_alloc(&req_header, &req_data, sizeof(req_data));
	ret = hikp_rsp_normal_check(cmd_ret);
	if (ret) {
		Err("pcie reg read cmd_ret check failed, ret: %d.\n", ret);
		goto free_cmd_ret;
	}
	ret = pcie_reg_read_result_show(cmd_ret);

free_cmd_ret:
	hikp_cmd_free(&cmd_ret);

	return ret;
}
