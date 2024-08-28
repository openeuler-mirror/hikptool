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

#ifndef SAS_DQE_H
#define SAS_DQE_H

#include "sas_tools_include.h"

struct sas_dqe_req_para {
	uint32_t chip_id;
	uint32_t die_id;
	uint32_t que_id;
	uint32_t dqe_id;
};

struct hikp_sas_dqe_dw0 {
	uint32_t abort_flag : 2;
	uint32_t abort_devtype : 1;
	uint32_t Rsvd0 : 2;
	uint32_t Rsponse_report : 1;
	uint32_t TLR_Ctrl : 2;
	uint32_t Phy_ID : 9;
	uint32_t Force_Phy : 1;
	uint32_t PORT : 4;
	uint32_t Rsvd1 : 5;
	uint32_t PRI : 1;
	uint32_t Rsvd5 : 1;
	uint32_t CMD : 3;
};

struct hikp_sas_dqe_dw1 {
	uint32_t Rsvd2 : 3;
	uint32_t Non_Busy_Constraint : 1;
	uint32_t SSP_Pass_Through : 1;
	uint32_t DIR : 2;
	uint32_t Reset : 1;
	uint32_t PIR_Present : 1;
	uint32_t Enable_Transport_Layer_Retry : 1;
	uint32_t Verify_Data_Transfer_Length : 1;
	uint32_t Frame_Type : 5;
	uint32_t Device_ID : 16;
};

struct hikp_sas_dqe_dw2 {
	uint32_t Command_Frame_Length : 9;
	uint32_t Leave_Affiliation_Open : 1;
	uint32_t Ncq_Tag : 5;
	uint32_t Max_Response_Frame_Length : 9;
	uint32_t Sg_Mode : 2;
	uint32_t First_Burst : 1;
	uint32_t Rsvd3 : 5;
};

struct hikp_sas_dqe_dw7 {
	uint32_t Rsvd4 : 15;
	uint32_t Double_Mode : 1;
	uint32_t Abort_IPTT : 16;
};

struct hisi_sas_dq_info {
	struct hikp_sas_dqe_dw0 dw0;
	struct hikp_sas_dqe_dw1 dw1;
	struct hikp_sas_dqe_dw2 dw2;
	uint32_t Initiator_Port_Transfer_Tag : 16;
	uint32_t Target_Port_Transfer_Tag : 16;
	uint32_t Data_Transfer_Length;
	uint32_t First_Burst_Num;
	uint32_t DIF_PRD_Table_Length : 16;
	uint32_t PRD_Table_Length : 16;
	struct hikp_sas_dqe_dw7 dw7;
};

int sas_dqe(const struct tool_sas_cmd *cmd);

#endif /* SAS_DQE_H */
