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
#include "os_common.h"
#include "pcie_common.h"
#include "hikptdev_plug.h"
#include "pcie_statistics.h"

static const char *g_global_width_name[GLOBAL_WIDTH_TABLE_SIZE] = {
	"PCIE_WIDTH_X1", "PCIE_WIDTH_X2", "PCIE_WIDTH_X4", "PCIE_WIDTH_X8", "PCIE_WIDTH_X16"
};

static const char *g_global_ndie_name[] = {
	"Ndie_A", "Ndie_B", "Ndie_C", "Ndie_D"
};

static int port_distribution_rsp_data_check(const struct hikp_cmd_ret *cmd_ret, uint32_t *port_num)
{
	size_t rsp_data_size, expect_data_size;
	struct pcie_port_info *port_info;
	int ret;

	ret = hikp_rsp_normal_check(cmd_ret);
	if (ret) {
		Err("PCIe Base", "port distribution cmd_ret normal check failed, ret: %d.\n", ret);
		return ret;
	}
	rsp_data_size = cmd_ret->rsp_data_num * sizeof(uint32_t);
	/* Check whether enough data of a port unit */
	if (rsp_data_size < sizeof(struct pcie_port_info)) {
		Err("PCIe Base", "port distribution rsp check failed, size: %u.\n",
			rsp_data_size);
		return -EINVAL;
	}
	/* Check whether enough data of n pairs */
	port_info = (struct pcie_port_info *)cmd_ret->rsp_data;
	*port_num = port_info->port_num;
	expect_data_size = sizeof(struct pcie_port_info) +
		sizeof(struct pcie_info_distribution_pair) * (*port_num);
	if (expect_data_size > rsp_data_size) {
		Err("PCIe Base",
		    "port distribution data size check failed, size: %u, expect size: %u.\n",
		    rsp_data_size, expect_data_size);
		return -EINVAL;
	}

	return 0;
}

static int pcie_portid_serdes_relation(const struct pcie_macro_info *macro_info,
				       uint32_t macro_num, uint32_t ndie_id)
{
	uint32_t i, j;

	if (ndie_id >= HIKP_ARRAY_SIZE(g_global_ndie_name)) {
		Info("PCIe Base", "ndie_id [%u]: %s\n", ndie_id, "UNKNOWN_NDIE");
		return -1;
	}

	if (macro_num >= MAX_MACRO_ONEPORT) {
		Info("PCIe Base", "macro_num [%u] exceeds the maximum array length\n", macro_num);
		return -1;
	}

	Info("PCIe Base", "\tndie_id: %s\n", g_global_ndie_name[ndie_id]);
	for (i = 0; i < macro_num; i++) {
		for (j = macro_info[i].lane_s; j <= macro_info[i].lane_e; j++)
			Info("PCIe Base", "\t\tmacro %d \t lane: %d\n", macro_info[i].id, j);
	}
	return 0;
}

int pcie_port_distribution_get(uint32_t chip_id)
{
	struct hikp_cmd_header req_header;
	struct hikp_cmd_ret *cmd_ret;
	struct pcie_info_req_para req_data = { 0 };
	uint32_t pair_num;
	struct pcie_port_info *port_info;
	uint32_t i;
	int ret;

	req_data.interface_id = chip_id;

	hikp_cmd_init(&req_header, PCIE_MOD, PCIE_INFO, INFO_DISTRIBUTION);
	cmd_ret = hikp_cmd_alloc(&req_header, &req_data, sizeof(req_data));
	ret = port_distribution_rsp_data_check(cmd_ret, &pair_num);
	if (ret)
		goto free_cmd_ret;

	port_info = (struct pcie_port_info *)cmd_ret->rsp_data;
	Info("PCIe Base", "Port Distribution Info (CHIP : 0x%x)Port_id Port_width\n", chip_id);
	for (i = 0; i < pair_num; i++) {
		if (port_info->info_pair[i].port_width >= HIKP_ARRAY_SIZE(g_global_width_name)) {
			Info("PCIe Base", "port_id[%u] %s\n", port_info->info_pair[i].port_id,
			"UNKNOWN_WIDTH");
			continue;
		}
		Info("PCIe Base", "port_id[%u] %s\n", port_info->info_pair[i].port_id,
			g_global_width_name[port_info->info_pair[i].port_width]);
		pcie_portid_serdes_relation(port_info->info_pair[i].macro_info,
					    port_info->info_pair[i].macro_num,
					    port_info->info_pair[i].ndie_id);
	}
free_cmd_ret:
	free(cmd_ret);

	return ret;
}

static int port_err_state_rsp_data_check(struct hikp_cmd_ret *cmd_ret)
{
	size_t rsp_data_size;
	int ret;

	ret = hikp_rsp_normal_check(cmd_ret);
	if (ret) {
		Err("PCIe Base", "error state get cmd_ret normal check failed, ret: %d.\n", ret);
		return ret;
	}
	rsp_data_size = cmd_ret->rsp_data_num * sizeof(uint32_t);
	if (rsp_data_size < sizeof(struct pcie_err_state)) {
		Err("PCIe Base",
		    "err state get rsp size check failed, rsp size: %u, expect size:%u.\n",
		    rsp_data_size, sizeof(struct pcie_err_state));
		return -EINVAL;
	}

	return 0;
}

int pcie_error_state_get(uint32_t port_id)
{
	struct hikp_cmd_header req_header;
	struct hikp_cmd_ret *cmd_ret;
	struct pcie_info_req_para req_data = { 0 };
	struct pcie_err_state *state;
	int ret;

	req_data.interface_id = port_id;

	hikp_cmd_init(&req_header, PCIE_MOD, PCIE_INFO, INFO_ERR_STATE_SHOW);
	cmd_ret = hikp_cmd_alloc(&req_header, &req_data, sizeof(req_data));
	ret = port_err_state_rsp_data_check(cmd_ret);
	if (ret)
		goto free_cmd_ret;

	state = (struct pcie_err_state *)cmd_ret->rsp_data;

	Info("PCIe Base", "phy_lane_err_counter = %u\n", state->test_cnt.bits.phy_lane_err_counter);
	Info("PCIe Base", "symbol_unlock_counter = %u\n",
	     state->symbol_unlock_cnt.bits.symbol_unlock_counter);
	Info("PCIe Base", "mac_int_status = 0x%x\n", state->mac_int_status);
	Info("PCIe Base", "loop_back_link_data_err_cnt = %u\n",
	     state->loop_link_data_err_cnt.bits.loop_back_link_data_err_cnt);
	Info("PCIe Base", "pcs_rx_err_cnt = %u\n", state->rx_err_cnt.bits.pcs_rx_err_cnt);
	Info("PCIe Base", "reg_framing_err_count = %u\n",
	     state->framing_err_cnt.bits.reg_framing_err_count);
	Info("PCIe Base", "dl_lcrc_err_num = %u\n", state->lcrc_err_num.bits.dl_lcrc_err_num);
	Info("PCIe Base", "dl_dcrc_err_num = %u\n", state->dcrc_err_num.bits.dl_dcrc_err_num);
free_cmd_ret:
	free(cmd_ret);

	return ret;
}

int pcie_error_state_clear(uint32_t port_id)
{
	struct hikp_cmd_header req_header;
	struct hikp_cmd_ret *cmd_ret;
	struct pcie_info_req_para req_data = { 0 };
	int ret;

	req_data.interface_id = port_id;
	hikp_cmd_init(&req_header, PCIE_MOD, PCIE_INFO, INFO_ERR_STATE_CLEAR);
	cmd_ret = hikp_cmd_alloc(&req_header, &req_data, sizeof(req_data));
	ret = hikp_rsp_normal_check(cmd_ret);
	free(cmd_ret);

	return ret;
}
