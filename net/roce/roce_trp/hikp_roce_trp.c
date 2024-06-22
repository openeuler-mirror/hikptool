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

#include "hikp_roce_trp.h"

struct cmd_roce_trp_param_t g_roce_trp_param_t = { 0 };
struct roce_trp_module g_roce_trp_module[] = {
	ROCE_TRP_HANDLE(TRP_RX),
	ROCE_TRP_HANDLE(GEN_AC),
	ROCE_TRP_HANDLE(PAYL),
	ROCE_TRP_HANDLE(COMMON),
};

static int hikp_roce_trp_help(struct major_cmd_ctrl *self, const char *argv)
{
	printf("\n  Usage: %s %s\n", self->cmd_ptr->name, "-i <interface>\n");
	printf("\n         %s\n", self->cmd_ptr->help_info);
	printf("  Options:\n\n");
	printf("    %s, %-25s %s\n", "-h", "--help", "display this help and exit");
	printf("    %s, %-25s %s\n", "-i", "--interface=<interface>", "device target, e.g. eth0");
	printf("    %s, %-25s %s\n", "-m", "--module=<module>",
	       "this is necessary param COMMON/TRP_RX/GEN_AC/PAYL");
	printf("    %s, %-25s %s\n", "-b", "--bank=<bank>",
	       "[option]set which bank to read. (default 0) "
	       "COMMON : 0~3\n PAYL: 0~1\n GEN_AC : 0~3\n ");
	printf("\n");

	return 0;
}

static int hikp_roce_trp_target(struct major_cmd_ctrl *self, const char *argv)
{
	self->err_no = tool_check_and_get_valid_bdf_id(argv, &(g_roce_trp_param_t.target));
	if (self->err_no !=  0) {
		snprintf(self->err_str, sizeof(self->err_str), "Unknown device %s.", argv);
		return self->err_no;
	}

	return 0;
}

static int hikp_roce_trp_module_select(struct major_cmd_ctrl *self, const char *argv)
{
	bool is_found;
	uint32_t i;

	for (i = 0; i < HIKP_ARRAY_SIZE(g_roce_trp_module); i++) {
		is_found = strncmp(argv, (const char *)g_roce_trp_module[i].module_name,
			sizeof(g_roce_trp_module[i].module_name)) == 0;
		if (is_found) {
			g_roce_trp_param_t.sub_cmd = g_roce_trp_module[i].sub_cmd_code;
			return 0;
		}
	}
	snprintf(self->err_str, sizeof(self->err_str), "Invalid module param!");
	self->err_no = -EINVAL;

	return self->err_no;
}

static int hikp_roce_trp_bank_get(struct major_cmd_ctrl *self, const char *argv)
{
	uint32_t temp;

	self->err_no = string_toui(argv, &temp);
	if (self->err_no) {
		snprintf(self->err_str, sizeof(self->err_str), "get roce_trp bank param failed.");
		return self->err_no;
	}

	g_roce_trp_param_t.bank_enter_flag = 1;
	g_roce_trp_param_t.bank_id = temp;
	return 0;
}

static int hikp_roce_trp_bank_check(void)
{
	switch (g_roce_trp_param_t.sub_cmd) {
	case (COMMON):
		if (g_roce_trp_param_t.bank_id > TRP_MAX_BANK_NUM)
			return -EINVAL;
		break;
	case (PAYL):
		if (g_roce_trp_param_t.bank_id > PAYL_MAX_BANK_NUM)
			return -EINVAL;
		break;
	case (GEN_AC):
		if (g_roce_trp_param_t.bank_id > GAC_MAX_BANK_NUM)
			return -EINVAL;
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static int hikp_roce_trp_get_data(struct hikp_cmd_ret **cmd_ret, const uint32_t *block_id)
{
	struct roce_trp_req_param req_data = { 0 };
	struct hikp_cmd_header req_header = { 0 };
	int ret;

	req_data.block_id = *block_id;
	req_data.bdf = g_roce_trp_param_t.target.bdf;
	req_data.bank_id = g_roce_trp_param_t.bank_id;
	if (g_roce_trp_param_t.sub_cmd == 0) {
		printf("please enter module name: -m/--modlue\n");
		return -EINVAL;
	}
	hikp_cmd_init(&req_header, ROCE_MOD, GET_ROCEE_TRP_CMD, g_roce_trp_param_t.sub_cmd);
	*cmd_ret = hikp_cmd_alloc(&req_header, &req_data, sizeof(req_data));
	ret = hikp_rsp_normal_check(*cmd_ret);
	if (ret) {
		printf("hikptool roce_trp get cmd data failed, ret: %d\n", ret);
		free(*cmd_ret);
		*cmd_ret = NULL;
	}

	return ret;
}

static void hikp_roce_trp_reg_data_free(uint32_t **offset, uint32_t **data)
{
	if (*offset) {
		free(*offset);
		*offset = NULL;
	}
	if (*data) {
		free(*data);
		*data = NULL;
	}
}

static void hikp_roce_trp_cmd_ret_free(struct hikp_cmd_ret **cmd_ret)
{
	if (*cmd_ret) {
		free(*cmd_ret);
		*cmd_ret = NULL;
	}
}

static int hikp_roce_trp_get_total_data_num(struct roce_trp_head *res_head,
					    uint32_t **offset, uint32_t **data, uint32_t *block_id)
{
	struct roce_trp_res_param *roce_trp_res;
	struct hikp_cmd_ret *cmd_ret = NULL;
	size_t max_size;
	size_t cur_size;
	int ret;

	ret = hikp_roce_trp_get_data(&cmd_ret, block_id);
	if (ret) {
		printf("hikptool roce_trp get total data failed\n");
		return ret;
	}

	roce_trp_res = (struct roce_trp_res_param *)cmd_ret->rsp_data;
	max_size = roce_trp_res->head.total_block_num * sizeof(uint32_t);
	*offset = (uint32_t *)calloc(1, max_size);
	*data = (uint32_t *)calloc(1, max_size);
	if ((*offset == NULL) || (*data == NULL)) {
		printf("hikptool roce_trp alloc log memmory 0x%zx failed\n", max_size);
		hikp_roce_trp_reg_data_free(offset, data);
		ret = -ENOMEM;
		goto get_data_error;
	}

	cur_size = roce_trp_res->head.cur_block_num * sizeof(uint32_t);
	if (cur_size > max_size) {
		printf("hikptool roce_trp log data copy size error, "
		       "data size: 0x%zx, max size: 0x%zx\n", cur_size, max_size);
		hikp_roce_trp_reg_data_free(offset, data);
		ret = -EINVAL;
		goto get_data_error;
	}
	memcpy(*offset, roce_trp_res->reg_data.offset, cur_size);
	memcpy(*data, roce_trp_res->reg_data.data, cur_size);

	*res_head = roce_trp_res->head;
	*block_id = roce_trp_res->block_id;
	ret = 0;

get_data_error:
	hikp_roce_trp_cmd_ret_free(&cmd_ret);
	return ret;
}

static int hikp_roce_trp_get_next_data(struct roce_trp_head *res_head,
				       uint32_t **offset, uint32_t **data,
				       uint32_t *block_id, size_t data_size)
{
	struct roce_trp_res_param *roce_trp_res;
	struct hikp_cmd_ret *cmd_ret = NULL;
	size_t cur_size;
	int ret;

	ret = hikp_roce_trp_get_data(&cmd_ret, block_id);
	if (ret) {
		printf("hikptool roce_trp get next data failed\n");
		return ret;
	}

	roce_trp_res = (struct roce_trp_res_param *)cmd_ret->rsp_data;
	cur_size = roce_trp_res->head.cur_block_num * sizeof(uint32_t);

	if (cur_size > data_size) {
		hikp_roce_trp_cmd_ret_free(&cmd_ret);
		printf("hikptool roce_trp next log data copy size error, "
		       "data size: 0x%zx, max size: 0x%zx\n", cur_size, data_size);
		return -EINVAL;
	}
	memcpy(*offset, roce_trp_res->reg_data.offset, cur_size);
	memcpy(*data, roce_trp_res->reg_data.data, cur_size);

	*block_id = roce_trp_res->block_id;
	res_head->cur_block_num = roce_trp_res->head.cur_block_num;
	res_head->total_block_num = res_head->total_block_num - roce_trp_res->head.cur_block_num;
	hikp_roce_trp_cmd_ret_free(&cmd_ret);

	return 0;
}

/* DON'T change the order of these arrays or add entries between! */
static const char *g_trp_common_reg_name[] = {
	"GEN_AC_QP_FIFO_FULL",
	"GEN_AC_QP_FIFO_EMPTY",
	"GEN_AC_QP_INNER_STA_0",
	"GEN_AC_QP_INNER_STA_1",
	"GEN_AC_QP_ALM",
	"GEN_AC_QP_TSP_CQE_CNT",
	"TRP_GET_PBL_FULL",
	"TRP_GET_PBL_EMPTY",
	"TRP_GET_PBL_INNER_ALM",
	"TRP_GET_PBL_INNER_STA",
	"TRP_GET_MPT_FSM",
	"TRP_GET_MPT_EMPTY",
	"TRP_GET_MPT_INNER_ALM",
	"TRP_GET_MPT_INNER_STA",
	"TRP_GET_SGE_FSM",
	"TRP_GET_SGE_EMPTY",
	"TRP_GET_SGE_INNER_ALM",
	"TRP_GET_SGE_INNER_STA",
	"TRP_GET_BA_EMPTY",
	"TRP_GET_BA_INNER_ALM",
	"TRP_GET_BA_INNER_STA",
	"TRP_DMAECMD_EMPTY_0",
	"TRP_DMAECMD_EMPTY_1",
	"TRP_DMAECMD_FULL",
	"TRP_GET_IRRL_FSM",
	"TRP_GET_IRRL_FULL",
	"TRP_GET_IRRL_EMPTY",
	"TRP_GET_IRRL_INNER_ALM",
	"TRP_GET_IRRL_INNER_STA",
	"TRP_GET_QPC_FSM",
	"TRP_GET_QPC_INNER_ALM",
	"TRP_GET_QPC_INNER_STA",
	"ROCEE_TRP_ECC_ERR_INFO",
	"ROCEE_TRP_ECC1B",
	"ROCEE_TRP_ECC2B",
	"ROCEE_TRP_FUN_RST_DFX",
	"TRP_GET_MPT_ERR_FLG",
	"TRP_GET_IRRL_ERR_FLG",
	"TRP_GET_QPC_ERR_FLG",
	"ROCEE_ECN_DB_CNT",
	"GEN_AC_QP_TSP_AE_CNT",
	"GEN_AC_QP_MDB_CQE_CNT",
	"GEN_AC_QP_LPRC_CQE_CNT",
	"TRP_CNP_CNT",
	"TRP_SGE_ERR_DROP_LEN",
	"TRP_SGE_AXI_CNT",
};

static const char *g_trp_trp_rx_reg_name[] = {
	"TRP_RX_CHECK_EN",
	"TRP_RX_WR_PAYL_AXI_ERR",
	"ROCEE_TRP_RX_STA",
	"RX_FIFO_FULL",
	"RX_FIFO_EMPTY_0",
	"RX_FIFO_EMPTY_1",
	"HEAD_BUFF_ECC",
	"HEAD_BUFF_ECC_ADDR",
	"TRP_RX_FIFO_EMPTY_0",
	"TRP_RX_FIFO_EMPTY_1",
	"TRP_RX_FIFO_EMPTY_2",
	"TRP_RX_FIFO_EMPTY_3",
};

static const char *g_trp_gen_ac_reg_name[] = {
	"GEN_AC_CQ_FIFO_FULL",
	"GEN_AC_CQ_FIFO_EMPTY",
	"GEN_AC_CQ_INNER_STA",
	"GEN_AC_CQ_ALM",
	"GEN_AC_CQ_CQE_CNT_0",
	"GEN_AC_CQ_CQE_CNT_1",
	"GEN_AC_CQ_CQE_CNT_2",
	"GEN_AC_CQ_CQE_CNT_3",
	"ROCEE_GENAC_ECC_ERR_INFO",
	"ROCEE_GENAC_ECC1B",
	"ROCEE_GENAC_ECC2B",
	"GEN_AC_DMAECMD_STA",
	"GEN_AC_DMAECMD_ALM",
	"SWQE_LINK_STA",
	"SWQE_LINK_ALM",
	"GEN_AC_CQ_MAIN_STA_0",
	"GEN_AC_CQ_MAIN_ALM",
	"GEN_AC_CQ_MAIN_STA_1",
	"POE_DFX_0",
	"POE_DFX_1",
	"POE_DFX_2",
};

static const char *g_trp_payl_reg_name[] = {
	"ROCEE_EXT_ATOMIC_DFX_0",
	"ROCEE_EXT_ATOMIC_DFX_1",
	"ROCEE_EXT_ATOMIC_DFX_2",
	"ROCEE_EXT_ATOMIC_DFX_3",
	"ATOMIC_DFX_0",
	"ATOMIC_DFX_1",
	"ATOMIC_DFX_2",
	"WR_PAYL_DFX_1",
	"PAYL_BUFF_DFX_0",
	"PAYL_BUFF_DFX_1",
	"PAYL_BUFF_DFX_2",
	"PAYL_BUFF_DFX_3",
	"PAYL_BUFF_DFX_4",
	"WR_PAYL_DFX_RC",
	"WR_PAYL_DFX_RO",
	"WR_PAYL_1_OST_NUM",
};

static const struct reg_name_info {
	enum roce_trp_type sub_cmd;
	const char **reg_name;
	uint8_t arr_len;
} g_trp_reg_name_info_table[] = {
	{COMMON, g_trp_common_reg_name, HIKP_ARRAY_SIZE(g_trp_common_reg_name)},
	{TRP_RX, g_trp_trp_rx_reg_name, HIKP_ARRAY_SIZE(g_trp_trp_rx_reg_name)},
	{GEN_AC, g_trp_gen_ac_reg_name, HIKP_ARRAY_SIZE(g_trp_gen_ac_reg_name)},
	{PAYL, g_trp_payl_reg_name, HIKP_ARRAY_SIZE(g_trp_payl_reg_name)},
};

static void hikp_roce_trp_print(uint8_t total_block_num,
				const uint32_t *offset, const uint32_t *data)
{
	const char **reg_name;
	uint8_t arr_len;
	uint32_t i;

	for (i = 0; i < HIKP_ARRAY_SIZE(g_trp_reg_name_info_table); i++) {
		if (g_trp_reg_name_info_table[i].sub_cmd != g_roce_trp_param_t.sub_cmd)
			continue;
		arr_len = g_trp_reg_name_info_table[i].arr_len;
		reg_name = g_trp_reg_name_info_table[i].reg_name;
		break;
	}

	if (i == HIKP_ARRAY_SIZE(g_trp_reg_name_info_table)) {
		printf("can't find reg name table for roce_trp sub_cmd %u.\n",
		       g_roce_trp_param_t.sub_cmd);
		return;
	}

	printf("**************TRP INFO*************\n");
	printf("%-40s[addr_offset] : reg_data\n", "reg_name");
	for (i = 0; i < total_block_num; i++)
		printf("%-40s[0x%08X] : 0x%08X\n",
		       i < arr_len ? reg_name[i] : "",
		       offset[i], data[i]);
	printf("***********************************\n");
}

static void hikp_roce_trp_execute(struct major_cmd_ctrl *self)
{
	struct roce_trp_head res_head;
	uint32_t *offset_start = NULL;
	uint32_t *data_start = NULL;
	uint8_t total_block_num;
	uint32_t *offset = NULL;
	uint32_t *data = NULL;
	uint32_t block_id;
	size_t data_size;
	uint32_t times;
	uint32_t i;

	block_id = 0;
	if (g_roce_trp_param_t.bank_enter_flag) {
		self->err_no = hikp_roce_trp_bank_check();
		if (self->err_no) {
			snprintf(self->err_str, sizeof(self->err_str), "Invalid bank number!");
			return;
		}
	}
	self->err_no = hikp_roce_trp_get_total_data_num(&res_head, &offset, &data, &block_id);
	if (self->err_no) {
		snprintf(self->err_str, sizeof(self->err_str),
			 "get the first roce_trp block dfx fail.");
		return;
	}
	total_block_num = res_head.total_block_num;
	res_head.total_block_num = res_head.total_block_num - res_head.cur_block_num;
	offset_start = offset;
	data_start = data;
	if (res_head.total_block_num) {
		times = res_head.total_block_num / ROCE_HIKP_TRP_REG_NUM + 1;
		for (i = 0; i < times; i++) {
			offset = offset + res_head.cur_block_num;
			data = data + res_head.cur_block_num;
			data_size = res_head.total_block_num * sizeof(uint32_t);
			self->err_no = hikp_roce_trp_get_next_data(&res_head, &offset,
								   &data, &block_id, data_size);
			if (self->err_no) {
				snprintf(self->err_str, sizeof(self->err_str),
					 "get multiple roce_trp block dfx fail.");
				hikp_roce_trp_reg_data_free(&offset_start, &data_start);
				return;
			}
		}
	}
	hikp_roce_trp_print(total_block_num, offset_start, data_start);
	hikp_roce_trp_reg_data_free(&offset_start, &data_start);
}

static void cmd_roce_trp_init(void)
{
	struct major_cmd_ctrl *major_cmd = get_major_cmd();

	major_cmd->option_count = 0;
	major_cmd->execute = hikp_roce_trp_execute;

	cmd_option_register("-h", "--help", false, hikp_roce_trp_help);
	cmd_option_register("-i", "--interface", true, hikp_roce_trp_target);
	cmd_option_register("-b", "--bank", true, hikp_roce_trp_bank_get);
	cmd_option_register("-m", "--module", true, hikp_roce_trp_module_select);
}

HIKP_CMD_DECLARE("roce_trp", "get roce_trp registers information", cmd_roce_trp_init);
