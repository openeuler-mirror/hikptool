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

#ifndef __HIKP_ROCE_TIMER_H__
#define __HIKP_ROCE_TIMER_H__

#include "hikp_roce_ext_common.h"

#define ROCE_HIKP_TIMER_REG_NUM 25
#define ROCE_TIMER_CMD_CLEAR (1 << 0)

enum roce_timer_cmd_type {
	TIMER_SHOW_QPC = 0x1,
	TIMER_SHOW_CQC,
	TIMER_QPC_CLEAR,
	TIMER_CQC_CLEAR,
};

struct cmd_roce_timer_params {
	struct tool_target target;
	uint8_t flag;
};

struct roce_timer_req_para {
	struct bdf_t bdf;
};

struct roce_timer_rsp_data {
	uint32_t reg_num;
	uint32_t timer_content[ROCE_HIKP_TIMER_REG_NUM][2];
};

#endif /* __HIKP_ROCE_TIMER_H__ */
