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

#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <unistd.h>
#include "tool_cmd.h"
#include "hikp_net_lib.h"
#include "hikp_nic_dfx.h"

#define dfx_get_max_reg_bffer_size(rsp_head) \
	(uint32_t)((rsp_head)->total_blk_num * MAX_DFX_DATA_NUM * sizeof(uint32_t))

struct nic_dfx_param g_dfx_param = { 0 };

static const struct dfx_module_cmd g_dfx_module_parse[] = {
	{"SSU", SSU_DFX_REG_DUMP},
	{"IGU_EGU", IGU_EGU_DFX_REG_DUMP},
	{"PPP", PPP_DFX_REG_DUMP},
	{"NCSI", NCSI_DFX_REG_DUMP},
	{"BIOS", BIOS_COMM_DFX_REG_DUMP},
	{"RCB", RCB_DFX_REG_DUMP},
	{"TXDMA", TXDMA_DFX_REG_DUMP},
	{"MASTER", MASTER_DFX_REG_DUMP},
};

static const struct dfx_type_parse g_dfx_type_parse[] = {
	{INCORRECT_REG_TYPE, WIDTH_32_BIT, "INCORRECT TYPE"},
	{TYPE_32_RX_ERROR_STATS, WIDTH_32_BIT, "32 bit RX error statistics"},
	{TYPE_32_RX_DROP_STATS, WIDTH_32_BIT, "32 bit RX drop statistics"},
	{TYPE_32_RX_NORMAL_STATS, WIDTH_32_BIT, "32 bit RX statistics"},
	{TYPE_32_TX_ERROR_STATS, WIDTH_32_BIT, "32 bit TX error statistics"},
	{TYPE_32_TX_DROP_STATS, WIDTH_32_BIT, "32 bit TX drop statistics"},
	{TYPE_32_TX_NORMAL_STATS, WIDTH_32_BIT, "32 bit TX statistics"},
	{TYPE_32_ERROR_STATUS, WIDTH_32_BIT, "32 bit error status"},
	{TYPE_32_RUNNING_STATUS, WIDTH_32_BIT, "32 bit running status"},
	{TYPE_32_CFG_STATUS, WIDTH_32_BIT, "32 bit config status"},
	{TYPE_32_RX_PORT_ERROR_STATS, WIDTH_32_BIT, "32 bit RX port error statistics"},
	{TYPE_32_RX_PORT_DROP_STATS, WIDTH_32_BIT, "32 bit RX port drop statistics"},
	{TYPE_32_RX_PORT_NORMAL_STATS, WIDTH_32_BIT, "32 bit RX port statistics"},
	{TYPE_32_TX_PORT_ERROR_STATS, WIDTH_32_BIT, "32 bit TX port error statistics"},
	{TYPE_32_TX_PORT_DROP_STATS, WIDTH_32_BIT, "32 bit TX port drop statistics"},
	{TYPE_32_TX_PORT_NORMAL_STATS, WIDTH_32_BIT, "32 bit TX port statistics"},
	{TYPE_32_PORT_ERROR_STATUS, WIDTH_32_BIT, "32 bit port error status"},
	{TYPE_32_PORT_RUNNING_STATUS, WIDTH_32_BIT, "32 bit port running status"},
	{TYPE_32_PORT_CFG_STATUS, WIDTH_32_BIT, "32 bit port config status"},
	{TYPE_32_COMM_STATS, WIDTH_32_BIT, "32 bit common statistics"},
	{TYPE_32_COMM_DROP_STATS, WIDTH_32_BIT, "32 bit common drop statistics"},
	{TYPE_32_COMM_ERROR_STATS, WIDTH_32_BIT, "32 bit common error statistics"},
	{TYPE_64_RX_ERROR_STATS, WIDTH_64_BIT, "64 bit RX error statistics"},
	{TYPE_64_RX_DROP_STATS, WIDTH_64_BIT, "64 bit RX drop statistics"},
	{TYPE_64_RX_NORMAL_STATS, WIDTH_64_BIT, "64 bit RX statistics"},
	{TYPE_64_TX_ERROR_STATS, WIDTH_64_BIT, "64 bit TX error statistics"},
	{TYPE_64_TX_DROP_STATS, WIDTH_64_BIT, "64 bit TX drop statistics"},
	{TYPE_64_TX_NORMAL_STATS, WIDTH_64_BIT, "64 bit TX statistics"},
	{TYPE_64_RX_PORT_ERROR_STATS, WIDTH_64_BIT, "64 bit RX port error statistics"},
	{TYPE_64_RX_PORT_DROP_STATS, WIDTH_64_BIT, "64 bit RX port drop statistics"},
	{TYPE_64_RX_PORT_NORMAL_STATS, WIDTH_64_BIT, "64 bit RX port statistics"},
	{TYPE_64_TX_PORT_ERROR_STATS, WIDTH_64_BIT, "64 bit TX port error statistics"},
	{TYPE_64_TX_PORT_DROP_STATS, WIDTH_64_BIT, "64 bit TX port drop statistics"},
	{TYPE_64_TX_PORT_NORMAL_STATS, WIDTH_64_BIT, "64 bit TX port statistics"},
	{TYPE_64_COMM_STATS, WIDTH_64_BIT, "64 bit common statistics"},
	{TYPE_64_COMM_DROP_STATS, WIDTH_64_BIT, "64 bit common drop statistics"},
	{TYPE_64_COMM_ERROR_STATS, WIDTH_64_BIT, "64 bit common error statistics"},
	{TYPE_64_TX_PF_ERROR_STATS, WIDTH_64_BIT, "64 bit TX pf error statistics"},
	{TYPE_64_TX_PF_DROP_STATS, WIDTH_64_BIT, "64 bit TX pf drop statistics"},
	{TYPE_64_TX_PF_NORMAL_STATS, WIDTH_64_BIT, "64 bit TX pf statistics"},
};

static void dfx_help_info(const struct major_cmd_ctrl *self)
{
	printf("\n  Usage: %s %s\n", self->cmd_ptr->name, "-i <interface>\n");
	printf("\n         %s\n", self->cmd_ptr->help_info);
	printf("  Options:\n\n");
	printf("    %s, %-25s %s\n", "-h", "--help", "display this help and exit");
	printf("    %s, %-25s %s\n", "-i", "--interface=<interface>", "device target, e.g. eth0~7");
	printf("    %s\n", "	[-m/--module SSU/IGU_EGU/PPP/NCSI/BIOS/RCB/TXDMA/MASTER] :"
	       "this is necessary param\n");
}

static int hikp_cmd_dfx_help(struct major_cmd_ctrl *self, const char *argv)
{
	dfx_help_info(self);
	return 0;
}

static int hikp_nic_cmd_dfx_target(struct major_cmd_ctrl *self, const char *argv)
{
	self->err_no = tool_check_and_get_valid_bdf_id(argv, &(g_dfx_param.target));
	if (self->err_no != 0) {
		snprintf(self->err_str, sizeof(self->err_str), "Unknown device %s.", argv);
		return self->err_no;
	}

	return 0;
}

static int hikp_nic_dfx_get_blk(struct hikp_cmd_ret **cmd_ret,
				uint32_t blk_id, uint32_t sub_cmd_code)
{
	struct nic_dfx_req_para req_data = { 0 };
	struct hikp_cmd_header req_header = { 0 };

	req_data.bdf = g_dfx_param.target.bdf;
	req_data.block_id = blk_id;
	hikp_cmd_init(&req_header, NIC_MOD, GET_DFX_INFO_CMD, sub_cmd_code);
	*cmd_ret = hikp_cmd_alloc(&req_header, &req_data, sizeof(req_data));

	return hikp_rsp_normal_check(*cmd_ret);
}

static int hikp_nic_get_first_blk_dfx(struct nic_dfx_rsp_head_t *rsp_head, uint32_t **reg_data,
				      uint32_t *max_dfx_size, uint32_t *version)
{
	struct nic_dfx_rsp_t *dfx_rsp = NULL;
	struct hikp_cmd_ret *cmd_ret = NULL;
	int ret;

	ret = hikp_nic_dfx_get_blk(&cmd_ret, 0, g_dfx_param.sub_cmd_code);
	if (ret < 0)
		goto err_out;

	dfx_rsp = (struct nic_dfx_rsp_t *)(cmd_ret->rsp_data);
	*version = cmd_ret->version;
	*rsp_head = dfx_rsp->rsp_head;
	if (rsp_head->total_blk_num == 0) {
		/* if total block number is zero, set total type number to zero anyway */
		rsp_head->total_type_num = 0;
		goto err_out;
	}
	*max_dfx_size = dfx_get_max_reg_bffer_size(rsp_head);
	*reg_data = (uint32_t *)calloc(1, *max_dfx_size);
	if (*reg_data == NULL) {
		HIKP_ERROR_PRINT("malloc log memory 0x%x failed.\n", *max_dfx_size);
		ret = -ENOMEM;
		goto err_out;
	}

	if (rsp_head->cur_blk_size > *max_dfx_size) {
		free(*reg_data);
		*reg_data = NULL;
		HIKP_ERROR_PRINT("blk0 reg_data copy size error, data size: 0x%x, max size: 0x%x\n",
				 rsp_head->cur_blk_size, *max_dfx_size);
		ret = -EINVAL;
		goto err_out;
	}
	memcpy(*reg_data, dfx_rsp->reg_data, rsp_head->cur_blk_size);

	*max_dfx_size -= (uint32_t)rsp_head->cur_blk_size;
err_out:
	hikp_cmd_free(&cmd_ret);

	return ret;
}

static int hikp_nic_get_blk_dfx(struct nic_dfx_rsp_head_t *rsp_head, uint32_t blk_id,
				uint32_t *reg_data, uint32_t *max_dfx_size)
{
	struct nic_dfx_rsp_t *dfx_rsp = NULL;
	struct hikp_cmd_ret *cmd_ret = NULL;
	int ret;

	ret = hikp_nic_dfx_get_blk(&cmd_ret, blk_id, g_dfx_param.sub_cmd_code);
	if (ret < 0)
		goto err_out;

	dfx_rsp = (struct nic_dfx_rsp_t *)(cmd_ret->rsp_data);
	*rsp_head = dfx_rsp->rsp_head;
	if (rsp_head->cur_blk_size > *max_dfx_size) {
		HIKP_ERROR_PRINT("blk%u reg_data copy size error, "
				 "data size: 0x%x, max size: 0x%x\n",
				 blk_id, rsp_head->cur_blk_size, *max_dfx_size);
		ret = -EINVAL;
		goto err_out;
	}
	memcpy(reg_data, dfx_rsp->reg_data, rsp_head->cur_blk_size);
	*max_dfx_size -= (uint32_t)rsp_head->cur_blk_size;

err_out:
	hikp_cmd_free(&cmd_ret);

	return ret;
}

static int cmd_dfx_module_select(struct major_cmd_ctrl *self, const char *argv)
{
	size_t arr_size = HIKP_ARRAY_SIZE(g_dfx_module_parse);
	bool is_found;
	size_t i;

	for (i = 0; i < arr_size; i++) {
		is_found = strncmp(argv, (const char *)g_dfx_module_parse[i].module_name,
						   sizeof(g_dfx_module_parse[i].module_name)) == 0;
		if (is_found) {
			g_dfx_param.sub_cmd_code = g_dfx_module_parse[i].sub_cmd_code;
			g_dfx_param.module_idx = i;
			g_dfx_param.flag |= MODULE_SET_FLAG;
			return 0;
		}
	}
	dfx_help_info(self);
	snprintf(self->err_str, sizeof(self->err_str), "-m/--module param error!!!");
	self->err_no = -EINVAL;

	return -EINVAL;
}

static void hikp_nic_dfx_print_b32(uint32_t num, uint32_t *reg_data)
{
	uint32_t word_num = num * WORD_NUM_PER_REG;
	uint16_t offset;
	uint32_t value;
	uint32_t index;
	uint32_t i;

	for (i = 0, index = 1; i < word_num; i = i + WORD_NUM_PER_REG, index++) {
		offset = (uint16_t)HI_GET_BITFIELD(reg_data[i], 0, DFX_REG_ADDR_MASK);
		value = reg_data[i + 1];
		printf("%03u: 0x%04x\t0x%08x\n", index, offset, value);
	}
}

static void hikp_nic_dfx_print_b64(uint32_t num, uint32_t *reg_data)
{
	uint32_t word_num = num * WORD_NUM_PER_REG;
	uint16_t offset;
	uint64_t value;
	uint32_t index;
	uint32_t i;

	for (i = 0, index = 1; i < word_num; i = i + WORD_NUM_PER_REG, index++) {
		offset = (uint16_t)HI_GET_BITFIELD(reg_data[i], 0, DFX_REG_ADDR_MASK);
		value = (uint64_t)reg_data[i + 1] |
			(HI_GET_BITFIELD((uint64_t)reg_data[i], DFX_REG_VALUE_OFF,
			DFX_REG_VALUE_MASK) << BIT_NUM_OF_WORD);
		printf("%03u: 0x%04x\t0x%016lx\n", index, offset, value);
	}
}

static bool is_type_found(uint16_t type_id, uint32_t *index)
{
	size_t arr_size = HIKP_ARRAY_SIZE(g_dfx_type_parse);
	size_t i;

	for (i = 0; i < arr_size; i++) {
		if (g_dfx_type_parse[i].type_id == type_id) {
			*index = i;
			return true;
		}
	}

	return false;
}

static void hikp_nic_dfx_print_type_head(uint8_t type_id, uint8_t *last_type_id)
{
	uint32_t index = 0;

	if (type_id != *last_type_id) {
		printf("-----------------------------------------------------\n");
		if (is_type_found(type_id, &index))
			printf("type name: %s\n\n", g_dfx_type_parse[index].type_name);
		else
			HIKP_WARN_PRINT("type name: unknown type, type id is %u\n\n", type_id);

		*last_type_id = type_id;
	}
}

static void hikp_nic_dfx_print(const struct nic_dfx_rsp_head_t *rsp_head, uint32_t *reg_data)
{
	struct nic_dfx_type_head *type_head;
	uint8_t last_type_id = 0;
	uint32_t *ptr = reg_data;
	uint32_t max_size;
	uint32_t num_u32;
	uint8_t i;

	max_size = dfx_get_max_reg_bffer_size(rsp_head);
	for (i = 0; i < rsp_head->total_type_num; i++) {
		type_head = (struct nic_dfx_type_head *)ptr;
		num_u32 = type_head->reg_num * WORD_NUM_PER_REG + 1; /* including type_head */
		if (max_size < num_u32 * sizeof(uint32_t)) {
			HIKP_ERROR_PRINT("register real size exceeds the max size\n");
			return;
		}
		ptr += num_u32;
		max_size -= num_u32 * sizeof(uint32_t);
	}

	ptr = reg_data;
	printf("****************** module %s reg dump start ********************\n",
		g_dfx_module_parse[g_dfx_param.module_idx].module_name);
	for (i = 0; i < rsp_head->total_type_num; i++) {
		type_head = (struct nic_dfx_type_head *)ptr;
		if (type_head->type_id == INCORRECT_REG_TYPE) {
			HIKP_ERROR_PRINT("No.%u type is incorrect reg type\n", i + 1u);
			break;
		}
		hikp_nic_dfx_print_type_head(type_head->type_id, &last_type_id);
		ptr++;
		if (type_head->bit_width == WIDTH_32_BIT) {
			hikp_nic_dfx_print_b32((uint32_t)type_head->reg_num, ptr);
		} else if (type_head->bit_width == WIDTH_64_BIT) {
			hikp_nic_dfx_print_b64((uint32_t)type_head->reg_num, ptr);
		} else {
			HIKP_ERROR_PRINT("type%u's bit width error.\n", type_head->type_id);
			break;
		}
		ptr += (uint32_t)type_head->reg_num * WORD_NUM_PER_REG;
	}
	printf("################### ====== dump end ====== ######################\n");
}

static void hikp_nic_dfx_cmd_execute(struct major_cmd_ctrl *self)
{
	struct nic_dfx_rsp_head_t rsp_head = { 0 };
	struct nic_dfx_rsp_head_t tmp_head = { 0 };
	uint32_t *reg_data = NULL;
	uint32_t real_reg_size;
	uint32_t max_dfx_size;
	uint32_t version;
	uint32_t i;

	if (!(g_dfx_param.flag & MODULE_SET_FLAG)) {
		self->err_no = -EINVAL;
		snprintf(self->err_str, sizeof(self->err_str), "Please specify a module.");
		dfx_help_info(self);
		return;
	}
	self->err_no = hikp_nic_get_first_blk_dfx(&rsp_head, &reg_data, &max_dfx_size, &version);
	if (self->err_no != 0) {
		snprintf(self->err_str, sizeof(self->err_str), "get the first block dfx fail.");
		return;
	}
	real_reg_size = (uint32_t)rsp_head.cur_blk_size;
	for (i = 1; i < rsp_head.total_blk_num; i++) {
		self->err_no = hikp_nic_get_blk_dfx(&tmp_head, i,
						    reg_data + (real_reg_size / sizeof(uint32_t)),
						    &max_dfx_size);
		if (self->err_no != 0) {
			snprintf(self->err_str, sizeof(self->err_str),
				 "getting block%u reg fail.", i);
			free(reg_data);
			return;
		}
		real_reg_size += (uint32_t)tmp_head.cur_blk_size;
		memset(&tmp_head, 0, sizeof(struct nic_dfx_rsp_head_t));
	}

	printf("DFX cmd version: 0x%x\n\n", version);
	hikp_nic_dfx_print((const struct nic_dfx_rsp_head_t *)&rsp_head, reg_data);
	free(reg_data);
}

static void cmd_nic_dfx_init(void)
{
	struct major_cmd_ctrl *major_cmd = get_major_cmd();

	major_cmd->option_count = 0;
	major_cmd->execute = hikp_nic_dfx_cmd_execute;

	cmd_option_register("-h", "--help", false, hikp_cmd_dfx_help);
	cmd_option_register("-i", "--interface", true, hikp_nic_cmd_dfx_target);
	cmd_option_register("-m", "--module", true, cmd_dfx_module_select);
}

HIKP_CMD_DECLARE("nic_dfx", "dump dfx info of hardware", cmd_nic_dfx_init);
