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
#include "sas_common.h"
#include "sas_tools_include.h"
#include "sas_dump_reg.h"

static int sas_dump_help(struct major_cmd_ctrl *self, const char *argv)
{
	printf("\n  Usage: %s\n", self->cmd_ptr->name);
	printf("\n         %s\n", self->cmd_ptr->help_info);
	printf("    %s, %-25s %s\n", "-c", "--chipid", "please input chip id[x]  first\n");
	printf("    %s, %-25s %s\n", "-d", "--dieid", "please input die id[x]  first\n");
	printf("\n  Options:\n\n");
	printf("    %s, %-25s %s\n", "-h", "--help", "display this help and exit\n");
	printf("    %s, %-25s %s\n", "-p", "--phyx", "set phy num(0-7) to dump reg\n");
	printf("    %s, %-25s %s\n", "-g", "--global", "dump global dfx reg\n");
	printf("    %s, %-25s %s\n", "-b", "--axi", "dump axi master reg\n");
	printf("\n");

	return 0;
}

static int sas_dump_global(struct major_cmd_ctrl *self, const char *argv)
{
	(void)sas_set_cmd_type(DUMP_GLOBAL);
	return 0;
}

static int sas_dump_phyx(struct major_cmd_ctrl *self, char const *argv)
{
	int ret;

	(void)sas_set_cmd_type(DUMP_PHYX);
	ret = sas_set_phy_id(self, argv);
	if (ret || sas_get_phy_id() > SAS_MAX_PHY_NUM) {
		snprintf(self->err_str, sizeof(self->err_str), "Invalid phyid.");
		self->err_no = ret;
		return -EINVAL;
	}

	return 0;
}

static int sas_dump_axi(struct major_cmd_ctrl *self, char const *argv)
{
	(void)sas_set_cmd_type(DUMP_AXI);
	return 0;
}

static int sas_dump_excute_funs_call(uint32_t cmd_type)
{
	if (cmd_type != SAS_UNKNOW_CMD)
		return sas_reg_dump(sas_get_cmd_p());

	return -1;
}

static void sas_dump_execute(struct major_cmd_ctrl *self)
{
	int ret, cmd;
	const char *suc_msg[] = {
		"sas_dump_global success.",
		"sas_dump_phy success.",
		"sas_dump_axi success."
	};
	const char *err_msg[] = {
		"sas_dump_global error.",
		"sas_dump_phy error.",
		"sas_dump_axi error.",
		"sas_dump failed, unknown type",
	};

	cmd = sas_get_cmd_type();
	ret = sas_dump_excute_funs_call(cmd);
	(void)sas_set_cmd_type(SAS_UNKNOW_CMD);
	if (ret == 0) {
		printf("%s\n", suc_msg[cmd]);
	} else {
		if (cmd == SAS_UNKNOW_CMD)
			cmd = DUMP_UNKNOWN_TYPE;
		snprintf(self->err_str, sizeof(self->err_str), "%s\n", err_msg[cmd]);
		self->err_no = ret;
	}
}

static void cmd_sas_dump_init(void)
{
	struct major_cmd_ctrl *major_cmd = get_major_cmd();

	major_cmd->option_count = 0;
	major_cmd->execute = sas_dump_execute;

	cmd_option_register("-c", "--chipid", true, sas_set_chip_id);
	cmd_option_register("-d", "--dieid", true, sas_set_die_id);
	cmd_option_register("-h", "--help", false, sas_dump_help);
	cmd_option_register("-g", "--global", false, sas_dump_global);
	cmd_option_register("-p", "--phyx", true, sas_dump_phyx);
	cmd_option_register("-b", "--axi", false, sas_dump_axi);
}

HIKP_CMD_DECLARE("sas_dump", "sas reg dump", cmd_sas_dump_init);
