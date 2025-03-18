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
#include "sata_common.h"
#include "sata_dump_reg.h"

static int sata_get_reg(const struct tool_sata_cmd *cmd, uint32_t *reg_save, uint32_t *reg_num)
{
	uint32_t i;
	struct hikp_cmd_ret *cmd_ret;
	struct hikp_cmd_header req_header = { 0 };
	struct sata_dump_req_para req_data = { 0 };

	req_data.die_id = cmd->die_id;
	req_data.phy_id = cmd->phy_id;
	req_data.chip_id = cmd->chip_id;

	hikp_cmd_init(&req_header, SATA_MOD, SATA_DUMP, cmd->sata_cmd_type);
	cmd_ret = hikp_cmd_alloc(&req_header, &req_data, sizeof(req_data));
	if (cmd_ret == NULL || cmd_ret->status != 0 || cmd_ret->rsp_data_num > RESP_MAX_NUM) {
		printf("hikp_data_proc err\n");
		hikp_cmd_free(&cmd_ret);
		return -1;
	}
	*reg_num = cmd_ret->rsp_data_num;
	for (i = 0; i < *reg_num; i++)
		reg_save[i] = cmd_ret->rsp_data[i];

	hikp_cmd_free(&cmd_ret);
	return 0;
}

static void sata_print_reg(const uint32_t *reg_save, uint32_t reg_num)
{
	uint32_t i;

	if (reg_num == 0) {
		printf("SATA dump is failed\n");
		return;
	}
	printf("  sata reg dump list:\n");
	for (i = 0; i < reg_num; i++)
		printf("    0x%08x\n", reg_save[i]);
}

int sata_reg_dump(struct tool_sata_cmd *cmd)
{
	int ret;
	uint32_t sata_reg_num = 0;
	uint32_t sata_reg_save[RESP_MAX_NUM] = { 0 };

	if (cmd == NULL)
		return -ENOSPC;

	ret = sata_get_reg(cmd, sata_reg_save, &sata_reg_num);
	if (ret)
		return ret;

	sata_print_reg(sata_reg_save, sata_reg_num);

	return 0;
}
