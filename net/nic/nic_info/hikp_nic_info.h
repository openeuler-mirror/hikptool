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

#ifndef __HIKP_NIC_INFO_H__
#define __HIKP_NIC_INFO_H__

#include "hikp_net_lib.h"

#define MAX_DIE_NUM 32

struct nic_info_req_para {
	struct bdf_t bdf;
};

struct nic_info_pf_info {
	uint8_t pf_mode;
	uint8_t mac_id;
	uint8_t mac_type;
	uint8_t rsv1;
	uint16_t func_num;
	uint16_t tqp_num;
	/**********************************************
	 * pf_cap_flag:
	 *     bit0 == ncsi enable status
	 *     bit1~bit31 == reserved
	 **********************************************/
	uint32_t pf_cap_flag;
	uint32_t rsvd[3];
};

enum nic_info_cmd_type {
	CHIP_INFO_DUMP = 0,
};

#define NIC_INFO_RSVD_NUM 10
struct nic_info_rsp_t {
	uint8_t mac_mode;
	uint8_t chip_id;
	uint8_t die_id;
	uint8_t pf_num;
	/**********************************************
	 * cap_flag:
	 *     bit0 == sgpio enable status
	 *     bit1 == tm enable status
	 *     bit2 == torus enable status
	 *     bit3~bit31 == reserved
	 **********************************************/
	uint32_t cap_flag;
	uint32_t rsv1[NIC_INFO_RSVD_NUM];
	struct nic_info_pf_info pf_info[HIKP_MAX_PF_NUM];
};

struct nic_info_param {
	struct tool_target target;
	struct nic_info_rsp_t info;
	char revision_id[MAX_PCI_ID_LEN + 1];
	uint8_t accessed_bus_id[MAX_DIE_NUM];
	uint8_t accessed_die_num;
	uint8_t numvfs;
	bool have_interface;
};

enum nic_info_mac_type {
	MAC_TYPE_ETH = 0,
	MAC_TYPE_ROH,
	MAC_TYPE_UB,
	MAC_TYPE_MAX,
};

#endif
