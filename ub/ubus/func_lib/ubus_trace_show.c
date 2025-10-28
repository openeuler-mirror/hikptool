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

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <inttypes.h>
#include "hikptdev_plug.h"
#include "ubus_common.h"
#include "ubus_trace_show.h"

static int trace_rsp_data_check(struct hikp_cmd_ret *cmd_ret)
{
	uint32_t trace_num;
	int ret;

	ret = hikp_rsp_normal_check(cmd_ret);
	if (ret) {
		printf("dump registers failed, ret: %d.\n", ret);
		return ret;
	}

	if (cmd_ret->rsp_data_num == 0) {
		printf("ub trace show without rsp data.\n");
		return -EINVAL;
	}
	/* 0: First uint32_t is trace num received from TF */
	trace_num = cmd_ret->rsp_data[0];
#define TRACER_DEPTH 64
	if (trace_num > TRACER_DEPTH || trace_num == 0) {
		printf("trace num is over range or zero, check failed.\n");
		return -EINVAL;
	}

	if ((cmd_ret->rsp_data_num - 1) * sizeof(uint32_t) != trace_num * sizeof(struct ubus_trace_data)) {
		printf("rsp data number check failed, rsp_data_num: %u, trace_num: %u.\n",
			cmd_ret->rsp_data_num, trace_num);
		return -EINVAL;
	}

	return 0;
}

static void trace_info_print(struct hikp_cmd_ret *cmd_ret)
{
	struct ubus_trace_data *ltssm_val;
	uint32_t trace_num;

	printf("ub tracer:\n");
#define TRACE_DATA_INDEX 1
	printf("trace detail:\n");
	trace_num = cmd_ret->rsp_data[0];
	ltssm_val = (struct ubus_trace_data *)(cmd_ret->rsp_data + TRACE_DATA_INDEX);
	for (uint32_t i = 0; i < trace_num; i++) {
		printf("\tltssm[%02u]: 0x%" PRIx64 "\n", i, *(uint64_t *)(ltssm_val + i));
	}
}

int ubus_trace_show_execute(uint32_t port_id)
{
	struct ubus_trace_show_req_para req_param = { 0 };
	struct hikp_cmd_header req_header = { 0 };
	struct hikp_cmd_ret *cmd_ret;
	int ret;

	hikp_cmd_init(&req_header, UBUS_MOD, UBUS_TRACE_SHOW, TRACE_SHOW);
	req_param.port_id = port_id;

	cmd_ret = hikp_cmd_alloc(&req_header, &req_param, sizeof(req_param));
	ret = trace_rsp_data_check(cmd_ret);
	if (ret)
		goto free_cmd_ret;

	trace_info_print(cmd_ret);

free_cmd_ret:
	hikp_cmd_free(&cmd_ret);

	return ret;
}
