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
#include "tool_lib.h"
#include "tool_cmd.h"
#include "cxl_tool_include.h"
#include "cxl_feature.h"
#include "hikptdev_plug.h"

struct tool_cxl_cmd g_cxl_dl_cmd = {
	.cmd_type = CXL_DL_UNKNOWN_TYPE,
	.port_id = (uint32_t)(-1),
};

static int cxl_dl_help(struct major_cmd_ctrl *self, const char *argv)
{
	printf("  Usage: %s\n", self->cmd_ptr->name);
	printf("         %s\n", self->cmd_ptr->help_info);
	printf("    %s, %-25s %s\n", "-i", "--interface", "please input port[x]  first");
	printf("  Options:\n");
	printf("    %s, %-25s %s\n", "-h", "--help", "display this help and exit");
	printf("    %s, %-25s %s\n", "-f", "--fsm_state", "show cxl dl fsm state");
	printf("    %s, %-25s %s\n", "-s", "--dfx_status", "show cxl dl dfx count");
	printf("    %s, %-25s %s\n", "-d", "--dump", "dump cxl dl Key reg");
	printf("    %s, %-25s %s\n", "-e", "--error", "show cxl dl err info");
	printf("\n");

	return 0;
}

static int cxl_dl_port_id_set(struct major_cmd_ctrl *self, const char *argv)
{
	uint32_t val;
	int ret;

	ret = string_toui(argv, &val);
	if (ret) {
		printf("cxl dl set port id err %d\n", ret);
		return ret;
	}
	g_cxl_dl_cmd.port_id = val;

	return 0;
}

static int cxl_dl_err_status(struct major_cmd_ctrl *self, const char *argv)
{
	g_cxl_dl_cmd.cmd_type = CXL_DL_ERR;

	return 0;
}

static int cxl_dl_dump(struct major_cmd_ctrl *self, const char *argv)
{
	g_cxl_dl_cmd.cmd_type = CXL_DL_DUMP;

	return 0;
}

static int cxl_dl_dfx(struct major_cmd_ctrl *self, const char *argv)
{
	g_cxl_dl_cmd.cmd_type = CXL_DL_DFX;

	return 0;
}

static int cxl_dl_fsm_state(struct major_cmd_ctrl *self, const char *argv)
{
	g_cxl_dl_cmd.cmd_type = CXL_DL_FSM_STATE;

	return 0;
}

static int cxl_dl_execute_process(void)
{
	uint32_t port_id = g_cxl_dl_cmd.port_id;
	uint32_t cmd_type = g_cxl_dl_cmd.cmd_type;

	if (cmd_type == CXL_DL_FSM_STATE || cmd_type == CXL_DL_DFX ||
		cmd_type == CXL_DL_DUMP || cmd_type == CXL_DL_ERR) {
		return cxl_reg_show_execute(port_id, CXL_DL, cmd_type);
	}

	g_cxl_dl_cmd.cmd_type = CXL_DL_UNKNOWN_TYPE;
	return -EPERM;
}

static void cxl_dl_execute(struct major_cmd_ctrl *self)
{
	int ret;
	uint32_t cmd_type;
	static const char *cxl_dl_succ_msg[] = {
		"",
		"cxl_dl_fsm_state_show success!",
		"cxl_dl_dfx_show success!",
		"cxl_dl_dump_show success!",
		"cxl_dl_error_info success!"
	};
	static const char *cxl_dl_err_msg[] = {
		"Error : unknown param_type!",
		"cxl_dl_fsm_state_show failed!",
		"cxl_dl_dfx_show failed!",
		"cxl_dl_dump_show failed!",
		"cxl_dl_error_info failed!"
	};

	ret = cxl_dl_execute_process();
	cmd_type = g_cxl_dl_cmd.cmd_type;
	if (ret) {
		/* In error branches, errors are printed and copied, and no check is required. */
		snprintf(self->err_str, sizeof(self->err_str), "%s\n", cxl_dl_err_msg[cmd_type]);
	} else {
		printf("%s\n", cxl_dl_succ_msg[cmd_type]);
	}

	self->err_no = ret;
}

static void cmd_cxl_dl_init(void)
{
	struct major_cmd_ctrl *major_cmd = get_major_cmd();

	major_cmd->option_count = 0;
	major_cmd->execute = cxl_dl_execute;

	cmd_option_register("-h", "--help", false, cxl_dl_help);
	cmd_option_register("-i", "--interface", true, cxl_dl_port_id_set);
	cmd_option_register("-f", "--fsm_state", false, cxl_dl_fsm_state);
	cmd_option_register("-s", "--dfx_status", false, cxl_dl_dfx);
	cmd_option_register("-d", "--dump", false, cxl_dl_dump);
	cmd_option_register("-e", "--error", false, cxl_dl_err_status);
}

HIKP_CMD_DECLARE("cxl_dl", "cxl_dl maininfo", cmd_cxl_dl_init);
