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
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include "hikptdev_plug.h"
#include "sas_common.h"
#include "sas_read_errcode.h"

static int sas_get_errcode(const struct tool_sas_cmd *cmd, uint32_t *reg_save, uint32_t *reg_num)
{
	struct hikp_cmd_header req_header;
	struct hikp_cmd_ret *cmd_ret;
	struct sas_errcode_req_para req_data = { 0 };

	req_data.chip_id = cmd->chip_id;
	req_data.die_id = cmd->die_id;

	hikp_cmd_init(&req_header, SAS_MOD, SAS_ERRCODE, cmd->sas_cmd_type);
	cmd_ret = hikp_cmd_alloc(&req_header, &req_data, sizeof(req_data));
	if (cmd_ret == NULL || cmd_ret->status != 0) {
		printf("sas_errcode excutes hikp_cmd_alloc err\n");
		free(cmd_ret);
		return -EINVAL;
	}
	*reg_num = cmd_ret->rsp_data_num;
	for (int i = 0; i < *reg_num; i++)
		reg_save[i] = cmd_ret->rsp_data[i];

	free(cmd_ret);
	return 0;
}

static void sas_print_errcode(uint32_t cmd_type, const uint32_t *reg_save, uint32_t reg_num)
{
	uint32_t i;
	const char *errcode_type[] = {
		"",
		"DWS_LOST",
		"RESET_PROB",
		"CRC_FAIL",
		"OPEN_REJ"
	};

	if (reg_num == 0) {
		printf("SAS error code read is failed\n");
		return;
	}
	if (cmd_type == ERRCODE_ALL) {
		printf("       DWS_LOST  RESET_PROB  CRC_FAIL  OPEN_REJ\n");
		for (i = 0; i < reg_num; i += SAS_ERR_NUM) {
			printf("phy%u  0x%08x    0x%08x      0x%08x    0x%08x\n", i / SAS_ERR_NUM,
			       reg_save[i + DWS_LOST], reg_save[i + RESET_PROB],
			       reg_save[i + CRC_FAIL], reg_save[i + OPEN_REJ]);
		}
	} else {
		printf("       %s\n", errcode_type[cmd_type]);
		for (i = 0; i < reg_num; i++)
			printf("phy%u  0x%08x\n", i, reg_save[i]);
	}
}

int sas_errcode_read(struct tool_sas_cmd *cmd)
{
	int ret;
	uint32_t reg_num = 0;
	uint32_t reg_save[RESP_MAX_NUM] = { 0 };

	if (cmd == NULL)
		return -ENOSPC;

	ret = sas_get_errcode(cmd, reg_save, &reg_num);
	if (ret)
		return ret;

	sas_print_errcode(cmd->sas_cmd_type, reg_save, reg_num);

	return 0;
}
