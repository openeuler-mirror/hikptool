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
#include "pcie_tools_include.h"
#include "pcie_common.h"

struct tool_pcie_cmd g_trace_cmd = {
	.cmd_type = 0,
	.chip_id = (uint32_t)(-1),
	.port_id = (uint32_t)(-1),
	.trace_mode_val = 0,
};


static int pcie_trace_help(struct major_cmd_ctrl *self, const char *argv)
{
	printf("\n  Usage: %s\n", self->cmd_ptr->name);
	printf("\n         %s\n", self->cmd_ptr->help_info);
	printf("    %s, %-25s %s\n", "-i", "--interface", "please input port[x]  first\n");
	printf("\n  Options:\n\n");
	printf("    %s, %-25s %s\n", "-h", "--help", "display this help and exit\n");
	printf("    %s, %-25s %s\n", "-c", "--clear", "clear ltssm trace\n");
	printf("    %s, %-25s %s\n", "-s", "--show", "show ltssm trace\n");
	printf("    %s, %-25s %s\n", "-m", "--mode",
	       "set ltssm trace mode val 1:recver_en 0:recver_dis\n");
	printf("    %s, %-25s %s\n", "-f", "--information", "display link information\n");
	printf("\n");

	return 0;
}

static int pcie_trace_clear(struct major_cmd_ctrl *self, const char *argv)
{
	g_trace_cmd.cmd_type = TRACE_CLEAR;

	return 0;
}


static int pcie_trace_show(struct major_cmd_ctrl *self, const char *argv)
{
	g_trace_cmd.cmd_type = TRACE_SHOW;

	return 0;
}

static int pcie_trace_mode_set(struct major_cmd_ctrl *self, const char *argv)
{
	int ret;
	uint32_t val = 0;

	g_trace_cmd.cmd_type = TRACE_MODE;
	ret = string_toui(argv, &val);
	if (ret || val > 1) {
		printf("tarce mode set err %d\n", ret);
		return -EINVAL;
	}
	g_trace_cmd.trace_mode_val = val;

	return 0;
}

static int pcie_link_information_get(struct major_cmd_ctrl *self, const char *argv)
{
	g_trace_cmd.cmd_type = TRACE_INFO;
	return 0;
}

static int pcie_port_id_set(struct major_cmd_ctrl *self, const char *argv)
{
	uint32_t val;
	int ret;

	ret = string_toui(argv, &val);
	if (ret) {
		printf("trace set port id err %d\n", ret);
		return -EINVAL;
	}
	g_trace_cmd.port_id = val;

	return 0;
}

static int pcie_trace_excute_funs_call(int cmd_type)
{
	struct pcie_comm_api *comm_api = pcie_get_comm_api();
	uint32_t recover_en = g_trace_cmd.trace_mode_val;
	uint32_t port_id = g_trace_cmd.port_id;

	if (cmd_type == TRACE_CLEAR)
		return comm_api->ltssm_trace_clear(port_id);
	else if (cmd_type == TRACE_SHOW)
		return comm_api->ltssm_trace_show(port_id);
	else if (cmd_type == TRACE_MODE)
		return comm_api->ltssm_trace_mode_set(port_id, recover_en);
	else if (cmd_type == TRACE_INFO)
		return comm_api->ltssm_link_information_get(port_id);
	else
		return -EINVAL;
}

static void pcie_trace_execute(struct major_cmd_ctrl *self)
{
	int ret;
	const char *suc_msg[] = {
		"",
		"pcie_trace_show success.",
		"pcie_trace_clear success.",
		"get mac link information success.",
		"pcie_trace_mode_set success."
	};
	const char *err_msg[] = {
		"pcie_trace sub command type error.",
		"pcie_trace_show error.",
		"pcie_trace_clear error.",
		"get mac link information error.",
		"pcie_trace_mode_set error."
	};

	ret = pcie_trace_excute_funs_call(g_trace_cmd.cmd_type);
	if (ret == 0) {
		printf("%s\n", suc_msg[g_trace_cmd.cmd_type]);
	} else {
		snprintf(self->err_str, sizeof(self->err_str), "%s\n",
			 err_msg[g_trace_cmd.cmd_type]);
		self->err_no = ret;
	}
}

static void cmd_pcie_trace_init(void)
{
	struct major_cmd_ctrl *major_cmd = get_major_cmd();

	major_cmd->option_count = 0;
	major_cmd->execute = pcie_trace_execute;

	cmd_option_register("-h", "--help", false, pcie_trace_help);
	cmd_option_register("-c", "--clear", false, pcie_trace_clear);
	cmd_option_register("-s", "--show", false, pcie_trace_show);
	cmd_option_register("-m", "--mode", true, pcie_trace_mode_set);
	cmd_option_register("-f", "--information", false, pcie_link_information_get);
	cmd_option_register("-i", "--interface", true, pcie_port_id_set);
}

HIKP_CMD_DECLARE("pcie_trace", "pcie ltssm trace", cmd_pcie_trace_init);
