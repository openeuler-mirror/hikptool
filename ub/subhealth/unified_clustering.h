/*
 * Copyright (c) 2025 Hisilicon Technologies Co., Ltd.
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

#ifndef UNIFIED_CLUSTERING_H
#define UNIFIED_CLUSTERING_H

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 时延数据实数类型 */
typedef double uc_real_t;

/* ========================================================================
 * 聚类结果结构
 * ======================================================================== */

/**
 * 异常簇结果
 * indices 和 values 由内部 malloc 分配，调用者通过 uc_free_result() 释放
 */
typedef struct {
	int *indices;           /* 原始索引列表 */
	uc_real_t *values;      /* 时延值列表 */
	int count;              /* 元素数量 */
	uc_real_t mean;         /* 簇均值 */
} uc_cluster_result_t;

/* ========================================================================
 * 公开接口
 * ======================================================================== */

/**
 * 统一聚类算法 - 主接口
 *
 * 基于 K-means++ 的一维时延异常检测：
 *   1. 过滤非正值（≤0 视为无效时延）
 *   2. Z-score 标准化
 *   3. 肘部法确定最佳 K（最大 min(n, MAX_ELBOW_K)）
 *   4. K-means++ 初始化 + Lloyd 迭代（最多 KMEANS_MAX_ITER 次）
 *   5. 显著性验证（max_mean > min_mean × CLUSTER_RATIO_THRESHOLD）
 *   6. 对高值簇递归细化（最大深度 10）
 *
 * @param values  时延数据数组
 * @param indices 原始索引数组（可为 NULL）
 * @param count   数据数量
 * @return 异常簇结果，无异常返回 NULL；调用者负责调用 uc_free_result() 释放
 */
uc_cluster_result_t *uc_unified_clustering(const uc_real_t *values,
					   const int *indices,
					   int count);

/**
 * 便捷接口：检测异常簇
 *
 * 等价于 uc_unified_clustering(values, indices, count)
 */
uc_cluster_result_t *uc_detect_anomaly(const uc_real_t *values,
					   const int *indices,
					   int count);

/**
 * 释放 uc_cluster_result_t 占用的内存
 */
void uc_free_result(uc_cluster_result_t *result);

#ifdef __cplusplus
}
#endif

#endif /* UNIFIED_CLUSTERING_H */
