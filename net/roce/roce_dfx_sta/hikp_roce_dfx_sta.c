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

#include "hikp_roce_dfx_sta.h"

static struct cmd_roce_dfx_sta_param_t g_roce_dfx_sta_param_t = { 0 };

int hikp_roce_set_dfx_sta_bdf(char *nic_name)
{
	return tool_check_and_get_valid_bdf_id(nic_name,
					       &g_roce_dfx_sta_param_t.target);
}

static int hikp_roce_dfx_sta_help(struct major_cmd_ctrl *self, const char *argv)
{
	HIKP_SET_USED(argv);

	printf("\n  Usage: %s %s\n", self->cmd_ptr->name, "-i <interface>\n");
	printf("\n         %s\n", self->cmd_ptr->help_info);
	printf("  Options:\n\n");
	printf("    %s, %-25s %s\n", "-h", "--help", "display this help and exit");
	printf("    %s, %-25s %s\n", "-i", "--interface=<interface>", "device target, e.g. eth0");
	printf("    %s, %-25s %s\n", "-c", "--clear=<clear>", "clear param count registers");
	printf("\n");

	return 0;
}

static int hikp_roce_dfx_sta_target(struct major_cmd_ctrl *self, const char *argv)
{
	self->err_no = tool_check_and_get_valid_bdf_id(argv, &(g_roce_dfx_sta_param_t.target));
	if (self->err_no != 0)
		snprintf(self->err_str, sizeof(self->err_str), "Unknown device %s.", argv);

	return self->err_no;
}

static int hikp_roce_dfx_sta_clear_set(struct major_cmd_ctrl *self, const char *argv)
{
	HIKP_SET_USED(self);
	HIKP_SET_USED(argv);

	g_roce_dfx_sta_param_t.reset_flag = 1;
	return 0;
}

/* DON'T change the order of this array or add entries between! */
static const char *g_dfx_sta_reg_name[] = {
	"PKT_RNR_STA",
	"PKT_RTY_STA",
	"MSN_RTY_STA",
};

static int hikp_roce_dfx_sta_get_data(struct hikp_cmd_ret **cmd_ret,
				      uint32_t block_id,
				      struct roce_ext_reg_name *reg_name)
{
	struct hikp_cmd_header req_header = { 0 };
	struct roce_dfx_sta_req_param req_data;
	uint32_t req_size;
	int ret;

	reg_name->reg_name = g_dfx_sta_reg_name;
	reg_name->arr_len = HIKP_ARRAY_SIZE(g_dfx_sta_reg_name);

	req_data.reset_flag = g_roce_dfx_sta_param_t.reset_flag;
	req_data.bdf = g_roce_dfx_sta_param_t.target.bdf;
	req_data.block_id = block_id;

	req_size = sizeof(struct roce_dfx_sta_req_param);
	hikp_cmd_init(&req_header, ROCE_MOD, GET_ROCEE_DFX_STA_CMD, 0);
	*cmd_ret = hikp_cmd_alloc(&req_header, &req_data, req_size);
	ret = hikp_rsp_normal_check(*cmd_ret);
	if (ret)
		printf("hikptool roce_dfx_sta get cmd data failed, ret: %d\n", ret);

	return ret;
}

void hikp_roce_dfx_sta_execute(struct major_cmd_ctrl *self)
{
	hikp_roce_ext_execute(self, GET_ROCEE_DFX_STA_CMD, hikp_roce_dfx_sta_get_data);
}

static void cmd_roce_dfx_sta_init(void)
{
	struct major_cmd_ctrl *major_cmd = get_major_cmd();

	major_cmd->option_count = 0;
	major_cmd->execute = hikp_roce_dfx_sta_execute;

	cmd_option_register("-h", "--help", false, hikp_roce_dfx_sta_help);
	cmd_option_register("-i", "--interface", true, hikp_roce_dfx_sta_target);
	cmd_option_register("-c", "--clear", false, hikp_roce_dfx_sta_clear_set);
}

HIKP_CMD_DECLARE("roce_dfx_sta", "get or clear RoCE dfx statistics", cmd_roce_dfx_sta_init);
