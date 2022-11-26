/*
 * Copyright (c) 2022 Hisilicon Technologies Co., Ltd.
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

#include "hikp_roce_pkt.h"

static struct cmd_roce_pkt_param_t g_roce_pkt_param_t = { 0 };

static int hikp_roce_pkt_help(struct major_cmd_ctrl *self, const char *argv)
{
	printf("\n  Usage: %s %s\n", self->cmd_ptr->name, "-i <interface>\n");
	printf("\n         %s\n", self->cmd_ptr->help_info);
	printf("  Options:\n\n");
	printf("    %s, %-25s %s\n", "-h", "--help", "display this help and exit");
	printf("    %s, %-25s %s\n", "-i", "--interface=<interface>", "device target, e.g. eth0");
	printf("    %s, %-25s %s\n", "-c", "--clear=<clear>", "clear param count registers");
	printf("\n");

	return 0;
}

static int hikp_roce_pkt_target(struct major_cmd_ctrl *self, const char *argv)
{
	self->err_no = tool_check_and_get_valid_bdf_id(argv, &(g_roce_pkt_param_t.target));
	if (self->err_no !=  0) {
		snprintf(self->err_str, sizeof(self->err_str), "Unknown device %s.", argv);
		return self->err_no;
	}

	return 0;
}

static int hikp_roce_pkt_clear_set(struct major_cmd_ctrl *self, const char *argv)
{
	g_roce_pkt_param_t.reset_flag = 1;
	return 0;
}

static int hikp_roce_pkt_get_data(struct hikp_cmd_ret **cmd_ret, struct roce_pkt_req_param req_data)
{
	struct hikp_cmd_header req_header = { 0 };

	hikp_cmd_init(&req_header, ROCE_MOD, GET_ROCEE_PKT_CMD, 0);
	*cmd_ret = hikp_cmd_alloc(&req_header, &req_data, sizeof(req_data));
	if (*cmd_ret == NULL) {
		printf("hikptool roce_pkt cmd_ret malloc failed\n");
		return -EIO;
	}

	return 0;
}

static void hikp_roce_pkt_print(uint32_t total_block_num,
				const uint32_t *offset, const uint32_t *data)
{
	uint32_t i;

	printf("**************PKT INFO*************\n");
	for (i = 0; i < total_block_num; i++)
		printf("[0x%08X] : 0x%08X\n", offset[i], data[i]);
	printf("***********************************\n");
}

static void hikp_roce_pkt_execute(struct major_cmd_ctrl *self)
{
	struct roce_pkt_req_param req_data = { 0 };
	struct roce_pkt_res_param *roce_pkt_res;
	struct hikp_cmd_ret *cmd_ret;
	int ret;

	req_data.bdf = g_roce_pkt_param_t.target.bdf;
	req_data.reset_flag = g_roce_pkt_param_t.reset_flag;
	ret = hikp_roce_pkt_get_data(&cmd_ret, req_data);
	if (ret < 0) {
		self->err_no = ret;
		return;
	}
	roce_pkt_res = (struct roce_pkt_res_param *)cmd_ret->rsp_data;
	hikp_roce_pkt_print(roce_pkt_res->total_block_num,
			    roce_pkt_res->reg_data.offset, roce_pkt_res->reg_data.data);

	free(cmd_ret);
	cmd_ret = NULL;
}

static void cmd_roce_pkt_init(void)
{
	struct major_cmd_ctrl *major_cmd = get_major_cmd();

	major_cmd->option_count = 0;
	major_cmd->execute = hikp_roce_pkt_execute;

	cmd_option_register("-h", "--help", false, hikp_roce_pkt_help);
	cmd_option_register("-i", "--interface", true, hikp_roce_pkt_target);
	cmd_option_register("-c", "--clear", false, hikp_roce_pkt_clear_set);
}

HIKP_CMD_DECLARE("roce_pkt", "get or clear roce_pkt registers information", cmd_roce_pkt_init);
