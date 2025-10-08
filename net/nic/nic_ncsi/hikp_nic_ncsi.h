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

struct nic_ncsi_tx_buf_resp {
	uint8_t tx_buf_empty : 1;
	uint8_t ctrl_sof : 1;
	uint8_t ctrl_eof : 1;
	uint8_t ctrl_err : 1;
	uint8_t ctrl_byte_sel : 2;
	uint8_t rsv0 : 2;
	uint8_t rsv1[7];
	uint32_t rsv2[58];    /* max resp data: 240 Bytes */
};

#define NIC_NCSI_VLAN_ENTRY_NUM		16
struct ncsi_vlan_entry {
	uint32_t vlan_id : 16;
	uint32_t entry_en : 1;
	uint32_t rsv : 15;
};

struct nic_ncsi_vlan_filter_resp {
	uint32_t filter_en_map : 16;
	uint32_t filer_type : 8;
	uint32_t rsv0 : 8;
	uint32_t entry_to_bmc_only_map : 16;
	uint32_t entry_to_bmc_map : 16;

	struct ncsi_vlan_entry entry[NIC_NCSI_VLAN_ENTRY_NUM];
	uint32_t rsv1[42];    /* max resp data: 240 Bytes */
};

#define NIC_NCSI_ETHER_ENTRY_NUM	4
struct ncsi_ether_entry {
	uint32_t entry_type : 16;
	uint32_t entry_en : 1;
	uint32_t rsv : 15;
};

struct nic_ncsi_ether_filter_resp {
	uint32_t ether_en_map : 4;
	uint32_t entry_to_bmc_only_map : 4;
	uint32_t entry_to_bmc_map : 4;
	uint32_t rsv0 : 20;
	uint32_t rsv1;
	struct ncsi_ether_entry entry[NIC_NCSI_ETHER_ENTRY_NUM];
	uint32_t rsv2[54];    /* max resp data: 240 Bytes */
};

#define NIC_NCSI_DMAC_ENTRY_NUM		8
struct ncsi_dmac_entry {
	uint32_t entry_cfg_l;
	uint32_t entry_cfg_h : 16;
	uint32_t entry_type : 2;
	uint32_t entry_en : 1;
	uint32_t rsv : 13;
};

struct nic_ncsi_dmac_filter_resp {
	uint32_t dmac_en_map : 8;
	uint32_t dmac_to_bmc_only : 8;
	uint32_t dmac_to_bmc : 8;
	uint32_t rsv0 : 8;
	uint32_t mc_ipv6_neighbor_en : 1;
	uint32_t mc_ipv6_route_en : 1;
	uint32_t mc_dhcpv6_relay_en : 1;
	uint32_t mc_to_bmc : 1;
	uint32_t mc_to_bmc_only : 1;
	uint32_t rsv1 : 27;
	uint32_t bc_arp_en : 1;
	uint32_t bc_dhcp_client : 1;
	uint32_t bc_dhcp_server : 1;
	uint32_t bc_netbios_en : 1;
	uint32_t bc_to_bmc : 1;
	uint32_t bc_to_bmc_only : 1;
	uint32_t rsv2 : 26;
	uint32_t rsv3;
	struct ncsi_dmac_entry entry[NIC_NCSI_DMAC_ENTRY_NUM];
	uint32_t rsv4[40];    /* max resp data: 240 Bytes */
};

#define NIC_NCSI_SMAC_ENTRY_NUM		8
struct ncsi_smac_entry {
	uint32_t entry_cfg_l;
	uint32_t entry_cfg_h : 16;
	uint32_t entry_en : 1;
	uint32_t entry_dport : 3;
	uint32_t entry_to_mac : 1;
	uint32_t rsv : 11;
};

struct nic_ncsi_smac_filter_resp {
	uint32_t smac_en_map : 8;
	uint32_t pt_pkt_en : 1;
	uint32_t rsv0 : 23;
	uint32_t rsv1;
	struct ncsi_smac_entry entry[NIC_NCSI_SMAC_ENTRY_NUM];
	uint32_t rsv2[42];    /* max resp data: 240 Bytes */
};

struct nic_ncsi_cmd_req {
	struct bdf_t bdf;
	uint32_t rsv0[30];    /* max req data: 128 Bytes */
};

struct ncsi_dump_mod_proc {
	const char *name;
	void (*show)(struct major_cmd_ctrl *self);
};

#define NCSI_PORT_TARGET_BIT	HI_BIT(0)
#define NCSI_DUMP_MODULE_BIT	HI_BIT(1)

struct nic_ncsi_cmd_info {
	struct tool_target target;
	uint32_t cmd_flag;
	const char *module_name;
};

struct nic_ncsi_collect_param {
	const char *net_dev_name;
};

int hikp_info_collect_nic_ncsi(void *data);

#endif  /* HIKP_NIC_NCSI_H */
