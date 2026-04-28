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

#include "hikp_imp_cmd.h"
#include <unistd.h>
#include <inttypes.h>
#include "hikp_imp_log.h"
#include "hikp_imp_reg_dump.h"

static struct imp_cmd_cfg g_imp_cmd_cfg = { 0 };

static int hikp_imp_cmd_help(struct major_cmd_ctrl *self, const char *argv)
{
	HIKP_SET_USED(argv);

	printf("\n  Usage: %s %s\n", self->cmd_ptr->name, "<function> -c <chip> -d <die>");
	printf("\n         %s\n", self->cmd_ptr->help_info);
	printf("  Options:\n\n");
	printf("    %s, %-25s %s\n", "-h", "--help", "display this help and exit");
	printf("    %s, %-25s %s\n", "-l", "--log", "dump imp log function");
	printf("    %s, %-25s %s\n", "-D", "--dump", "dump imp dfx register");
	printf("    %s, %-25s %s\n", "-c", "--chip=<chip>", "chip id for dump imp dfx");
	printf("    %s, %-25s %s\n", "-d", "--die=<die>", "die id for dump imp dfx");
	printf("\n");

	return 0;
}

static int hikp_imp_cmd_check_option(struct major_cmd_ctrl *self)
{
	const char *option[] = {"unknown", "-l", "-D"};

	if ((g_imp_cmd_cfg.param_mask & PARAM_FUNC_MASK) != 0) {
		snprintf(self->err_str, sizeof(self->err_str),
			 "The %s options are already existed.",
			 (g_imp_cmd_cfg.func_type < HIKP_ARRAY_SIZE(option)) ?
			 option[g_imp_cmd_cfg.func_type] : "unknown");
		self->err_no = -EINVAL;
		return self->err_no;
	}

	return 0;
}

static int hikp_imp_cmd_dump_log(struct major_cmd_ctrl *self, const char *argv)
{
	int ret;

	HIKP_SET_USED(argv);

	ret = hikp_imp_cmd_check_option(self);
	if (ret)
		return ret;

	g_imp_cmd_cfg.param_mask |= PARAM_FUNC_MASK;
	g_imp_cmd_cfg.func_type = IMP_FUNC_DUMP_LOG;

	return 0;
}

static int hikp_imp_cmd_dump_reg(struct major_cmd_ctrl *self, const char *argv)
{
	int ret;

	HIKP_SET_USED(argv);

	ret = hikp_imp_cmd_check_option(self);
	if (ret)
		return ret;

	g_imp_cmd_cfg.param_mask |= PARAM_FUNC_MASK;
	g_imp_cmd_cfg.func_type = IMP_FUNC_DUMP_REG;

	return 0;
}

static int hikp_imp_cmd_get_chip(struct major_cmd_ctrl *self, const char *argv)
{
	char *endptr = NULL;
	uint64_t chip;

	chip = strtoul(argv, &endptr, 0);
	if ((endptr <= argv) || (*endptr != '\0') || (chip > UINT8_MAX)) {
		snprintf(self->err_str, sizeof(self->err_str),
			 "Invalid chip id: %" PRIu64 ".", chip);
		self->err_no = -EINVAL;
		return -EINVAL;
	}
	g_imp_cmd_cfg.param_mask |= PARAM_CHIP_MASK;
	g_imp_cmd_cfg.chip = (uint8_t)chip;

	return 0;
}

static int hikp_imp_cmd_get_die(struct major_cmd_ctrl *self, const char *argv)
{
	char *endptr = NULL;
	uint64_t die;

	die = strtoul(argv, &endptr, 0);
	if ((endptr <= argv) || (*endptr != '\0') || (die > UINT8_MAX)) {
		snprintf(self->err_str, sizeof(self->err_str),
			 "Invalid die id: %" PRIu64 ".", die);
		self->err_no = -EINVAL;
		return -EINVAL;
	}
	g_imp_cmd_cfg.param_mask |= PARAM_DIE_MASK;
	g_imp_cmd_cfg.die = (uint8_t)die;

	return 0;
}

static void hikp_imp_cmd_execute(struct major_cmd_ctrl *self)
{
	if ((g_imp_cmd_cfg.param_mask & PARAM_FUNC_MASK) == 0 ||
	    (g_imp_cmd_cfg.param_mask & PARAM_CHIP_MASK) == 0 ||
	    (g_imp_cmd_cfg.param_mask & PARAM_DIE_MASK) == 0) {
		snprintf(self->err_str, sizeof(self->err_str),
			 "Need input function or chip or die param!");
		self->err_no = -EINVAL;
		return;
	}

	if (g_imp_cmd_cfg.func_type == IMP_FUNC_DUMP_LOG)
		hikp_imp_dump_log(self, &g_imp_cmd_cfg);
	else
		hikp_imp_dump_dfx_reg(self, &g_imp_cmd_cfg);
}

static void cmd_imp_dfx_init(void)
{
	struct major_cmd_ctrl *major_cmd = get_major_cmd();

	major_cmd->option_count = 0;
	major_cmd->execute = hikp_imp_cmd_execute;

	cmd_option_register("-h", "--help", false, hikp_imp_cmd_help);
	cmd_option_register("-l", "--log", false, hikp_imp_cmd_dump_log);
	cmd_option_register("-D", "--dump", false, hikp_imp_cmd_dump_reg);
	cmd_option_register("-c", "--chip", true, hikp_imp_cmd_get_chip);
	cmd_option_register("-d", "--die", true, hikp_imp_cmd_get_die);
}

HIKP_CMD_DECLARE("imp", "query the DFX information of the IMP firmware.", cmd_imp_dfx_init);
