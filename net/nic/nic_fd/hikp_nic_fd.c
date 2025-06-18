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
#include <inttypes.h>
#include "hikp_nic_fd.h"

struct key_info {
	const char *key_name;
	uint16_t key_type;
	uint8_t key_length;
};

static const struct key_info g_meta_data_key_info[] = {
	{"packet_type_id", PACKET_TYPE_ID, 6},
	{"fragement",      IP_FRAGEMENT,   1},
	{"roce_type",      ROCE_TYPE,      1},
	{"next_key",       NEXT_KEY,       5},
	{"vlan_num",       VLAN_NUMBER,    2},
	{"src_port",       SRC_VPORT,      12},
	{"des_port",       DST_VPORT,      12},
	{"tunnel_packet",  TUNNEL_PACKET,  1},
};

static const struct key_info g_tuple_key_info[] = {
	{"ot_dmac",         OUTER_DST_MAC,      48},
	{"ot_smac",         OUTER_SRC_MAC,      48},
	{"ot_vlan_tag_fst", OUTER_VLAN_TAG_FST, 16},
	{"ot_vlan_tag_sec", OUTER_VLAN_TAG_SEC, 16},
	{"ot_eth_type",     OUTER_ETH_TYPE,     16},
	{"ot_l2_rsv",       OUTER_L2_RSV,       16},
	{"ot_ip_tos",       OUTER_IP_TOS,       8},
	{"ot_ip_proto",     OUTER_IP_PROTO,     8},
	{"ot_sip",          OUTER_SRC_IP,       32},
	{"ot_dip",          OUTER_DST_IP,       32},
	{"ot_l3_rsv",       OUTER_L3_RSV,       16},
	{"ot_sp",           OUTER_SRC_PORT,     16},
	{"ot_dp",           OUTER_DST_PORT,     16},
	{"ot_l4_rsv",       OUTER_L4_RSV,       32},
	{"ot_tun_vni",      OUTER_TUN_VNI,      24},
	{"ot_tun_flow_id",  OUTER_TUN_FLOW_ID,  8},
	{"in_dmac",         INNER_DST_MAC,      48},
	{"in_smac",         INNER_SRC_MAC,      48},
	{"in_vlan_tag_fst", INNER_VLAN_TAG_FST, 16},
	{"in_vlan_tag_sec", INNER_VLAN_TAG_SEC, 16},
	{"in_eth_type",     INNER_ETH_TYPE,     16},
	{"in_l2_rsv",       INNER_L2_RSV,       16},
	{"in_ip_tos",       INNER_IP_TOS,       8},
	{"in_ip_proto",     INNER_IP_PROTO,     8},
	{"in_sip",          INNER_SRC_IP,       32},
	{"in_dip",          INNER_DST_IP,       32},
	{"in_l3_rsv",       INNER_L3_RSV,       16},
	{"in_sp",           INNER_SRC_PORT,     16},
	{"in_dp",           INNER_DST_PORT,     16},
	{"in_l4_rsv",       INNER_L4_RSV,       32},
};

struct nic_fd_action {
	bool drop;
	bool q_vid;
	uint16_t qid;
	bool cnt_vld;
	uint16_t cnt_id;
	bool nxt_vld;
	uint16_t next_input_key;
	bool rule_id_vld;
	uint16_t rule_id;
	bool tc_ovrd_en;
	uint16_t queue_region_size;
};

static struct nic_fd_param g_fd_param = {0};
static struct nic_fd_hw_info g_fd_hw_info = {0};

static int hikp_nic_query_fd_hw_info(struct hikp_cmd_header *req_header, const struct bdf_t *bdf,
				     uint8_t stage, void *data, size_t len);
static int hikp_nic_query_fd_rules(struct hikp_cmd_header *req_header, const struct bdf_t *bdf,
				   uint8_t stage, void *data, size_t len);
static int hikp_nic_query_fd_counter(struct hikp_cmd_header *req_header, const struct bdf_t *bdf,
				     uint8_t stage, void *data, size_t len);

static void hikp_nic_show_fd_hw_info(const void *data);
static void hikp_nic_show_fd_rules(const void *data);
static void hikp_nic_show_fd_counter(const void *data);

#define NIC_FD_HW_INFO_NAME "hw_info"
#define NIC_FD_RULES_NAME "rules"
#define NIC_FD_COUNTER_NAME "counter"

static const struct fd_feature_cmd g_fd_feature_cmd[] = {
	{NIC_FD_HW_INFO_NAME, NIC_FD_HW_INFO_DUMP,       false,
	 hikp_nic_query_fd_hw_info, hikp_nic_show_fd_hw_info},
	{NIC_FD_RULES_NAME,   NIC_FD_RULES_INFO_DUMP,    true,
	 hikp_nic_query_fd_rules,   hikp_nic_show_fd_rules},
	{NIC_FD_COUNTER_NAME, NIC_FD_COUNTER_STATS_DUMP, true,
	 hikp_nic_query_fd_counter, hikp_nic_show_fd_counter},
};

void hikp_nic_set_fd_idx(int feature_idx, int stage_no)
{
	g_fd_param.id = -1;
	g_fd_param.feature_idx = feature_idx;
	g_fd_param.stage_no = stage_no;
}

static int hikp_nic_fd_cmd_help(struct major_cmd_ctrl *self, const char *argv)
{
	HIKP_SET_USED(argv);

	printf("\n  Usage: %s %s\n", self->cmd_ptr->name, "-i <device>");
	printf("\n         %s\n", self->cmd_ptr->help_info);
	printf("\n  Options:\n\n");
	printf("    %s, %-25s %s\n", "-h", "--help", "display this help and exit");
	printf("    %s, %-25s %s\n", "-i", "--interface=<interface>",
	       "device target or bdf id, e.g. eth0~7 or 0000:35:00.0");
	printf("%s\n",
	       "      [-du/--dump hw_info ]\n"
	       "              dump fd hardware info.\n"
	       "      [-du/--dump rules -st/--stage <stage_no> -id/--index <rule_id> ]\n"
	       "              dump all rules or one rule info of certain stage fd.\n"
	       "      [-du/--dump counter -st/--stage <stage_no> -id/--index <counter_id> ]\n"
	       "              dump all counters or one counter stats of certain stage fd.\n");
	printf("    Note: dump all entries without '-id/--index'\n");

	return 0;
}

static uint16_t hikp_nic_get_tcam_data_size(uint16_t key_max_bit)
{
	uint16_t max_key_bytes;

	max_key_bytes = HIKP_DIV_ROUND_UP(key_max_bit / HIKP_BITS_PER_BYTE, HIKP_DWORDS_BYTE);
	max_key_bytes = max_key_bytes * HIKP_DWORDS_BYTE;

	return max_key_bytes;
}

static void hikp_nic_show_fd_key_info(struct nic_fd_hw_info *hw_info)
{
	struct nic_fd_key_cfg *key_cfg;
	size_t fd_mask_cnt;
	uint16_t i;
	size_t j;

	for (i = 0; i < NIC_FD_STAGE_NUM; i++) {
		if (hw_info->alloc.stage_entry_num[i] == 0)
			continue;

		key_cfg = &hw_info->key_cfg[i];
		printf("fd stage%u key info:\n", (uint16_t)(i + 1));
		printf("  key_select: %s\n",
		       key_cfg->key_select == HNS3_FD_KEY_BASE_ON_TUPLE ? "tuple" : "packet");
		printf("  inner_src_ipv6_word_en: 0x%x\n", key_cfg->inner_src_ipv6_word_en);
		printf("  inner_dest_ipv6_word_en: 0x%x\n", key_cfg->inner_dest_ipv6_word_en);
		printf("  outer_src_ipv6_word_en: 0x%x\n", key_cfg->outer_src_ipv6_word_en);
		printf("  outer_dest_ipv6_word_en: 0x%x\n", key_cfg->outer_dest_ipv6_word_en);

		if (key_cfg->key_select == HNS3_FD_KEY_BASE_ON_PTYPE) {
			HIKP_WARN_PRINT("Unsupport for parsing packet type key.\n");
			continue;
		}

		printf("  fd enable key info[mask: 0x%x]:\n", key_cfg->tuple_mask);
		fd_mask_cnt = HIKP_ARRAY_SIZE(g_tuple_key_info);
		for (j = 0; j < fd_mask_cnt; j++) {
			if (hikp_get_bit(key_cfg->tuple_mask, j) == 0)
				printf("    %s\n", g_tuple_key_info[j].key_name);
		}

		printf("  fd meta info[mask: 0x%x]:\n", key_cfg->meta_data_mask);
		fd_mask_cnt = HIKP_ARRAY_SIZE(g_meta_data_key_info);
		for (j = 0; j < fd_mask_cnt; j++) {
			if (hikp_get_bit(key_cfg->meta_data_mask, j) == 0)
				printf("    %s\n", g_meta_data_key_info[j].key_name);
		}
	}
}

static const char *hikp_nic_get_fd_mode_name(uint8_t mode)
{
	struct hikp_nic_fd_mode_info {
		uint8_t mode;
		const char *name;
	} mode_info[] = {
		{FD_MODE_DEPTH_2K_WIDTH_400B_STAGE_1, "one level mode, 2k * 400b"},
		{FD_MODE_DEPTH_1K_WIDTH_400B_STAGE_2, "two level mode, 1k * 400b per level"},
		{FD_MODE_DEPTH_4K_WIDTH_200B_STAGE_1, "one level mode, 4k * 200b"},
		{FD_MODE_DEPTH_2K_WIDTH_200B_STAGE_2, "two level mode, 2k * 200 per level"},
	};
	size_t i;

	for (i = 0; i < HIKP_ARRAY_SIZE(mode_info); i++) {
		if (mode_info[i].mode == mode)
			return mode_info[i].name;
	}

	return "unknown mode";
}

static void hikp_nic_show_fd_hw_info(const void *data)
{
	struct nic_fd_hw_info *hw_info = (struct nic_fd_hw_info *)data;
	uint16_t i;

	printf("fd hardware info:\n");
	printf("  fd_mode: %s\n", hikp_nic_get_fd_mode_name(hw_info->mode));
	printf("  fd_enable=%s\n", hw_info->enable ? "enable" : "disable");
	printf("  max key bit width: %u\n", hw_info->key_max_bit);
	for (i = 0; i < NIC_FD_STAGE_NUM; i++) {
		printf("  stage%u_entry_num=%u\n", (uint16_t)(i + 1),
		       hw_info->alloc.stage_entry_num[i]);
		printf("  stage%u_counter_num=%u\n", (uint16_t)(i + 1),
		       hw_info->alloc.stage_counter_num[i]);
	}

	hikp_nic_show_fd_key_info(hw_info);
}

static uint16_t hikp_nic_get_max_key_len(uint8_t mode)
{
	if (mode == FD_MODE_DEPTH_2K_WIDTH_400B_STAGE_1 ||
	    mode == FD_MODE_DEPTH_1K_WIDTH_400B_STAGE_2)
		return NIC_KEY_LEN_400B;
	else if (mode == FD_MODE_DEPTH_4K_WIDTH_200B_STAGE_1 ||
		 mode == FD_MODE_DEPTH_2K_WIDTH_200B_STAGE_2)
		return NIC_KEY_LEN_200B;

	return 0;
}

static void hikp_nic_print_tuple(const struct key_info *tuple_key,
				 const uint8_t *tcam_x, const uint8_t *tcam_y)
{
#define HIKP_NIC_FD_TUPLE_KEY_LEN_6_BYTES   6
#define HIKP_NIC_FD_TUPLE_KEY_LEN_4_BYTES   4
#define HIKP_NIC_FD_TUN_VNI_LEN             3
	uint32_t tun_vni = 0;
	uint64_t mask = 0;
	uint8_t i;

	/* The bit width of all tuple type key are less than 64Bit. */
	for (i = 0; i < HIKP_DIV_ROUND_UP(tuple_key->key_length, HIKP_BITS_PER_BYTE); i++)
		mask |= (((uint64_t)(*(tcam_x + i) ^ *(tcam_y + i))) << (i * HIKP_BITS_PER_BYTE));

	/* The mask isn't zero, means that the tuple key is valid. */
	if (mask == 0)
		return;

	switch (tuple_key->key_type) {
	case OUTER_DST_MAC:
	case OUTER_SRC_MAC:
	case INNER_DST_MAC:
	case INNER_SRC_MAC:
		printf("\t  %s[mask=0x%" PRIx64 "]: ", tuple_key->key_name, mask);
		printf("%02x:%02x:%02x:%02x:%02x:%02x\n", *(tcam_y + 5), *(tcam_y + 4),
		       *(tcam_y + 3), *(tcam_y + 2), *(tcam_y + 1), *tcam_y);
		break;
	case OUTER_ETH_TYPE:
	case INNER_ETH_TYPE:
		printf("\t  %s[mask=0x%" PRIx64 "]: ", tuple_key->key_name, mask);
		printf("0x%x\n", *(uint16_t *)tcam_y);
		break;
	case OUTER_VLAN_TAG_FST:
	case OUTER_VLAN_TAG_SEC:
	case INNER_VLAN_TAG_FST:
	case INNER_VLAN_TAG_SEC:
	case OUTER_L2_RSV:
	case INNER_L2_RSV:
	case OUTER_L3_RSV:
	case INNER_L3_RSV:
	case OUTER_SRC_PORT:
	case OUTER_DST_PORT:
	case INNER_SRC_PORT:
	case INNER_DST_PORT:
		printf("\t  %s[mask=0x%" PRIx64 "]: ", tuple_key->key_name, mask);
		printf("%u\n", *(uint16_t *)tcam_y);
		break;
	case OUTER_IP_TOS:
	case INNER_IP_TOS:
	case OUTER_IP_PROTO:
	case INNER_IP_PROTO:
	case OUTER_TUN_FLOW_ID:
		printf("\t  %s[mask=0x%" PRIx64 "]: ", tuple_key->key_name, mask);
		printf("0x%x\n", *tcam_y);
		break;
	case OUTER_SRC_IP:
	case OUTER_DST_IP:
	case INNER_SRC_IP:
	case INNER_DST_IP:
		printf("\t  %s[mask=0x%" PRIx64 "]: ", tuple_key->key_name, mask);
		printf("%u.%u.%u.%u\n", *(tcam_y + 3), *(tcam_y + 2), *(tcam_y + 1), *tcam_y);
		break;
	case OUTER_L4_RSV:
	case INNER_L4_RSV:
		printf("\t  %s[mask=0x%" PRIx64 "]: ", tuple_key->key_name, mask);
		printf("%u\n", *(uint32_t *)tcam_y);
		break;
	case OUTER_TUN_VNI:
		for (i = 0; i < HIKP_NIC_FD_TUN_VNI_LEN; i++)
			tun_vni |= (((uint32_t)*(tcam_y + i)) << (i * HIKP_BITS_PER_BYTE));
		printf("\t  %s[mask=0x%" PRIx64 "]: ", tuple_key->key_name, mask);
		printf("0x%x\n", tun_vni);
		break;
	default:
		printf("unknown tuple key type(%u)\n", tuple_key->key_type);
		break;
	}
}

static void hikp_nic_print_meta_data(uint16_t type, uint32_t val)
{
	const char *vlan_str[] = {"no tag", "tag2 only", "tag1 only", "tag1+tag2"};

	switch (type) {
	case PACKET_TYPE_ID:
	case NEXT_KEY:
		printf("%u", val);
		break;
	case IP_FRAGEMENT:
		printf("%s", val == 0 ? "NON-IP frag packet" : "IP frag packet");
		break;
	case ROCE_TYPE:
		printf("%s", val == 0 ? "NIC packet" : "RoCE packet");
		break;
	case VLAN_NUMBER:
		printf("%s", vlan_str[val]);
		break;
	case SRC_VPORT:
	case DST_VPORT:
		printf("0x%x", val);
		break;
	case TUNNEL_PACKET:
		printf("%s", val == 0 ? "non-tunnel packet" : "tunnel packet");
		break;
	default:
		printf("unknown meta type(%u)", type);
		break;
	}
	printf("\n");
}

static void hikp_nic_fd_print_key(const struct nic_fd_rule_info *rule,
				  const struct nic_fd_key_cfg *key_cfg, uint16_t max_key_bytes)
{
	uint32_t tcam_offset;
	const uint8_t *key_x;
	const uint8_t *key_y;
	uint16_t tuple_cnt;
	uint16_t j;

	printf("\tKey:\n");
	tcam_offset = 0;
	key_x = rule->tcam_data;
	key_y = rule->tcam_data + max_key_bytes;
	for (j = 0; j < MAX_TUPLE; j++) {
		if (hikp_get_bit(key_cfg->tuple_mask, j) == 0) {
			tuple_cnt = g_tuple_key_info[j].key_length / HIKP_BITS_PER_BYTE;
			hikp_nic_print_tuple(&g_tuple_key_info[j], key_x + tcam_offset,
					     key_y + tcam_offset);
			tcam_offset += tuple_cnt;
		}
	}
}

static uint16_t hikp_nic_get_active_meta_width(uint32_t meta_mask)
{
	uint16_t width = 0;
	uint16_t i;

	for (i = 0; i < MAX_META_DATA; i++) {
		if (hikp_get_bit(meta_mask, i) == 0)
			width += g_meta_data_key_info[i].key_length;
	}

	return width;
}

static void hikp_nic_fd_print_meta_data(struct nic_fd_rule_info *rule)
{
	struct nic_fd_key_cfg *key_cfg;
	uint16_t active_meta_width;
	uint16_t active_tcam_size;
	uint16_t meta_data_region;
	uint64_t meta_data = 0;
	uint16_t max_key_bytes;
	uint16_t max_key_len;
	uint16_t meta_bytes;
	uint16_t tuple_size;
	uint16_t cur_pos;
	uint8_t *key_y;
	uint16_t end;
	uint16_t val;
	int i;

	max_key_bytes = hikp_nic_get_tcam_data_size(g_fd_hw_info.key_max_bit);
	key_y = rule->tcam_data + max_key_bytes;

	max_key_len = hikp_nic_get_max_key_len(g_fd_hw_info.mode);
	active_tcam_size = max_key_len / HIKP_BITS_PER_BYTE;

	key_cfg = &g_fd_hw_info.key_cfg[NIC_FD_STAGE_1];
	active_meta_width = hikp_nic_get_active_meta_width(key_cfg->meta_data_mask);
	meta_bytes = HIKP_DIV_ROUND_UP(active_meta_width, HIKP_BITS_PER_BYTE);
	meta_data_region = active_tcam_size - meta_bytes;
	if (meta_bytes > sizeof(meta_data)) {
		printf("meta data copy size error, data size: %u, max size: %zu\n",
		       meta_bytes, sizeof(meta_data));
		return;
	}
	memcpy(&meta_data, &key_y[meta_data_region], meta_bytes);
	printf("\t  meta_data[meta_data=0x%" PRIx64 "]:\n", meta_data);
	cur_pos = meta_bytes * HIKP_BITS_PER_BYTE;
	end = cur_pos - 1;
	for (i = MAX_META_DATA - 1; i >= 0; i--) {
		if (hikp_get_bit(key_cfg->meta_data_mask, i) == 0) {
			tuple_size = g_meta_data_key_info[i].key_length;
			cur_pos -= tuple_size;
			val = hikp_get_field(meta_data, GENMASK(end, cur_pos), cur_pos);
			printf("\t    %s: ", g_meta_data_key_info[i].key_name);
			end -= tuple_size;
			hikp_nic_print_meta_data(g_meta_data_key_info[i].key_type, val);
		}
	}
}

static void hikp_nic_parse_ad_data(const struct nic_fd_rule_info *rule,
				   struct nic_fd_action *action)
{
	uint32_t ad_data_h = rule->ad_data_h;
	uint32_t ad_data_l = rule->ad_data_l;

	action->drop = !!(ad_data_l & HI_BIT(NIC_FD_AD_DROP_B));
	action->cnt_vld = !!(ad_data_l & HI_BIT(NIC_FD_AD_USE_COUNTER_B));
	if (action->cnt_vld) {
		action->cnt_id = ad_data_h & HI_BIT(NIC_FD_AD_COUNTER_HIGH_BIT_B) ? 1 : 0;
		action->cnt_id <<= NIC_FD_AD_COUNTER_HIGH_BIT;
		action->cnt_id |= hikp_get_field(ad_data_l, NIC_FD_AD_COUNTER_NUM_M,
						 NIC_FD_AD_COUNTER_NUM_S);
	}
	action->nxt_vld = !!(ad_data_l & HI_BIT(NIC_FD_AD_NXT_STEP_B));
	if (action->nxt_vld)
		action->next_input_key = hikp_get_field(ad_data_l, NIC_FD_AD_NXT_KEY_M,
							NIC_FD_AD_NXT_KEY_S);

	action->rule_id_vld = !!(ad_data_h & HI_BIT(NIC_FD_AD_WR_RULE_ID_B));
	if (action->rule_id_vld)
		action->rule_id = hikp_get_field(ad_data_h, NIC_FD_AD_RULE_ID_M,
						 NIC_FD_AD_RULE_ID_S);

	action->tc_ovrd_en = !!(ad_data_h & HI_BIT(NIC_FD_AD_QUEUE_REGION_EN_B));
	if (action->tc_ovrd_en) {
		action->queue_region_size = hikp_get_field(ad_data_h, NIC_FD_AD_QUEUE_REGION_SIZE_M,
							   NIC_FD_AD_QUEUE_REGION_SIZE_S);
	}
	action->q_vid = !!(ad_data_l & HI_BIT(NIC_FD_AD_DIRECT_QID_B));
	if (action->q_vid || action->tc_ovrd_en) {
		action->qid = ad_data_h & HI_BIT(NIC_FD_AD_QUEUE_ID_HIGH_BIT_B) ? 1 : 0;
		action->qid <<= NIC_FD_AD_QUEUE_ID_HIGH_BIT;
		action->qid |= hikp_get_field(ad_data_l, NIC_FD_AD_QID_M, NIC_FD_AD_QID_S);
	}
}

static void hikp_nic_fd_print_ad_data(struct nic_fd_rule_info *rule)
{
	struct nic_fd_action action = {0};
	uint64_t ad_data;

	ad_data = (uint64_t)rule->ad_data_h << NIC_FD_AD_DATA_S | rule->ad_data_l;
	printf("\n\tAction[ad data: 0x%" PRIx64 "]:\n", ad_data);

	hikp_nic_parse_ad_data(rule, &action);

	if (action.drop)
		printf("\t  Drop/accecpt: Drop");

	if (action.q_vid)
		printf("\t  Direct Queue id: %u", action.qid);

	if (action.cnt_vld)
		printf("\t  Counter id: %u", action.cnt_id);

	if (action.nxt_vld)
		printf("\t  Next input key: %u", action.next_input_key);

	if (action.rule_id_vld)
		printf("\t  Rule id: %u", action.rule_id);

	if (action.tc_ovrd_en)
		printf("\t  start qid:%u    Queue region size: %u",
		       action.qid, 1u << action.queue_region_size);
}

static void hikp_nic_show_fd_rules(const void *data)
{
	struct nic_fd_rules *rules = ((union nic_fd_feature_info *)data)->rules;
	uint16_t stage_no = g_fd_param.stage_no - 1;
	struct nic_fd_rules *stage_rules;
	struct nic_fd_key_cfg *key_cfg;
	struct nic_fd_rule_info *rule;
	uint16_t max_key_bytes;
	size_t one_rule_size;
	uint32_t i;

	key_cfg = &g_fd_hw_info.key_cfg[stage_no];
	stage_rules = &rules[stage_no];

	max_key_bytes = hikp_nic_get_tcam_data_size(g_fd_hw_info.key_max_bit);
	one_rule_size = sizeof(struct nic_fd_rule_info) +
			sizeof(uint8_t) * max_key_bytes * HIKP_NIC_KEY_DIR_NUM;

	printf("fd stage%d rules info[rule_num=%u]:\n", g_fd_param.stage_no, stage_rules->rule_cnt);
	for (i = 0; i < stage_rules->rule_cnt; i++) {
		rule = (struct nic_fd_rule_info *)((uint8_t *)(stage_rules->rule) +
		       i * one_rule_size);
		printf(" rule_idx: %u\n", rule->idx);
		if (rule->valid == 0) {
			printf("\tDriver doesn't configure the rule with this id!\n");
			return;
		}

		hikp_nic_fd_print_key(rule, key_cfg, max_key_bytes);

		/* The meta data position is unknown if fd mode is unknown. */
		if (g_fd_hw_info.mode <= FD_MODE_DEPTH_2K_WIDTH_200B_STAGE_2)
			hikp_nic_fd_print_meta_data(rule);

		hikp_nic_fd_print_ad_data(rule);
		printf("\n");
	}
}

static void hikp_nic_show_fd_counter(const void *data)
{
	struct nic_fd_counter *counter = ((union nic_fd_feature_info *)data)->counter;
	uint16_t stage_no = g_fd_param.stage_no - 1;
	struct nic_counter_entry *entry;
	uint32_t i;

	printf("fd stage%d counter info:\n", g_fd_param.stage_no);
	printf(" idx | hit_cnt\n");
	for (i = 0; i < counter[stage_no].counter_size; i++) {
		entry = &counter[stage_no].entry[i];
		printf(" %3u | %" PRIu64 "\n", entry->idx, entry->value);
	}
}

static int hikp_nic_fd_get_blk(struct hikp_cmd_header *req_header,
			       const struct nic_fd_req_para *req_data,
			       void *buf, size_t buf_len, struct nic_fd_rsp_head *rsp_head)
{
	struct hikp_cmd_ret *cmd_ret;
	struct nic_fd_rsp *rsp;
	int ret = 0;

	cmd_ret = hikp_cmd_alloc(req_header, req_data, sizeof(*req_data));
	if (hikp_rsp_normal_check(cmd_ret)) {
		ret = -EIO;
		goto out;
	}

	rsp = (struct nic_fd_rsp *)cmd_ret->rsp_data;
	if (rsp->rsp_head.cur_blk_size > buf_len ||
	    rsp->rsp_head.cur_blk_size > sizeof(rsp->rsp_data)) {
		HIKP_ERROR_PRINT("nic_fd block context copy size error, "
				 "dst buffer size=%zu, src buffer size=%zu, "
				 "data size=%u.\n", buf_len, sizeof(rsp->rsp_data),
				 rsp->rsp_head.cur_blk_size);
		ret = -EINVAL;
		goto out;
	}
	memcpy(buf, rsp->rsp_data, rsp->rsp_head.cur_blk_size);
	rsp_head->total_blk_num = rsp->rsp_head.total_blk_num;
	rsp_head->cur_blk_size = rsp->rsp_head.cur_blk_size;
	rsp_head->next_entry_idx = rsp->rsp_head.next_entry_idx;
	rsp_head->cur_blk_entry_cnt = rsp->rsp_head.cur_blk_entry_cnt;

out:
	hikp_cmd_free(&cmd_ret);
	return ret;
}

static int hikp_nic_query_fd_hw_info(struct hikp_cmd_header *req_header, const struct bdf_t *bdf,
				     uint8_t stage, void *data, size_t len)
{
	struct nic_fd_rsp_head rsp_head = {0};
	struct nic_fd_req_para req_data = {0};
	uint32_t total_blk_size;
	uint8_t total_blk_num;
	uint8_t blk_id = 0;
	int ret = 0;

	req_data.bdf = *bdf;
	req_data.block_id = blk_id;
	req_data.stage = stage;
	ret = hikp_nic_fd_get_blk(req_header, &req_data, data, len, &rsp_head);
	if (ret != 0) {
		HIKP_ERROR_PRINT("Fail to get block-%u context.\n", blk_id);
		return ret;
	}
	total_blk_num = rsp_head.total_blk_num;
	total_blk_size = rsp_head.cur_blk_size;

	/* Copy the remaining block content if total block number is greater than 1. */
	for (blk_id = 1; blk_id < total_blk_num; blk_id++) {
		req_data.block_id = blk_id;
		ret = hikp_nic_fd_get_blk(req_header, &req_data, (uint8_t *)data + total_blk_size,
					  len - total_blk_size, &rsp_head);
		if (ret != 0) {
			HIKP_ERROR_PRINT("Fail to get block-%u context.\n", blk_id);
			return ret;
		}
		total_blk_size += rsp_head.cur_blk_size;
	}

	return ret;
}

static int hikp_nic_query_fd_rules(struct hikp_cmd_header *req_header, const struct bdf_t *bdf,
				   uint8_t stage, void *data, size_t len)
{
	struct nic_fd_rsp_head rsp_head = {0};
	struct nic_fd_req_para req_data = {0};
	struct nic_fd_rule_info *entry;
	struct nic_fd_rules *rules;
	uint32_t entry_cnt = 0;
	uint16_t max_key_bytes;
	uint32_t left_buf_len;
	size_t one_rule_size;
	uint32_t idx;
	int ret = 0;

	HIKP_SET_USED(len);

	if (stage >= NIC_FD_STAGE_NUM) {
		HIKP_ERROR_PRINT("The fd stage number(%d) is error!\n", stage + 1);
		return -EIO;
	}

	rules = (struct nic_fd_rules *)data + stage;
	entry = rules->rule;
	max_key_bytes = hikp_nic_get_tcam_data_size(g_fd_hw_info.key_max_bit);
	one_rule_size = sizeof(struct nic_fd_rule_info) + max_key_bytes * HIKP_NIC_KEY_DIR_NUM;

	req_data.bdf = *bdf;
	req_data.stage = stage;
	req_data.query_single_entry = g_fd_param.id != -1 ? 1 : 0;
	idx = g_fd_param.id != -1 ? g_fd_param.id : 0;
	while (idx < g_fd_hw_info.alloc.stage_entry_num[stage]) {
		req_data.cur_entry_idx = idx;
		left_buf_len = one_rule_size *
			       (g_fd_hw_info.alloc.stage_entry_num[stage] - entry_cnt);
		ret = hikp_nic_fd_get_blk(req_header, &req_data,
					  (uint8_t *)entry + entry_cnt * one_rule_size,
					  left_buf_len, &rsp_head);
		if (ret != 0) {
			HIKP_ERROR_PRINT("Fail to get rules after entry idx=%u, ret=%d.\n",
					 idx, ret);
			return ret;
		}
		if (rsp_head.cur_blk_entry_cnt + entry_cnt > g_fd_hw_info.alloc.stage_entry_num[stage]) {
			HIKP_ERROR_PRINT("The sum of entry number (%u) after block-%u "
					 "is over the maximum entry number (%u) of this stage.",
					 rsp_head.cur_blk_entry_cnt + entry_cnt, idx,
					 g_fd_hw_info.alloc.stage_entry_num[stage]);
			return -EINVAL;
		}
		entry_cnt += rsp_head.cur_blk_entry_cnt;
		if (rsp_head.next_entry_idx <= idx) {
			HIKP_ERROR_PRINT("The next entry index (%u) is less than or equal with the curent(%u).\n",
					 rsp_head.next_entry_idx, idx);
			return -EINVAL;
		}
		idx = rsp_head.next_entry_idx;
		if (req_data.query_single_entry == 1)
			break;
	}
	rules->rule_cnt = entry_cnt;

	return ret;
}

static int hikp_nic_query_fd_counter(struct hikp_cmd_header *req_header, const struct bdf_t *bdf,
				     uint8_t stage, void *data, size_t len)
{
	struct nic_fd_rsp_head rsp_head = {0};
	struct nic_fd_req_para req_data = {0};
	struct nic_counter_entry *entry;
	struct nic_fd_counter *counter;
	uint32_t entry_size = 0;
	size_t left_buf_len;
	uint16_t idx;
	int ret = 0;

	HIKP_SET_USED(len);

	if (stage >= NIC_FD_STAGE_NUM) {
		HIKP_ERROR_PRINT("The fd stage number(%d) is error!\n", stage + 1);
		return -EIO;
	}

	counter = (struct nic_fd_counter *)data + stage;
	entry = counter->entry;

	req_data.bdf = *bdf;
	req_data.stage = stage;
	req_data.query_single_entry = g_fd_param.id != -1 ? 1 : 0;
	idx = g_fd_param.id != -1 ? g_fd_param.id : 0;
	while (idx < g_fd_hw_info.alloc.stage_counter_num[stage]) {
		req_data.cur_entry_idx = idx;
		left_buf_len = sizeof(struct nic_counter_entry) *
			       (g_fd_hw_info.alloc.stage_counter_num[stage] - entry_size);
		ret = hikp_nic_fd_get_blk(req_header, &req_data, entry + entry_size,
					  left_buf_len, &rsp_head);
		if (ret != 0) {
			HIKP_ERROR_PRINT("Fail to get the counter after entry idx=%u, ret=%d.\n",
					 idx, ret);
			return ret;
		}
		if (rsp_head.cur_blk_entry_cnt + entry_size > g_fd_hw_info.alloc.stage_counter_num[stage]) {
			HIKP_ERROR_PRINT("The sum of entry number (%u) after block-%u "
					 "is over the maximum counter number (%u) of this stage.",
					 rsp_head.cur_blk_entry_cnt + entry_size, idx,
					 g_fd_hw_info.alloc.stage_counter_num[stage]);
			return -EINVAL;
		}
		entry_size += rsp_head.cur_blk_entry_cnt;
		if (rsp_head.next_entry_idx <= idx) {
			HIKP_ERROR_PRINT("The next entry index (%u) is less than or equal with the curent(%u).\n",
					 rsp_head.next_entry_idx, idx);
			return -EINVAL;
		}
		idx = rsp_head.next_entry_idx;
		if (req_data.query_single_entry == 1)
			break;
	}
	counter->counter_size = entry_size;

	return ret;
}

static int hikp_nic_get_fd_hw_info(const struct bdf_t *bdf, struct nic_fd_hw_info *hw_info)
{
	struct hikp_cmd_header req_header = {0};

	if (!g_fd_feature_cmd[g_fd_param.feature_idx].need_query_hw_spec)
		return 0;

	hikp_cmd_init(&req_header, NIC_MOD, GET_FD_INFO_CMD, NIC_FD_HW_INFO_DUMP);
	return hikp_nic_query_fd_hw_info(&req_header, bdf, NIC_FD_STAGE_1, (void *)hw_info,
					 sizeof(*hw_info));
}

static int hikp_nic_fd_alloc_rules_buf(struct nic_fd_rules *rules,
				       struct nic_fd_hw_info *hw_info, uint16_t stage_no)
{
	uint16_t max_key_bytes;

	max_key_bytes = hikp_nic_get_tcam_data_size(hw_info->key_max_bit);
	rules[stage_no].rule =
		(struct nic_fd_rule_info *)calloc(hw_info->alloc.stage_entry_num[stage_no],
		sizeof(struct nic_fd_rule_info) +
		sizeof(uint8_t) * max_key_bytes * HIKP_NIC_KEY_DIR_NUM);
	if (rules[stage_no].rule == NULL) {
		HIKP_ERROR_PRINT("Fail to alloc rule memory of stage1.\n");
		return -ENOMEM;
	}

	return 0;
}

static int hikp_nic_fd_alloc_counter_buf(struct nic_fd_counter *counter,
					 struct nic_fd_hw_info *hw_info, uint16_t stage_no)
{
	counter[stage_no].entry =
		(struct nic_counter_entry *)calloc(hw_info->alloc.stage_counter_num[stage_no],
		sizeof(struct nic_counter_entry));
	if (counter[stage_no].entry == NULL) {
		HIKP_ERROR_PRINT("Fail to alloc counter memory of stage1.\n");
		return -ENOMEM;
	}

	return 0;
}

static union nic_fd_feature_info *hikp_nic_fd_data_alloc(const struct fd_feature_cmd *fd_cmd)
{
	uint16_t stage_no = g_fd_param.stage_no - 1;
	union nic_fd_feature_info *fd_data;
	int ret = 0;

	fd_data = (union nic_fd_feature_info *)calloc(1, sizeof(union nic_fd_feature_info));
	if (fd_data == NULL) {
		HIKP_ERROR_PRINT("Fail to allocate nic_ppp_feature_info memory.\n");
		return NULL;
	}

	if (strcmp(fd_cmd->feature_name, NIC_FD_RULES_NAME) == 0)
		ret = hikp_nic_fd_alloc_rules_buf(fd_data->rules, &g_fd_hw_info, stage_no);
	else if (strcmp(fd_cmd->feature_name, NIC_FD_COUNTER_NAME) == 0)
		ret = hikp_nic_fd_alloc_counter_buf(fd_data->counter, &g_fd_hw_info, stage_no);
	if (ret != 0)
		goto out;

	return fd_data;

out:
	free(fd_data);
	return NULL;
}

static void hikp_nic_fd_data_free(union nic_fd_feature_info *fd_data)
{
	uint16_t stage_no = g_fd_param.stage_no - 1;
	const struct fd_feature_cmd *fd_cmd;

	fd_cmd = &g_fd_feature_cmd[g_fd_param.feature_idx];
	if (strcmp(fd_cmd->feature_name, NIC_FD_RULES_NAME) == 0) {
		free(fd_data->rules[stage_no].rule);
		fd_data->rules[stage_no].rule = NULL;
	} else if (strcmp(fd_cmd->feature_name, NIC_FD_COUNTER_NAME) == 0) {
		free(fd_data->counter[stage_no].entry);
		fd_data->counter[stage_no].entry = NULL;
	}

	free(fd_data);
}

static int hikp_nic_check_fd_hw_info(const struct nic_fd_hw_info *hw_info,
				     const struct fd_feature_cmd *fd_cmd)
{
	uint16_t max_key_bytes, active_key_bits;
	uint16_t i;

	if (strcmp(fd_cmd->feature_name, NIC_FD_RULES_NAME) == 0) {
		/* Stage2 does not support query. So only stage1 is verified. */
		if(hw_info->alloc.stage_entry_num[NIC_FD_STAGE_1] == 0) {
			HIKP_ERROR_PRINT("The stage1's entry number is zero.\n");
			return -EINVAL;
		}
		if (hw_info->mode > FD_MODE_DEPTH_2K_WIDTH_200B_STAGE_2)
			HIKP_WARN_PRINT("Unknown fd mode(%u), "
					"unsupport for displaying meta data info.\n",
					hw_info->mode);

		for (i = 0; i < NIC_FD_STAGE_NUM; i++) {
			if (hw_info->alloc.stage_entry_num[i] != 0 &&
				hw_info->key_cfg[i].key_select != HNS3_FD_KEY_BASE_ON_TUPLE) {
				HIKP_ERROR_PRINT("Only support for displaying the tuple key info, "
						 "stage-%d key_select=%u\n",
						 i + 1, hw_info->key_cfg[i].key_select);
				return -EOPNOTSUPP;
			}
		}

		max_key_bytes = hikp_nic_get_tcam_data_size(hw_info->key_max_bit);
		active_key_bits = hikp_nic_get_max_key_len(hw_info->mode);
		if (active_key_bits > max_key_bytes * HIKP_BITS_PER_BYTE) {
			HIKP_ERROR_PRINT("The active tcam bits(%u) is more than the max key bits(%d).\n",
					 active_key_bits, max_key_bytes * HIKP_BITS_PER_BYTE);
			return -EINVAL;
		}
	} else if (strcmp(fd_cmd->feature_name, NIC_FD_COUNTER_NAME) == 0) {
		if (hw_info->alloc.stage_counter_num[NIC_FD_STAGE_1] == 0) {
			HIKP_ERROR_PRINT("The stage1's counter number is zero.\n");
			return -EINVAL;
		}
	}

	return 0;
}

static int hikp_nic_fd_check_entry_index_valid(struct major_cmd_ctrl *self,
					       struct nic_fd_param *fd_param,
					       const struct fd_feature_cmd *fd_cmd,
					       const struct nic_fd_hw_info *hw_info)
{
	uint32_t hw_entry_size;
	uint8_t stage_no;

	/* Querying single entry info need to check entry_id validity. */
	if (fd_param->id != -1) {
		stage_no = fd_param->stage_no - 1;
		hw_entry_size = fd_cmd->sub_cmd_code == NIC_FD_RULES_INFO_DUMP ?
				hw_info->alloc.stage_entry_num[stage_no] :
				hw_info->alloc.stage_counter_num[stage_no];
		if ((uint32_t)fd_param->id >= hw_entry_size) {
			snprintf(self->err_str, sizeof(self->err_str),
				 "entry id(%d) must be less than hardware specifications(%u).",
				 fd_param->id, hw_entry_size);
			self->err_no = -EINVAL;
			return self->err_no;
		}
	}

	return 0;
}

static int hikp_nic_fd_check_input_param(struct major_cmd_ctrl *self,
					 const struct nic_fd_param *fd_param)
{
	const struct bdf_t *bdf = &fd_param->target.bdf;
	const struct fd_feature_cmd *fd_cmd;

	if (bdf->dev_id != 0) {
		snprintf(self->err_str, sizeof(self->err_str), "VF does not support query!");
		return -EINVAL;
	}

	if (fd_param->feature_idx == -1) {
		hikp_nic_fd_cmd_help(self, NULL);
		snprintf(self->err_str, sizeof(self->err_str), "-du/--dump parameter error!");
		return -EINVAL;
	}

	fd_cmd = &g_fd_feature_cmd[g_fd_param.feature_idx];
	if (fd_param->stage_no == -1 && fd_cmd->sub_cmd_code != NIC_FD_HW_INFO_DUMP) {
		snprintf(self->err_str, sizeof(self->err_str),
			 "please input '-st/--stage' parameter.");
		return -EINVAL;
	}

	if (fd_cmd->sub_cmd_code == NIC_FD_HW_INFO_DUMP &&
	    (fd_param->id != -1 || fd_param->stage_no != -1)) {
		snprintf(self->err_str, sizeof(self->err_str),
			 "no need '-id/--index' and '-st/--stage' parameter.");
		return -EINVAL;
	}

	return 0;
}

void hikp_nic_fd_cmd_execute(struct major_cmd_ctrl *self)
{
	struct bdf_t *bdf = &g_fd_param.target.bdf;
	const struct fd_feature_cmd *fd_cmd;
	union nic_fd_feature_info *fd_data;
	struct hikp_cmd_header req_header = {0};
	int ret;

	ret = hikp_nic_fd_check_input_param(self, &g_fd_param);
	if (ret != 0) {
		self->err_no = ret;
		return;
	}

	ret = hikp_nic_get_fd_hw_info(bdf, &g_fd_hw_info);
	if (ret != 0) {
		snprintf(self->err_str, sizeof(self->err_str),
			 "fail to obtain fd hardware configuration.");
		self->err_no = ret;
		return;
	}

	fd_cmd = &g_fd_feature_cmd[g_fd_param.feature_idx];
	ret = hikp_nic_check_fd_hw_info(&g_fd_hw_info, fd_cmd);
	if (ret != 0) {
		self->err_no = ret;
		return;
	}

	ret = hikp_nic_fd_check_entry_index_valid(self, &g_fd_param, fd_cmd, &g_fd_hw_info);
	if (ret != 0)
		return;

	fd_data = hikp_nic_fd_data_alloc(fd_cmd);
	if (fd_data == NULL) {
		HIKP_ERROR_PRINT("Fail to alloc fd data memory.\n");
		self->err_no = -ENOMEM;
		return;
	}

	/* The 'hw_info' cmd no need to input stage number,
	 * because it queries all stages information.
	 */
	hikp_cmd_init(&req_header, NIC_MOD, GET_FD_INFO_CMD, fd_cmd->sub_cmd_code);
	ret = fd_cmd->query(&req_header, bdf, g_fd_param.stage_no - 1, fd_data, sizeof(*fd_data));
	if (ret != 0) {
		snprintf(self->err_str, sizeof(self->err_str), "failed to query %s info, ret = %d.",
			 fd_cmd->feature_name, ret);
		self->err_no = ret;
		goto out;
	}

	printf("############## NIC FD: %s info ############\n", fd_cmd->feature_name);
	fd_cmd->show(fd_data);
	printf("#################### END #######################\n");

out:
	hikp_nic_fd_data_free(fd_data);
}

int hikp_nic_cmd_get_fd_target(struct major_cmd_ctrl *self, const char *argv)
{
	self->err_no = tool_check_and_get_valid_bdf_id(argv, &(g_fd_param.target));
	if (self->err_no != 0) {
		snprintf(self->err_str, sizeof(self->err_str), "unknown device!");
		return self->err_no;
	}

	return 0;
}

static int hikp_nic_cmd_fd_feature_select(struct major_cmd_ctrl *self, const char *argv)
{
	size_t feat_size = HIKP_ARRAY_SIZE(g_fd_feature_cmd);
	size_t i;

	for (i = 0; i < feat_size; i++) {
		if (strcmp(argv, g_fd_feature_cmd[i].feature_name) == 0) {
			g_fd_param.feature_idx = i;
			return 0;
		}
	}

	hikp_nic_fd_cmd_help(self, NULL);
	snprintf(self->err_str, sizeof(self->err_str), "please input valid subfunction.");
	self->err_no = -EINVAL;

	return self->err_no;
}

static int hikp_nic_cmd_fd_parse_index(struct major_cmd_ctrl *self, const char *argv)
{
#define HIKP_UINT16_MAX 0xffff
	uint32_t id;

	self->err_no = string_toui(argv, &id);
	if (self->err_no) {
		snprintf(self->err_str, sizeof(self->err_str),
			 "parse --id/--index parameter failed.");
		return self->err_no;
	}

	if (id > HIKP_UINT16_MAX) {
		snprintf(self->err_str, sizeof(self->err_str),
			 "the id is truncated. it should less than %u.", HIKP_UINT16_MAX);
		self->err_no = -EINVAL;
		return self->err_no;
	}

	g_fd_param.id = (uint16_t)id;

	return 0;
}

static int hikp_nic_cmd_fd_parse_stage(struct major_cmd_ctrl *self, const char *argv)
{
	uint32_t stage_no;

	self->err_no = string_toui(argv, &stage_no);
	if (self->err_no) {
		snprintf(self->err_str, sizeof(self->err_str),
			 "parse -stage/--stage_no parameter failed.");
		return self->err_no;
	}

	if (stage_no == 0 || stage_no > NIC_FD_STAGE_NUM) {
		snprintf(self->err_str, sizeof(self->err_str),
			 "-stage/--stage_no parameter is invalid, please use 1 or 2!");
		self->err_no = -EINVAL;
		return self->err_no;
	}

	if (stage_no - 1 == NIC_FD_STAGE_2) {
		snprintf(self->err_str, sizeof(self->err_str),
			 "unsupport for querying stage%u entry!", stage_no);
		self->err_no = -EOPNOTSUPP;
		return self->err_no;
	}

	g_fd_param.stage_no = (uint16_t)stage_no;

	return 0;
}

static void cmd_nic_get_fd_init(void)
{
	struct major_cmd_ctrl *major_cmd = get_major_cmd();

	g_fd_param.id = -1;
	g_fd_param.stage_no = -1;
	g_fd_param.feature_idx = -1;
	major_cmd->option_count = 0;
	major_cmd->execute = hikp_nic_fd_cmd_execute;

	cmd_option_register("-h", "--help", false, hikp_nic_fd_cmd_help);
	cmd_option_register("-i", "--interface", true, hikp_nic_cmd_get_fd_target);
	cmd_option_register("-du", "--dump", true, hikp_nic_cmd_fd_feature_select);
	cmd_option_register("-id", "--index", true, hikp_nic_cmd_fd_parse_index);
	cmd_option_register("-st", "--stage", true, hikp_nic_cmd_fd_parse_stage);
}

HIKP_CMD_DECLARE("nic_fd", "dump fd info of nic!", cmd_nic_get_fd_init);
