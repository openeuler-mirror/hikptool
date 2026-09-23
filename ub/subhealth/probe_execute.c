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
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>
#include <float.h>
/* ========================================================================
 * 常量
 * ======================================================================== */

#define MAX_SUPPORTED_UBPU   2          /* 最多支持 2 个 UBPU（0/1） */
#define MAX_PING_RESULTS     4096       /* 单次 urma_ping 结果行数上限 */
#define URMA_PING_LOG_FILE   "urma_ping_output.log"
#define URMA_PING_RAW_OUTPUT_MAX   (512U * 1024U)
#define PROBE_EXEC_WATCHDOG_SEC   180U
#define PROBE_EXEC_KILL_GRACE_SEC 5U
#define WATCHDOG_POLL_NS          100000000L /* 100 ms */
/* RESTCONF 查询命令（获取 slot ID 以构造 slot.slot.slot.slot 本地 IP） */
#define CURL_CMD \
	"curl -X GET " \
	"--unix-socket /run/ubm/socket/ubm_nuds/restconf.sock " \
	"\"http://localhost/restconf/data/huawei-vbussw-service:vbussw-service/" \
	"static-urma-eids\" " \
	"-H \"Accept: application/yang-data+xml\" " \
	"-H \"Content-Type: application/yang-data+xml\" 2>/dev/null"

#define CURL_CMD_LOCAL_SLOT \
	"curl --silent --show-error --fail --max-time 3 " \
	"-X GET " \
	"--unix-socket /run/ubm/socket/ubm_nuds/restconf.sock " \
	"\"http://localhost/restconf/data/" \
	"huawei-lingqu-topology:lingqu-topology/nodes\" " \
	"-H \"Accept: application/yang-data+xml\" " \
	"-H \"Content-Type: application/yang-data+xml\" 2>/dev/null"
/* ========================================================================
 * 数据结构
 * ======================================================================== */

struct ubpu_probe_data {
	bool present;
	char src_eid[MAX_EID_LEN];
	uint32_t packet_count_intra;
	uint32_t packet_count_inter;
	uint32_t packet_size;
	int intra_dst_num;
	char intra_dst_eids[MAX_PROBE_TARGETS][MAX_EID_LEN];
	int inter_dst_num;
	char inter_dst_eids[MAX_PROBE_TARGETS][MAX_EID_LEN];
};

struct node_probe_data {
	char ip[64];
	int ubpu_count;
	struct ubpu_probe_data ubpus[MAX_SUPPORTED_UBPU];
};

struct probe_task {
	int task_id;
	char src_eid[MAX_EID_LEN];
	char dst_eid[MAX_EID_LEN];
	uint32_t packet_size;
	uint32_t packet_count;
	int top_n;
	bool is_inter;
};

enum probe_exec_status {
	PROBE_EXEC_OK = 0,  /* 获得足够 RTT 样本，可写入结果 JSON。 */
	PROBE_EXEC_TIMEOUT, /* 所有探测包均明确输出 Request timeout。 */
	PROBE_EXEC_FAIL,    /* 命令、配置、解析或内部执行失败。 */
	PROBE_EXEC_STATUS_COUNT
};

struct probe_task_result {
	double latency;                 /* 仅 PROBE_EXEC_OK 时有效。 */
	enum probe_exec_status status;  /* 成功、全部超时或失败。 */
};

struct thread_arg {
	struct probe_task *task;
	struct probe_task_result *result;
};

struct probe_exec_summary {
	unsigned int status_count[PROBE_EXEC_STATUS_COUNT];
};

/* ========================================================================
 * 本机 IP 列表
 * ======================================================================== */

static char g_local_ips[MAX_NODE_NUM][64];
static int g_local_ip_count = 0;

static pthread_mutex_t g_urma_ping_log_mutex = PTHREAD_MUTEX_INITIALIZER;

static const char *probe_exec_status_to_string(enum probe_exec_status status)
{
	switch (status) {
	case PROBE_EXEC_OK:
		return "ok";
	case PROBE_EXEC_TIMEOUT:
		return "timeout";
	case PROBE_EXEC_FAIL:
		return "fail";
	default:
		return "UNKNOWN";
	}
}
/*
 * 返回值：
 *   1：子进程已经退出并被回收
 *   0：等待超时
 *  <0：系统调用失败
 */
static int wait_child_with_timeout(pid_t pid, unsigned int timeout_sec,
				   int *status)
{
	struct timespec start;
	struct timespec now;
	struct timespec delay;
	time_t elapsed_sec;
	pid_t wait_ret;

	if (clock_gettime(CLOCK_MONOTONIC, &start) != 0)
		return -errno;

	for (;;) {
		wait_ret = waitpid(pid, status, WNOHANG);
		if (wait_ret == pid)
			return 1;

		if (wait_ret < 0) {
			if (errno == EINTR)
				continue;
			return -errno;
		}

		if (clock_gettime(CLOCK_MONOTONIC, &now) != 0)
			return -errno;

		elapsed_sec = now.tv_sec - start.tv_sec;
		if (elapsed_sec > (time_t)timeout_sec ||
		    (elapsed_sec == (time_t)timeout_sec &&
		     now.tv_nsec >= start.tv_nsec))
			return 0;

		delay.tv_sec = 0;
		delay.tv_nsec = WATCHDOG_POLL_NS;
		(void)nanosleep(&delay, NULL);
	}
}

static int write_all(int fd, const void *buffer, size_t size)
{
	const unsigned char *pos = buffer;
	size_t written = 0;

	while (written < size) {
		ssize_t ret = write(fd, pos + written, size - written);

		if (ret < 0) {
			if (errno == EINTR)
				continue;
			return -errno;
		}

		if (ret == 0)
			return -EIO;

		written += (size_t)ret;
	}

	return 0;
}

static int read_all(int fd, void *buffer, size_t size)
{
	unsigned char *pos = buffer;
	size_t read_size = 0;

	while (read_size < size) {
		ssize_t ret = read(fd, pos + read_size, size - read_size);

		if (ret < 0) {
			if (errno == EINTR)
				continue;
			return -errno;
		}

		if (ret == 0)
			return -EIO;

		read_size += (size_t)ret;
	}

	return 0;
}

/*
 * 先向整个探测进程组发送 SIGTERM。
 * 5 秒后仍未结束，则发送 SIGKILL。
 */
static void terminate_probe_process_group(pid_t child_pid)
{
	int status;
	int wait_ret;
	pid_t ret;

	/* 负 PID 表示向整个进程组发送信号 */
	(void)kill(-child_pid, SIGTERM);

	/* 防止进程组尚未建立，只杀到了空进程组 */
	(void)kill(child_pid, SIGTERM);

	wait_ret = wait_child_with_timeout(child_pid,
					   PROBE_EXEC_KILL_GRACE_SEC,
					   &status);

	/*
	 * 即使 Step2 子进程已经退出，进程组中仍可能残留
	 * shell 或 urma_ping，因此继续清理整个进程组。
	 */
	(void)kill(-child_pid, SIGKILL);

	if (wait_ret == 1 || wait_ret == -ECHILD)
		return;

	(void)kill(child_pid, SIGKILL);

	do {
		ret = waitpid(child_pid, &status, 0);
	} while (ret < 0 && errno == EINTR);
}
static void init_probe_task_result(struct probe_task_result *result)
{
	memset(result, 0, sizeof(*result));
	result->status = PROBE_EXEC_FAIL;
}

static void append_raw_output(char **raw_output, size_t *raw_len,
			      size_t *raw_capacity, const char *line,
			      bool *truncated)
{
	size_t line_len;
	size_t required;
	size_t new_capacity;
	char *new_output;

	if (*truncated)
		return;

	line_len = strlen(line);
	if (line_len > URMA_PING_RAW_OUTPUT_MAX - *raw_len) {
		*truncated = true;
		return;
	}

	required = *raw_len + line_len + 1;
	if (*raw_output == NULL || required > *raw_capacity) {
		new_capacity = *raw_capacity == 0 ? 4096U : *raw_capacity;
		while (new_capacity < required &&
		       new_capacity < URMA_PING_RAW_OUTPUT_MAX)
			new_capacity *= 2U;
		if (new_capacity > URMA_PING_RAW_OUTPUT_MAX)
			new_capacity = URMA_PING_RAW_OUTPUT_MAX;
		if (new_capacity < required) {
			*truncated = true;
			return;
		}

		new_output = (char *)realloc(*raw_output, new_capacity);
		if (!new_output) {
			*truncated = true;
			return;
		}
		*raw_output = new_output;
		*raw_capacity = new_capacity;
	}
	if (*raw_output == NULL) {
		*truncated = true;
		return;
	}

	memcpy(*raw_output + *raw_len, line, line_len);
	*raw_len += line_len;
	(*raw_output)[*raw_len] = '\0';
}

static void write_urma_ping_log_entry(const struct probe_task *task,
				      const struct probe_task_result *result,
				      const char *cmd,
				      const char *raw_output,
				      bool raw_output_truncated,
				      int success_count,
				      int request_timeout_count,
				      int exit_code)
{
	FILE *log_fp;

	pthread_mutex_lock(&g_urma_ping_log_mutex);
	log_fp = fopen(URMA_PING_LOG_FILE, "a");
	if (!log_fp) {
		pthread_mutex_unlock(&g_urma_ping_log_mutex);
		return;
	}

	fputs("========== urma_ping begin ==========\n", log_fp);
	fprintf(log_fp, "task_id: %d\n", task->task_id);
	fprintf(log_fp, "src_eid: %s\n", task->src_eid);
	fprintf(log_fp, "dst_eid: %s\n", task->dst_eid);
	fprintf(log_fp, "packet_size: %u\n", task->packet_size);
	fprintf(log_fp, "packet_count: %u\n", task->packet_count);
	fprintf(log_fp, "required_samples: %d\n", task->top_n);
	fprintf(log_fp, "Status: %s\n",
		probe_exec_status_to_string(result->status));
	fprintf(log_fp, "success_count: %d\n", success_count);
	fprintf(log_fp, "request_timeout_count: %d\n",
		request_timeout_count);
	fprintf(log_fp, "exit_code: %d\n", exit_code);
	if (cmd && cmd[0] != '\0')
		fprintf(log_fp, "cmd: %s\n", cmd);
	fputs("-------------------------------------\n", log_fp);
	if (raw_output && raw_output[0] != '\0')
		fputs(raw_output, log_fp);
	else
		fputs("[no raw output]\n", log_fp);
	if (raw_output_truncated)
		fputs("[raw output truncated]\n", log_fp);
	if (success_count + request_timeout_count < (int)task->packet_count)
		fputs("[raw output incomplete]\n", log_fp);
	fputs("========== urma_ping end ============\n", log_fp);
	fclose(log_fp);
	pthread_mutex_unlock(&g_urma_ping_log_mutex);
}

static void add_local_ip(const char *ip)
{
	int i;

	if (g_local_ip_count >= MAX_NODE_NUM)
		return;

	for (i = 0; i < g_local_ip_count; i++) {
		if (strcmp(g_local_ips[i], ip) == 0)
			return;
	}

	snprintf(g_local_ips[g_local_ip_count], 64, "%s", ip);
	g_local_ip_count++;
}

/* Step 0a - 执行 ip addr 获取本机 IPv4 地址列表 */
static void load_local_ips(void)
{
	FILE *fp;
	char line[512];

	g_local_ip_count = 0;

	fp = popen("ip addr 2>/dev/null", "r");
	if (!fp)
		return;

	while (fgets(line, sizeof(line), fp)) {
		char *inet_ptr = strstr(line, "inet ");
		char ip[64];
		char *slash;
		struct in_addr addr;

		if (!inet_ptr)
			continue;

		/* 跳过 IPv6 的 inet6 前缀（已用 "inet " 匹配） */
		inet_ptr += strlen("inet ");
		slash = strchr(inet_ptr, '/');
		if (!slash)
			continue;

		memcpy(ip, inet_ptr, (size_t)(slash - inet_ptr));
		ip[slash - inet_ptr] = '\0';

		if (inet_pton(AF_INET, ip, &addr) == 1)
			add_local_ip(ip);
	}

	pclose(fp);
}

/* Step 0b - 通过 RESTCONF API 获取 slot IP（slot.slot.slot.slot） */
static void load_slot_id_ips(void)
{
	FILE *fp;
	char line[512];
	int slot_id = -1;
	int status;
	char ip[64];

	fp = popen(CURL_CMD_LOCAL_SLOT, "r");
	if (!fp)
		return;

	while (fgets(line, sizeof(line), fp)) {
		char *start = strstr(line, "<slot>");
		char *end;
		char *parse_end;
		char value[64];
		long parsed;
		size_t value_len;

		if (!start)
			continue;

		start += strlen("<slot>");
		end = strstr(start, "</slot>");
		if (!end)
			continue;

		value_len = (size_t)(end - start);
		if (value_len == 0 || value_len >= sizeof(value))
			continue;

		memcpy(value, start, value_len);
		value[value_len] = '\0';

		errno = 0;
		parsed = strtol(value, &parse_end, 10);
		if (errno != 0 || parse_end == value || *parse_end != '\0')
			continue;

		/* 如果实际 slot 范围更小，应改成真实范围 */
		if (parsed <= 0 || parsed > 255)
			continue;

		slot_id = (int)parsed;
		break;
	}

	status = pclose(fp);
	if (status != 0 || slot_id < 0)
		return;

	snprintf(ip, sizeof(ip), "%d.%d.%d.%d",
		 slot_id, slot_id, slot_id, slot_id);
	add_local_ip(ip);
}

static bool is_local_ip(const char *ip)
{
	int i;

	for (i = 0; i < g_local_ip_count; i++) {
		if (strcmp(g_local_ips[i], ip) == 0)
			return true;
	}
	return false;
}

/* ========================================================================
 * 探测计划 JSON 解析
 * ======================================================================== */

static int parse_eid_list(sh_json *obj, const char *key, const char *alias,
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

static void parse_ubpu_probe_data(sh_json *ubpu_obj, struct ubpu_probe_data *ubpu)
{
	sh_json *item;

	ubpu->present = true;

	item = sh_json_get_item(ubpu_obj, "src_eid");
	if (!item)
		item = sh_json_get_item(ubpu_obj, "src_port_eid");
	if (item && item->kind == SH_JSON_BUF)
		snprintf(ubpu->src_eid, MAX_EID_LEN, "%s", item->str);

	item = sh_json_get_item(ubpu_obj, "packet_count_intra");
	if (item)
		ubpu->packet_count_intra = (uint32_t)item->dval;

	item = sh_json_get_item(ubpu_obj, "packet_count_inter");
	if (item)
		ubpu->packet_count_inter = (uint32_t)item->dval;

	item = sh_json_get_item(ubpu_obj, "packet_size");
	if (item)
		ubpu->packet_size = (uint32_t)item->dval;

	ubpu->intra_dst_num = parse_eid_list(ubpu_obj, "intra_l1_dst_eids",
					     "intra_l1_dst_port_eids",
					     ubpu->intra_dst_eids,
					     MAX_PROBE_TARGETS);
	ubpu->inter_dst_num = parse_eid_list(ubpu_obj, "inter_l1_dst_eids",
					     "inter_l1_dst_port_eids",
					     ubpu->inter_dst_eids,
					     MAX_PROBE_TARGETS);
}

static void parse_local_node_ubpus(sh_json *node_obj, struct node_probe_data *node)
{
	memset(node, 0, sizeof(*node));

	for (sh_json *child = node_obj->head; child; child = child->fwd) {
		int ubpu_id;

		if (!child->name)
			continue;

		/* "0"/"1" 是 UBPU 端口 key */
		ubpu_id = atoi(child->name);
		if (ubpu_id < 0 || ubpu_id >= MAX_SUPPORTED_UBPU)
			continue;

		parse_ubpu_probe_data(child, &node->ubpus[ubpu_id]);
		if (ubpu_id + 1 > node->ubpu_count)
			node->ubpu_count = ubpu_id + 1;
	}
}

/* ========================================================================
 * 校验输入文件是否为探测对计划（probe plan）
 *
 * 探测对计划的结构要求：
 *   1) 顶层存在 "_l1_maps"（L1 交换机 -> EID 列表映射）；
 *   2) 至少存在一个 IP 节点，其下 UBPU 含有非空 src_eid，
 *      且具备至少一个 intra_l1_dst_eids / inter_l1_dst_eids 元素。
 * 不符合时返回 -EINVAL。
 * ======================================================================== */
static int check_probe_plan_file(sh_json *root)
{
	struct in_addr addr;
	sh_json *l1_maps;
	sh_json *top;

	if (root == NULL || root->kind != SH_JSON_MAP)
		return -EINVAL;

	/* 1) 必须有 L1 映射表 */
	l1_maps = sh_json_get_item_cs(root, "_l1_maps");
	if (l1_maps == NULL || l1_maps->kind != SH_JSON_MAP)
		return -EINVAL;

	/* 2) 至少一个 IP 节点下存在有效探测对（src_eid + 目标列表） */
	for (top = root->head; top != NULL; top = top->fwd) {
		sh_json *ubpu;

		if (top->name == NULL ||
		    top->kind != SH_JSON_MAP ||
		    inet_pton(AF_INET, top->name, &addr) != 1)
			continue;

		for (ubpu = top->head; ubpu != NULL; ubpu = ubpu->fwd) {
			sh_json *src_eid;
			sh_json *dsts;
			int addr_num = 0;
			int idst_num = 0;

			if (ubpu->kind != SH_JSON_MAP)
				continue;

			src_eid = sh_json_get_item_cs(ubpu,
								   "src_eid");
			if (src_eid == NULL || src_eid->kind != SH_JSON_BUF ||
			    src_eid->str == NULL ||
			    src_eid->str[0] == '\0')
				continue;

			dsts = sh_json_get_item_cs(
				ubpu, "intra_l1_dst_eids");
			if (dsts != NULL && dsts->kind == SH_JSON_SEQ)
				idst_num = sh_json_item_count(dsts);
			dsts = sh_json_get_item_cs(
				ubpu, "inter_l1_dst_eids");
			if (dsts != NULL && dsts->kind == SH_JSON_SEQ)
				addr_num = sh_json_item_count(dsts);

			if (idst_num > 0 || addr_num > 0)
				return 0;
		}
	}

	return -EINVAL;
}

/* ========================================================================
 * 输出探测计划输入文件的格式错误信息。
 * ======================================================================== */
static void report_invalid_probe_plan(const char *plan_file)
{
	fprintf(stderr,
		"[ERROR] '%s' is not a valid probe plan file: "
		"missing _probe_config.\n", plan_file);
}

/* 遍历 JSON 顶层 key，收集本机节点的探测数据 */
static void collect_probe_data(sh_json *root, struct node_probe_data *nodes,
			       int *node_count)
{
	struct in_addr addr;

	for (sh_json *top = root->head; top; top = top->fwd) {
		if (!top->name)
			continue;

		/* 顶层 key 不是合法 IPv4（如 L1 映射表）→ 跳过 */
		if (inet_pton(AF_INET, top->name, &addr) != 1)
			continue;
		if (top->kind != SH_JSON_MAP)
			continue;

		/* 非本机节点 → 跳过 */
		if (!is_local_ip(top->name))
			continue;

		if (*node_count >= MAX_NODE_NUM)
			break;

		parse_local_node_ubpus(top, &nodes[*node_count]);
		snprintf(nodes[*node_count].ip, sizeof(nodes[*node_count].ip),
			 "%s", top->name);
		(*node_count)++;
	}
}

/* ========================================================================
 * 单次 urma_ping 执行
 * ======================================================================== */

static void classify_latency_result(const struct probe_task *task,
				    struct probe_task_result *result,
				    int success_count,
				    int request_timeout_count,
				    int exit_code)
{
	int output_count = success_count + request_timeout_count;

	/* 所有发包均有明确超时输出时，即使 urma_ping 返回非零也算 timeout。 */
	if (task->packet_count > 0 && success_count == 0 &&
	    request_timeout_count == (int)task->packet_count) {
		result->status = PROBE_EXEC_TIMEOUT;
	} else if (exit_code != 0 || output_count != (int)task->packet_count) {
		result->status = PROBE_EXEC_FAIL;
	} else if (success_count >= task->top_n) {
		result->status = PROBE_EXEC_OK;
	} else {
		result->status = PROBE_EXEC_FAIL;
	}
}

static int run_urma_ping_once(const struct probe_task *task,
			      double *success_latencies,
			      struct probe_task_result *result)
{
	char cmd[MAX_PATH_LEN] = {0};
	char line[1024];
	char *raw_output = NULL;
	size_t raw_len = 0;
	size_t raw_capacity = 0;
	bool raw_output_truncated = false;
	bool config_error = false;
	bool pipe_read_error = false;
	FILE *fp = NULL;
	int cmd_len;
	int wait_status;
	int success_count = 0;
	int request_timeout_count = 0;
	int exit_code = -1;

	init_probe_task_result(result);

	cmd_len = snprintf(cmd, sizeof(cmd),
			   "urma_ping %s -I %s -s %u -c %u 2>&1",
			   task->dst_eid, task->src_eid, task->packet_size,
			   task->packet_count);
	if (cmd_len < 0 || (size_t)cmd_len >= sizeof(cmd)) {
		goto out;
	}

	fp = popen(cmd, "r");
	if (!fp) {
		goto out;
	}

	/* 始终排空管道，避免输出达到保存上限后子进程阻塞在 write()。 */
	while (fgets(line, sizeof(line), fp)) {
		char *time_ptr;

		append_raw_output(&raw_output, &raw_len, &raw_capacity, line,
				  &raw_output_truncated);

		if (strstr(line, "Request timeout") != NULL)
			request_timeout_count++;
		if (strstr(line, "Failed to find") != NULL)
			config_error = true;

		time_ptr = strstr(line, "time=");
		if (time_ptr && success_count < MAX_PING_RESULTS) {
			char *parse_end;
			double latency;

			errno = 0;
			latency = strtod(time_ptr + strlen("time="), &parse_end);
			if (errno == 0 && parse_end != time_ptr + strlen("time=") &&
			    latency > 0.0) {
				success_latencies[success_count++] = latency;
			}
		}
	}
	if (ferror(fp)) {
		pipe_read_error = true;
	}

	wait_status = pclose(fp);
	fp = NULL;
	if (wait_status == -1)
		goto out;

	if (WIFEXITED(wait_status)) {
		exit_code = WEXITSTATUS(wait_status);
		if (pipe_read_error || config_error ||
		    exit_code == 126 || exit_code == 127) {
			result->status = PROBE_EXEC_FAIL;
		} else {
			classify_latency_result(task, result, success_count,
						request_timeout_count, exit_code);
		}
	}

out:
	if (fp)
		(void)pclose(fp);
	write_urma_ping_log_entry(task, result, cmd, raw_output,
				  raw_output_truncated, success_count,
				  request_timeout_count, exit_code);
	free(raw_output);
	return success_count;
}

/* ========================================================================
 * Top-N 裁剪均值：降序排列，去掉最大最小后取中间平均
 * ======================================================================== */

static int cmp_double_desc(const void *a, const void *b)
{
	double da = *(const double *)a;
	double db = *(const double *)b;

	return (db > da) - (db < da);
}

static double calc_trimmed_mean(double *latencies, int count, int top_n)
{
	double *sorted;
	double sum = 0.0;
	int trim_count;
	int keep_count;
	int i;

	/* 只取最近 top_n 个成功样本（与 coverage_k 对齐） */
	if (count > top_n) {
		sorted = latencies + (count - top_n);
		keep_count = top_n;
	} else {
		sorted = latencies;
		keep_count = count;
	}

	qsort(sorted, (size_t)keep_count, sizeof(double), cmp_double_desc);

	trim_count = (keep_count - LATENCY_TOP_K) / 2;
	if (trim_count < 0)
		trim_count = 0;

	for (i = trim_count; i < keep_count - trim_count; i++)
		sum += sorted[i];

	return sum / (double)(keep_count - 2 * trim_count);
}

/* ========================================================================
 * 执行一次探测任务（多线程入口）
 * ======================================================================== */

static void execute_urma_ping(struct probe_task *task,
			      struct probe_task_result *result)
{
	double *success_latencies = NULL;
	int success_count;

	success_latencies = (double *)calloc((size_t)MAX_PING_RESULTS, sizeof(double));
	if (!success_latencies) {
		init_probe_task_result(result);
		write_urma_ping_log_entry(task, result, "", NULL, false,
					  0, 0, -1);
		return;
	}

	success_count = run_urma_ping_once(task, success_latencies, result);
	if (result->status == PROBE_EXEC_OK) {
		result->latency = calc_trimmed_mean(success_latencies,
						    success_count,
						    task->top_n);
	}

	free(success_latencies);
}

static void *probe_thread_func(void *arg)
{
	struct thread_arg *ta = (struct thread_arg *)arg;

	execute_urma_ping(ta->task, ta->result);
	return NULL;
}

/* ========================================================================
 * 探测任务填充与并发执行
 * ======================================================================== */

static int populate_probe_tasks(const struct node_probe_data *node,
				struct probe_task *tasks, int max_tasks,
				int *ubpu_offsets, int max_ubpu)
{
	int task_count = 0;
	int u, i;

	memset(ubpu_offsets, 0, (size_t)(max_ubpu + 1) * sizeof(int));

	for (u = 0; u < node->ubpu_count && u < max_ubpu; u++) {
		const struct ubpu_probe_data *ubpu = &node->ubpus[u];

		/* 记录该 UBPU 的任务起始下标，供结果输出按 UBPU 切分 */
		ubpu_offsets[u] = task_count;

		if (!ubpu->present || strlen(ubpu->src_eid) == 0)
			continue;

		for (i = 0; i < ubpu->intra_dst_num &&
			    task_count < max_tasks; i++) {
			struct probe_task *task = &tasks[task_count];

			task->task_id = task_count;
			snprintf(task->src_eid, MAX_EID_LEN, "%s",
				 ubpu->src_eid);
			snprintf(task->dst_eid, MAX_EID_LEN, "%s",
				 ubpu->intra_dst_eids[i]);
			task->packet_size = ubpu->packet_size;
			task->packet_count = ubpu->packet_count_intra;
			task->top_n = DEFAULT_COVERAGE_K;
			task->is_inter = false;
			task_count++;
		}

		for (i = 0; i < ubpu->inter_dst_num &&
			    task_count < max_tasks; i++) {
			struct probe_task *task = &tasks[task_count];

			task->task_id = task_count;
			snprintf(task->src_eid, MAX_EID_LEN, "%s",
				 ubpu->src_eid);
			snprintf(task->dst_eid, MAX_EID_LEN, "%s",
				 ubpu->inter_dst_eids[i]);
			task->packet_size = ubpu->packet_size;
			task->packet_count = ubpu->packet_count_inter;
			task->top_n = DEFAULT_COVERAGE_K;
			task->is_inter = true;
			task_count++;
		}
	}

	/* 任务总数（含最后一个 UBPU 的结束下标） */
	ubpu_offsets[node->ubpu_count < max_ubpu ? node->ubpu_count : max_ubpu] =
		task_count;

	return task_count;
}

static void create_probe_threads(const struct probe_task *tasks, int task_count,
				 struct probe_task_result *results)
{
	pthread_t threads[MAX_PROBE_TARGETS * 2];
	struct thread_arg args[MAX_PROBE_TARGETS * 2];
	bool created[MAX_PROBE_TARGETS * 2];
	int i;

	for (i = 0; i < task_count; i++) {
		int thread_ret;

		init_probe_task_result(&results[i]);
		args[i].task = (struct probe_task *)&tasks[i];
		args[i].result = &results[i];
		thread_ret = pthread_create(&threads[i], NULL,
					    probe_thread_func, &args[i]);
		created[i] = thread_ret == 0;
		if (!created[i]) {
			write_urma_ping_log_entry(&tasks[i], &results[i], "",
						  NULL, false, 0, 0, -1);
		}
	}

	/* 只 join 创建成功的线程，避免 join 未初始化线程句柄 */
	for (i = 0; i < task_count; i++) {
		if (created[i]) {
			int thread_ret = pthread_join(threads[i], NULL);

			if (thread_ret != 0) {
				init_probe_task_result(&results[i]);
				write_urma_ping_log_entry(&tasks[i], &results[i],
							  "", NULL, false, 0, 0, -1);
			}
		}
	}
}

/* ========================================================================
 * 结果输出（仅保留真实且样本充足的时延结果）
 * ======================================================================== */

static void write_intra_results(sh_json *ubpu_obj,
				const struct probe_task *tasks, int task_count,
				const struct probe_task_result *results)
{
	sh_json *dst_arr = sh_json_create_arr();
	sh_json *lat_arr = sh_json_create_arr();
	int i;

	for (i = 0; i < task_count; i++) {
		if (tasks[i].is_inter)
			continue;
		if (results[i].status != PROBE_EXEC_OK)
			continue;

		sh_json_push(dst_arr, sh_json_create_str(tasks[i].dst_eid));
		sh_json_push(lat_arr, sh_json_create_num(results[i].latency));
	}

	sh_json_attach(ubpu_obj, "intra_l1_dst_eids", dst_arr);
	sh_json_attach(ubpu_obj, "intra_l1_latencies", lat_arr);
}

static void write_inter_results(sh_json *ubpu_obj,
				const struct probe_task *tasks, int task_count,
				const struct probe_task_result *results)
{
	sh_json *dst_arr = sh_json_create_arr();
	sh_json *lat_arr = sh_json_create_arr();
	int i;

	for (i = 0; i < task_count; i++) {
		if (!tasks[i].is_inter)
			continue;
		if (results[i].status != PROBE_EXEC_OK)
			continue;

		sh_json_push(dst_arr, sh_json_create_str(tasks[i].dst_eid));
		sh_json_push(lat_arr, sh_json_create_num(results[i].latency));
	}

	sh_json_attach(ubpu_obj, "inter_l1_dst_eids", dst_arr);
	sh_json_attach(ubpu_obj, "inter_l1_latencies", lat_arr);
}

/* 输出单个本机节点的探测结果 JSON */
static sh_json *build_node_result_json(const struct node_probe_data *node,
				     const struct probe_task *tasks,
				     const struct probe_task_result *results,
				     int task_count, const int *ubpu_offsets)
{
	sh_json *node_obj = sh_json_create_obj();
	int u;

	(void)task_count; /* 使用 ubpu_offsets 确定任务范围 */

	if (!node_obj)
		return NULL;

	for (u = 0; u < node->ubpu_count; u++) {
		const struct ubpu_probe_data *ubpu = &node->ubpus[u];
		sh_json *ubpu_obj;
		char ubpu_key[16];
		int ubpu_task_start;
		int ubpu_task_count;

		if (!ubpu->present)
			continue;

		ubpu_obj = sh_json_create_obj();
		if (!ubpu_obj)
			continue;

		sh_json_put_num(ubpu_obj, "packet_count_intra",
					ubpu->packet_count_intra);
		sh_json_put_num(ubpu_obj, "packet_count_inter",
					ubpu->packet_count_inter);
		sh_json_put_num(ubpu_obj, "packet_size",
					ubpu->packet_size);
		sh_json_put_str(ubpu_obj, "src_eid", ubpu->src_eid);

		/* 仅输出该 UBPU 自身的探测任务结果 */
		ubpu_task_start = ubpu_offsets[u];
		ubpu_task_count = ubpu_offsets[u + 1] - ubpu_offsets[u];

		write_intra_results(ubpu_obj, tasks + ubpu_task_start,
				    ubpu_task_count, results + ubpu_task_start);
		write_inter_results(ubpu_obj, tasks + ubpu_task_start,
				    ubpu_task_count, results + ubpu_task_start);

		snprintf(ubpu_key, sizeof(ubpu_key), "%d", u);
		sh_json_attach(node_obj, ubpu_key, ubpu_obj);
	}

	return node_obj;
}

static void update_probe_exec_summary(struct probe_exec_summary *summary,
				      const struct probe_task_result *results,
				      int result_count)
{
	int i;

	for (i = 0; i < result_count; i++) {
		enum probe_exec_status status = results[i].status;

		if (status >= PROBE_EXEC_OK && status < PROBE_EXEC_STATUS_COUNT)
			summary->status_count[status]++;
		else
			summary->status_count[PROBE_EXEC_FAIL]++;
	}
}

static unsigned int probe_exec_summary_total(
	const struct probe_exec_summary *summary)
{
	unsigned int total = 0;
	int status;

	for (status = PROBE_EXEC_OK; status < PROBE_EXEC_STATUS_COUNT; status++)
		total += summary->status_count[status];
	return total;
}

static void print_probe_exec_summary(const struct probe_exec_summary *summary)
{
	fprintf(stdout,
		"[INFO] Probe execution summary: total=%u, ok=%u, "
		"timeout=%u, failed=%u.\n",
		probe_exec_summary_total(summary),
		summary->status_count[PROBE_EXEC_OK],
		summary->status_count[PROBE_EXEC_TIMEOUT],
		summary->status_count[PROBE_EXEC_FAIL]);
}

/* ========================================================================
 * 公开接口
 * ======================================================================== */

static int get_plan_coverage_k(sh_json *root, uint32_t *coverage_k)
{
	sh_json *config;
	sh_json *item;
	double value;

	if (root == NULL || coverage_k == NULL)
		return -EINVAL;

	config = sh_json_get_item_cs(root, "_probe_config");
	if (config == NULL || config->kind != SH_JSON_MAP)
		return -EINVAL;

	item = sh_json_get_item_cs(config, "coverage_k");
	if (item == NULL || item->kind != SH_JSON_NUM)
		return -EINVAL;

	value = item->dval;
	if (value < MIN_COVERAGE_K || value > MAX_COVERAGE_K ||
	    fabs(value - (double)item->ival) > DBL_EPSILON)
		return -EINVAL;

	*coverage_k = (uint32_t)item->ival;
	return 0;
}

static int sub_health_probe_execute_impl(const char *plan_file,
					 const char *result_file)
{
	char *content = NULL;
	sh_json *root = NULL;
	sh_json *result_root = NULL;
	struct node_probe_data *nodes = NULL;
	struct probe_exec_summary summary = {0};
	FILE *log_fp;
	int node_count = 0;
	int next_task_id = 0;
	uint32_t coverage_k;
	int ret = 0;
	int n;

	if (!plan_file || !result_file)
		return -EINVAL;

	/* 堆分配以避免超大栈帧（每节点 ~4MB，栈上限 8KB） */
	nodes = (struct node_probe_data *)calloc((size_t)MAX_NODE_NUM,
						 sizeof(struct node_probe_data));
	if (!nodes)
		return -ENOMEM;

	/* Step 0: 获取本机 IP 列表 */
	load_local_ips();
	load_slot_id_ips();

	/* Step 1: 读取并解析探测计划 */
	{
		FILE *fp;
		long fsize;
		size_t nread;

		fp = fopen(plan_file, "rb");
		if (!fp) {
			ret = -errno;
			goto cleanup;
		}

		fseek(fp, 0, SEEK_END);
		fsize = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		if (fsize <= 0 || fsize > MAX_JSON_BUF_SIZE) {
			fclose(fp);
			ret = -EINVAL;
			goto cleanup;
		}

		content = (char *)malloc((size_t)fsize + 1);
		if (!content) {
			fclose(fp);
			ret = -ENOMEM;
			goto cleanup;
		}

		nread = fread(content, 1, (size_t)fsize, fp);
		fclose(fp);
		if (nread != (size_t)fsize) {
			free(content);
			ret = -EIO;
			goto cleanup;
		}
		content[fsize] = '\0';
	}

	root = sh_json_parse(content);
	free(content);
	if (!root) {
		ret = -EINVAL;
		goto cleanup;
	}

	ret = get_plan_coverage_k(root, &coverage_k);
	if (ret != 0) {
		if (sh_json_get_item_cs(root, "_probe_config") == NULL) {
			report_invalid_probe_plan(plan_file);
		} else {
			fprintf(stderr,
				"[ERROR] Probe plan requires integer "
				"_probe_config.coverage_k in range [%d, %d].\n",
				MIN_COVERAGE_K, MAX_COVERAGE_K);
		}
		goto cleanup;
	}

	/* 校验输入确为探测对计划（拒绝拓扑/结果/检测等其它文件） */
	ret = check_probe_plan_file(root);
	if (ret != 0) {
		fprintf(stderr,
			"[ERROR] '%s' is not a valid probe plan file: "
			"missing _l1_maps or valid node UBPU probe pairs.\n",
			plan_file);
		goto cleanup;
	}

	/* 提取顶层 L1 映射表（供探测结果输出） */
	sh_json *plan_l1_maps = sh_json_get_item(root, "_l1_maps");

	/* 每次运行清空重写原始探测日志 */
	log_fp = fopen(URMA_PING_LOG_FILE, "w");
	if (log_fp)
		fclose(log_fp);

	/* Step 2: 遍历顶层 key，定位本机节点 */
	collect_probe_data(root, nodes, &node_count);

	if (node_count == 0) {
		sh_json_delete(root);
		ret = -ENOENT;
		goto cleanup;
	}

	/* Step 3-4: 填充任务并多线程并发执行探测 */
	result_root = sh_json_create_obj();
	if (!result_root) {
		sh_json_delete(root);
		ret = -ENOMEM;
		goto cleanup;
	}

	/* 将全局 L1 映射表写入结果顶层（供 Step 3 使用） */
	if (plan_l1_maps && plan_l1_maps->kind == SH_JSON_MAP) {
		sh_json *result_maps = sh_json_create_obj();

		if (result_maps) {
			for (sh_json *map = plan_l1_maps->head; map;
			     map = map->fwd) {
				if (!map->name || map->kind != SH_JSON_SEQ)
					continue;
				sh_json *arr = sh_json_create_arr();

				if (arr) {
					for (sh_json *e = map->head; e;
					     e = e->fwd) {
						if (e->kind != SH_JSON_BUF)
							continue;
						sh_json_push(arr,
							sh_json_create_str(
								e->str));
					}
					sh_json_attach(result_maps,
							      map->name, arr);
				}
			}
			sh_json_attach(result_root, "_l1_maps",
					      result_maps);
		}
	}

	for (n = 0; n < node_count; n++) {
		int max_tasks = MAX_PROBE_TARGETS * MAX_SUPPORTED_UBPU;
		struct probe_task *tasks;
		struct probe_task_result *results;
		int ubpu_offsets[MAX_SUPPORTED_UBPU + 1];
		int task_count;
		sh_json *node_obj;
		int i;

		tasks = (struct probe_task *)calloc((size_t)max_tasks,
						    sizeof(struct probe_task));
		results = (struct probe_task_result *)calloc((size_t)max_tasks,
							    sizeof(struct probe_task_result));
		if (!tasks || !results) {
			free(tasks);
			free(results);
			sh_json_delete(result_root);
			sh_json_delete(root);
			ret = -ENOMEM;
			goto cleanup;
		}

		task_count = populate_probe_tasks(&nodes[n], tasks, max_tasks,
						  ubpu_offsets,
						  MAX_SUPPORTED_UBPU);
		for (i = 0; i < task_count; i++) {
			tasks[i].task_id = next_task_id++;
			tasks[i].top_n = (int)coverage_k;
		}

		create_probe_threads(tasks, task_count, results);
		update_probe_exec_summary(&summary, results, task_count);

		node_obj = build_node_result_json(&nodes[n], tasks, results,
						  task_count, ubpu_offsets);
		if (node_obj)
			sh_json_attach(result_root, nodes[n].ip, node_obj);

		free(tasks);
		free(results);
	}

	sh_json_delete(root);
	root = NULL;
	print_probe_exec_summary(&summary);

	/* Step 5: 输出探测结果 */
	{
		char *json_str = sh_json_write(result_root);
		FILE *fp;

		sh_json_delete(result_root);
		if (!json_str) {
			ret = -ENOMEM;
			goto cleanup;
		}

		fp = fopen(result_file, "w");
		if (!fp) {
			free(json_str);
			ret = -errno;
			goto cleanup;
		}
		if (fwrite(json_str, 1, strlen(json_str), fp) != strlen(json_str)) {
			fclose(fp);
			free(json_str);
			unlink(result_file);
			ret = -EIO;
			goto cleanup;
		}
		fputc('\n', fp);
		fclose(fp);
		free(json_str);
	}

cleanup:
	free(nodes);
	return ret;
}

int sub_health_probe_execute(const char *plan_file,
			     const char *result_file)
{
	int ret_pipe[2];
	int child_status;
	int child_ret;
	int wait_ret;
	int ret;
	pid_t child_pid;

	if (plan_file == NULL || result_file == NULL)
		return -EINVAL;

	if (pipe(ret_pipe) != 0)
		return -errno;

	/*
	 * 避免 fork 后子进程继承尚未刷新的标准输出缓冲，
	 * 导致之前的日志被重复输出。
	 */
	(void)fflush(NULL);

	child_pid = fork();
	if (child_pid < 0) {
		ret = -errno;
		close(ret_pipe[0]);
		close(ret_pipe[1]);
		return ret;
	}

	if (child_pid == 0) {
		int write_ret;

		close(ret_pipe[0]);

		/*
		 * Step2 子进程成为进程组组长。
		 * 后续 popen 创建的 shell 和 urma_ping 会继承该进程组。
		 */
		if (setpgid(0, 0) != 0)
			child_ret = -errno;
		else
			child_ret = sub_health_probe_execute_impl(
				plan_file, result_file);

		/*
		 * _exit() 不刷新stdio，所以这里主动刷新，
		 * 确保探测汇总和错误日志能够输出。
		 */
		(void)fflush(NULL);

		write_ret = write_all(ret_pipe[1], &child_ret,
				      sizeof(child_ret));
		close(ret_pipe[1]);

		_exit(write_ret == 0 ? EXIT_SUCCESS : EXIT_FAILURE);
	}

	/* 父进程 */
	close(ret_pipe[1]);

	/*
	 * 父子进程都尝试设置进程组，消除父进程开始等待与
	 * 子进程执行 setpgid() 之间的竞争窗口。
	 * 真正的设置结果由子进程返回值负责检查。
	 */
	(void)setpgid(child_pid, child_pid);

	wait_ret = wait_child_with_timeout(child_pid,
					   PROBE_EXEC_WATCHDOG_SEC,
					   &child_status);
	if (wait_ret == 0) {
		fprintf(stderr,
			"[ERROR] Probe execution watchdog timed out "
			"after %u seconds.\n",
			PROBE_EXEC_WATCHDOG_SEC);

		terminate_probe_process_group(child_pid);
		close(ret_pipe[0]);

		/*
		 * 防止保留旧结果或读取到没有完整写完的结果文件。
		 */
		(void)unlink(result_file);
		return -ETIMEDOUT;
	}

	if (wait_ret < 0) {
		terminate_probe_process_group(child_pid);
		close(ret_pipe[0]);
		return wait_ret;
	}

	/*
	 * 子进程因信号终止，或者没有成功把真实返回值写入管道。
	 */
	if (!WIFEXITED(child_status) ||
	    WEXITSTATUS(child_status) != EXIT_SUCCESS) {
		/*
		 * 清理可能残留的 shell/urma_ping。
		 */
		(void)kill(-child_pid, SIGKILL);
		close(ret_pipe[0]);
		return -EIO;
	}

	ret = read_all(ret_pipe[0], &child_ret, sizeof(child_ret));
	close(ret_pipe[0]);
	if (ret != 0)
		return ret;

	return child_ret;
}
