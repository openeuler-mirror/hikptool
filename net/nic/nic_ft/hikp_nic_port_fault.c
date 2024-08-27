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

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include "hikp_nic_port_fault.h"

static struct tool_target g_port_fault_target;

static int hikp_nic_port_fault_query(const struct bdf_t *bdf,
				     struct nic_port_fault_status *info)
{
	struct nic_port_fault_req_para req = { 0 };
	struct hikp_cmd_header header = { 0 };
	struct nic_port_fault_rsp *rsp;
	struct hikp_cmd_ret *cmd_ret;
	int ret = 0;

	req.bdf = *bdf;
	hikp_cmd_init(&header, NIC_MOD, GET_PORT_FAULT_STATUS,
		      NIC_PORT_FAULT_INFO_DUMP);
	cmd_ret = hikp_cmd_alloc(&header, &req, sizeof(req));
	if (cmd_ret == NULL || cmd_ret->status != 0) {
		ret = cmd_ret ? (int)(-cmd_ret->status) : -EIO;
		HIKP_ERROR_PRINT("fail to get port fault, retcode: %d\n", ret);
		hikp_cmd_free(&cmd_ret);

		return ret;
	}

	rsp = (struct nic_port_fault_rsp *)cmd_ret->rsp_data;
	memcpy(info, rsp->data, sizeof(struct nic_port_fault_status));
	hikp_cmd_free(&cmd_ret);

	return 0;
}

static void hikp_nic_format_port_fault_info(struct nic_port_fault_status *info)
{
	if (info->cdr_core_status > NIC_PORT_FAULT_INVALID)
		info->cdr_core_status = NIC_PORT_FAULT_INVALID;

	if (info->cdr_flash_status > NIC_PORT_FAULT_INVALID)
		info->cdr_flash_status = NIC_PORT_FAULT_INVALID;

	if (info->fault_9545_status > NIC_PORT_FAULT_INVALID)
		info->fault_9545_status = NIC_PORT_FAULT_INVALID;

	if (info->hilink_ref_status > NIC_PORT_FAULT_INVALID)
		info->hilink_ref_status = NIC_PORT_FAULT_INVALID;
}

static void hikp_nic_port_fault_show(struct nic_port_fault_status *info)
{
	const char *port_fault_info[] = {
		"OK",
		"Device error",
		"Device not support",
		"Invalid"
	};

	hikp_nic_format_port_fault_info(info);
	printf("############ NIC port fault status ###############\n");
	printf("cdr flash : %s.\n", port_fault_info[info->cdr_flash_status]);
	printf("cdr core  : %s.\n", port_fault_info[info->cdr_core_status]);
	printf("9545 fault: %s.\n", port_fault_info[info->fault_9545_status]);
	printf("hilink ref: %s.\n", port_fault_info[info->hilink_ref_status]);
	printf("#################### END #######################\n");
}

void hikp_nic_port_fault_cmd_execute(struct major_cmd_ctrl *self)
{
	struct bdf_t *bdf = &g_port_fault_target.bdf;
	struct nic_port_fault_status info = { 0 };
	int ret;

	ret = hikp_nic_port_fault_query(bdf, &info);
	if (ret != 0) {
		snprintf(self->err_str, sizeof(self->err_str), "fail to get fault info.");
		self->err_no = ret;
		return;
	}

	hikp_nic_port_fault_show(&info);
}

static int hikp_nic_port_fault_cmd_help(struct major_cmd_ctrl *self, const char *argv)
{
	printf("\n  Usage: %s %s\n", self->cmd_ptr->name, "-i <device>");
	printf("\n         %s\n", self->cmd_ptr->help_info);
	printf("\n  Options:\n\n");
	printf("    %s, %-25s %s\n", "-h", "--help", "display this help and exit");
	printf("    %s, %-25s %s\n", "-i", "--interface=<interface>",
	       "device target or bdf id, e.g. eth0~7 or 0000:35:00.0");

	return 0;
}

int hikp_nic_port_fault_get_target(struct major_cmd_ctrl *self, const char *argv)
{
	self->err_no = tool_check_and_get_valid_bdf_id(argv, &g_port_fault_target);
	if (self->err_no != 0) {
		snprintf(self->err_str, sizeof(self->err_str), "unknown device!");
		return self->err_no;
	}

	if (g_port_fault_target.bdf.dev_id != 0) {
		snprintf(self->err_str, sizeof(self->err_str), "VF is not supported!");
		self->err_no = -EOPNOTSUPP;
		return self->err_no;
	}

	return 0;
}

static void cmd_nic_port_fault_init(void)
{
	struct major_cmd_ctrl *major_cmd = get_major_cmd();

	major_cmd->option_count = 0;
	major_cmd->execute = hikp_nic_port_fault_cmd_execute;

	cmd_option_register("-h", "--help", false, hikp_nic_port_fault_cmd_help);
	cmd_option_register("-i", "--interface", true, hikp_nic_port_fault_get_target);
}

HIKP_CMD_DECLARE("nic_port_fault", "dump port fault of nic!", cmd_nic_port_fault_init);
