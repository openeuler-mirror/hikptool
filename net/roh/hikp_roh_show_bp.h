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

#ifndef HIKP_ROH_SHOW_BP_H
#define HIKP_ROH_SHOW_BP_H

#include "hikp_net_lib.h"

struct cmd_roh_show_bp_param {
	struct tool_target target;
	uint8_t flag;
};

struct roh_show_bp_req_paras {
	struct bdf_t bdf;
};

union bp_val {
	uint32_t val;
	struct {
		uint32_t st_ssu_pfc_mac0 : 8;
		uint32_t st_ssu_pfc_mac1 : 8;
		uint32_t st_roh_egu_tx_bp_mac0 : 1;
		uint32_t st_roh_egu_tx_bp_mac1 : 1;
		uint32_t st_hllc_roh_flit_bp_mac0 : 1;
		uint32_t st_hllc_roh_flit_bp_mac1 : 1;
		uint32_t rsv : 12;
	} _val;
};

struct roh_show_bp_rsp_t {
	uint8_t mac_id;
	uint32_t bp_val;
};

#define ROH_CMD_SHOW_BP 1
#define VERIFY_MAC_ID 2
#define BP_SIZE 8

#endif /* HIKP_ROH_SHOW_BP_H */
