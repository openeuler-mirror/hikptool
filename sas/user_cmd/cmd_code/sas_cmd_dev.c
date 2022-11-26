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
#include "sas_read_dev.h"

static int sas_dev_help(struct major_cmd_ctrl *self, const char *argv)
{
	printf("\n  Usage: %s\n", self->cmd_ptr->name);
	printf("\n         %s\n", self->cmd_ptr->help_info);
	printf("    %s, %-25s %s\n", "-c", "--chipid", "please input chip id[x]  first\n");
	printf("    %s, %-25s %s\n", "-d", "--dieid", "please input die id[x]  first\n");
	printf("\n  Options:\n\n");
	printf("    %s, %-25s %s\n", "-h", "--help", "display this help and exit\n");
	printf("    %s, %-25s %s\n", "-l", "--link", "dispaly device type and speed\n");
	printf("    %s, %-25s %s\n", "-i", "--info", "dispaly the device detail information\n");
	printf("\n");

	return 0;
}

static int sas_dev_link(struct major_cmd_ctrl *self, const char *argv)
{
	return sas_set_cmd_type(DEV_LINK);
}

static int sas_dev_info(struct major_cmd_ctrl *self, const char *argv)
{
	int ret;

	(void)sas_set_cmd_type(DEV_INFO);
	ret = sas_set_dev_id(self, argv);
	if (ret || sas_get_dev_id() >= MAX_DEVICE_NUM) {
		printf("device id is too large(>=%d)\n", MAX_DEVICE_NUM);
		return -EINVAL;
	}
	return ret;
}

static int sas_dev_excute_funs_call(uint32_t cmd_type)
{
	if (cmd_type != SAS_UNKNOW_CMD)
		return sas_dev(sas_get_cmd_p());

	return -EINVAL;
}

static void sas_dev_execute(struct major_cmd_ctrl *self)
{
	int ret, cmd;
	const char *suc_msg[] = {
		"sas_dev_link success.",
		"sas_dev_info success.",
	};
	const char *err_msg[] = {
		"sas_dev_link error.",
		"sas_dev_info error.",
		"sas_dev failed, unknown type",
	};

	cmd = sas_get_cmd_type();
	ret = sas_dev_excute_funs_call(cmd);
	(void)sas_set_cmd_type(SAS_UNKNOW_CMD);
	if (ret == 0) {
		printf("%s\n", suc_msg[cmd]);
	} else {
		if (cmd == SAS_UNKNOW_CMD)
			cmd = DEV_UNKNOWN_TYPE;
		snprintf(self->err_str, sizeof(self->err_str), "%s\n", err_msg[cmd]);
		self->err_no = ret;
	}
}

static void cmd_sas_dev_init(void)
{
	struct major_cmd_ctrl *major_cmd = get_major_cmd();

	major_cmd->option_count = 0;
	major_cmd->execute = sas_dev_execute;

	cmd_option_register("-c", "--chipid", true, sas_set_chip_id);
	cmd_option_register("-d", "--dieid", true, sas_set_die_id);
	cmd_option_register("-h", "--help", false, sas_dev_help);
	cmd_option_register("-l", "--link", false, sas_dev_link);
	cmd_option_register("-i", "--info", true, sas_dev_info);
}

HIKP_CMD_DECLARE("sas_dev", "sas device information ", cmd_sas_dev_init);
