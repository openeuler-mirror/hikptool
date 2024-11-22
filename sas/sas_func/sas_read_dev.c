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
#include "sas_read_dev.h"

static int sas_get_dev(const struct tool_sas_cmd *cmd, uint32_t *reg_save, uint32_t *reg_num)
{
	int i;
	struct hikp_cmd_header req_header;
	struct hikp_cmd_ret *cmd_ret;
	struct sas_dev_req_para req_data = { 0 };

	req_data.chip_id = cmd->chip_id;
	req_data.die_id = cmd->die_id;
	req_data.dev_id = cmd->dev_id;

	hikp_cmd_init(&req_header, SAS_MOD, SAS_DEV, cmd->sas_cmd_type);
	cmd_ret = hikp_cmd_alloc(&req_header, &req_data, sizeof(req_data));
	if (cmd_ret == NULL || cmd_ret->status != 0 || cmd_ret->rsp_data_num > RESP_MAX_NUM) {
		printf("sas_dqe excutes hikp_cmd_alloc err\n");
		hikp_cmd_free(&cmd_ret);
		return -EINVAL;
	}
	*reg_num = cmd_ret->rsp_data_num;
	for (i = 0; i < *reg_num; i++)
		reg_save[i] = cmd_ret->rsp_data[i];

	hikp_cmd_free(&cmd_ret);
	return 0;
}

static void print_dev_link(const uint32_t *reg_save, uint32_t reg_num)
{
	uint32_t i;
	uint32_t index, index1;
	uint32_t phy_status = reg_save[0];
	uint32_t link_type = reg_save[1];
	uint32_t link_speed = reg_save[2];
	const char *dev_type[] = {
		"not SATA",
		"SATA",
	};

	const char *dev_speed[] = {
		"1.5Gbps",
		"3.0GBPS",
		"6.0Gbps",
		"12.0GBPS",
	};

	for (i = 0; i <= SAS_MAX_PHY_NUM; i++) {
		if ((phy_status >> i) & 0x1) {
			index = ((link_type >> i) & 0x1) ? 1 : 0;
			index1 = ((link_speed >> (i * LINK_SPEED_WIDTH)) & 0xf) - LINK_SPEED_OFFSET;
			if (index1 < HIKP_ARRAY_SIZE(dev_speed))
				printf("device on phy%u is %s, link speed is %s\n",
				       i, dev_type[index], dev_speed[index1]);
		}
	}
}

static void sas_print_dev(const uint32_t *reg_save, uint32_t reg_num, uint32_t cmd_type)
{
	if (reg_num < REG_NUM_DEV_LINK_MAX) {
		printf("SAS device is failed\n");
		return;
	}
	switch (cmd_type) {
	case DEV_LINK:
		print_dev_link(reg_save, reg_num);
		break;
	default:
		printf("cmd_type is error\n");
	}
}


int sas_dev(const struct tool_sas_cmd *cmd)
{
	int ret;
	uint32_t reg_num = 0;
	uint32_t reg_save[RESP_MAX_NUM] = { 0 };

	if (cmd == NULL)
		return -ENOSPC;

	ret = sas_get_dev(cmd, reg_save, &reg_num);
	if (ret)
		return ret;

	sas_print_dev(reg_save, reg_num, cmd->sas_cmd_type);
	return 0;
}
