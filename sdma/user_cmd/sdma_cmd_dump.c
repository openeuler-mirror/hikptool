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
#include "hikptdev_plug.h"
#include "tool_lib.h"
#include "tool_cmd.h"
#include "sdma_tools_include.h"
#include "sdma_common.h"
#include "sdma_dump_reg.h"

struct tool_sdma_cmd g_sdma_dump_cmd = {
	.sdma_cmd_type = DUMP_UNKNOWN,
	.chip_id = (uint32_t)(-1),
	.die_id = (uint32_t)(-1),
	.chn_id = (uint32_t)(-1),
};

static int sdma_dump_help(struct major_cmd_ctrl *self, const char *argv)
{
	int ret;

	ret = sdma_dev_check();
	if (ret) {
		printf("The current environment not support this feature!\n");
		return ret;
	}
	HIKP_SET_USED(argv);

	printf("\n  Usage: %s\n", self->cmd_ptr->name);
	printf("\n         %s\n", self->cmd_ptr->help_info);
	printf("    %s, %-25s %s\n", "-c", "--chipid", "please input chip id[x]  first\n");
	printf("    %s, %-25s %s\n", "-d", "--dieid", "please input die id[x]  first\n");
	printf("    %s, %-25s %s\n", "-n", "--chnid", "please input chn id[x]  first\n");
	printf("\n  Options:\n\n");
	printf("    %s, %-25s %s\n", "-h", "--help", "display this help and exit\n");
	printf("    %s, %-25s %s\n", "-s", "--chnstatus", "dump sdma channel status dfx reg\n");
	printf("\tParameter Limitation: -c --chipid and -d --dieid is necessary,");
	printf(" -n --chnid is invalid\n");
	printf("\tUsage: -s -c [chipid] -d [dieid], e.g. -s -c 0 -d 0\n\n");
	printf("    %s, %-25s %s\n", "-p", "--pc", "dump sdma pc channel dfx reg\n");
	printf("\tParameter Limitation: All three parameters are necessary,");
	printf(" the -n --chnid range is limited to 0-31\n");
	printf("\tUsage: -p -c [chipid] -d [dieid] -n [chnid], e.g. -p -c 0 -d 0 -n 31\n\n");
	printf("    %s, %-25s %s\n", "-v", "--vc", "dump sdma vc channel dfx reg\n");
	printf("\tParameter Limitation: All three parameters are necessary,");
	printf(" the -n --chnid range is limited to 0-159\n");
	printf("\tUsage: -v -c [chipid] -d [dieid] -n [chnid], e.g. -v -c 0 -d 0 -n 159\n\n");
	printf("\n");

	return 0;
}

static int sdma_set_id(struct major_cmd_ctrl *self, const char *argv, uint32_t *id)
{
	uint32_t val = 0;
	int ret;

	ret = string_toui(argv, &val);
	if (ret) {
		snprintf(self->err_str, sizeof(self->err_str), "Invalid id.");
		self->err_no = ret;
		return ret;
	}
	*id = val;
	return ret;
}

static int sdma_set_chip_id(struct major_cmd_ctrl *self, const char *argv)
{
	return sdma_set_id(self, argv, &g_sdma_dump_cmd.chip_id);
}

static int sdma_set_die_id(struct major_cmd_ctrl *self, const char *argv)
{
	return sdma_set_id(self, argv, &g_sdma_dump_cmd.die_id);
}

static int sdma_set_chn_id(struct major_cmd_ctrl *self, const char *argv)
{
	return sdma_set_id(self, argv, &g_sdma_dump_cmd.chn_id);
}

static int sdma_dump_chn_status(struct major_cmd_ctrl *self, const char *argv)
{
	HIKP_SET_USED(self);
	HIKP_SET_USED(argv);

	g_sdma_dump_cmd.sdma_cmd_type = DUMP_CHN_STATUS;
	return 0;
}

static int sdma_dump_chn_pc(struct major_cmd_ctrl *self, const char *argv)
{
	HIKP_SET_USED(self);
	HIKP_SET_USED(argv);

	g_sdma_dump_cmd.sdma_cmd_type = DUMP_CHN_PC;
	return 0;
}

static int sdma_dump_chn_vc(struct major_cmd_ctrl *self, const char *argv)
{
	HIKP_SET_USED(self);
	HIKP_SET_USED(argv);

	g_sdma_dump_cmd.sdma_cmd_type = DUMP_CHN_VC;
	return 0;
}

static int sdma_dump_excute_function_call(uint32_t cmd_type)
{
	if (cmd_type != DUMP_UNKNOWN)
		return sdma_reg_dump(&g_sdma_dump_cmd);

	return -EINVAL;
}

static void sdma_dump_execute(struct major_cmd_ctrl *self)
{
	int ret;
	const char *suc_msg[] = {
		"",
		"sdma_dump_chn_status success.",
		"sdma_dump_dfx_pc success.",
		"sdma_dump_dfx_vc success."
	};
	const char *err_msg[] = {
		"sdma_dump failed, unknown cmd type",
		"sdma_dump_chn_status error.",
		"sdma_dump_dfx_pc error.",
		"sdma_dump_dfx_vc error."
	};

	ret = sdma_dump_excute_function_call(g_sdma_dump_cmd.sdma_cmd_type);
	if (ret == 0)
		printf("%s\n", suc_msg[g_sdma_dump_cmd.sdma_cmd_type]);
	else {
		snprintf(self->err_str, sizeof(self->err_str), "%s\n",
			 err_msg[g_sdma_dump_cmd.sdma_cmd_type]);
		self->err_no = ret;
	}
}

static void cmd_sdma_dump_init(void)
{
	struct major_cmd_ctrl *major_cmd = get_major_cmd();

	major_cmd->option_count = 0;
	major_cmd->execute = sdma_dump_execute;

	cmd_option_register("-c", "--chipid", true, sdma_set_chip_id);
	cmd_option_register("-d", "--dieid", true, sdma_set_die_id);
	cmd_option_register("-n", "--chnid", true, sdma_set_chn_id);
	cmd_option_register("-h", "--help", false, sdma_dump_help);
	cmd_option_register("-s", "--chnstatus", false, sdma_dump_chn_status);
	cmd_option_register("-p", "--pc", false, sdma_dump_chn_pc);
	cmd_option_register("-v", "--vc", false, sdma_dump_chn_vc);
}

HIKP_CMD_DECLARE("sdma_dump", "sdma reg dump", cmd_sdma_dump_init);
