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

struct tool_cxl_cmd g_cxl_rcrb_cmd = {
	.cmd_type = CXL_RCRB_UNKNOWN_TYPE,
	.port_id = (uint32_t)(-1),
};

static int cxl_rcrb_help(struct major_cmd_ctrl *self, const char *argv)
{
	HIKP_SET_USED(argv);

	printf("  Usage: %s\n", self->cmd_ptr->name);
	printf("         %s\n", self->cmd_ptr->help_info);
	printf("    %s, %-25s %s\n", "-i", "--interface", "please input port[x]  first");
	printf("  Options:\n");
	printf("    %s, %-25s %s\n", "-h", "--help", "display this help and exit");
	printf("    %s, %-25s %s\n", "-l", "--link_info", "show cxl.io and cxl.mem link info");
	printf("    %s, %-25s %s\n", "-p", "--pci_hdr_info", "show cxl rcrb cfg space header");
	printf("    %s, %-25s %s\n", "-d", "--dump", "dump cxl rcrb reg");
	printf("\n");

	return 0;
}

static int cxl_rcrb_port_id_set(struct major_cmd_ctrl *self, const char *argv)
{
	uint32_t val;
	int ret;

	HIKP_SET_USED(self);

	ret = string_toui(argv, &val);
	if (ret) {
		printf("cxl rcrb set port id err %d\n", ret);
		return ret;
	}
	g_cxl_rcrb_cmd.port_id = val;

	return 0;
}

static int cxl_rcrb_link_status(struct major_cmd_ctrl *self, const char *argv)
{
	HIKP_SET_USED(self);
	HIKP_SET_USED(argv);

	g_cxl_rcrb_cmd.cmd_type = CXL_RCRB_LINK;

	return 0;
}

static int cxl_rcrb_header_info(struct major_cmd_ctrl *self, const char *argv)
{
	HIKP_SET_USED(self);
	HIKP_SET_USED(argv);

	g_cxl_rcrb_cmd.cmd_type = CXL_RCRB_HDR;

	return 0;
}

static int cxl_rcrb_dump(struct major_cmd_ctrl *self, const char *argv)
{
	HIKP_SET_USED(self);
	HIKP_SET_USED(argv);

	g_cxl_rcrb_cmd.cmd_type = CXL_RCRB_DUMP;

	return 0;
}

static int cxl_rcrb_execute_process(void)
{
	uint32_t port_id = g_cxl_rcrb_cmd.port_id;
	uint32_t cmd_type = g_cxl_rcrb_cmd.cmd_type;

	if (cmd_type == CXL_RCRB_LINK || cmd_type == CXL_RCRB_HDR ||
		cmd_type == CXL_RCRB_DUMP) {
		return cxl_reg_show_execute(port_id, CXL_RCRB, cmd_type);
	}

	g_cxl_rcrb_cmd.cmd_type = CXL_RCRB_UNKNOWN_TYPE;
	return -EPERM;
}

static void cxl_rcrb_execute(struct major_cmd_ctrl *self)
{
	int ret;
	uint32_t cmd_type;
	const char *cxl_rcrb_succ_msg[] = {
		"",
		"cxl_rcrb_link_status_show success!",
		"cxl_rcrb_cfg_header_show success!",
		"cxl_rcrb_dump_show success!"
	};
	const char *cxl_rcrb_err_msg[] = {
		"Error : unknown param_type!",
		"cxl_rcrb_link_status_show failed!",
		"cxl_rcrb_cfg_header_show failed!",
		"cxl_rcrb_dump_show failed!"
	};

	ret = cxl_rcrb_execute_process();
	cmd_type = g_cxl_rcrb_cmd.cmd_type;
	if (ret) {
		/* In error branches, errors are printed and copied, and no check is required. */
		snprintf(self->err_str, sizeof(self->err_str), "%s\n", cxl_rcrb_err_msg[cmd_type]);
	} else {
		printf("%s\n", cxl_rcrb_succ_msg[cmd_type]);
	}

	self->err_no = ret;
}

static void cmd_cxl_rcrb_init(void)
{
	struct major_cmd_ctrl *major_cmd = get_major_cmd();

	major_cmd->option_count = 0;
	major_cmd->execute = cxl_rcrb_execute;

	cmd_option_register("-h", "--help", false, cxl_rcrb_help);
	cmd_option_register("-i", "--interface", true, cxl_rcrb_port_id_set);
	cmd_option_register("-l", "--link_info", false, cxl_rcrb_link_status);
	cmd_option_register("-p", "--pci_hdr_info", false, cxl_rcrb_header_info);
	cmd_option_register("-d", "--dump", false, cxl_rcrb_dump);
}

HIKP_CMD_DECLARE("cxl_rcrb", "cxl_rcrb maininfo", cmd_cxl_rcrb_init);
