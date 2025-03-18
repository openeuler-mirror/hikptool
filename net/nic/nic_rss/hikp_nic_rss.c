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

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "hikp_nic_rss.h"

static struct nic_rss_param g_rss_param = { 0 };

/* ipv4-tcp */
#define HIKP_RSS_NONFRAG_IPV4_TCP_EN_TCP_D  HI_BIT(0)
#define HIKP_RSS_NONFRAG_IPV4_TCP_EN_TCP_S  HI_BIT(1)
#define HIKP_RSS_NONFRAG_IPV4_TCP_EN_IP_D   HI_BIT(2)
#define HIKP_RSS_NONFRAG_IPV4_TCP_EN_IP_S   HI_BIT(3)

/* ipv4-udp */
#define HIKP_RSS_NONFRAG_IPV4_UDP_EN_UDP_D  HI_BIT(4)
#define HIKP_RSS_NONFRAG_IPV4_UDP_EN_UDP_S  HI_BIT(5)
#define HIKP_RSS_NONFRAG_IPV4_UDP_EN_IP_D   HI_BIT(6)
#define HIKP_RSS_NONFRAG_IPV4_UDP_EN_IP_S   HI_BIT(7)

/* ipv4-sctp */
#define HIKP_RSS_NONFRAG_IPV4_SCTP_EN_SCTP_D    HI_BIT(8)
#define HIKP_RSS_NONFRAG_IPV4_SCTP_EN_SCTP_S    HI_BIT(9)
#define HIKP_RSS_NONFRAG_IPV4_SCTP_EN_IP_D      HI_BIT(10)
#define HIKP_RSS_NONFRAG_IPV4_SCTP_EN_IP_S      HI_BIT(11)
#define HIKP_RSS_NONFRAG_IPV4_SCTP_EN_SCTP_VTAG HI_BIT(12)

/* ipv4-other */
#define HIKP_RSS_NONFRAG_IPV4_OTHER_EN_IP_D  HI_BIT(13)
#define HIKP_RSS_NONFRAG_IPV4_OTHER_EN_IP_S  HI_BIT(14)

/* ipv4-frag */
#define HIKP_RSS_FRAG_IPV4_EN_IP_D   HI_BIT(15)
#define HIKP_RSS_FRAG_IPV4_EN_IP_S   HI_BIT(16)

/* ipv6-tcp */
#define HIKP_RSS_NONFRAG_IPV6_TCP_EN_TCP_D  HI_BIT(0)
#define HIKP_RSS_NONFRAG_IPV6_TCP_EN_TCP_S  HI_BIT(1)
#define HIKP_RSS_NONFRAG_IPV6_TCP_EN_IP_D   HI_BIT(2)
#define HIKP_RSS_NONFRAG_IPV6_TCP_EN_IP_S   HI_BIT(3)

/* ipv6-udp */
#define HIKP_RSS_NONFRAG_IPV6_UDP_EN_UDP_D  HI_BIT(4)
#define HIKP_RSS_NONFRAG_IPV6_UDP_EN_UDP_S  HI_BIT(5)
#define HIKP_RSS_NONFRAG_IPV6_UDP_EN_IP_D   HI_BIT(6)
#define HIKP_RSS_NONFRAG_IPV6_UDP_EN_IP_S   HI_BIT(7)

/* ipv6-sctp */
#define HIKP_RSS_NONFRAG_IPV6_SCTP_EN_SCTP_D    HI_BIT(8)
#define HIKP_RSS_NONFRAG_IPV6_SCTP_EN_SCTP_S    HI_BIT(9)
#define HIKP_RSS_NONFRAG_IPV6_SCTP_EN_IP_D      HI_BIT(10)
#define HIKP_RSS_NONFRAG_IPV6_SCTP_EN_IP_S      HI_BIT(11)
#define HIKP_RSS_NONFRAG_IPV6_SCTP_EN_SCTP_VTAG HI_BIT(12)

/* ipv6-other */
#define HIKP_RSS_NONFRAG_IPV6_OTHER_EN_IP_D   HI_BIT(13)
#define HIKP_RSS_NONFRAG_IPV6_OTHER_EN_IP_S   HI_BIT(14)

/* ipv6-frag */
#define HIKP_RSS_FRAG_IPV6_EN_IP_D   HI_BIT(15)
#define HIKP_RSS_FRAG_IPV6_EN_IP_S   HI_BIT(16)

#define HIKP_RSS_FRAG_IPV4_MASK \
	(HIKP_RSS_FRAG_IPV4_EN_IP_D | HIKP_RSS_FRAG_IPV4_EN_IP_S)
#define HIKP_RSS_NONFRAG_IPV4_OTHER_MASK \
	(HIKP_RSS_NONFRAG_IPV4_OTHER_EN_IP_D | HIKP_RSS_NONFRAG_IPV4_OTHER_EN_IP_S)
#define HIKP_RSS_NONFRAG_IPV4_TCP_MASK \
	(HIKP_RSS_NONFRAG_IPV4_TCP_EN_TCP_D | HIKP_RSS_NONFRAG_IPV4_TCP_EN_TCP_S | \
	 HIKP_RSS_NONFRAG_IPV4_TCP_EN_IP_D | HIKP_RSS_NONFRAG_IPV4_TCP_EN_IP_S)
#define HIKP_RSS_NONFRAG_IPV4_UDP_MASK \
	(HIKP_RSS_NONFRAG_IPV4_UDP_EN_UDP_D | HIKP_RSS_NONFRAG_IPV4_UDP_EN_UDP_S | \
	 HIKP_RSS_NONFRAG_IPV4_UDP_EN_IP_D | HIKP_RSS_NONFRAG_IPV4_UDP_EN_IP_S)
#define HIKP_RSS_NONFRAG_IPV4_SCTP_MASK \
	(HIKP_RSS_NONFRAG_IPV4_SCTP_EN_SCTP_D | HIKP_RSS_NONFRAG_IPV4_SCTP_EN_SCTP_S | \
	 HIKP_RSS_NONFRAG_IPV4_SCTP_EN_IP_D | HIKP_RSS_NONFRAG_IPV4_SCTP_EN_IP_S | \
	 HIKP_RSS_NONFRAG_IPV4_SCTP_EN_SCTP_VTAG)

#define HIKP_RSS_FRAG_IPV6_MASK \
	((uint64_t)(HIKP_RSS_FRAG_IPV6_EN_IP_D | HIKP_RSS_FRAG_IPV6_EN_IP_S) << 32)
#define HIKP_RSS_NONFRAG_IPV6_OTHER_MASK \
	((uint64_t)(HIKP_RSS_NONFRAG_IPV6_OTHER_EN_IP_D | \
	 HIKP_RSS_NONFRAG_IPV6_OTHER_EN_IP_S) << 32)
#define HIKP_RSS_NONFRAG_IPV6_TCP_MASK \
	((uint64_t)(HIKP_RSS_NONFRAG_IPV6_TCP_EN_TCP_D | HIKP_RSS_NONFRAG_IPV6_TCP_EN_TCP_S | \
	 HIKP_RSS_NONFRAG_IPV6_TCP_EN_IP_D | HIKP_RSS_NONFRAG_IPV6_TCP_EN_IP_S) << 32)
#define HIKP_RSS_NONFRAG_IPV6_UDP_MASK \
	((uint64_t)(HIKP_RSS_NONFRAG_IPV6_UDP_EN_UDP_D | HIKP_RSS_NONFRAG_IPV6_UDP_EN_UDP_S | \
	 HIKP_RSS_NONFRAG_IPV6_UDP_EN_IP_D | HIKP_RSS_NONFRAG_IPV6_UDP_EN_IP_S) << 32)
#define HIKP_RSS_NONFRAG_IPV6_SCTP_MASK \
	((uint64_t)(HIKP_RSS_NONFRAG_IPV6_SCTP_EN_SCTP_D | HIKP_RSS_NONFRAG_IPV6_SCTP_EN_SCTP_S | \
	 HIKP_RSS_NONFRAG_IPV6_SCTP_EN_IP_D | HIKP_RSS_NONFRAG_IPV6_SCTP_EN_IP_S | \
	 HIKP_RSS_NONFRAG_IPV6_SCTP_EN_SCTP_VTAG) << 32)

enum hikp_nic_rss_pkt_type {
	IPV4_FRAG = 0,
	IPV4_TCP,
	IPV4_UDP,
	IPV4_SCTP,
	IPV4_OTHER,
	IPV6_FRAG,
	IPV6_TCP,
	IPV6_UDP,
	IPV6_SCTP,
	IPV6_OTHER,
};

static void hikp_nic_rss_show_algo(const void *data);
static void hikp_nic_rss_show_hash_key(const void *data);
static void hikp_nic_rss_show_tuple(const void *data);
static void hikp_nic_rss_show_reta_table(const void *data);
static void hikp_nic_rss_show_tc_mode(const void *data);

static const struct rss_feature_cmd g_rss_feature_cmd[] = {
	{"algo",    RSS_ALGO_DUMP,    hikp_nic_rss_show_algo},
	{"key",     RSS_KEY_DUMP,     hikp_nic_rss_show_hash_key},
	{"tuple",   RSS_TUPLE_DUMP,   hikp_nic_rss_show_tuple},
	{"reta",    RSS_RETA_DUMP,    hikp_nic_rss_show_reta_table},
	{"tc_mode", RSS_TC_MODE_DUMP, hikp_nic_rss_show_tc_mode},
};

void hikp_nic_rss_cmd_set_feature_idx(int feature_idx)
{
	g_rss_param.feature_idx = feature_idx;
}

static int hikp_nic_rss_cmd_help(struct major_cmd_ctrl *self, const char *argv)
{
	HIKP_SET_USED(argv);

	printf("\n  Usage: %s %s\n", self->cmd_ptr->name, "-i <device>");
	printf("\n         %s\n", self->cmd_ptr->help_info);
	printf("\n  Options:\n\n");
	printf("    %s, %-25s %s\n", "-h", "--help", "display this help and exit");
	printf("	%s, %-25s %s\n", "-i", "--interface=<interface>",
	       "device target or bdf id, e.g. eth0~7 or 0000:35:00.0");
	printf("      %s\n",
	       "[-g/--get <options>]\n"
	       "          algo    : get hash algorithm.\n"
	       "          key     : get hash key.\n"
	       "          tuple   : get configuration of tuple\n"
	       "          reta    : get reta table.\n"
	       "          tc_mode : get rss tc mode.\n");

	return 0;
}

static void hikp_nic_rss_show_algo(const void *data)
{
	uint8_t hash_algo = *(uint8_t *)data;

	switch (hash_algo) {
	case HIKP_RSS_HASH_TOEPLITZ:
		printf("RSS hash algo: Toeplitz.\n");
		break;
	case HIKP_RSS_HASH_SIMPLE_XOR:
		printf("RSS hash algo: Simple-XOR.\n");
		break;
	case HIKP_RSS_HASH_SYMMETRIC_TOEPLITZ:
		printf("RSS hash algo: Symmetric Toeplitz.\n");
		break;
	default:
		printf("hash_algo=0x%x\n", hash_algo);
		break;
	}
}

static void hikp_nic_rss_show_hash_key(const void *data)
{
	struct rss_hash_key *rss_key = (struct rss_hash_key *)data;
	uint16_t i;

	printf("key len: %u\n", rss_key->key_len);
	printf("KEY: ");
	for (i = 0; i < rss_key->key_len; i++) {
		if (i >= HIKP_RSS_HASH_KEY_LEN_MAX) {
			HIKP_ERROR_PRINT("The cmd data is truncated.\n");
			break;
		}
		printf("%02X", rss_key->key[i]);
	}
	printf("\n");
}

static void hikp_nic_rss_get_ipv4_tuples_name(enum hikp_nic_rss_pkt_type pkt_type,
					      uint64_t rss_type, char *tuples_name, uint8_t len)
{
	switch (pkt_type) {
	case IPV4_FRAG:
		snprintf(tuples_name, len, "%s %s",
			 (rss_type & HIKP_RSS_FRAG_IPV4_EN_IP_S) ? "IP_S" : "",
			 (rss_type & HIKP_RSS_FRAG_IPV4_EN_IP_D) ? "IP_D" : "");
		break;
	case IPV4_TCP:
		snprintf(tuples_name, len, "%s %s %s %s",
			 (rss_type & HIKP_RSS_NONFRAG_IPV4_TCP_EN_IP_S) ? "IP_S" : "",
			 (rss_type & HIKP_RSS_NONFRAG_IPV4_TCP_EN_IP_D) ? "IP_D" : "",
			 (rss_type & HIKP_RSS_NONFRAG_IPV4_TCP_EN_TCP_S) ? "TCP_S" : "",
			 (rss_type & HIKP_RSS_NONFRAG_IPV4_TCP_EN_TCP_D) ? "TCP_D" : "");
		break;
	case IPV4_UDP:
		snprintf(tuples_name, len, "%s %s %s %s",
			 (rss_type & HIKP_RSS_NONFRAG_IPV4_UDP_EN_IP_S) ? "IP_S" : "",
			 (rss_type & HIKP_RSS_NONFRAG_IPV4_UDP_EN_IP_D) ? "IP_D" : "",
			 (rss_type & HIKP_RSS_NONFRAG_IPV4_UDP_EN_UDP_S) ? "UDP_S" : "",
			 (rss_type & HIKP_RSS_NONFRAG_IPV4_UDP_EN_UDP_D) ? "UDP_D" : "");
		break;
	case IPV4_SCTP:
		snprintf(tuples_name, len, "%s %s %s %s %s",
			 (rss_type & HIKP_RSS_NONFRAG_IPV4_SCTP_EN_IP_S) ? "IP_S" : "",
			 (rss_type & HIKP_RSS_NONFRAG_IPV4_SCTP_EN_IP_D) ? "IP_D" : "",
			 (rss_type & HIKP_RSS_NONFRAG_IPV4_SCTP_EN_SCTP_S) ? "SCTP_S" : "",
			 (rss_type & HIKP_RSS_NONFRAG_IPV4_SCTP_EN_SCTP_D) ? "SCTP_D" : "",
			 (rss_type & HIKP_RSS_NONFRAG_IPV4_SCTP_EN_SCTP_VTAG) ? "SCTP_VTAG" : "");
		break;
	case IPV4_OTHER:
		snprintf(tuples_name, len, "%s %s",
			 (rss_type & HIKP_RSS_NONFRAG_IPV4_OTHER_EN_IP_S) ? "IP_S" : "",
			 (rss_type & HIKP_RSS_NONFRAG_IPV4_OTHER_EN_IP_D) ? "IP_D" : "");
		break;
	default:
		break;
	}
}

static void hikp_nic_rss_get_ipv6_tuples_name(enum hikp_nic_rss_pkt_type pkt_type,
					      uint64_t rss_type, char *tuples_name, uint8_t len)
{
	switch (pkt_type) {
	case IPV6_FRAG:
		snprintf(tuples_name, len, "%s %s",
			 (rss_type & HIKP_RSS_FRAG_IPV6_EN_IP_S) ? "IP_S" : "",
			 (rss_type & HIKP_RSS_FRAG_IPV6_EN_IP_D) ? "IP_D" : "");
		break;
	case IPV6_TCP:
		snprintf(tuples_name, len, "%s %s %s %s",
			 (rss_type & HIKP_RSS_NONFRAG_IPV6_TCP_EN_IP_S) ? "IP_S" : "",
			 (rss_type & HIKP_RSS_NONFRAG_IPV6_TCP_EN_IP_D) ? "IP_D" : "",
			 (rss_type & HIKP_RSS_NONFRAG_IPV6_TCP_EN_TCP_S) ? "TCP_S" : "",
			 (rss_type & HIKP_RSS_NONFRAG_IPV6_TCP_EN_TCP_D) ? "TCP_D" : "");
		break;
	case IPV6_UDP:
		snprintf(tuples_name, len, "%s %s %s %s",
			 (rss_type & HIKP_RSS_NONFRAG_IPV6_UDP_EN_IP_S) ? "IP_S" : "",
			 (rss_type & HIKP_RSS_NONFRAG_IPV6_UDP_EN_IP_D) ? "IP_D" : "",
			 (rss_type & HIKP_RSS_NONFRAG_IPV6_UDP_EN_UDP_S) ? "UDP_S" : "",
			 (rss_type & HIKP_RSS_NONFRAG_IPV6_UDP_EN_UDP_D) ? "UDP_D" : "");
		break;
	case IPV6_SCTP:
		snprintf(tuples_name, len, "%s %s %s %s %s",
			 (rss_type & HIKP_RSS_NONFRAG_IPV6_SCTP_EN_IP_S) ? "IP_S" : "",
			 (rss_type & HIKP_RSS_NONFRAG_IPV6_SCTP_EN_IP_D) ? "IP_D" : "",
			 (rss_type & HIKP_RSS_NONFRAG_IPV6_SCTP_EN_SCTP_S) ? "SCTP_S" : "",
			 (rss_type & HIKP_RSS_NONFRAG_IPV6_SCTP_EN_SCTP_D) ? "SCTP_D" : "",
			 (rss_type & HIKP_RSS_NONFRAG_IPV6_SCTP_EN_SCTP_VTAG) ? "SCTP_VTAG" : "");
		break;
	case IPV6_OTHER:
		snprintf(tuples_name, len, "%s %s",
			 (rss_type & HIKP_RSS_NONFRAG_IPV6_OTHER_EN_IP_S) ? "IP_S" : "",
			 (rss_type & HIKP_RSS_NONFRAG_IPV6_OTHER_EN_IP_D) ? "IP_D" : "");
		break;
	default:
		break;
	}
}

static void hikp_nic_rss_get_tuples_name(enum hikp_nic_rss_pkt_type pkt_type, uint64_t rss_type,
					 char *tuples_name, uint8_t len)
{
	hikp_nic_rss_get_ipv4_tuples_name(pkt_type, rss_type, tuples_name, len);
	hikp_nic_rss_get_ipv6_tuples_name(pkt_type, rss_type, tuples_name, len);
	if (tuples_name[0] == '\0')
		HIKP_WARN_PRINT("Invalid packet type.\n");
}

static void hikp_nic_rss_show_tuple(const void *data)
{
#define RSS_MAX_TUPLES_NAME_LEN 64

	const struct {
		const char *str; /* Type name. */
		enum hikp_nic_rss_pkt_type pkt_type;
		uint64_t rss_type; /* Type value. */
	} rss_type_table[] = {
		{ "ipv4-frag", IPV4_FRAG, HIKP_RSS_FRAG_IPV4_MASK },
		{ "ipv4-tcp", IPV4_TCP, HIKP_RSS_NONFRAG_IPV4_TCP_MASK },
		{ "ipv4-udp", IPV4_UDP, HIKP_RSS_NONFRAG_IPV4_UDP_MASK },
		{ "ipv4-sctp", IPV4_SCTP, HIKP_RSS_NONFRAG_IPV4_SCTP_MASK },
		{ "ipv4-other", IPV4_OTHER, HIKP_RSS_NONFRAG_IPV4_OTHER_MASK },
		{ "ipv6-frag", IPV6_FRAG, HIKP_RSS_FRAG_IPV6_MASK },
		{ "ipv6-tcp", IPV6_TCP, HIKP_RSS_NONFRAG_IPV4_TCP_MASK },
		{ "ipv6-udp", IPV6_UDP, HIKP_RSS_NONFRAG_IPV6_UDP_MASK },
		{ "ipv6-sctp", IPV6_SCTP, HIKP_RSS_NONFRAG_IPV6_SCTP_MASK },
		{ "ipv6-other", IPV6_OTHER, HIKP_RSS_NONFRAG_IPV6_OTHER_MASK },
	};
	struct rss_tuple_fields *tuple_info = (struct rss_tuple_fields *)data;
	size_t type_size = HIKP_ARRAY_SIZE(rss_type_table);
	size_t i;

	for (i = 0; i < type_size; i++) {
		if (tuple_info->tuple_field == 0) {
			printf("RSS disable");
			break;
		}

		char tuples_name[RSS_MAX_TUPLES_NAME_LEN] = {0};

		if (tuple_info->tuple_field & rss_type_table[i].rss_type) {
			hikp_nic_rss_get_tuples_name(rss_type_table[i].pkt_type,
						     tuple_info->tuple_field,
						     tuples_name, RSS_MAX_TUPLES_NAME_LEN);
			printf("%s enable field: %s\n", rss_type_table[i].str, tuples_name);
		}
	}
	printf("\n");
}

static void hikp_nic_rss_show_reta_table(const void *data)
{
	struct rss_reta_info *reta_info = (struct rss_reta_info *)data;
	uint16_t i;

	for (i = 0; i < reta_info->reta_size; i++) {
		if (i >= HIKP_RSS_RETA_SIZE_MAX) {
			HIKP_ERROR_PRINT("The cmd data is truncated.\n");
			break;
		}
		printf("RSS RETA configuration: hash index=%u, queue=%u\n", i, reta_info->reta[i]);
	}
}

static void hikp_nic_rss_show_tc_mode(const void *data)
{
	struct rss_tc_mode_entry *tc_mode_entry = (struct rss_tc_mode_entry *)data;
	uint16_t tc;

	printf("tc_num  | tc_valid | tc_offset | tc_size\n");
	for (tc = 0; tc < HIKP_NIC_MAX_TC_NUM; tc++) {
		printf("%u       |     %u    |    %04u   |    %u\n",
		       tc, tc_mode_entry[tc].tc_valid,
		       tc_mode_entry[tc].tc_offset, tc_mode_entry[tc].tc_size);
	}
}

static int hikp_nic_rss_get_blk(struct hikp_cmd_header *req_header,
				const struct nic_rss_req_para *req_data,
				void *buf, size_t buf_len, struct nic_rss_rsp_head *rsp_head)
{
	struct hikp_cmd_ret *cmd_ret;
	struct nic_rss_rsp *rsp;
	int ret = 0;

	cmd_ret = hikp_cmd_alloc(req_header, req_data, sizeof(*req_data));
	ret = hikp_rsp_normal_check(cmd_ret);
	if (ret) {
		HIKP_ERROR_PRINT("failed to get block-%u context.\n", req_data->block_id);
		goto out;
	}

	rsp = (struct nic_rss_rsp *)cmd_ret->rsp_data;
	if (rsp->rsp_head.cur_blk_size > buf_len ||
	    rsp->rsp_head.cur_blk_size > sizeof(rsp->rsp_data)) {
		HIKP_ERROR_PRINT("nic_rss block-%u copy size error, "
				 "dst buffer size=%zu, src buffer size=%zu, "
				 "data size=%u.\n", req_data->block_id, buf_len,
				 sizeof(rsp->rsp_data), rsp->rsp_head.cur_blk_size);
		ret = -EINVAL;
		goto out;
	}
	memcpy(buf, rsp->rsp_data, rsp->rsp_head.cur_blk_size);
	rsp_head->total_blk_num = rsp->rsp_head.total_blk_num;
	rsp_head->cur_blk_size = rsp->rsp_head.cur_blk_size;

out:
	hikp_cmd_free(&cmd_ret);
	return ret;
}

static int hikp_nic_query_rss_feature(struct hikp_cmd_header *req_header, const struct bdf_t *bdf,
				      union nic_rss_feature_info *data)
{
	struct nic_rss_rsp_head rsp_head = {0};
	struct nic_rss_req_para req_data;
	size_t buf_len = sizeof(*data);
	uint32_t total_blk_size;
	uint8_t total_blk_num;
	uint8_t blk_id = 0;
	int ret;

	req_data.bdf = *bdf;

	req_data.block_id = blk_id;
	ret = hikp_nic_rss_get_blk(req_header, &req_data, data, buf_len, &rsp_head);
	if (ret != 0)
		return ret;

	total_blk_num = rsp_head.total_blk_num;
	total_blk_size = rsp_head.cur_blk_size;

	/* Copy the remaining block content if total block number is greater than 1. */
	for (blk_id = 1; blk_id < total_blk_num; blk_id++) {
		req_data.block_id = blk_id;
		ret = hikp_nic_rss_get_blk(req_header, &req_data,
					   (uint8_t *)data + total_blk_size,
					   buf_len - total_blk_size, &rsp_head);
		if (ret != 0)
			return ret;
		total_blk_size += rsp_head.cur_blk_size;
	}

	return ret;
}

void hikp_nic_rss_cmd_execute(struct major_cmd_ctrl *self)
{
	union nic_rss_feature_info rss_data = {0};
	const struct rss_feature_cmd *rss_cmd;
	struct hikp_cmd_header req_header = {0};
	int ret;

	if (g_rss_param.feature_idx == -1) {
		hikp_nic_rss_cmd_help(self, NULL);
		snprintf(self->err_str, sizeof(self->err_str), "-g/--get param error!");
		self->err_no = -EINVAL;
		return;
	}

	rss_cmd = &g_rss_feature_cmd[g_rss_param.feature_idx];
	hikp_cmd_init(&req_header, NIC_MOD, GET_RSS_INFO_CMD, rss_cmd->sub_cmd_code);
	ret = hikp_nic_query_rss_feature(&req_header, &g_rss_param.target.bdf, &rss_data);
	if (ret != 0) {
		snprintf(self->err_str, sizeof(self->err_str), "failed to query %s, ret = %d.",
			 rss_cmd->feature_name, ret);
		self->err_no = ret;
		return;
	}

	printf("################### RSS %s ###################\n", rss_cmd->feature_name);
	rss_cmd->show(&rss_data);
	printf("#################### END #######################\n");
}

int hikp_nic_cmd_get_rss_target(struct major_cmd_ctrl *self, const char *argv)
{
	self->err_no = tool_check_and_get_valid_bdf_id(argv, &(g_rss_param.target));
	if (self->err_no != 0) {
		snprintf(self->err_str, sizeof(self->err_str), "unknown device!");
		return self->err_no;
	}

	return 0;
}

static int hikp_nic_cmd_rss_feature_select(struct major_cmd_ctrl *self, const char *argv)
{
	size_t feat_size = HIKP_ARRAY_SIZE(g_rss_feature_cmd);
	size_t i;

	for (i = 0; i < feat_size; i++) {
		if (strcmp(argv, g_rss_feature_cmd[i].feature_name) == 0) {
			g_rss_param.feature_idx = i;
			return 0;
		}
	}

	hikp_nic_rss_cmd_help(self, NULL);
	snprintf(self->err_str, sizeof(self->err_str), "-g/--get param error!!!");
	self->err_no = -EINVAL;

	return self->err_no;
}

static void cmd_nic_get_rss_init(void)
{
	struct major_cmd_ctrl *major_cmd = get_major_cmd();

	g_rss_param.feature_idx = -1;

	major_cmd->option_count = 0;
	major_cmd->execute = hikp_nic_rss_cmd_execute;

	cmd_option_register("-h", "--help", false, hikp_nic_rss_cmd_help);
	cmd_option_register("-i", "--interface", true, hikp_nic_cmd_get_rss_target);
	cmd_option_register("-g", "--get", true, hikp_nic_cmd_rss_feature_select);
}

HIKP_CMD_DECLARE("nic_rss", "show rss info of nic!", cmd_nic_get_rss_init);
