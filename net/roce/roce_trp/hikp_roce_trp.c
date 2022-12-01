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
	       "COMMON : 0~3\n PAYL: 0~1\n GEN_AC : 0~1\n ");
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
	uint32_t temp;

	temp = g_roce_trp_param_t.bank_id;
	switch (g_roce_trp_param_t.sub_cmd) {
	case (COMMON):
		if (temp > TRP_MAX_BANK_NUM || temp < 0)
			return -EINVAL;
		break;
	case (PAYL):
		if (temp > PAYL_MAX_BANK_NUM || temp < 0)
			return -EINVAL;
		break;
	case (GEN_AC):
		if (temp > GAC_MAX_BANK_NUM || temp < 0)
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

	req_data.block_id = *block_id;
	req_data.bdf = g_roce_trp_param_t.target.bdf;
	req_data.bank_id = g_roce_trp_param_t.bank_id;
	if (g_roce_trp_param_t.sub_cmd == 0) {
		printf("please enter module name: -m/--modlue\n");
		return -EINVAL;
	}
	hikp_cmd_init(&req_header, ROCE_MOD, GET_ROCEE_TRP_CMD, g_roce_trp_param_t.sub_cmd);
	*cmd_ret = hikp_cmd_alloc(&req_header, &req_data, sizeof(req_data));
	if (*cmd_ret == NULL) {
		printf("hikptool roce_trp cmd_ret malloc failed\n");
		return -EIO;
	}

	return 0;
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
		printf("hikptool roce_trp alloc log memmory 0x%x failed\n", max_size);
		ret = -ENOMEM;
		goto get_data_error;
	}

	cur_size = roce_trp_res->head.cur_block_num * sizeof(uint32_t);
	if (cur_size > max_size) {
		printf("hikptool roce_trp log data copy size error, "
		       "data size: 0x%x, max size: 0x%x\n", cur_size, max_size);
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
		       "data size: 0x%x, max size: 0x%x\n", cur_size, data_size);
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

static void hikp_roce_trp_print(uint8_t total_block_num,
				const uint32_t *offset, const uint32_t *data)
{
	uint32_t i;

	printf("**************TRP INFO*************\n");
	for (i = 0; i < total_block_num; i++)
		printf("[0x%08X] : 0x%08X\n", offset[i], data[i]);
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
		hikp_roce_trp_reg_data_free(&offset, &data);
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
