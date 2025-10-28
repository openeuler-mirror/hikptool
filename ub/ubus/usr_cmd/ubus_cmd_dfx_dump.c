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
#include "ubus_dfx_dump.h"

struct tool_ubus_dfx_dump_cmd {
	enum ubus_dfx_dump_cmd_type cmd_type;
	uint32_t port_id;
};

struct tool_ubus_dfx_dump_cmd g_ubus_dfx_cmd = {
	.cmd_type = DFX_DUMP_HELP,
	.port_id = (uint32_t)(-1),
};

static int ubus_dfx_dump_help(struct major_cmd_ctrl *self, const char *argv)
{
	HIKP_SET_USED(argv);

	printf("\n  ubus dfx register dump\n");
	printf("\n  Usage: %s --reg [OPTION] [PARA...]\n", self->cmd_ptr->name);
	printf("\n  Options:\n\n");
	printf("    %s, %-25s %s\n", "-h", "--help", "display this help and exit");
	printf("    %s, %-25s %s\n", "-i", "--portidx", "please input port[x] first");
	printf("    %s, %-25s %s\n", "-d", "--dump", "dump important regs");
	printf("\n");

	return 0;
}

static int ub_port_set(struct major_cmd_ctrl *self, const char *argv)
{
	uint32_t val;
	int ret;

	HIKP_SET_USED(self);

	ret = string_toui(argv, &val);
	if (ret) {
		printf("set port id err %d.\n", ret);
		return -EINVAL;
	}

	g_ubus_dfx_cmd.port_id = val;

	return 0;
}

static int ub_dump_set(struct major_cmd_ctrl *self, const char *argv)
{
	HIKP_SET_USED(self);
	HIKP_SET_USED(argv);

	g_ubus_dfx_cmd.cmd_type = DFX_DUMP_SHOW;

	return 0;
}

static void ubus_dfx_dump_execute(struct major_cmd_ctrl *self)
{
	int ret;

	static const char *ubus_dfx_dump_succ_msg[] = {
		"",
		"ubus_dfx_dump success!",
	};
	static const char *ubus_dfx_dump_err_msg[] = {
		"Error : unknown param_type!",
		"ubus_dfx_dump failed!",
	};

	g_ubus_dfx_cmd.cmd_type = DFX_DUMP_SHOW;
	ret = ubus_dfx_dump_show_execute(g_ubus_dfx_cmd.port_id);
	if (ret) {
		/* In error branches, errors are printed and copied, and no check is required. */
		snprintf(self->err_str, sizeof(self->err_str), "%s",
			 ubus_dfx_dump_err_msg[DFX_DUMP_SHOW]);
	} else {
		printf("%s\n", ubus_dfx_dump_succ_msg[DFX_DUMP_SHOW]);
	}

	self->err_no = ret;
}

void cmd_ubus_dfx_init(void)
{
	struct major_cmd_ctrl *major_cmd = get_major_cmd();

	major_cmd->execute = ubus_dfx_dump_execute;

	cmd_option_register("-h", "--help", false, ubus_dfx_dump_help);
	cmd_option_register("-d", "--dump", false, ub_dump_set);
	cmd_option_register("-i", "--portidx", true, ub_port_set);
}
