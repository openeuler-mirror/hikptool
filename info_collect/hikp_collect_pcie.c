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

#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include "hikp_collect_lib.h"
#include "hikp_collect.h"
#include "tool_lib.h"
#include "hikptdev_plug.h"
#include "pcie_link_ltssm.h"
#include "pcie_statistics.h"
#include "pcie_common.h"
#include "pcie_reg_dump.h"

#define PCIE_DEV_LEN 512
#define PCIE_DEV_PATH "/sys/bus/pci/devices"
#define MAX_NIMBUS_NUM_ALL 8

/* Optimization barrier */
#ifndef barrier
/* The "volatile" is due to gcc bugs */
# define barrier() __asm__ __volatile__("": : :"memory")
#endif

struct pcie_id_info {
	uint32_t chip_id;
	uint32_t port_id;
};

/* get pcie config space info */
static void collect_pcie_common(char *pcie_dev_name)
{
	struct info_collect_cmd pcie_cmd_arr = {0};
	int ret;

	pcie_cmd_arr.group = GROUP_PCIE;
	pcie_cmd_arr.log_name = pcie_dev_name;
	pcie_cmd_arr.args[ARGS_IDX0] = "lspci";
	pcie_cmd_arr.args[ARGS_IDX1] = "-vvvxxxx";
	pcie_cmd_arr.args[ARGS_IDX2] = "-s";
	pcie_cmd_arr.args[ARGS_IDX3] = pcie_dev_name;
	pcie_cmd_arr.args[ARGS_IDX4] = NULL;
	ret = hikp_collect_log(pcie_cmd_arr.group, pcie_cmd_arr.log_name,
					 hikp_collect_exec, (void *)&pcie_cmd_arr);
	if (ret)
		HIKP_ERROR_PRINT("collect_pcie_common failed: %d\n", ret);
}

static void collect_pcie_single_cfg(void)
{
	char dev_name[PCIE_DEV_LEN];
	struct dirent *ptr = NULL;
	DIR *dir = NULL;

	if ((dir = opendir(PCIE_DEV_PATH)) == NULL) {
		perror("failed to open path \n");
		return;
	}

	while ((ptr = readdir(dir)) != NULL) {
		if ((strcmp(ptr->d_name, ".") == 0) || (strcmp(ptr->d_name, "..") == 0)) {
			continue;
		} else if (ptr->d_type == DT_LNK) {
			memset(dev_name, 0, sizeof(dev_name));
			strncpy(dev_name, ptr->d_name, sizeof(dev_name) - 1);
			dev_name[sizeof(dev_name) - 1] = '\0';
			collect_pcie_common(dev_name);
		}
	}

	closedir(dir);
}

/* get pcie config tree info */
static void collect_pcie_cfg_tree(void)
{
	struct info_collect_cmd pcie_cmd_arr = {
		.group = GROUP_PCIE,
		.log_name = "pcie_tree",
		.args = {"lspci", "-tv", NULL},
	};
	int ret;

	ret = hikp_collect_log(pcie_cmd_arr.group, pcie_cmd_arr.log_name,
					 hikp_collect_exec, (void *)&pcie_cmd_arr);
	if (ret)
		HIKP_ERROR_PRINT("collect_pcie_cfg_tree failed: %d\n", ret);
}

static int pcie_mv_dumplog(void)
{
	struct info_collect_cmd pcie_cmd_arr = { 0 };

	pcie_cmd_arr.group = GROUP_PCIE;
	pcie_cmd_arr.args[ARGS_IDX0] = "mv";
	pcie_cmd_arr.args[ARGS_IDX1] = dumpreg_log_file;

	return hikp_move_files(&pcie_cmd_arr);
}

static int collect_pcie_local_info(void *data)
{
	struct pcie_id_info *info = (struct pcie_id_info *)data;
	uint32_t port_id;

	port_id = info->port_id;
	printf("chip_id:%u, port_id:%u\n", info->chip_id, port_id);
	/* do dump action for each port */
	/* step 1  pcie trace */
	printf("hikptool pcie_trace -i %u -s\n", port_id);
	(void)pcie_ltssm_trace_show(port_id);
	/* step 2  pcie link status */
	printf("hikptool pcie_trace -i %u -f\n", port_id);
	(void)pcie_ltssm_link_status_get(port_id);
	/* step 3  pcie err cnt */
	printf("hikptool pcie_info -i %u -es\n", port_id);
	(void)pcie_error_state_get(port_id);
	/* step 4  pcie pm trace */
	printf("hikptool pcie_trace -i %u -pm\n",  port_id);
	(void)pcie_pm_trace(port_id);

	return 0;
}

static int pcie_port_distribution_info(void *data)
{
	uint32_t chip_id = *(uint32_t *)(data);
	int ret;

	printf("hikptool pcie_info -i %u -d\n",  chip_id);
	ret = pcie_port_distribution_get(chip_id);
	if (ret)
		HIKP_ERROR_PRINT("pcie_port_distribution_get failed: %d\n", ret);
	return ret;
}

static void collect_pcie_local(void)
{
	struct pcie_info_req_para req_data = { 0 };
	char name[MAX_LOG_NAME_LEN + 1] = { 0 };
	struct pcie_port_info *port_info = NULL;
	struct hikp_cmd_ret *cmd_ret = NULL;
	struct hikp_cmd_header req_header;
	struct pcie_id_info info;
	uint32_t port_num;
	uint32_t port_id;
	uint32_t i, j;
	int ret;

	for (i = 0; i < MAX_NIMBUS_NUM_ALL; i++) {
		req_data.interface_id = i;

		memset(name, 0, MAX_LOG_NAME_LEN + 1);
		(void)snprintf(name, MAX_LOG_NAME_LEN, "pcie_local_nimbus_%u", i);

		ret = hikp_collect_log(GROUP_PCIE, name, pcie_port_distribution_info, (void *)&i);
		if (ret) {
			HIKP_INFO_PRINT("Nimbus:%u hikp_collect_log pcie_port_distribution_info unsuccessful!\n", i);
			return;
		}

		hikp_cmd_init(&req_header, PCIE_MOD, PCIE_INFO, INFO_DISTRIBUTION);
		cmd_ret = hikp_cmd_alloc(&req_header, &req_data, sizeof(req_data));
		ret = port_distribution_rsp_data_check(cmd_ret, &port_num);
		if (ret) {
			HIKP_ERROR_PRINT("port_distribution_rsp_data_check failed: %d\n", ret);
			hikp_cmd_free(&cmd_ret);
			return;
		}

		port_info = (struct pcie_port_info *)cmd_ret->rsp_data;
		for (j = 0; j < port_num; j++) {
			port_id = port_info->info_pair[j].port_id;
			info.chip_id = i;
			info.port_id = port_info->info_pair[j].port_id;

			memset(name, 0, MAX_LOG_NAME_LEN + 1);
			(void)snprintf(name, MAX_LOG_NAME_LEN, "pcie_local_port_%u", j);

			ret = hikp_collect_log(GROUP_PCIE, name, collect_pcie_local_info, (void *)&info);
			if (ret) {
				HIKP_ERROR_PRINT("hikp_collect_log collect_pcie_local_info failed: %d\n", ret);
				hikp_cmd_free(&cmd_ret);
				return;
			}
			/* step 1  pcie dumpreg core level */
			(void)pcie_dumpreg_do_dump(port_id, DUMP_GLOBAL_LEVEL);
			(void)pcie_mv_dumplog();
			/* step 2  pcie dumpreg port level */
			(void)pcie_dumpreg_do_dump(port_id, DUMP_PORT_LEVEL);
			(void)pcie_mv_dumplog();
		}
		hikp_cmd_free(&cmd_ret);
	}
}

void collect_pcie_info(void)
{
	collect_pcie_cfg_tree();

	collect_pcie_single_cfg();

	collect_pcie_local();
}
