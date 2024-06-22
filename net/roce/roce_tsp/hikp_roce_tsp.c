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

#include "hikp_roce_tsp.h"

static struct cmd_roce_tsp_param_t g_roce_tsp_param_t = { 0 };
static struct roce_tsp_module g_roce_tsp_module[] = {
	TSP_HANDLE(COMMON),
	TSP_HANDLE(TDP),
	TSP_HANDLE(TGP_TMP),
};

static int hikp_roce_tsp_help(struct major_cmd_ctrl *self, const char *argv)
{
	printf("\n  Usage: %s %s\n", self->cmd_ptr->name, "-i <interface>\n");
	printf("\n         %s\n", self->cmd_ptr->help_info);
	printf("  Options:\n\n");
	printf("    %s, %-25s %s\n", "-h", "--help", "display this help and exit");
	printf("    %s, %-25s %s\n", "-i", "--interface=<interface>", "device target, e.g. eth0");
	printf("    %s, %-25s %s\n", "-m", "--module=<module>",
	       "this is necessary param COMMON/TDP/TGP_TMP");
	printf("    %s, %-25s %s\n", "-c", "--clear=<clear>", "clear TDP registers");
	printf("    %s, %-25s %s\n", "-b", "--bank=<bank>",
	       "[option]set which bank to read. (default 0) COMMON :  0~7\n TGP_TMP:  0~3\n ");
	printf("\n");

	return 0;
}

static int hikp_roce_tsp_target(struct major_cmd_ctrl *self, const char *argv)
{
	self->err_no = tool_check_and_get_valid_bdf_id(argv, &(g_roce_tsp_param_t.target));
	if (self->err_no != 0) {
		snprintf(self->err_str, sizeof(self->err_str), "Unknown device %s.", argv);
		return self->err_no;
	}

	return 0;
}

static int hikp_roce_tsp_module_select(struct major_cmd_ctrl *self, const char *argv)
{
	bool is_found;
	uint32_t i;

	for (i = 0; i < HIKP_ARRAY_SIZE(g_roce_tsp_module); i++) {
		is_found = strncmp(argv, (const char *)g_roce_tsp_module[i].module_name,
			sizeof(g_roce_tsp_module[i].module_name)) == 0;
		if (is_found) {
			g_roce_tsp_param_t.sub_cmd_code = g_roce_tsp_module[i].sub_cmd_code;
			return 0;
		}
	}
	snprintf(self->err_str, sizeof(self->err_str), "Invalid module param!");
	self->err_no = -EINVAL;

	return -EINVAL;
}

static int hikp_roce_tsp_bank_get(struct major_cmd_ctrl *self, const char *argv)
{
	uint32_t temp;

	self->err_no = string_toui(argv, &temp);
	if (self->err_no) {
		snprintf(self->err_str, sizeof(self->err_str), "get roce_tsp bank param failed.");
		return self->err_no;
	}

	g_roce_tsp_param_t.bank_enter_flag = 1;
	g_roce_tsp_param_t.bank_id = temp;
	return 0;
}

static int hikp_roce_tsp_bank_check(void)
{
	switch (g_roce_tsp_param_t.sub_cmd_code) {
	case (COMMON):
		if (g_roce_tsp_param_t.bank_id > MAX_TSP_BANK_NUM)
			return -EINVAL;
		break;
	case (TGP_TMP):
		if (g_roce_tsp_param_t.bank_id > MAX_TGP_TMP_BANK_NUM)
			return -EINVAL;
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static int hikp_roce_tsp_clear_set(struct major_cmd_ctrl *self, const char *argv)
{
	g_roce_tsp_param_t.reset_flag = 1;

	return 0;
}

static int hikp_roce_tsp_clear_module_check(void)
{
	if (g_roce_tsp_param_t.sub_cmd_code == TDP)
		return 0;

	return -EINVAL;
}

static int hikp_roce_tsp_get_data(struct hikp_cmd_ret **cmd_ret,
				  struct roce_tsp_req_param req_data, uint32_t sub_cmd_code)
{
	struct hikp_cmd_header req_header = { 0 };
	int ret;

	if (g_roce_tsp_param_t.sub_cmd_code == 0) {
		printf("please enter module name: -m/--modlue\n");
		return -EINVAL;
	}
	hikp_cmd_init(&req_header, ROCE_MOD, GET_ROCEE_TSP_CMD, sub_cmd_code);
	*cmd_ret = hikp_cmd_alloc(&req_header, &req_data, sizeof(req_data));
	ret = hikp_rsp_normal_check(*cmd_ret);
	if (ret) {
		printf("hikptool roce_tsp get cmd data failed, ret: %d\n", ret);
		free(*cmd_ret);
		*cmd_ret = NULL;
	}

	return ret;
}

/* DON'T change the order of these arrays or add entries between! */
static const char *g_tsp_common_reg_name[] = {
	"ROCEE_TPP_ECC_ERR",
	"ROCEE_TWP_STA",
	"ROCEE_TWP_ALM",
	"ROCEE_TWP_STA1",
	"TSP_INDRECT_RW_STA",
	"TSP_INDRECT_RD_CTRL",
	"TSP_INDRECT_RD_DATA",
	"ROCEE_TSP_OVF",
};

static const char *g_tsp_tdp_reg_name[] = {
	"TDP_M_MEM_INIT_DONE",
	"TDP_M_ECC1B",
	"TDP_M_ECC2B",
	"TDP_M_ECC_ERR_INFO",
	"TDP_M_ALM",
	"ROCEE_TDP_CNT_CFG0",
	"ROCEE_TDP_CNT_CFG1",
	"ROCEE_TDP_IN_CNT_ENB",
	"ROCEE_TDP_TWP_CNT0_CFG",
	"ROCEE_TDP_TWP_CNT1_CFG",
	"ROCEE_TDP_TRP_CNT",
	"ROCEE_TDP_MDB_CNT",
	"ROCEE_TDP_LP_CNT",
	"ROCEE_TDP_QMM_CNT",
	"ROCEE_TDP_TWP_CNT0",
	"ROCEE_TDP_TWP_CNT1",
	"TDP_V_ECC1B",
	"TDP_V_ECC2B",
	"TDP_V_ECC_ERR_INFO",
	"TDP_V_ALM",
	"TDP_V_STA",
	"ROCEE_TSP_OOO_ERR",
	"TDP_M_STA",
	"TDP_M_STA1",
};

static const char *g_tsp_tgp_tmp_reg_name[] = {
	"ROCEE_TGP_ALM0",
	"ROCEE_TGP_ALM1",
	"ROCEE_TGP_STA0",
	"ROCEE_TGP_STA1",
	"TGP_INDRECT_RD_CTRL",
	"TGP_INDRECT_RD_STA",
	"TGP_INDRECT_RD_DATA",
	"ROCEE_TMP_ALM0",
	"ROCEE_TMP_ALM1",
	"ROCEE_TMP_STA0",
	"ROCEE_TMP_STA1",
};

static const struct reg_name_info {
	enum roce_tsp_sub_cmd_code sub_cmd;
	const char **reg_name;
	uint8_t arr_len;
} g_tsp_reg_name_info_table[] = {
	{COMMON, g_tsp_common_reg_name, HIKP_ARRAY_SIZE(g_tsp_common_reg_name)},
	{TDP, g_tsp_tdp_reg_name, HIKP_ARRAY_SIZE(g_tsp_tdp_reg_name)},
	{TGP_TMP, g_tsp_tgp_tmp_reg_name, HIKP_ARRAY_SIZE(g_tsp_tgp_tmp_reg_name)},
};

static void hikp_roce_tsp_print(uint32_t total_block_num,
				const uint32_t *offset, const uint32_t *data)
{
	const char **reg_name;
	uint8_t arr_len;
	uint32_t i;

	for (i = 0; i < HIKP_ARRAY_SIZE(g_tsp_reg_name_info_table); i++) {
		if (g_tsp_reg_name_info_table[i].sub_cmd != g_roce_tsp_param_t.sub_cmd_code)
			continue;
		arr_len = g_tsp_reg_name_info_table[i].arr_len;
		reg_name = g_tsp_reg_name_info_table[i].reg_name;
		break;
	}

	if (i == HIKP_ARRAY_SIZE(g_tsp_reg_name_info_table)) {
		printf("can't find reg name table for roce_tsp sub_cmd %u.\n",
		       g_roce_tsp_param_t.sub_cmd_code);
		return;
	}

	printf("**************TSP INFO*************\n");
	printf("%-40s[addr_offset] : reg_data\n", "reg_name");
	for (i = 0; i < total_block_num; i++)
		printf("%-40s[0x%08X] : 0x%08X\n",
		       i < arr_len ? reg_name[i] : "",
		       offset[i], data[i]);
	printf("***********************************\n");
}

static void hikp_roce_tsp_execute(struct major_cmd_ctrl *self)
{
	struct roce_tsp_res_param *roce_tsp_res;
	struct roce_tsp_req_param req_data;
	struct hikp_cmd_ret *cmd_ret;
	int ret;

	if (g_roce_tsp_param_t.reset_flag) {
		self->err_no = hikp_roce_tsp_clear_module_check();
		if (self->err_no) {
			snprintf(self->err_str, sizeof(self->err_str),
				 "roce_tsp clear function module selection error.");
			return;
		}
	}

	if (g_roce_tsp_param_t.bank_enter_flag) {
		self->err_no = hikp_roce_tsp_bank_check();
		if (self->err_no) {
			snprintf(self->err_str, sizeof(self->err_str), "Invalid bank number!");
			return;
		}
	}
	req_data.bdf = g_roce_tsp_param_t.target.bdf;
	req_data.reset_flag = g_roce_tsp_param_t.reset_flag;
	req_data.bank_id = g_roce_tsp_param_t.bank_id;
	ret = hikp_roce_tsp_get_data(&cmd_ret, req_data, g_roce_tsp_param_t.sub_cmd_code);
	if (ret < 0) {
		self->err_no = ret;
		return;
	}

	roce_tsp_res = (struct roce_tsp_res_param *)cmd_ret->rsp_data;
	if (roce_tsp_res->total_block_num > ROCE_HIKP_TSP_REG_NUM) {
		printf("version might not match, adjust the reg num to %d.\n",
		       ROCE_HIKP_TSP_REG_NUM);
		roce_tsp_res->total_block_num = ROCE_HIKP_TSP_REG_NUM;
	}

	hikp_roce_tsp_print(roce_tsp_res->total_block_num,
			    roce_tsp_res->reg_data.offset, roce_tsp_res->reg_data.data);

	free(cmd_ret);
	cmd_ret = NULL;
}

static void cmd_roce_tsp_init(void)
{
	struct major_cmd_ctrl *major_cmd = get_major_cmd();

	major_cmd->option_count = 0;
	major_cmd->execute = hikp_roce_tsp_execute;

	cmd_option_register("-h", "--help", false, hikp_roce_tsp_help);
	cmd_option_register("-i", "--interface", true, hikp_roce_tsp_target);
	cmd_option_register("-m", "--module", true, hikp_roce_tsp_module_select);
	cmd_option_register("-b", "--bank", true, hikp_roce_tsp_bank_get);
	cmd_option_register("-c", "--clear", false, hikp_roce_tsp_clear_set);
}

HIKP_CMD_DECLARE("roce_tsp", "get or clear roce_tsp registers information", cmd_roce_tsp_init);
