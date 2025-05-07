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

#include "hikp_core_ring.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <inttypes.h>
#include "tool_cmd.h"
#include "hikptdev_plug.h"

static uint32_t g_cmd_param_mask = {0};

static int hikp_core_ring_help(struct major_cmd_ctrl *self, const char *argv)
{
	HIKP_SET_USED(argv);

	printf("\n  Usage: %s %s\n", self->cmd_ptr->name, "-d");
	printf("\n         %s\n", self->cmd_ptr->help_info);
	printf("  Options:\n\n");
	printf("    %s, %-25s %s\n", "-h", "--help", "display this help and exit");
	printf("    %s, %-25s %s\n", "-d", "--dump", "dump the ring info of the cpu core");
	printf("\n");

	return 0;
}

static int hikp_core_ring_get_info(struct major_cmd_ctrl *self, const char *argv)
{
	HIKP_SET_USED(self);
	HIKP_SET_USED(argv);

	g_cmd_param_mask |= PARAM_DUMP_MASK;

	return 0;
}

static void hikp_core_ring_print_info(const struct core_ring_info *ring_info)
{
	uint32_t cnt = 0;

	for (uint32_t chip = 0; chip < ring_info->chip_num; chip++) {
		printf("chip %u core ring:\n", chip);
		for (uint32_t cluster = 0; cluster < ring_info->per_cluster_num; cluster++) {
			if (cnt >= RING_DATA_MAX)
				break;

			printf("\tcluster%u ring info: 0x%" PRIx64 "\n",
			       cluster, ring_info->ring_data[cnt++]);
		}
	}
}

static void hikp_core_ring_dump(struct major_cmd_ctrl *self)
{
	struct hikp_cmd_header req_header = {0};
	struct core_ring_req cmd_req = {0};
	struct hikp_cmd_ret *cmd_ret;

	hikp_cmd_init(&req_header, CORE_RING_MOD, CORE_RING_DUMP, RING_INFO_DUMP);
	cmd_ret = hikp_cmd_alloc(&req_header, &cmd_req, sizeof(cmd_req));
	self->err_no = hikp_rsp_normal_check(cmd_ret);
	if (self->err_no != 0) {
		snprintf(self->err_str, sizeof(self->err_str), "get core ring info failed.");
		hikp_cmd_free(&cmd_ret);
		return;
	}

	hikp_core_ring_print_info((struct core_ring_info *)cmd_ret->rsp_data);

	hikp_cmd_free(&cmd_ret);
}

static void hikp_core_ring_cmd_execute(struct major_cmd_ctrl *self)
{
	if ((g_cmd_param_mask & PARAM_DUMP_MASK) == 0) {
		snprintf(self->err_str, sizeof(self->err_str), "Need input -d param!");
		self->err_no = -EINVAL;
		return;
	}

	hikp_core_ring_dump(self);
}

static void cmd_core_ring_info_init(void)
{
	struct major_cmd_ctrl *major_cmd = get_major_cmd();

	major_cmd->option_count = 0;
	major_cmd->execute = hikp_core_ring_cmd_execute;

	cmd_option_register("-h", "--help", false, hikp_core_ring_help);
	cmd_option_register("-d", "--dump", false, hikp_core_ring_get_info);
}

HIKP_CMD_DECLARE("cpu_ring", "dump cpu core ring info.", cmd_core_ring_info_init);
