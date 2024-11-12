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
#include "sas_read_dqe.h"

static int sas_get_dqe(const struct tool_sas_cmd *cmd, uint32_t *reg_save, uint32_t *reg_num)
{
	struct hikp_cmd_header req_header;
	struct hikp_cmd_ret *cmd_ret;
	struct sas_dqe_req_para req_data = { 0 };

	req_data.chip_id = cmd->chip_id;
	req_data.die_id = cmd->die_id;
	req_data.que_id = cmd->que_id;
	req_data.dqe_id = cmd->dqe_id;

	hikp_cmd_init(&req_header, SAS_MOD, SAS_DQE, cmd->sas_cmd_type);
	cmd_ret = hikp_cmd_alloc(&req_header, &req_data, sizeof(req_data));
	if (cmd_ret == NULL || cmd_ret->status != 0 || cmd_ret->rsp_data_num > RESP_MAX_NUM) {
		printf("sas_dqe excutes hikp_cmd_alloc err\n");
		free(cmd_ret);
		return -EINVAL;
	}
	*reg_num = cmd_ret->rsp_data_num;
	for (int i = 0; i < *reg_num; i++)
		reg_save[i] = cmd_ret->rsp_data[i];

	free(cmd_ret);
	return 0;
}

static void print_dqe_info(const void *reg_save, uint32_t reg_num)
{
	volatile struct hisi_sas_dq_info *dqe = (volatile struct hisi_sas_dq_info *)(reg_save);

	printf("The dqe dw0 information as below:\n");
	printf("abort_flag: %u\n", dqe->dw0.abort_flag);
	printf("abort_devtype: %u\n", dqe->dw0.abort_devtype);
	printf("Rsponse_report: %u\n", dqe->dw0.Rsponse_report);
	printf("TLR_Ctrl: %u\n", dqe->dw0.TLR_Ctrl);
	printf("Phy_ID: %u\n", dqe->dw0.Phy_ID);
	printf("Force_Phy: %u\n", dqe->dw0.Force_Phy);
	printf("PORT: 0x%x\n", dqe->dw0.PORT);
	printf("PRI: %u\n", dqe->dw0.PRI);
	printf("CMD: %u\n", dqe->dw0.CMD);

	printf("The dqe dw1 information as below:\n");
	printf("Non_Busy_Constraint: %u\n", dqe->dw1.Non_Busy_Constraint);
	printf("SSP_Pass_Through: %u\n", dqe->dw1.SSP_Pass_Through);
	printf("DIR: %u\n", dqe->dw1.DIR);
	printf("Reset: %u\n", dqe->dw1.Reset);
	printf("PIR_Present: %u\n", dqe->dw1.PIR_Present);
	printf("Enable_Transport_Layer_Retry: %u\n", dqe->dw1.Enable_Transport_Layer_Retry);
	printf("Verify_Data_Transfer_Length: 0x%x\n", dqe->dw1.Verify_Data_Transfer_Length);
	printf("Frame_Type: %u\n", dqe->dw1.Frame_Type);
	printf("Device_ID: %u\n", dqe->dw1.Device_ID);

	printf("The dqe dw2 information as below:\n");
	printf("Command_Frame_Length: %u\n", dqe->dw2.Command_Frame_Length);
	printf("Leave_Affiliation_Open: %u\n", dqe->dw2.Leave_Affiliation_Open);
	printf("Ncq_Tag: %u\n", dqe->dw2.Ncq_Tag);
	printf("Max_Response_Frame_Length: %u\n", dqe->dw2.Max_Response_Frame_Length);
	printf("Sg_Mode: %u\n", dqe->dw2.Sg_Mode);
	printf("First_Burst: %u\n", dqe->dw2.First_Burst);

	printf("Initiator_Port_Transfer_Tag: %u\n", dqe->Initiator_Port_Transfer_Tag);
	printf("Target_Port_Transfer_Tag: %u\n", dqe->Target_Port_Transfer_Tag);
	printf("Data_Transfer_Length: %u\n", dqe->Data_Transfer_Length);
	printf("First_Burst_Num: %u\n", dqe->First_Burst_Num);
	printf("DIF_PRD_Table_Length: %u\n", dqe->DIF_PRD_Table_Length);
	printf("PRD_Table_Length: %u\n", dqe->PRD_Table_Length);

	printf("The dqe dw7 information as below:\n");
	printf("Double_Mode: %u\n", dqe->dw7.Double_Mode);
	printf("Abort_IPTT: %u\n", dqe->dw7.Abort_IPTT);
}

int sas_dqe(const struct tool_sas_cmd *cmd)
{
	int ret;
	uint32_t reg_num = 0;
	uint32_t reg_save[RESP_MAX_NUM] = { 0 };

	if (cmd == NULL)
		return -ENOSPC;

	ret = sas_get_dqe(cmd, reg_save, &reg_num);
	if (ret)
		return ret;

	if (reg_num < REG_NUM_DQE_MAX) {
		printf("SAS dqe is failed\n");
		return -EINVAL;
	}
	print_dqe_info(reg_save, reg_num);
	return 0;
}
