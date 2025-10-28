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
#include "ubus_trace_show.h"

struct tool_ubus_trace_cmd {
	enum ubus_trace_show_cmd_type cmd_type;
	uint32_t port_id;
};

static struct tool_ubus_trace_cmd g_ubus_trace_cmd = {
	.cmd_type = TRACE_HELP,
	.port_id = (uint32_t)(-1),
};

static int ubus_trace_help(struct major_cmd_ctrl *self, const char *argv)
{
	HIKP_SET_USED(argv);

	printf("\n  ubus trace info dump\n");
	printf("\n  Usage: %s --trace [OPTION] [PARA...]\n", self->cmd_ptr->name);
	printf("\n  Options:\n");
	printf("    %s, %-25s %s\n", "-i", "--portidx", "input ub port index");
	printf("    %s, %-25s %s\n", "-s", "--show", "show the trace info");
	printf("    %s, %-25s %s\n", "-h", "--help", "display this help");
	printf("\n");

	return 0;
}

static int ubus_port_id_set(struct major_cmd_ctrl *self, const char *argv)
{
	uint32_t val;
	int ret;

	HIKP_SET_USED(self);

	ret = string_toui(argv, &val);
	if (ret) {
		printf("ubus set port id err %d\n", ret);
		return ret;
	}
	g_ubus_trace_cmd.port_id = val;

	return 0;
}

static int ubus_trace_show_set(struct major_cmd_ctrl *self, const char *argv)
{
	HIKP_SET_USED(self);
	HIKP_SET_USED(argv);

	g_ubus_trace_cmd.cmd_type = TRACE_SHOW;

	return 0;
}

static int ubus_trace_execute_process(void)
{
	uint32_t cmd_type = g_ubus_trace_cmd.cmd_type;
	uint32_t port_id = g_ubus_trace_cmd.port_id;

	if (cmd_type == TRACE_SHOW)
		return ubus_trace_show_execute(port_id);

	return -EPERM;
}

static void ubus_trace_execute(struct major_cmd_ctrl *self)
{
	uint32_t cmd_type;
	int ret;

	static const char *ubus_trace_show_succ_msg[] = {
		"",
		"ubus_trace_show success!",
	};
	static const char *ubus_trace_show_err_msg[] = {
		"Error : unknown param_type!",
		"ubus_trace_show failed!",
	};

	ret = ubus_trace_execute_process();
	cmd_type = g_ubus_trace_cmd.cmd_type;
	if (ret) {
		/* In error branches, errors are printed and copied, and no check is required. */
		snprintf(self->err_str, sizeof(self->err_str), "%s",
			 ubus_trace_show_err_msg[cmd_type]);
	} else {
		printf("%s\n", ubus_trace_show_succ_msg[cmd_type]);
	}

	self->err_no = ret;
}

void cmd_ubus_trace_init(void)
{
	struct major_cmd_ctrl *major_cmd = get_major_cmd();

	major_cmd->execute = ubus_trace_execute;
	cmd_option_register("-h", "--help", false, ubus_trace_help);
	cmd_option_register("-i", "--portidx", true, ubus_port_id_set);
	cmd_option_register("-s", "--show", false, ubus_trace_show_set);
}
