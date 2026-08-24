/*
 * Copyright (c) 2024 Hisilicon Technologies Co., Ltd.
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

#include "sub_health.h"
#include "sh_json.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "sub_health.h"
#include "sh_json.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include "tool_lib.h"
/* 电互连 L1 名称 */
#define ELECTRIC_L1_NAME "1D-FULLMESSH"

/* ========================================================================
 * 内部拓扑表示（扁平节点列表 + L1 索引）
 * ======================================================================== */

struct plan_node {
	char ip[64];
	int l1_id;                                  /* 所属 L1 索引 */
	int ubpu_num;                               /* UBPU 端口数量 */
	char ubpu_eids[MAX_UBPU_NUM][MAX_EID_LEN];  /* 每 UBPU 的 EID */
};

struct plan_l1 {
	char name[64];
	int node_count;
	int node_indices[MAX_NODE_NUM];
	bool redundant;
};

static struct plan_node g_nodes[MAX_NODE_NUM];
static int g_node_count = 0;

static struct plan_l1 g_l1s[MAX_L1_SWITCH_NUM];
static int g_l1_count = 0;

static uint32_t g_l2_num = 0;

static int read_file_content(const char *file, char **out_content)
{
	FILE *fp;
	long fsize;
	char *buf;
	size_t nread;

	if (file == NULL || out_content == NULL)
		return -EINVAL;

	*out_content = NULL;

	fp = fopen(file, "rb");
	if (fp == NULL)
		return -errno;

	if (fseek(fp, 0, SEEK_END) != 0) {
		fclose(fp);
		return -EIO;
	}

	fsize = ftell(fp);
	if (fsize < 0) {
		fclose(fp);
		return -EIO;
	}

	if (fseek(fp, 0, SEEK_SET) != 0) {
		fclose(fp);
		return -EIO;
	}

	if (fsize == 0 || fsize > MAX_JSON_BUF_SIZE) {
		fclose(fp);
		return -EINVAL;
	}

	buf = malloc((size_t)fsize + 1);
	if (buf == NULL) {
		fclose(fp);
		return -ENOMEM;
	}

	nread = fread(buf, 1, (size_t)fsize, fp);
	if (nread != (size_t)fsize) {
		fclose(fp);
		free(buf);
		return -EIO;
	}

	if (fclose(fp) != 0) {
		free(buf);
		return -EIO;
	}

	buf[fsize] = '\0';
	*out_content = buf;
	return 0;
}

/* ========================================================================
 * 解析拓扑 JSON（格式见 spec §7.1）
 * ======================================================================== */
static void reset_topology(void)
{
	g_node_count = 0;
	g_l1_count = 0;
	g_l2_num = 0;

	memset(g_nodes, 0, sizeof(g_nodes));
	memset(g_l1s, 0, sizeof(g_l1s));
}

static int parse_ubpu_id(const char *text, int *ubpu_id)
{
	const char *p;
	int value = 0;

	if (text == NULL || text[0] == '\0' || ubpu_id == NULL)
		return -EINVAL;

	for (p = text; *p != '\0'; p++) {
		int digit;

		if (*p < '0' || *p > '9')
			return -EINVAL;

		digit = *p - '0';

		/*
		 * Check the range before multiplication to prevent
		 * integer overflow caused by an excessively long ID.
		 */
		if (value > (MAX_UBPU_NUM - 1) / 10)
			return -ERANGE;

		value = value * 10 + digit;
		if (value >= MAX_UBPU_NUM)
			return -ERANGE;
	}

	*ubpu_id = value;
	return 0;
}

static int parse_ubpu_entries(sh_json *node_obj,
			      struct plan_node *node)
{
	bool seen[MAX_UBPU_NUM] = { false };
	sh_json *port_item;
	int valid_count = 0;
	int i;

	if (node_obj == NULL || node == NULL)
		return -EINVAL;

	for (port_item = node_obj->head;
	     port_item != NULL;
	     port_item = port_item->fwd) {
		int ubpu_id;
		int written;
		int ret;

		if (port_item->name == NULL ||
		    port_item->kind != SH_JSON_BUF ||
		    port_item->str == NULL ||
		    port_item->str[0] == '\0') {
			HIKP_ERROR_PRINT(
				"Invalid UBPU entry for node %s\n",
				node->ip);
			return -EINVAL;
		}

		ret = parse_ubpu_id(port_item->name, &ubpu_id);
		if (ret != 0) {
			HIKP_ERROR_PRINT(
				"Invalid UBPU ID '%s' for node %s\n",
				port_item->name, node->ip);
			return ret;
		}

		if (seen[ubpu_id]) {
			HIKP_ERROR_PRINT(
				"Duplicate UBPU ID %d for node %s\n",
				ubpu_id, node->ip);
			return -EINVAL;
		}

		written = snprintf(node->ubpu_eids[ubpu_id],
				   sizeof(node->ubpu_eids[ubpu_id]),
				   "%s", port_item->str);
		if (written < 0 ||
		    (size_t)written >= sizeof(node->ubpu_eids[ubpu_id])) {
			HIKP_ERROR_PRINT(
				"EID is too long: node=%s, UBPU=%d\n",
				node->ip, ubpu_id);
			return -E2BIG;
		}

		seen[ubpu_id] = true;
		valid_count++;

		/*
		 * ubpu_num is the maximum UBPU ID plus one.
		 */
		if (ubpu_id >= node->ubpu_num)
			node->ubpu_num = ubpu_id + 1;
	}

	if (valid_count == 0) {
		HIKP_ERROR_PRINT(
			"No UBPU entry found for node %s\n",
			node->ip);
		return -EINVAL;
	}

	/*
	 * The subsequent code iterates from 0 to ubpu_num - 1,
	 * so missing UBPU IDs are not allowed.
	 */
	for (i = 0; i < node->ubpu_num; i++) {
		if (!seen[i]) {
			HIKP_ERROR_PRINT(
				"Missing UBPU ID %d for node %s\n",
				i, node->ip);
			return -EINVAL;
		}
	}

	return 0;
}

static int parse_l1_nodes(sh_json *l1_obj,
			  struct plan_l1 *l1_info)
{
	sh_json *ip_item;

	if (l1_obj == NULL || l1_info == NULL)
		return -EINVAL;

	for (ip_item = l1_obj->head;
	     ip_item != NULL;
	     ip_item = ip_item->fwd) {
		struct plan_node *node;
		struct in_addr addr;
		int written;
		int ret;

		if (ip_item->name == NULL ||
		    ip_item->kind != SH_JSON_MAP ||
		    inet_pton(AF_INET, ip_item->name, &addr) != 1) {
			HIKP_ERROR_PRINT(
				"Invalid node under L1 %s\n",
				l1_info->name);
			return -EINVAL;
		}

		if (g_node_count >= MAX_NODE_NUM) {
			HIKP_ERROR_PRINT(
				"Too many topology nodes: maximum=%d\n",
				MAX_NODE_NUM);
			return -E2BIG;
		}

		if (l1_info->node_count >= MAX_NODE_PER_L1) {
			HIKP_ERROR_PRINT(
				"Too many nodes under L1 %s: maximum=%d\n",
				l1_info->name, MAX_NODE_PER_L1);
			return -E2BIG;
		}

		node = &g_nodes[g_node_count];

		written = snprintf(node->ip, sizeof(node->ip),
				   "%s", ip_item->name);
		if (written < 0 ||
		    (size_t)written >= sizeof(node->ip)) {
			HIKP_ERROR_PRINT(
				"Node IP is too long: %s\n",
				ip_item->name);
			return -E2BIG;
		}

		/*
		 * g_l1_count is the index of the current L1.
		 * It is incremented after all nodes are parsed.
		 */
		node->l1_id = g_l1_count;
		node->ubpu_num = 0;

		ret = parse_ubpu_entries(ip_item, node);
		if (ret != 0)
			return ret;

		l1_info->node_indices[l1_info->node_count] =
			g_node_count;

		l1_info->node_count++;
		g_node_count++;
	}

	return 0;
}

static int parse_l1_switches(sh_json *l1_switches)
{
	sh_json *l1;

	if (l1_switches == NULL ||
	    l1_switches->kind != SH_JSON_MAP)
		return -EINVAL;

	for (l1 = l1_switches->head;
	     l1 != NULL;
	     l1 = l1->fwd) {
		struct plan_l1 *l1_info;
		int written;
		int ret;

		if (l1->kind != SH_JSON_MAP ||
		    l1->name == NULL ||
		    l1->name[0] == '\0') {
			HIKP_ERROR_PRINT("Invalid L1 switch entry\n");
			return -EINVAL;
		}

		if (g_l1_count >= MAX_L1_SWITCH_NUM) {
			HIKP_ERROR_PRINT(
				"Too many L1 switches: maximum=%d\n",
				MAX_L1_SWITCH_NUM);
			return -E2BIG;
		}

		l1_info = &g_l1s[g_l1_count];

		written = snprintf(l1_info->name,
				   sizeof(l1_info->name),
				   "%s", l1->name);
		if (written < 0 ||
		    (size_t)written >= sizeof(l1_info->name)) {
			HIKP_ERROR_PRINT(
				"L1 switch name is too long: %s\n",
				l1->name);
			return -E2BIG;
		}

		l1_info->node_count = 0;

		ret = parse_l1_nodes(l1, l1_info);
		if (ret != 0)
			return ret;

		if (l1_info->node_count == 0) {
			HIKP_ERROR_PRINT(
				"No node found under L1 %s\n",
				l1_info->name);
			return -EINVAL;
		}

		g_l1_count++;
	}

	return 0;
}

static int parse_topology(const char *content)
{
	sh_json *root = NULL;
	sh_json *l2_arr;
	sh_json *l2_item;
	sh_json *l1_switches;
	int l2_count;
	int ret = -EINVAL;

	/*
	 * Do not retain topology data from an earlier parse.
	 */
	reset_topology();

	if (content == NULL) {
		HIKP_ERROR_PRINT("Topology content is NULL\n");
		return -EINVAL;
	}

	root = sh_json_parse(content);
	if (root == NULL) {
		HIKP_ERROR_PRINT("Failed to parse topology JSON\n");
		return -EINVAL;
	}

	if (root->kind != SH_JSON_MAP) {
		HIKP_ERROR_PRINT(
			"Topology JSON root must be an object\n");
		goto out;
	}

	l2_arr = sh_json_get_item(root, "l2_switches");
	if (l2_arr == NULL || l2_arr->kind != SH_JSON_SEQ) {
		HIKP_ERROR_PRINT("Invalid l2_switches field\n");
		goto out;
	}

	l2_count = sh_json_item_count(l2_arr);
	if (l2_count > MAX_L2_SWITCH_NUM) {
		HIKP_ERROR_PRINT(
			"Too many L2 switches: count=%d, maximum=%d\n",
			l2_count, MAX_L2_SWITCH_NUM);
		ret = -E2BIG;
		goto out;
	}

	for (l2_item = l2_arr->head;
	     l2_item != NULL;
	     l2_item = l2_item->fwd) {
		if (l2_item->kind != SH_JSON_BUF ||
		    l2_item->str == NULL ||
		    l2_item->str[0] == '\0') {
			HIKP_ERROR_PRINT(
				"Invalid L2 switch entry\n");
			goto out;
		}
	}

	g_l2_num = (uint32_t)l2_count;

	l1_switches = sh_json_get_item(root, "l1_switches");
	if (l1_switches == NULL ||
	    l1_switches->kind != SH_JSON_MAP) {
		HIKP_ERROR_PRINT("Invalid l1_switches field\n");
		goto out;
	}

	ret = parse_l1_switches(l1_switches);
	if (ret != 0)
		goto out;

	if (g_node_count == 0) {
		HIKP_ERROR_PRINT(
			"No valid node found in topology\n");
		ret = -EINVAL;
		goto out;
	}

	ret = 0;

out:
	sh_json_delete(root);

	if (ret != 0)
		reset_topology();

	return ret;
}
/* ========================================================================
 * L1 冗余检测：两 L1 节点 IP 交集 > 1 则后者标记冗余
 * ======================================================================== */
static int count_ip_intersection(const struct plan_l1 *a,
				 const struct plan_l1 *b)
{
	int count = 0;
	int i, j;

	for (i = 0; i < a->node_count; i++) {
		for (j = 0; j < b->node_count; j++) {
			if (strcmp(g_nodes[a->node_indices[i]].ip,
				   g_nodes[b->node_indices[j]].ip) == 0) {
				count++;
				break;
			}
		}
	}
	return count;
}

static void detect_redundant_l1_switches(void)
{
	int i, j;

	for (i = 0; i < g_l1_count; i++) {
		for (j = i + 1; j < g_l1_count; j++) {
			if (count_ip_intersection(&g_l1s[i], &g_l1s[j]) > 1)
				g_l1s[j].redundant = true;
		}
	}
}

/* 候选 L1 与本 L1 是否有任一 IP 重叠 */
static int has_ip_overlap(const struct plan_l1 *a, const struct plan_l1 *b)
{
	return count_ip_intersection(a, b) > 0;
}
/* ========================================================================
 * 探测发包数计算
 * ======================================================================== */
static void calc_packet_counts(uint32_t coverage_k, uint32_t l2_num,
			       bool is_electric_link,
			       uint32_t *packet_count_intra,
			       uint32_t *packet_count_inter)
{
	if (is_electric_link) {
		*packet_count_intra = coverage_k;
		*packet_count_inter = 0;
	} else {
		*packet_count_intra = coverage_k * CPU_PORT_COUNT;
		*packet_count_inter = (l2_num == 0) ?
					0 : *packet_count_intra * l2_num;
	}
}

/*
 * Add a newly created and unattached item to an object.
 *
 * On success, ownership of item is transferred to object.
 * On failure, item remains owned by the caller.
 */

/* ========================================================================
 * intra-L1 目的端口列表：同 L1 下同索引 ubpu 互探
 * ======================================================================== */
static int collect_intra_dsts(const struct plan_node *src_node, int ubpu_id,
				  sh_json *dst_array)
{
	const struct plan_l1 *l1;
	int i;

	if (src_node == NULL || dst_array == NULL)
		return -EINVAL;

	if (src_node->l1_id < 0 || src_node->l1_id >= g_l1_count)
		return -EINVAL;

	if (ubpu_id < 0 || ubpu_id >= src_node->ubpu_num)
		return -EINVAL;

	if (dst_array->kind != SH_JSON_SEQ)
		return -EINVAL;

	l1 = &g_l1s[src_node->l1_id];

	for (i = 0; i < l1->node_count; i++) {
		const struct plan_node *other;
		sh_json *dst_item;
		int node_index = l1->node_indices[i];

		if (node_index < 0 || node_index >= g_node_count)
			return -EINVAL;

		other = &g_nodes[node_index];

		if (other == src_node)
			continue;

		if (ubpu_id >= other->ubpu_num)
			continue;

		if (other->ubpu_eids[ubpu_id][0] == '\0')
			continue;

		dst_item = sh_json_create_str(other->ubpu_eids[ubpu_id]);
		if (dst_item == NULL)
			return -ENOMEM;

		sh_json_push(dst_array, dst_item);
	}

	return 0;
}
/* ========================================================================
 * inter-L1 目的端口列表：先去重再遍历，每候选 L1 最多 3 个节点
 * ======================================================================== */
static int collect_inter_dsts(const struct plan_node *src_node, int ubpu_id,
				  sh_json *dst_array)
{
	const struct plan_l1 *src_l1;
	int i;

	if (src_node == NULL || dst_array == NULL)
		return -EINVAL;

	if (src_node->l1_id < 0 || src_node->l1_id >= g_l1_count)
		return -EINVAL;

	if (ubpu_id < 0 || ubpu_id >= src_node->ubpu_num)
		return -EINVAL;

	if (dst_array->kind != SH_JSON_SEQ)
		return -EINVAL;

	src_l1 = &g_l1s[src_node->l1_id];

	for (i = 0; i < g_l1_count; i++) {
		const struct plan_l1 *cand = &g_l1s[i];
		int select_num = 0;
		int j;

		/* Exclude the source L1. */
		if (i == src_node->l1_id)
			continue;

		/* Exclude globally redundant L1s. */
		if (cand->redundant)
			continue;

		/* Exclude L1s with overlapping IP addresses. */
		if (has_ip_overlap(src_l1, cand))
			continue;

		for (j = 0; j < cand->node_count &&
				select_num < MAX_CANDIDATE_PER_L1; j++) {
			const struct plan_node *other;
			sh_json *dst_item;
			int node_index = cand->node_indices[j];

			if (node_index < 0 || node_index >= g_node_count)
				return -EINVAL;

			other = &g_nodes[node_index];

			if (ubpu_id >= other->ubpu_num)
				continue;

			if (other->ubpu_eids[ubpu_id][0] == '\0')
				continue;

			dst_item =
				sh_json_create_str(other->ubpu_eids[ubpu_id]);
			if (!dst_item)
				return -ENOMEM;

			sh_json_push(dst_array, dst_item);

			/* Count only destinations actually added. */
			select_num++;
				}
	}

	return 0;
}
/* ========================================================================
 * 将全局 L1 的 EID 映射表直接写入节点 JSON
 * ======================================================================== */
static int add_global_l1_maps(sh_json *root)
{
	sh_json *maps_obj;
	int i;
	int ret;

	if (root == NULL)
		return -EINVAL;

	maps_obj = sh_json_create_obj();
	if (maps_obj == NULL)
		return -ENOMEM;

	for (i = 0; i < g_l1_count; i++) {
		const struct plan_l1 *l1 = &g_l1s[i];
		sh_json *eids_arr;
		int j;
		int k;

		if (l1->redundant)
			continue;

		eids_arr = sh_json_create_arr();
		if (eids_arr == NULL) {
			ret = -ENOMEM;
			goto err_maps;
		}

		for (j = 0; j < l1->node_count; j++) {
			const struct plan_node *node;
			int node_index = l1->node_indices[j];

			if (node_index < 0 || node_index >= g_node_count) {
				ret = -EINVAL;
				goto err_array;
			}

			node = &g_nodes[node_index];

			for (k = 0; k < node->ubpu_num; k++) {
				sh_json *eid_item;

				if (node->ubpu_eids[k][0] == '\0')
					continue;

				eid_item =
					sh_json_create_str(node->ubpu_eids[k]);
				if (eid_item == NULL) {
					ret = -ENOMEM;
					goto err_array;
				}

				sh_json_push(eids_arr, eid_item);
			}
		}

		if (!sh_json_attach_checked(maps_obj, l1->name,
						  eids_arr)) {
			ret = -ENOMEM;
			goto err_array;
						  }

		/* eids_arr is now owned by maps_obj. */
		continue;

		err_array:
				sh_json_delete(eids_arr);
		goto err_maps;
	}

	if (!sh_json_attach_checked(root, "_l1_maps", maps_obj)) {
		ret = -ENOMEM;
		goto err_maps;
	}

	/* maps_obj is now owned by root. */
	return 0;

	err_maps:
		sh_json_delete(maps_obj);
	return ret;
}
static int build_ubpu_probe_json(const struct plan_node *src_node,
				 int ubpu_id,
				 uint32_t packet_count_intra,
				 uint32_t packet_count_inter,
				 uint32_t packet_size,
				 sh_json **out)
{
	sh_json *obj = NULL;
	sh_json *intra_arr = NULL;
	sh_json *inter_arr = NULL;
	int ret;

	if (out == NULL)
		return -EINVAL;

	*out = NULL;

	if (src_node == NULL ||
		ubpu_id < 0 ||
		ubpu_id >= src_node->ubpu_num ||
		src_node->ubpu_eids[ubpu_id][0] == '\0')
		return -EINVAL;

	obj = sh_json_create_obj();
	if (obj == NULL)
		return -ENOMEM;

	if (sh_json_put_num(obj, "packet_count_intra",
					packet_count_intra) == NULL) {
		ret = -ENOMEM;
		goto err;
					}

	if (sh_json_put_num(obj, "packet_count_inter",
					packet_count_inter) == NULL) {
		ret = -ENOMEM;
		goto err;
					}

	if (sh_json_put_num(obj, "packet_size",
					packet_size) == NULL) {
		ret = -ENOMEM;
		goto err;
					}

	if (!sh_json_put_str_checked(
			obj, "src_eid", src_node->ubpu_eids[ubpu_id])) {
		ret = -ENOMEM;
		goto err;
			}

	intra_arr = sh_json_create_arr();
	if (intra_arr == NULL) {
		ret = -ENOMEM;
		goto err;
	}

	ret = collect_intra_dsts(src_node, ubpu_id, intra_arr);
	if (ret != 0)
		goto err;

	if (!sh_json_attach_checked(
			obj, "intra_l1_dst_eids", intra_arr)) {
		ret = -ENOMEM;
		goto err;
			}

	/* Ownership transferred to obj. */
	intra_arr = NULL;

	inter_arr = sh_json_create_arr();
	if (inter_arr == NULL) {
		ret = -ENOMEM;
		goto err;
	}

	ret = collect_inter_dsts(src_node, ubpu_id, inter_arr);
	if (ret != 0)
		goto err;

	if (!sh_json_attach_checked(
			obj, "inter_l1_dst_eids", inter_arr)) {
		ret = -ENOMEM;
		goto err;
			}

	/* Ownership transferred to obj. */
	inter_arr = NULL;

	*out = obj;
	return 0;

	err:
		/*
		 * Non-NULL arrays have not been attached to obj yet.
		 * Attached arrays are recursively released with obj.
		 */
		sh_json_delete(inter_arr);
	sh_json_delete(intra_arr);
	sh_json_delete(obj);
	return ret;
}
/* ========================================================================
 * 生成完整探测计划 JSON
 * ======================================================================== */
static int generate_probe_plan_json(uint32_t coverage_k,
					uint32_t packet_count_intra,
					uint32_t packet_count_inter,
					uint32_t packet_size,
					sh_json **out)
{
	sh_json *config;
	sh_json *root;
	int ret;
	int i;

	if (out == NULL)
		return -EINVAL;

	*out = NULL;

	root = sh_json_create_obj();
	if (root == NULL)
		return -ENOMEM;

	config = sh_json_create_obj();
	if (config == NULL) {
		ret = -ENOMEM;
		goto err_root;
	}

	if (sh_json_put_num(config, "coverage_k", coverage_k) == NULL) {
		sh_json_delete(config);
		ret = -ENOMEM;
		goto err_root;
	}

	if (!sh_json_attach_checked(root, "_probe_config", config)) {
		sh_json_delete(config);
		ret = -ENOMEM;
		goto err_root;
	}

	ret = add_global_l1_maps(root);
	if (ret != 0)
		goto err_root;

	for (i = 0; i < g_node_count; i++) {
		const struct plan_node *node = &g_nodes[i];
		sh_json *node_obj;
		int u;

		if (node->l1_id < 0 || node->l1_id >= g_l1_count) {
			ret = -EINVAL;
			goto err_root;
		}

		if (g_l1s[node->l1_id].redundant)
			continue;

		node_obj = sh_json_create_obj();
		if (node_obj == NULL) {
			ret = -ENOMEM;
			goto err_root;
		}

		for (u = 0; u < node->ubpu_num; u++) {
			sh_json *ubpu_obj = NULL;
			char ubpu_key[16];
			int written;

			if (node->ubpu_eids[u][0] == '\0')
				continue;

			ret = build_ubpu_probe_json(
				node, u,
				packet_count_intra,
				packet_count_inter,
				packet_size,
				&ubpu_obj);
			if (ret != 0) {
				HIKP_ERROR_PRINT(
					"Failed to build probe plan: "
					"node=%s, UBPU=%d, error=%d\n",
					node->ip, u, ret);
				goto err_node;
			}

			written = snprintf(ubpu_key, sizeof(ubpu_key),
					   "%d", u);
			if (written < 0 ||
				(size_t)written >= sizeof(ubpu_key)) {
				sh_json_delete(ubpu_obj);
				ret = -E2BIG;
				goto err_node;
				}

			if (!sh_json_attach_checked(
					node_obj, ubpu_key, ubpu_obj)) {
				sh_json_delete(ubpu_obj);
				ret = -ENOMEM;
				goto err_node;
					}
		}

		if (!sh_json_attach_checked(
				root, node->ip, node_obj)) {
			ret = -ENOMEM;
			goto err_node;
				}

		/* node_obj is now owned by root. */
		continue;

		err_node:
				sh_json_delete(node_obj);
		goto err_root;
	}

	*out = root;
	return 0;

	err_root:
		sh_json_delete(root);
	return ret;
}
/* ========================================================================
 * 公开接口
 * ======================================================================== */
int sub_health_probe_plan(const char *topology_file, uint32_t coverage_k,
			  uint32_t packet_size, const char *output_file)
{
	char *content = NULL;
	sh_json *root = NULL;
	char *json_str = NULL;
	FILE *fp;
	size_t json_len;
	uint32_t packet_count_intra;
	uint32_t packet_count_inter;
	bool is_electric_link;
	int saved_errno;
	int ret;

	if (topology_file == NULL || output_file == NULL)
		return -EINVAL;

	if (coverage_k < MIN_COVERAGE_K ||
		coverage_k > MAX_COVERAGE_K)
		return -EINVAL;

	ret = read_file_content(topology_file, &content);
	if (ret != 0)
		return ret;

	ret = parse_topology(content);
	free(content);
	content = NULL;
	if (ret != 0)
		return ret;

	detect_redundant_l1_switches();

	is_electric_link =
		(g_l1_count == 1) &&
		(strcmp(g_l1s[0].name, ELECTRIC_L1_NAME) == 0);

	calc_packet_counts(coverage_k, g_l2_num, is_electric_link,
			   &packet_count_intra,
			   &packet_count_inter);

	ret = generate_probe_plan_json(coverage_k,
					   packet_count_intra,
					   packet_count_inter,
					   packet_size,
					   &root);
	if (ret != 0)
		return ret;

	json_str = sh_json_write(root);
	sh_json_delete(root);
	root = NULL;
	if (json_str == NULL)
		return -ENOMEM;

	fp = fopen(output_file, "wb");
	if (fp == NULL) {
		saved_errno = errno;
		free(json_str);
		return -saved_errno;
	}

	json_len = strlen(json_str);

	if (fwrite(json_str, 1, json_len, fp) != json_len ||
		fputc('\n', fp) == EOF) {
		saved_errno = errno ? errno : EIO;

		/*
		 * 保留前面真正的写入错误；
		 * 此处关闭失败不覆盖 saved_errno。
		 */
		fclose(fp);
		free(json_str);
		unlink(output_file);
		return -saved_errno;
		}

	if (fclose(fp) != 0) {
		saved_errno = errno ? errno : EIO;
		free(json_str);
		unlink(output_file);
		return -saved_errno;
	}

	free(json_str);
	return 0;
}
