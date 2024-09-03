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

#include "hikp_ub_crd.h"

static struct ub_crd_param g_ub_crd_param = { 0 };

static int hikp_ub_crd_help(struct major_cmd_ctrl *self, const char *argv)
{
	printf("\n  Usage: %s %s\n", self->cmd_ptr->name, "-i <interface>\n");
	printf("\n         %s\n", self->cmd_ptr->help_info);
	printf("  Options:\n\n");
	printf("    %s, %-25s %s\n", "-h", "--help", "display this help and exit");
	printf("    %s, %-25s %s\n", "-i", "--interface=<interface>",
	       "device target or bdf id, e.g. ubn0 or 0000:35:00.0");
	printf("\n");
	return 0;
}

static int hikp_ub_crd_target(struct major_cmd_ctrl *self, const char *argv)
{
	self->err_no = tool_check_and_get_valid_bdf_id(argv, &(g_ub_crd_param.target));
	if (self->err_no) {
		snprintf(self->err_str, sizeof(self->err_str), "Unknown device %s.", argv);
		return self->err_no;
	}
	return 0;
}

static uint32_t hikp_ub_show_crd(uint32_t off, struct ub_crd_rsp *crd_rsp, uint32_t num_rows,
				 char const *crds[][2])
{
	uint32_t reg_index;
	uint32_t i;

	for (i = 0; i < num_rows; i++) {
		union cut_reg reg;

		reg_index = off + i;
		reg.value = crd_rsp->cut_reg_value[reg_index];
		if (strcmp(crds[i][0], "NULL") != 0)
			printf("%-28s : %#x\n", crds[i][0], reg.cut[0]);

		if (strcmp(crds[i][1], "NULL") != 0)
			printf("%-28s : %#x\n", crds[i][1], reg.cut[1]);
	}
	return off + num_rows;
}

static int hikp_ub_query_crd(void)
{
	struct hikp_cmd_header req_header = { 0 };
	struct ub_crd_req_para req_data = { 0 };
	struct hikp_cmd_ret *cmd_ret = NULL;
	struct ub_crd_rsp *crd_rsp = NULL;
	uint32_t offset;

	char const *init_crds[][2] = {
		{"CFG_REMOTE_ICRD", "CFG_REMOTE_LCRD"},
		{"CFG_REMOTE_CRD_VL6", "CFG_REMOTE_CRD_VL7"},
		{"CFG_REMOTE_CRD_VL4", "CFG_REMOTE_CRD_VL5"},
		{"CFG_REMOTE_CRD_VL2", "CFG_REMOTE_CRD_VL3"},
		{"CFG_REMOTE_CRD_VL0", "CFG_REMOTE_CRD_VL1"},
		{"CFG_REMOTE_CRD_VL8", "NULL"} };

	char const *temp_crds[][2] = {
		{"TX_LCRD_VNA_EXIST_NUM", "NULL"}, {"TX_ICRD_VNA_EXIST_NUM", "NULL"},
		{"TX_CRD_VN0_EXIST_NUM", "NULL"}, {"TX_CRD_VN1_EXIST_NUM", "NULL"},
		{"TX_CRD_VN2_EXIST_NUM", "NULL"}, {"TX_CRD_VN3_EXIST_NUM", "NULL"},
		{"TX_CRD_VN4_EXIST_NUM", "NULL"}, {"TX_CRD_VN5_EXIST_NUM", "NULL"},
		{"TX_CRD_VN6_EXIST_NUM", "NULL"}, {"TX_CRD_VN7_EXIST_NUM", "NULL"},
		{"TX_ACK_EXIST_NUM", "NULL"}, {"TX_ROH_LCRD_LOCAL_NUM", "NULL"} };

	hikp_cmd_init(&req_header, UB_MOD, GET_UB_CRD_INFO_CMD, UB_CRD_INFO_DUMP);
	req_data.bdf = g_ub_crd_param.target.bdf;
	cmd_ret = hikp_cmd_alloc(&req_header, &req_data, sizeof(req_data));
	if (cmd_ret == NULL || cmd_ret->status != 0) {
		hikp_cmd_free(&cmd_ret);
		return -EIO;
	}

	crd_rsp = (struct ub_crd_rsp *)(cmd_ret->rsp_data);

	printf("******************** CREDIT CNT START ********************\n");
	printf("-------------------  INIT CREDIT START   -----------------\n");
	offset = hikp_ub_show_crd(0, crd_rsp, NUM_ROWS_INIT_CRDS, init_crds);
	printf("-------------------  INIT CREDIT END  --------------------\n");
	printf("-------------------  TEMP CREDIT START  ------------------\n");
	offset = hikp_ub_show_crd(offset, crd_rsp, NUM_ROWS_TEMP_CRDS, temp_crds);
	printf("-------------------  TEMP CREDIT END  --------------------\n");
	printf("********************* CREDIT CNT END *********************\n");

	hikp_cmd_free(&cmd_ret);
	return 0;
}

static void hikp_ub_crd_cmd_execute(struct major_cmd_ctrl *self)
{
	int ret;

	ret = hikp_ub_query_crd();
	if (ret != 0) {
		snprintf(self->err_str, sizeof(self->err_str), "fail to get ub crd info.");
		self->err_no = ret;
		return;
	}
}

static void cmd_ub_crd_init(void)
{
	struct major_cmd_ctrl *major_cmd = get_major_cmd();

	major_cmd->option_count = 0;
	major_cmd->execute = hikp_ub_crd_cmd_execute;

	cmd_option_register("-h", "--help", false, hikp_ub_crd_help);
	cmd_option_register("-i", "--interface", true, hikp_ub_crd_target);
}

HIKP_CMD_DECLARE("ub_crd", "get ub crd information", cmd_ub_crd_init);
