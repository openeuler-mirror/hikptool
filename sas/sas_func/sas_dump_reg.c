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
#include "sas_dump_reg.h"

static int sas_get_reg(const struct tool_sas_cmd *cmd, uint32_t *reg_save, uint32_t *reg_num)
{
	struct hikp_cmd_header req_header;
	struct hikp_cmd_ret *cmd_ret;
	struct sas_dump_req_para req_data = { 0 };

	req_data.chip_id = cmd->chip_id;
	req_data.die_id = cmd->die_id;
	req_data.phy_id = cmd->phy_id;

	hikp_cmd_init(&req_header, SAS_MOD, SAS_DUMP, cmd->sas_cmd_type);
	cmd_ret = hikp_cmd_alloc(&req_header, &req_data, sizeof(req_data));
	if (cmd_ret == NULL || cmd_ret->status != 0 || cmd_ret->rsp_data_num > RESP_MAX_NUM) {
		printf("sas_dump excutes hikp_cmd_alloc err\n");
		hikp_cmd_free(&cmd_ret);
		return -1;
	}
	*reg_num = cmd_ret->rsp_data_num;
	for (int i = 0; i < *reg_num; i++)
		reg_save[i] = cmd_ret->rsp_data[i];

	hikp_cmd_free(&cmd_ret);
	return 0;
}

static void sas_print_reg(uint32_t cmd_type, const uint32_t *reg_save, uint32_t reg_num)
{
	uint32_t i;

	if (reg_num == 0) {
		printf("SAS dump is failed\n");
		return;
	}
	printf("  sas reg dump list:\n");
	for (i = 0; i < reg_num; i += 1)
		printf("    0x%08x\n", reg_save[i]);
}

int sas_reg_dump(struct tool_sas_cmd *cmd)
{
	int ret;
	uint32_t reg_num = 0;
	uint32_t reg_save[RESP_MAX_NUM] = { 0 };

	if (cmd == NULL)
		return -ENOSPC;

	ret = sas_get_reg(cmd, reg_save, &reg_num);
	if (ret)
		return ret;

	sas_print_reg(cmd->sas_cmd_type, reg_save, reg_num);

	return 0;
}
