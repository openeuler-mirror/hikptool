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
#include "sas_read_errcode.h"

static int sas_errcode_help(struct major_cmd_ctrl *self, const char *argv)
{
	printf("\n  Usage: %s\n", self->cmd_ptr->name);
	printf("\n         %s\n", self->cmd_ptr->help_info);
	printf("    %s, %-25s %s\n", "-c", "--chipid", "please input chip id[x]  first\n");
	printf("    %s, %-25s %s\n", "-d", "--dieid", "please input die id[x]  first\n");
	printf("\n  Options:\n\n");
	printf("    %s, %-25s %s\n", "-h", "--help", "display this help and exit\n");
	printf("    %s, %-25s %s\n", "-t", "--type", "read error code of 8 phys\n");
	printf("\n");

	return 0;
}

static int sas_errcode_one(struct major_cmd_ctrl *self, char const *argv)
{
	int ret;
	uint32_t val = 0;

	ret = string_toui(argv, &val);
	if (ret || val >= SAS_MAX_ERR_NUM) {
		snprintf(self->err_str, sizeof(self->err_str), "Invalid error code type.");
		self->err_no = ret;
		return -EINVAL;
	}

	(void)sas_set_cmd_type(val);
	return 0;
}

static int sas_errcode_excute_funs_call(uint32_t cmd_type)
{
	if (cmd_type != SAS_UNKNOW_CMD)
		return sas_errcode_read(sas_get_cmd_p());

	return -EINVAL;
}

static void sas_errcode_execute(struct major_cmd_ctrl *self)
{
	int ret, cmd;
	const char *suc_msg[] = {
		"sas_errcode_all success.",
		"sas_errcode_dws_lost success.",
		"sas_errcode_reset_prob success.",
		"sas_errcode_crc_fail success.",
		"sas_errcode_open_rej success.",
	};
	const char *err_msg[] = {
		"sas_errcode_all error.",
		"sas_errcode_dws_lost error.",
		"sas_errcode_reset_prob error.",
		"sas_errcode_crc_fail error.",
		"sas_errcode_open_rej error.",
		"sas_errcode failed, unknown type",
	};

	cmd = sas_get_cmd_type();
	ret = sas_errcode_excute_funs_call(cmd);
	(void)sas_set_cmd_type(SAS_UNKNOW_CMD);
	if (ret == 0) {
		printf("%s\n", suc_msg[cmd]);
	} else {
		if (cmd == SAS_UNKNOW_CMD)
			cmd = ERRCODE_UNKNOWN_TYPE;
		snprintf(self->err_str, sizeof(self->err_str), "%s\n", err_msg[cmd]);
		self->err_no = ret;
	}
}

static void cmd_sas_errcode_init(void)
{
	struct major_cmd_ctrl *major_cmd = get_major_cmd();

	major_cmd->option_count = 0;
	major_cmd->execute = sas_errcode_execute;

	cmd_option_register("-h", "--help", false, sas_errcode_help);
	cmd_option_register("-c", "--chipid", true, sas_set_chip_id);
	cmd_option_register("-d", "--dieid", true, sas_set_die_id);
	cmd_option_register("-t", "--type", true, sas_errcode_one);
}

HIKP_CMD_DECLARE("sas_errcode", "sas read error code", cmd_sas_errcode_init);
