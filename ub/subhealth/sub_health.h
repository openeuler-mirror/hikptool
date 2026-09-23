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

#ifndef SUB_HEALTH_H
#define SUB_HEALTH_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <errno.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================
 * 4. 常量
 * ======================================================================== */

/* 4.1 容量限制 */
#define MAX_NODE_NUM        1024
#define MAX_NODE_PER_L1     128
#define MAX_UBPU_NUM        16
#define MAX_PORT_NUM        16
#define MAX_PROBE_TARGETS   128
#define MAX_L1_SWITCH_NUM   256
#define MAX_L2_SWITCH_NUM   256
#define MAX_EID_LEN         64
#define MAX_PATH_LEN        512
#define MAX_JSON_BUF_SIZE   (30 * 1024 * 1024)  /* 30MB */
#define MAX_L1_EID_NUM      256
/* JSON 中间文件类型与版本 */
#define SUB_HEALTH_META_KEY           "_sub_health_meta"
#define SUB_HEALTH_FILE_TYPE_KEY      "file_type"
#define SUB_HEALTH_SCHEMA_VERSION_KEY "schema_version"

#define SUB_HEALTH_FILE_PROBE_PLAN    "probe_plan"
#define SUB_HEALTH_FILE_PROBE_RESULT  "probe_result"

#define SUB_HEALTH_SCHEMA_VERSION     1
/* 4.1 拓扑相关 */
#define CPU_PORT_COUNT       2
#define MAX_CANDIDATE_PER_L1 3

/* 4.1 探测参数 */
#define DEFAULT_COVERAGE_K          5
#define MIN_COVERAGE_K              3
#define MAX_COVERAGE_K              20
#define DEFAULT_PACKET_SIZE         4096
#define DEFAULT_PACKET_COUNT_INTRA  10
#define LATENCY_TOP_K               3

/* 4.1 阈值 */
#define THRESHOLD_TIME_DEFAULT  100

/* 4.2 硬编码关键字面量 */
#define L1_UPLINK_FACTOR         4.0
#define CLUSTER_RATIO_THRESHOLD  2.0
#define KMEANS_MAX_ITER          300
#define KMEANS_CONVERGENCE_EPS   1e-9
#define MAX_ELBOW_K              10
#define MAX_CONSECUTIVE_WINDOW   3

/* ========================================================================
 * 性能计时宏
 * ======================================================================== */

#define PERF_DECLARE(var) \
	struct timespec var##_start, var##_end; \
	double var##_ms = 0.0

#define PERF_START(var) \
	clock_gettime(CLOCK_MONOTONIC, &var##_start)

#define PERF_END(var) \
	clock_gettime(CLOCK_MONOTONIC, &var##_end)

#define PERF_PRINT(var, name) \
	do { \
		var##_ms = ((var##_end.tv_sec - var##_start.tv_sec) * 1000.0 + \
			   (var##_end.tv_nsec - var##_start.tv_nsec) / 1000000.0); \
		printf("[PERF] %s: %.2f ms\n", name, var##_ms); \
	} while (0)

/* ========================================================================
 * 3. 数据结构
 * ======================================================================== */

/* 端口 EID 信息 */
struct port_eid_info {
	uint32_t physical_id;            /* 端口物理 ID */
	char eid[MAX_EID_LEN];           /* URMA EID 字符串 */
};

/* UBPU 信息 - 最多 16 端口 */
struct ubpu_info {
	uint32_t ubpu_id;                /* UBPU 编号 */
	uint32_t port_num;               /* 端口数量 */
	struct port_eid_info ports[MAX_PORT_NUM];
};

/* 节点信息 */
struct node_info {
	char ip[64];                     /* 节点 IP */
	uint32_t ubpu_num;               /* UBPU 数量（最多 MAX_UBPU_NUM） */
	struct ubpu_info ubpus[MAX_UBPU_NUM];
};

/* L1 交换机信息 */
struct l1_switch_info {
	char name[64];                   /* L1 交换机名 */
	char node_ip[64];                /* 该 L1 的节点 IP */
	struct node_info node;           /* 节点信息 */
};

/* 网络拓扑 */
struct network_topology {
	uint32_t l2_switch_num;                                    /* L2 交换机数量 */
	char l2_switches[MAX_L2_SWITCH_NUM][64];                   /* L2 交换机名列表 */
	uint32_t l1_switch_num;                                    /* L1 交换机数量 */
	struct l1_switch_info l1_switches[MAX_L1_SWITCH_NUM];      /* L1 交换机列表 */
};

/* 探测对配置（按端口粒度） */
struct probe_pair {
	uint32_t packet_count_intra;                               /* 每 intra 目的端口发包数 */
	uint32_t packet_count_inter;                               /* 每 inter 目的端口发包数 */
	uint32_t packet_size;                                      /* 包大小 */
	char src_eid[MAX_EID_LEN];                                 /* 源端口 EID */
	uint32_t intra_l1_dst_num;                                 /* intra 目的端口数量 */
	char intra_l1_dst_eids[MAX_PORT_NUM][MAX_EID_LEN];         /* intra 目的 EID 列表 */
	uint32_t inter_l1_dst_num;                                 /* inter 目的端口数量 */
	char inter_l1_dst_eids[MAX_PORT_NUM][MAX_EID_LEN];         /* inter 目的 EID 列表 */
};

/* 探测计划（按 Node -> UBPU -> Port 组织） */
struct probe_plan {
	char node_ip[64];
	uint32_t ubpu_num;
	struct {
		uint32_t ubpu_id;
		uint32_t port_num;
		struct {
			uint32_t port_id;
			struct probe_pair pair;
		} ports[MAX_PORT_NUM];
	} ubpus[MAX_UBPU_NUM];
};

/* 探测结果（含时延，单位 ms） */
struct probe_result {
	uint32_t packet_count_intra;
	uint32_t packet_count_inter;
	uint32_t packet_size;
	char src_eid[MAX_EID_LEN];
	uint32_t intra_l1_dst_num;
	char intra_l1_dst_eids[MAX_PORT_NUM][MAX_EID_LEN];
	double intra_l1_latencies[MAX_PORT_NUM];       /* 时延单位：ms */
	uint32_t inter_l1_dst_num;
	char inter_l1_dst_eids[MAX_PORT_NUM][MAX_EID_LEN];
	double inter_l1_latencies[MAX_PORT_NUM];       /* 时延单位：ms */
};

/* 超时状态 */
enum timeout_status {
	TIMEOUT_NONE = 0,      /* 所有包都成功 */
	TIMEOUT_PARTIAL = 1,   /* 部分包超时 */
	TIMEOUT_FULL = 2,      /* 所有包都超时 */
};

/* 故障域枚举 */
enum sub_health_domain {
	SUB_HEALTH_NONE = 0,
	SUB_HEALTH_NODE_UPLINK,         /* 1: 节点到 L1 上行 */
	SUB_HEALTH_L1_UPLINK,           /* 2: L1 到 L2 上行 */
	SUB_HEALTH_L2_DOWNLINK,         /* 3: L2 到 L1 下行 */
	SUB_HEALTH_INTRA_L1_DOWNLINK,   /* 4: L1 内下行 */
	SUB_HEALTH_INTER_L1_DOWNLINK,   /* 5: 跨 L1 下行 */
};

/* ========================================================================
 * 5. 模块接口
 * ======================================================================== */

/* topology_generator.c */
int generate_topology_from_restconf(const char *output_file);

/* probe_plan.c */
int sub_health_probe_plan(const char *topology_file, uint32_t coverage_k,
			  uint32_t packet_size, const char *output_file);

/* probe_execute.c */
int sub_health_probe_execute(const char *plan_file,
				 const char *result_file);

/* sub_health_detect.c */
int sub_health_detect(const char *probe_result_file, const char *output_file,
					  uint32_t time_threshold);
#ifdef __cplusplus
}
#endif

#endif /* SUB_HEALTH_H */
