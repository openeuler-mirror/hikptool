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

#ifndef HIKP_NIC_TORUS_H
#define HIKP_NIC_TORUS_H

#include "hikp_net_lib.h"

#define NIC_TORUS_MAC_ANTI_SPOOF_EN_MASK	0x1
#define NIC_TORUS_ALW_LPBK_MASK			0x2
#define NIC_TORUS_ALW_LCL_LPBK_MASK		0x4
#define NIC_TORUS_ALW_DST_OVRD_MASK		0x8

#define NIC_TORUS_UC_LAN_PAIR_EN	16
#define NIC_TORUS_MC_BC_LAN_PAIR_EN	17
#define NIC_TORUS_LLDP_LAN_PAIR_EN	18
#define NIC_TORUS_TC2VLANPRI_MAPPING_EN	19
#define NIC_TORUS_LPBK_DROP_EN		20

#define NIC_TORUS_MAC_ID_M	GENMASK(1, 0)
#define NIC_TORUS_MAC_ID_S	0

#define NIC_TORUS_TC0_MAP_TC_M	GENMASK(2, 0)
#define NIC_TORUS_TC0_MAP_TC_S	0
#define NIC_TORUS_TC1_MAP_TC_M	GENMASK(8, 6)
#define NIC_TORUS_TC1_MAP_TC_S	6
#define NIC_TORUS_TC2_MAP_TC_M	GENMASK(14, 12)
#define NIC_TORUS_TC2_MAP_TC_S	12
#define NIC_TORUS_TC3_MAP_TC_M	GENMASK(20, 18)
#define NIC_TORUS_TC3_MAP_TC_S	18

enum nic_torus_sub_cmd_type {
	NIC_TORUS_INFO_DUMP,
};

struct nic_torus_req_para {
	struct bdf_t bdf;
	uint8_t block_id;
	uint8_t rsv[3];
};

struct nic_torus_rsp_head {
	uint8_t total_blk_num;
	uint8_t curr_blk_size; /* real data size, not contain head size. */
	uint16_t rsv;
};

#define NIC_TORUS_MAX_RSP_DATA    6
struct nic_torus_rsp {
	struct nic_torus_rsp_head head;
	uint32_t data[NIC_TORUS_MAX_RSP_DATA];
};

struct nic_torus_info {
	uint32_t enable;
	uint32_t lan_prt_pair;
	uint32_t lan_fwd_tc_cfg;
	uint32_t pause_time_out;
	uint8_t pause_time_out_en;
	uint8_t vlan_fe;
	uint8_t nic_switch_param;
	uint8_t roce_switch_param;
	uint32_t ets_tcg0_mapping;
};

int hikp_nic_torus_get_target(struct major_cmd_ctrl *self, const char *argv);
void hikp_nic_torus_cmd_execute(struct major_cmd_ctrl *self);
#endif /* HIKP_NIC_TORUS_H */
