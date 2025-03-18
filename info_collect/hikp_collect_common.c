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

static int software_version_exec(void *data)
{
	const struct info_collect_cmd software_version_cmds[] = {
		{
			.args = {"uname", "-a", NULL},
		},
		{
			.log_name = "os-release",
			.args = {"cat", "/etc/*release", NULL},
		},
		{
			.log_name = "os-latest",
			.args = {"cat", "/etc/*latest", NULL},
		},
	};
	HIKP_SET_USED(data);
	size_t i, size;
	int ret;

	size = HIKP_ARRAY_SIZE(software_version_cmds);
	for (i = 0; i < size; i++) {
		if (!strcmp(software_version_cmds[i].args[ARGS_IDX0], "cat"))
			ret = hikp_collect_cat_glob_exec((void *)&software_version_cmds[i]);
		else
			ret = hikp_collect_exec((void *)&software_version_cmds[i]);

		if (ret)
			HIKP_ERROR_PRINT("collect software_version_cmds[%zu] log failed: %d\n",
					 i, ret);
	}

	return 0;
}

static int mem_info_exec(void *data)
{
	const struct info_collect_cmd mem_info_cmds[] = {
		{
			.args = {"cat", "/proc/meminfo", NULL},
		},
		{
			.args = {"free", "-m", NULL},
		},
		{
			.args = {"vmstat", NULL},
		},
		{
			.args = {"cat", "/proc/iomem", NULL},
		},
	};
	HIKP_SET_USED(data);
	size_t i, size;
	int ret;

	size = HIKP_ARRAY_SIZE(mem_info_cmds);
	for (i = 0; i < size; i++) {
		ret = hikp_collect_exec((void *)&mem_info_cmds[i]);
		if (ret)
			HIKP_ERROR_PRINT("collect mem_info_cmds[%zu] log failed: %d\n", i, ret);
	}

	return 0;
}

static int process_info_exec(void *data)
{
	const struct info_collect_cmd process_info_cmds[] = {
		{
			.args = {"ps", "euf", NULL},
		},
		{
			.args = {"ps", "aux", NULL},
		},
		{
			.args = {"top", "-bn", "1", NULL},
		},
		{
			.args = {"mpstat", NULL},
		},
	};
	HIKP_SET_USED(data);
	size_t i, size;
	int ret;

	size = HIKP_ARRAY_SIZE(process_info_cmds);
	for (i = 0; i < size; i++) {
		ret = hikp_collect_exec((void *)&process_info_cmds[i]);
		if (ret)
			HIKP_ERROR_PRINT("collect process_info_cmds[%zu] log failed: %d\n",
					 i, ret);
	}

	return 0;
}

static int config_info_exec(void *data)
{
	struct info_collect_cmd config_info_cmds[] = {
		{
			.args = {"cat", "/proc/cmdline", NULL},
		},
		{
			.args = {"getconf", "PAGE_SIZE", NULL},
		},
		{
			.group = GROUP_COMMON,
			.log_name = "config",
			.args = {"cp", "-f", "/boot/config-*", NULL},
		},
		{
			.group = GROUP_COMMON,
			.log_name = "smmu",
			.args = {"cp", "-rf", "/sys/class/iommu", NULL},
		},
	};
	HIKP_SET_USED(data);
	size_t i, size;
	int ret;

	size = HIKP_ARRAY_SIZE(config_info_cmds);
	for (i = 0; i < size; i++) {
		char *log_name = config_info_cmds[i].log_name;

		if (log_name && !strcmp(log_name, "config"))
			ret = hikp_collect_cp_glob_exec((void *)&config_info_cmds[i]);
		else if (log_name && !strcmp(log_name, "smmu"))
			ret = hikp_save_files(&config_info_cmds[i]);
		else
			ret = hikp_collect_exec((void *)&config_info_cmds[i]);

		if (ret)
			HIKP_ERROR_PRINT("collect process_info_cmds[%zu] log failed: %d\n",
					 i, ret);
	}

	return 0;
}

static int service_info_exec(void *data)
{
	const struct info_collect_cmd service_info_cmds[] = {
		{
			.args = {"service", "iptables", "status", NULL},
		},
		{
			.args = {"service", "irqbalance", "status", NULL},
		},
	};
	HIKP_SET_USED(data);
	size_t i, size;
	int ret;

	size = HIKP_ARRAY_SIZE(service_info_cmds);
	for (i = 0; i < size; i++) {
		ret = hikp_collect_exec((void *)&service_info_cmds[i]);
		if (ret)
			HIKP_ERROR_PRINT("collect service_info_cmds[%zu] log failed: %d\n",
					 i, ret);
	}

	return 0;
}

static void collect_software_info(void)
{
	int ret;

	ret = hikp_collect_log(GROUP_COMMON, "software_version", software_version_exec, (void *)NULL);
	if (ret)
		HIKP_ERROR_PRINT("software_version_exec failed: %d\n", ret);

	ret = hikp_collect_log(GROUP_COMMON, "mem_info", mem_info_exec, (void *)NULL);
	if (ret)
		HIKP_ERROR_PRINT("mem_info_exec failed: %d\n", ret);

	ret = hikp_collect_log(GROUP_COMMON, "process_info", process_info_exec, (void *)NULL);
	if (ret)
		HIKP_ERROR_PRINT("process_info_exec failed: %d\n", ret);

	ret = hikp_collect_log(GROUP_COMMON, "config_info", config_info_exec, (void *)NULL);
	if (ret)
		HIKP_ERROR_PRINT("config_info_exec failed: %d\n", ret);

	ret = hikp_collect_log(GROUP_COMMON, "service_info", service_info_exec, (void *)NULL);
	if (ret)
		HIKP_ERROR_PRINT("service_info_exec failed: %d\n", ret);
}

static int hardware_info_exec(void *data)
{
	const struct info_collect_cmd hardware_cmds[] = {
		{
			.args = {"cat", MIDR_EL1_PATH, NULL},
		},
		{
			.args = {"cat", "/sys/bus/cpu/devices/cpu0/cpufreq/scaling_governor", NULL},
		},
		{
			.args = {"cat", "/sys/devices/system/cpu/online", NULL},
		},
		{
			.args = {"numactl", "-H", NULL},
		},
		{
			.args = {"numastat", NULL},
		},
		{
			.args = {"lscpu", NULL},
		},
		{
			.args = {"dmidecode", NULL},
		},
	};
	HIKP_SET_USED(data);
	size_t i, size;
	int ret;

	size = HIKP_ARRAY_SIZE(hardware_cmds);
	for (i = 0; i < size; i++) {
		ret = hikp_collect_exec((void *)&hardware_cmds[i]);
		if (ret)
			HIKP_ERROR_PRINT("collect hardware_cmds[%zu] log failed: %d\n", i, ret);
	}

	return 0;
}

static void collect_hardware_info(void)
{
	int ret;

	ret = hikp_collect_log(GROUP_COMMON, "hardware_info", hardware_info_exec, (void *)NULL);
	if (ret)
		HIKP_ERROR_PRINT("hardware_info_exec failed: %d\n", ret);
}

void collect_common_log(void)
{
	collect_software_info();
	collect_hardware_info();
}
