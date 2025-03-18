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

struct tool_pcie_cmd g_dumpreg_cmd = {
	.cmd_type = 0,
	.chip_id = (uint32_t)(-1),
	.port_id = (uint32_t)(-1),
	.trace_mode_val = 0,
	.dump_level_val = DUMP_PORT_LEVEL,
};

static int pcie_dumpreg_help(struct major_cmd_ctrl *self, const char *argv)
{
	HIKP_SET_USED(argv);

	printf("\n  Usage: %s\n", self->cmd_ptr->name);
	printf("\n         %s\n", self->cmd_ptr->help_info);
	printf("    %s, %-25s %s\n", "-i", "--interface", "please input port[x] first\n");
	printf("    %s, %-25s %s\n", "-l", "--level",
	       "set dump level 1:global, 2:port classification\n");
	printf("\n  Options:\n\n");
	printf("    %s, %-25s %s\n", "-h", "--help", "display this help and exit\n");
	printf("    %s, %-25s %s\n", "-d", "--dump",
	       "dump important regs to pcie_dumpreg.log file\n");
	printf("\n");

	return 0;
}

static int pcie_port_set(struct major_cmd_ctrl *self, const char *argv)
{
	uint32_t val;
	int ret;

	HIKP_SET_USED(self);

	ret = string_toui(argv, &val);
	if (ret) {
		printf("info set port id err %d.\n", ret);
		return -EINVAL;
	}
	g_dumpreg_cmd.port_id = val;

	return 0;
}

static int dump_level_set(struct major_cmd_ctrl *self, const char *argv)
{
	uint32_t val = 0;
	int ret;

	HIKP_SET_USED(self);

	ret = string_toui(argv, &val);
	if (ret || val < DUMP_GLOBAL_LEVEL || val > DUMP_PORT_LEVEL) {
		printf("info set id err, ret = %d, val = %u\n", ret, val);
		return -EINVAL;
	}
	g_dumpreg_cmd.dump_level_val = val;

	return 0;
}

static int pcie_dumpreg_dump(struct major_cmd_ctrl *self, const char *argv)
{
	HIKP_SET_USED(self);
	HIKP_SET_USED(argv);

	g_dumpreg_cmd.cmd_type = DUMPREG_DUMP;

	return 0;
}

static int pcie_dumpreg_excute_funs_call(uint32_t cmd_type)
{
	struct pcie_comm_api *comm_api = pcie_get_comm_api();
	uint32_t port_id = g_dumpreg_cmd.port_id;
	uint32_t level = g_dumpreg_cmd.dump_level_val;

	if (cmd_type != DUMPREG_DUMP)
		return -EINVAL;

	return comm_api->reg_dump(port_id, level);
}

static void pcie_dumpreg_execute(struct major_cmd_ctrl *self)
{
	int ret;
	const char *suc_msg[] = {
		"",
		"dumpreg_dump success."
	};
	const char *err_msg[] = {
		"pcie_dumpreg sub command type error.",
		"dumpreg_dump error."
	};

	ret = pcie_dumpreg_excute_funs_call(g_dumpreg_cmd.cmd_type);
	if (ret) {
		snprintf(self->err_str, sizeof(self->err_str), "%s\n",
			 err_msg[g_dumpreg_cmd.cmd_type]);
		self->err_no = ret;
		return;
	}
	printf("%s\n", suc_msg[g_dumpreg_cmd.cmd_type]);
}

static void cmd_pcie_dumpreg_init(void)
{
	struct major_cmd_ctrl *major_cmd = get_major_cmd();

	major_cmd->option_count = 0;
	major_cmd->execute = pcie_dumpreg_execute;

	cmd_option_register("-h", "--help", false, pcie_dumpreg_help);
	cmd_option_register("-d", "--dump", false, pcie_dumpreg_dump);
	cmd_option_register("-i", "--interface", true, pcie_port_set);
	cmd_option_register("-l", "--level", true, dump_level_set);
}

HIKP_CMD_DECLARE("pcie_dumpreg", "pcie dump important regs for problem location",
		 cmd_pcie_dumpreg_init);
