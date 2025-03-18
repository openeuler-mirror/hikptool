/*
 * Copyright (c) 2024 Hisilicon Technologies Co., Ltd.
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

#include <unistd.h>
#include <string.h>
#include "hikp_collect_lib.h"
#include "hikp_collect.h"
#include "tool_lib.h"
#include "sdma_dump_reg.h"
#include "sdma_tools_include.h"

#define PC_MAX_NUM      32
#define VC_MAX_NUM      160
#define SDMA_DIE_MAX    4
#define SDMA_DIE_CHANGE 2
#define SDMA_DUMP_DELAY 50000
#define BUFFER_LENTH    1024

typedef int (*reg_info_func_t)(uint32_t, uint32_t);

enum sdma_dump_type {
	SDMA_DUMP_UNKNOWN = 0,
	SDMA_DUMP_CHN_STATUS,
	SDMA_DUMP_CHN_PC,
	SDMA_DUMP_CHN_VC,
};

struct reg_op {
	char *func_name;
	reg_info_func_t func;
	uint32_t sdma_die;
};

static int sdma_dmesg_exec(void *data)
{
	struct info_collect_cmd *cmd = (struct info_collect_cmd *)data;
	char dmesg_cmd[MAX_LOG_NAME_LEN] = {0};
	char buffer[BUFFER_LENTH] = {0};
	int i = 0;
	FILE *fp;

	while (cmd->args[i] != 0) {
		strcat(dmesg_cmd, cmd->args[i]);
		strcat(dmesg_cmd, " ");
		i++;
	}

	fp = popen(dmesg_cmd, "r");
	if (fp == NULL) {
		perror("popen");
		return -errno;
	}

	while (fgets(buffer, sizeof(buffer), fp) != NULL) {
		printf("%s", buffer);
	}

	pclose(fp);

	return 0;
}

static void collect_sdma_kernel_log(void)
{
	struct info_collect_cmd sdma_kernel_cmds[] = {
		{
			.log_name = "dmesg",
			.args = {"dmesg", "|", "grep", "sdma", NULL},
		},
	};
	size_t i, size;
	int ret;

	size = HIKP_ARRAY_SIZE(sdma_kernel_cmds);
	for (i = 0; i < size; i++) {
		ret = hikp_collect_log(GROUP_SDMA, sdma_kernel_cmds[i].log_name,
				       sdma_dmesg_exec, (void *)&sdma_kernel_cmds[i]);
		if (ret) {
			HIKP_ERROR_PRINT("collect %s log failed: %d\n",
					 sdma_kernel_cmds[i].log_name, ret);
		}
	}
}

static void collect_sdma_debugfs_log(void)
{
	struct info_collect_cmd sdma_debugfs_cmds[] = {
		{
			.log_name = "sdma_channels",
			.args = {"cat", "/sys/kernel/debug/sdma/sdma_channels", NULL},
		},
		{
			.log_name = "sdma_error",
			.args = {"cat", "/sys/kernel/debug/sdma/sdma_error", NULL},
		},
		{
			.log_name = "sdma_stats",
			.args = {"cat", "/sys/kernel/debug/sdma/sdma_stats", NULL},
		},
	};
	size_t i, size;
	int ret;

	size = HIKP_ARRAY_SIZE(sdma_debugfs_cmds);
	for (i = 0; i < size; i++) {
		ret = hikp_collect_log(GROUP_SDMA, sdma_debugfs_cmds[i].log_name,
				       hikp_collect_cat_glob_exec, (void *)&sdma_debugfs_cmds[i]);
		if (ret) {
			HIKP_ERROR_PRINT("collect %s log failed: %d\n",
					 sdma_debugfs_cmds[i].log_name, ret);
		}
	}
}

static int sdma_reg_log(void *data)
{
	struct reg_op *op = (struct reg_op *)data;
	uint32_t chip, die;
	int ret;

	chip = op->sdma_die / SDMA_DIE_CHANGE;
	die = op->sdma_die % SDMA_DIE_CHANGE;
	ret = op->func(chip, die);
	if (ret)
		HIKP_ERROR_PRINT("%s chip%u die%u failed: %d\n", op->func_name, chip, die, ret);

	return ret;
}

static int sdma_chn_status_dump_info(uint32_t chip_id, uint32_t die_id)
{
	struct tool_sdma_cmd cmd = {
		.chip_id = chip_id,
		.die_id = die_id,
	};
	int ret;

	printf("hikptool sdma_dump -s -c %u -d %u\n", cmd.chip_id, cmd.die_id);
	printf("  sdma%u channel status\n", SDMA_DIE_CHANGE * cmd.chip_id + cmd.die_id);
	cmd.sdma_cmd_type = SDMA_DUMP_CHN_STATUS;
	ret = sdma_reg_dump(&cmd);
	if (ret) {
		HIKP_ERROR_PRINT("dump channel status failed: %d\n", ret);
		return ret;
	}

	return 0;
}

static int sdma_pc_dump_info(uint32_t chip_id, uint32_t die_id)
{
	struct tool_sdma_cmd cmd = {
		.chip_id = chip_id,
		.die_id = die_id,
	};
	uint32_t i;
	int ret;

	cmd.sdma_cmd_type = SDMA_DUMP_CHN_PC;
	for (i = 0; i < PC_MAX_NUM; i++) {
		printf("hikptool sdma_dump -p -c %u -d %u -n %u\n", cmd.chip_id, cmd.die_id, i);
		printf("  sdma%u pc chn%u\n", SDMA_DIE_CHANGE * cmd.chip_id + cmd.die_id, i);
		cmd.chn_id = i;
		ret = sdma_reg_dump(&cmd);
		if (ret) {
			HIKP_ERROR_PRINT("dump pc chn%u reg failed: %d\n", i, ret);
			return ret;
		}
	}

	return 0;
}

static int sdma_vc_dump_info(uint32_t chip_id, uint32_t die_id)
{
	struct tool_sdma_cmd cmd = {
		.chip_id = chip_id,
		.die_id = die_id,
	};
	uint32_t i;
	int ret;

	cmd.sdma_cmd_type = SDMA_DUMP_CHN_VC;
	for (i = 0; i < VC_MAX_NUM; i++) {
		printf("hikptool sdma_dump -v -c %u -d %u -n %u\n", cmd.chip_id, cmd.die_id, i);
		printf("  sdma%u vc chn%u\n", SDMA_DIE_CHANGE * cmd.chip_id + cmd.die_id, i);
		cmd.chn_id = i;
		ret = sdma_reg_dump(&cmd);
		if (ret) {
			HIKP_ERROR_PRINT("dump vc chn%u reg failed: %d\n", i, ret);
			return ret;
		}
	}

	return 0;
}

static void collect_sdma_reg_log(void)
{
	struct reg_op ch_op = {
		.func = sdma_chn_status_dump_info,
		.func_name = "sdma_chn_status_dump_info",
	};
	struct reg_op pc_op = {
		.func = sdma_pc_dump_info,
		.func_name = "sdma_pc_dump_info",
	};
	struct reg_op vc_op = {
		.func = sdma_vc_dump_info,
		.func_name = "sdma_vc_dump_info",
	};
	char log_name[MAX_LOG_NAME_LEN] = {0};
	uint32_t i;
	int ret;

	for (i = 0; i < SDMA_DIE_MAX; i++) {
		ch_op.sdma_die = i;
		memset(log_name, 0, MAX_LOG_NAME_LEN);
		(void)snprintf(log_name, MAX_LOG_NAME_LEN, "sdma%u_channel_status_dump", i);

		ret = hikp_collect_log(GROUP_SDMA, log_name, sdma_reg_log, (void *)&ch_op);
		if (ret)
			HIKP_ERROR_PRINT("%s failed: %d\n", ch_op.func_name, ret);
		usleep(SDMA_DUMP_DELAY);

		pc_op.sdma_die = i;
		memset(log_name, 0, MAX_LOG_NAME_LEN);
		(void)snprintf(log_name, MAX_LOG_NAME_LEN, "sdma%u_pc_dump", i);

		ret = hikp_collect_log(GROUP_SDMA, log_name, sdma_reg_log, (void *)&pc_op);
		if (ret)
			HIKP_ERROR_PRINT("%s failed: %d\n", pc_op.func_name, ret);
		usleep(SDMA_DUMP_DELAY);

		vc_op.sdma_die = i;
		memset(log_name, 0, MAX_LOG_NAME_LEN);
		(void)snprintf(log_name, MAX_LOG_NAME_LEN, "sdma%u_vc_dump", i);

		ret = hikp_collect_log(GROUP_SDMA, log_name, sdma_reg_log, (void *)&vc_op);
		if (ret)
			HIKP_ERROR_PRINT("%s failed: %d\n", vc_op.func_name, ret);
		usleep(SDMA_DUMP_DELAY);
	}
}

void collect_sdma_log(void)
{
	collect_sdma_kernel_log();
	collect_sdma_debugfs_log();
	collect_sdma_reg_log();
}
