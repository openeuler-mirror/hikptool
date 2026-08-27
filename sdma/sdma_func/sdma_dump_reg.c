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
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <stdio.h>
#include <errno.h>
#include "tool_lib.h"
#include "hikptdev_plug.h"
#include "sdma_common.h"
#include "sdma_dump_reg.h"

#define TARGET_DIR "/sys/devices/platform/"
#define PREFIX "HISI0431"
#define PREFIX_LEN 8

int sdma_dev_check(void)
{
	struct dirent *entry;
	DIR *dir;

	dir = opendir(TARGET_DIR);
	if (dir == NULL) {
		perror("opendir");
		return -errno;
	}

	while ((entry = readdir(dir)) != NULL) {
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
			continue;

		if (strlen(entry->d_name) >= PREFIX_LEN) {
			if (strncmp(entry->d_name, PREFIX, PREFIX_LEN) == 0) {
				closedir(dir);
				return 0;
			}
		}
	}

	closedir(dir);
	return -ENODEV;
}

static int sdma_rsp_normal_check(const struct hikp_cmd_ret *cmd_ret)
{
	if (cmd_ret == NULL)
		return -ENOSPC;

	if (cmd_ret->status != 0)
		return -EINVAL;

	if (cmd_ret->rsp_data_num > RESP_MAX_NUM)
		return -E2BIG;

	return 0;
}

static int sdma_get_reg(const struct tool_sdma_cmd *cmd, uint32_t *reg_save, uint32_t *reg_num)
{
	struct sdma_dump_req_para req_data = { 0 };
	struct hikp_cmd_header req_header = { 0 };
	struct hikp_cmd_ret *cmd_ret;
	uint32_t i;
	int ret;

	req_data.chip_id = cmd->chip_id;
	req_data.die_id = cmd->die_id;
	req_data.chn_id = cmd->chn_id;

	hikp_cmd_init(&req_header, SDMA_MOD, SDMA_DUMP, cmd->sdma_cmd_type);
	cmd_ret = hikp_cmd_alloc(&req_header, &req_data, sizeof(req_data));
	ret = sdma_rsp_normal_check(cmd_ret);
	if (ret) {
		printf("check cmd ret failed, ret: %d.\n", ret);
		hikp_cmd_free(&cmd_ret);
		return ret;
	}
	*reg_num = cmd_ret->rsp_data_num;
	for (i = 0; i < *reg_num; i++)
		reg_save[i] = cmd_ret->rsp_data[i];

	hikp_cmd_free(&cmd_ret);

	return 0;
}

/* DON'T change the order of these arrays or add entries between! */
static const char *g_sdma_chn_reg_name[] = {
	"SDMAM_CH_CTRL",
	"SDMAM_CH_TEST",
	"SDMAM_CH_TYPER",
	"SDMAM_IRQ_STATUS",
	"SDMAM_IRQ_CTRL",
	"SDMAM_CH_BYPASS_CTRL",
	"SDMAM_CH_CQE_STATUS",
	"SDMAM_CH_STATUS",
	"SDMAM_CH_SECURITY",
	"SDMAM_CH_SQBASER_L",
	"SDMAM_CH_SQBASER_H",
	"SDMAM_CH_SQ_ATTR",
	"SDMAM_CH_SQTDBR",
	"SDMAM_CH_SQHDBR",
	"SDMAM_CH_CQBASER_L",
	"SDMAM_CH_CQBASER_H",
	"SDMAM_CH_CQ_ATTR",
	"SDMAM_CH_CQTDBR",
	"SDMAM_CH_CQHDBR",
	"SDMAM_CH_ICDT_OSTD_TH",
	"SDMAM_ATOMICERR_MASK",
	"SDMAM_ERRSTATUS_MASK",
	"reserve",
	"reserve",
	"reserve",
	"reserve",
	"reserve",
	"reserve",
	"reserve",
	"reserve",
	"reserve",
	"reserve",
	"reserve",
	"reserve",
	"reserve",
	"reserve",
	"reserve",
	"reserve",
	"CH_DFX_INFO0",
	"SDMAM_DBP_CONFIG0",
};

static const char *g_sdma_pc_reg_name[] = {
	"DFX_PC_SEL_INFO0",
	"DFX_PC_SEL_INFO1",
	"DFX_PC_SEL_INFO2",
	"DFX_PC_SEL_INFO3",
	"DFX_PC_SEL_INFO4",
	"DFX_PC_SEL_INFO5",
	"DFX_PC_SEL_INFO6",
	"DFX_PC_SEL_INFO7",
	"DFX_PC_SEL_INFO8",
	"DFX_PC_SEL_INFO9",
	"DFX_PC_SEL_INFO10",
	"DFX_PC_SEL_INFO11",
	"DFX_PC_SEL_INFO12",
	"DFX_PC_SEL_INFO13",
	"DFX_PC_SEL_INFO14",
	"DFX_PC_SEL_INFO15",
	"DFX_PC_SEL_INFO16",
	"DFX_PC_SEL_INFO17",
	"DFX_PC_SEL_INFO18",
	"DFX_PC_SEL_INFO19",
	"DFX_PC_SEL_INFO20",
	"DFX_PC_SEL_INFO21",
	"DFX_PC_SEL_INFO22",
	"DFX_PC_SEL_INFO23",
	"DFX_PC_SEL_INFO24",
	"DFX_PC_SEL_INFO25",
	"DFX_PC_SEL_INFO26",
};

static const char *g_sdma_vc_reg_name[] = {
	"DFX_VC_SEL_INFO0",
	"DFX_VC_SEL_INFO1",
	"DFX_VC_SEL_INFO2",
	"DFX_VC_SEL_INFO3",
	"DFX_VC_SEL_INFO4",
	"DFX_VC_SEL_INFO5",
	"DFX_VC_SEL_INFO6",
	"DFX_VC_SEL_INFO7",
	"DFX_VC_SEL_INFO8",
	"DFX_VC_SEL_INFO9",
	"DFX_VC_SEL_INFO10",
	"DFX_VC_SEL_INFO11",
	"DFX_VC_SEL_INFO12",
	"DFX_VC_SEL_INFO13",
	"DFX_VC_SEL_INFO14",
	"DFX_VC_SEL_INFO15",
	"DFX_VC_SEL_INFO16",
	"DFX_VC_SEL_INFO17",
	"DFX_VC_SEL_INFO18",
};

static const struct reg_name_info {
	enum sdma_dump_cmd_type sub_cmd;
	const char **reg_name;
	uint8_t arr_len;
} g_sdma_reg_name_info_table[] = {
	{DUMP_CHN_STATUS, g_sdma_chn_reg_name, HIKP_ARRAY_SIZE(g_sdma_chn_reg_name)},
	{DUMP_CHN_PC, g_sdma_pc_reg_name, HIKP_ARRAY_SIZE(g_sdma_pc_reg_name)},
	{DUMP_CHN_VC, g_sdma_vc_reg_name, HIKP_ARRAY_SIZE(g_sdma_vc_reg_name)},
};

static void sdma_print_reg(const uint32_t *reg_save, uint32_t reg_num, uint32_t sub_cmd)
{
	const char **reg_name;
	uint32_t index = 0;
	uint8_t arr_len;

	if (reg_num == 0) {
		printf("SDMA dump is failed\n");
		return;
	}

	for (index = 0; index < HIKP_ARRAY_SIZE(g_sdma_reg_name_info_table); index++) {
		if (g_sdma_reg_name_info_table[index].sub_cmd != sub_cmd)
			continue;
		arr_len = g_sdma_reg_name_info_table[index].arr_len;
		reg_name = g_sdma_reg_name_info_table[index].reg_name;
		break;
	}

	if (index == HIKP_ARRAY_SIZE(g_sdma_reg_name_info_table)) {
		printf("can't find reg name table for sdma sub_cmd %u.\n",
		       sub_cmd);
		return;
	}

	printf("  sdma reg dump list:\n");
	printf("%-40s  reg_data\n", "reg_name");
	for (index = 0; index < reg_num; index++) {
		const char *cur_name = index < arr_len ? reg_name[index] : "";

		if (strcmp(cur_name, "reserve") != 0)
			printf("%-40s  0x%08x\n", cur_name, reg_save[index]);
	}
}

int sdma_reg_dump(struct tool_sdma_cmd *cmd)
{
	uint32_t sdma_reg_save[RESP_MAX_NUM] = { 0 };
	uint32_t sdma_reg_num = 0;
	int ret;

	if (cmd == NULL)
		return -EINVAL;

	ret = sdma_dev_check();
	if (ret) {
		printf("The current environment not support this feature!\n");
		return ret;
	}

	ret = sdma_get_reg(cmd, sdma_reg_save, &sdma_reg_num);
	if (ret)
		return ret;

	sdma_print_reg(sdma_reg_save, sdma_reg_num, cmd->sdma_cmd_type);

	return 0;
}
