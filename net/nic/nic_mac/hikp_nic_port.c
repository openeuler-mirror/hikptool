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
#include "hikp_mac_cmd.h"
#include "hikp_nic_port.h"

static struct cmd_port_info g_port_info = {0};

struct mac_item g_fec_table[] = {
	{PORT_CFG_FEC_NOT_SET, "unset"},
	{PORT_CFG_FEC_RSFEC, "rsfec"},
	{PORT_CFG_FEC_BASEFEC, "basefec"},
	{PORT_CFG_FEC_NOFEC, "nofec"},
	{PORT_CFG_FEC_AUTO, "auto"},
	{PORT_CFG_FEC_LLRSFEC, "llrsfec"}
};

struct mac_item g_speed_table[] = {
	{PORT_SPEED_NOT_SET, "unset"},
	{PORT_SPEED_10MB, "10M"},
	{PORT_SPEED_100MB, "100M"},
	{PORT_SPEED_1GB, "1G"},
	{PORT_SPEED_10GB, "10G"},
	{PORT_SPEED_25GB, "25G"},
	{PORT_SPEED_40GB, "40G"},
	{PORT_SPEED_50GB, "50G"},
	{PORT_SPEED_100GB, "100G"},
	{PORT_SPEED_200GB, "200G"},
};

struct mac_item g_an_table[] = {
	{PORT_CFG_NOT_SET, "unset"},
	{PORT_CFG_AN_ON, "on"},
	{PORT_CFG_AN_OFF, "off"},
};

struct mac_item g_adapt_table[] = {
	{PORT_CFG_ADAPT_NOT_SET, "unset"},
	{PORT_CFG_ADAPT_ON, "on"},
	{PORT_CFG_ADAPT_OFF, "off"}
};

struct mac_item g_duplex_table[] = {
	{HIKP_MAC_DUPLEX_UNKNOWN, "unset"},
	{HIKP_MAC_DUPLEX_FULL, "full"},
	{HIKP_MAC_DUPLEX_HALF, "half"},
};

struct mac_item g_sds_rate_table[] = {
	{HIKP_MAC_SDS_RATE_UNKNOWN, "unset"},
	{HIKP_MAC_SDS_RATE_1P25G, "1.25G"},
	{HIKP_MAC_SDS_RATE_10P3125G, "10.3125G"},
	{HIKP_MAC_SDS_RATE_25P78125G, "25.78125G"},
	{HIKP_MAC_SDS_RATE_26P5625G, "26.5625G"},
	{HIKP_MAC_SDS_RATE_53P125G, "53.125G"},
};

struct mac_item g_lanes_table[] = {
	{HIKP_MAC_LANES_UNKNOWN, "unset"},
	{HIKP_MAC_LANES_X1, "X1"},
	{HIKP_MAC_LANES_X2, "X2"},
	{HIKP_MAC_LANES_X4, "X4"},
	{HIKP_MAC_LANES_X8, "X8"},
};

static void mac_show_mask(uint32_t mask, const struct mac_item *table, uint32_t size)
{
	uint32_t i;

	for (i = 0; i < size; i++) {
		if (mask & table[i].key)
			printf("%s ", table[i].name);
	}
	printf("\n");
}

static void mac_print_modes(const char *label, uint32_t mask,
			    const struct mac_item *table, uint32_t size)
{
	printf("%s: ", label);

	mac_show_mask(mask, table, size);
}

static const char *mac_get_str(uint32_t val, const struct mac_item *table,
			       uint32_t size, const char *none_str)
{
	const char *str = none_str;
	uint32_t i;

	for (i = 0; i < size; i++) {
		if (table[i].key == val) {
			str = table[i].name;
			break;
		}
	}

	return str;
}

static void mac_print_enum(const char *label, uint32_t val, const struct mac_item *table,
			   uint32_t size, const char *none_str)
{
	const char *str = mac_get_str(val, table, size, none_str);

	if (str == NULL)
		return;

	printf("%s: %s\n", label, str);
}

static void mac_print_enable(const char *label, uint32_t val)
{
	const char *str = (val != 0) ? "on" : "off";

	printf("%s: %s\n", label, str);
}

static void mac_print_link(const char *label, uint32_t val)
{
	const char *str = (val != 0) ? "link" : "no link";

	printf("%s: %s\n", label, str);
}

static int mac_cmd_get_dfx_cfg(uint32_t sub_cmd, struct hikp_cmd_ret **cmd_ret)
{
	struct hikp_cmd_header req_header = {0};

	hikp_cmd_init(&req_header, MAC_MOD, MAC_CMD_PORT, sub_cmd);
	*cmd_ret = hikp_cmd_alloc(&req_header, &g_port_info.target.bdf,
				  sizeof(g_port_info.target.bdf));
	if (*cmd_ret == NULL)
		return -ENOSPC;

	if ((*cmd_ret)->status != 0) {
		free(*cmd_ret);
		*cmd_ret = NULL;
		return -EAGAIN;
	}

	return 0;
}

static bool is_ge_speed(uint32_t speed)
{
	return speed == PORT_SPEED_10MB || speed == PORT_SPEED_100MB || speed == PORT_SPEED_1GB;
}

static void mac_show_speed(uint32_t speed, uint32_t lanes)
{
	const char *speed_str = mac_get_str(speed, g_speed_table,
					    HIKP_ARRAY_SIZE(g_speed_table), NULL);
	const char *lanes_str = mac_get_str(lanes, g_lanes_table,
					    HIKP_ARRAY_SIZE(g_lanes_table), NULL);

	if (is_ge_speed(speed) || speed_str == NULL || lanes_str == NULL)
		printf("speed: %s\n", (speed_str != NULL) ? speed_str : "unknown");
	else
		printf("speed: %s_%s\n", speed_str, lanes_str);
}

static void mac_cmd_disp_eth_mac_info(const struct mac_cmd_mac_dfx *mac_dfx)
{
	printf("\n========================== MAC INFO ==========================\n");
	mac_show_speed(mac_dfx->speed, mac_dfx->lanes);
	mac_print_enum("fec", mac_dfx->fec, g_fec_table, HIKP_ARRAY_SIZE(g_fec_table), "unknown");
	mac_print_enum("duplex", mac_dfx->duplex, g_duplex_table,
		       HIKP_ARRAY_SIZE(g_duplex_table), "unknown");
	mac_print_enum("sds_rate", mac_dfx->sds_rate, g_sds_rate_table,
		       HIKP_ARRAY_SIZE(g_sds_rate_table), "unknown");
	mac_print_enable("mac_tx_en", mac_dfx->mac_tx_en);
	mac_print_enable("mac_rx_en", mac_dfx->mac_rx_en);
	mac_print_link("pcs_link", mac_dfx->pcs_link);
	mac_print_link("mac_link", mac_dfx->mac_link);
	printf("pma_ctrl = %u\n", mac_dfx->pma_ctrl);
	printf("rf_lf = 0x%x\n", mac_dfx->rf_lf);
	printf("pcs_err = 0x%x\n", mac_dfx->pcs_err_cnt);
}

static void mac_cmd_show_eth_mac(struct major_cmd_ctrl *self)
{
	struct mac_cmd_mac_dfx *mac_dfx = NULL;
	struct hikp_cmd_ret *cmd_ret = NULL;
	int ret;

	ret = mac_cmd_get_dfx_cfg(QUERY_PORT_MAC_DFX, &cmd_ret);
	if (ret != 0) {
		printf("hikp_data_proc get mac dfx failed.\n");
		self->err_no = -ENOSPC;
		return;
	}

	mac_dfx = (struct mac_cmd_mac_dfx *)(cmd_ret->rsp_data);
	mac_cmd_disp_eth_mac_info(mac_dfx);
	free(cmd_ret);
	cmd_ret = NULL;
}

static void mac_cmd_disp_roh_mac_info(const struct mac_cmd_roh_mac_dfx *mac_dfx)
{
	printf("\n========================== MAC INFO ==========================\n");
	mac_show_speed(mac_dfx->speed, mac_dfx->lanes);
	mac_print_enum("fec", mac_dfx->fec, g_fec_table, HIKP_ARRAY_SIZE(g_fec_table), "unknown");
	mac_print_enum("sds_rate", mac_dfx->sds_rate, g_sds_rate_table,
		       HIKP_ARRAY_SIZE(g_sds_rate_table), "unknown");
	printf("tx_link_lanes: %u\n", mac_dfx->tx_link_lanes);
	printf("rx_link_lanes: %u\n", mac_dfx->rx_link_lanes);
	mac_print_link("pcs_link", mac_dfx->pcs_link);
	mac_print_link("mac_link", mac_dfx->mac_link);
	printf("tx_retry_cnt: %u\n", mac_dfx->tx_retry_cnt);
}

static void mac_cmd_show_roh_mac(struct major_cmd_ctrl *self)
{
	struct mac_cmd_roh_mac_dfx *mac_dfx = NULL;
	struct hikp_cmd_ret *cmd_ret = NULL;
	int ret;

	ret = mac_cmd_get_dfx_cfg(QUERY_PORT_ROH_MAC_DFX, &cmd_ret);
	if (ret) {
		snprintf(self->err_str, sizeof(self->err_str), "mac get roh mac dfx failed.");
		self->err_no = -ENOSPC;
		return;
	}

	mac_dfx = (struct mac_cmd_roh_mac_dfx *)(cmd_ret->rsp_data);
	mac_cmd_disp_roh_mac_info(mac_dfx);
	free(cmd_ret);
	cmd_ret = NULL;
}

static void mac_cmd_show_mac(struct major_cmd_ctrl *self)
{
	struct mac_cmd_port_hardware *hw = NULL;
	struct hikp_cmd_ret *hw_cmd_ret = NULL;
	int ret;

	ret = mac_cmd_get_dfx_cfg(QUERY_PORT_HARDWARE, &hw_cmd_ret);
	if (ret) {
		snprintf(self->err_str, sizeof(self->err_str), "mac get hardware dfx failed.");
		self->err_no = -ENOSPC;
		return;
	}

	hw = (struct mac_cmd_port_hardware *)(hw_cmd_ret->rsp_data);
	if (hw->cmd_mac_type == CMD_MAC_TYPE_ROH || hw->cmd_mac_type == CMD_MAC_TYPE_UB)
		mac_cmd_show_roh_mac(self);
	else
		mac_cmd_show_eth_mac(self);

	free(hw_cmd_ret);
	hw_cmd_ret = NULL;
}

static void mac_cmd_disp_link_info(struct mac_cmd_link_dfx *link_dfx)
{
	struct mac_port_param *port_cfg = &link_dfx->port_cfg;

	printf("\n======================= PORT LINK INFO =======================\n");
	mac_print_enum("adapt", port_cfg->adapt, g_adapt_table,
		       HIKP_ARRAY_SIZE(g_adapt_table), "unknown");
	mac_print_enum("autoneg", port_cfg->an, g_an_table, HIKP_ARRAY_SIZE(g_an_table), "unknown");
	mac_show_speed(port_cfg->speed, port_cfg->lanes);
	mac_print_enum("fec", port_cfg->fec, g_fec_table, HIKP_ARRAY_SIZE(g_fec_table), "unknown");
	mac_print_enum("duplex", port_cfg->duplex, g_duplex_table,
		       HIKP_ARRAY_SIZE(g_duplex_table), "unknown");
	mac_print_enum("sds_rate", port_cfg->sds_rate, g_sds_rate_table,
		       HIKP_ARRAY_SIZE(g_sds_rate_table), "unknown");
	mac_print_link("port link", link_dfx->port_link);
	mac_print_enable("port enable", link_dfx->port_enable);
	mac_print_enable("link debug", link_dfx->link_debug_en);
	mac_print_enable("link report", link_dfx->link_report_en);
	printf("cur_link_machine = 0x%x\n", link_dfx->cur_link_machine);
	printf("his_link_machine = 0x%x\n", link_dfx->his_link_machine);
}

static void mac_cmd_show_link(struct major_cmd_ctrl *self)
{
	struct mac_cmd_link_dfx *link_dfx = NULL;
	struct hikp_cmd_ret *cmd_ret = NULL;
	int ret;

	ret = mac_cmd_get_dfx_cfg(QUERY_PORT_LINK_DFX, &cmd_ret);
	if (ret != 0) {
		printf("hikp_data_proc get link dfx failed.\n");
		self->err_no = -ENOSPC;
		return;
	}

	link_dfx = (struct mac_cmd_link_dfx *)(cmd_ret->rsp_data);
	mac_cmd_disp_link_info(link_dfx);
	free(cmd_ret);
}

static void mac_cmd_disp_phy_reg(const uint16_t *reg, uint32_t num)
{
#define PRINT_OFFSET    4

	uint32_t i;

	for (i = 0; i < num; i++) {
		printf("reg %2u : 0x%04x\t", i, reg[i]);

		if (i % PRINT_OFFSET == (PRINT_OFFSET - 1))
			printf("\n");
	}
}

static void mac_cmd_disp_phy_info(const struct mac_cfg_phy_cfg *phy_param,
				  const struct mac_cmd_phy_dfx *phy_dfx)
{
	struct mac_item dup_ext_tbl[] = {
		{HIKP_PHY_DUPLEX_HALF, "half"}, {HIKP_PHY_DUPLEX_FULL, "full"},
	};
	struct mac_item phy_abi_tbl[] = {
		{HIKP_MAC_PHY_ABI_10M_HALF, "10M/Half"}, {HIKP_MAC_PHY_ABI_10M_FULL, "10M/Full"},
		{HIKP_MAC_PHY_ABI_100M_HALF, "100M/Half"},
		{HIKP_MAC_PHY_ABI_100M_FULL, "100M/Full"},
		{HIKP_MAC_PHY_ABI_1000M_HALF, "1000M/Half"},
		{HIKP_MAC_PHY_ABI_1000M_FULL, "1000M/Full"},
	};

	printf("\n========================== PHY INFO ==========================\n");
	printf("phy_addr = %u\n", phy_param->phy_addr);
	mac_print_enable("autoneg", phy_param->autoneg);
	printf("speed: %u\n", phy_param->speed);
	mac_print_enum("duplex", phy_param->duplex, dup_ext_tbl,
		       HIKP_ARRAY_SIZE(dup_ext_tbl), "unknown");
	mac_print_modes("supported", phy_param->supported, phy_abi_tbl,
			HIKP_ARRAY_SIZE(phy_abi_tbl));
	mac_print_modes("advertised",  phy_param->advertising, phy_abi_tbl,
			HIKP_ARRAY_SIZE(phy_abi_tbl));
	mac_print_modes("LP advertised",  phy_param->lp_advertising, phy_abi_tbl,
			HIKP_ARRAY_SIZE(phy_abi_tbl));

	printf("--------------------------------------------------------------\n");
	mac_cmd_disp_phy_reg(phy_dfx->reg_val, MAC_PHY_DFX_REG_NUM);
}

static void mac_cmd_show_phy(struct major_cmd_ctrl *self)
{
	struct mac_cfg_phy_cfg *phy_cfg = NULL;
	struct mac_cmd_phy_dfx *phy_dfx = NULL;
	struct hikp_cmd_ret *phy_cfg_ret = NULL;
	struct hikp_cmd_ret *phy_dfx_ret = NULL;
	int ret;

	ret = mac_cmd_get_dfx_cfg(QUERY_PHY_KSETTING, &phy_cfg_ret);
	if (ret != 0) {
		printf("hikp_data_proc get phy cfg failed.\n");
		self->err_no = -ENOSPC;
		return;
	}
	phy_cfg = (struct mac_cfg_phy_cfg *)(phy_cfg_ret->rsp_data);

	ret = mac_cmd_get_dfx_cfg(QUERY_PORT_PHY_DFX, &phy_dfx_ret);
	if (ret != 0) {
		printf("hikp_data_proc get phy dfx failed.\n");
		self->err_no = -ENOSPC;
		free(phy_cfg_ret);
		return;
	}
	phy_dfx = (struct mac_cmd_phy_dfx *)(phy_dfx_ret->rsp_data);
	mac_cmd_disp_phy_info(phy_cfg, phy_dfx);
	free(phy_cfg_ret);
	free(phy_dfx_ret);
}

static void mac_cmd_disp_port_param(const char *label, const struct mac_port_param *port)
{
	const char *adapt_str = mac_get_str(port->adapt, g_adapt_table,
					    HIKP_ARRAY_SIZE(g_adapt_table), "unset");
	const char *speed_str = mac_get_str(port->speed, g_speed_table,
					    HIKP_ARRAY_SIZE(g_speed_table), "unset");
	const char *lanes_str = mac_get_str(port->lanes, g_lanes_table,
					    HIKP_ARRAY_SIZE(g_lanes_table), "unset");
	const char *dup_str = mac_get_str(port->duplex, g_duplex_table,
					  HIKP_ARRAY_SIZE(g_duplex_table), "unset");
	const char *sds_str = mac_get_str(port->sds_rate, g_sds_rate_table,
					  HIKP_ARRAY_SIZE(g_sds_rate_table), "unset");
	const char *fec_str = mac_get_str(port->fec, g_fec_table,
					  HIKP_ARRAY_SIZE(g_fec_table), "unset");
	const char *an_str = mac_get_str(port->an, g_an_table,
					 HIKP_ARRAY_SIZE(g_an_table), "unset");

	printf("%s\t|%-10s%-10s%-10s%-10s%-10s%-10s%-10s\n", label,
	       adapt_str, an_str, speed_str, lanes_str, fec_str, dup_str, sds_str);
}

static void mac_cmd_disp_arb_info(const struct mac_cmd_arb_dfx *arb_dfx)
{
	printf("\n======================== ARB LINK INFO =======================\n");

	printf("area\t|adapt    |an       |speed    |lanes    |fec      |duplex   |sds_rate\n");
	printf("----------------------------------------------------------------------------\n");

	mac_cmd_disp_port_param("Default", &arb_dfx->default_cfg);
	mac_cmd_disp_port_param("BIOS", &arb_dfx->bios_cfg);
	mac_cmd_disp_port_param("TOOL", &arb_dfx->user_cfg);
	mac_cmd_disp_port_param("ARB", &arb_dfx->arb_cfg);
	mac_cmd_disp_port_param("Final", &arb_dfx->port_cfg);
}

static void mac_cmd_disp_hot_plug_card_info(const struct cmd_hot_plug_card_info *hpc_dfx)
{
	printf("\n===================== HOT PLUG CARD INFO =====================\n");

	printf("hot plug card in position: 0x%x\n", hpc_dfx->in_pos);
	printf("support type: 0x%x\n", hpc_dfx->support_type);
	if (hpc_dfx->in_pos)
		printf("current type: 0x%x\n", hpc_dfx->cur_type);
	printf("----------------------------------------------------------------------------\n");
}

static void mac_cmd_show_arb(struct major_cmd_ctrl *self)
{
	struct mac_cmd_arb_dfx *arb_dfx = NULL;
	struct hikp_cmd_ret *cmd_ret = NULL;
	int ret;

	ret = mac_cmd_get_dfx_cfg(QUERY_PORT_ARB_DFX, &cmd_ret);
	if (ret != 0) {
		printf("hikp_data_proc get arb dfx failed.\n");
		self->err_no = -ENOSPC;
		return;
	}

	arb_dfx = (struct mac_cmd_arb_dfx *)(cmd_ret->rsp_data);
	mac_cmd_disp_arb_info(arb_dfx);
	free(cmd_ret);
}

static void mac_cmd_show_hot_plug_card(struct major_cmd_ctrl *self)
{
	struct cmd_hot_plug_card_info *hpc_dfx = NULL;
	struct hikp_cmd_ret *cmd_ret = NULL;
	int ret;

	ret = mac_cmd_get_dfx_cfg(QUERY_HOT_PLUG_CARD_DFX, &cmd_ret);
	if (ret != 0) {
		printf("hikp_data_proc get hot plug card dfx failed.\n");
		self->err_no = -ENOSPC;
		return;
	}

	hpc_dfx = (struct cmd_hot_plug_card_info *)(cmd_ret->rsp_data);
	mac_cmd_disp_hot_plug_card_info(hpc_dfx);
	free(cmd_ret);
}

static void mac_cmd_print_cdr_dfx(struct mac_cmd_cdr_dfx *cdr_dfx, struct mac_port_cdr_dfx *info)
{
	struct mac_item type_table[] = {
		{PORT_CDR_TYPE_A, "cdr_a"}, {PORT_CDR_TYPE_B, "cdr_b"},
	};
	struct mac_item cdr_a_mode[] = {
		{CDR_A_MODE_2PLL, "2pll"}, {CDR_A_MODE_FASTPI, "fastpi"},
	};
	struct mac_item cdr_b_mode[] = {
		{CDR_B_MODE_PCS, "pcs"}, {CDR_B_MODE_CDR, "cdr"},
	};
	struct mac_item status_table[] = {
		{CDR_STATUS_NORMAL, "normal"}, {CDR_STATUS_ERROR, "error"},
	};
	const char *type_str = mac_get_str(cdr_dfx->cdr_type,
					   type_table, HIKP_ARRAY_SIZE(type_table), "unknown");
	const char *mode_str = "NA";

	for (uint32_t i = 0; i < cdr_dfx->cdr_num; i++) {
		if (cdr_dfx->cdr_type == PORT_CDR_TYPE_A) {
			mode_str = mac_get_str(info->dfx[i].cdr_mode, cdr_a_mode,
					       HIKP_ARRAY_SIZE(cdr_a_mode), "unknown");
		} else if (cdr_dfx->cdr_type == PORT_CDR_TYPE_B) {
			mode_str = mac_get_str(info->dfx[i].cdr_mode, cdr_b_mode,
					       HIKP_ARRAY_SIZE(cdr_b_mode), "unknown");
		}
		printf("\t|0x%-8x%-9u%-10s%-10s%-10s\n", info->dfx[i].cdr_addr,
		       info->dfx[i].cdr_start_lane, type_str, mode_str,
		       mac_get_str(info->dfx[i].cdr_err,
				   status_table, HIKP_ARRAY_SIZE(status_table), "unknown"));
	}
}

static void mac_cmd_disp_cdr_info(struct mac_cmd_cdr_dfx *cdr_dfx)
{
	uint8_t cdr_max_num = HIKP_ARRAY_SIZE(cdr_dfx->wire_cdr.dfx);

	if (!cdr_dfx->cdr_num)
		return;

	if (cdr_dfx->cdr_num > cdr_max_num) {
		printf("the cdr_num(%u) exceeds %u\n", cdr_dfx->cdr_num, cdr_max_num);
		return;
	}

	printf("\n======================== PORT CDR INFO =======================\n");
	printf("direct\t|addr     |lane    |type     |mode     |status   \n");
	printf("----------------------------------------------------------------------------\n");

	printf("WIRE");
	mac_cmd_print_cdr_dfx(cdr_dfx, &cdr_dfx->wire_cdr);

	printf("HOST");
	mac_cmd_print_cdr_dfx(cdr_dfx, &cdr_dfx->host_cdr);
}

static void mac_cmd_show_cdr(struct major_cmd_ctrl *self)
{
	struct mac_cmd_cdr_dfx *cdr_dfx = NULL;
	struct hikp_cmd_ret *cmd_ret = NULL;
	int ret;

	ret = mac_cmd_get_dfx_cfg(QUERY_PORT_CDR_DFX, &cmd_ret);
	if (ret != 0) {
		self->err_no = -ENOSPC;
		snprintf(self->err_str, sizeof(self->err_str), "mac get cdr dfx failed.");
		return;
	}

	cdr_dfx = (struct mac_cmd_cdr_dfx *)(cmd_ret->rsp_data);
	mac_cmd_disp_cdr_info(cdr_dfx);
	free(cmd_ret);
}

static void mac_cmd_show_port_dfx(struct major_cmd_ctrl *self, uint32_t mask)
{
	struct mac_cmd_dfx_callback dfx_cb[] = {
		{MAC_LSPORT_LINK, mac_cmd_show_link},
		{MAC_LSPORT_MAC, mac_cmd_show_mac},
		{MAC_LSPORT_PHY, mac_cmd_show_phy},
		{MAC_LSPORT_ARB, mac_cmd_show_arb},
		{MAC_HOT_PLUG_CARD, mac_cmd_show_hot_plug_card},
		{MAC_LSPORT_CDR, mac_cmd_show_cdr}
	};
	size_t size = HIKP_ARRAY_SIZE(dfx_cb);
	size_t i;

	for (i = 0; i < size; i++) {
		if (mask & dfx_cb[i].mask)
			dfx_cb[i].show_dfx(self);
	}
}

static int mac_cmd_get_port_dfx_cap(uint32_t *cap)
{
	struct mac_cmd_port_hardware *port_hw = NULL;
	struct mac_cmd_port_dfx_cap *dfx_cap = NULL;
	struct hikp_cmd_ret *dfx_cap_resp = NULL;
	struct hikp_cmd_ret *hw_cmd_ret = NULL;
	int ret;

	ret = mac_cmd_get_dfx_cfg(QUERY_PORT_INFO_DFX_CAP, &dfx_cap_resp);
	if (ret == 0) {
		dfx_cap = (struct mac_cmd_port_dfx_cap *)dfx_cap_resp->rsp_data;
		*cap = dfx_cap->cap_bit_map;
		free(dfx_cap_resp);
		dfx_cap_resp = NULL;
		return ret;
	}

	/* not support get capability, so use old process */
	ret = mac_cmd_get_dfx_cfg(QUERY_PORT_HARDWARE, &hw_cmd_ret);
	if (ret)
		return ret;

	*cap = MAC_LSPORT_LINK | MAC_LSPORT_MAC | MAC_LSPORT_ARB |
	       MAC_HOT_PLUG_CARD | MAC_LSPORT_CDR;
	port_hw = (struct mac_cmd_port_hardware *)(hw_cmd_ret->rsp_data);
	if (port_hw->port_type == HIKP_PORT_TYPE_PHY ||
	    port_hw->port_type == HIKP_PORT_TYPE_PHY_SDS)
		*cap |= MAC_LSPORT_PHY;

	free(hw_cmd_ret);
	hw_cmd_ret = NULL;
	return ret;
}

static void mac_cmd_port_execute(struct major_cmd_ctrl *self)
{
	uint32_t dfx_cap;
	int ret;

	if (!g_port_info.port_flag) {
		self->err_no = -EINVAL;
		snprintf(self->err_str, sizeof(self->err_str), "Need port id.");
		return;
	}

	ret = mac_cmd_get_port_dfx_cap(&dfx_cap);
	if (ret) {
		self->err_no = ret;
		snprintf(self->err_str, sizeof(self->err_str), "Get DFX capability failed.");
		return;
	}

	mac_cmd_show_port_dfx(self, dfx_cap);
}

static int mac_cmd_get_port_target(struct major_cmd_ctrl *self, const char *argv)
{
	self->err_no = tool_check_and_get_valid_bdf_id(argv, &g_port_info.target);
	if (self->err_no) {
		snprintf(self->err_str, sizeof(self->err_str), "Unknown device %s.", argv);
		return self->err_no;
	}
	g_port_info.port_flag = true;

	return 0;
}

static int mac_cmd_port_show_help(struct major_cmd_ctrl *self, const char *argv)
{
	printf("\n  Usage: %s %s\n", self->cmd_ptr->name, "-i <interface>");
	printf("\n         %s\n", self->cmd_ptr->help_info);
	printf("\n  Options:\n\n");
	printf("    %s, %-25s %s\n", "-h", "--help", "display this help and exit");
	printf("    %s, %-25s %s\n", "-i", "--interface=<interface>",
	       "device target or bdf id, e.g. eth0~3 or 0000:34:00.0");
	printf("\n");

	return 0;
}

static void cmd_mac_get_port_init(void)
{
	struct major_cmd_ctrl *major_cmd = get_major_cmd();

	g_port_info.port_flag = false;

	major_cmd->option_count = 0;
	major_cmd->execute = mac_cmd_port_execute;

	cmd_option_register("-h", "--help",       false, mac_cmd_port_show_help);
	cmd_option_register("-i", "--interface",  true,  mac_cmd_get_port_target);
}

HIKP_CMD_DECLARE("nic_port", "query nic port information", cmd_mac_get_port_init);
