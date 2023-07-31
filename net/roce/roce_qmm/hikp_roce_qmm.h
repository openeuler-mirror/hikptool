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

#ifndef __HIKP_ROCE_QMM_H__
#define __HIKP_ROCE_QMM_H__

#include "hikp_roce_ext_common.h"

#define ROCE_HIKP_QMM_REG_NUM 36
#define QMM_BANK_NUM 0x7

struct cmd_roce_qmm_param_t {
	struct tool_target target;
	uint32_t bank_id;
};

struct roce_qmm_rsp_data {
	uint32_t reg_num;
	uint32_t qmm_content[ROCE_HIKP_QMM_REG_NUM][2];
};

struct roce_qmm_req_para {
	struct bdf_t bdf;
	uint32_t bank_id;
};

enum roce_qmm_cmd_type {
	QMM_SHOW_CQC = 0x1,
	QMM_SHOW_QPC = 0x2,
	QMM_SHOW_TOP = 0x3,
};

#endif /* __HIKP_ROCE_QMM_H__ */
