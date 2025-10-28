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
#include "tool_lib.h"
#include "tool_cmd.h"
#include "hikptdev_plug.h"
#include "ubus_common.h"
#include "ubus_err_info.h"

struct tool_ubus_err_cmd {
	enum ubus_err_info_cmd_type cmd_type;
	uint32_t port_id;
};

static struct tool_ubus_err_cmd g_ubus_err_cmd = {
	.cmd_type = ERR_INFO_HELP,
	.port_id = (uint32_t)(-1),
};

static int ubus_err_help(struct major_cmd_ctrl *self, const char *argv)
{
	HIKP_SET_USED(argv);

	printf("\n  ubus err info\n");
	printf("\n  Usage: %s --err [OPTION] [PARA...]\n", self->cmd_ptr->name);
	printf("\n  Options:\n");
	printf("    %s, %-25s %s\n", "-i", "--portidx", "input ubus port index");
	printf("    %s, %-25s %s\n", "-ec", "--clear", "clear the err status");
	printf("    %s, %-25s %s\n", "-es", "--show", "show the err info");
	printf("    %s, %-25s %s\n", "-h", "--help", "display this help");
	printf("\n");

	return 0;
}

static int ubus_err_cmd_show_set(struct major_cmd_ctrl *self, const char *argv)
{
	HIKP_SET_USED(self);
	HIKP_SET_USED(argv);

	g_ubus_err_cmd.cmd_type = ERR_INFO_SHOW;

	return 0;
}

static int ubus_err_cmd_clear_set(struct major_cmd_ctrl *self, const char *argv)
{
	HIKP_SET_USED(self);
	HIKP_SET_USED(argv);

	g_ubus_err_cmd.cmd_type = ERR_INFO_CLEAR;

	return 0;
}

static int ubus_err_port_id_set(struct major_cmd_ctrl *self, const char *argv)
{
	uint32_t val;
	int ret;

	HIKP_SET_USED(self);

	ret = string_toui(argv, &val);
	if (ret) {
		printf("ubus set port id err %d\n", ret);
		return ret;
	}
	g_ubus_err_cmd.port_id = val;

	return 0;
}

static int ubus_err_execute_process(void)
{
	uint32_t cmd_type = g_ubus_err_cmd.cmd_type;
	uint32_t port_id = g_ubus_err_cmd.port_id;

	if (cmd_type == ERR_INFO_SHOW)
		return ubus_err_show_execute(port_id);

	if (cmd_type == ERR_INFO_CLEAR)
		return ubus_err_clear_execute(port_id);

	return -EPERM;
}

static void ubus_err_execute(struct major_cmd_ctrl *self)
{
	uint32_t cmd_type;
	int ret;

	static const char *ubus_err_cmd_succ_msg[] = {
		"",
		"ubus_err_show success!",
		"ubus_err_clear success!",
	};
	static const char *ubus_err_cmd_err_msg[] = {
		"Error : unknown param_type!",
		"ubus_err_show failed!",
		"ubus_err_clear failed!",
	};

	ret = ubus_err_execute_process();
	cmd_type = g_ubus_err_cmd.cmd_type;
	if (ret) {
		/* In error branches, errors are printed and copied, and no check is required. */
		snprintf(self->err_str, sizeof(self->err_str), "%s",
			 ubus_err_cmd_err_msg[cmd_type]);
	} else {
		printf("%s\n", ubus_err_cmd_succ_msg[cmd_type]);
	}

	self->err_no = ret;
}

void cmd_ubus_err_init(void)
{
	struct major_cmd_ctrl *major_cmd = get_major_cmd();

	major_cmd->execute = ubus_err_execute;
	cmd_option_register("-h", "--help", false, ubus_err_help);
	cmd_option_register("-i", "--portidx", true, ubus_err_port_id_set);
	cmd_option_register("-ec", "--clear", false, ubus_err_cmd_clear_set);
	cmd_option_register("-es", "--show", false, ubus_err_cmd_show_set);
}
