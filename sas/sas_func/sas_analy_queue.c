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
#include "sas_analy_queue.h"

static int sas_get_res(const struct tool_sas_cmd *cmd, uint32_t *reg_save, uint32_t *reg_num)
{
	struct hikp_cmd_ret *cmd_ret;
	struct hikp_cmd_header req_header = { 0 };
	struct sas_analy_para req_data = { 0 };

	req_data.chip_id = cmd->chip_id;
	req_data.die_id = cmd->die_id;
	req_data.phy_id = cmd->phy_id;

	if (cmd->sas_cmd_type == ANADQ_NUM)
		hikp_cmd_init(&req_header, SAS_MOD, SAS_ANADQ, ANADQ_NUM);
	else if (cmd->sas_cmd_type == ANACQ_NUM)
		hikp_cmd_init(&req_header, SAS_MOD, SAS_ANACQ, ANACQ_NUM);
	else if (cmd->sas_cmd_type == ANADQ_PRT)
		hikp_cmd_init(&req_header, SAS_MOD, SAS_ANADQ, ANADQ_PRT);
	else
		hikp_cmd_init(&req_header, SAS_MOD, SAS_ANACQ, ANACQ_PRT);

	cmd_ret = hikp_cmd_alloc(&req_header, &req_data, sizeof(req_data));
	if (cmd_ret == NULL || cmd_ret->status != 0) {
		printf("sas_analy excutes hikp_cmd_alloc err\n");
		free(cmd_ret);
		return -EINVAL;
	}
	*reg_num = cmd_ret->rsp_data_num;
	for (int i = 0; i < *reg_num; i++)
		reg_save[i] = cmd_ret->rsp_data[i];

	free(cmd_ret);
	return 0;
}

static void sas_print_prt(const uint32_t *reg_save, uint32_t reg_num)
{
	uint32_t i;

	if (reg_num == 0) {
		printf("SAS get queue pointer is failed\n");
		return;
	}
	printf("  sas queue READ/WRITE pointer:\n");
	printf("    QUEUE        READ         WRITE\n");
	for (i = 0; i < reg_num; i += REG_NUM_DQ)
		printf("     %u    0x%04x      0x%08x\n",
		       i / REG_NUM_DQ, reg_save[i], reg_save[i + 1]);
}

static void sas_print_dqnum(const uint32_t *reg_save, uint32_t reg_num)
{
	if (reg_num < DQE_NUM_REG) {
		printf("SAS get dq number is failed\n");
		return;
	}
	printf("  sas delive dq num is %u\n", reg_save[0]);
	printf("  sas dqe wait to complete is %u\n", reg_save[1]);
}

static void sas_print_cqnum(const uint32_t *reg_save, uint32_t reg_num)
{
	if (reg_num < CQ_COAL_CNT) {
		printf("SAS get cq number is failed\n");
		return;
	}
	printf("  sas delive cq num is %u\n", reg_save[CQE_NUM_BYTE]);
	if ((reg_save[CQ_COAL] & CQ_COAL_ENABLE) != CQ_COAL_ENABLE) {
		printf("  sas cq int coal is disable\n");
		return;
	}
	printf("  sas cq int coal is enable\n");
	printf("  sas cq int coal time is 0x%08x\n", reg_save[CQ_COAL_TIME]);
	printf("  sas cq int coal count is 0x%02x\n", reg_save[CQ_COAL_CNT]);
}

int sas_analy_cmd(struct tool_sas_cmd *cmd)
{
	int ret;
	uint32_t reg_num = 0;
	uint32_t reg_save[RESP_MAX_NUM] = { 0 };

	if (cmd == NULL)
		return -ENOSPC;

	ret = sas_get_res(cmd, reg_save, &reg_num);
	if (ret)
		return ret;

	if (cmd->sas_cmd_type == ANADQ_NUM)
		sas_print_dqnum(reg_save, reg_num);
	else if (cmd->sas_cmd_type == ANACQ_NUM)
		sas_print_cqnum(reg_save, reg_num);
	else
		sas_print_prt(reg_save, reg_num);

	return 0;
}
