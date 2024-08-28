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

#ifndef HIKP_NIC_MAC_DUMP_H
#define HIKP_NIC_MAC_DUMP_H

#include "hikp_net_lib.h"

struct mac_dump_mod_proc {
	uint32_t module_id;
	uint32_t sub_cmd;
	const char *name;
};

enum dump_module {
	MOD_RX_MAC = 0,
	MOD_RX_PCS,
	MOD_RX_RSFEC,
	MOD_RX_BRFEC,
	MOD_RXPMA_CORE,
	MOD_RXPMA_LANE,
	MOD_TXPMA_LANE,
	MOD_TXPMA_CORE,
	MOD_TX_BRFEC,
	MOD_TX_RSFEC,
	MOD_TX_PCS,
	MOD_TX_MAC,
	MOD_MIB,
	MOD_COM,
	MOD_GE,
	MOD_MAC_COMM,
	MOD_AN,
	MOD_LT,
	MOD_ID_MAX,
};

struct dump_reg_req {
	struct bdf_t bdf;
	uint32_t blk_id;
};

struct reg_rsp_info {
	uint64_t addr : 16;
	uint64_t val : 48;
};

struct cmd_mac_dump {
	struct tool_target target;
	bool port_flag;
	uint32_t blk_num[MOD_ID_MAX];
	const char *module_name;
};
#endif /* HIKP_NIC_MAC_DUMP_H */
