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
#include <sys/types.h>
#include <unistd.h>

#include "hikp_nic_gro.h"

static struct tool_target g_gro_target;

static int hikp_nic_gro_query(const struct bdf_t *bdf, struct nic_gro_info *info)
{
	struct nic_gro_req_para req = { 0 };
	struct hikp_cmd_header header = { 0 };
	struct hikp_cmd_ret *cmd_ret;
	struct nic_gro_rsp *rsp;

	req.bdf = *bdf;
	hikp_cmd_init(&header, NIC_MOD, GET_GRO_INFO_CMD, NIC_GRO_INFO_DUMP);
	cmd_ret = hikp_cmd_alloc(&header, &req, sizeof(req));
	if (cmd_ret == NULL || cmd_ret->status != 0) {
		HIKP_ERROR_PRINT("fail to get gro info, retcode: %u\n",
				 cmd_ret ? cmd_ret->status : EIO);
		free(cmd_ret);
		return -EIO;
	}

	rsp = (struct nic_gro_rsp *)cmd_ret->rsp_data;
	*info = *(struct nic_gro_info *)rsp->data;
	free(cmd_ret);

	return 0;
}

static void hikp_nic_gro_show(const struct nic_gro_info *info)
{
	printf("################ NIC GRO info ##################\n");
	printf("gro_en: %s\n", info->gro_en ? "true" : "false");
	printf("max_coal_bd_num: %u\n", info->max_coal_bd_num);
	printf("#################### END #######################\n");
}

static void hikp_nic_gro_cmd_execute(struct major_cmd_ctrl *self)
{
	struct bdf_t *bdf = &g_gro_target.bdf;
	struct nic_gro_info info;
	int ret;

	ret = hikp_nic_gro_query(bdf, &info);
	if (ret != 0) {
		snprintf(self->err_str, sizeof(self->err_str), "fail to obtain gro info.");
		self->err_no = ret;
		return;
	}

	hikp_nic_gro_show(&info);
}

static int hikp_nic_gro_cmd_help(struct major_cmd_ctrl *self, const char *argv)
{
	printf("\n  Usage: %s %s\n", self->cmd_ptr->name, "-i <device>");
	printf("\n         %s\n", self->cmd_ptr->help_info);
	printf("\n  Options:\n\n");
	printf("    %s, %-25s %s\n", "-h", "--help", "display this help and exit");
	printf("    %s, %-25s %s\n", "-i", "--interface=<interface>",
	       "device target or bdf id, e.g. eth0~7 or 0000:35:00.0");

	return 0;
}

static int hikp_nic_gro_get_target(struct major_cmd_ctrl *self, const char *argv)
{
	self->err_no = tool_check_and_get_valid_bdf_id(argv, &g_gro_target);
	if (self->err_no != 0) {
		snprintf(self->err_str, sizeof(self->err_str), "unknown device!");
		return self->err_no;
	}

	return 0;
}

static void cmd_nic_gro_init(void)
{
	struct major_cmd_ctrl *major_cmd = get_major_cmd();

	major_cmd->option_count = 0;
	major_cmd->execute = hikp_nic_gro_cmd_execute;

	cmd_option_register("-h", "--help", false, hikp_nic_gro_cmd_help);
	cmd_option_register("-i", "--interface", true, hikp_nic_gro_get_target);
}

HIKP_CMD_DECLARE("nic_gro", "dump gro info of nic!", cmd_nic_gro_init);
