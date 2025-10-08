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

static void nic_ncsi_cmd_get_cnt_dfx(struct major_cmd_ctrl *self)
{
	struct hikp_cmd_ret *cmd_resp = NULL;
	struct hikp_cmd_header req_header = {0};
	struct nic_ncsi_cmd_req ncsi_req = {0};

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

static int nic_ncsi_cmd_get_entry_response_data(struct major_cmd_ctrl *self, uint32_t sub_cmd,
						struct hikp_cmd_ret **cmd_resp)
{
	struct hikp_cmd_header req_header = {0};
	struct nic_ncsi_cmd_req ncsi_req = {0};

	memcpy(&ncsi_req.bdf, &g_ncsi_cmd_info.target.bdf, sizeof(ncsi_req.bdf));
	hikp_cmd_init(&req_header, NIC_MOD, GET_NCSI_TABLE_ENTRY_INFO_CMD, sub_cmd);
	*cmd_resp = hikp_cmd_alloc(&req_header, &ncsi_req, sizeof(ncsi_req));
	self->err_no = hikp_rsp_normal_check(*cmd_resp);
	if (self->err_no)
		snprintf(self->err_str, sizeof(self->err_str),
			 "Get ncsi filter(0x%x) info failed.", sub_cmd);

	return self->err_no;
}

static void nic_ncsi_cmd_show_tx_buf_status(struct major_cmd_ctrl *self)
{
	struct nic_ncsi_tx_buf_resp *tx_buf_rsp;
	struct hikp_cmd_ret *cmd_resp = NULL;
	int ret;

	ret = nic_ncsi_cmd_get_entry_response_data(self, NIC_NCSI_GET_BUF_STATUS, &cmd_resp);
	if (ret)
		goto ERR_OUT;

	tx_buf_rsp = (struct nic_ncsi_tx_buf_resp *)cmd_resp->rsp_data;
	printf("ncsi tx buffer status:\n");
	printf("%-30s: %u\n", "tx_buf_empty", tx_buf_rsp->tx_buf_empty);
	printf("%-30s: %u\n", "ctrl_sof", tx_buf_rsp->ctrl_sof);
	printf("%-30s: %u\n", "ctrl_eof", tx_buf_rsp->ctrl_eof);
	printf("%-30s: %u\n", "ctrl_err", tx_buf_rsp->ctrl_err);
	printf("%-30s: %u\n", "ctrl_byte_sel", tx_buf_rsp->ctrl_byte_sel);

ERR_OUT:
	hikp_cmd_free(&cmd_resp);
}

static void nic_ncsi_cmd_show_vlan_filter_cfg(struct major_cmd_ctrl *self)
{
	struct nic_ncsi_vlan_filter_resp *vlan_rsp = NULL;
	struct hikp_cmd_ret *cmd_resp = NULL;
	int ret;

	ret = nic_ncsi_cmd_get_entry_response_data(self, NIC_NCSI_GET_VLAN_FILTER_TBL, &cmd_resp);
	if (ret)
		goto ERR_OUT;

	vlan_rsp = (struct nic_ncsi_vlan_filter_resp *)cmd_resp->rsp_data;
	printf("vlan filter config:\n");
	printf("%-30s: 0x%x\n", "filter_enable",vlan_rsp->filter_en_map);
	printf("%-30s: 0x%x\n", "filter_type", vlan_rsp->filer_type);
	printf("%-30s: 0x%x\n", "entry_to_bmc", vlan_rsp->entry_to_bmc_map);
	printf("%-30s: 0x%x\n", "entry_to_bmc_only", vlan_rsp->entry_to_bmc_only_map);

	printf("enabled entries:\n");
	printf("%5s   |%10s\n", "id", "vlan_id");
	for (uint32_t i = 0; i < NIC_NCSI_VLAN_ENTRY_NUM; i++) {
		if (!vlan_rsp->entry[i].entry_en)
			continue;
		printf("%5u   |%10u\n", i, vlan_rsp->entry[i].vlan_id);
	}

ERR_OUT:
	hikp_cmd_free(&cmd_resp);
}

static void nic_ncsi_cmd_show_ether_filter_cfg(struct major_cmd_ctrl *self)
{
	struct nic_ncsi_ether_filter_resp *ether_rsp = NULL;
	struct hikp_cmd_ret *cmd_resp = NULL;
	int ret;

	ret = nic_ncsi_cmd_get_entry_response_data(self, NIC_NCSI_GET_ETHER_FILTER_TBL, &cmd_resp);
	if (ret)
		goto ERR_OUT;

	ether_rsp = (struct nic_ncsi_ether_filter_resp *)cmd_resp->rsp_data;
	printf("ether filter config:\n");
	printf("%-30s: 0x%x\n", "filter_enable", ether_rsp->ether_en_map);
	printf("%-30s: 0x%x\n", "entry_to_bmc", ether_rsp->entry_to_bmc_map);
	printf("%-30s: 0x%x\n", "entry_to_bmc_only", ether_rsp->entry_to_bmc_only_map);

	printf("enabled entries:\n");
	printf("%5s   |%13s\n", "id", "ether_type");
	for (uint32_t i = 0; i < NIC_NCSI_ETHER_ENTRY_NUM; i++) {
		if (!ether_rsp->entry[i].entry_en)
			continue;

		printf("%5u   |%13u\n", i, ether_rsp->entry[i].entry_type);
	}

ERR_OUT:
	hikp_cmd_free(&cmd_resp);
}

static void nic_ncsi_cmd_show_dmac_filter_cfg(struct major_cmd_ctrl *self)
{
	struct nic_ncsi_dmac_filter_resp *dmac_rsp = NULL;
	struct hikp_cmd_ret *cmd_resp = NULL;
	int ret;

	ret = nic_ncsi_cmd_get_entry_response_data(self, NIC_NCSI_GET_DMAC_FILTER_TBL, &cmd_resp);
	if (ret)
		goto ERR_OUT;

	dmac_rsp = (struct nic_ncsi_dmac_filter_resp *)cmd_resp->rsp_data;
	printf("dmac filter config:\n");
	printf("%-30s: 0x%x\n", "filter_enable", dmac_rsp->dmac_en_map);
	printf("%-30s: 0x%x\n", "entry_to_bmc", dmac_rsp->dmac_to_bmc);
	printf("%-30s: 0x%x\n", "entry_to_bmc_only", dmac_rsp->dmac_to_bmc_only);

	printf("enabled entries:\n");
	printf("%5s   |%10s   |%10s   |%13s\n", "id", "type", "entry_h", "entry_l");
	for (uint32_t i = 0; i < NIC_NCSI_DMAC_ENTRY_NUM; i++) {
		if (!dmac_rsp->entry[i].entry_en)
			continue;
		printf("%5u   |%10u   |%#10x   |%#13x\n",
		       i, dmac_rsp->entry[i].entry_type,
		       dmac_rsp->entry[i].entry_cfg_h, dmac_rsp->entry[i].entry_cfg_l);
	}

	printf("multicast_filter: %16s(%u)%15s(%u)%16s(%u)%11s(%u)%15s(%u)\n",
	       "ipv6_neighbor", dmac_rsp->mc_ipv6_neighbor_en,
	       "ipv6_route", dmac_rsp->mc_ipv6_route_en,
	       "dhcpv6_relay", dmac_rsp->mc_dhcpv6_relay_en,
	       "to_bmc", dmac_rsp->mc_to_bmc, "to_bmc_only", dmac_rsp->mc_to_bmc_only);

	printf("broadcast_filter: %16s(%u)%15s(%u)%16s(%u)%11s(%u)%15s(%u)%15s(%u)\n",
	       "arp", dmac_rsp->bc_arp_en, "dhcp_client", dmac_rsp->bc_dhcp_client,
	       "dhcp_server", dmac_rsp->bc_dhcp_server, "netbios", dmac_rsp->bc_netbios_en,
	       "to_bmc", dmac_rsp->bc_to_bmc, "to_bmc_only", dmac_rsp->bc_to_bmc_only);

ERR_OUT:
	hikp_cmd_free(&cmd_resp);
}

static void nic_ncsi_cmd_show_smac_filter_cfg(struct major_cmd_ctrl *self)
{
	struct nic_ncsi_smac_filter_resp *smac_rsp = NULL;
	struct hikp_cmd_ret *cmd_resp = NULL;
	int ret;

	ret = nic_ncsi_cmd_get_entry_response_data(self, NIC_NCSI_GET_SMAC_FILTER_TBL, &cmd_resp);
	if (ret)
		goto ERR_OUT;

	smac_rsp = (struct nic_ncsi_smac_filter_resp *)cmd_resp->rsp_data;
	printf("smac filter config:\n");
	printf("%-30s: 0x%x\n", "filter_enable", smac_rsp->smac_en_map);
	printf("%-30s: 0x%x\n", "pt_pkt_enable", smac_rsp->pt_pkt_en);

	printf("enabled entries:\n");
	printf("%5s   |%10s   |%15s   |%10s   |%13s\n",
	       "id", "dport", "entry_to_mac", "entry_h", "entry_l");
	for (uint32_t i = 0; i < NIC_NCSI_SMAC_ENTRY_NUM; i++) {
		if (!smac_rsp->entry[i].entry_en)
			continue;
		printf("%5u   |%10u   |%15u   |%#10x   |%#13x\n",
		       i, smac_rsp->entry[i].entry_dport,
		       smac_rsp->entry[i].entry_to_mac, smac_rsp->entry[i].entry_cfg_h,
		       smac_rsp->entry[i].entry_cfg_l);
	}

ERR_OUT:
	hikp_cmd_free(&cmd_resp);
}

static void nic_ncsi_cmd_get_filter_cfg(struct major_cmd_ctrl *self)
{
	struct ncsi_dump_mod_proc mod_info[] = {
		{"tx_buf",	nic_ncsi_cmd_show_tx_buf_status},
		{"vlan",	nic_ncsi_cmd_show_vlan_filter_cfg},
		{"ether",	nic_ncsi_cmd_show_ether_filter_cfg},
		{"dmac",	nic_ncsi_cmd_show_dmac_filter_cfg},
		{"smac",	nic_ncsi_cmd_show_smac_filter_cfg},
	};
	uint32_t size = HIKP_ARRAY_SIZE(mod_info);

	if (!g_ncsi_cmd_info.module_name) {
		self->err_no = -EPERM;
		snprintf(self->err_str, sizeof(self->err_str), "dump module is null.");
		return;
	}

	if (strcmp(g_ncsi_cmd_info.module_name, "all") == 0) {
		for (uint32_t i = 0; i < size; i++) {
			mod_info[i].show(self);
			printf("----------------------------------"
			       "------------------------------------------\n");
		}
	} else {
		for (uint32_t i = 0; i < size; i++) {
			if (strcmp(g_ncsi_cmd_info.module_name, mod_info[i].name) != 0)
				continue;

			return mod_info[i].show(self);
		}

		self->err_no = -EINVAL;
		snprintf(self->err_str, sizeof(self->err_str),
			 "Invalid filter conifg module: %s.", g_ncsi_cmd_info.module_name);
	}
}

static void nic_ncsi_cmd_execute(struct major_cmd_ctrl *self)
{
	if ((g_ncsi_cmd_info.cmd_flag & NCSI_PORT_TARGET_BIT) == 0) {
		self->err_no = -EINVAL;
		snprintf(self->err_str, sizeof(self->err_str), "Need port id.");
		return;
	}

	if (g_ncsi_cmd_info.cmd_flag & NCSI_DUMP_MODULE_BIT)
		nic_ncsi_cmd_get_filter_cfg(self);
	else
		nic_ncsi_cmd_get_cnt_dfx(self);
}

static int nic_ncsi_cmd_get_port_info(struct major_cmd_ctrl *self, const char *argv)
{
	self->err_no = tool_check_and_get_valid_bdf_id(argv, &g_ncsi_cmd_info.target);
	if (self->err_no) {
		snprintf(self->err_str, sizeof(self->err_str), "Unknown device %s.", argv);
		return self->err_no;
	}
	g_ncsi_cmd_info.cmd_flag |= NCSI_PORT_TARGET_BIT;

	return 0;
}

static int nic_ncsi_cmd_get_dump_mode(struct major_cmd_ctrl *self, const char *argv)
{
	HIKP_SET_USED(self);

	g_ncsi_cmd_info.cmd_flag |= NCSI_DUMP_MODULE_BIT;
	g_ncsi_cmd_info.module_name = argv;

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
	printf("    %s, %-25s %s\n", "-d", "--dump", "specify the module name for the dump "
	       "e.g. tx_buf, vlan, ether, dmac, smac, all");
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

	g_ncsi_cmd_info.cmd_flag = 0;
	major_cmd->option_count = 0;
	major_cmd->execute = nic_ncsi_cmd_execute;

	cmd_option_register("-h", "--help",       false, nic_ncsi_cmd_show_help);
	cmd_option_register("-i", "--interface",  true,  nic_ncsi_cmd_get_port_info);
	cmd_option_register("-d", "--dump",       true,  nic_ncsi_cmd_get_dump_mode);
}

HIKP_CMD_DECLARE("nic_ncsi", "query nic port ncsi information", cmd_nic_get_ncsi_init);
