/*
 * Copyright (c) 2023 Hisilicon Technologies Co., Ltd.
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

#include "hikp_ub_bp.h"

static struct ub_bp_param g_ub_bp_param = { 0 };

static int hikp_ub_bp_help(struct major_cmd_ctrl *self, const char *argv)
{
	printf("\n  Usage: %s %s\n", self->cmd_ptr->name, "-i <interface>\n");
	printf("\n         %s\n", self->cmd_ptr->help_info);
	printf("  Options:\n\n");
	printf("    %s, %-25s %s\n", "-h", "--help", "display this help and exit");
	printf("    %s, %-25s %s\n", "-i", "--interface=<interface>",
	       "device target or bdf id, e.g. ubn0 or 0000:35:00.0");
	printf("\n");
	return 0;
}

static int hikp_ub_bp_target(struct major_cmd_ctrl *self, const char *argv)
{
	self->err_no = tool_check_and_get_valid_bdf_id(argv, &(g_ub_bp_param.target));
	if (self->err_no) {
		snprintf(self->err_str, sizeof(self->err_str), "Unknown device %s.", argv);
		return self->err_no;
	}
	return 0;
}

static void hikp_ub_bp_info_show(const struct ub_bp_rsp *info)
{
	int bp_val_offset;

	printf("%-28s : %hhu\n", "mac id", info->mac_id);
	printf("%-28s : ", "back pressure status");

	for (bp_val_offset = MAX_VL_NUM; bp_val_offset >= 0; bp_val_offset--) {
		printf("%d", HI_GETBIT(info->bp_val, bp_val_offset) ? 1 : 0);
	}
	printf("\n");
}

static int hikp_ub_query_bp(const struct bdf_t *bdf)
{
	struct hikp_cmd_header header = { 0 };
	struct ub_bp_req_para req = { 0 };
	struct hikp_cmd_ret *cmd_ret;
	struct ub_bp_rsp *rsp;

	req.bdf = *bdf;
	hikp_cmd_init(&header, UB_MOD, GET_UB_BP_INFO_CMD, UB_BP_INFO_DUMP);
	cmd_ret = hikp_cmd_alloc(&header, &req, sizeof(req));
	if (cmd_ret == NULL || cmd_ret->status != 0) {
		free(cmd_ret);
		cmd_ret = NULL;
		return -EIO;
	}

	rsp = (struct ub_bp_rsp *)cmd_ret->rsp_data;
	hikp_ub_bp_info_show(rsp);

	free(cmd_ret);
	cmd_ret = NULL;
	return 0;
}

static void hikp_ub_bp_cmd_execute(struct major_cmd_ctrl *self)
{
	struct bdf_t *bdf = &g_ub_bp_param.target.bdf;
	int ret;

	ret = hikp_ub_query_bp(bdf);
	if (ret != 0) {
		snprintf(self->err_str, sizeof(self->err_str), "fail to get ub bp info.");
		self->err_no = ret;
		return;
	}
}

static void cmd_ub_bp_init(void)
{
	struct major_cmd_ctrl *major_cmd = get_major_cmd();

	major_cmd->option_count = 0;
	major_cmd->execute = hikp_ub_bp_cmd_execute;

	cmd_option_register("-h", "--help", false, hikp_ub_bp_help);
	cmd_option_register("-i", "--interface", true, hikp_ub_bp_target);
}

HIKP_CMD_DECLARE("ub_bp", "get ub bp information", cmd_ub_bp_init);
