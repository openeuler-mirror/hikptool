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
#include "tool_cmd.h"
#include "sata_tools_include.h"
#include "sata_common.h"
#include "sata_dump_reg.h"

struct tool_sata_cmd g_sata_dump_cmd = {
	.sata_cmd_type = DUMP_UNKNOWN,
	.phy_id = (uint32_t)(-1),
	.chip_id = (uint32_t)(-1),
	.die_id = (uint32_t)(-1),
};

static int sata_dump_help(struct major_cmd_ctrl *self, const char *argv)
{
	HIKP_SET_USED(argv);

	printf("\n  Usage: %s\n", self->cmd_ptr->name);
	printf("\n         %s\n", self->cmd_ptr->help_info);
	printf("    %s, %-25s %s\n", "-c", "--chipid", "please input chip id[x]  first\n");
	printf("    %s, %-25s %s\n", "-d", "--dieid", "please input die id[x]  first\n");
	printf("\n  Options:\n\n");
	printf("    %s, %-25s %s\n", "-h", "--help", "display this help and exit\n");
	printf("    %s, %-25s %s\n", "-p", "--portx", "set port num(0-1) to dump reg\n");
	printf("    %s, %-25s %s\n", "-g", "--global", "dump global dfx reg\n");
	printf("\n");

	return 0;
}

static int sata_set_id(struct major_cmd_ctrl *self, const char *argv, uint32_t *id)
{
	int ret;
	uint32_t val = 0;

	ret = string_toui(argv, &val);
	if (ret) {
		snprintf(self->err_str, sizeof(self->err_str), "Invalid id.");
		self->err_no = ret;
		return ret;
	}
	*id = val;
	return ret;
}

static int sata_set_chip_id(struct major_cmd_ctrl *self, const char *argv)
{
	HIKP_SET_USED(self);
	HIKP_SET_USED(argv);

	return sata_set_id(self, argv, &g_sata_dump_cmd.chip_id);
}

static int sata_set_die_id(struct major_cmd_ctrl *self, const char *argv)
{
	HIKP_SET_USED(self);
	HIKP_SET_USED(argv);

	return sata_set_id(self, argv, &g_sata_dump_cmd.die_id);
}

static int sata_dump_global(struct major_cmd_ctrl *self, const char *argv)
{
	HIKP_SET_USED(self);
	HIKP_SET_USED(argv);

	g_sata_dump_cmd.sata_cmd_type = DUMP_GLOBAL;
	return 0;
}

static int sata_dump_portx(struct major_cmd_ctrl *self, const char *argv)
{
	int ret;
	uint32_t val = 0;

	g_sata_dump_cmd.sata_cmd_type = DUMP_PORTX;
	ret = string_toui(argv, &val);
	if (ret || val > SATA_MAX_PORT_NUM) {
		snprintf(self->err_str, sizeof(self->err_str), "Invalid portid.");
		self->err_no = ret;
		return -EINVAL;
	}
	g_sata_dump_cmd.phy_id = val;

	return 0;
}

static int sata_dump_excute_funs_call(uint32_t cmd_type)
{
	if (cmd_type != DUMP_UNKNOWN)
		return sata_reg_dump(&g_sata_dump_cmd);

	return -1;
}

static void sata_dump_execute(struct major_cmd_ctrl *self)
{
	int ret;
	const char *suc_msg[] = {
		"",
		"sata_dump_global success.",
		"sata_dump_port success.",
		"sata_dump_axi success."
	};
	const char *err_msg[] = {
		"sata_dump failed, unknown cmd type",
		"sata_dump_global error.",
		"sata_dump_port error.",
		"sata_dump_axi error."
	};

	ret = sata_dump_excute_funs_call(g_sata_dump_cmd.sata_cmd_type);
	if (ret == 0) {
		printf("%s\n", suc_msg[g_sata_dump_cmd.sata_cmd_type]);
	} else {
		snprintf(self->err_str, sizeof(self->err_str), "%s\n",
			 err_msg[g_sata_dump_cmd.sata_cmd_type]);
		self->err_no = ret;
	}
}

static void cmd_sata_dump_init(void)
{
	struct major_cmd_ctrl *major_cmd = get_major_cmd();

	major_cmd->option_count = 0;
	major_cmd->execute = sata_dump_execute;

	cmd_option_register("-c", "--chipid", true, sata_set_chip_id);
	cmd_option_register("-d", "--dieid", true, sata_set_die_id);
	cmd_option_register("-h", "--help", false, sata_dump_help);
	cmd_option_register("-g", "--global", false, sata_dump_global);
	cmd_option_register("-p", "--portx", true, sata_dump_portx);
}

HIKP_CMD_DECLARE("sata_dump", "sata reg dump", cmd_sata_dump_init);
