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
	printf("\n");

	return 0;
}

static int hikp_roce_qmm_target(struct major_cmd_ctrl *self, const char *argv)
{
	self->err_no = tool_check_and_get_valid_bdf_id(argv, &(g_roce_qmm_param.target));
	if (self->err_no != 0) {
		snprintf(self->err_str, sizeof(self->err_str), "Unknown device %s.", argv);
		return self->err_no;
	}

	return 0;
}

static int hikp_roce_qmm_bank_get(struct major_cmd_ctrl *self, const char *argv)
{
	char *endptr = NULL;
	int bank_num;

	bank_num = strtol(argv, &endptr, 0);
	if ((endptr <= argv) || (*endptr != '\0') || bank_num > QMM_BANK_NUM || bank_num < 0) {
		snprintf(self->err_str, sizeof(self->err_str), "Invalid bank number!\n");
		self->err_no = -EINVAL;
		return -EINVAL;
	}

	g_roce_qmm_param.bank_id = bank_num;
	return 0;
}

static void hikp_roce_qmm_print(struct roce_qmm_rsp_data *qmm_rsp)
{
	int index = 0;

	while (index < qmm_rsp->reg_num) {
		printf("0x%08X : 0x%08X\n", qmm_rsp->qmm_content[index][0],
		       qmm_rsp->qmm_content[index][1]);
		index++;
	}
	printf("***************************************\n");
}

static int hikp_roce_qmm_show_cqc(struct major_cmd_ctrl *self)
{
	struct roce_qmm_req_para req_data = { 0 };
	struct roce_qmm_rsp_data *qmm_rsp = NULL;
	struct hikp_cmd_header req_header = { 0 };
	struct hikp_cmd_ret *cmd_ret = NULL;
	int ret;

	req_data.bdf = g_roce_qmm_param.target.bdf;

	hikp_cmd_init(&req_header, ROCE_MOD, GET_ROCEE_QMM_CMD, QMM_SHOW_CQC);
	cmd_ret = hikp_cmd_alloc(&req_header, &req_data, sizeof(req_data));
	ret = hikp_rsp_normal_check(cmd_ret);
	if (ret != 0)
		goto out;

	qmm_rsp = (struct roce_qmm_rsp_data *)(cmd_ret->rsp_data);
	printf("**************QMM CQC INFO*************\n");
	hikp_roce_qmm_print(qmm_rsp);
out:
	free(cmd_ret);
	cmd_ret = NULL;
	return ret;
}

static int hikp_roce_qmm_show_qpc(struct major_cmd_ctrl *self)
{
	struct roce_qmm_req_para req_data = { 0 };
	struct roce_qmm_rsp_data *qmm_rsp = NULL;
	struct hikp_cmd_header req_header = { 0 };
	struct hikp_cmd_ret *cmd_ret = NULL;
	int ret;

	req_data.bdf = g_roce_qmm_param.target.bdf;
	req_data.bank_id = g_roce_qmm_param.bank_id;

	hikp_cmd_init(&req_header, ROCE_MOD, GET_ROCEE_QMM_CMD, QMM_SHOW_QPC);
	cmd_ret = hikp_cmd_alloc(&req_header, &req_data, sizeof(req_data));
	ret = hikp_rsp_normal_check(cmd_ret);
	if (ret != 0)
		goto out;

	qmm_rsp = (struct roce_qmm_rsp_data *)(cmd_ret->rsp_data);
	printf("**************QMM QPC INFO*************\n");
	hikp_roce_qmm_print(qmm_rsp);
out:
	free(cmd_ret);
	cmd_ret = NULL;
	return ret;
}

static int hikp_roce_qmm_show_top(struct major_cmd_ctrl *self)
{
	struct roce_qmm_req_para req_data = { 0 };
	struct roce_qmm_rsp_data *qmm_rsp = NULL;
	struct hikp_cmd_header req_header = { 0 };
	struct hikp_cmd_ret *cmd_ret = NULL;
	int ret;

	req_data.bdf = g_roce_qmm_param.target.bdf;

	hikp_cmd_init(&req_header, ROCE_MOD, GET_ROCEE_QMM_CMD, QMM_SHOW_TOP);
	cmd_ret = hikp_cmd_alloc(&req_header, &req_data, sizeof(req_data));
	ret = hikp_rsp_normal_check(cmd_ret);
	if (ret != 0)
		goto out;

	qmm_rsp = (struct roce_qmm_rsp_data *)(cmd_ret->rsp_data);
	printf("**************QMM TOP INFO*************\n");
	hikp_roce_qmm_print(qmm_rsp);
out:
	free(cmd_ret);
	cmd_ret = NULL;
	return ret;
}

static void hikp_roce_qmm_execute(struct major_cmd_ctrl *self)
{
	int (*func[])(struct major_cmd_ctrl *self) = {
		hikp_roce_qmm_show_cqc, hikp_roce_qmm_show_qpc, hikp_roce_qmm_show_top
	};
	char *function[] = {"show cqc", "show qpc", "show top"};
	int ret;

	for (int i = 0; i < HIKP_ARRAY_SIZE(func); i++) {
		ret = func[i](self);
		if (ret != 0) {
			self->err_no = -EINVAL;
			snprintf(self->err_str, sizeof(self->err_str),
				 "roce_qmm %s function failed\n", function[i]);
			break;
		}
	}
}

static void cmd_roce_qmm_init(void)
{
	struct major_cmd_ctrl *major_cmd = get_major_cmd();

	major_cmd->option_count = 0;
	major_cmd->execute = hikp_roce_qmm_execute;

	cmd_option_register("-h", "--help", false, hikp_roce_qmm_help);
	cmd_option_register("-i", "--interface", true, hikp_roce_qmm_target);
	cmd_option_register("-b", "--bank", true, hikp_roce_qmm_bank_get);
}

HIKP_CMD_DECLARE("roce_qmm", "get roce_qmm registers information", cmd_roce_qmm_init);
