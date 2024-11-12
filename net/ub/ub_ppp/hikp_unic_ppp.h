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

#ifndef HIKP_UNIC_PPP_H
#define HIKP_UNIC_PPP_H

#include "hikp_net_lib.h"

#define HIKP_UNIC_IP_ADDR_FMT_SIZE	50
#define MAX_IP_ADDR_STR_LEN		50
#define IP_ADDR_LEN			16
#define IP_ADDR_TBL_LEN			8

#define HIKP_UNIC_IP_ADDR_LEN		4
#define HIKP_UNIC_GUID_BITMAP_LEN	8
#define HIKP_UNIC_GUID_ADDR_LEN 	16

#define UNIC_PPP_MAX_RSP_DATA			57
#define HIKP_UNIC_PPP_MAX_FEATURE_NAME_LEN	20

#define UNIC_PPP_IP_TBL_NAME	"ip"
#define UNIC_PPP_GUID_TBL_NAME	"guid"

enum unic_ppp_sub_cmd_type {
	UNIC_PPP_ENTRY_HW_SPEC_GET = 0,
	UNIC_IP_TBL_DUMP,
	UNIC_GUID_TBL_DUMP,
};

enum unic_ppp_feature_idx {
	UNIC_PPP_IP_FEATURE_IDX = 0,
	UNIC_PPP_GUID_FEATURE_IDX,
	UNIC_PPP_INIT_FEATURE_IDX = -1,
};

struct hikp_unic_ppp_hw_resources {
	uint16_t uc_guid_tbl_size;
	uint16_t mc_guid_tbl_size;
	uint32_t ip_max_mem_size;
	uint32_t ip_overflow_size;
	uint32_t rsv;
	/* port information */
	uint16_t total_func_num;    /* contain PF and VF. */
	uint16_t abs_func_id_base;  /* The absolute func_id of the first VF in this port. */
	uint32_t rsv1[11];
};

struct unic_ip_entry {
	uint32_t index;
	uint32_t function_id;
	uint32_t ip_addr[HIKP_UNIC_IP_ADDR_LEN];
};

struct unic_ip_tbl {
	uint32_t entry_size;
	struct unic_ip_entry *entry;
};

struct unic_guid_uc_entry {
	uint32_t function_id;
	uint8_t guid_addr[HIKP_UNIC_GUID_ADDR_LEN];
};

struct unic_guid_uc_tbl {
	uint32_t entry_size;
	struct unic_guid_uc_entry *entry;
};

struct unic_guid_mc_entry {
	uint32_t idx;
	uint8_t guid_addr[HIKP_UNIC_GUID_ADDR_LEN];
	uint32_t function_bitmap[8];
};

struct unic_guid_mc_tbl {
	uint32_t entry_size;
	struct unic_guid_mc_entry *entry;
};

struct unic_guid_tbl {
	struct unic_guid_uc_tbl uc_tbl;
	struct unic_guid_mc_tbl mc_tbl;
};

union unic_ppp_feature_info {
	struct unic_guid_tbl guid_tbl;
	struct unic_ip_tbl ip_tbl;
};

struct unic_ppp_feature_cmd {
	const char feature_name[HIKP_UNIC_PPP_MAX_FEATURE_NAME_LEN];
	uint32_t sub_cmd_code;
	bool need_query_hw_res;
	int (*query)(struct hikp_cmd_header *req_header,
		     const struct bdf_t *bdf, void *data);
	void (*show)(const void *data);
};

struct unic_ppp_param {
	struct tool_target target;
	int feature_idx;
};

struct unic_ppp_req_para {
	struct bdf_t bdf;
	uint8_t block_id;
	union {
		uint8_t is_unicast; /* 1: uc GUID, 0: mc GUID. */
		uint8_t rsv; /* firmware ignores it if isn't used to query GUID table. */
	};
	uint8_t rsv1[2];
	uint32_t cur_entry_idx; /* firmware queries GUID/IP table from the value. */
};

struct unic_ppp_rsp_head {
	uint8_t total_blk_num;
	uint8_t cur_blk_size;  /* real data size, not contain head size. */
	uint16_t rsv;
	/* firmware must set following fields when query GUID/IP table. */
	uint32_t next_entry_idx;
	uint32_t cur_blk_entry_cnt;
};

typedef struct unic_ppp_rsp {
	struct unic_ppp_rsp_head rsp_head; /* 12 Byte */
	uint32_t rsp_data[UNIC_PPP_MAX_RSP_DATA];
} unic_ppp_rsp_t;

#endif /* HIKP_UNIC_PPP_H */
