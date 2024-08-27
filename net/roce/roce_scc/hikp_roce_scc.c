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

#include "hikp_roce_scc.h"

struct cmd_roce_scc_param_t g_roce_scc_param_t = { 0 };
struct roce_scc_module g_roce_scc_module[] = {
	{ "COMMON", SCC_COMMON },
	ROCE_SCC_HANDLE(DCQCN),
	ROCE_SCC_HANDLE(DIP),
	ROCE_SCC_HANDLE(HC3),
	ROCE_SCC_HANDLE(LDCP),
	ROCE_SCC_HANDLE(CFG),
};

int hikp_roce_set_scc_bdf(char *nic_name)
{
	return tool_check_and_get_valid_bdf_id(nic_name,
					       &g_roce_scc_param_t.target);
}

void hikp_roce_set_scc_submodule(uint32_t module)
{
	g_roce_scc_param_t.sub_cmd = module;
}

static int hikp_roce_scc_help(struct major_cmd_ctrl *self, const char *argv)
{
	printf("\n  Usage: %s %s\n", self->cmd_ptr->name, "-i <interface>\n");
	printf("\n         %s\n", self->cmd_ptr->help_info);
	printf("  Options:\n\n");
	printf("    %s, %-25s %s\n", "-h", "--help", "display this help and exit");
	printf("    %s, %-25s %s\n", "-i", "--interface=<interface>", "device target, e.g. eth0");
	printf("    %s, %-25s %s\n", "-m", "--module=<module>", "this is necessary param "
	       "COMMON/DCQCN/DIP/HC3/LDCP/CFG");
	printf("    %s, %-25s %s\n", "-c", "--clear=<clear>",
	       "[Only Work for COMMON]clear param count registers");
	printf("\n");

	return 0;
}

static int hikp_roce_scc_target(struct major_cmd_ctrl *self, const char *argv)
{
	self->err_no = tool_check_and_get_valid_bdf_id(argv, &(g_roce_scc_param_t.target));
	if (self->err_no !=  0) {
		snprintf(self->err_str, sizeof(self->err_str), "Unknown device %s.", argv);
		return self->err_no;
	}

	return 0;
}

static int hikp_roce_scc_module_select(struct major_cmd_ctrl *self, const char *argv)
{
	bool is_found;
	uint32_t i;

	for (i = 0; i < HIKP_ARRAY_SIZE(g_roce_scc_module); i++) {
		is_found = strncmp(argv, (const char *)g_roce_scc_module[i].module_name,
			sizeof(g_roce_scc_module[i].module_name)) == 0;
		if (is_found) {
			g_roce_scc_param_t.sub_cmd = g_roce_scc_module[i].sub_cmd_code;
			return 0;
		}
	}

	snprintf(self->err_str, sizeof(self->err_str), "Invalid module param!");
	self->err_no = -EINVAL;
	return -EINVAL;
}

static int hikp_roce_scc_clear_set(struct major_cmd_ctrl *self, const char *argv)
{
	g_roce_scc_param_t.reset_flag = 1;

	return 0;
}

static int hikp_roce_scc_clear_module_check(void)
{
	if (g_roce_scc_param_t.sub_cmd == SCC_COMMON)
		return 0;

	return -EINVAL;
}

static int hikp_roce_scc_get_data(struct hikp_cmd_ret **cmd_ret, const uint32_t *block_id)
{
	struct roce_scc_req_param req_data = { 0 };
	struct hikp_cmd_header req_header = { 0 };
	int ret;

	req_data.block_id = *block_id;
	req_data.bdf = g_roce_scc_param_t.target.bdf;
	req_data.reset_flag = g_roce_scc_param_t.reset_flag;
	if (g_roce_scc_param_t.sub_cmd == 0) {
		printf("please enter module name: -m/--modlue\n");
		return -EINVAL;
	}

	hikp_cmd_init(&req_header, ROCE_MOD, GET_ROCEE_SCC_CMD, g_roce_scc_param_t.sub_cmd);
	*cmd_ret = hikp_cmd_alloc(&req_header, &req_data, sizeof(req_data));
	ret = hikp_rsp_normal_check(*cmd_ret);
	if (ret) {
		printf("hikptool roce_scc get cmd data failed, ret: %d\n", ret);
		hikp_cmd_free(cmd_ret);
	}

	return ret;
}

static void hikp_roce_scc_reg_data_free(uint32_t **offset, uint32_t **data)
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

static int hikp_roce_scc_get_total_data_num(struct roce_scc_head *res_head,
					    uint32_t **offset, uint32_t **data, uint32_t *block_id)
{
	struct roce_scc_res_param *roce_scc_res;
	struct hikp_cmd_ret *cmd_ret = NULL;
	size_t max_size;
	size_t cur_size;
	int ret;

	ret = hikp_roce_scc_get_data(&cmd_ret, block_id);
	if (ret) {
		printf("hikptool roce_scc get total data failed\n");
		return ret;
	}

	roce_scc_res = (struct roce_scc_res_param *)cmd_ret->rsp_data;
	if (!roce_scc_res->head.total_block_num) {
		printf("hikptool roce_scc total_block_num error!\n");
		ret = -EINVAL;
		goto get_data_error;
	}

	max_size = roce_scc_res->head.total_block_num * sizeof(uint32_t);
	*offset = (uint32_t *)calloc(1, max_size);
	*data = (uint32_t *)calloc(1, max_size);
	if ((*offset == NULL) || (*data == NULL)) {
		printf("hikptool roce_scc alloc log memmory 0x%zx failed\n", max_size);
		ret = -ENOMEM;
		goto get_data_error;
	}

	cur_size = roce_scc_res->head.cur_block_num * sizeof(uint32_t);
	if (cur_size > max_size) {
		printf("hikptool roce_scc log data copy size error, "
		       "data size: 0x%zx, max size: 0x%zx\n", cur_size, max_size);
		ret = -EINVAL;
		goto get_data_error;
	}
	memcpy(*offset, roce_scc_res->reg_data.offset, cur_size);
	memcpy(*data, roce_scc_res->reg_data.data, cur_size);

	*res_head = roce_scc_res->head;
	*block_id = roce_scc_res->block_id;
	ret = 0;

get_data_error:
	hikp_cmd_free(&cmd_ret);
	return ret;
}

static int hikp_roce_scc_get_next_data(struct roce_scc_head *res_head,
				       uint32_t **offset, uint32_t **data,
				       uint32_t *block_id, size_t data_size)
{
	struct roce_scc_res_param *roce_scc_res;
	struct hikp_cmd_ret *cmd_ret = NULL;
	size_t cur_size;
	int ret;

	ret = hikp_roce_scc_get_data(&cmd_ret, block_id);
	if (ret) {
		printf("hikptool roce_scc get next data failed\n");
		return ret;
	}

	roce_scc_res = (struct roce_scc_res_param *)cmd_ret->rsp_data;
	cur_size = roce_scc_res->head.cur_block_num * sizeof(uint32_t);
	if (cur_size > data_size) {
		hikp_cmd_free(&cmd_ret);
		printf("hikptool roce_scc next log data copy size error, "
		       "data size: 0x%zx, max size: 0x%zx\n", cur_size, data_size);
		return -EINVAL;
	}
	memcpy(*offset, roce_scc_res->reg_data.offset, cur_size);
	memcpy(*data, roce_scc_res->reg_data.data, cur_size);

	*block_id = roce_scc_res->block_id;
	res_head->cur_block_num = roce_scc_res->head.cur_block_num;
	res_head->total_block_num = res_head->total_block_num - roce_scc_res->head.cur_block_num;
	hikp_cmd_free(&cmd_ret);

	return 0;
}

/* DON'T change the order of these arrays or add entries between! */
static const char *g_scc_common_reg_name[] = {
	"SCC_MODE_SEL",
	"SCC_OUTSTANDING_CTRL",
	"SCC_FW_BASE_ADDR",
	"SCC_MEM_START_INIT",
	"SCC_MEM_INIT_DONE",
	"SCC_FW_REQ_CNT",
	"SCC_FW_RSP_CNT",
	"SCC_FW_CNT_CTRL",
	"SCC_GLB_OUTSTANDING",
	"SCC_AXI_OUTSTANDING",
	"SCC_FW_REQRSP_CNT0",
	"SCC_FW_REQRSP_CNT1",
	"SCC_OUTSTANDING_ID",
	"SCC_OUTSTANDING_STS",
	"SCC_CACHEMISS_LOAD_CNT",
	"SCC_CACHEMISS_STORE_CNT",
	"FW_PROCESS_TIME",
	"SCC_INT_EN",
	"SCC_INT_SRC",
	"SCC_ECC_1BIT_CNT",
	"SCC_ECC_1BIT_INFO",
	"SCC_ECC_MBIT_INFO",
	"ECC_ERR_INJ_SEL",
	"CTX_RDWR_ERR_INFO",
	"SCC_FW_REQRSP_CNT2",
	"SCC_LOAD_CAL_CFG_0",
	"SCC_LOAD_CAL_CFG_1",
	"SCC_LOAD_CAL_CFG_2",
	"SCC_LOAD_CAL_CFG_3",
	"SCC_LOAD_CAL_CFG_4",
	"SCC_LOAD_CAL_CFG_5",
	"SCC_LOAD_CAL_CFG_6",
	"SCC_LOAD_CAL_CFG_7",
	"SCC_MAX_PROCESS_TIME",
	"SCC_TIMEOUT_SET",
	"SCC_TIMEOUT_CNT",
	"SCC_TIME_STA_EN",
	"SCC_INPUT_REQ_CNT",
	"SCC_OUTPUT_RSP_CNT",
	"SCC_INOUT_CNT_CFG",
};

static const char *g_scc_dcqcn_reg_name[] = {
	"SCC_TEMP_CFG0",
	"SCC_TEMP_CFG1",
	"SCC_TEMP_CFG2",
	"SCC_TEMP_CFG3",
	"ROCEE_CNP_PKT_RX_CNT",
	"ROCEE_CNP_PKT_TX_CNT",
	"ROCEE_ECN_DB_CNT0",
	"ROCEE_ECN_DB_CNT1",
	"ROCEE_ECN_DB_CNT2",
	"ROCEE_ECN_DB_CNT3",
};

static const char *g_scc_dip_reg_name[] = {
	"SCC_TEMP_CFG0",
	"SCC_TEMP_CFG1",
	"SCC_TEMP_CFG2",
	"SCC_TEMP_CFG3",
};

static const char *g_scc_hc3_reg_name[] = {
	"SCC_TEMP_CFG0",
	"SCC_TEMP_CFG1",
	"SCC_TEMP_CFG2",
	"SCC_TEMP_CFG3",
};

static const char *g_scc_ldcp_reg_name[] = {
	"SCC_TEMP_CFG0",
	"SCC_TEMP_CFG1",
	"SCC_TEMP_CFG2",
	"SCC_TEMP_CFG3",
};

static const char *g_scc_cfg_reg_name[] = {
	"ROCEE_TM_CFG",
};

static const struct reg_name_info {
	enum roce_scc_type sub_cmd;
	const char **reg_name;
	uint8_t arr_len;
} g_scc_reg_name_info_table[] = {
	{SCC_COMMON, g_scc_common_reg_name, HIKP_ARRAY_SIZE(g_scc_common_reg_name)},
	{DCQCN, g_scc_dcqcn_reg_name, HIKP_ARRAY_SIZE(g_scc_dcqcn_reg_name)},
	{DIP, g_scc_dip_reg_name, HIKP_ARRAY_SIZE(g_scc_dip_reg_name)},
	{HC3, g_scc_hc3_reg_name, HIKP_ARRAY_SIZE(g_scc_hc3_reg_name)},
	{LDCP, g_scc_ldcp_reg_name, HIKP_ARRAY_SIZE(g_scc_ldcp_reg_name)},
	{CFG, g_scc_cfg_reg_name, HIKP_ARRAY_SIZE(g_scc_cfg_reg_name)},
};

static void hikp_roce_scc_print(uint8_t total_block_num,
				const uint32_t *offset, const uint32_t *data)
{
	const char **reg_name;
	uint8_t arr_len;
	uint32_t i;

	for (i = 0; i < HIKP_ARRAY_SIZE(g_scc_reg_name_info_table); i++) {
		if (g_scc_reg_name_info_table[i].sub_cmd != g_roce_scc_param_t.sub_cmd)
			continue;
		arr_len = g_scc_reg_name_info_table[i].arr_len;
		reg_name = g_scc_reg_name_info_table[i].reg_name;
		break;
	}

	if (i == HIKP_ARRAY_SIZE(g_scc_reg_name_info_table)) {
		printf("can't find reg name table for roce_scc sub_cmd %u.\n",
		       g_roce_scc_param_t.sub_cmd);
		return;
	}

	printf("**************SCC INFO*************\n");
	printf("%-40s[addr_offset] : reg_data\n", "reg_name");
	for (i = 0; i < total_block_num; i++)
		printf("%-40s[0x%08X] : 0x%08X\n",
		       i < arr_len ? reg_name[i] : "",
		       offset[i], data[i]);
	printf("***********************************\n");
}

void hikp_roce_scc_execute(struct major_cmd_ctrl *self)
{
	struct roce_scc_head res_head;
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
	if (g_roce_scc_param_t.reset_flag) {
		self->err_no = hikp_roce_scc_clear_module_check();
		if (self->err_no) {
			snprintf(self->err_str, sizeof(self->err_str),
				 "roce_scc clear function module selection error.");
			return;
		}
	}
	self->err_no = hikp_roce_scc_get_total_data_num(&res_head, &offset, &data, &block_id);
	if (self->err_no) {
		snprintf(self->err_str, sizeof(self->err_str),
			 "get the first roce_scc block dfx fail.");
		hikp_roce_scc_reg_data_free(&offset, &data);
		return;
	}
	total_block_num = res_head.total_block_num;
	res_head.total_block_num = res_head.total_block_num - res_head.cur_block_num;
	offset_start = offset;
	data_start = data;
	if (res_head.total_block_num) {
		times = res_head.total_block_num / ROCE_HIKP_SCC_REG_NUM + 1;
		for (i = 0; i < times; i++) {
			offset = offset + res_head.cur_block_num;
			data = data + res_head.cur_block_num;
			data_size = res_head.total_block_num * sizeof(uint32_t);
			self->err_no = hikp_roce_scc_get_next_data(&res_head, &offset,
								   &data, &block_id, data_size);
			if (self->err_no) {
				snprintf(self->err_str, sizeof(self->err_str),
					 "get multiple roce_scc block dfx fail.");
				hikp_roce_scc_reg_data_free(&offset_start, &data_start);
				return;
			}
		}
	}
	hikp_roce_scc_print(total_block_num, offset_start, data_start);
	hikp_roce_scc_reg_data_free(&offset_start, &data_start);
}

static void cmd_roce_scc_init(void)
{
	struct major_cmd_ctrl *major_cmd = get_major_cmd();

	major_cmd->option_count = 0;
	major_cmd->execute = hikp_roce_scc_execute;

	cmd_option_register("-h", "--help", false, hikp_roce_scc_help);
	cmd_option_register("-i", "--interface", true, hikp_roce_scc_target);
	cmd_option_register("-m", "--module", true, hikp_roce_scc_module_select);
	cmd_option_register("-c", "--clear", false, hikp_roce_scc_clear_set);
}

HIKP_CMD_DECLARE("roce_scc", "get or clear roce_scc registers information", cmd_roce_scc_init);
