/*
 * Copyright (c) 2025 Hisilicon Technologies Co., Ltd.
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
#include "ras_tools_include.h"
#include "ras_dump_data.h"

struct tool_ras_cmd g_ras_dump_cmd = {
	.ras_cmd_type = DUMP_DFX,
};

static int ras_dump_help(struct major_cmd_ctrl *self, const char *argv)
{
	HIKP_SET_USED(argv);

	printf("\n  Usage: %s\n", self->cmd_ptr->name);
	printf("\n         %s\n", self->cmd_ptr->help_info);
	printf("\n  Options:\n\n");
	printf("    %s, %-25s %s\n", "-h", "--help", "display this help and exit\n");
	printf("    %s, %-25s %s\n", "-c", "--clear", "clearing memory dfx data\n");
	printf("\n");

	return 0;
}

static enum ras_dump_cmd_type ras_get_cmd_type(void)
{
	return g_ras_dump_cmd.ras_cmd_type;
}

static void ras_set_cmd_type(enum ras_dump_cmd_type type)
{
	g_ras_dump_cmd.ras_cmd_type = type;
}

static int ras_set_clear(struct major_cmd_ctrl *self, const char *argv)
{
	HIKP_SET_USED(self);
	HIKP_SET_USED(argv);

	ras_set_cmd_type(DUMP_CLEAR);
	return 0;
}

static int ras_dump_execute_process(void)
{
	if (ras_get_cmd_type() == DUMP_DFX)
		return ras_data_dump(&g_ras_dump_cmd);
	else if (ras_get_cmd_type() == DUMP_CLEAR)
		return ras_data_clear(&g_ras_dump_cmd);
	else
		return -EINVAL;
}

static void ras_dump_execute(struct major_cmd_ctrl *self)
{
	int ret;
	const char *suc_msg[] = {
		"ras dfx data dump success.",
		"ras dfx data clear success."
	};
	const char *err_msg[] = {
		"ras dfx data dump error.",
		"ras dfx data clear error."
	};

	ret = ras_dump_execute_process();
	if (ret == 0) {
		printf("%s\n", suc_msg[ras_get_cmd_type()]);
	} else {
		snprintf(self->err_str, sizeof(self->err_str), "%s\n",
			 err_msg[ras_get_cmd_type()]);
		self->err_no = ret;
	}
}

static void cmd_ras_dump_init(void)
{
	struct major_cmd_ctrl *major_cmd = get_major_cmd();

	major_cmd->option_count = 0;
	major_cmd->execute = ras_dump_execute;

	cmd_option_register("-c", "--clear", false, ras_set_clear);
	cmd_option_register("-h", "--help", false, ras_dump_help);
}

HIKP_CMD_DECLARE("bbox_export", "export black box data to file", cmd_ras_dump_init);
