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

#include "hikp_ub_link.h"

static struct ub_link_param g_ub_link_param = { 0 };

static int hikp_ub_link_help(struct major_cmd_ctrl *self, const char *argv)
{
	HIKP_SET_USED(argv);

	printf("\n  Usage: %s %s\n", self->cmd_ptr->name, "-i <interface>\n");
	printf("\n         %s\n", self->cmd_ptr->help_info);
	printf("  Options:\n\n");
	printf("    %s, %-25s %s\n", "-h", "--help", "display this help and exit");
	printf("    %s, %-25s %s\n", "-i", "--interface=<interface>",
	       "device target or bdf id, e.g. ubn0 or 0000:35:00.0");
	printf("\n");
	return 0;
}

static int hikp_ub_link_target(struct major_cmd_ctrl *self, const char *argv)
{
	self->err_no = tool_check_and_get_valid_bdf_id(argv, &(g_ub_link_param.target));
	if (self->err_no != 0) {
		snprintf(self->err_str, sizeof(self->err_str), "Unknown device %s.", argv);
		return self->err_no;
	}

	return 0;
}

static void hikp_ub_link_info_show(const struct ub_link_rsp *info)
{
	printf("%-32s %u\n", "mac id:", info->mac_id);
	printf("%-32s %s\n", "hdlc link status:", info->hdlc_link ? "true" : "false");
	printf("%-32s %s\n", "hpcs link status:", info->hpcs_link ? "true" : "false");
	printf("%-32s 0x%08x\n", "hdlc_link_fsm register value:", info->hdlc_link_fsm);
	printf("%-32s 0x%08x\n", "hpcs_link_fsm register value:", info->hpcs_link_fsm);
}

static int hikp_ub_query_link_info(const struct bdf_t *bdf)
{
	struct hikp_cmd_header header = { 0 };
	struct ub_link_req_paras req = { 0 };
	struct hikp_cmd_ret *cmd_ret;
	struct ub_link_rsp *rsp;

	req.bdf = *bdf;
	hikp_cmd_init(&header, UB_MOD, GET_UB_LINK_INFO_CMD, UB_LINK_INFO_DUMP);
	cmd_ret = hikp_cmd_alloc(&header, &req, sizeof(req));
	if (cmd_ret == NULL || cmd_ret->status != 0) {
		hikp_cmd_free(&cmd_ret);
		return -EIO;
	}

	rsp = (struct ub_link_rsp *)cmd_ret->rsp_data;
	hikp_ub_link_info_show(rsp);

	hikp_cmd_free(&cmd_ret);
	return 0;
}

static void hikp_ub_link_cmd_execute(struct major_cmd_ctrl *self)
{
	struct bdf_t *bdf = &g_ub_link_param.target.bdf;
	int ret;

	ret = hikp_ub_query_link_info(bdf);
	if (ret != 0) {
		snprintf(self->err_str, sizeof(self->err_str), "fail to get link info.");
		self->err_no = ret;
		return;
	}
}

static void cmd_ub_link_init(void)
{
	struct major_cmd_ctrl *major_cmd = get_major_cmd();

	major_cmd->option_count = 0;
	major_cmd->execute = hikp_ub_link_cmd_execute;

	cmd_option_register("-h", "--help", false, hikp_ub_link_help);
	cmd_option_register("-i", "--interface", true, hikp_ub_link_target);
}

HIKP_CMD_DECLARE("ub_link", "get ub link information", cmd_ub_link_init);
