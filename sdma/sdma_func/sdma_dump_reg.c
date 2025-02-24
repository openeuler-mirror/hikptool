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

static void sdma_print_reg(const uint32_t *reg_save, uint32_t reg_num)
{
	uint32_t i;

	if (reg_num == 0) {
		printf("SDMA dump is failed\n");
		return;
	}
	printf("  sdma reg dump list:\n");
	for (i = 0; i < reg_num; i++)
		printf("    0x%08x\n", reg_save[i]);
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

	sdma_print_reg(sdma_reg_save, sdma_reg_num);

	return 0;
}
