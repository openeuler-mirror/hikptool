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

#ifndef HIKP_NIC_NCSI_H
#define HIKP_NIC_NCSI_H

#include "hikp_net_lib.h"

struct nic_ncsi_dfx_info {
	uint16_t ncsi_control_total;
	uint16_t ncsi_eth_to_ub_total;
	uint16_t ncsi_ub_to_eth_total;
	uint16_t ncsi_control_good;
	uint16_t ncsi_eth_to_ub_good;
	uint16_t ncsi_ub_to_eth_good;
	uint16_t ncsi_eth_to_ub_arp;
	uint16_t ncsi_eth_to_ub_free_arp;
	uint16_t ncsi_eth_to_ub_ipv6_ra;
	uint16_t ncsi_eth_to_ub_dhcpv4;
	uint16_t ncsi_eth_to_ub_dhcpv6;
	uint16_t ncsi_eth_to_ub_lldp;
	uint16_t ncsi_ub_to_eth_ipv4;
	uint16_t ncsi_ub_to_eth_ipv6;
	uint16_t ncsi_ub_to_eth_ipnotify;
	uint16_t ncsi_ub_to_eth_dhcpv4;
	uint16_t ncsi_ub_to_eth_dhcpv6;
	uint16_t ncsi_ub_to_eth_lldp;
};

struct nic_ncsi_cmd_resp {
	uint8_t ncsi_en;
	uint8_t rsv0[3];
	struct nic_ncsi_dfx_info ncsi_dfx;
	uint32_t rsv1[50];    /* max resp data: 240 Bytes */
};

struct nic_ncsi_cmd_req {
	struct bdf_t bdf;
	uint32_t rsv0[30];    /* max req data: 128 Bytes */
};

struct nic_ncsi_cmd_info {
	struct tool_target target;
	bool port_flag;
};

#endif  /* HIKP_NIC_NCSI_H */
