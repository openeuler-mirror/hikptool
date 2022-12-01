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

#include "tool_lib.h"
#include "tool_cmd.h"
#include "pcie_common_api.h"
#include "pcie_common.h"
#include "pcie_tools_include.h"

struct tool_pcie_cmd g_info_cmd = {
	.cmd_type = 0,
	.chip_id = (uint32_t)(-1),
	.port_id = (uint32_t)(-1),
	.trace_mode_val = 0,
};

static int pcie_info_help(struct major_cmd_ctrl *self, const char *argv)
{
	printf("\n  Usage: %s\n", self->cmd_ptr->name);
	printf("\n         %s\n", self->cmd_ptr->help_info);
	printf("    %s, %-25s %s\n", "-i", "--interface",
	       "please input chip[x] or port[x] first\n");
	printf("\n  Options:\n\n");
	printf("    %s, %-25s %s\n", "-h", "--help", "display this help and exit\n");
	printf("    %s, %-25s %s\n", "-d", "--distribution", "show distribution on this chip\n");
	printf("    %s, %-25s %s\n", "-es", "--error-show", "show error state on this port\n");
	printf("    %s, %-25s %s\n", "-ec", "--error-clear", "clear error state on this port\n");
	printf("\n");

	return 0;
}

static int pcie_distribution_show(struct major_cmd_ctrl *self, const char *argv)
{
	g_info_cmd.cmd_type = INFO_DISTRIBUTION;

	return 0;
}

static int pcie_err_state_show(struct major_cmd_ctrl *self, const char *argv)
{
	g_info_cmd.cmd_type = INFO_ERR_STATE_SHOW;

	return 0;
}

static int pcie_err_state_clear(struct major_cmd_ctrl *self, const char *argv)
{
	g_info_cmd.cmd_type = INFO_ERR_STATE_CLEAR;

	return 0;
}

static int pcie_port_chip_set(struct major_cmd_ctrl *self, const char *argv)
{
	uint32_t val;
	int ret;

	ret = string_toui(argv, &val);
	if (ret) {
		printf("info set id err %d\n", ret);
		return -EINVAL;
	}
	g_info_cmd.chip_id = val;
	g_info_cmd.port_id = val;

	return 0;
}

static int pcie_info_excute_funs_call(uint32_t cmd_type)
{
	struct pcie_comm_api *comm_api = pcie_get_comm_api();
	uint32_t chip_id = g_info_cmd.chip_id;
	uint32_t port_id = g_info_cmd.port_id;

	if (!comm_api)
		return -EINVAL;

	if (cmd_type == INFO_DISTRIBUTION)
		return comm_api->distribution_show(chip_id);
	else if (cmd_type == INFO_ERR_STATE_SHOW)
		return comm_api->err_status_show(port_id);
	else if (cmd_type == INFO_ERR_STATE_CLEAR)
		return comm_api->err_status_clear(port_id);
	else
		return -EINVAL;
}

static void pcie_info_execute(struct major_cmd_ctrl *self)
{
	int ret;
	const char *suc_msg[] = {
		"",
		"distribution_show success.",
		"err_status_show success.",
		"err_status_clear success."
	};
	const char *err_msg[] = {
		"pcie_info sub command type error.",
		"distribution_show error.",
		"err_status_show error.",
		"err_status_clear error."
	};

	ret = pcie_info_excute_funs_call(g_info_cmd.cmd_type);
	if (ret) {
		snprintf(self->err_str, sizeof(self->err_str), "%s\n",
			 err_msg[g_info_cmd.cmd_type]);
		self->err_no = ret;
		return;
	}
	printf("%s\n", suc_msg[g_info_cmd.cmd_type]);
}

static void cmd_pcie_info_init(void)
{
	struct major_cmd_ctrl *major_cmd = get_major_cmd();

	major_cmd->option_count = 0;
	major_cmd->execute = pcie_info_execute;

	cmd_option_register("-h", "--help", false, pcie_info_help);
	cmd_option_register("-d", "--distribution", false, pcie_distribution_show);
	cmd_option_register("-es", "--error-show", false, pcie_err_state_show);
	cmd_option_register("-ec", "--error-clear", false, pcie_err_state_clear);
	cmd_option_register("-i", "--interface", true, pcie_port_chip_set);
}

HIKP_CMD_DECLARE("pcie_info", "pcie information", cmd_pcie_info_init);
