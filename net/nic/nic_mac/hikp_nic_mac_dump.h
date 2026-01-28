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

#define PER_BLK_DATA_SIZE	30
#define MAX_REG_NAME_LEN	64
#define ROUND_UP(val, base)	(((val) + (base) - 1) / (base))

struct mac_reg_name {
	const char name[MAX_REG_NAME_LEN];
};

struct mac_reg_info {
	const struct mac_reg_name* reg_name;
	uint8_t reg_num;
};

struct mac_type_name_parse {
	const struct mac_reg_info *reg_info_list;
	uint8_t data_len;
	uint8_t blk_size;
	bool is_blk;
};

struct mac_dump_mod_proc {
	uint32_t module_id;
	uint32_t sub_cmd;
	const char *name;
	const struct mac_type_name_parse *mac_name_parse;
	uint32_t mac_name_parse_size;
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

struct nic_mac_collect_param {
	const char *net_dev_name;
	const char *module_name;
};

int hikp_info_collect_nic_mac(void *data);

int mac_cmd_dump_module_cfg(struct major_cmd_ctrl *self, const char *argv);
int mac_cmd_dump_reg_target(struct major_cmd_ctrl *self, const char *argv);
void mac_cmd_dump_execute(struct major_cmd_ctrl *self);
#endif /* HIKP_NIC_MAC_DUMP_H */
