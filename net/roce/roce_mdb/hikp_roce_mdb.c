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

#include <unistd.h>
#include "hikp_roce_mdb.h"

#define ROCE_MDB_CMD_CLEAR HI_BIT(0)
static struct cmd_roce_mdb_param g_roce_mdb_param = { 0 };

static int hikp_roce_mdb_help(struct major_cmd_ctrl *self, const char *argv)
{
	printf("\n  Usage: %s %s\n", self->cmd_ptr->name, "-i <interface>\n");
	printf("\n         %s\n", self->cmd_ptr->help_info);
	printf("  Options:\n\n");
	printf("    %s, %-25s %s\n", "-h", "--help", "display this help and exit");
	printf("    %s, %-25s %s\n", "-i", "--interface=<interface>", "device target, e.g. eth0");
	printf("    %s, %-25s %s\n", "-c", "--clear=<clear>", "clear mdb registers");
	printf("\n");

	return 0;
}

static int hikp_roce_mdb_target(struct major_cmd_ctrl *self, const char *argv)
{
	self->err_no = tool_check_and_get_valid_bdf_id(argv, &(g_roce_mdb_param.target));
	if (self->err_no) {
		snprintf(self->err_str, sizeof(self->err_str), "Unknown device %s.", argv);
		return self->err_no;
	}

	return 0;
}

static int hikp_roce_mdb_clear_set(struct major_cmd_ctrl *self, const char *argv)
{
	g_roce_mdb_param.reset_flag = ROCE_MDB_CMD_CLEAR;
	return 0;
}

static void hikp_roce_mdb_print(uint32_t reg_num, struct roce_mdb_rsp_data *mdb_rsp)
{
	uint32_t i;

	printf("**************MDB INFO*************\n");
	printf("addr_offset      :         reg_data\n");
	for (i = 0; i < reg_num; i++)
		printf("0x%08X : 0x%08X\n", mdb_rsp->reg_offset[i], mdb_rsp->reg_data[i]);
	printf("***********************************\n");
}

static int hikp_roce_mdb_show(struct major_cmd_ctrl *self)
{
	struct roce_mdb_req_para req_data = { 0 };
	struct roce_mdb_rsp_data *mdb_rsp = NULL;
	struct hikp_cmd_header req_header = { 0 };
	struct hikp_cmd_ret *cmd_ret = NULL;
	uint32_t reg_num;
	int ret;

	req_data.bdf = g_roce_mdb_param.target.bdf;
	if (g_roce_mdb_param.reset_flag)
		hikp_cmd_init(&req_header, ROCE_MOD, GET_ROCEE_MDB_CMD, MDB_CLEAR);
	else
		hikp_cmd_init(&req_header, ROCE_MOD, GET_ROCEE_MDB_CMD, MDB_SHOW);

	cmd_ret = hikp_cmd_alloc(&req_header, &req_data, sizeof(req_data));
	ret = hikp_rsp_normal_check(cmd_ret);
	if (ret != 0)
		goto exec_error;

	reg_num = cmd_ret->rsp_data_num / ROCE_HIKP_REG_SWICTH;
	if (reg_num != ROCE_HIKP_MDB_REG_NUM) {
		printf("version might not match.\n");
		ret = -1;
		goto exec_error;
	}

	mdb_rsp = (struct roce_mdb_rsp_data *)(cmd_ret->rsp_data);
	hikp_roce_mdb_print(reg_num, mdb_rsp);
	ret = 0;

exec_error:
	free(cmd_ret);
	cmd_ret = NULL;
	return ret;
}

static void hikp_roce_mdb_execute(struct major_cmd_ctrl *self)
{
	self->err_no = hikp_roce_mdb_show(self);
	if (self->err_no)
		return;

	if (g_roce_mdb_param.reset_flag)
		printf("clear roce_mdb reg success.\n");
	else
		printf("show roce_mdb reg success.\n");
}

static void cmd_roce_mdb_init(void)
{
	struct major_cmd_ctrl *major_cmd = get_major_cmd();

	major_cmd->option_count = 0;
	major_cmd->execute = hikp_roce_mdb_execute;

	cmd_option_register("-h", "--help", false, hikp_roce_mdb_help);
	cmd_option_register("-i", "--interface", true, hikp_roce_mdb_target);
	cmd_option_register("-c", "--clear", false, hikp_roce_mdb_clear_set);
}

HIKP_CMD_DECLARE("roce_mdb", "get or clear roce_mdb registers information", cmd_roce_mdb_init);
