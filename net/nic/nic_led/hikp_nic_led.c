/*
 * Copyright (c) 2025 Hisilicon Technologies Co., Ltd.
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
#include "hikp_nic_led.h"

static struct nic_led_cmd_info g_led_cmd_info = {0};

static const char *nic_led_cmd_get_speed_status(uint8_t speed_led)
{
	struct speed_status led_speed[] = {
		{SPEED_1G_BLINK,	"1G"},
		{SPEED_10G_BLINK,	"10G"},
		{SPEED_25G_BLINK,	"25G"},
		{SPEED_40G_BLINK,	"40G"},
		{SPEED_50G_BLINK,	"50G"},
		{SPEED_100G_BLINK,	"100G"},
		{SPEED_200G_BLINK,	"200G"},
		{SPEED_NO_LINK_BLINK,	"NO_LINK"},
		{SPEED_TEST_BLINK,	"TEST_MODE"},
		{SPEED_ERROR_BLINK,	"ERROR_MODE"},
	};
	size_t size = HIKP_ARRAY_SIZE(led_speed);

	for (size_t i = 0; i < size; i++) {
		if (speed_led == led_speed[i].status)
			return led_speed[i].speed;
	}

	return "unknown";
}

static bool nic_led_cmd_param_check(struct major_cmd_ctrl *self)
{
	if ((g_led_cmd_info.cmd_flag & LED_PORT_TARGET_BIT) == 0) {
		self->err_no = -EINVAL;
		snprintf(self->err_str, sizeof(self->err_str), "Need port id.");
		return false;
	}

	return true;
}

static void nic_led_cmd_execute(struct major_cmd_ctrl *self)
{
	struct hikp_cmd_header header = { 0 };
	struct nic_led_resp *led_rsp = NULL;
	struct nic_led_req_para req = { 0 };
	struct hikp_cmd_ret *cmd_ret;

	if (!nic_led_cmd_param_check(self))
		return;

	req.bdf = g_led_cmd_info.port_dev.bdf;
	hikp_cmd_init(&header, NIC_MOD, GET_PORT_LED_CFG, NIC_LED_CFG_DUMP);
	cmd_ret = hikp_cmd_alloc(&header, &req, sizeof(req));
	self->err_no = hikp_rsp_normal_check(cmd_ret);
	if (self->err_no) {
		snprintf(self->err_str, sizeof(self->err_str), "Get led dfx info failed.");
		goto ERR_OUT;
	}

	led_rsp = (struct nic_led_resp *)cmd_ret->rsp_data;
	printf("%-40s: %u\n", "led_en", led_rsp->led_en);
	printf("%-40s: %s\n", "speed_led", nic_led_cmd_get_speed_status(led_rsp->speed_led_status));
	printf("%-40s: 0x%x(0x%x)\n", "led_err_status(sw_status)",
	       led_rsp->hw_err_mode, led_rsp->sw_err_mode);
	printf("%-40s: 0x%x(0x%x)\n", "led_locate_status(sw_status)",
	       led_rsp->hw_locate_mode, led_rsp->sw_locate_mode);
	printf("%-40s: 0x%x(0x%x)\n", "led_active_status(sw_status)",
	       led_rsp->hw_active_mode, led_rsp->sw_active_mode);

ERR_OUT:
	hikp_cmd_free(&cmd_ret);
}

static int nic_led_cmd_help(struct major_cmd_ctrl *self, const char *argv)
{
	HIKP_SET_USED(argv);

	printf("\n  Usage: %s %s\n", self->cmd_ptr->name, "-i <device>");
	printf("\n         %s\n", self->cmd_ptr->help_info);
	printf("\n  Options:\n\n");
	printf("    %s, %-25s %s\n", "-h", "--help", "display this help and exit");
	printf("    %s, %-25s %s\n", "-i", "--interface=<interface>",
	       "device target or bdf id, e.g. eth0~3 or 0000:35:00.0");

	return 0;
}

static int nic_led_get_target(struct major_cmd_ctrl *self, const char *argv)
{
	self->err_no = tool_check_and_get_valid_bdf_id(argv, &g_led_cmd_info.port_dev);
	if (self->err_no != 0) {
		snprintf(self->err_str, sizeof(self->err_str), "unknown device!");
		return self->err_no;
	}

	g_led_cmd_info.cmd_flag |= LED_PORT_TARGET_BIT;

	return 0;
}

static void cmd_nic_led_init(void)
{
	struct major_cmd_ctrl *major_cmd = get_major_cmd();

	major_cmd->option_count = 0;
	major_cmd->execute = nic_led_cmd_execute;

	cmd_option_register("-h", "--help", false, nic_led_cmd_help);
	cmd_option_register("-i", "--interface", true, nic_led_get_target);
}

HIKP_CMD_DECLARE("nic_led", "dump led configuration of port!", cmd_nic_led_init);
