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

#include "hikp_collect_lib.h"
#include "hikp_collect.h"
#include "tool_lib.h"
#include "sas_tools_include.h"
#include "sas_common.h"
#include "sas_analy_queue.h"
#include "sas_dump_reg.h"
#include "sas_read_dev.h"
#include "sas_read_errcode.h"
#include "sata_dump_reg.h"

#define CHIP_MAX_SIZE 10
#define DIE_MAX_SIZE 10

typedef int (*reg_info_func_t)(uint32_t, uint32_t);

enum sata_dump_type {
	SATA_DUMP_UNKNOWN = 0,
	SATA_DUMP_GLOBAL,
	SATA_DUMP_PORTX,
};

struct reg_op {
	char *func_name;
	reg_info_func_t func;
};

static int sas_sata_reg_log(void *data)
{
	struct reg_op *op = (struct reg_op *)data;
	bool stop_flag = false;
	uint32_t i, j;
	int ret;

	for (i = 0; i < CHIP_MAX_SIZE; i++) {
		for (j = 0; j < DIE_MAX_SIZE; j++) {
			ret = op->func(i, j);
			if (ret) {
				HIKP_ERROR_PRINT("%s chip%u die%u failed: %d\n", op->func_name,
						i, j, ret);
				/*
				 * Stop collection when the die id is 0, indicating that the
				 * current chip id is not supported.
				 * */
				if (j == 0)
					stop_flag = true;

				break;
			}
		}

		if (stop_flag)
			break;
	}

	return 0;
}

static void collect_sas_path_log(char *group)
{
	struct info_collect_cmd sas_path_cmd = {
		.log_name = "ls_by-path",
		.args = {"ls", "-l", "/dev/disk/by-path/", NULL},
	};
	int ret;

	ret = hikp_collect_log(group, sas_path_cmd.log_name,
			       hikp_collect_exec, (void *)&sas_path_cmd);
	if (ret)
		HIKP_ERROR_PRINT("collect %s log failed: %d\n", sas_path_cmd.log_name, ret);
}

static void collect_sas_phy_log(void)
{
	struct info_collect_cmd sas_phy_cmds[] = {
		{
			.log_name = "phy-invalid_dword_count",
			.args = {"cat", "/sys/class/sas_phy/*/invalid_dword_count", NULL},
		},
		{
			.log_name = "phy-negotiated_linkrate",
			.args = {"cat", "/sys/class/sas_phy/*/negotiated_linkrate", NULL},
		},
		{
			.log_name = "phy-enable",
			.args = {"cat", "/sys/class/sas_phy/*/enable", NULL},
		},
	};
	size_t i, size;
	int ret;

	size = HIKP_ARRAY_SIZE(sas_phy_cmds);
	for (i = 0; i < size; i++) {
		ret = hikp_collect_log(GROUP_SAS, sas_phy_cmds[i].log_name,
				       hikp_collect_cat_glob_exec, (void *)&sas_phy_cmds[i]);
		if (ret)
			HIKP_ERROR_PRINT("collect %s log failed: %d\n",
					 sas_phy_cmds[i].log_name, ret);
	}
}

static void collect_sas_host_log(void)
{
	struct info_collect_cmd sas_host_cmds[] = {
		{
			.log_name = "host-nr_hw_queues",
			.args = {"cat", "/sys/class/scsi_host/host*/nr_hw_queues", NULL},
		},
		{
			.log_name = "host-intr",
			.args = {"cat", "/sys/class/scsi_host/host*/intr*", NULL},
		},
	};
	size_t i, size;
	int ret;

	size = HIKP_ARRAY_SIZE(sas_host_cmds);
	for (i = 0; i < size; i++) {
		ret = hikp_collect_log(GROUP_SAS, sas_host_cmds[i].log_name,
				       hikp_collect_cat_glob_exec, (void *)&sas_host_cmds[i]);
		if (ret)
			HIKP_ERROR_PRINT("collect %s log failed: %d\n",
					 sas_host_cmds[i].log_name, ret);
	}
}

static void collect_sas_disk_log(void)
{
	struct info_collect_cmd sas_disk_cmds[] = {
		{
			.log_name = "disk-scheduler",
			.args = {"cat", "/sys/block/sd*/queue/scheduler", NULL},
		},
		{
			.log_name = "disk-max",
			.args = {"cat", "/sys/block/sd*/queue/max*", NULL},
		},
		{
			.log_name = "disk-state",
			.args = {"cat", "/sys/block/sd*/device/state", NULL},
		},
	};
	size_t i, size;
	int ret;

	size = HIKP_ARRAY_SIZE(sas_disk_cmds);
	for (i = 0; i < size; i++) {
		ret = hikp_collect_log(GROUP_SAS, sas_disk_cmds[i].log_name,
				       hikp_collect_cat_glob_exec, (void *)&sas_disk_cmds[i]);
		if (ret)
			HIKP_ERROR_PRINT("collect %s log failed: %d\n",
					 sas_disk_cmds[i].log_name, ret);
	}
}

static int collect_sas_lsscsi_log_exec(void *data)
{
	const struct info_collect_cmd sas_lsscsi_cmds[] = {
		{
			.args = {"lsscsi", "-lg", NULL},
		},
		{
			.args = {"lsscsi", "-pvt", NULL},
		},
		{
			.args = {"lsscsi", "-H", NULL},
		},
	};
	size_t i, size;
	(void)data;
	int ret;

	size = HIKP_ARRAY_SIZE(sas_lsscsi_cmds);
	for (i = 0; i < size; i++) {
		ret = hikp_collect_exec((void *)&sas_lsscsi_cmds[i]);
		if (ret)
			return ret;
	}

	return 0;
}

static void collect_sas_lsscsi_log(char *group)
{
	int ret;

	ret = hikp_collect_log(group, "lsscsi", collect_sas_lsscsi_log_exec, (void *)NULL);
	if (ret)
		HIKP_ERROR_PRINT("collect lsscsi log failed: %d\n", ret);
}

static void collect_sas_copy_files(void)
{
	struct info_collect_cmd sas_copy_cmds[] = {
		{
			.group = GROUP_SAS,
			.log_name = "debugfs",
			.args = {"cp", "-rf", "/sys/kernel/debug/hisi_sas/", NULL},
		},
		{
			.group = GROUP_SAS,
			.log_name = "parameters_hw",
			.args = {"cp", "-rf", "/sys/module/hisi_sas_v3_hw/parameters/", NULL},
		},
		{
			.group = GROUP_SAS,
			.log_name = "parameters_main",
			.args = {"cp", "-rf", "/sys/module/hisi_sas_main/parameters/", NULL},
		},
	};
	size_t i, size;
	int ret;

	size = HIKP_ARRAY_SIZE(sas_copy_cmds);
	for (i = 0; i < size; i++) {
		ret = hikp_save_files(&sas_copy_cmds[i]);
		if (ret)
			HIKP_ERROR_PRINT("cp %s failed: %d\n",
					 sas_copy_cmds[i].args[ARGS_IDX2], ret);
	}
}

static int sas_anacq_info(uint32_t chip_id, uint32_t die_id)
{
	struct tool_sas_cmd cmd = {
		.chip_id = chip_id,
		.die_id = die_id,
	};
	int ret;

	printf("hikptool sas_anacq -c %u -d %u -s\n", cmd.chip_id, cmd.die_id);
	cmd.sas_cmd_type = ANACQ_NUM;
	ret = sas_analy_cmd(&cmd);
	if (ret) {
		HIKP_ERROR_PRINT("collect cq number failed: %d\n", ret);
		return ret;
	}

	printf("hikptool sas_anacq -c %u -d %u -p\n", cmd.chip_id, cmd.die_id);
	cmd.sas_cmd_type = ANACQ_PRT;
	ret = sas_analy_cmd(&cmd);
	if (ret)
		HIKP_ERROR_PRINT("collect cq read/write pointer failed: %d\n", ret);

	return ret;
}

static int sas_anadq_info(uint32_t chip_id, uint32_t die_id)
{
	struct tool_sas_cmd cmd = {
		.chip_id = chip_id,
		.die_id = die_id,
	};
	int ret;

	printf("hikptool sas_anadq -c %u -d %u -s\n", cmd.chip_id, cmd.die_id);
	cmd.sas_cmd_type = ANADQ_NUM;
	ret = sas_analy_cmd(&cmd);
	if (ret) {
		HIKP_ERROR_PRINT("collect dq num failed: %d\n", ret);
		return ret;
	}

	printf("hikptool sas_anadq -c %u -d %u -p\n", cmd.chip_id, cmd.die_id);
	cmd.sas_cmd_type = ANADQ_PRT;
	ret = sas_analy_cmd(&cmd);
	if (ret)
		HIKP_ERROR_PRINT("collect dq read/write pointer failed: %d\n", ret);

	return ret;
}

static int sas_dump_info(uint32_t chip_id, uint32_t die_id)
{
	struct tool_sas_cmd cmd = {
		.chip_id = chip_id,
		.die_id = die_id,
	};
	uint32_t i;
	int ret;

	printf("hikptool sas_dump -c %u -d %u -g\n", cmd.chip_id, cmd.die_id);
	cmd.sas_cmd_type = DUMP_GLOBAL;
	ret = sas_reg_dump(&cmd);
	if (ret) {
		HIKP_ERROR_PRINT("dump global failed: %d\n", ret);
		return ret;
	}

	for (i = 0; i <= SAS_MAX_PHY_NUM; i++) {
		printf("hikptool sas_dump -c %u -d %u -p %u\n", cmd.chip_id, cmd.die_id, i);
		cmd.sas_cmd_type = DUMP_PHYX;
		cmd.phy_id = i;
		ret = sas_reg_dump(&cmd);
		if (ret) {
			HIKP_ERROR_PRINT("dump phy %u failed: %d\n", i, ret);
			return ret;
		}
	}

	printf("hikptool sas_dump -c %u -d %u -b\n", cmd.chip_id, cmd.die_id);
	cmd.sas_cmd_type = DUMP_AXI;
	ret = sas_reg_dump(&cmd);
	if (ret)
		HIKP_ERROR_PRINT("dump axi failed: %d\n", ret);

	return ret;
}

static int sas_dev_info(uint32_t chip_id, uint32_t die_id)
{
	struct tool_sas_cmd cmd = {
		.chip_id = chip_id,
		.die_id = die_id,
	};

	printf("hikptool sas_dev -c %u -d %u -l\n", cmd.chip_id, cmd.die_id);
	cmd.sas_cmd_type = DEV_LINK;
	return sas_dev(&cmd);
}

static int sas_errcode_info(uint32_t chip_id, uint32_t die_id)
{
	struct tool_sas_cmd cmd = {
		.chip_id = chip_id,
		.die_id = die_id,
	};
	uint32_t i;
	int ret;

	for (i = 0; i < SAS_MAX_ERR_NUM; i++) {
		printf("hikptool sas_errcode -c %u -d %u -t %u\n", cmd.chip_id, cmd.die_id, i);
		cmd.sas_cmd_type = i;
		ret = sas_errcode_read(&cmd);
		if (ret) {
			HIKP_ERROR_PRINT("collect errcode %u info failed: %d\n", i, ret);
			return ret;
		}
	}

	return 0;
}

static void collect_sas_reg_log(void)
{
	struct reg_op op = {0};
	int ret;

	op.func = sas_anacq_info;
	op.func_name = "sas_anacq_info";
	ret = hikp_collect_log(GROUP_SAS, "sas_anacq", sas_sata_reg_log, (void *)&op);
	if (ret)
		HIKP_INFO_PRINT("%s failed: %d\n", op.func_name, ret);

	op.func = sas_anadq_info;
	op.func_name = "sas_anadq_info";
	ret = hikp_collect_log(GROUP_SAS, "sas_anadq", sas_sata_reg_log, (void *)&op);
	if (ret)
		HIKP_INFO_PRINT("%s failed: %d\n", op.func_name, ret);

	op.func = sas_dump_info;
	op.func_name = "sas_dump_info";
	ret = hikp_collect_log(GROUP_SAS, "sas_dump", sas_sata_reg_log, (void *)&op);
	if (ret)
		HIKP_INFO_PRINT("%s failed: %d\n", op.func_name, ret);

	op.func = sas_dev_info;
	op.func_name = "sas_dev_info";
	ret = hikp_collect_log(GROUP_SAS, "sas_dev", sas_sata_reg_log, (void *)&op);
	if (ret)
		HIKP_INFO_PRINT("%s failed: %d\n", op.func_name, ret);

	op.func = sas_errcode_info;
	op.func_name = "sas_errcode_info";
	ret = hikp_collect_log(GROUP_SAS, "sas_errcode", sas_sata_reg_log, (void *)&op);
	if (ret)
		HIKP_INFO_PRINT("%s failed: %d\n", op.func_name, ret);
}

void collect_sas_log(void)
{
	collect_sas_phy_log();
	collect_sas_host_log();
	collect_sas_disk_log();
	collect_sas_copy_files();
	collect_sas_path_log(GROUP_SAS);
	collect_sas_lsscsi_log(GROUP_SAS);
	collect_sas_reg_log();
}

static int sata_reg_dump_info(uint32_t chip_id, uint32_t die_id)
{
	struct tool_sata_cmd cmd = {
		.chip_id = chip_id,
		.die_id = die_id,
	};
	uint32_t i;
	int ret;

	printf("hikptool sata_dump -c %u -d %u -g\n", cmd.chip_id, cmd.die_id);
	cmd.sata_cmd_type = SATA_DUMP_GLOBAL;
	ret = sata_reg_dump(&cmd);
	if (ret) {
		HIKP_ERROR_PRINT("dump global failed: %d\n", ret);
		return ret;
	}

	cmd.sata_cmd_type = SATA_DUMP_PORTX;
	for (i = 0; i <= 1; i++) {
		printf("hikptool sata_dump -c %u -d %u -p %u\n", cmd.chip_id, cmd.die_id, i);
		cmd.phy_id = i;
		ret = sata_reg_dump(&cmd);
		if (ret) {
			HIKP_INFO_PRINT("dump port%u reg failed: %d\n", i, ret);
			return ret;
		}
	}

	return 0;
}

static void collect_sata_reg_log(void)
{
	struct reg_op op = {
		.func = sata_reg_dump_info,
		.func_name = "sata_reg_dump_info",
	};
	int ret;

	ret = hikp_collect_log(GROUP_SATA, "reg_dump", sas_sata_reg_log, (void *)&op);
	if (ret)
		HIKP_INFO_PRINT("%s failed: %d\n", op.func_name, ret);
}

void collect_sata_log(void)
{
	collect_sas_path_log(GROUP_SATA);
	collect_sas_lsscsi_log(GROUP_SATA);
	collect_sata_reg_log();
}
