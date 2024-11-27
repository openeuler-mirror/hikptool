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

#ifndef HIKP_NIC_PPP_H
#define HIKP_NIC_PPP_H

#include "hikp_net_lib.h"

enum nic_ppp_sub_cmd_type {
	NIC_PPP_HW_RES_DUMP = 0,
	NIC_MAC_TBL_DUMP,
	NIC_VLAN_TBL_DUMP,
	NIC_MNG_TBL_DUMP,
	NIC_PROMISCUOUS_TBL_DUMP,
	NIC_VLAN_OFFLOAD_DUMP,
};

#define HIKP_NIC_ENTRY_FUNC_BITMAP_CNT 8

struct hikp_nic_ppp_hw_resources {
	uint32_t max_key_mem_size;
	uint32_t overflow_cam_size;
	uint32_t mng_tbl_size;
	uint16_t port_vlan_tbl_size;
	uint16_t vf_vlan_tbl_size;
	/* port information */
	uint16_t total_func_num;    /* contain PF and VF. */
	uint16_t abs_func_id_base;  /* The absolute func_id of the first VF in this port. */
	uint32_t mac_id;
	uint32_t rsv1[10];
};

/* struct mac_vlan_uc_entry::e_vport field definition:
 * bit[0-2]: pf_id
 * bit[3-10]: vf_id
 */
#define HIKP_NIC_PF_ID_MASK 0x7
#define HIKP_NIC_PF_ID_S    0
#define HIKP_NIC_VF_ID_MASK 0x7f8
#define HIKP_NIC_VF_ID_S    3

struct mac_vlan_uc_entry {
	uint32_t idx;
	uint8_t mac_addr[HIKP_NIC_ETH_MAC_ADDR_LEN];
	uint16_t valid;
	uint16_t ingress_port;
	uint16_t vlan_id;
	uint16_t mac_en;
	uint16_t vmdq1;
	uint16_t u_m;
	uint16_t e_vport_type;
	uint16_t e_vport;
	uint16_t rsv;
};

struct mac_vlan_mc_entry {
	uint32_t idx;
	uint8_t mac_addr[HIKP_NIC_ETH_MAC_ADDR_LEN];
	uint16_t rsv;
	uint32_t function_bitmap[HIKP_NIC_ENTRY_FUNC_BITMAP_CNT];
};

/* Data from firmware is unicast MAC table entry, entry_size is accumulated on tool side. */
struct mac_vlan_uc_tbl {
	uint32_t entry_size;
	struct mac_vlan_uc_entry *entry;
};

/* Data from firmware is multicast MAC table entry, entry_size is accumulated on tool side. */
struct mac_vlan_mc_tbl {
	uint32_t entry_size;
	struct mac_vlan_mc_entry *entry;
};

struct nic_mac_tbl {
	struct mac_vlan_uc_tbl uc_tbl;
	struct mac_vlan_mc_tbl mc_tbl;
};

struct port_vlan_tbl_entry {
	uint16_t vlan_id;
	uint8_t port_bitmap;
	uint8_t rsv;
};

/* Data from firmware is port vlan table entry, entry_size is accumulated on tool side. */
struct port_vlan_tbl {
	uint32_t entry_size;
	struct port_vlan_tbl_entry *entry;
};

struct vf_vlan_tbl_entry {
	uint16_t vlan_id;
	uint16_t rsv;
	uint32_t func_bitmap[HIKP_NIC_ENTRY_FUNC_BITMAP_CNT]; /* 256bits */
};

/* Data from firmware is vf vlan table entry, entry_size is accumulated on tool side. */
struct vf_vlan_tbl {
	uint32_t entry_size;
	struct vf_vlan_tbl_entry *entry;
};

struct nic_vlan_tbl {
	struct vf_vlan_tbl vf_vlan_tbl;
	struct port_vlan_tbl port_vlan_tbl;
};

struct manager_entry {
	uint32_t entry_no;
	uint8_t mac_addr[HIKP_NIC_ETH_MAC_ADDR_LEN];
	uint8_t mac_mask;
	uint8_t ether_mask;
	uint16_t ether_type;
	uint16_t vlan_id;
	uint8_t vlan_mask;
	uint8_t i_port_bitmap;
	uint8_t i_port_dir;
	/* 1: Drop the packet when match, 0: Forward the packet to E_vport/queue_id when match. */
	uint8_t drop;
	/* port type value
	 * 1: Network port(corresponding to MAC, the one facing the external switch),
	 * 0: RX egress port
	 */
	uint8_t e_port_type;
	uint8_t pf_id;
	uint16_t vf_id;
	uint16_t q_id;
	uint16_t rsv;
};

/* Data from firmware is manager entry, entry_size is accumulated on tool side. */
struct nic_mng_tbl {
	uint32_t entry_size;
	struct manager_entry *entry;
};

struct func_vlan_offload_cfg {
	uint16_t vlan_fe;
	uint16_t pvid;
	uint8_t port_vlan_bypass; /* 0: off, 1: on, 2: unsupport port vlan */
	uint8_t accept_tag1;
	uint8_t accept_tag2;
	uint8_t accept_untag1;
	uint8_t accept_untag2;
	uint8_t insert_tag1;
	uint8_t insert_tag2;
	uint8_t shift_tag;
	uint8_t strip_tag1;
	uint8_t strip_tag2;
	uint8_t drop_tag1;
	uint8_t drop_tag2;
	uint8_t pri_only_tag1;
	uint8_t pri_only_tag2;
	uint8_t rsv[2];
};

#define HIKP_FILTER_FE_NIC_INGRESS_B    HI_BIT(0)
#define HIKP_FILTER_FE_NIC_EGRESS_B     HI_BIT(1)
struct nic_vlan_offload_cfg {
	uint8_t port_vlan_fe;
	uint8_t rsv;
	uint16_t func_num;  /* contain PF and VFs */
	struct func_vlan_offload_cfg func[HIKP_NIC_MAX_FUNC_NUM];
};

struct func_promisc_cfg {
	uint8_t func_id;
	uint8_t uc_en;
	uint8_t mc_en;
	uint8_t bc_en;
};

struct nic_promisc_tbl {
	uint16_t func_num;
	uint16_t rsv;
	struct func_promisc_cfg func[HIKP_NIC_MAX_FUNC_NUM];
};

union nic_ppp_feature_info {
	struct nic_mac_tbl mac_tbl;
	struct nic_vlan_tbl vlan_tbl;
	struct nic_mng_tbl mng_tbl;
	struct nic_promisc_tbl promisc_tbl;
	struct nic_vlan_offload_cfg vlan_offload;
};

struct nic_ppp_rsp_head {
	uint8_t total_blk_num;
	uint8_t cur_blk_size;  /* real data size, not contain head size. */
	uint16_t rsv;
	/* firmware must set following fields when query MAC/VLAN/MNG table. */
	uint32_t next_entry_idx;
	uint32_t cur_blk_entry_cnt;
};

#define NIC_PPP_MAX_RSP_DATA        57
struct nic_ppp_rsp {
	struct nic_ppp_rsp_head rsp_head; /* 12 Byte */
	uint32_t rsp_data[NIC_PPP_MAX_RSP_DATA];
};

struct nic_ppp_req_para {
	struct bdf_t bdf;
	uint8_t block_id;
	union {
		uint8_t is_unicast; /* 1: uc MAC, 0: mc MAC. */
		uint8_t is_port_vlan; /* 1: port vlan table, 0: vf vlan table. */
		uint8_t rsv; /* firmware ignores it if isn't used to query MAC/VLAN table. */
	};
	uint8_t rsv1[2];
	uint32_t cur_entry_idx; /* firmware queries MAC/VLAN/MNG table from the valuue. */
};

struct nic_ppp_param {
	struct tool_target target;
	int feature_idx;
	/* 'func_id' is used to control whether to
	 * query all entries or the entry of a specified function.
	 * It will be used when query MAC or VLAN table. Value range:
	 *    -1: query all entries
	 *     0: means PF
	 *   > 0: means VF
	 */
	int func_id;
	int is_uc; /* Must be specified when query one function entry for mac or vlan cmd. */
};

#define HIKP_PPP_MAX_FEATURE_NAME_LEN   20
struct ppp_feature_cmd {
	const char feature_name[HIKP_PPP_MAX_FEATURE_NAME_LEN];
	uint32_t sub_cmd_code;
	bool need_query_hw_res;
	int (*query)(struct hikp_cmd_header *req_header,
		     const struct bdf_t *bdf, void *data, size_t len);
	void (*show)(const void *data);
};

#endif /* HIKP_NIC_PPP_H */
