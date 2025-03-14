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
#include "tool_cmd.h"
#include "hikp_net_lib.h"
#include "hikptdev_plug.h"
#include "hikp_nic_ncsi.h"

static struct nic_ncsi_cmd_info g_ncsi_cmd_info = {0};

static void nic_ncsi_cmd_print_dfx_info(struct nic_ncsi_cmd_resp *ncsi_info)
{
	printf("port ncsi: %s\n", ncsi_info->ncsi_en ? "enable" : "disable");
	if (!ncsi_info->ncsi_en)
		return;    /* ncsi not enable do not print dfx info */

	printf("processing packet statistics\n");
	printf("\tncsi_control_total: %u\n", ncsi_info->ncsi_dfx.ncsi_control_total);
	printf("\tncsi_eth_to_ub_total: %u\n", ncsi_info->ncsi_dfx.ncsi_eth_to_ub_total);
	printf("\tncsi_ub_to_eth_total: %u\n", ncsi_info->ncsi_dfx.ncsi_ub_to_eth_total);
	printf("\tncsi_control_good: %u\n", ncsi_info->ncsi_dfx.ncsi_control_good);
	printf("\tncsi_eth_to_ub_good: %u\n", ncsi_info->ncsi_dfx.ncsi_eth_to_ub_good);
	printf("\tncsi_ub_to_eth_good: %u\n", ncsi_info->ncsi_dfx.ncsi_ub_to_eth_good);

	printf("\tncsi_eth_to_ub_arp: %u\n", ncsi_info->ncsi_dfx.ncsi_eth_to_ub_arp);
	printf("\tncsi_eth_to_ub_free_arp: %u\n", ncsi_info->ncsi_dfx.ncsi_eth_to_ub_free_arp);
	printf("\tncsi_eth_to_ub_ipv6_ra: %u\n", ncsi_info->ncsi_dfx.ncsi_eth_to_ub_ipv6_ra);
	printf("\tncsi_eth_to_ub_dhcpv4: %u\n", ncsi_info->ncsi_dfx.ncsi_eth_to_ub_dhcpv4);
	printf("\tncsi_eth_to_ub_dhcpv6: %u\n", ncsi_info->ncsi_dfx.ncsi_eth_to_ub_dhcpv6);
	printf("\tncsi_eth_to_ub_lldp: %u\n", ncsi_info->ncsi_dfx.ncsi_eth_to_ub_lldp);

	printf("\tncsi_ub_to_eth_ipv4: %u\n", ncsi_info->ncsi_dfx.ncsi_ub_to_eth_ipv4);
	printf("\tncsi_ub_to_eth_ipv6: %u\n", ncsi_info->ncsi_dfx.ncsi_ub_to_eth_ipv6);
	printf("\tncsi_ub_to_eth_ipnotify: %u\n", ncsi_info->ncsi_dfx.ncsi_ub_to_eth_ipnotify);
	printf("\tncsi_ub_to_eth_dhcpv4: %u\n", ncsi_info->ncsi_dfx.ncsi_ub_to_eth_dhcpv4);
	printf("\tncsi_ub_to_eth_dhcpv6: %u\n", ncsi_info->ncsi_dfx.ncsi_ub_to_eth_dhcpv6);
	printf("\tncsi_ub_to_eth_lldp: %u\n", ncsi_info->ncsi_dfx.ncsi_ub_to_eth_lldp);
}

static void nic_ncsi_cmd_execute(struct major_cmd_ctrl *self)
{
	struct hikp_cmd_ret *cmd_resp = NULL;
	struct hikp_cmd_header req_header = {0};
	struct nic_ncsi_cmd_req ncsi_req = {0};

	if (!g_ncsi_cmd_info.port_flag) {
		self->err_no = -EINVAL;
		snprintf(self->err_str, sizeof(self->err_str), "Need port id.");
		return;
	}

	memcpy(&ncsi_req.bdf, &g_ncsi_cmd_info.target.bdf, sizeof(ncsi_req.bdf));
	hikp_cmd_init(&req_header, NIC_MOD, GET_NCSI_INFO_CMD, NIC_NCSI_GET_DFX_INFO);
	cmd_resp = hikp_cmd_alloc(&req_header, &ncsi_req, sizeof(ncsi_req));
	self->err_no = hikp_rsp_normal_check(cmd_resp);
	if (self->err_no) {
		snprintf(self->err_str, sizeof(self->err_str), "Get ncsi dfx info failed.");
		goto ERR_OUT;
	}

	nic_ncsi_cmd_print_dfx_info((struct nic_ncsi_cmd_resp *)cmd_resp->rsp_data);

ERR_OUT:
	hikp_cmd_free(&cmd_resp);
}

static int nic_ncsi_cmd_get_port_info(struct major_cmd_ctrl *self, const char *argv)
{
	self->err_no = tool_check_and_get_valid_bdf_id(argv, &g_ncsi_cmd_info.target);
	if (self->err_no) {
		snprintf(self->err_str, sizeof(self->err_str), "Unknown device %s.", argv);
		return self->err_no;
	}
	g_ncsi_cmd_info.port_flag = true;

	return 0;
}

static int nic_ncsi_cmd_show_help(struct major_cmd_ctrl *self, const char *argv)
{
	HIKP_SET_USED(argv);

	printf("\n  Usage: %s %s\n", self->cmd_ptr->name, "-i <interface>");
	printf("\n         %s\n", self->cmd_ptr->help_info);
	printf("\n  Options:\n\n");
	printf("    %s, %-25s %s\n", "-h", "--help", "display this help and exit");
	printf("    %s, %-25s %s\n", "-i", "--interface=<interface>",
	       "device target or bdf id, e.g. eth0~3 or 0000:35:00.0");
	printf("\n");

	return 0;
}

int hikp_info_collect_nic_ncsi(void *data)
{
	struct nic_ncsi_collect_param *param = (struct nic_ncsi_collect_param *)data;
	struct major_cmd_ctrl *major_cmd = get_major_cmd();
	int ret;

	memset(&g_ncsi_cmd_info, 0, sizeof(g_ncsi_cmd_info));

	ret = nic_ncsi_cmd_get_port_info(major_cmd, param->net_dev_name);
	if (ret)
		return ret;

	printf("hikptool nic_ncsi -i %s\n", param->net_dev_name);
	nic_ncsi_cmd_execute(major_cmd);

	return ret;
}

static void cmd_nic_get_ncsi_init(void)
{
	struct major_cmd_ctrl *major_cmd = get_major_cmd();

	g_ncsi_cmd_info.port_flag = false;
	major_cmd->option_count = 0;
	major_cmd->execute = nic_ncsi_cmd_execute;

	cmd_option_register("-h", "--help",       false, nic_ncsi_cmd_show_help);
	cmd_option_register("-i", "--interface",  true,  nic_ncsi_cmd_get_port_info);
}

HIKP_CMD_DECLARE("nic_ncsi", "query nic port ncsi information", cmd_nic_get_ncsi_init);
