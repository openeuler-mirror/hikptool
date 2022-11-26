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

#ifndef HIKP_NIC_RSS_H
#define HIKP_NIC_RSS_H

#include "hikp_net_lib.h"

enum nic_rss_sub_cmd_type {
	RSS_ALGO_DUMP = 0,
	RSS_KEY_DUMP,
	RSS_TUPLE_DUMP,
	RSS_RETA_DUMP,
	RSS_TC_MODE_DUMP,
};

#define HIKP_RSS_RETA_SIZE_MAX 2048
struct rss_reta_info {
	uint16_t reta_size;
	uint16_t rsv;
	uint16_t reta[HIKP_RSS_RETA_SIZE_MAX];
};

#define HIKP_RSS_HASH_KEY_LEN_MAX 128
struct rss_hash_key {
	uint16_t key_len;
	uint16_t rsv;
	uint8_t key[HIKP_RSS_HASH_KEY_LEN_MAX];
};

struct rss_tc_mode_entry {
	uint16_t tc_offset;
	uint16_t tc_size;
	uint8_t tc_valid;
	uint8_t rsv[3];
};

struct rss_tuple_fields {
	uint64_t tuple_field;  /* IPV4: LOW 32BIT, IPV6: HIGH 32BIT */
	uint64_t rsv;
};

#define HIKP_RSS_HASH_TOEPLITZ           0
#define HIKP_RSS_HASH_SIMPLE_XOR         1
#define HIKP_RSS_HASH_SYMMETRIC_TOEPLITZ 2

union nic_rss_feature_info {
	uint8_t hash_algo;
	struct rss_hash_key hash_key;
	struct rss_tuple_fields tuples;
	struct rss_reta_info reta_info;
	struct rss_tc_mode_entry tc_mode_entry[HIKP_NIC_MAX_TC_NUM];
};

struct nic_rss_rsp_head {
	uint8_t total_blk_num;
	uint8_t cur_blk_size;  /* real data size, not contain head size. */
	uint16_t rsv;
};

#define NIC_RSS_MAX_RSP_DATA  59
struct nic_rss_rsp {
	struct nic_rss_rsp_head rsp_head; /* 4 Byte */
	uint32_t rsp_data[NIC_RSS_MAX_RSP_DATA];
};

struct nic_rss_req_para {
	struct bdf_t bdf;
	uint8_t block_id;
};

struct nic_rss_param {
	struct tool_target target;
	int feature_idx;
};

#define HIKP_RSS_MAX_FEATURE_NAME_LEN 20
struct rss_feature_cmd {
	const char feature_name[HIKP_RSS_MAX_FEATURE_NAME_LEN];
	uint32_t sub_cmd_code;
	void (*show)(const void *data);
};

#endif /* HIKP_NIC_RSS_H */
