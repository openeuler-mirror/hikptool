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
#include "hikp_roce_qmm.h"

static struct cmd_roce_qmm_param_t g_roce_qmm_param = { 0 };

static int hikp_roce_qmm_help(struct major_cmd_ctrl *self, const char *argv)
{
	printf("\n  Usage: %s %s\n", self->cmd_ptr->name, "-i <interface>\n");
	printf("\n         %s\n", self->cmd_ptr->help_info);
	printf("  Options:\n\n");
	printf("    %s, %-25s %s\n", "-h", "--help", "display this help and exit");
	printf("    %s, %-25s %s\n", "-i", "--interface=<interface>", "device target, e.g. eth0");
	printf("    %s, %-25s %s\n", "-b", "--bank=<bank>",
	       "[option]bank number, e.g. 0~7. (default 0)");
	printf("    %s, %-25s %s\n", "-e", "--extend", "query extend qmm registers");
	printf("\n");

	return 0;
}

static int hikp_roce_qmm_target(struct major_cmd_ctrl *self, const char *argv)
{
	self->err_no = tool_check_and_get_valid_bdf_id(argv, &(g_roce_qmm_param.target));
	if (self->err_no != 0) {
		snprintf(self->err_str, sizeof(self->err_str), "Unknown device %s.\n", argv);
		return self->err_no;
	}

	return 0;
}

static int hikp_roce_qmm_bank_get(struct major_cmd_ctrl *self, const char *argv)
{
	char *endptr = NULL;
	uint64_t bank_num;

	bank_num = strtoul(argv, &endptr, 0);
	if ((endptr <= argv) || (*endptr != '\0') || bank_num > QMM_BANK_NUM) {
		snprintf(self->err_str, sizeof(self->err_str), "Invalid bank number!\n");
		self->err_no = -EINVAL;
		return -EINVAL;
	}

	g_roce_qmm_param.bank_id = (uint32_t)bank_num;
	return 0;
}

/* DON'T change the order of these arrays or add entries between! */
static const char *g_qmm_top_reg_name[] = {
	"ROCEE_QMM_SRQC_ALM",
	"ROCEE_QMM_MPT_ALM",
	"ROCEE_QMM_ECC_ERR",
	"QMM_AXI_RESP_ERR",
	"ROCEE_LPRC_RO",
	"ROCEE_LPRC_RC",
	"QMM_LPRC_EMPTY_RD",
	"QPC_DMAE_EMPTY_RD",
	"QMM_LPRC_FULL_WR",
	"QPC_DMAE_FULL_WR",
	"ROCEE_QMM_QPC_ALM",
	"ROCEE_QMM_GMV_ALM",
};

static const char *g_qmm_cqc_reg_name[] = {
	"ROCEE_CQC_SRH_REQ_RO_BK0",
	"ROCEE_CQC_SRH_REQ_RO_BK1",
	"ROCEE_CQC_ECC_ERR",
	"ROCEE_CQC_RESP_ERR",
	"CQC_RW_REQ_RO_BK0",
	"CQC_RW_REQ_RO_BK1",
	"ROCEE_QMM_CQC_ALM",
};

static const char *g_qmm_qpc_reg_name[] = {
	"QMM_QPC_SRH_CNT_0",
	"QMM_QPC_SRH_CNT_1",
	"ROCEE_QPC_EMPTY_RD",
	"ROCEE_QPC_FULL_WR",
	"ROCEE_QPC_SRH_REQ_RO_0",
	"ROCEE_QPC_SRH_REQ_RO_1",
	"QMM_QPC_CLR_CNT0_0",
	"QMM_QPC_CLR_CNT1_0",
	"QMM_QPC_CLR_CNT2_0",
	"QMM_QPC_CLR_CNT3_0",
	"QMM_QPC_CLR_CNT0_1",
	"QMM_QPC_CLR_CNT1_1",
	"QMM_QPC_CLR_CNT2_1",
	"QMM_QPC_CLR_CNT3_1",
	"QPC_RW_REQ_RO_0",
	"QPC_RW_REQ_RO_1",
	"QPC_WQE_ECC_ERR",
};

static const char *g_qmm_top_ext_reg_name[] = {
	"ROCEE_QMM_SRQC_CACHE_RO",
	"ROCEE_QMM_SRQC_DMAE_RO",
	"ROCEE_QMM_SRQC_SRH_DFX",
	"ROCEE_QMM_SRQC_SRH_REQ_RO",
	"ROCEE_QMM_SRQC_RW_REQ_RO",
	"ROCEE_QMM_MPT_CACHE_RO",
	"ROCEE_QMM_MPT_DMAE_RO",
	"ROCEE_QMM_MPT_SRH_DFX",
	"ROCEE_QMM_MPT_SRH_REQ_RO",
	"QPC_DMAE_INTF_RO",
};

static const char *g_qmm_cqc_ext_reg_name[] = {
	"ROCEE_CQC_SRH_DFX_BK0",
	"ROCEE_CQC_SRH_DFX_BK1",
	"ROCEE_CQC_EMPTY_RD",
	"ROCEE_CQC_FULL_WR",
	"ROCEE_CQC_CACHE_RO_BK0",
	"ROCEE_CQC_CACHE_RO_BK1",
	"ROCEE_CQC_DMAE_RO",
};

static const char *g_qmm_qpc_ext_reg_name[] = {
	"ROCEE_QPC_SRH_DFX_0",
	"ROCEE_QPC_SRH_DFX_1",
	"ROCEE_QPC_SRH_REQ_RO_0",
	"ROCEE_QPC_SRH_REQ_RO_1",
	"ROCEE_QMM_QPC_CACHE_RO_0",
	"ROCEE_QMM_QPC_CACHE_RO_0",
	"QMM_WQE_CACHE_RO",
	"IRRL_CACHE_RO",
};

static const struct reg_name_info {
	enum roce_qmm_cmd_type sub_cmd;
	const char **reg_name;
	uint8_t arr_len;
} g_qmm_reg_name_info_table[] = {
	{QMM_SHOW_CQC, g_qmm_cqc_reg_name, HIKP_ARRAY_SIZE(g_qmm_cqc_reg_name)},
	{QMM_SHOW_QPC, g_qmm_qpc_reg_name, HIKP_ARRAY_SIZE(g_qmm_qpc_reg_name)},
	{QMM_SHOW_TOP, g_qmm_top_reg_name, HIKP_ARRAY_SIZE(g_qmm_top_reg_name)},
	{QMM_SHOW_CQC_EXT, g_qmm_cqc_ext_reg_name, HIKP_ARRAY_SIZE(g_qmm_cqc_ext_reg_name)},
	{QMM_SHOW_QPC_EXT, g_qmm_qpc_ext_reg_name, HIKP_ARRAY_SIZE(g_qmm_qpc_ext_reg_name)},
	{QMM_SHOW_TOP_EXT, g_qmm_top_ext_reg_name, HIKP_ARRAY_SIZE(g_qmm_top_ext_reg_name)},
};

static void hikp_roce_qmm_print(struct roce_qmm_rsp_data *qmm_rsp)
{
	const char **reg_name;
	uint32_t index = 0;
	uint8_t arr_len;

	for (index = 0; index < HIKP_ARRAY_SIZE(g_qmm_reg_name_info_table); index++) {
		if (g_qmm_reg_name_info_table[index].sub_cmd != g_roce_qmm_param.sub_cmd)
			continue;
		arr_len = g_qmm_reg_name_info_table[index].arr_len;
		reg_name = g_qmm_reg_name_info_table[index].reg_name;
		break;
	}

	if (index == HIKP_ARRAY_SIZE(g_qmm_reg_name_info_table)) {
		printf("can't find reg name table for roce_qmm sub_cmd %u.\n",
		       g_roce_qmm_param.sub_cmd);
		return;
	}

	printf("**************QMM %s INFO*************\n",
	       g_roce_qmm_param.sub_name);
	printf("%-40s[addr_offset] : reg_data\n", "reg_name");
	index = 0;
	while (index < qmm_rsp->reg_num) {
		printf("%-40s[0x%08X] : 0x%08X\n",
		       index < arr_len ? reg_name[index] : "",
		       qmm_rsp->qmm_content[index][0],
		       qmm_rsp->qmm_content[index][1]);
		index++;
	}
	printf("***************************************\n");
}

static int hikp_roce_qmm_get_data(struct hikp_cmd_ret **cmd_ret,
				  uint32_t block_id,
				  struct roce_ext_reg_name *reg_name)
{
	struct roce_qmm_req_para_ext req_data_ext;
	struct hikp_cmd_header req_header = { 0 };
	uint32_t req_size;
	int ret, i;

	if (reg_name) {
		for (i = 0; i < HIKP_ARRAY_SIZE(g_qmm_reg_name_info_table); i++) {
			if (g_qmm_reg_name_info_table[i].sub_cmd != g_roce_qmm_param.sub_cmd)
				continue;
			reg_name->arr_len = g_qmm_reg_name_info_table[i].arr_len;
			reg_name->reg_name = g_qmm_reg_name_info_table[i].reg_name;
			break;
		}

		if (i == HIKP_ARRAY_SIZE(g_qmm_reg_name_info_table))
			return -EINVAL;
	}

	req_data_ext.origin_param.bdf = g_roce_qmm_param.target.bdf;
	req_data_ext.origin_param.bank_id = g_roce_qmm_param.bank_id;
	req_data_ext.block_id = block_id;

	req_size = g_roce_qmm_param.ext_flag ?
		   sizeof(struct roce_qmm_req_para) :
		   sizeof(struct roce_qmm_req_para_ext);
	hikp_cmd_init(&req_header, ROCE_MOD, GET_ROCEE_QMM_CMD,
		      g_roce_qmm_param.sub_cmd);
	*cmd_ret = hikp_cmd_alloc(&req_header, &req_data_ext, req_size);
	ret = hikp_rsp_normal_check(*cmd_ret);
	if (ret)
		printf("hikptool roce_qmm cmd_ret malloc failed, sub_cmd = %u, ret = %d.\n",
			g_roce_qmm_param.sub_cmd, ret);

	return ret;
}

static void hikp_roce_qmm_execute_origin(struct major_cmd_ctrl *self)
{
	struct roce_qmm_rsp_data *roce_qmm_res;
	struct hikp_cmd_ret *cmd_ret;

	self->err_no = hikp_roce_qmm_get_data(&cmd_ret, 0, NULL);
	if (self->err_no) {
		printf("hikptool roce_qmm get data failed.\n");
		goto exec_error;
	}

	roce_qmm_res = (struct roce_qmm_rsp_data *)cmd_ret->rsp_data;
	if (roce_qmm_res->reg_num > ROCE_HIKP_QMM_REG_NUM) {
		printf("version might not match, adjust the reg num to %d.\n",
		       ROCE_HIKP_QMM_REG_NUM);
		roce_qmm_res->reg_num = ROCE_HIKP_QMM_REG_NUM;
	}

	hikp_roce_qmm_print(roce_qmm_res);

exec_error:
	hikp_cmd_free(&cmd_ret);
}

static void hikp_roce_qmm_execute(struct major_cmd_ctrl *self)
{
	static const struct cmd_type_info {
		enum roce_qmm_cmd_type sub_cmd;
		enum roce_qmm_cmd_type sub_ext_cmd;
		const char *sub_name;
	} sub_cmd_info_table[] = {
		{QMM_SHOW_CQC, QMM_SHOW_CQC_EXT, "CQC"},
		{QMM_SHOW_QPC, QMM_SHOW_QPC_EXT, "QPC"},
		{QMM_SHOW_TOP, QMM_SHOW_TOP_EXT, "TOP"},
	};

	for (int i = 0; i < HIKP_ARRAY_SIZE(sub_cmd_info_table); i++) {
		g_roce_qmm_param.sub_name = sub_cmd_info_table[i].sub_name;
		if (g_roce_qmm_param.ext_flag) {
			g_roce_qmm_param.sub_cmd = sub_cmd_info_table[i].sub_ext_cmd;
			hikp_roce_ext_execute(self, GET_ROCEE_QMM_CMD,
					      hikp_roce_qmm_get_data);
		} else {
			g_roce_qmm_param.sub_cmd = sub_cmd_info_table[i].sub_cmd;
			hikp_roce_qmm_execute_origin(self);
		}
		if (self->err_no) {
			snprintf(self->err_str, sizeof(self->err_str),
				 "roce_qmm show %s function failed\n",
				 sub_cmd_info_table[i].sub_name);
			break;
		}
	}
}

static int hikp_roce_qmm_ext_set(struct major_cmd_ctrl *self, const char *argv)
{
	g_roce_qmm_param.ext_flag = true;

	return 0;
}

static void cmd_roce_qmm_init(void)
{
	struct major_cmd_ctrl *major_cmd = get_major_cmd();

	major_cmd->option_count = 0;
	major_cmd->execute = hikp_roce_qmm_execute;

	cmd_option_register("-h", "--help", false, hikp_roce_qmm_help);
	cmd_option_register("-i", "--interface", true, hikp_roce_qmm_target);
	cmd_option_register("-b", "--bank", true, hikp_roce_qmm_bank_get);
	cmd_option_register("-e", "--extend", false, hikp_roce_qmm_ext_set);
}

HIKP_CMD_DECLARE("roce_qmm", "get roce_qmm registers information", cmd_roce_qmm_init);
