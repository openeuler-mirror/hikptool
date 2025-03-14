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

#include "hikp_roce_caep.h"

static struct cmd_roce_caep_param_t g_roce_caep_param_t = { 0 };

int hikp_roce_set_caep_bdf(char *nic_name)
{
	return tool_check_and_get_valid_bdf_id(nic_name,
					       &g_roce_caep_param_t.target);
}

void hikp_roce_set_caep_mode(uint32_t mode)
{
	g_roce_caep_param_t.sub_cmd = mode;
}

static int hikp_roce_caep_help(struct major_cmd_ctrl *self, const char *argv)
{
	HIKP_SET_USED(argv);

	printf("\n  Usage: %s %s\n", self->cmd_ptr->name, "-i <interface>\n");
	printf("\n         %s\n", self->cmd_ptr->help_info);
	printf("  Options:\n\n");
	printf("    %s, %-25s %s\n", "-h", "--help", "display this help and exit");
	printf("    %s, %-25s %s\n", "-i", "--interface=<interface>", "device target, e.g. eth0");
	printf("    %s, %-25s %s\n", "-e", "--extend", "query extend caep registers");
	printf("\n");

	return 0;
}

static int hikp_roce_caep_target(struct major_cmd_ctrl *self, const char *argv)
{
	self->err_no = tool_check_and_get_valid_bdf_id(argv, &(g_roce_caep_param_t.target));
	if (self->err_no !=  0) {
		snprintf(self->err_str, sizeof(self->err_str), "Unknown device %s.\n", argv);
		return self->err_no;
	}

	return 0;
}

static int hikp_roce_caep_get_data(struct hikp_cmd_ret **cmd_ret,
				   uint32_t block_id,
				   struct roce_ext_reg_name *reg_name)
{
	struct roce_caep_req_param_ext req_data_ext;
	struct hikp_cmd_header req_header = { 0 };
	uint32_t req_size;
	int ret;

	HIKP_SET_USED(reg_name);

	req_data_ext.origin_param.bdf = g_roce_caep_param_t.target.bdf;
	req_data_ext.block_id = block_id;

	req_size = (g_roce_caep_param_t.sub_cmd == CAEP_ORIGIN) ?
		   sizeof(struct roce_caep_req_param) :
		   sizeof(struct roce_caep_req_param_ext);
	hikp_cmd_init(&req_header, ROCE_MOD, GET_ROCEE_CAEP_CMD,
		      g_roce_caep_param_t.sub_cmd);
	*cmd_ret = hikp_cmd_alloc(&req_header, &req_data_ext, req_size);
	ret = hikp_rsp_normal_check(*cmd_ret);
	if (ret)
		printf("hikptool roce_caep cmd_ret malloc failed, sub_cmd = %u, ret = %d.\n",
			g_roce_caep_param_t.sub_cmd, ret);

	return ret;
}

static void hikp_roce_caep_print(uint32_t total_block_num,
				 const uint32_t *offset, const uint32_t *data)
{
	uint32_t i;

	printf("**************CAEP INFO*************\n");
	for (i = 0; i < total_block_num; i++)
		printf("[0x%08X] : 0x%08X\n", offset[i], data[i]);
	printf("************************************\n");
}

static void hikp_roce_caep_execute_origin(struct major_cmd_ctrl *self)
{
	struct roce_caep_res_param *roce_caep_res;
	struct hikp_cmd_ret *cmd_ret;

	self->err_no = hikp_roce_caep_get_data(&cmd_ret, 0, NULL);
	if (self->err_no) {
		printf("hikptool roce_caep get data failed.\n");
		goto exec_error;
	}

	roce_caep_res = (struct roce_caep_res_param *)cmd_ret->rsp_data;
	if (roce_caep_res->total_block_num > ROCE_HIKP_CAEP_REG_NUM) {
		printf("version might not match, adjust the reg num to %d.\n",
		       ROCE_HIKP_CAEP_REG_NUM);
		roce_caep_res->total_block_num = ROCE_HIKP_CAEP_REG_NUM;
	}

	hikp_roce_caep_print(roce_caep_res->total_block_num,
			     roce_caep_res->reg_data.offset,
			     roce_caep_res->reg_data.data);

exec_error:
	hikp_cmd_free(&cmd_ret);
}

void hikp_roce_caep_execute(struct major_cmd_ctrl *self)
{
	if (g_roce_caep_param_t.sub_cmd == CAEP_ORIGIN)
		hikp_roce_caep_execute_origin(self);
	else
		hikp_roce_ext_execute(self, GET_ROCEE_CAEP_CMD,
				      hikp_roce_caep_get_data);
}

static int hikp_roce_caep_ext_set(struct major_cmd_ctrl *self, const char *argv)
{
	HIKP_SET_USED(self);
	HIKP_SET_USED(argv);

	g_roce_caep_param_t.sub_cmd = CAEP_EXT;

	return 0;
}

static void cmd_roce_caep_init(void)
{
	struct major_cmd_ctrl *major_cmd = get_major_cmd();

	major_cmd->option_count = 0;
	major_cmd->execute = hikp_roce_caep_execute;

	cmd_option_register("-h", "--help", false, hikp_roce_caep_help);
	cmd_option_register("-i", "--interface", true, hikp_roce_caep_target);
	cmd_option_register("-e", "--extend", false, hikp_roce_caep_ext_set);
}

HIKP_CMD_DECLARE("roce_caep", "get roce_caep registers information", cmd_roce_caep_init);
