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

#ifndef SAS_ANALY_DQ_H
#define SAS_ANALY_DQ_H

#include "sas_tools_include.h"

#define REG_NUM_DQ 2
#define REG_NUM_CQ 2
#define REG_NUM_CQ_MAX 4
#define REG_NUM_PTR_MAX 32
#define CQE_NUM_BYTE 0
#define DQE_NUM_REG 2
#define CQ_COAL 1
#define CQ_COAL_TIME 2
#define CQ_COAL_CNT 3
#define CQ_COAL_ENABLE 3

struct sas_analy_para {
	uint32_t chip_id;
	uint32_t die_id;
	uint32_t phy_id;
};

int sas_analy_cmd(struct tool_sas_cmd *cmd);

#endif /* SAS_ANALY_DQ_H */
