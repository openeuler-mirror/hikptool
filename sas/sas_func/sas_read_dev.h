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

#ifndef __SAS_DEV_H_
#define __SAS_DEV_H_

#include "sas_tools_include.h"

struct sas_dev_req_para {
	uint32_t chip_id;
	uint32_t die_id;
	uint32_t dev_id;
};

struct hikp_sas_itct_dw0 {
	uint64_t dev_type : 2;
	uint64_t dev_valid : 1;
	uint64_t break_reply_en : 1;
	uint64_t reserved0 : 12;
	uint64_t smp_timeout : 8;
	uint64_t tlr_en : 1;
	uint64_t awt_continue : 1;
	uint64_t reserved : 38;
};

struct hikp_sas_itct_dw2 {
	uint64_t I_T_nexus_loss : 16;
	uint64_t awt_initial_value : 16;
	uint64_t maximum_connect_time : 16;
	uint64_t reject_to_open_limit : 16;
};

struct hikp_sas_itct {
	struct hikp_sas_itct_dw0 dw0;
	uint64_t sas_addr;
	struct hikp_sas_itct_dw2 dw2;
};

int sas_dev(const struct tool_sas_cmd *cmd);

#endif
