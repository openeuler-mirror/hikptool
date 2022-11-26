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
#include "sas_analy_queue.h"

static int sas_anadq_help(struct major_cmd_ctrl *self, const char *argv)
{
	printf("\n  Usage: %s\n", self->cmd_ptr->name);
	printf("\n         %s\n", self->cmd_ptr->help_info);
	printf("    %s, %-25s %s\n", "-c", "--chipid", "please input chip id[x]  first\n");
	printf("    %s, %-25s %s\n", "-d", "--dieid", "please input die id[x]  first\n");
	printf("\n  Options:\n\n");
	printf("    %s, %-25s %s\n", "-h", "--help", "display this help and exit\n");
	printf("    %s, %-25s %s\n", "-p", "--pointer", "dispaly dq queue read/write pointer\n");
	printf("    %s, %-25s %s\n", "-s", "--number", "dispaly dq number\n");
	printf("\n");

	return 0;
}

static int sas_anadq_prt(struct major_cmd_ctrl *self, const char *argv)
{
	return sas_set_cmd_type(ANADQ_PRT);
}

static int sas_anadq_num(struct major_cmd_ctrl *self, const char *argv)
{
	return sas_set_cmd_type(ANADQ_NUM);
}

static int sas_anadq_excute_funs_call(uint32_t cmd_type)
{
	if (cmd_type != SAS_UNKNOW_CMD)
		return sas_analy_cmd(sas_get_cmd_p());

	return -1;
}

static void sas_anadq_execute(struct major_cmd_ctrl *self)
{
	int ret, cmd;
	const char *suc_msg[] = {
		"sas_analy_dq_prt success.",
		"sas_analy_dq_num success.",
	};
	const char *err_msg[] = {
		"sas_analy_dq_prt error.",
		"sas_analy_dq_num error.",
		"sas_analy_dq failed, unknown type",
	};

	cmd = sas_get_cmd_type();
	ret = sas_anadq_excute_funs_call(cmd);
	(void)sas_set_cmd_type(SAS_UNKNOW_CMD);
	if (ret == 0) {
		printf("%s\n", suc_msg[cmd]);
	} else {
		if (cmd == SAS_UNKNOW_CMD)
			cmd = ANADQ_UNKNOWN_TYPE;
		snprintf(self->err_str, sizeof(self->err_str), "%s\n", err_msg[cmd]);
		self->err_no = ret;
	}
}

static void cmd_sas_anadq_init(void)
{
	struct major_cmd_ctrl *major_cmd = get_major_cmd();

	major_cmd->option_count = 0;
	major_cmd->execute = sas_anadq_execute;

	cmd_option_register("-c", "--chipid", true, sas_set_chip_id);
	cmd_option_register("-d", "--dieid", true, sas_set_die_id);
	cmd_option_register("-h", "--help", false, sas_anadq_help);
	cmd_option_register("-p", "--pointer", false, sas_anadq_prt);
	cmd_option_register("-s", "--number", false, sas_anadq_num);
}

HIKP_CMD_DECLARE("sas_anadq", "sas analysis dq queue ", cmd_sas_anadq_init);
