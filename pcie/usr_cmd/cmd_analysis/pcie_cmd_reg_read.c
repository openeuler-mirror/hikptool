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
#include "tool_lib.h"
#include "tool_cmd.h"
#include "pcie_common_api.h"
#include "pcie_common.h"
#include "pcie_tools_include.h"
#include "pcie_reg_dump.h"
#include "pcie_reg_read.h"

struct tool_pcie_cmd g_regread_cmd = {
	.cmd_type = 0,
	.chip_id = (uint32_t)(-1),
	.port_id = (uint32_t)(-1),
	.trace_mode_val = 0,
	.dump_level_val = DUMP_PORT_LEVEL,
	.read_offset_val = (uint32_t)(-1),
	.read_module_val = (uint32_t)(-1),
};

static int pcie_reg_read_help(struct major_cmd_ctrl *self, const char *argv)
{
	printf("\n  Usage: %s\n", self->cmd_ptr->name);
	printf("\n         %s\n", self->cmd_ptr->help_info);
	printf("    %s, %-25s %s\n", "-i", "--interface", "please input port[x] first\n");
	printf("    %s, %-25s %s\n", "-m", "--module", "set read module\n");
	printf("    %s, %-25s %s\n", "-o", "--offset", "set reg offset\n");
	printf("\n  Options:\n\n");
	printf("    %s, %-25s %s\n", "-h", "--help", "display this help and exit\n");
	printf("    %s, %-25s %s\n", "-r", "--read", "read pcie reg in specific module\n");
	printf("\n");

	return 0;
}

static int pcie_port_set(struct major_cmd_ctrl *self, const char *argv)
{
	uint32_t val;
	int ret;

	ret = string_toui(argv, &val);
	if (ret) {
		printf("info set port id err %d.\n", ret);
		return -EINVAL;
	}
	g_regread_cmd.port_id = val;

	return 0;
}

static int read_module_set(struct major_cmd_ctrl *self, const char *argv)
{
	uint32_t val;
	int ret;

	ret = pcie_read_name2module_id(argv, &val);
	if (ret) {
		printf("undefined module \"%s\".\n", argv);
		return -EINVAL;
	}
	g_regread_cmd.read_module_val = val;

	return 0;
}

static int read_offset_set(struct major_cmd_ctrl *self, const char *argv)
{
	uint32_t val;
	int ret;

	ret = string_toui(argv, &val);
	if (ret) {
		printf("info set offset err %d.\n", ret);
		return -EINVAL;
	}
	g_regread_cmd.read_offset_val = val;

	return 0;
}

static int pcie_reg_read_exe(struct major_cmd_ctrl *self, const char *argv)
{
	g_regread_cmd.cmd_type = REGRD_READ;

	return 0;
}

static int pcie_reg_read_excute_funs_call(uint32_t cmd_type)
{
	struct pcie_comm_api *comm_api = pcie_get_comm_api();
	uint32_t port_id = g_regread_cmd.port_id;
	uint32_t module_id = g_regread_cmd.read_module_val;
	uint32_t offset = g_regread_cmd.read_offset_val;

	if (cmd_type == REGRD_READ)
		return comm_api->reg_read(port_id, module_id, offset);
	else
		return -EINVAL;

	return 0;
}

static void pcie_reg_read_execute(struct major_cmd_ctrl *self)
{
	int ret;
	const char *suc_msg[] = {
		"",
		"regrd_read success."
	};
	const char *err_msg[] = {
		"pcie_regrd sub command type error.",
		"regrd_read error."
	};

	ret = pcie_reg_read_excute_funs_call(g_regread_cmd.cmd_type);
	if (ret == 0) {
		printf("%s\n", suc_msg[g_regread_cmd.cmd_type]);
	} else {
		snprintf(self->err_str, sizeof(self->err_str), "%s\n",
			 err_msg[g_regread_cmd.cmd_type]);
		self->err_no = ret;
	}
}

static void cmd_pcie_reg_read_init(void)
{
	struct major_cmd_ctrl *major_cmd = get_major_cmd();

	major_cmd->option_count = 0;
	major_cmd->execute = pcie_reg_read_execute;

	cmd_option_register("-h", "--help", false, pcie_reg_read_help);
	cmd_option_register("-r", "--read", false, pcie_reg_read_exe);
	cmd_option_register("-i", "--interface", true, pcie_port_set);
	cmd_option_register("-m", "--module", true, read_module_set);
	cmd_option_register("-o", "--offset", true, read_offset_set);
}

HIKP_CMD_DECLARE("pcie_regrd", "pcie reg read for problem location", cmd_pcie_reg_read_init);
