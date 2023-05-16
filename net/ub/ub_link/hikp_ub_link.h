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

#ifndef HIKP_UB_LINK_H
#define HIKP_UB_LINK_H

#include "hikp_net_lib.h"

enum ub_link_sub_cmd_type {
	UB_LINK_INFO_DUMP = 0,
};

struct ub_link_param {
	struct tool_target target;
};

struct ub_link_req_paras {
	struct bdf_t bdf;
};

struct ub_link_rsp {
	uint8_t mac_id;
	uint8_t hdlc_link;
	uint8_t hpcs_link;
	uint8_t rsvd;
	uint32_t hdlc_link_fsm;
	uint32_t hpcs_link_fsm;
};

#endif /* HIKP_UB_LINK_H */
