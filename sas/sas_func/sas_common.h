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

#ifndef __SAS_COMMON_H_
#define __SAS_COMMON_H_

#define RESP_MAX_NUM 60
#define SAS_MAX_PHY_NUM 7
#define SAS_MAX_ERR_NUM 6
#define SAS_ERR_NUM 4
#define SAS_QUEUE_NUM 16
#define IPTT_ICT_STATUS 29
#define LINK_SPEED_OFFSET 8
#define LINK_SPEED_WIDTH 4

#define DWS_LOST 0
#define RESET_PROB 1
#define CRC_FAIL 2
#define OPEN_REJ 3

/* SAS command code */
enum sas_cmd_type {
	SAS_DUMP = 0,
	SAS_ANADQ,
	SAS_ANACQ,
	SAS_ERRCODE,
	SAS_DEV,
	SAS_DQE,
};

enum sas_dump_cmd_type {
	DUMP_GLOBAL,
	DUMP_PHYX,
	DUMP_AXI,
	DUMP_UNKNOWN_TYPE,
};

enum sas_anadq_cmd_type {
	ANADQ_PRT,
	ANADQ_NUM,
	ANADQ_UNKNOWN_TYPE,
};

enum sas_anacq_cmd_type {
	/*
	 * cq and dq functions use same interface to communication with TF, their subcommands
	 * must be coded in a unified manner, so ANACQ_PRT is defined as 3.
	 */
	ANACQ_PRT = 3,
	ANACQ_NUM,
	ANACQ_UNKNOWN_TYPE,
};

enum sas_errcode_cmd_type {
	ERRCODE_ALL,
	ERRCODE_DWS_LOST,
	ERRCODE_RESET_PROB,
	ERRCODE_CRC_FAIL,
	ERRCODE_OPEN_REJ,
	ERRCODE_UNKNOWN_TYPE,
};

enum sas_dev_cmd_type {
	DEV_LINK,
	DEV_UNKNOWN_TYPE,
};

enum sas_dqe_cmd_type {
	DQE_INFO,
	DQE_UNKNOWN_TYPE,
};

#endif
