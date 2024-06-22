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

static struct cmd_roce_mdb_param g_roce_mdb_param = { 0 };

static int hikp_roce_mdb_help(struct major_cmd_ctrl *self, const char *argv)
{
	printf("\n  Usage: %s %s\n", self->cmd_ptr->name, "-i <interface>\n");
	printf("\n         %s\n", self->cmd_ptr->help_info);
	printf("  Options:\n\n");
	printf("    %s, %-25s %s\n", "-h", "--help", "display this help and exit");
	printf("    %s, %-25s %s\n", "-i", "--interface=<interface>", "device target, e.g. eth0");
	printf("    %s, %-25s %s\n", "-c", "--clear=<clear>", "clear mdb registers");
	printf("    %s, %-25s %s\n", "-e", "--extend", "query extend mdb registers");
	printf("\n");

	return 0;
}

static int hikp_roce_mdb_target(struct major_cmd_ctrl *self, const char *argv)
{
	self->err_no = tool_check_and_get_valid_bdf_id(argv, &(g_roce_mdb_param.target));
	if (self->err_no) {
		snprintf(self->err_str, sizeof(self->err_str), "Unknown device %s.\n", argv);
		return self->err_no;
	}

	return 0;
}

static int hikp_roce_mdb_clear_set(struct major_cmd_ctrl *self, const char *argv)
{
	g_roce_mdb_param.flag |= ROCE_MDB_CMD_CLEAR;

	return 0;
}

/* DON'T change the order of this array or add entries between! */
static const char *g_mdb_reg_name[] = {
	"ROCEE_DWQE_WQE_ISSUE_CNT",
	"ROCEE_DWQE_WQE_EXEC_CNT",
	"ROCEE_DWQE_WQE_DROP_CNT",
	"ROCEE_DWQE_SQDB_ISSUE_CNT",
	"ROCEE_DWQE_SQDB_EXEC_CNT",
	"ROCEE_MBX_ISSUE_CNT",
	"ROCEE_MBX_EXEC_CNT",
	"ROCEE_DB_ISSUE_CNT",
	"ROCEE_DB_EXEC_CNT",
	"ROCEE_EQDB_ISSUE_CNT",
	"MDB_ALM",
	"ROCEE_MDB_EMPTY",
	"ROCEE_MDB_FULL",
	"MDB_STA_0",
	"MDB_STA_1",
	"MDB_STA_2",
	"MDB_MEM_INIT_DONE",
	"ROCEE_MDB_ECC_ERR",
	"ROCEE_MDB_ECC_ERR_INFO",
	"MDB_STA_3",
	"MDB_STA_4",
	"MDB_STA_5",
};

static void hikp_roce_mdb_print(uint32_t reg_num, struct roce_mdb_rsp_data *mdb_rsp)
{
	uint8_t arr_len = HIKP_ARRAY_SIZE(g_mdb_reg_name);
	uint32_t i;

	printf("**************MDB INFO*************\n");
	printf("%-40s[addr_offset] : reg_data\n", "reg_name");
	for (i = 0; i < reg_num; i++)
		printf("%-40s[0x%08X] : 0x%08X\n",
		       i < arr_len ? g_mdb_reg_name[i] : "",
		       mdb_rsp->reg_offset[i], mdb_rsp->reg_data[i]);
	printf("***********************************\n");
}

static int hikp_roce_mdb_get_data(struct hikp_cmd_ret **cmd_ret,
				  uint32_t block_id)
{
	struct roce_mdb_req_param_ext req_data_ext;
	struct hikp_cmd_header req_header = { 0 };
	uint32_t req_size;
	int ret;

	req_data_ext.origin_param.bdf = g_roce_mdb_param.target.bdf;
	req_data_ext.block_id = block_id;

	req_size = (g_roce_mdb_param.flag & ROCE_MDB_CMD_EXT) ?
		   sizeof(struct roce_mdb_req_param_ext) :
		   sizeof(struct roce_mdb_req_para);
	hikp_cmd_init(&req_header, ROCE_MOD, GET_ROCEE_MDB_CMD,
		      g_roce_mdb_param.sub_cmd);
	*cmd_ret = hikp_cmd_alloc(&req_header, &req_data_ext, req_size);
	ret = hikp_rsp_normal_check(*cmd_ret);
	if (ret)
		printf("hikptool roce_mdb cmd_ret malloc failed, sub_cmd = %u, ret = %d.\n",
			g_roce_mdb_param.sub_cmd, ret);

	return ret;
}

static void hikp_roce_mdb_execute_origin(struct major_cmd_ctrl *self)
{
	struct roce_mdb_rsp_data *mdb_rsp = NULL;
	struct hikp_cmd_ret *cmd_ret = NULL;
	uint32_t reg_num;

	self->err_no = hikp_roce_mdb_get_data(&cmd_ret, 0);
	if (self->err_no) {
		printf("hikptool roce_mdb get data failed\n");
		goto exec_error;
	}

	reg_num = cmd_ret->rsp_data_num / ROCE_HIKP_REG_SWICTH;
	if (reg_num != ROCE_HIKP_MDB_REG_NUM) {
		printf("version might not match.\n");
		self->err_no = -EPROTO;
		goto exec_error;
	}

	mdb_rsp = (struct roce_mdb_rsp_data *)(cmd_ret->rsp_data);
	hikp_roce_mdb_print(reg_num, mdb_rsp);

exec_error:
	if (cmd_ret)
		free(cmd_ret);
}

static void hikp_roce_mdb_execute(struct major_cmd_ctrl *self)
{
	if (g_roce_mdb_param.flag & ROCE_MDB_CMD_EXT) {
		g_roce_mdb_param.sub_cmd = (g_roce_mdb_param.flag & ROCE_MDB_CMD_CLEAR) ?
					   MDB_CLEAR_EXT : MDB_EXT;
		hikp_roce_ext_execute(self, GET_ROCEE_MDB_CMD,
				      hikp_roce_mdb_get_data);
	} else {
		g_roce_mdb_param.sub_cmd = (g_roce_mdb_param.flag & ROCE_MDB_CMD_CLEAR) ?
					   MDB_CLEAR : MDB_SHOW;
		hikp_roce_mdb_execute_origin(self);
	}
}

static int hikp_roce_mdb_ext_set(struct major_cmd_ctrl *self, const char *argv)
{
	g_roce_mdb_param.flag |= ROCE_MDB_CMD_EXT;

	return 0;
}

static void cmd_roce_mdb_init(void)
{
	struct major_cmd_ctrl *major_cmd = get_major_cmd();

	major_cmd->option_count = 0;
	major_cmd->execute = hikp_roce_mdb_execute;

	cmd_option_register("-h", "--help", false, hikp_roce_mdb_help);
	cmd_option_register("-i", "--interface", true, hikp_roce_mdb_target);
	cmd_option_register("-c", "--clear", false, hikp_roce_mdb_clear_set);
	cmd_option_register("-e", "--extend", false, hikp_roce_mdb_ext_set);
}

HIKP_CMD_DECLARE("roce_mdb", "get or clear roce_mdb registers information", cmd_roce_mdb_init);
