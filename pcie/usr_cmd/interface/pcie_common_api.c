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

#include "pcie_common_api.h"
#include "pcie_link_ltssm.h"
#include "pcie_statistics.h"
#include "pcie_reg_dump.h"
#include "pcie_reg_read.h"

struct pcie_comm_api g_tools_api = {
	.ltssm_trace_show = pcie_ltssm_trace_show,
	.ltssm_trace_clear = pcie_ltssm_trace_clear,
	.ltssm_trace_mode_set = pcie_ltssm_trace_mode_set,
	.ltssm_link_information_get = pcie_ltssm_link_status_get,
	.distribution_show = pcie_port_distribution_get,
	.err_status_show = pcie_error_state_get,
	.err_status_clear = pcie_error_state_clear,
	.reg_dump = pcie_dumpreg_do_dump,
	.reg_read = pcie_reg_read,
	.pm_trace = pcie_pm_trace,
};


struct pcie_comm_api *pcie_get_comm_api(void)
{
	return &g_tools_api;
}
