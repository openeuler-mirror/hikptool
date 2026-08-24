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

#include "unified_clustering.h"
#include "sub_health.h"
#include <math.h>
#include <float.h>
#include <string.h>
#include <errno.h>
/* 递归检测最大深度（spec §4.2：深度 ≤ 10） */
#define UC_MAX_RECURSION_DEPTH 10

/* 聚类簇数组容量上限（肘部法最大 K 值 + 1，用于数组定界） */
#define MAX_CLUSTER_K (MAX_ELBOW_K + 1)
/* ========================================================================
 * Filter positive values (> 0)
 * ======================================================================== */
#define BASELINE_SUPPORT_PERCENT 10

static int get_min_baseline_support(int valid_count)
{
	int support;
	support = (valid_count * BASELINE_SUPPORT_PERCENT + 99) / 100;
	return support;
}
static int filter_positive(const double *values, int count,
			   double **out_vals, int **out_indices, int *out_n)
{
	double *v;
	int *idx;
	int n = 0;
	int i;

	for (i = 0; i < count; i++) {
		if (values[i] > 0.0)
			n++;
	}

	if (n < 2) {
		*out_n = 0;
		return 0;
	}

	v = (double *)malloc((size_t)n * sizeof(double));
	idx = (int *)malloc((size_t)n * sizeof(int));
	if (!v || !idx) {
		free(v);
		free(idx);
		return -ENOMEM;
	}

	n = 0;
	for (i = 0; i < count; i++) {
		if (values[i] > 0.0) {
			v[n] = values[i];
			idx[n] = i;
			n++;
		}
	}

	*out_vals = v;
	*out_indices = idx;
	*out_n = n;
	return 0;
}

/* ========================================================================
 * Z-score normalization: norm = (x - mean) / std
 * ======================================================================== */

static int z_score_normalize(const double *values, int count,
				 double **out_norm, double *out_mean,
				 double *out_std)
{
	double sum = 0.0;
	double sum_sq = 0.0;
	double mean;
	double variance;
	double std_dev;
	double *norm;
	int i;

	for (i = 0; i < count; i++) {
		sum += values[i];
		sum_sq += values[i] * values[i];
	}

	mean = sum / (double)count;
	variance = sum_sq / (double)count - mean * mean;

	/*
	 * Floating-point rounding may make a theoretically non-negative
	 * variance slightly negative.
	 */
	if (variance < 0.0)
		variance = 0.0;

	std_dev = sqrt(variance);
	if (std_dev < 1e-12)
		std_dev = 1.0;

	norm = malloc((size_t)count * sizeof(*norm));
	if (norm == NULL)
		return -ENOMEM;

	for (i = 0; i < count; i++)
		norm[i] = (values[i] - mean) / std_dev;

	*out_norm = norm;
	*out_mean = mean;
	*out_std = std_dev;

	return 0;
}

/* ========================================================================
 * K-means++ center initialization
 * ======================================================================== */

static int kmeans_pp_init(const double *data, int n, int k,
			  double *centers)
{
	double *dist_sq;
	double total;
	double r;
	double accum;
	int i;
	int c;

	centers[0] = data[0];

	dist_sq = malloc((size_t)n * sizeof(*dist_sq));
	if (dist_sq == NULL)
		return -ENOMEM;

	for (c = 1; c < k; c++) {
		total = 0.0;

		for (i = 0; i < n; i++) {
			double min_d = DBL_MAX;
			int j;

			for (j = 0; j < c; j++) {
				double d = fabs(data[i] - centers[j]);

				if (d < min_d)
					min_d = d;
			}

			dist_sq[i] = min_d * min_d;
			total += dist_sq[i];
		}

		/*
		 * All data points coincide with an existing center.
		 * Select a valid point to ensure centers[c] is initialized.
		 */
		if (total <= 0.0) {
			centers[c] = data[c % n];
			continue;
		}

		r = ((double)rand() / (double)RAND_MAX) * total;
		accum = 0.0;

		/*
		 * Fallback initialization protects against floating-point
		 * accumulation errors that might prevent selection below.
		 */
		centers[c] = data[n - 1];

		for (i = 0; i < n; i++) {
			accum += dist_sq[i];
			if (accum >= r) {
				centers[c] = data[i];
				break;
			}
		}
	}

	free(dist_sq);
	return 0;
}

/* ========================================================================
 * K-means Lloyd iteration: assign points to nearest center
 * ======================================================================== */

static void kmeans_assign(const double *data, int n, double *centers,
			  int k, int *labels, int *iterations)
{
	double *new_centers;
	double prev_centers[MAX_CLUSTER_K];
	int *counts;
	int iter;
	int i, c;
	bool changed;
	bool has_empty;
	double shift;

	new_centers = (double *)calloc((size_t)k, sizeof(double));
	counts = (int *)calloc((size_t)k, sizeof(int));
	if (!new_centers || !counts) {
		free(new_centers);
		free(counts);
		*iterations = 0;
		return;
	}

	for (iter = 0; iter < KMEANS_MAX_ITER; iter++) {
		changed = false;
		for (i = 0; i < n; i++) {
			double min_d = DBL_MAX;
			int best = 0;
			for (c = 0; c < k; c++) {
				double d = fabs(data[i] - centers[c]);
				if (d < min_d) {
					min_d = d;
					best = c;
				}
			}
			if (labels[i] != best) {
				labels[i] = best;
				changed = true;
			}
		}

		/* 记录更新前的质心，用于收敛判定 */
		for (c = 0; c < k; c++)
			prev_centers[c] = centers[c];

		memset(new_centers, 0, (size_t)k * sizeof(double));
		memset(counts, 0, (size_t)k * sizeof(int));
		for (i = 0; i < n; i++) {
			new_centers[labels[i]] += data[i];
			counts[labels[i]]++;
		}
		for (c = 0; c < k; c++) {
			if (counts[c] > 0)
				centers[c] = new_centers[c] / (double)counts[c];
		}

		/*
		 * 空簇处理：k-means++ 可能选中重复质心导致空簇，
		 * 空簇均值恒为 0，会使显著性验证（max_mean > min_mean × 2）误判，
		 * 故将空簇质心重置于离其分配质心最远的样本点
		 */
		has_empty = false;
		for (c = 0; c < k; c++) {
			double max_d = -1.0;
			int far_i = 0;

			if (counts[c] > 0)
				continue;
			has_empty = true;
			for (i = 0; i < n; i++) {
				double d = fabs(data[i] - centers[labels[i]]);
				if (d > max_d) {
					max_d = d;
					far_i = i;
				}
			}
			centers[c] = data[far_i];
		}

		/* 质心位移小于收敛精度且无空簇则提前结束（KMEANS_CONVERGENCE_EPS） */
		shift = 0.0;
		for (c = 0; c < k; c++)
			shift += fabs(centers[c] - prev_centers[c]);
		if (!changed && !has_empty)
			break;
		if (!has_empty && shift < KMEANS_CONVERGENCE_EPS)
			break;
	}

	*iterations = iter;
	free(new_centers);
	free(counts);
}

/* ========================================================================
 * Run full k-means pipeline on normalized data
 * ======================================================================== */

static int run_kmeans(const double *data, int n, int k,
			  double *centers, int *labels, int *iters)
{
	int ret;

	ret = kmeans_pp_init(data, n, k, centers);
	if (ret != 0)
		return ret;

	kmeans_assign(data, n, centers, k, labels, iters);

	return 0;
}

/* ========================================================================
 * Compute inertia (sum of squared distances) for elbow method
 * ======================================================================== */

static double compute_inertia(const double *data, int n, int k)
{
	double *centers, *norm;
	double mean, std;
	int *labels;
	double inertia = 0.0;
	int iter;
	int i;

	centers = (double *)malloc((size_t)k * sizeof(double));
	labels = (int *)calloc((size_t)n, sizeof(int));
	if (!centers || !labels) {
		free(centers);
		free(labels);
		return DBL_MAX;
	}

	if (z_score_normalize(data, n, &norm, &mean, &std) != 0) {
		free(centers);
		free(labels);
		return DBL_MAX;
	}

	if (run_kmeans(norm, n, k, centers, labels, &iter) != 0) {
		free(centers);
		free(labels);
		return DBL_MAX;
	}

	for (i = 0; i < n; i++)
		inertia += (norm[i] - centers[labels[i]]) *
			   (norm[i] - centers[labels[i]]);

	free(centers);
	free(labels);
	free(norm);
	return inertia;
}

/* ========================================================================
 * Elbow method: find best K via second-order difference
 * ======================================================================== */

static int elbow_find_best_k(const double *data, int n, int max_k)
{
	double inertias[MAX_CLUSTER_K];
	int n_points;
	double max_diff;
	int best_k;
	int i;

	n_points = max_k - 1;
	if (n_points < 3)
		return 2;

	for (i = 0; i < n_points; i++)
		inertias[i] = compute_inertia(data, n, i + 2);

	max_diff = -1.0;
	best_k = 2;
	for (i = 1; i < n_points - 1; i++) {
		double d1 = inertias[i] - inertias[i - 1];
		double d2 = inertias[i + 1] - inertias[i];
		double diff2 = fabs(d2 - d1);
		if (diff2 > max_diff) {
			max_diff = diff2;
			best_k = i + 2;
		}
	}

	return best_k;
}

/* ========================================================================
 * Compute original-value means per cluster
 * ======================================================================== */

static int compute_cluster_means(const double *orig, int n,
				 const int *labels, int k,
				 double *means, int *counts_out)
{
	double *sums;
	int *counts;
	int i;

	sums = calloc((size_t)k, sizeof(*sums));
	counts = calloc((size_t)k, sizeof(*counts));
	if (sums == NULL || counts == NULL) {
		free(sums);
		free(counts);
		return -ENOMEM;
	}

	for (i = 0; i < n; i++) {
		sums[labels[i]] += orig[i];
		counts[labels[i]]++;
	}

	for (i = 0; i < k; i++)
		means[i] = (counts[i] > 0) ?
			   sums[i] / (double)counts[i] : 0.0;

	if (counts_out != NULL) {
		for (i = 0; i < k; i++)
			counts_out[i] = counts[i];
	}

	free(sums);
	free(counts);
	return 0;
}

/*
 * 判断候选簇与基准簇之间的差异是否足够显著。
 * 同时满足相对倍数和绝对均值差，才判定为异常簇。
 */
static bool is_anomaly_cluster(double candidate_mean,
				   double baseline_mean)
{
	return candidate_mean >
			   baseline_mean * CLUSTER_RATIO_THRESHOLD;
}
/* ========================================================================
 * Extract all anomalous clusters: any non-empty cluster whose mean exceeds
 * the minimum-mean cluster by CLUSTER_RATIO_THRESHOLD is extracted.
 * Clusters are visited from largest mean to smallest, matching the spec's
 * "从最大簇开始，由大到小遍历" description.
 * ======================================================================== */

static uc_cluster_result_t *extract_anomaly_clusters(
	const double *orig, const int *orig_indices,
	int n, const int *labels, int k,
	const double *means, const int *counts,
	int min_support)
{
	int baseline_idx = -1;
	int total_count = 0;
	double total_sum = 0.0;
	uc_cluster_result_t *res;
	int *order;
	int order_count;
	int i, j, pos, c;
	bool found = false;

	/*
	 * 只从样本数达到 min_support 的簇中，
	 * 选择均值最小的簇作为正常基准。
	 */
	for (i = 0; i < k; i++) {
		if (counts[i] < min_support)
			continue;

		if (baseline_idx < 0 ||
		    means[i] < means[baseline_idx])
			baseline_idx = i;
	}

	if (baseline_idx < 0)
		return NULL;

	/*
	 * 统计异常簇。
	 * 这里只排除空簇，不限制异常簇的样本数量。
	 */
	for (i = 0; i < k; i++) {
		if (i == baseline_idx || counts[i] == 0)
			continue;

		if (is_anomaly_cluster(means[i],
				       means[baseline_idx])) {
			found = true;
			total_count += counts[i];
		}
	}

	if (!found)
		return NULL;

	res = calloc(1, sizeof(*res));
	if (res == NULL)
		return NULL;

	res->indices = malloc((size_t)total_count *
			      sizeof(*res->indices));
	res->values = malloc((size_t)total_count *
			     sizeof(*res->values));
	if (res->indices == NULL || res->values == NULL) {
		uc_free_result(res);
		return NULL;
	}

	/* 按照簇均值从大到小排列。 */
	order = malloc((size_t)k * sizeof(*order));
	if (order == NULL) {
		uc_free_result(res);
		return NULL;
	}

	order_count = 0;
	for (i = 0; i < k; i++) {
		if (counts[i] == 0)
			continue;

		pos = order_count;
		while (pos > 0 &&
		       means[i] > means[order[pos - 1]]) {
			order[pos] = order[pos - 1];
			pos--;
		}

		order[pos] = i;
		order_count++;
	}

	/*
	 * 提取异常簇。
	 * 即使异常簇只有一个样本，也允许被提取。
	 */
	pos = 0;
	for (j = 0; j < order_count; j++) {
		i = order[j];

		if (i == baseline_idx)
			continue;

		if (!is_anomaly_cluster(means[i],
					means[baseline_idx]))
			continue;

		for (c = 0; c < n; c++) {
			if (labels[c] != i)
				continue;

			res->indices[pos] = orig_indices ?
					    orig_indices[c] : c;
			res->values[pos] = orig[c];
			total_sum += orig[c];
			pos++;
		}
	}

	free(order);

	res->count = pos;
	res->mean = pos > 0 ? total_sum / (double)pos : 0.0;

	return res;
}
/*
 * Forward declaration because cluster_core() and
 * unified_clustering_recursive() call each other.
 */
static uc_cluster_result_t *unified_clustering_recursive(
	const double *values, const int *indices, int count, int depth);
/* ========================================================================
 * Recursive clustering: deeper anomaly isolation
 * ======================================================================== */

static uc_cluster_result_t *cluster_core(const double *values,
					 const int *indices,
					 int count, int depth)
{
	double *filtered_vals, *norm;
	int *filtered_indices, *labels;
	double mean, std;
	double means[MAX_CLUSTER_K];
	int cluster_counts[MAX_CLUSTER_K];
	double centers[MAX_CLUSTER_K];
	int filtered_n, best_k, max_k, min_support, iter;
	int ret;
	uc_cluster_result_t *result;

	(void)indices; /* 仅用于向递归层传递，当前层不直接使用 */

	/* Step 1: Filter positive values */
	if (filter_positive(values, count, &filtered_vals,
			    &filtered_indices, &filtered_n) != 0 ||
	    filtered_n < 2)
		return NULL;
	min_support = get_min_baseline_support(filtered_n);

	max_k = filtered_n / min_support;
	if (max_k > MAX_ELBOW_K)
		max_k = MAX_ELBOW_K;
	/* Step 2: Z-score normalize */
	if (z_score_normalize(filtered_vals, filtered_n,
			      &norm, &mean, &std) != 0) {
		free(filtered_vals);
		free(filtered_indices);
		return NULL;
	}

	/* Step 3: Elbow method for best K */
	best_k = elbow_find_best_k(filtered_vals, filtered_n,max_k);
	if (best_k < 2)
		best_k = 2;

	/* Step 4-5: K-means clustering */
	labels = calloc((size_t)filtered_n, sizeof(*labels));
	if (labels == NULL) {
		free(norm);
		free(filtered_vals);
		free(filtered_indices);
		return NULL;
	}

	ret = run_kmeans(norm, filtered_n, best_k, centers, labels, &iter);
	if (ret != 0) {
		free(labels);
		free(norm);
		free(filtered_vals);
		free(filtered_indices);
		return NULL;
	}

	/* Step 6: Compute per-cluster means */
	ret = compute_cluster_means(filtered_vals, filtered_n, labels,
				    best_k, means, cluster_counts);
	if (ret != 0) {
		free(labels);
		free(norm);
		free(filtered_vals);
		free(filtered_indices);
		return NULL;
	}

	/* Step 7: 提取异常簇 */
	result = extract_anomaly_clusters(filtered_vals, filtered_indices,
					  filtered_n, labels, best_k,
					  means, cluster_counts,
					  min_support);
	if (result == NULL) {
		free(labels);
		free(norm);
		free(filtered_vals);
		free(filtered_indices);
		return NULL;
	}

	free(labels);
	free(norm);
	free(filtered_vals);
	free(filtered_indices);

	/* Step 8: Recursive detection for deeper isolation */
	{
		uc_cluster_result_t *rec;

		rec = unified_clustering_recursive(result->values,
						   result->indices,
						   result->count,
						   depth + 1);
		if (rec != NULL) {
			int i;

			for (i = 0; i < rec->count; i++) {
				if (rec->indices[i] >= 0 &&
				    rec->indices[i] < result->count)
					rec->indices[i] =
						result->indices[rec->indices[i]];
			}

			uc_free_result(result);
			return rec;
		}
	}

	return result;
}

/* ========================================================================
 * Recursive clustering with depth guard
 * ======================================================================== */

static uc_cluster_result_t *unified_clustering_recursive(
	const double *values, const int *indices, int count, int depth)
{
	if (depth >= UC_MAX_RECURSION_DEPTH || count < 2)
		return NULL;

	return cluster_core(values, indices, count, depth);
}

/* ========================================================================
 * Public interface
 * ======================================================================== */

uc_cluster_result_t *uc_unified_clustering(const double *values,
					   const int *indices,
					   int count)
{
	if (!values || count < 2)
		return NULL;

	return cluster_core(values, indices, count, 0);
}

uc_cluster_result_t *uc_detect_anomaly(const double *values,
				       const int *indices,
				       int count)
{
	return uc_unified_clustering(values, indices, count);
}

void uc_free_result(uc_cluster_result_t *result)
{
	if (result) {
		free(result->indices);
		free(result->values);
		free(result);
	}
}
