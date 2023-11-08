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

#include "hikp_roce_ext_common.h"

static void hikp_roce_ext_reg_data_free(struct reg_data *reg)
{
	if (reg->offset) {
		free(reg->offset);
		reg->offset = NULL;
	}

	if (reg->data) {
		free(reg->data);
		reg->data = NULL;
	}
}

static void hikp_roce_ext_cmd_ret_free(struct hikp_cmd_ret *cmd_ret)
{
	if (cmd_ret)
		free(cmd_ret);
}

static const struct cmd_type_info {
	enum roce_cmd_type cmd_type;
	const char *cmd_name;
	uint8_t reg_array_length;
} cmd_info_table[] = {
	{GET_ROCEE_MDB_CMD, "MDB", ROCE_HIKP_MDB_REG_NUM_EXT},
	{GET_ROCEE_GMV_CMD, "GMV", ROCE_HIKP_GMV_REG_NUM_EXT},
	{GET_ROCEE_CAEP_CMD, "CAEP", ROCE_HIKP_CAEP_REG_NUM_EXT},
	{GET_ROCEE_PKT_CMD, "PKT", ROCE_HIKP_PKT_REG_NUM_EXT},
	{GET_ROCEE_SCC_CMD, "SCC", ROCE_HIKP_SCC_REG_NUM_EXT},
	{GET_ROCEE_QMM_CMD, "QMM", ROCE_HIKP_QMM_REG_NUM_EXT},
	{GET_ROCEE_TIMER_CMD, "TIMER", ROCE_HIKP_TIMER_REG_NUM_EXT},
	{GET_ROCEE_TRP_CMD, "TRP", ROCE_HIKP_TRP_REG_NUM_EXT},
	{GET_ROCEE_TSP_CMD, "TSP", ROCE_HIKP_TSP_REG_NUM_EXT},
	{GET_ROCEE_RST_CMD, "RST", ROCE_HIKP_RST_REG_NUM},
	{GET_ROCEE_GLOBAL_CFG_CMD, "GLOBAL_CFG", ROCE_HIKP_GLOBAL_CFG_REG_NUM},
	{GET_ROCEE_BOND_CMD, "BOND", ROCE_HIKP_BOND_REG_NUM},
};

static int get_cmd_info_table_idx(enum roce_cmd_type cmd_type)
{
	int array_size = sizeof(cmd_info_table) / sizeof(struct cmd_type_info);
	int i;

	for (i = 0; i < array_size; i++)
		if (cmd_type == cmd_info_table[i].cmd_type)
			return i;

	return -ENOENT;
}

static const char *get_cmd_name(enum roce_cmd_type cmd_type)
{
	int idx;

	idx = get_cmd_info_table_idx(cmd_type);
	if (idx >= 0)
		return cmd_info_table[idx].cmd_name;

	printf("Failed to get cmd name, cmd_type = %d\n", cmd_type);
	return NULL;
}

static int get_cmd_reg_array_length(enum roce_cmd_type cmd_type)
{
	int idx;

	idx = get_cmd_info_table_idx(cmd_type);
	if (idx >= 0)
		return cmd_info_table[idx].reg_array_length;

	printf("Failed to get cmd reg array length, cmd_type = %d\n", cmd_type);
	return idx;
}

static int hikp_roce_ext_get_res(enum roce_cmd_type cmd_type,
				 uint32_t block_id,
				 struct roce_ext_head *res_head,
				 struct reg_data *reg,
				 int (*get_data)(struct hikp_cmd_ret **cmd_ret,
						 uint32_t block_id))
{
	int reg_array_length = get_cmd_reg_array_length(cmd_type);
	const char *cmd_name = get_cmd_name(cmd_type);
	struct roce_ext_res_param *roce_ext_res;
	struct hikp_cmd_ret *cmd_ret;
	size_t max_size;
	size_t cur_size;
	int ret;

	/* reg_array_length greater than or equal to 0 ensures that cmd_name
	 * is not NULL, so cmd_name does not need to be checked.
	 */
	if (reg_array_length < 0)
		return reg_array_length;

	ret = get_data(&cmd_ret, block_id);
	if (ret) {
		printf("hikptool roce_%s get data failed!\n", cmd_name);
		goto get_data_error;
	}

	roce_ext_res = (struct roce_ext_res_param *)cmd_ret->rsp_data;
	*res_head = roce_ext_res->head;
	max_size = res_head->total_block_num * sizeof(uint32_t);

	if (block_id == 0) {
		reg->offset = (uint32_t *)calloc(res_head->total_block_num, sizeof(uint32_t));
		reg->data = (uint32_t *)calloc(res_head->total_block_num, sizeof(uint32_t));
		if ((reg->offset == NULL) || (reg->data == NULL)) {
			printf("hikptool roce_%s alloc log memmory 0x%zx failed!\n",
				cmd_name, max_size);
			ret = -ENOMEM;
			hikp_roce_ext_reg_data_free(reg);
			goto get_data_error;
		}
	}

	cur_size = res_head->cur_block_num * sizeof(uint32_t);
	if (cur_size > max_size) {
		printf("hikptool roce_%s log data copy size error, data size: 0x%zx, max size: 0x%zx\n",
		       cmd_name, cur_size, max_size);
		ret = -EINVAL;
		hikp_roce_ext_reg_data_free(reg);
		goto get_data_error;
	}

	memcpy(reg->offset + block_id,
	       (uint32_t *)&roce_ext_res->reg_data, cur_size);
	memcpy(reg->data + block_id,
	       (uint32_t *)&roce_ext_res->reg_data + reg_array_length, cur_size);

get_data_error:
	hikp_roce_ext_cmd_ret_free(cmd_ret);
	return ret;
}

static void hikp_roce_ext_print(const char *cmd_name, uint32_t total_block_num,
				const uint32_t *offset, const uint32_t *data)
{
	int i;

	printf("**************%s INFO*************\n", cmd_name);
	for (i = 0; i < total_block_num; i++)
		printf("[0x%08X] : 0x%08X\n", offset[i], data[i]);
	printf("************************************\n");
}

void hikp_roce_ext_execute(struct major_cmd_ctrl *self,
			   enum roce_cmd_type cmd_type,
			   int (*get_data)(struct hikp_cmd_ret **cmd_ret,
					   uint32_t block_id))
{
	uint32_t queried_block_id = 0;
	struct roce_ext_head res_head;
	struct reg_data reg = { 0 };

	do {
		self->err_no = hikp_roce_ext_get_res(cmd_type, queried_block_id,
						     &res_head, &reg, get_data);
		if (self->err_no)
			return;

		queried_block_id += res_head.cur_block_num;
	} while (queried_block_id < res_head.total_block_num);

	hikp_roce_ext_print(get_cmd_name(cmd_type), res_head.total_block_num,
			    reg.offset, reg.data);

	hikp_roce_ext_reg_data_free(&reg);
}
