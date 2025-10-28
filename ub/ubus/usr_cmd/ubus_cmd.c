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

#include "tool_lib.h"
#include "tool_cmd.h"
#include "hikptdev_plug.h"
#include "ubus_common.h"

static bool g_sub_command_set = false;

static int ubus_cmd_help(struct major_cmd_ctrl *major_cmd, const char *argv)
{
	HIKP_SET_USED(argv);

	printf("  %s\n", major_cmd->cmd_ptr->help_info);
	printf("\n  Usage: hikptool %s [sub command] [OPTION] [PARA...]\n",
	       major_cmd->cmd_ptr->name);
	printf("\n  Options:\n\n");
	printf("    %-25s %s\n", "[sub command]", "sub command for ubus");
	printf("         usage: choose one sub command at a time.\n");
	printf("    %s, %-25s %s\n", "-h", "--help", "display this help and exit");
	printf("\n  Sub commands: \n");
	printf("    %s, %-25s\n", "--msgq", "ubus msgq dump");
	printf("    %s, %-25s\n", "--reg", "ubus dfx register dump");
	printf("    %s, %-25s\n", "--trace", "ubus trace info");
	printf("    %s, %-25s\n", "--err", "ubus err info");
	printf("\n  sub command help info use [sub command] -h \n");
	printf("\n");

	return 0;
}

static inline void ubus_sub_cmd_init(struct major_cmd_ctrl *major_cmd,
				    void (sub_cmd_init)(void))
{
	g_sub_command_set = true;
	// reset -h option
	if (major_cmd->option_count > 0)
		major_cmd->option_count--;

	sub_cmd_init();
}

static int ubus_msgq_dump_set(struct major_cmd_ctrl *major_cmd, const char *argv)
{
	HIKP_SET_USED(argv);

	if (g_sub_command_set) {
		printf("ubus sub command: msgq dump, repeated or conflict.\n");
		return -EINVAL;
	}

	ubus_sub_cmd_init(major_cmd, cmd_ubus_msgq_dump_init);

	return 0;
}

static int ubus_dfx_set(struct major_cmd_ctrl *major_cmd, const char *argv)
{
	HIKP_SET_USED(argv);

	if (g_sub_command_set) {
		printf("ubus sub command: dfx register, repeated or conflict.\n");
		return -EINVAL;
	}

	ubus_sub_cmd_init(major_cmd, cmd_ubus_dfx_init);

	return 0;
}

static int ubus_trace_set(struct major_cmd_ctrl *major_cmd, const char *argv)
{
	HIKP_SET_USED(argv);

	if (g_sub_command_set) {
		printf("ubus sub command: trace, repeated or conflict.\n");
		return -EINVAL;
	}

	ubus_sub_cmd_init(major_cmd, cmd_ubus_trace_init);

	return 0;
}

static int ubus_err_set(struct major_cmd_ctrl *major_cmd, const char *argv)
{
	HIKP_SET_USED(argv);

	if (g_sub_command_set) {
		printf("ubus sub command: err info, repeated or conflict.\n");
		return -EINVAL;
	}

	ubus_sub_cmd_init(major_cmd, cmd_ubus_err_init);

	return 0;
}

static void ubus_cmd_init(void)
{
	struct major_cmd_ctrl *major_cmd = get_major_cmd();

	major_cmd->option_count = 0;
	cmd_option_register("", "--msgq", false, ubus_msgq_dump_set);
	cmd_option_register("", "--reg",  false, ubus_dfx_set);
	cmd_option_register("", "--trace", false, ubus_trace_set);
	cmd_option_register("", "--err",  false, ubus_err_set);
	cmd_option_register("-h", "--help", false, ubus_cmd_help);
}

HIKP_CMD_DECLARE("ubus", "ubus tools for dfx", ubus_cmd_init);
