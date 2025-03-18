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
#include "hikp_roce_gmv.h"

static struct cmd_roce_gmv_param g_roce_gmv_param = { 0 };

int hikp_roce_set_gmv_bdf(char *nic_name)
{
	return tool_check_and_get_valid_bdf_id(nic_name,
					       &g_roce_gmv_param.target);
}

void hikp_roce_set_gmv_index(uint32_t gmv_index)
{
	g_roce_gmv_param.gmv_index = gmv_index;
}

static int hikp_roce_gmv_help(struct major_cmd_ctrl *self, const char *argv)
{
	HIKP_SET_USED(argv);

	printf("\n  Usage: %s %s\n", self->cmd_ptr->name, "-i <interface>\n");
	printf("\n         %s\n", self->cmd_ptr->help_info);
	printf("  Options:\n\n");
	printf("    %s, %-25s %s\n", "-h", "--help", "display this help and exit");
	printf("    %s, %-25s %s\n", "-i", "--interface=<interface>", "device target, e.g. eth0");
	printf("    %s, %-25s %s\n", "-x", "--gmv_index",
	       "[option]set which gid to read. (default 0)");
	printf("\n");

	return 0;
}

static int hikp_roce_gmv_target(struct major_cmd_ctrl *self, const char *argv)
{
	self->err_no = tool_check_and_get_valid_bdf_id(argv, &(g_roce_gmv_param.target));
	if (self->err_no) {
		snprintf(self->err_str, sizeof(self->err_str), "Unknown device %s.", argv);
		return self->err_no;
	}

	return 0;
}

static int hikp_roce_gmv_idxget(struct major_cmd_ctrl *self, const char *argv)
{
	char *endptr = NULL;
	uint64_t index;

	index = strtoul(argv, &endptr, 0);
	if ((endptr <= argv) || (*endptr != '\0') ||
	    (index >= ROCE_MAX_HIKPTOOL_GMV)) {
		snprintf(self->err_str, sizeof(self->err_str), "Invalid gmv_index.");
		self->err_no = -EINVAL;
		return -EINVAL;
	}
	g_roce_gmv_param.gmv_index = (uint32_t)index;

	return 0;
}

/* DON'T change the order of this array or add entries between! */
static const char *g_gmv_reg_name[] = {
	"ROCEE_VF_GMV_RO0",
	"ROCEE_VF_GMV_RO1",
	"ROCEE_VF_GMV_RO2",
	"ROCEE_VF_GMV_RO3",
	"ROCEE_VF_GMV_RO4",
	"ROCEE_VF_GMV_RO5",
	"ROCEE_VF_GMV_RO6",
};

static void hikp_roce_gmv_print(uint32_t reg_num, struct roce_gmv_rsp_data *gmv_rsp)
{
	uint8_t arr_len = HIKP_ARRAY_SIZE(g_gmv_reg_name);
	uint32_t i;

	printf("*******************GMV INFO****************\n");
	printf("%-40s[addr_offset] : reg_data\n", "reg_name");
	for (i = 0; i < reg_num; i++)
		printf("%-40s[0x%08X] : 0x%08X\n",
		       i < arr_len ? g_gmv_reg_name[i] : "",
		       gmv_rsp->reg_offset[i], gmv_rsp->reg_data[i]);
	printf("*******************************************\n");
}

void hikp_roce_gmv_execute(struct major_cmd_ctrl *self)
{
	struct roce_gmv_req_para req_data = { 0 };
	struct roce_gmv_rsp_data *gmv_rsp = NULL;
	struct hikp_cmd_header req_header = { 0 };
	struct hikp_cmd_ret *cmd_ret = NULL;
	uint32_t reg_num;
	int ret;

	HIKP_SET_USED(self);

	req_data.bdf = g_roce_gmv_param.target.bdf;
	req_data.gmv_index = g_roce_gmv_param.gmv_index;
	hikp_cmd_init(&req_header, ROCE_MOD, GET_ROCEE_GMV_CMD, GMV_SHOW);
	cmd_ret = hikp_cmd_alloc(&req_header, &req_data, sizeof(req_data));
	ret = hikp_rsp_normal_check(cmd_ret);
	if (ret != 0)
		goto exec_error;

	if (cmd_ret->rsp_data_num == 0) {
		printf("roce show gmv single-port info cmd param mem read failed\n");
		goto exec_error;
	}
	reg_num = cmd_ret->rsp_data_num / ROCE_HIKP_GMV_REG_SWICTH;
	if (reg_num != ROCE_HIKP_GMV_REG_NUM) {
		printf("version might not match\n");
		goto exec_error;
	}

	gmv_rsp = (struct roce_gmv_rsp_data *)(cmd_ret->rsp_data);
	hikp_roce_gmv_print(reg_num, gmv_rsp);
exec_error:
	hikp_cmd_free(&cmd_ret);
}

static void cmd_roce_gmv_init(void)
{
	struct major_cmd_ctrl *major_cmd = get_major_cmd();

	major_cmd->option_count = 0;
	major_cmd->execute = hikp_roce_gmv_execute;

	cmd_option_register("-h", "--help", false, hikp_roce_gmv_help);
	cmd_option_register("-i", "--interface", true, hikp_roce_gmv_target);
	cmd_option_register("-x", "--gmv_index", true, hikp_roce_gmv_idxget);
}

HIKP_CMD_DECLARE("roce_gmv", "get roce_gmv registers information", cmd_roce_gmv_init);
