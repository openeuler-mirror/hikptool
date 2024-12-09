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
#include "sas_read_dqe.h"

static int sas_dqe_help(struct major_cmd_ctrl *self, const char *argv)
{
	printf("\n  Usage: %s\n", self->cmd_ptr->name);
	printf("\n         %s\n", self->cmd_ptr->help_info);
	printf("    %s, %-25s %s\n", "-c", "--chipid", "please input chip id[x]  first\n");
	printf("    %s, %-25s %s\n", "-d", "--dieid", "please input die id[x]  first\n");
	printf("    %s, %-25s %s\n", "-q", "--queue", "please input queue id[x]  first\n");
	printf("\n  Options:\n\n");
	printf("    %s, %-25s %s\n", "-h", "--help", "display this help and exit\n");
	printf("    %s, %-25s %s\n", "-i", "--info", "display the dqe detail information\n");
	printf("\n");

	return 0;
}

static int sas_dqe_info(struct major_cmd_ctrl *self, const char *argv)
{
	int ret;

	(void)sas_set_cmd_type(DQE_INFO);
	ret = sas_set_dqe_id(self, argv);
	return ret;
}

static int sas_set_queue_id(struct major_cmd_ctrl *self, const char *argv)
{
	int ret;

	ret = sas_set_que_id(self, argv);
	if (ret || sas_get_que_id() >= SAS_QUEUE_NUM) {
		snprintf(self->err_str, sizeof(self->err_str), "Invalid queue id.");
		self->err_no = ret;
		return -EINVAL;
	}

	return 0;
}

static int sas_dqe_excute_funs_call(uint32_t cmd_type)
{
	if ((cmd_type != SAS_UNKNOW_CMD) && (sas_get_que_id() != (uint32_t)(-1)))
		return sas_dqe(sas_get_cmd_p());

	return -EINVAL;
}

static void sas_dqe_execute(struct major_cmd_ctrl *self)
{
	int ret, cmd;
	const char *suc_msg[] = {
		"sas_dqe_info success.",
	};
	const char *err_msg[] = {
		"sas_dqe_info error.",
		"sas_dqe failed, unknown type",
	};

	cmd = sas_get_cmd_type();
	ret = sas_dqe_excute_funs_call(cmd);
	(void)sas_set_cmd_type(SAS_UNKNOW_CMD);
	if (ret == 0) {
		printf("%s\n", suc_msg[cmd]);
	} else {
		if (cmd == SAS_UNKNOW_CMD)
			cmd = DQE_UNKNOWN_TYPE;
		snprintf(self->err_str, sizeof(self->err_str), "%s\n", err_msg[cmd]);
		self->err_no = ret;
	}
}

static void cmd_sas_dqe_init(void)
{
	struct major_cmd_ctrl *major_cmd = get_major_cmd();

	major_cmd->option_count = 0;
	major_cmd->execute = sas_dqe_execute;

	cmd_option_register("-c", "--chipid", true, sas_set_chip_id);
	cmd_option_register("-d", "--dieid", true, sas_set_die_id);
	cmd_option_register("-h", "--help", false, sas_dqe_help);
	cmd_option_register("-q", "--queue", true, sas_set_queue_id);
	cmd_option_register("-i", "--info", true, sas_dqe_info);
}

HIKP_CMD_DECLARE("sas_dqe", "sas dqe information ", cmd_sas_dqe_init);
