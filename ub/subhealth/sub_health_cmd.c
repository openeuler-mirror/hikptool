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
#include "tool_cmd.h"
#include "tool_lib.h"

#include <errno.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

/* ========================================================================
 * 参数掩码与执行步骤
 * ======================================================================== */

#define PARAM_TOPOLOGY_MASK       0x01
#define PARAM_PLAN_MASK           0x02
#define PARAM_RESULT_MASK         0x04
#define PARAM_COVERAGE_MASK       0x08
#define PARAM_PACKET_SIZE_MASK    0x10
#define PARAM_TIME_THRESHOLD_MASK 0x20
#define PARAM_STEP_MASK           0x40

enum exec_step {
	STEP_FULL = 0,    /* 全流程 */
	STEP_PLAN = 1,    /* 仅探测规划 */
	STEP_EXECUTE = 2, /* 仅探测执行 */
	STEP_DETECT = 3,  /* 仅检测分析 */
};

/* 命令行参数 */
struct sub_health_args {
	char topology_file[MAX_PATH_LEN];
	char probe_plan_file[MAX_PATH_LEN];
	char result_file[MAX_PATH_LEN];
	uint32_t coverage_k;
	uint32_t packet_size;
	uint32_t time_threshold;
	uint32_t exec_step;
	uint32_t param_mask;
};

/* 执行上下文（spec §6） */
struct execute_ctx {
	struct major_cmd_ctrl *major_cmd;
	int ret;
	bool full_process;       /* step == 0 时为全流程 */
	bool auto_gen_topology;  /* 全流程且未指定 -t 时启用 */
	char temp_topology[MAX_PATH_LEN];
};

static const char fixed_plan_name[] = "probe_plan.json";
static const char fixed_result_name[] = "probe_result.json";
static const char fixed_output_name[] = "detection.json";

static struct sub_health_args g_args_info = { 0 };

/* ========================================================================
 * 参数记录回调
 * ======================================================================== */

static int cmd_help(struct major_cmd_ctrl *self, const char *argv)
{
	HIKP_SET_USED(argv);

	printf("\n  Usage: %s %s [options]\n",
	       get_tool_name(), self->cmd_ptr->name);
	printf("\n         %s\n", self->cmd_ptr->help_info);
	printf("  Options:\n\n");

	printf("    %s, %-25s %s\n", "-h", "--help",
	       "display this help and exit");
	printf("    %s, %-25s %s\n", "-t", "--topology=<file>",
	       "topology input (required for step1, optional in full)");
	printf("    %s, %-25s %s\n", "-p", "--probe-plan=<file>",
	       "probe plan input file (required for step2)");
	printf("    %s, %-25s %s\n", "-r", "--result=<file>",
	       "probe result input file (required for step3)");
	printf("    %s, %-25s %s\n", "-k", "--coverage=<num>",
	       "link coverage times [3-20], default 5; valid in step1/full");
	printf("    %s, %-25s %s\n", "-s", "--packet-size=<num>",
	       "packet size in bytes [4-4096], default 4096; valid in step1/full");
	printf("    %s, %-25s %s\n", "-T", "--time-threshold=<num>",
	       "latency threshold in ms, default 100; valid in step3/full");
	printf("    %s, %-25s %s\n", "-1", "--step1",
	       "execute step 1 only (probe planning), options: -t -k -s");
	printf("    %s, %-25s %s\n", "-2", "--step2",
	       "execute step 2 only (probe execution), options: -p");
	printf("    %s, %-25s %s\n", "-3", "--step3",
	       "execute step 3 only (fault detection), options: -r -T");

	printf("\n  Examples:\n\n");

	printf("    # Full pipeline with auto topology (electric link only)\n");
	printf("    %s %s\n", get_tool_name(), self->cmd_ptr->name);

	printf("    # Full pipeline with a topology file\n");
	printf("    %s %s -t topology.json\n",
	       get_tool_name(), self->cmd_ptr->name);

	printf("    # Full pipeline with tuning\n");
	printf("    %s %s -t topology.json -k 10 -s 4096 -T 1000\n",
	       get_tool_name(), self->cmd_ptr->name);

	printf("    # Step 1 only: probe planning (options: -t -k -s)\n");
	printf("    %s %s -1 -t topology.json -k 5 -s 4096\n",
	       get_tool_name(), self->cmd_ptr->name);

	printf("    # Step 2 only: probe execution (options: -p)\n");
	printf("    %s %s -2 -p probe_plan.json\n",
	       get_tool_name(), self->cmd_ptr->name);

	printf("    # Step 3 only: fault detection (options: -r -T)\n");
	printf("    %s %s -3 -r probe_result.json -T 1000\n",
	       get_tool_name(), self->cmd_ptr->name);

	printf("\n  Outputs are always written to the current directory:\n");
	printf("    probe_plan.json, probe_result.json, detection.json\n");
	printf("\n");

	return 0;
}

static int cmd_set_step1(struct major_cmd_ctrl *self, const char *argv)
{
	HIKP_SET_USED(argv);

	if ((g_args_info.param_mask & PARAM_STEP_MASK) != 0) {
		snprintf(self->err_str, sizeof(self->err_str),
			 "Step option already set.");
		self->err_no = -EINVAL;
		return -EINVAL;
	}

	g_args_info.param_mask |= PARAM_STEP_MASK;
	g_args_info.exec_step = STEP_PLAN;
	return 0;
}

static int cmd_set_step2(struct major_cmd_ctrl *self, const char *argv)
{
	HIKP_SET_USED(argv);

	if ((g_args_info.param_mask & PARAM_STEP_MASK) != 0) {
		snprintf(self->err_str, sizeof(self->err_str),
			 "Step option already set.");
		self->err_no = -EINVAL;
		return -EINVAL;
	}

	g_args_info.param_mask |= PARAM_STEP_MASK;
	g_args_info.exec_step = STEP_EXECUTE;
	return 0;
}

static int cmd_set_step3(struct major_cmd_ctrl *self, const char *argv)
{
	HIKP_SET_USED(argv);

	if ((g_args_info.param_mask & PARAM_STEP_MASK) != 0) {
		snprintf(self->err_str, sizeof(self->err_str),
			 "Step option already set.");
		self->err_no = -EINVAL;
		return -EINVAL;
	}

	g_args_info.param_mask |= PARAM_STEP_MASK;
	g_args_info.exec_step = STEP_DETECT;
	return 0;
}

static int copy_path_arg(struct major_cmd_ctrl *self, const char *arg,
			 char *out, size_t out_size, const char *arg_name)
{
	size_t len;

	if (self == NULL || arg == NULL || out == NULL || out_size == 0 ||
	    arg_name == NULL)
		return -EINVAL;

	if (arg[0] == '\0') {
		snprintf(self->err_str, sizeof(self->err_str),
			 "%s path cannot be empty.", arg_name);
		self->err_no = -EINVAL;
		return -EINVAL;
	}

	len = strlen(arg);
	if (len >= out_size) {
		snprintf(self->err_str, sizeof(self->err_str),
			 "%s path is too long (maximum %zu characters).",
			 arg_name, out_size - 1);
		self->err_no = -ENAMETOOLONG;
		return -ENAMETOOLONG;
	}

	memcpy(out, arg, len + 1);
	return 0;
}

/* 输入文件存在性检查：stat + access */
static int check_input_exists(struct major_cmd_ctrl *self, const char *path,
			      const char *arg_name)
{
	struct stat st;

	if (stat(path, &st) != 0) {
		snprintf(self->err_str, sizeof(self->err_str),
			 "%s input file '%s' does not exist.", arg_name, path);
		self->err_no = -ENOENT;
		return -ENOENT;
	}

	if (!S_ISREG(st.st_mode) || access(path, R_OK) != 0) {
		snprintf(self->err_str, sizeof(self->err_str),
			 "%s input file '%s' is not a readable file.",
			 arg_name, path);
		self->err_no = -EACCES;
		return -EACCES;
	}

	return 0;
}

static int cmd_set_topology(struct major_cmd_ctrl *self, const char *argv)
{
	int ret;

	ret = copy_path_arg(self, argv, g_args_info.topology_file,
			    sizeof(g_args_info.topology_file), "Topology");
	if (ret != 0)
		return ret;

	g_args_info.param_mask |= PARAM_TOPOLOGY_MASK;
	return 0;
}

static int cmd_set_plan(struct major_cmd_ctrl *self, const char *argv)
{
	int ret;

	ret = copy_path_arg(self, argv, g_args_info.probe_plan_file,
			    sizeof(g_args_info.probe_plan_file), "Probe plan");
	if (ret != 0)
		return ret;

	g_args_info.param_mask |= PARAM_PLAN_MASK;
	return 0;
}

static int cmd_set_result(struct major_cmd_ctrl *self, const char *argv)
{
	int ret;

	ret = copy_path_arg(self, argv, g_args_info.result_file,
			    sizeof(g_args_info.result_file), "Probe result");
	if (ret != 0)
		return ret;

	g_args_info.param_mask |= PARAM_RESULT_MASK;
	return 0;
}

static int cmd_set_coverage(struct major_cmd_ctrl *self, const char *argv)
{
	char *endptr = NULL;
	uint64_t val;

	val = strtoul(argv, &endptr, 0);
	if (endptr <= argv || *endptr != '\0' ||
	    val < MIN_COVERAGE_K || val > MAX_COVERAGE_K) {
		snprintf(self->err_str, sizeof(self->err_str),
			 "Coverage must be in range [%d, %d].",
			 MIN_COVERAGE_K, MAX_COVERAGE_K);
		self->err_no = -EINVAL;
		return -EINVAL;
	}

	g_args_info.coverage_k = (uint32_t)val;
	g_args_info.param_mask |= PARAM_COVERAGE_MASK;
	return 0;
}

static int cmd_set_packet_size(struct major_cmd_ctrl *self, const char *argv)
{
	char *endptr = NULL;
	uint64_t val;

	val = strtoul(argv, &endptr, 0);
	if (endptr <= argv || *endptr != '\0' || val < 4 || val > 4096) {
		snprintf(self->err_str, sizeof(self->err_str),
			 "Packet size must be in range [4, 4096].");
		self->err_no = -EINVAL;
		return -EINVAL;
	}

	g_args_info.packet_size = (uint32_t)val;
	g_args_info.param_mask |= PARAM_PACKET_SIZE_MASK;
	return 0;
}

static int cmd_set_time_threshold(struct major_cmd_ctrl *self,
				  const char *argv)
{
	char *endptr = NULL;
	uint64_t val;

	errno = 0;
	val = strtoul(argv, &endptr, 0);
	if (endptr <= argv || *endptr != '\0' || errno == ERANGE ||
	    val > UINT32_MAX) {
		snprintf(self->err_str, sizeof(self->err_str),
			 "Invalid time threshold.");
		self->err_no = -EINVAL;
		return -EINVAL;
	}

	g_args_info.time_threshold = (uint32_t)val;
	g_args_info.param_mask |= PARAM_TIME_THRESHOLD_MASK;
	return 0;
}

/* ========================================================================
 * 默认值填充
 * ======================================================================== */

static void apply_defaults(void)
{
	if (!(g_args_info.param_mask & PARAM_COVERAGE_MASK))
		g_args_info.coverage_k = DEFAULT_COVERAGE_K;
	if (!(g_args_info.param_mask & PARAM_PACKET_SIZE_MASK))
		g_args_info.packet_size = DEFAULT_PACKET_SIZE;
	if (!(g_args_info.param_mask & PARAM_TIME_THRESHOLD_MASK))
		g_args_info.time_threshold = THRESHOLD_TIME_DEFAULT;
}

/* ========================================================================
 * 参数校验
 * ======================================================================== */

static int set_usage_error(struct major_cmd_ctrl *self,
			   const char *reason, const char *usage)
{
	snprintf(self->err_str, sizeof(self->err_str),
		 "%s Usage: %s %s %s", reason, get_tool_name(),
		 self->cmd_ptr->name, usage);
	self->err_no = -EINVAL;
	return -EINVAL;
}

static int validate_args(struct major_cmd_ctrl *self)
{
	uint32_t step = g_args_info.exec_step;
	uint32_t mask = g_args_info.param_mask;

	switch (step) {
	case STEP_FULL:
		if (mask & (PARAM_PLAN_MASK | PARAM_RESULT_MASK))
			return set_usage_error(
				self, "Invalid option for full process.",
				"[-t <topology_file>] [-k num] [-s num] [-T num].");
		break;
	case STEP_PLAN:
		if (mask & (PARAM_PLAN_MASK | PARAM_RESULT_MASK |
			    PARAM_TIME_THRESHOLD_MASK))
			return set_usage_error(
				self, "Invalid option for Step 1.",
				"-1 -t <topology_file> [-k num] [-s num].");
		if (!(mask & PARAM_TOPOLOGY_MASK))
			return set_usage_error(
				self, "Step 1 requires -t.",
				"-1 -t <topology_file> [-k num] [-s num].");
		break;
		case STEP_EXECUTE:
			if (mask & (PARAM_TOPOLOGY_MASK |
					PARAM_RESULT_MASK |
					PARAM_COVERAGE_MASK |
					PARAM_PACKET_SIZE_MASK |
					PARAM_TIME_THRESHOLD_MASK))
				return set_usage_error(
					self, "Invalid option for Step 2.",
					"-2 -p <probe_plan_file>.");
		if (!(mask & PARAM_PLAN_MASK))
			return set_usage_error(
				self, "Step 2 requires -p.",
				"-2 -p <probe_plan_file>.");
		break;
	case STEP_DETECT:
		if (mask & (PARAM_TOPOLOGY_MASK | PARAM_PLAN_MASK |
			    PARAM_COVERAGE_MASK | PARAM_PACKET_SIZE_MASK))
			return set_usage_error(
				self, "Invalid option for Step 3.",
				"-3 -r <probe_result_file> [-T num].");
		if (!(mask & PARAM_RESULT_MASK))
			return set_usage_error(
				self, "Step 3 requires -r.",
				"-3 -r <probe_result_file> [-T num].");
		break;
	default:
		return set_usage_error(self, "Invalid execution step.",
				       "-t <topology_file>.");
	}

	return 0;
}

/* ========================================================================
 * 全流程编排
 * ======================================================================== */

/* 全流程编排参数（输出始终为 CWD 固定文件名） */
static void setup_execute_params(struct execute_ctx *ctx)
{
	ctx->full_process = (g_args_info.exec_step == STEP_FULL);

	if (!ctx->full_process)
		return;

	/*
	 * 全流程必须使用本次流水线产生的固定中间文件：
	 * Step 2 读取 Step 1 输出的 probe_plan.json；
	 * Step 3 读取 Step 2 输出的 probe_result.json。
	 * 因此即使命令行传入 -p/-r，也不能改变全流程内部的数据串联。
	 */
	snprintf(g_args_info.probe_plan_file,
		 sizeof(g_args_info.probe_plan_file), "%s", fixed_plan_name);
	snprintf(g_args_info.result_file,
		 sizeof(g_args_info.result_file), "%s", fixed_result_name);

	ctx->auto_gen_topology =
		((g_args_info.param_mask & PARAM_TOPOLOGY_MASK) == 0);
	if (ctx->auto_gen_topology) {
		snprintf(ctx->temp_topology, sizeof(ctx->temp_topology),
			 "topology.json");
	}
}

/* 获取当前工作目录用于回显 */
static const char *get_cwd_str(char *buf, size_t size)
{
	if (getcwd(buf, size) == NULL)
		snprintf(buf, size, ".");
	return buf;
}

/* 覆盖前警告 */
static void warn_if_overwrite(const char *path)
{
	if (access(path, F_OK) == 0)
		HIKP_WARN_PRINT("%s already exists. It will be overwritten.\n",
				path);
}

static void run_step1(struct execute_ctx *ctx)
{
	PERF_DECLARE(step1);
	const char *topology_file = g_args_info.topology_file;
	char cwd_buf[MAX_PATH_LEN];
	const char *cwd = get_cwd_str(cwd_buf, sizeof(cwd_buf));
	int ret;

	/* 全流程且未指定 -t：自动生成拓扑（仅电组网）。 */
	if (ctx->full_process && ctx->auto_gen_topology) {
		printf("[INFO] Generating topology from RESTCONF API...\n");
		ret = generate_topology_from_restconf(ctx->temp_topology);
		if (ret != 0) {
			snprintf(ctx->major_cmd->err_str,
				 sizeof(ctx->major_cmd->err_str),
				 "Failed to generate topology: %d", ret);
			ctx->major_cmd->err_no = ret;
			ctx->ret = ret;
			return;
		}
		topology_file = ctx->temp_topology;
	} else {
		ret = check_input_exists(ctx->major_cmd, topology_file,
					 "Topology");
		if (ret != 0) {
			ctx->ret = ret;
			return;
		}
	}

	printf("[INFO] Generating probe plan (k=%u)...\n",
	       g_args_info.coverage_k);

	PERF_START(step1);
	ctx->ret = sub_health_probe_plan(topology_file,
					 g_args_info.coverage_k,
					 g_args_info.packet_size,
					 fixed_plan_name);
	PERF_END(step1);

	if (ctx->ret != 0) {
		snprintf(ctx->major_cmd->err_str,
			 sizeof(ctx->major_cmd->err_str),
			 "Probe planning failed: %d", ctx->ret);
		ctx->major_cmd->err_no = ctx->ret;
		return;
	}

	warn_if_overwrite(fixed_plan_name);
	PERF_PRINT(step1, "Probe planning");
	printf("[INFO] Probe plan written to: %s/%s\n",
	       cwd, fixed_plan_name);

	/* 自动生成的拓扑文件在 Step 1 完成后删除。 */
	if (ctx->full_process && ctx->auto_gen_topology)
		unlink(ctx->temp_topology);
}

static void run_step2(struct execute_ctx *ctx)
{
	PERF_DECLARE(step2);
	const char *plan_in = g_args_info.probe_plan_file;
	char cwd_buf[MAX_PATH_LEN];
	const char *cwd = get_cwd_str(cwd_buf, sizeof(cwd_buf));

	/* 检查输入文件存在性 */
	if (check_input_exists(ctx->major_cmd, plan_in, "Probe plan") != 0) {
		ctx->ret = -ENOENT;
		return;
	}

	printf("[INFO] Executing probes...\n");

	PERF_START(step2);
	ctx->ret = sub_health_probe_execute(plan_in, fixed_result_name);
	PERF_END(step2);

	if (ctx->ret != 0) {
		snprintf(ctx->major_cmd->err_str,
			 sizeof(ctx->major_cmd->err_str),
			 "Probe execution failed: %d", ctx->ret);
		ctx->major_cmd->err_no = ctx->ret;
		return;
	}

	warn_if_overwrite(fixed_result_name);
	PERF_PRINT(step2, "Probe execution");
	printf("[INFO] Probe results written to: %s/%s\n",
	       cwd, fixed_result_name);
}

static void run_step3(struct execute_ctx *ctx)
{
	PERF_DECLARE(step3);
	const char *result_in = g_args_info.result_file;
	char cwd_buf[MAX_PATH_LEN];
	const char *cwd = get_cwd_str(cwd_buf, sizeof(cwd_buf));

	/* 检查输入文件存在性 */
	if (check_input_exists(ctx->major_cmd, result_in, "Probe result") != 0) {
		ctx->ret = -ENOENT;
		return;
	}

	printf("[INFO] Detecting faults (threshold=%ums)...\n",
	       g_args_info.time_threshold);

	PERF_START(step3);
	ctx->ret = sub_health_detect(result_in, fixed_output_name,
				     g_args_info.time_threshold);
	PERF_END(step3);

	if (ctx->ret != 0) {
		snprintf(ctx->major_cmd->err_str,
			 sizeof(ctx->major_cmd->err_str),
			 "Fault detection failed: %d", ctx->ret);
		ctx->major_cmd->err_no = ctx->ret;
		return;
	}

	warn_if_overwrite(fixed_output_name);
	PERF_PRINT(step3, "Fault detection");
	printf("[INFO] Detection results written to: %s/%s\n",
	       cwd, fixed_output_name);
	printf("[INFO] Diagnostic log written to: "
	       "%s/sub_health_detect.log\n", cwd);
}

static void sub_health_execute(struct execute_ctx *ctx)
{
	PERF_DECLARE(total);

	PERF_START(total);

	if (g_args_info.exec_step == STEP_FULL ||
	    g_args_info.exec_step == STEP_PLAN)
		run_step1(ctx);
	if (ctx->ret != 0)
		return;

	if (g_args_info.exec_step == STEP_FULL ||
	    g_args_info.exec_step == STEP_EXECUTE)
		run_step2(ctx);
	if (ctx->ret != 0)
		return;

	if (g_args_info.exec_step == STEP_FULL ||
	    g_args_info.exec_step == STEP_DETECT)
		run_step3(ctx);
	if (ctx->ret != 0)
		return;

	PERF_END(total);
	PERF_PRINT(total, "Total pipeline");
}

/* ========================================================================
 * 命令入口
 * ======================================================================== */

static void cmd_execute(struct major_cmd_ctrl *self)
{
	struct execute_ctx ctx;

	memset(&ctx, 0, sizeof(ctx));
	ctx.major_cmd = self;

	apply_defaults();

	if (validate_args(self) != 0)
		return;

	setup_execute_params(&ctx);
	sub_health_execute(&ctx);
}

/* ========================================================================
 * 初始化
 * ======================================================================== */

static void cmd_sub_health_init(void)
{
	struct major_cmd_ctrl *major_cmd = get_major_cmd();

	memset(&g_args_info, 0, sizeof(g_args_info));

	major_cmd->option_count = 0;
	major_cmd->execute = cmd_execute;

	cmd_option_register("-h", "--help", false, cmd_help);
	cmd_option_register("-t", "--topology", true, cmd_set_topology);
	cmd_option_register("-p", "--probe-plan", true, cmd_set_plan);
	cmd_option_register("-r", "--result", true, cmd_set_result);
	cmd_option_register("-k", "--coverage", true, cmd_set_coverage);
	cmd_option_register("-s", "--packet-size", true, cmd_set_packet_size);
	cmd_option_register("-T", "--time-threshold", true,
			    cmd_set_time_threshold);
	cmd_option_register("-1", "--step1", false, cmd_set_step1);
	cmd_option_register("-2", "--step2", false, cmd_set_step2);
	cmd_option_register("-3", "--step3", false, cmd_set_step3);
}

HIKP_CMD_DECLARE("sub_health",
		 "network sub-health detection and fault localization.",
		 cmd_sub_health_init);
