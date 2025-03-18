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
#include "hikp_roce_timer.h"

static struct cmd_roce_timer_params g_roce_timer_param = { 0 };

int hikp_roce_set_timer_bdf(char *nic_name)
{
	return tool_check_and_get_valid_bdf_id(nic_name,
					       &g_roce_timer_param.target);
}

static int hikp_roce_timer_help(struct major_cmd_ctrl *self, const char *argv)
{
	HIKP_SET_USED(argv);

	printf("\n  Usage: %s %s\n", self->cmd_ptr->name, "-i <interface>\n");
	printf("\n         %s\n", self->cmd_ptr->help_info);
	printf("  Options:\n\n");
	printf("    %s, %-25s %s\n", "-h", "--help", "display this help and exit");
	printf("    %s, %-25s %s\n", "-i", "--interface=<interface>", "device target, e.g. eth0");
	printf("    %s, %-25s %s\n", "-c", "--clear=<clear>", "clear timer registers");
	printf("\n");

	return 0;
}

static int hikp_roce_timer_target(struct major_cmd_ctrl *self, const char *argv)
{
	self->err_no = tool_check_and_get_valid_bdf_id(argv, &(g_roce_timer_param.target));
	if (self->err_no != 0) {
		snprintf(self->err_str, sizeof(self->err_str), "Unknown device %s.", argv);
		return self->err_no;
	}
	return 0;
}

static int hikp_roce_timer_clear_set(struct major_cmd_ctrl *self, const char *argv)
{
	HIKP_SET_USED(self);
	HIKP_SET_USED(argv);

	g_roce_timer_param.flag = ROCE_TIMER_CMD_CLEAR;
	return 0;
}

/* DON'T change the order of these arrays or add entries between! */
static const char *g_timer_qpc_reg_name[] = {
	"QPC_AXI_ERR",
	"QPC_SEARCH_CNT",
	"QPC_DB_SEND_CNT",
	"FIFO_FILL0",
	"FIFO_FILL1",
	"FIFO_FILL2",
	"FIFO_OVER_FLOW",
	"QPC_START_CNT",
	"QPC_DB_SEND_NUM_CNT",
	"ROCEE_TIMER_QPC_ECC_ERR",
	"ROCEE_TIMER_QPC_ECC_ERR_INFO",
	"START_TYPE_ERR_CNT",
};

static const char *g_timer_cqc_reg_name[] = {
	"TIMER_MEM_INIT_DONE",
	"CQC_AXI_ERR",
	"CQC_SEARCH_CNT",
	"CQC_DB_SEND_CNT",
	"CQC_FIFO_FILL0",
	"CQC_FIFO_FILL1",
	"CQC_FIFO_FILL2",
	"CQC_START_CNT",
	"CQC_DB_SEND_NUM_CNT",
	"FLR_DONE_STATE",
	"ZERO_ADDR_ACC",
	"ROCEE_TIMER_CQC_ECC_ERR",
	"ROCEE_TIMER_CQC_ECC_ERR_INFO",
	"TIMER_STA_0",
	"CQC_LOSE_DB_CNT",
	"TIMER_TDP_DONE_CNT",
	"CQC_PAGE_OVER_CNT",
};

static void hikp_roce_timer_print(struct roce_timer_rsp_data *timer_rsp,
				  enum roce_timer_cmd_type cmd_type)
{
	const char **reg_name;
	uint32_t index = 0;
	uint8_t arr_len;

	if (cmd_type == TIMER_SHOW_QPC) {
		reg_name = g_timer_qpc_reg_name;
		arr_len = HIKP_ARRAY_SIZE(g_timer_qpc_reg_name);
	} else {
		reg_name = g_timer_cqc_reg_name;
		arr_len = HIKP_ARRAY_SIZE(g_timer_cqc_reg_name);
	}

	printf("%-40s[addr_offset] : reg_data\n", "reg_name");
	while (index < timer_rsp->reg_num) {
		printf("%-40s[0x%08X] : 0x%08X\n",
		       index < arr_len ? reg_name[index] : "",
		       timer_rsp->timer_content[index][0],
		       timer_rsp->timer_content[index][1]);
		index++;
	}
	printf("*****************************************\n");
}

static int hikp_roce_timer_show_qpc(struct major_cmd_ctrl *self)
{
	struct roce_timer_req_para req_data = { 0 };
	struct roce_timer_rsp_data *timer_rsp = NULL;
	struct hikp_cmd_header req_header = { 0 };
	struct hikp_cmd_ret *cmd_ret = NULL;
	int ret;

	HIKP_SET_USED(self);

	req_data.bdf = g_roce_timer_param.target.bdf;
	if (g_roce_timer_param.flag)
		hikp_cmd_init(&req_header, ROCE_MOD, GET_ROCEE_TIMER_CMD, TIMER_QPC_CLEAR);
	else
		hikp_cmd_init(&req_header, ROCE_MOD, GET_ROCEE_TIMER_CMD, TIMER_SHOW_QPC);

	cmd_ret = hikp_cmd_alloc(&req_header, &req_data, sizeof(req_data));
	ret = hikp_rsp_normal_check(cmd_ret);
	if (ret != 0)
		goto out;

	timer_rsp = (struct roce_timer_rsp_data *)(cmd_ret->rsp_data);
	if (timer_rsp->reg_num > ROCE_HIKP_TIMER_REG_NUM) {
		printf("version might not match, adjust the reg num to %d.\n",
		       ROCE_HIKP_TIMER_REG_NUM);
		timer_rsp->reg_num = ROCE_HIKP_TIMER_REG_NUM;
	}

	printf("**************QPC TIMER INFO*************\n");
	hikp_roce_timer_print(timer_rsp, TIMER_SHOW_QPC);
out:
	hikp_cmd_free(&cmd_ret);
	return ret;
}

static int hikp_roce_timer_show_cqc(struct major_cmd_ctrl *self)
{
	struct roce_timer_req_para req_data = { 0 };
	struct roce_timer_rsp_data *timer_rsp = NULL;
	struct hikp_cmd_header req_header = { 0 };
	struct hikp_cmd_ret *cmd_ret = NULL;
	int ret;

	HIKP_SET_USED(self);

	req_data.bdf = g_roce_timer_param.target.bdf;
	if (g_roce_timer_param.flag)
		hikp_cmd_init(&req_header, ROCE_MOD, GET_ROCEE_TIMER_CMD, TIMER_CQC_CLEAR);
	else
		hikp_cmd_init(&req_header, ROCE_MOD, GET_ROCEE_TIMER_CMD, TIMER_SHOW_CQC);

	cmd_ret = hikp_cmd_alloc(&req_header, &req_data, sizeof(req_data));
	ret = hikp_rsp_normal_check(cmd_ret);
	if (ret != 0)
		goto out;

	timer_rsp = (struct roce_timer_rsp_data *)(cmd_ret->rsp_data);
	if (timer_rsp->reg_num > ROCE_HIKP_TIMER_REG_NUM) {
		printf("version might not match, adjust the reg num to %d.\n",
		       ROCE_HIKP_TIMER_REG_NUM);
		timer_rsp->reg_num = ROCE_HIKP_TIMER_REG_NUM;
	}

	printf("**************CQC TIMER INFO*************\n");
	hikp_roce_timer_print(timer_rsp, TIMER_SHOW_CQC);
out:
	hikp_cmd_free(&cmd_ret);
	return ret;
}

static int hikp_roce_timer_clear(struct major_cmd_ctrl *self)
{
	int ret_clear_qpc;
	int ret_clear_cqc;

	ret_clear_cqc = hikp_roce_timer_show_cqc(self);
	if (ret_clear_cqc != 0) {
		printf("clear cqc failed\n");
		return ret_clear_cqc;
	}

	ret_clear_qpc = hikp_roce_timer_show_qpc(self);
	if (ret_clear_qpc != 0) {
		printf("clear qpc failed\n");
		return ret_clear_qpc;
	}

	return 0;
}

void hikp_roce_timer_execute(struct major_cmd_ctrl *self)
{
	int (*func[])(struct major_cmd_ctrl *self) = {
		hikp_roce_timer_show_cqc, hikp_roce_timer_show_qpc
	};
	const char *function[] = {"show cqc", "show qpc"};
	size_t i = 0;
	int ret;

	if (g_roce_timer_param.flag) {
		ret = hikp_roce_timer_clear(self);
	} else {
		for (i = 0; i < HIKP_ARRAY_SIZE(func); i++) {
			ret = func[i](self);
			if (ret != 0)
				break;
		}
	}

	if (ret != 0) {
		self->err_no = -EINVAL;
		snprintf(self->err_str, sizeof(self->err_str),
			 "roce_timer %s function failed\n", function[i]);
	}
}

static void cmd_roce_timer_init(void)
{
	struct major_cmd_ctrl *major_cmd = get_major_cmd();

	major_cmd->option_count = 0;
	major_cmd->execute = hikp_roce_timer_execute;

	cmd_option_register("-h", "--help", false, hikp_roce_timer_help);
	cmd_option_register("-i", "--interface", true, hikp_roce_timer_target);
	cmd_option_register("-c", "--clear", false, hikp_roce_timer_clear_set);
}

HIKP_CMD_DECLARE("roce_timer", "get or clear roce_timer registers information",
		 cmd_roce_timer_init);
