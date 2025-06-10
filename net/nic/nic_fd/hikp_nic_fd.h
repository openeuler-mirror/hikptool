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

#ifndef HIKP_NIC_FD_H
#define HIKP_NIC_FD_H

#include "hikp_net_lib.h"

/* OUTER_XXX indicates tuples in tunnel header of tunnel packet
 * INNER_XXX indicate tuples in tunneled header of tunnel packet or
 *           tuples of non-tunnel packet
 */
enum nic_fd_tuple {
	OUTER_DST_MAC = 0,
	OUTER_SRC_MAC,
	OUTER_VLAN_TAG_FST,
	OUTER_VLAN_TAG_SEC,
	OUTER_ETH_TYPE,
	OUTER_L2_RSV,
	OUTER_IP_TOS,
	OUTER_IP_PROTO,
	OUTER_SRC_IP,
	OUTER_DST_IP,
	OUTER_L3_RSV,
	OUTER_SRC_PORT,
	OUTER_DST_PORT,
	OUTER_L4_RSV,
	OUTER_TUN_VNI,
	OUTER_TUN_FLOW_ID,
	INNER_DST_MAC,
	INNER_SRC_MAC,
	INNER_VLAN_TAG_FST,
	INNER_VLAN_TAG_SEC,
	INNER_ETH_TYPE,
	INNER_L2_RSV,
	INNER_IP_TOS,
	INNER_IP_PROTO,
	INNER_SRC_IP,
	INNER_DST_IP,
	INNER_L3_RSV,
	INNER_SRC_PORT,
	INNER_DST_PORT,
	INNER_L4_RSV,
	MAX_TUPLE,
};

enum nic_fd_meta_data {
	PACKET_TYPE_ID = 0,
	IP_FRAGEMENT,
	ROCE_TYPE,
	NEXT_KEY,
	VLAN_NUMBER,
	SRC_VPORT,
	DST_VPORT,
	TUNNEL_PACKET,
	MAX_META_DATA,
};

enum nic_fd_sub_cmd_type {
	NIC_FD_HW_INFO_DUMP = 0,
	NIC_FD_RULES_INFO_DUMP,
	NIC_FD_COUNTER_STATS_DUMP,
};

enum HNS3_FD_KEY_TYPE {
	HNS3_FD_KEY_BASE_ON_PTYPE,
	HNS3_FD_KEY_BASE_ON_TUPLE,
};

#define NIC_KEY_LEN_400B    400
#define NIC_KEY_LEN_200B    200

enum nic_fd_mode {
	FD_MODE_DEPTH_2K_WIDTH_400B_STAGE_1 = 0,
	FD_MODE_DEPTH_1K_WIDTH_400B_STAGE_2,
	FD_MODE_DEPTH_4K_WIDTH_200B_STAGE_1,
	FD_MODE_DEPTH_2K_WIDTH_200B_STAGE_2,
};

enum nic_fd_stage {
	NIC_FD_STAGE_1 = 0,
	NIC_FD_STAGE_2,
	NIC_FD_STAGE_NUM,
};

struct nic_fd_alloc {
	uint32_t stage_entry_num[NIC_FD_STAGE_NUM];
	uint16_t stage_counter_num[NIC_FD_STAGE_NUM];
};

struct nic_fd_key_cfg {
	uint8_t stage;
	uint8_t key_select;
	uint8_t inner_src_ipv6_word_en;
	uint8_t inner_dest_ipv6_word_en;
	uint8_t outer_src_ipv6_word_en;
	uint8_t outer_dest_ipv6_word_en;
	uint8_t rsv1[2];
	uint32_t tuple_mask;
	uint32_t meta_data_mask;
};

struct nic_fd_hw_info {
	uint8_t mode;
	uint8_t enable;
	/* Max key bit width hwardware supported and unrelated to mode. */
	uint16_t key_max_bit;
	struct nic_fd_alloc alloc;
	struct nic_fd_key_cfg key_cfg[NIC_FD_STAGE_NUM];
};

#define NIC_FD_AD_DATA_S                32
#define NIC_FD_AD_DROP_B                0
#define NIC_FD_AD_DIRECT_QID_B          1
#define NIC_FD_AD_QID_S                 2
#define NIC_FD_AD_QID_M                 GENMASK(11, 2)
#define NIC_FD_AD_USE_COUNTER_B         12
#define NIC_FD_AD_COUNTER_NUM_S         13
#define NIC_FD_AD_COUNTER_NUM_M         GENMASK(19, 13)
#define NIC_FD_AD_NXT_STEP_B            20
#define NIC_FD_AD_NXT_KEY_S             21
#define NIC_FD_AD_NXT_KEY_M             GENMASK(25, 21)
#define NIC_FD_AD_WR_RULE_ID_B          0
#define NIC_FD_AD_RULE_ID_S             1
#define NIC_FD_AD_RULE_ID_M             GENMASK(12, 1)
#define NIC_FD_AD_QUEUE_REGION_EN_B     16
#define NIC_FD_AD_QUEUE_REGION_SIZE_S	17
#define NIC_FD_AD_QUEUE_REGION_SIZE_M   GENMASK(20, 17)
#define NIC_FD_AD_COUNTER_HIGH_BIT      7
#define NIC_FD_AD_COUNTER_HIGH_BIT_B	26
#define NIC_FD_AD_QUEUE_ID_HIGH_BIT     10
#define NIC_FD_AD_QUEUE_ID_HIGH_BIT_B	21

#define HIKP_DWORDS_BYTE        4

#define HIKP_NIC_KEY_DIR_NUM    2
struct nic_fd_rule_info {
	uint32_t idx; /* relative idx */
	uint8_t valid;
	uint8_t rsv[3];
	uint32_t ad_data_l;
	uint32_t ad_data_h;
	/* Note: TCAM data must be an integer multiple of 4. This is an agreement with the firmware.
	 * The memory will be allocated to save key_x and key_y.
	 * Memory order: first key_x, then key_y.
	 */
	uint8_t tcam_data[0];  /* first key_x, then key_y memory */
};

/* Data from firmware is rule info, rule_cnt is accumulated on tool side. */
struct nic_fd_rules {
	uint32_t rule_cnt;
	struct nic_fd_rule_info *rule;
};

struct nic_counter_entry {
	uint16_t idx;
	uint16_t rsv1[3];
	uint64_t value;
};

/* Data from firmware is counter entry, counter_size is accumulated on tool side. */
struct nic_fd_counter {
	uint32_t counter_size;
	struct nic_counter_entry *entry;
};

union nic_fd_feature_info {
	struct nic_fd_hw_info hw_info;
	struct nic_fd_rules rules[NIC_FD_STAGE_NUM];
	struct nic_fd_counter counter[NIC_FD_STAGE_NUM];
};

struct nic_fd_rsp_head {
	uint8_t total_blk_num;
	uint8_t cur_blk_size;  /* real data size, not contain head size. */
	uint16_t rsv1;
	/* firmware must set following fields when query fd rules and counter. */
	uint32_t next_entry_idx;
	uint16_t cur_blk_entry_cnt;
	uint16_t rsv2;
};

#define NIC_FD_MAX_RSP_DATA        57
struct nic_fd_rsp {
	struct nic_fd_rsp_head rsp_head; /* 12 Byte */
	uint32_t rsp_data[NIC_FD_MAX_RSP_DATA];
};

struct nic_fd_req_para {
	struct bdf_t bdf;
	uint8_t block_id;
	uint8_t stage;  /* 0: stage1, 1: stage2 */
	uint8_t query_single_entry;
	uint8_t rsv1;
	uint32_t cur_entry_idx;
};

struct nic_fd_param {
	struct tool_target target;
	int feature_idx;
	/* The following fields for querying one rule or counter entry. */
	int id;
	int stage_no;
	bool query_single_entry;
};

#define HIKP_FD_MAX_FEATURE_NAME_LEN   20
struct fd_feature_cmd {
	const char feature_name[HIKP_FD_MAX_FEATURE_NAME_LEN];
	uint32_t sub_cmd_code;
	bool need_query_hw_spec;
	int (*query)(struct hikp_cmd_header *req_header, const struct bdf_t *bdf,
		     uint8_t stage, void *data, size_t len);
	void (*show)(const void *data);
};

int hikp_nic_cmd_get_fd_target(struct major_cmd_ctrl *self, const char *argv);
void hikp_nic_fd_cmd_execute(struct major_cmd_ctrl *self);
void hikp_nic_set_fd_idx(int feature_idx, int stage_no);

#endif /* HIKP_NIC_FD_H */
