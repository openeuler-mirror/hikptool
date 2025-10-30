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
#include "ummu_common.h"
#include "ummu_reg_dump.h"

struct tool_ummu_reg_cmd {
	enum ummu_cmd_type cmd_type;
	enum ummu_dump_cmd_type dump_type;
	uint8_t cache_idx;
	uint8_t sync_timeout_open;
	uint8_t rr_win_num;
	uint8_t kcmd_entry_no;
	uint8_t ummu_id;
	bool set_flag;
	bool reg_flag;
};

static struct tool_ummu_reg_cmd g_ummu_reg_cmd = {
	.cmd_type = UMMU_CMD_HELP,
	.dump_type = UMMU_HELP,
	.cache_idx = 0,
	.sync_timeout_open = 0,
	.rr_win_num = 0,
	.kcmd_entry_no = 0,
	.ummu_id = MAX_UMMU_NUM,
	.set_flag = false,
	.reg_flag = false,
};

static int ummu_reg_command(struct major_cmd_ctrl *self, const char *argv)
{
	HIKP_SET_USED(self);
	HIKP_SET_USED(argv);

	g_ummu_reg_cmd.reg_flag = true;

	return 0;
}

static int ummu_reg_dump_help(struct major_cmd_ctrl *self, const char *argv)
{
	HIKP_SET_USED(argv);

	if (!self || !self->cmd_ptr)
		return -EINVAL;
	printf("  Usage: hikptool %s SUBCOMMAND [OPTION] [PARA...]\n", self->cmd_ptr->name);
	printf("         %s\n", self->cmd_ptr->help_info);
	printf("  Subcommands: must choose one subcommand at a time\n");
	printf("        %-25s %s\n", "--reg", "ummu dfx for registers");
	printf("  Options: can only choose one option at a time\n");
	printf("    %s, %-25s %s\n", "-h", "--help", "display this help and exit");
	printf("    %s, %-25s %s\n", "-d", "--dump", "dump registers");
	printf("    usage: -d [reg_type] -[k, i, r, u] [val], e.g. -d kcmd -k 1 -u 0\n");
	printf("    reg_type includes [kcmd, umcmd, ubif, tbu, tcu, sky, cnt]\n");
	printf("    [cnt] will display the number of UMMU, choose one of them to display info\n");
	printf("    %s, %-25s %s\n", "-s", "--sync_timeout_set",
	       "set sync_timeout_open register which can prevent system freeze");
	printf("    usage: -s 1: validate sync_timeout_open; -s 0: invalidate sync_timeout_open\n");
	printf("  Parameters:\n");
	printf("    %s, %-25s %s\n", "-i", "--cache_idx",
	       "set cache_idx for UMMU_SWIF_UMCMD_CACHE_DFX4 when "
	       "reg_type is set to umcmd, range is 0 to 15, default val is 0");
	printf("    %s, %-25s %s\n", "-r", "--rr_win_num",
	       "set rr_win_num for UMMU_SWIF_UMCMD_RR_WIN_DFX0 when "
	       "reg_type is set to umcmd, range is 0 to 6, default val is 0");
	printf("    %s, %-25s %s\n", "-k", "--kcmd_entry_no",
	       "set kcmd_entry_no for UMMU_SWIF_KCMDQ_DFX_CMD_ENTRY_STATUS "
	       "when reg_type is set to kcmd, range is 0 to 15, default val is 0");
	printf("    %s, %-25s %s\n", "-u", "--ummu_id",
	       "set ummu_id(range from 0 to 7) to choose UMMU. By default, display all UMMU info");
	printf("\n");

	g_ummu_reg_cmd.set_flag = true;

	return 0;
}

static int ummu_reg_dump_type_set(struct major_cmd_ctrl *self, const char *argv)
{
#define MAX_ENTRY_STR_SIZE 64
	char str[MAX_ENTRY_STR_SIZE] = { 0 };
	size_t len;

	if (!self || !argv)
		return -EINVAL;

	if (g_ummu_reg_cmd.set_flag) {
		printf("Failed to set dump type, can only choose one option at a time\n");
		return -EPERM;
	}

	len = strlen(argv);
	if (len >= MAX_ENTRY_STR_SIZE || len == 0 ) {
		printf("ummu reg type string err length: %zu\n", len);
		return -EPERM;
	}
	memcpy(str, argv, len);

	if (!strcmp(str, "kcmd")) {
		g_ummu_reg_cmd.dump_type = UMMU_KCMD_DUMP;
	} else if (!strcmp(str, "umcmd")) {
		g_ummu_reg_cmd.dump_type = UMMU_UMCMD_DUMP;
	} else if (!strcmp(str, "ubif")) {
		g_ummu_reg_cmd.dump_type = UMMU_UBIF_DUMP;
	} else if (!strcmp(str, "tbu")) {
		g_ummu_reg_cmd.dump_type = UMMU_TBU_DUMP;
	} else if (!strcmp(str, "tcu")) {
		g_ummu_reg_cmd.dump_type = UMMU_TCU_DUMP;
	} else if (!strcmp(str, "sky")) {
		g_ummu_reg_cmd.dump_type = UMMU_SKY_DUMP;
	} else if (!strcmp(str, "cnt")) {
		g_ummu_reg_cmd.dump_type = UMMU_CNT_DUMP;
	} else {
		printf("ummu set reg type(%s) err\n", str);
		return -EINVAL;
	}

	g_ummu_reg_cmd.cmd_type = UMMU_CMD_DUMP;
	g_ummu_reg_cmd.set_flag = true;

	return 0;
}

static int ummu_reg_dump_execute_process(void)
{
	int ret;

	switch (g_ummu_reg_cmd.dump_type) {
	case UMMU_KCMD_DUMP:
		ret = ummu_dump_kcmd_execute(g_ummu_reg_cmd.kcmd_entry_no, g_ummu_reg_cmd.ummu_id);
		break;
	case UMMU_UMCMD_DUMP:
		ret = ummu_dump_umcmd_execute(g_ummu_reg_cmd.cache_idx, g_ummu_reg_cmd.rr_win_num,
					      g_ummu_reg_cmd.ummu_id);
		break;
	case UMMU_UBIF_DUMP:
		ret = ummu_dump_ubif_execute(g_ummu_reg_cmd.ummu_id);
		break;
	case UMMU_TBU_DUMP:
		ret = ummu_dump_tbu_execute(g_ummu_reg_cmd.ummu_id);
		break;
	case UMMU_TCU_DUMP:
		ret = ummu_dump_tcu_execute(g_ummu_reg_cmd.ummu_id);
		break;
	case UMMU_SKY_DUMP:
		ret = ummu_dump_sky_execute(g_ummu_reg_cmd.ummu_id);
		break;
	case UMMU_CNT_DUMP:
		if (g_ummu_reg_cmd.ummu_id != MAX_UMMU_NUM) {
			printf("Error : Options don't match!, "
			       "don't enter ummu_id.\n");
			ret = -EINVAL;
			break;
		}

		ret = ummu_dump_cnt_execute();
		break;
	default:
		ret = -EPERM;
		break;
	}

	return ret;
}

static int ummu_cache_idx_set(struct major_cmd_ctrl *self, const char *argv)
{
	uint8_t val;
	int ret;

	HIKP_SET_USED(self);

	if (!argv)
		return -EINVAL;

	ret = string_toub(argv, &val);
	if (ret) {
		printf("ummu set cache idx err %d\n", ret);
		return ret;
	}

	if (val > MAX_CACHE_IDX) {
		printf("cache_idx[%u] is out of bound[%u]\n", val, MAX_CACHE_IDX);
		return -EINVAL;
	}

	g_ummu_reg_cmd.cache_idx = val;

	return 0;
}

static int ummu_sync_timeout_set(struct major_cmd_ctrl *self, const char *argv)
{
	uint8_t val;
	int ret;

	HIKP_SET_USED(self);

	if (!argv)
		return -EINVAL;

	if (g_ummu_reg_cmd.set_flag) {
		printf("Failed to set dump type, can only choose one option at a time\n");
		return -EPERM;
	}

	ret = string_toub(argv, &val);
	if (ret) {
		printf("ummu set sync timeout value err %d\n", ret);
		return ret;
	}

	if (val > MAX_SYNC_TIMEOUT_VAL) {
		printf("sync_timeout_val[%u] is out of bound[%u]\n", val, MAX_SYNC_TIMEOUT_VAL);
		return -EINVAL;
	}

	g_ummu_reg_cmd.sync_timeout_open = val;
	g_ummu_reg_cmd.cmd_type = UMMU_CMD_SYNC_TIMEOUT;
	g_ummu_reg_cmd.set_flag = true;

	return 0;
}

static int ummu_rr_win_num_set(struct major_cmd_ctrl *self, const char *argv)
{
	uint8_t val;
	int ret;

	HIKP_SET_USED(self);

	if (!argv)
		return -EINVAL;

	ret = string_toub(argv, &val);
	if (ret) {
		printf("ummu set rr win num err %d\n", ret);
		return ret;
	}

	if (val > MAX_RR_WIN_NUM) {
		printf("rr_win_num[%u] is out of bound[%u]\n", val, MAX_RR_WIN_NUM);
		return -EINVAL;
	}

	g_ummu_reg_cmd.rr_win_num = val;

	return 0;
}

static int ummu_kcmd_entry_no_set(struct major_cmd_ctrl *self, const char *argv)
{
	uint8_t val;
	int ret;

	HIKP_SET_USED(self);

	if (!argv)
		return -EINVAL;

	ret = string_toub(argv, &val);
	if (ret) {
		printf("ummu set kcmd entry no err %d\n", ret);
		return ret;
	}

	if (val > MAX_KCMD_ENTRY_NO) {
		printf("kcmd_entry_no[%u] is out of bound[%u]\n", val, MAX_KCMD_ENTRY_NO);
		return -EINVAL;
	}

	g_ummu_reg_cmd.kcmd_entry_no = val;

	return 0;
}

static int ummu_id_set(struct major_cmd_ctrl *self, const char *argv)
{
	uint8_t val;
	int ret;

	HIKP_SET_USED(self);

	if (!argv)
		return -EINVAL;

	ret = string_toub(argv, &val);
	if (ret) {
		printf("ummu set ummu id err %d\n", ret);
		return ret;
	}

	if (val > (MAX_UMMU_NUM - 1)) {
		printf("ummu_id[%hhu] is out of bound[%d]\n", val, (MAX_UMMU_NUM - 1));
		return -EINVAL;
	}

	g_ummu_reg_cmd.ummu_id = val;

	return 0;
}

static void ummu_reg_dump_execute(struct major_cmd_ctrl *self)
{
	int ret;
	static const char *ummu_msgq_dump_succ_msg[] = {
		"",
		"ummu_kcmd_reg_dump success!",
		"ummu_umcmd_reg_dump success!",
		"ummu_ubif_reg_dump success!",
		"ummu_tbu_reg_dump success!",
		"ummu_tcu_reg_dump success!",
		"ummu_sky_reg_dump success!",
		"ummu_cnt_dump success!",
	};
	static const char *ummu_msgq_dump_err_msg[] = {
		"Error: unknown reg_type!",
		"ummu_kcmd_reg_dump failed!",
		"ummu_umcmd_reg_dump failed!",
		"ummu_ubif_reg_dump failed!",
		"ummu_tbu_reg_dump failed!",
		"ummu_tcu_reg_dump failed!",
		"ummu_sky_reg_dump failed!",
		"ummu_cnt_dump failed!",
	};

	ret = ummu_reg_dump_execute_process();
	if (ret)
		snprintf(self->err_str, sizeof(self->err_str), "%s",
			 ummu_msgq_dump_err_msg[g_ummu_reg_cmd.dump_type]);
	else
		printf("%s\n", ummu_msgq_dump_succ_msg[g_ummu_reg_cmd.dump_type]);

	self->err_no = ret;
}

static void ummu_sync_timeout_execute(struct major_cmd_ctrl *self)
{
	int ret;

	ret = ummu_set_sync_timeout(g_ummu_reg_cmd.sync_timeout_open, g_ummu_reg_cmd.ummu_id);
	if (ret)
		snprintf(self->err_str, sizeof(self->err_str),
		"ummu_set_sync_timeout failed");
	else
		printf("ummu_set_sync_timeout success!\n");

	self->err_no = ret;
}

static int ummu_check_ummu_id(void)
{
	uint32_t ummu_cnt;
	int ret;

	/* user does not enter ummu_id */
	if (g_ummu_reg_cmd.ummu_id == MAX_UMMU_NUM)
		return 0;

	ret = ummu_get_valid_cnt(&ummu_cnt);
	if (ret)
		return ret;

	if (g_ummu_reg_cmd.ummu_id >= ummu_cnt) {
		printf("Error : Invalid ummu_id[%u]!\n", g_ummu_reg_cmd.ummu_id);
		return -EINVAL;
	}
	return 0;
}

static void ummu_cmd_execute(struct major_cmd_ctrl *self)
{
	int ret = 0;

	if (!self) {
		printf("Error : major_cmd_ctrl is nullptr!\n");
		return;
	}

	if (!g_ummu_reg_cmd.reg_flag) {
		snprintf(self->err_str, sizeof(self->err_str), "Option --reg is needed!");
		self->err_no = -EINVAL;
		return;
	}

	ret = ummu_check_ummu_id();
	if (ret)
		goto out;

	switch (g_ummu_reg_cmd.cmd_type) {
	case UMMU_CMD_SYNC_TIMEOUT:
		ummu_sync_timeout_execute(self);
		break;
	case UMMU_CMD_DUMP:
		ummu_reg_dump_execute(self);
		break;
	default:
		snprintf(self->err_str, sizeof(self->err_str), "Unknown param_type!");
		ret = -EINVAL;
		break;
	}
out:
	self->err_no = ret;
}

static void cmd_ummu_reg_dump_init(void)
{
	struct major_cmd_ctrl *major_cmd = get_major_cmd();

	major_cmd->option_count = 0;
	major_cmd->execute = ummu_cmd_execute;

	cmd_option_register("", "--reg", false, ummu_reg_command);
	cmd_option_register("-h", "--help", false, ummu_reg_dump_help);
	cmd_option_register("-d", "--dump", true, ummu_reg_dump_type_set);
	cmd_option_register("-i", "--cache_idx", true, ummu_cache_idx_set);
	cmd_option_register("-s", "--sync_timeout_set", true, ummu_sync_timeout_set);
	cmd_option_register("-r", "--rr_win_num", true, ummu_rr_win_num_set);
	cmd_option_register("-k", "--kcmd_entry_no", true, ummu_kcmd_entry_no_set);
	cmd_option_register("-u", "--ummu_id", true, ummu_id_set);
}

HIKP_CMD_DECLARE("ummu", "ummu dfx for registers dump", cmd_ummu_reg_dump_init);
