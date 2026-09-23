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
#include "unified_clustering.h"
#include "sh_json.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <float.h>

#define DETECT_LOG_FILE "sub_health_detect.log"
#define MAX_SUPPORTED_UBPU 2

/* 故障域短名称（日志用） */
static const char *const g_domain_short_names[] = {
	"NONE",
	"NODE_UPLINK",
	"L1_UPLINK",
	"L2_DOWNLINK",
	"INTRA_L1_DOWNLINK",
	"INTER_L1_DOWNLINK"
};

/* 故障域完整名称（JSON 输出用） */
static const char *const g_domain_full_names[] = {
	"SUB_HEALTH_NONE",
	"SUB_HEALTH_NODE_UPLINK",
	"SUB_HEALTH_L1_UPLINK",
	"SUB_HEALTH_L2_DOWNLINK",
	"SUB_HEALTH_INTRA_L1_DOWNLINK",
	"SUB_HEALTH_INTER_L1_DOWNLINK"
};

/* ========================================================================
 * 数据结构
 * ======================================================================== */

struct l1_map {
	char name[64];
	int eid_num;
	char eids[MAX_L1_EID_NUM][MAX_EID_LEN];
};

struct ubpu_probe_data {
	bool present;
	char src_eid[MAX_EID_LEN];
	int intra_dst_num;
	char intra_dst_eids[MAX_PROBE_TARGETS][MAX_EID_LEN];
	double intra_latencies[MAX_PROBE_TARGETS];
	int inter_dst_num;
	char inter_dst_eids[MAX_PROBE_TARGETS][MAX_EID_LEN];
	double inter_latencies[MAX_PROBE_TARGETS];
	struct l1_map l1_maps[MAX_L1_SWITCH_NUM];
	int l1_map_count;
};

/* 空间聚类连续性分析结果 */
struct cluster_info {
	bool has_anomaly;
	int anomaly_count;
	bool is_consecutive;
	double mean;
};

/* 单个 UBPU 的检测上下文 */
struct ubpu_ctx {
	char node_ip[64];
	int ubpu_id;
	struct ubpu_probe_data *data;
	double time_threshold;
	bool abnormal_intra[MAX_PROBE_TARGETS];
	bool abnormal_inter[MAX_PROBE_TARGETS];
	bool cluster_abnormal_intra[MAX_PROBE_TARGETS];
	bool cluster_abnormal_inter[MAX_PROBE_TARGETS];
	struct cluster_info intra_cluster;
	struct cluster_info inter_cluster;
	/* inter 聚类结果（故障域判定用，用后释放） */
	uc_cluster_result_t *inter_res;
	enum sub_health_domain domains[MAX_PROBE_TARGETS];
	int domain_count;
	/* 异常链路列表（intra 在前 + inter 在后） */
	char abnormal_eids[MAX_PROBE_TARGETS * 2][MAX_EID_LEN];
	double abnormal_latencies[MAX_PROBE_TARGETS * 2];
	int abnormal_count;
};

/* ========================================================================
 * JSON 解析
 * ======================================================================== */

static int parse_supported_ubpu_id(const char *text, int *ubpu_id)
{
	if (text == NULL || ubpu_id == NULL)
		return -EINVAL;

	if (strcmp(text, "0") == 0) {
		*ubpu_id = 0;
		return 0;
	}

	if (strcmp(text, "1") == 0) {
		*ubpu_id = 1;
		return 0;
	}

	return -EINVAL;
}

static bool is_valid_eid(const char *eid)
{
	struct in6_addr addr;

	if (eid == NULL || eid[0] == '\0' || strlen(eid) >= MAX_EID_LEN)
		return false;

	return inet_pton(AF_INET6, eid, &addr) == 1;
}

static int get_uint32_field(sh_json *obj, const char *key,
			    uint32_t min_value, uint32_t max_value)
{
	sh_json *item;
	double value;

	if (obj == NULL || key == NULL)
		return -EINVAL;

	item = sh_json_get_item_cs(obj, key);
	if (item == NULL || item->kind != SH_JSON_NUM)
		return -EINVAL;

	value = item->dval;
	if (!isfinite(value) || value < (double)min_value ||
	    value > (double)max_value ||
	    fabs(value - (double)(uint32_t)value) > DBL_EPSILON)
		return -EINVAL;

	return 0;
}

static int validate_latency_pair(sh_json *ubpu_obj, const char *dst_key,
				 const char *latency_key, int *sample_count)
{
	sh_json *dst_array;
	sh_json *latency_array;
	int dst_count;
	int latency_count;
	int i;

	if (ubpu_obj == NULL || dst_key == NULL || latency_key == NULL ||
	    sample_count == NULL)
		return -EINVAL;

	dst_array = sh_json_get_item_cs(ubpu_obj, dst_key);
	latency_array = sh_json_get_item_cs(ubpu_obj, latency_key);
	if (dst_array == NULL || dst_array->kind != SH_JSON_SEQ ||
	    latency_array == NULL || latency_array->kind != SH_JSON_SEQ)
		return -EINVAL;

	dst_count = sh_json_item_count(dst_array);
	latency_count = sh_json_item_count(latency_array);
	if (dst_count < 0 || dst_count > MAX_PROBE_TARGETS ||
	    dst_count != latency_count)
		return -EINVAL;

	for (i = 0; i < dst_count; i++) {
		sh_json *dst = sh_json_item_at(dst_array, i);
		sh_json *latency = sh_json_item_at(latency_array, i);

		if (dst == NULL || dst->kind != SH_JSON_BUF ||
		    !is_valid_eid(dst->str) || latency == NULL ||
		    latency->kind != SH_JSON_NUM ||
		    !isfinite(latency->dval) || latency->dval <= 0.0)
			return -EINVAL;
	}

	*sample_count = dst_count;
	return 0;
}

static int validate_l1_maps(sh_json *root)
{
	sh_json *maps;
	sh_json *map;
	int map_count = 0;

	maps = sh_json_get_item_cs(root, "_l1_maps");
	if (maps == NULL || maps->kind != SH_JSON_MAP)
		return -EINVAL;

	for (map = maps->head; map != NULL; map = map->fwd) {
		sh_json *eid;
		int eid_count;

		if (map_count >= MAX_L1_SWITCH_NUM || map->name == NULL ||
		    map->name[0] == '\0' || strlen(map->name) >= 64 ||
		    map->kind != SH_JSON_SEQ)
			return -EINVAL;

		eid_count = sh_json_item_count(map);
		if (eid_count <= 0 || eid_count > MAX_L1_EID_NUM)
			return -EINVAL;

		for (eid = map->head; eid != NULL; eid = eid->fwd) {
			if (eid->kind != SH_JSON_BUF ||
			    !is_valid_eid(eid->str))
				return -EINVAL;
		}

		map_count++;
	}

	return map_count > 0 ? 0 : -EINVAL;
}

static int validate_result_ubpu(sh_json *ubpu_obj, int *sample_count)
{
	sh_json *src_eid;
	int intra_count;
	int inter_count;

	if (ubpu_obj == NULL || ubpu_obj->kind != SH_JSON_MAP ||
	    sample_count == NULL)
		return -EINVAL;

	src_eid = sh_json_get_item_cs(ubpu_obj, "src_eid");
	if (src_eid == NULL || src_eid->kind != SH_JSON_BUF ||
	    !is_valid_eid(src_eid->str))
		return -EINVAL;

	if (get_uint32_field(ubpu_obj, "packet_count_intra", 1, 4096) != 0 ||
	    get_uint32_field(ubpu_obj, "packet_count_inter", 0, 4096) != 0 ||
	    get_uint32_field(ubpu_obj, "packet_size", 4,
			     DEFAULT_PACKET_SIZE) != 0 ||
	    validate_latency_pair(ubpu_obj, "intra_l1_dst_eids",
				  "intra_l1_latencies", &intra_count) != 0 ||
	    validate_latency_pair(ubpu_obj, "inter_l1_dst_eids",
				  "inter_l1_latencies", &inter_count) != 0)
		return -EINVAL;

	*sample_count = intra_count + inter_count;
	return 0;
}

static int validate_result_node(sh_json *node_obj, int *sample_count)
{
	bool seen[MAX_SUPPORTED_UBPU] = { false };
	sh_json *ubpu;
	int ubpu_count = 0;

	if (node_obj == NULL || node_obj->kind != SH_JSON_MAP ||
	    sample_count == NULL)
		return -EINVAL;

	for (ubpu = node_obj->head; ubpu != NULL; ubpu = ubpu->fwd) {
		int ubpu_samples;
		int ubpu_id;

		if (parse_supported_ubpu_id(ubpu->name, &ubpu_id) != 0 ||
		    seen[ubpu_id] ||
		    validate_result_ubpu(ubpu, &ubpu_samples) != 0)
			return -EINVAL;

		seen[ubpu_id] = true;
		*sample_count += ubpu_samples;
		ubpu_count++;
	}

	return ubpu_count > 0 ? 0 : -EINVAL;
}

static int validate_probe_result(sh_json *root, int *sample_count)
{
	struct in_addr addr;
	sh_json *top;
	int maps_count = 0;
	int node_count = 0;

	if (sample_count == NULL ||
	    validate_l1_maps(root) != 0)
		return -EINVAL;

	*sample_count = 0;
	for (top = root->head; top != NULL; top = top->fwd) {
		if (top->name == NULL)
			return -EINVAL;

		if (strcmp(top->name, "_l1_maps") == 0) {
			maps_count++;
			continue;
		}

		if (node_count >= MAX_NODE_NUM || top->kind != SH_JSON_MAP ||
		    inet_pton(AF_INET, top->name, &addr) != 1 ||
		    validate_result_node(top, sample_count) != 0)
			return -EINVAL;

		node_count++;
	}

	if (maps_count != 1 || node_count == 0)
		return -EINVAL;

	return 0;
}

static int parse_latency_list(sh_json *obj, const char *key, double *latencies,
			      int max_num)
{
	sh_json *item;
	int count = 0;

	item = sh_json_get_item(obj, key);
	if (!item || item->kind != SH_JSON_SEQ)
		return 0;

	for (sh_json *e = item->head; e && count < max_num; e = e->fwd) {
		if (e->kind != SH_JSON_NUM)
			continue;
		latencies[count] = e->dval;
		count++;
	}
	return count;
}

static int parse_dst_list(sh_json *obj, const char *key, const char *alias,
			  char (*eids)[MAX_EID_LEN], int max_num)
{
	sh_json *item;
	int count = 0;

	item = sh_json_get_item(obj, key);
	if (!item && alias)
		item = sh_json_get_item(obj, alias);
	if (!item || item->kind != SH_JSON_SEQ)
		return 0;

	for (sh_json *e = item->head; e && count < max_num; e = e->fwd) {
		if (e->kind != SH_JSON_BUF)
			continue;
		snprintf(eids[count], MAX_EID_LEN, "%s", e->str);
		count++;
	}
	return count;
}

static void parse_ubpu_data(sh_json *ubpu_obj, struct ubpu_probe_data *ubpu)
{
	sh_json *item;

	ubpu->present = true;

	item = sh_json_get_item(ubpu_obj, "src_eid");
	if (!item)
		item = sh_json_get_item(ubpu_obj, "src_port_eid");
	if (item && item->kind == SH_JSON_BUF)
		snprintf(ubpu->src_eid, MAX_EID_LEN, "%s", item->str);

	{
		int n_dst;
		int n_lat;

		/* dst_eids 与 latencies 等长、一一对应，取两者较小值 */
		n_dst = parse_dst_list(ubpu_obj, "intra_l1_dst_eids",
				       "intra_l1_dst_port_eids",
				       ubpu->intra_dst_eids,
				       MAX_PROBE_TARGETS);
		n_lat = parse_latency_list(ubpu_obj, "intra_l1_latencies",
					   ubpu->intra_latencies,
					   MAX_PROBE_TARGETS);
		ubpu->intra_dst_num = (n_dst < n_lat) ? n_dst : n_lat;

		n_dst = parse_dst_list(ubpu_obj, "inter_l1_dst_eids",
				       "inter_l1_dst_port_eids",
				       ubpu->inter_dst_eids,
				       MAX_PROBE_TARGETS);
		n_lat = parse_latency_list(ubpu_obj, "inter_l1_latencies",
					   ubpu->inter_latencies,
					   MAX_PROBE_TARGETS);
		ubpu->inter_dst_num = (n_dst < n_lat) ? n_dst : n_lat;
	}
}

/* 解析单个节点下的全部 UBPU 数据（含 L1 EID 映射表） */
static void parse_node_data(sh_json *node_obj, struct ubpu_probe_data *ubpus,
			    const struct l1_map *global_maps,
			    int global_map_count)
{
	int i;

	memset(ubpus, 0, MAX_SUPPORTED_UBPU * sizeof(*ubpus));

	for (sh_json *child = node_obj->head; child; child = child->fwd) {
		int ubpu_id;

		if (!child->name)
			continue;

		/* 输入已校验，这里仍使用严格解析，避免非数字 key 落到 UBPU 0。 */
		if (parse_supported_ubpu_id(child->name, &ubpu_id) != 0)
			continue;

		parse_ubpu_data(child, &ubpus[ubpu_id]);

		/* 将全局 L1 映射表复制到该 UBPU */
		if (ubpus[ubpu_id].l1_map_count == 0 && global_maps) {
			for (i = 0; i < global_map_count; i++) {
				if (ubpus[ubpu_id].l1_map_count >= MAX_L1_SWITCH_NUM)
					break;
				memcpy(&ubpus[ubpu_id].l1_maps[ubpus[ubpu_id].l1_map_count],
				       &global_maps[i], sizeof(struct l1_map));
				ubpus[ubpu_id].l1_map_count++;
			}
		}
	}
}

/* ========================================================================
 * 聚类结果连续性分析
 * ======================================================================== */

static int cmp_int_asc(const void *a, const void *b)
{
	int ia = *(const int *)a;
	int ib = *(const int *)b;

	return (ia > ib) - (ia < ib);
}

static void space_clustering_analyze(const uc_cluster_result_t *res,
				     struct cluster_info *info)
{
	int *sorted_indices = NULL;
	int i;

	memset(info, 0, sizeof(*info));

	if (!res)
		return;

	info->has_anomaly = true;
	info->anomaly_count = res->count;
	info->mean = res->mean;

	/* SPEC §5.5: 将异常 indices[] 排序后检查是否连续 */
	sorted_indices = (int *)malloc((size_t)res->count * sizeof(int));
	if (!sorted_indices)
		return;

	memcpy(sorted_indices, res->indices, (size_t)res->count * sizeof(int));
	qsort(sorted_indices, (size_t)res->count, sizeof(int), cmp_int_asc);

	info->is_consecutive = true;
	for (i = 1; i < res->count; i++) {
		if (sorted_indices[i] != sorted_indices[i - 1] + 1) {
			info->is_consecutive = false;
			break;
		}
	}

	free(sorted_indices);
}

/* ========================================================================
 * 中位数计算（升序排序后取 sorted[count/2]）
 * ======================================================================== */

static int cmp_double_asc(const void *a, const void *b)
{
	double da = *(const double *)a;
	double db = *(const double *)b;

	return (da > db) - (da < db);
}

static double compute_median(const double *vals, int count)
{
	double *copy;
	double result;

	if (count <= 0)
		return 0.0;

	copy = (double *)malloc((size_t)count * sizeof(double));
	if (!copy)
		return 0.0;

	memcpy(copy, vals, (size_t)count * sizeof(double));
	qsort(copy, (size_t)count, sizeof(double), cmp_double_asc);
	result = copy[count / 2];
	free(copy);
	return result;
}

/* ========================================================================
 * L1_UPLINK 中位数比较（剔除聚类异常后的正常数据）
 * ======================================================================== */

static bool second_round_clustering(double median_intra, double median_inter)
{
	if (median_intra <= 0.0 && median_inter <= 0.0)
		return false;

	if (median_intra <= 0.0)
		return median_inter > 0.0;

	return median_inter > median_intra * L1_UPLINK_FACTOR;
}

static bool check_l1_uplink(const struct ubpu_ctx *ctx)
{
	double normal_intra[MAX_PROBE_TARGETS];
	double normal_inter[MAX_PROBE_TARGETS];
	int normal_intra_count = 0;
	int normal_inter_count = 0;
	double median_intra, median_inter;
	int i;

	for (i = 0; i < ctx->data->intra_dst_num; i++) {
		if (!ctx->cluster_abnormal_intra[i])
			normal_intra[normal_intra_count++] = ctx->data->intra_latencies[i];
	}
	for (i = 0; i < ctx->data->inter_dst_num; i++) {
		if (!ctx->cluster_abnormal_inter[i])
			normal_inter[normal_inter_count++] = ctx->data->inter_latencies[i];
	}

	median_intra = compute_median(normal_intra, normal_intra_count);
	median_inter = compute_median(normal_inter, normal_inter_count);

	return second_round_clustering(median_intra, median_inter);
}

/* ========================================================================
 * L2_DOWNLINK 检查：3 个连续异常 EID 是否同属同一 L1
 * ======================================================================== */

static int find_eid_l1(const struct ubpu_probe_data *ubpu, const char *eid)
{
	int i, j;

	for (i = 0; i < ubpu->l1_map_count; i++) {
		for (j = 0; j < ubpu->l1_maps[i].eid_num; j++) {
			if (strcmp(ubpu->l1_maps[i].eids[j], eid) == 0)
				return i;
		}
	}
	return -1;
}

static bool check_l2_downlink(const struct ubpu_ctx *ctx)
{
	const uc_cluster_result_t *res = ctx->inter_res;
	int *sorted_indices = NULL;
	int count;
	int i;

	/* 前置条件：inter 聚类异常数量 >= 3 */
	if (!res || res->count < MAX_CONSECUTIVE_WINDOW)
		return false;

	count = res->count;

	/* 无 L1 映射（旧格式 JSON）→ 降级为旧行为 */
	if (ctx->data->l1_map_count == 0)
		return true;

	/* SPEC §5.5: 将异常 indices[] 排序后检查连续性和滑动窗口 */
	sorted_indices = (int *)malloc((size_t)count * sizeof(int));
	if (!sorted_indices)
		return false;
	memcpy(sorted_indices, res->indices, (size_t)count * sizeof(int));
	qsort(sorted_indices, (size_t)count, sizeof(int), cmp_int_asc);

	for (i = 0; i <= count - MAX_CONSECUTIVE_WINDOW; i++) {
		int idx0 = sorted_indices[i];
		int idx1 = sorted_indices[i + 1];
		int idx2 = sorted_indices[i + 2];
		int l1_0, l1_1, l1_2;

		/* 要求索引连续（idx1-idx0==1 && idx2-idx1==1） */
		if (idx1 != idx0 + 1 || idx2 != idx1 + 1)
			continue;

		/* 通过 L1 映射表查询这 3 个异常 EID 所属 L1 */
		l1_0 = find_eid_l1(ctx->data, ctx->data->inter_dst_eids[idx0]);
		l1_1 = find_eid_l1(ctx->data, ctx->data->inter_dst_eids[idx1]);
		l1_2 = find_eid_l1(ctx->data, ctx->data->inter_dst_eids[idx2]);

		if (l1_0 >= 0 && l1_0 == l1_1 && l1_1 == l1_2) {
			free(sorted_indices);
			return true;
		}
	}

	free(sorted_indices);
	return false;
}

/* ========================================================================
 * 故障域判定（决策树，每条件独立触发）
 * ======================================================================== */

static void collect_fault_domains(struct ubpu_ctx *ctx)
{
	bool has_intra = ctx->data->intra_dst_num > 0;
	bool has_inter = ctx->data->inter_dst_num > 0;
	bool all_intra_abnormal = true;
	bool all_inter_abnormal = true;
	bool node_uplink = false;
	int i;

	/* 1. NODE_UPLINK */
	if (has_intra) {
		for (i = 0; i < ctx->data->intra_dst_num; i++) {
			if (!ctx->abnormal_intra[i])
				all_intra_abnormal = false;
		}
	}
	if (has_inter) {
		for (i = 0; i < ctx->data->inter_dst_num; i++) {
			if (!ctx->abnormal_inter[i])
				all_inter_abnormal = false;
		}
	}

	if (has_intra && has_inter)
		node_uplink = all_intra_abnormal && all_inter_abnormal;
	else if (has_intra)
		node_uplink = all_intra_abnormal;
	else if (has_inter)
		node_uplink = all_inter_abnormal;

	if (node_uplink)
		ctx->domains[ctx->domain_count++] = SUB_HEALTH_NODE_UPLINK;

	/* 2. L1_UPLINK（剔除聚类异常后正常数据中位数比较） */
	if (check_l1_uplink(ctx))
		ctx->domains[ctx->domain_count++] = SUB_HEALTH_L1_UPLINK;

	/* 3. L2_DOWNLINK vs INTER_L1_DOWNLINK（连续且 >= 3 个） */
	if (ctx->inter_cluster.has_anomaly && ctx->inter_cluster.is_consecutive &&
	    ctx->inter_cluster.anomaly_count >= MAX_CONSECUTIVE_WINDOW) {
		if (check_l2_downlink(ctx))
			ctx->domains[ctx->domain_count++] = SUB_HEALTH_L2_DOWNLINK;
		else
			ctx->domains[ctx->domain_count++] = SUB_HEALTH_INTER_L1_DOWNLINK;
	}

	/* 4. INTER_L1_DOWNLINK（非连续异常） */
	if (ctx->inter_cluster.has_anomaly && !ctx->inter_cluster.is_consecutive)
		ctx->domains[ctx->domain_count++] = SUB_HEALTH_INTER_L1_DOWNLINK;

	/* 5. INTRA_L1_DOWNLINK（域内聚类异常或唯一探测对超阈值） */
	if (ctx->intra_cluster.has_anomaly)
		ctx->domains[ctx->domain_count++] = SUB_HEALTH_INTRA_L1_DOWNLINK;
	else if (ctx->data->intra_dst_num == 1 &&
		 ctx->data->intra_latencies[0] > ctx->time_threshold)
		ctx->domains[ctx->domain_count++] = SUB_HEALTH_INTRA_L1_DOWNLINK;

	/* 6. INTER_L1_DOWNLINK（连续但不足 3 个） */
	if (ctx->inter_cluster.has_anomaly && ctx->inter_cluster.is_consecutive &&
	    ctx->inter_cluster.anomaly_count < MAX_CONSECUTIVE_WINDOW)
		ctx->domains[ctx->domain_count++] = SUB_HEALTH_INTER_L1_DOWNLINK;
}

/* ========================================================================
 * 构建异常链路列表（intra 在前 + inter 在后）
 * ======================================================================== */

static void build_abnormal_list(struct ubpu_ctx *ctx)
{
	int i;

	ctx->abnormal_count = 0;

	for (i = 0; i < ctx->data->intra_dst_num; i++) {
		if (!ctx->abnormal_intra[i])
			continue;
		snprintf(ctx->abnormal_eids[ctx->abnormal_count],
			 MAX_EID_LEN, "%s", ctx->data->intra_dst_eids[i]);
		ctx->abnormal_latencies[ctx->abnormal_count] =
			ctx->data->intra_latencies[i];
		ctx->abnormal_count++;
	}

	for (i = 0; i < ctx->data->inter_dst_num; i++) {
		if (!ctx->abnormal_inter[i])
			continue;
		snprintf(ctx->abnormal_eids[ctx->abnormal_count],
			 MAX_EID_LEN, "%s", ctx->data->inter_dst_eids[i]);
		ctx->abnormal_latencies[ctx->abnormal_count] =
			ctx->data->inter_latencies[i];
		ctx->abnormal_count++;
	}
}

/* ========================================================================
 * 单个 UBPU 的完整检测流程
 * ======================================================================== */

static void detect_ubpu(struct ubpu_ctx *ctx)
{
	uc_cluster_result_t *intra_res;
	uc_cluster_result_t *inter_res;
	int i;

	/* 1. 时延阈值标记 */
	for (i = 0; i < ctx->data->intra_dst_num; i++)
		ctx->abnormal_intra[i] =
			ctx->data->intra_latencies[i] > ctx->time_threshold;
	for (i = 0; i < ctx->data->inter_dst_num; i++)
		ctx->abnormal_inter[i] =
			ctx->data->inter_latencies[i] > ctx->time_threshold;

	/* 2. 统一聚类检测（intra / inter 分别运行） */
	intra_res = uc_unified_clustering(ctx->data->intra_latencies, NULL,
					  ctx->data->intra_dst_num);
	inter_res = uc_unified_clustering(ctx->data->inter_latencies, NULL,
					  ctx->data->inter_dst_num);

	if (intra_res) {
		for (i = 0; i < intra_res->count; i++) {
			int idx = intra_res->indices[i];

			if (idx >= 0 && idx < MAX_PROBE_TARGETS) {
				ctx->cluster_abnormal_intra[idx] = true;
				ctx->abnormal_intra[idx] = true;
			}
		}
	}
	if (inter_res) {
		for (i = 0; i < inter_res->count; i++) {
			int idx = inter_res->indices[i];

			if (idx >= 0 && idx < MAX_PROBE_TARGETS) {
				ctx->cluster_abnormal_inter[idx] = true;
				ctx->abnormal_inter[idx] = true;
			}
		}
	}

	/* 3. 连续性分析 */
	space_clustering_analyze(intra_res, &ctx->intra_cluster);
	space_clustering_analyze(inter_res, &ctx->inter_cluster);

	/* 4. 故障域判定（需要 inter 聚类结果做 L2_DOWNLINK 检查） */
	ctx->inter_res = inter_res;
	collect_fault_domains(ctx);

	/* 5. 构建异常链路列表 */
	build_abnormal_list(ctx);

	uc_free_result(intra_res);
	uc_free_result(inter_res);
}

/* ========================================================================
 * 诊断日志输出
 * ======================================================================== */

static void write_detect_log(FILE *log_fp, const struct ubpu_ctx *ctx)
{
	int i;

	fprintf(log_fp, "====== UBPU %s:%d 检测结果 ======\n\n",
		ctx->node_ip, ctx->ubpu_id);

	fprintf(log_fp, "【检测流程】\n");
	fprintf(log_fp, "1. 时间阈值检测 (%.0fms)\n", ctx->time_threshold);

	fprintf(log_fp, "   - L1 内数据：");
	for (i = 0; i < ctx->data->intra_dst_num; i++)
		fprintf(log_fp, "%.3f ", ctx->data->intra_latencies[i]);
	if (ctx->data->intra_dst_num > 0) {
		bool all_above = true;

		for (i = 0; i < ctx->data->intra_dst_num; i++) {
			if (ctx->data->intra_latencies[i] <= ctx->time_threshold) {
				all_above = false;
				break;
			}
		}
		fprintf(log_fp, "→ %s\n", all_above ? "全部 > 阈值" : "全部 ≤ 阈值");
	} else {
		fprintf(log_fp, "→ 无数据\n");
	}

	fprintf(log_fp, "   - 跨 L1 数据：");
	for (i = 0; i < ctx->data->inter_dst_num; i++)
		fprintf(log_fp, "%.3f ", ctx->data->inter_latencies[i]);
	if (ctx->data->inter_dst_num > 0) {
		bool all_above = true;

		for (i = 0; i < ctx->data->inter_dst_num; i++) {
			if (ctx->data->inter_latencies[i] <= ctx->time_threshold) {
				all_above = false;
				break;
			}
		}
		fprintf(log_fp, "→ %s\n", all_above ? "全部 > 阈值" : "全部 ≤ 阈值");
	} else {
		fprintf(log_fp, "→ 无数据\n");
	}

	fprintf(log_fp, "\n2. 空间聚类检测\n");
	if (ctx->intra_cluster.has_anomaly) {
		fprintf(log_fp, "   - L1 内数据：聚类检测：检测到异常(数量=%d, 异常值=[",
			ctx->intra_cluster.anomaly_count);
		for (i = 0; i < ctx->data->intra_dst_num; i++) {
			if (ctx->cluster_abnormal_intra[i])
				fprintf(log_fp, "%.3f ", ctx->data->intra_latencies[i]);
		}
		fprintf(log_fp, "])\n");
	} else {
		fprintf(log_fp, "   - L1 内数据：聚类检测：无异常\n");
	}

	if (ctx->inter_cluster.has_anomaly) {
		fprintf(log_fp,
			"   - 跨 L1 数据：聚类检测：检测到异常(数量=%d, 连续=%s, 异常值=[",
			ctx->inter_cluster.anomaly_count,
			ctx->inter_cluster.is_consecutive ? "是" : "否");
		for (i = 0; i < ctx->data->inter_dst_num; i++) {
			if (ctx->cluster_abnormal_inter[i])
				fprintf(log_fp, "%.3f ", ctx->data->inter_latencies[i]);
		}
		fprintf(log_fp, "])\n");
	} else {
		fprintf(log_fp, "   - 跨 L1 数据：聚类检测：无异常\n");
	}

	fprintf(log_fp, "\n3. 第二轮聚类（正常数据中位数比较）\n");
	{
		double normal_intra[MAX_PROBE_TARGETS];
		double normal_inter[MAX_PROBE_TARGETS];
		int n_intra = 0, n_inter = 0;
		double median_intra, median_inter;

		for (i = 0; i < ctx->data->intra_dst_num; i++) {
			if (!ctx->cluster_abnormal_intra[i])
				normal_intra[n_intra++] = ctx->data->intra_latencies[i];
		}
		for (i = 0; i < ctx->data->inter_dst_num; i++) {
			if (!ctx->cluster_abnormal_inter[i])
				normal_inter[n_inter++] = ctx->data->inter_latencies[i];
		}
		median_intra = compute_median(normal_intra, n_intra);
		median_inter = compute_median(normal_inter, n_inter);

		fprintf(log_fp, "   - L1 内正常中位数 = %.3f\n", median_intra);
		fprintf(log_fp, "   - 跨 L1 正常中位数 = %.3f\n", median_inter);
		fprintf(log_fp, "   - %.3f > %.3f × %.1f = %.3f → %s\n",
			median_inter, median_intra, L1_UPLINK_FACTOR,
			median_intra * L1_UPLINK_FACTOR,
			second_round_clustering(median_intra, median_inter) ?
				"有显著差异 → L1_UPLINK" : "无显著差异");
	}

	fprintf(log_fp, "\n【故障域判定】\n");
	if (ctx->domain_count == 0) {
		fprintf(log_fp, "  - 无故障域\n");
	} else {
		for (i = 0; i < ctx->domain_count; i++)
			fprintf(log_fp, "  - %s\n",
				g_domain_short_names[ctx->domains[i]]);
	}

	fprintf(log_fp, "\n  异常链路：\n");
	if (ctx->abnormal_count == 0) {
		fprintf(log_fp, "    无\n");
	} else {
		for (i = 0; i < ctx->abnormal_count; i++) {
			fprintf(log_fp, "    %s → %s (%.3f ms)\n",
				ctx->data->src_eid, ctx->abnormal_eids[i],
				ctx->abnormal_latencies[i]);
		}
	}
	fprintf(log_fp, "\n");
}

/* ========================================================================
 * JSON 检测结果输出
 * ======================================================================== */

static void add_ubpu_result_json(sh_json *ubpu_obj, const struct ubpu_ctx *ctx)
{
	sh_json *eids_arr = sh_json_create_arr();
	sh_json *lats_arr = sh_json_create_arr();
	sh_json *domain_arr = sh_json_create_arr();
	int i;

	sh_json_put_str(ubpu_obj, "src_eid", ctx->data->src_eid);

	for (i = 0; i < ctx->abnormal_count; i++) {
		sh_json_push(eids_arr,
				     sh_json_create_str(ctx->abnormal_eids[i]));
		sh_json_push(lats_arr,
				     sh_json_create_num(ctx->abnormal_latencies[i]));
	}
	for (i = 0; i < ctx->domain_count; i++) {
		sh_json_push(domain_arr,
				     sh_json_create_str(g_domain_full_names[ctx->domains[i]]));
	}

	sh_json_attach(ubpu_obj, "sub_health_dst_eids", eids_arr);
	sh_json_attach(ubpu_obj, "sub_health_latencies", lats_arr);
	sh_json_attach(ubpu_obj, "sub_health_domain", domain_arr);
}

/* ========================================================================
 * 公开接口
 * ======================================================================== */

int sub_health_detect(const char *probe_result_file, const char *output_file,
		      uint32_t time_threshold)
{
	struct l1_map *global_maps = NULL;
	sh_json *result_root = NULL;
	char *json_str = NULL;
	char *content = NULL;
	sh_json *root = NULL;
	FILE *log_fp = NULL;
	FILE *output_fp = NULL;
	int global_map_count = 0;
	int valid_sample_count = 0;
	int ret = 0;

	if (!probe_result_file)
		return -EINVAL;

	/* 读取探测结果文件 */
	{
		FILE *fp;
		long fsize;
		size_t nread;

		fp = fopen(probe_result_file, "rb");
		if (!fp)
			return -errno;

		fseek(fp, 0, SEEK_END);
		fsize = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		if (fsize <= 0 || fsize > MAX_JSON_BUF_SIZE) {
			fclose(fp);
			return -EINVAL;
		}

		content = (char *)malloc((size_t)fsize + 1);
		if (!content) {
			fclose(fp);
			return -ENOMEM;
		}

		nread = fread(content, 1, (size_t)fsize, fp);
		fclose(fp);
		if (nread != (size_t)fsize) {
			free(content);
			return -EIO;
		}
		content[fsize] = '\0';
	}

	root = sh_json_parse(content);
	free(content);
	content = NULL;
	if (!root) {
		fprintf(stderr,
			"[ERROR] Invalid probe result file '%s': invalid JSON.\n",
			probe_result_file);
		return -EINVAL;
	}

	ret = validate_probe_result(root, &valid_sample_count);
	if (ret != 0) {
		fprintf(stderr,
			"[ERROR] Invalid probe result file '%s': expected a "
			"Step 2 output with file_type='%s', schema_version=%d, "
			"valid UBPU fields and paired EID/latency arrays.\n",
			probe_result_file, SUB_HEALTH_FILE_PROBE_RESULT,
			SUB_HEALTH_SCHEMA_VERSION);
		sh_json_delete(root);
		return ret;
	}

	if (valid_sample_count == 0)
		fprintf(stderr,
			"[WARN] Probe result file '%s' contains no valid latency "
			"samples.\n", probe_result_file);

	/* 诊断日志：每次运行清空重写 */
	log_fp = fopen(DETECT_LOG_FILE, "w");
	if (!log_fp) {
		ret = -errno;
		goto out;
	}

	/* 从 JSON 顶层解析全局 L1→EID 映射表（_l1_maps） */
	global_maps = (struct l1_map *)calloc((size_t)MAX_L1_SWITCH_NUM,
					      sizeof(struct l1_map));
	if (!global_maps) {
		ret = -ENOMEM;
		goto out;
	}

	{
		sh_json *l1_maps_obj = sh_json_get_item(root, "_l1_maps");

		if (l1_maps_obj && l1_maps_obj->kind == SH_JSON_MAP) {
			for (sh_json *map = l1_maps_obj->head;
			     map && global_map_count < MAX_L1_SWITCH_NUM;
			     map = map->fwd) {
				int eid_count = 0;

				if (!map->name || map->kind != SH_JSON_SEQ)
					continue;
				snprintf(global_maps[global_map_count].name,
					 sizeof(global_maps[global_map_count].name),
					 "%s", map->name);
				for (sh_json *e = map->head;
				     e && eid_count < MAX_L1_EID_NUM;
				     e = e->fwd) {
					if (e->kind != SH_JSON_BUF)
						continue;
					snprintf(global_maps[global_map_count].eids[eid_count],
						 MAX_EID_LEN, "%s", e->str);
					eid_count++;
				}
				global_maps[global_map_count].eid_num = eid_count;
				global_map_count++;
			}
		}
	}

	/* 如需结构化输出，先创建结果根节点，再与日志共用同一轮检测。 */
	if (output_file && strlen(output_file) > 0) {
		result_root = sh_json_create_obj();
		if (!result_root) {
			ret = -ENOMEM;
			goto out;
		}
	}

	/* 每个 UBPU 只检测一次，检测结果同时用于日志和可选 JSON 输出。 */
	for (sh_json *node = root->head; node; node = node->fwd) {
		struct ubpu_probe_data *ubpus = NULL;
		int u;

		if (!node->name || node->kind != SH_JSON_MAP)
			continue;
		if (strcmp(node->name, SUB_HEALTH_META_KEY) == 0 ||
		    strcmp(node->name, "_l1_maps") == 0)
			continue;

		/* 堆分配以避免超大栈帧（ubpu_probe_data 含 l1_maps[256] → ~4MB/个） */
		ubpus = (struct ubpu_probe_data *)calloc(
			(size_t)MAX_SUPPORTED_UBPU, sizeof(struct ubpu_probe_data));
		if (!ubpus)
			continue;

		parse_node_data(node, ubpus, global_maps, global_map_count);

		for (u = 0; u < MAX_SUPPORTED_UBPU; u++) {
			struct ubpu_ctx *ctx;
			sh_json *node_obj;
			sh_json *ubpu_obj;
			char ubpu_key[16];

			if (!ubpus[u].present)
				continue;

			ctx = (struct ubpu_ctx *)calloc(1, sizeof(struct ubpu_ctx));
			if (!ctx)
				continue;

			snprintf(ctx->node_ip, sizeof(ctx->node_ip), "%s", node->name);
			ctx->ubpu_id = u;
			ctx->data = &ubpus[u];
			ctx->time_threshold = (double)time_threshold;

			detect_ubpu(ctx);
			write_detect_log(log_fp, ctx);

			if (!result_root ||
			    (ctx->abnormal_count == 0 && ctx->domain_count == 0)) {
				free(ctx);
				continue;
			}

			node_obj = sh_json_get_item(result_root, node->name);
			if (!node_obj) {
				node_obj = sh_json_create_obj();
				if (!node_obj) {
					free(ctx);
					continue;
				}
				sh_json_attach(result_root, node->name, node_obj);
			}

			ubpu_obj = sh_json_create_obj();
			if (!ubpu_obj) {
				free(ctx);
				continue;
			}

			add_ubpu_result_json(ubpu_obj, ctx);
			snprintf(ubpu_key, sizeof(ubpu_key), "%d", u);
			sh_json_attach(node_obj, ubpu_key, ubpu_obj);
			free(ctx);
		}

		free(ubpus);
	}

	fclose(log_fp);
	log_fp = NULL;

	if (result_root) {
		size_t json_len;

		json_str = sh_json_write(result_root);
		if (!json_str) {
			ret = -ENOMEM;
			goto out;
		}

		output_fp = fopen(output_file, "w");
		if (!output_fp) {
			ret = -errno;
			goto out;
		}

		json_len = strlen(json_str);
		if (fwrite(json_str, 1, json_len, output_fp) != json_len) {
			fclose(output_fp);
			output_fp = NULL;
			unlink(output_file);
			ret = -EIO;
			goto out;
		}

		fputc('\n', output_fp);
		fclose(output_fp);
		output_fp = NULL;
	}

out:
	if (output_fp)
		fclose(output_fp);
	if (log_fp)
		fclose(log_fp);
	free(json_str);
	sh_json_delete(result_root);
	free(global_maps);
	sh_json_delete(root);
	return ret;
}
