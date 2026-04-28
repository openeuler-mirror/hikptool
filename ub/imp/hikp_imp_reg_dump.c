/*
 * Copyright (c) 2026 Hisilicon Technologies Co., Ltd.
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

#include "hikp_imp_reg_dump.h"
#include "hikptdev_plug.h"

static const char *g_imp_dfx_32b_reg_list[] = {
	"DEVICE_RAS_STATUS_0",
	"IMP_NS_CFG",
	"IMP_DTCM1_1BIT_ERR_CNT",
	"IMP_DTCM0_1BIT_ERR_CNT",
	"CMDQ_MEM_1BIT_ERR_CNT",
	"MGTQ_MEM_1BIT_ERR_CNT",
	"IMP_DTCM1_2BIT_ERR_CNT",
	"IMP_DTCM0_2BIT_ERR_CNT",
	"CMDQ_MEM_2BIT_ERR_CNT",
	"MGTQ_MEM_2BIT_ERR_CNT",
	"IMP_AHB_SLV_ERR",
	"IMP_AXI_MST_ERR",
	"IMP_BUS_ERR_FUNID",
	"IMP_DTCM1_ECC_ERR",
	"IMP_DTCM0_ECC_ERR",
	"CMDQ_MEM_ECC_ERR",
	"MGTQ_MEM_ECC_ERR",
	"IMP_ABNORMAL_STATUS",
	"DFX_IMP_CMT_VLD_0",
	"DFX_IMP_CMT_VLD_1",
	"DFX_IMP_CMT_PC_0",
	"DFX_IMP_CMT_PC_1",
	"DFX_IMP_MCU",
	"DFX_IMP_MCU_AW_STATUS",
	"DFX_IMP_MCU_AR_STATUS",
	"DFX_IMP_MCU_AW_CNT",
	"DFX_IMP_MCU_AR_CNT",
	"DFX_IMP_MCU_ICACHE_CNT",
	"DFX_IMP2MST_AW_CNT",
	"DFX_IMP2MST_AR_CNT",
	"DFX_IMP2TAI_TWQE_W_CNT",
	"DFX_IMP_MCU_LAST_AW",
	"DFX_IMP_MCU_LAST_AR",
	"DFX_IMP_MST_LAST_AW_LOW",
	"DFX_IMP_MST_LAST_AW_HIGH",
	"DFX_IMP_MST_LAST_AR_LOW",
	"DFX_IMP_MST_LAST_AR_HIGH",
	"DFX_IMP_HANDSHAKE_STATUS",
	"DFX_IMP_CLK_ICG_E_STATUS",
	"DFX_IMP_AW_0",
	"DFX_IMP_AW_1",
	"DFX_IMP_AW_2",
	"DFX_IMP_AW_3",
	"DFX_IMP_AW_4",
	"DFX_IMP_AW_5",
	"DFX_IMP_AW_6",
	"DFX_IMP_AW_7",
	"DFX_IMP_AR_0",
	"DFX_IMP_AR_1",
	"DFX_IMP_AR_2",
	"DFX_IMP_AR_3",
	"DFX_IMP_AR_4",
	"DFX_IMP_AR_5",
	"DFX_IMP_AR_6",
	"DFX_IMP_AR_7",
	"IMP_DBG_RSV_REG_0",
	"IMP_DBG_RSV_REG_1",
	"IMP_DBG_RSV_REG_2",
	"IMP_DBG_RSV_REG_3",
	"IMP_DBG_RSV_REG_4",
	"IMP_DBG_RSV_REG_5",
	"IMP_DBG_RSV_REG_6",
	"IMP_DBG_RSV_REG_7",
	"IMP_DBG_RSV_REG_8",
	"IMP_DBG_RSV_REG_9",
	"IMP_DBG_RSV_REG_10",
	"IMP_DBG_RSV_REG_11",
	"IMP_DBG_RSV_REG_12",
	"IMP_DBG_RSV_REG_13",
	"IMP_DBG_RSV_REG_14",
	"IMP_DBG_RSV_REG_15",
	"IMP_DBG_RSV_REG_16",
	"IMP_DBG_RSV_REG_17",
	"IMP_DBG_RSV_REG_18",
	"IMP_DBG_RSV_REG_19",
	"IMP_DBG_RSV_REG_20",
	"IMP_DBG_RSV_REG_21",
	"IMP_DBG_RSV_REG_22",
	"IMP_DBG_RSV_REG_23",
	"IMP_DBG_RSV_REG_24",
	"IMP_DBG_RSV_REG_25",
	"IMP_DBG_RSV_REG_26",
	"IMP_DBG_RSV_REG_27",
	"IMP_DBG_RSV_REG_28",
	"IMP_DBG_RSV_REG_29",
	"IMP_DBG_RSV_REG_30",
	"IMP_DBG_RSV_REG_31",
	"DFX_DM_CFGSPACE_RSV_REG_0",
	"DFX_DM_CFGSPACE_RSV_REG_1",
	"DFX_DM_CFGSPACE_RSV_REG_2",
	"DFX_DM_CFGSPACE_RSV_REG_3",
	"DFX_DM_CFGSPACE_RSV_REG_4",
	"DFX_DM_CFGSPACE_RSV_REG_5",
	"DFX_DM_CFGSPACE_RSV_REG_6",
	"DFX_DM_CFGSPACE_RSV_REG_7",
	"DFX_DM_CFGSPACE_RSV_REG_8",
	"DFX_DM_CFGSPACE_RSV_REG_9",
	"DFX_DM_CFGSPACE_RSV_REG_10",
	"DFX_DM_CFGSPACE_RSV_REG_11",
	"DFX_DM_CFGSPACE_RSV_REG_12",
	"DFX_DM_CFGSPACE_RSV_REG_13",
	"DFX_DM_CFGSPACE_RSV_REG_14",
	"DFX_DM_CFGSPACE_RSV_REG_15",
	"DFX_MASTER_AMB0_WR_REQ_CNT",
	"DFX_MASTER_AMB0_RD_REQ_CNT",
	"DFX_MASTER_AMB0_AWACK_CNT",
	"DFX_MASTER_AMB0_WLAST_CNT",
	"DFX_MASTER_AMB0_BRESP_CNT",
	"DFX_MASTER_AMB0_RD_OUTSTANDING",
	"DFX_MASTER_AMB0_LAST_WR_H",
	"DFX_MASTER_AMB0_LAST_WR_L",
	"DFX_MASTER_AMB0_LAST_RD_H",
	"DFX_MASTER_AMB0_LAST_RD_L",
	"DFX_MASTER_AMB0_BKPR_STS",
	"DFX_MASTER_AMB1_WR_REQ_CNT",
	"DFX_MASTER_AMB1_RD_REQ_CNT",
	"DFX_MASTER_AMB1_AWACK_CNT",
	"DFX_MASTER_AMB1_WLAST_CNT",
	"DFX_MASTER_AMB1_BRESP_CNT",
	"DFX_MASTER_AMB1_RD_OUTSTANDING",
	"DFX_MASTER_AMB1_LAST_WR_H",
	"DFX_MASTER_AMB1_LAST_WR_L",
	"DFX_MASTER_AMB1_LAST_RD_H",
	"DFX_MASTER_AMB1_LAST_RD_L",
	"DFX_MASTER_AMB1_BKPR_STS",
};

static const char *g_imp_dfx_64b_reg_list[] = {
	/* Currently, there are no supported registers; reserved. */
};

static int dump_reg_get_blk_data(struct hikp_cmd_ret **cmd_ret, uint32_t blk_id,
				 struct imp_cmd_cfg *cmd_cfg)
{
	struct imp_dump_reg_rsp_data *reg_resp = NULL;
	struct hikp_cmd_header req_header = {0};
	struct imp_dump_req_para dump_req = {0};
	int ret;

	dump_req.chip = cmd_cfg->chip;
	dump_req.die = cmd_cfg->die;
	dump_req.block_id = blk_id;
	hikp_cmd_init(&req_header, IMP_MOD, IMP_MOD_DUMP_REG_CMD, DUMP_DFX_REG);
	*cmd_ret = hikp_cmd_alloc(&req_header, &dump_req, sizeof(dump_req));
	ret = hikp_rsp_normal_check(*cmd_ret);
	if (ret)
		return ret;

	reg_resp = (struct imp_dump_reg_rsp_data *)((*cmd_ret)->rsp_data);
	return (int)reg_resp->ret_code;
}

static void dump_reg_print_32b_reg(struct imp_dump_reg_rsp_data *reg_resp,
				   uint32_t *reg_cnt)
{
	uint32_t reg_32b_size = HIKP_ARRAY_SIZE(g_imp_dfx_32b_reg_list);
	uint32_t *reg_val = (uint32_t *)reg_resp->reg_data;
	uint32_t cur_reg_size;
	uint32_t i;

	cur_reg_size = reg_resp->cur_blk_size / sizeof(uint32_t);
	for (i = 0; i < cur_reg_size; i++) {
		if (*reg_cnt >= reg_32b_size)
			return;

		printf("%-45s: 0x%08x\n", g_imp_dfx_32b_reg_list[*reg_cnt], reg_val[i]);

		(*reg_cnt)++;
	}
}

static void dump_reg_print_64b_reg(struct imp_dump_reg_rsp_data *reg_resp,
				   uint32_t *reg_cnt)
{
	uint32_t reg_64b_size = HIKP_ARRAY_SIZE(g_imp_dfx_64b_reg_list);
	uint32_t cur_reg_size;
	uint32_t i;
	uint64_t *reg_val = (uint64_t *)reg_resp->reg_data;

	cur_reg_size = reg_resp->cur_blk_size / sizeof(uint64_t);

	for (i = 0; i < cur_reg_size; i++) {
		if (*reg_cnt >= reg_64b_size)
			return;

		printf("%-45s: 0x%016lx\n", g_imp_dfx_64b_reg_list[*reg_cnt], reg_val[i]);

		(*reg_cnt)++;
	}
}

static int dump_reg_get_first_blk(struct imp_cmd_cfg *cmd_cfg, uint16_t *total_blk,
				  uint32_t *reg_32b_cnt, uint32_t *reg_64b_cnt)
{
	struct imp_dump_reg_rsp_data *reg_resp = NULL;
	struct hikp_cmd_ret *cmd_ret = NULL;
	int ret;

	ret = dump_reg_get_blk_data(&cmd_ret, 0, cmd_cfg);
	if (ret)
		goto err_out;

	reg_resp = (struct imp_dump_reg_rsp_data *)(cmd_ret->rsp_data);

	*total_blk = reg_resp->total_blk_num;

	if (reg_resp->total_blk_num == 0 ||
	    reg_resp->cur_blk_size == 0 ||
	    reg_resp->cur_blk_size > sizeof(reg_resp->reg_data)) {
		printf("total_blk_num: %u or cur_blk_size: %u is invalid\n",
		       reg_resp->total_blk_num, reg_resp->cur_blk_size);
		ret = -EINVAL;
		goto err_out;
	}

	printf("hardware version: %u\n", reg_resp->hw_version);
	printf("---------------------------------------------------------\n");

	if (reg_resp->reg_width == WIDTH_32_BIT)
		dump_reg_print_32b_reg(reg_resp, reg_32b_cnt);
	else
		dump_reg_print_64b_reg(reg_resp, reg_64b_cnt);

err_out:
	hikp_cmd_free(&cmd_ret);

	return ret;
}

static int dump_reg_get_remain_data(uint32_t blk_id,
				    struct imp_cmd_cfg *cmd_cfg,
				    uint32_t *reg_32b_cnt, uint32_t *reg_64b_cnt)
{
	struct imp_dump_reg_rsp_data *reg_resp = NULL;
	struct hikp_cmd_ret *cmd_ret = NULL;
	int ret;

	ret = dump_reg_get_blk_data(&cmd_ret, blk_id, cmd_cfg);
	if (ret)
		goto err_out;

	reg_resp = (struct imp_dump_reg_rsp_data *)(cmd_ret->rsp_data);

	if (reg_resp->total_blk_num == 0 ||
	    reg_resp->cur_blk_size == 0 ||
	    reg_resp->cur_blk_size > sizeof(reg_resp->reg_data)) {
		printf("total_blk_num: %u or cur_blk_size: %u is invalid\n",
		       reg_resp->total_blk_num, reg_resp->cur_blk_size);
		ret = -EINVAL;
		goto err_out;
	}

	if (reg_resp->reg_width == WIDTH_32_BIT)
		dump_reg_print_32b_reg(reg_resp, reg_32b_cnt);
	else
		dump_reg_print_64b_reg(reg_resp, reg_64b_cnt);

err_out:
	hikp_cmd_free(&cmd_ret);

	return ret;
}

void hikp_imp_dump_dfx_reg(struct major_cmd_ctrl *self, struct imp_cmd_cfg *cmd_cfg)
{
	uint32_t reg_32b_cnt = 0;
	uint32_t reg_64b_cnt = 0;
	uint16_t total_blk = 0;
	uint32_t i;

	self->err_no = dump_reg_get_first_blk(cmd_cfg, &total_blk, &reg_32b_cnt, &reg_64b_cnt);
	if (self->err_no) {
		snprintf(self->err_str, sizeof(self->err_str), "get the first block data failed.");
		return;
	}

	for (i = 1; i < total_blk; i++) {
		self->err_no = dump_reg_get_remain_data(i, cmd_cfg, &reg_32b_cnt, &reg_64b_cnt);
		if (self->err_no) {
			snprintf(self->err_str, sizeof(self->err_str),
				 "getting block%u reg data failed.", i);
			return;
		}
	}
}
