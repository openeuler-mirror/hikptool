/*
 * Copyright (c) 2023 Hisilicon Technologies Co., Ltd.
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

#ifndef __HIKP_ROCE_GLOBAL_CFG_H__
#define __HIKP_ROCE_GLOBAL_CFG_H__

#include "hikp_roce_ext_common.h"

struct cmd_roce_global_cfg_param {
	struct tool_target target;
	uint32_t sub_cmd;
};

struct roce_global_cfg_req_param {
	struct bdf_t bdf;
	uint32_t block_id;
};

enum roce_global_cfg_cmd_type {
	ROCE_GLB_GENAC = 0,
	ROCE_GLB_TRP_BANK,
	ROCE_GLB_TRP_RX,
	ROCE_GLB_TPP_M,
	ROCE_GLB_QMM,
	ROCE_GLB_TGP_TMP,
	ROCE_GLB_TDP_M,
	ROCE_GLB_NICL,
};

#endif /* __HIKP_ROCE_GLOBAL_CFG_H__ */
