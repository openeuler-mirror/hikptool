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

struct tool_cxl_cmd g_cxl_cpa_cmd = {
	.cmd_type = CPA_UNKNOWN_TYPE,
	.port_id = (uint32_t)(-1),
};

static int cxl_cpa_help(struct major_cmd_ctrl *self, const char *argv)
{
	printf("  Usage: %s\n", self->cmd_ptr->name);
	printf("         %s\n", self->cmd_ptr->help_info);
	printf("    %s, %-25s %s\n", "-i", "--interface", "please input port[x]  first");
	printf("  Options:\n");
	printf("    %s, %-25s %s\n", "-h", "--help", "display this help and exit");
	printf("    %s, %-25s %s\n", "-e", "--error", "show cpa error info");
	printf("    %s, %-25s %s\n", "-d", "--dump", "dump cpa Key config");
	printf("    %s, %-25s %s\n", "-m", "--mmrg", "show cpa mmrg window config");
	printf("    %s, %-25s %s\n", "-c", "--config", "show cpa config");
	printf("\n");

	return 0;
}

static int cxl_port_id_set(struct major_cmd_ctrl *self, const char *argv)
{
	uint32_t val;
	int ret;

	ret = string_toui(argv, &val);
	if (ret) {
		printf("cxl cpa set port id err %d\n", ret);
		return ret;
	}
	g_cxl_cpa_cmd.port_id = val;

	return 0;
}

static int cxl_cpa_err_status_show(struct major_cmd_ctrl *self, const char *argv)
{
	g_cxl_cpa_cmd.cmd_type = CPA_ERR;

	return 0;
}

static int cxl_cpa_mmrg_show(struct major_cmd_ctrl *self, const char *argv)
{
	g_cxl_cpa_cmd.cmd_type = CPA_MMRG;

	return 0;
}

static int cxl_cpa_dump(struct major_cmd_ctrl *self, const char *argv)
{
	g_cxl_cpa_cmd.cmd_type = CPA_DUMP;

	return 0;
}

static int cxl_cpa_config(struct major_cmd_ctrl *self, const char *argv)
{
	g_cxl_cpa_cmd.cmd_type = CPA_CONFIG;

	return 0;
}

static int cxl_cpa_execute_process(void)
{
	uint32_t port_id = g_cxl_cpa_cmd.port_id;
	uint32_t cmd_type = g_cxl_cpa_cmd.cmd_type;

	if (cmd_type == CPA_ERR || cmd_type == CPA_MMRG ||
		cmd_type == CPA_DUMP || cmd_type == CPA_CONFIG) {
		return cxl_reg_show_execute(port_id, CXL_CPA, cmd_type);
	}

	g_cxl_cpa_cmd.cmd_type = CPA_UNKNOWN_TYPE;
	return -EPERM;
}

static void cxl_cpa_execute(struct major_cmd_ctrl *self)
{
	int ret;
	uint32_t cmd_type;
	static const char *cxl_cpa_succ_msg[] = {
		"",
		"cxl_cpa_err_status_show success!",
		"cxl_cpa_mmrg_cfg_show success!",
		"cxl_cpa_dump_reg_show success!",
		"cxl_cpa_config_show success!"
	};
	static const char *cxl_cpa_err_msg[] = {
		"Error : unknown param_type!",
		"cxl_cpa_err_status_show failed!",
		"cxl_cpa_mmrg_cfg_show failed!",
		"cxl_cpa_dump_reg_show failed!",
		"cxl_cpa_config_show failed!"
	};

	ret = cxl_cpa_execute_process();
	cmd_type = g_cxl_cpa_cmd.cmd_type;
	if (ret) {
		/* In error branches, errors are printed and copied, and no check is required. */
		snprintf(self->err_str, sizeof(self->err_str), "%s\n", cxl_cpa_err_msg[cmd_type]);
	} else {
		printf("%s\n", cxl_cpa_succ_msg[cmd_type]);
	}

	self->err_no = ret;
}

static void cmd_cxl_cpa_init(void)
{
	struct major_cmd_ctrl *major_cmd = get_major_cmd();

	major_cmd->option_count = 0;
	major_cmd->execute = cxl_cpa_execute;

	cmd_option_register("-h", "--help", false, cxl_cpa_help);
	cmd_option_register("-i", "--interface", true, cxl_port_id_set);
	cmd_option_register("-e", "--error", false, cxl_cpa_err_status_show);
	cmd_option_register("-d", "--dump", false, cxl_cpa_dump);
	cmd_option_register("-c", "--config", false, cxl_cpa_config);
	cmd_option_register("-m", "--mmrg", false, cxl_cpa_mmrg_show);
}

HIKP_CMD_DECLARE("cxl_cpa", "cxl_cpa maininfo", cmd_cxl_cpa_init);
