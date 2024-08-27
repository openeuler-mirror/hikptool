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

#include "hikp_roce_rst.h"

static struct cmd_roce_rst_param g_roce_rst_param = { 0 };

int hikp_roce_set_rst_bdf(char *nic_name)
{
	return tool_check_and_get_valid_bdf_id(nic_name,
					       &g_roce_rst_param.target);
}

static int hikp_roce_rst_help(struct major_cmd_ctrl *self, const char *argv)
{
	printf("\n  Usage: %s %s\n", self->cmd_ptr->name, "-i <interface>\n");
	printf("\n         %s\n", self->cmd_ptr->help_info);
	printf("  Options:\n\n");
	printf("    %s, %-25s %s\n", "-h", "--help", "display this help and exit");
	printf("    %s, %-25s %s\n", "-i", "--interface=<interface>", "device target, e.g. eth0");
	printf("\n");

	return 0;
}

static int hikp_roce_rst_target(struct major_cmd_ctrl *self, const char *argv)
{
	self->err_no = tool_check_and_get_valid_bdf_id(argv, &(g_roce_rst_param.target));
	if (self->err_no)
		snprintf(self->err_str, sizeof(self->err_str), "Unknown device %s.\n", argv);

	return self->err_no;
}

static int hikp_roce_rst_get_data(struct hikp_cmd_ret **cmd_ret,
				  uint32_t block_id,
				  struct roce_ext_reg_name *reg_name)
{
	struct hikp_cmd_header req_header = { 0 };
	struct roce_rst_req_param req_data;
	uint32_t req_size;
	int ret;

	req_data.bdf = g_roce_rst_param.target.bdf;
	req_data.block_id = block_id;

	req_size = sizeof(struct roce_rst_req_param);
	hikp_cmd_init(&req_header, ROCE_MOD, GET_ROCEE_RST_CMD,
		      g_roce_rst_param.sub_cmd);
	*cmd_ret = hikp_cmd_alloc(&req_header, &req_data, req_size);
	ret = hikp_rsp_normal_check(*cmd_ret);
	if (ret)
		printf("hikptool roce_rst cmd_ret malloc failed, sub_cmd = %u, ret = %d.\n",
			g_roce_rst_param.sub_cmd, ret);

	return ret;
}

void hikp_roce_rst_execute(struct major_cmd_ctrl *self)
{
	hikp_roce_ext_execute(self, GET_ROCEE_RST_CMD, hikp_roce_rst_get_data);
}

static void cmd_roce_rst_init(void)
{
	struct major_cmd_ctrl *major_cmd = get_major_cmd();

	major_cmd->option_count = 0;
	major_cmd->execute = hikp_roce_rst_execute;

	cmd_option_register("-h", "--help", false, hikp_roce_rst_help);
	cmd_option_register("-i", "--interface", true, hikp_roce_rst_target);
}

HIKP_CMD_DECLARE("roce_rst", "get roce_rst registers information", cmd_roce_rst_init);
