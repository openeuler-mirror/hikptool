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
#include "hikp_nic_mac_dump.h"

static struct cmd_mac_dump g_dump_reg_info = {0};

static const struct mac_reg_name rx_mac_phy_link_status_r[] = {
	{ "RX_MAC_PHY0_LINK_STATUS" },
	{ "RX_MAC_PHY1_LINK_STATUS" },
	{ "RX_MAC_PHY2_LINK_STATUS" },
	{ "RX_MAC_PHY3_LINK_STATUS" },
	{ "RX_MAC_PHY4_LINK_STATUS" },
	{ "RX_MAC_PHY5_LINK_STATUS" },
	{ "RX_MAC_PHY6_LINK_STATUS" },
	{ "RX_MAC_PHY7_LINK_STATUS" },
	{ "RX_MAC_PHY0_LINK_CONTROL" },
	{ "RX_MAC_PHY1_LINK_CONTROL" },
	{ "RX_MAC_PHY2_LINK_CONTROL" },
	{ "RX_MAC_PHY3_LINK_CONTROL" },
	{ "RX_MAC_PHY4_LINK_CONTROL" },
	{ "RX_MAC_PHY5_LINK_CONTROL" },
	{ "RX_MAC_PHY6_LINK_CONTROL" },
	{ "RX_MAC_PHY7_LINK_CONTROL" },
	{ "RX_MAC_PHY0_LFRF_STATUS" },
	{ "RX_MAC_PHY1_LFRF_STATUS" },
	{ "RX_MAC_PHY2_LFRF_STATUS" },
	{ "RX_MAC_PHY3_LFRF_STATUS" },
	{ "RX_MAC_PHY4_LFRF_STATUS" },
	{ "RX_MAC_PHY5_LFRF_STATUS" },
	{ "RX_MAC_PHY6_LFRF_STATUS" },
	{ "RX_MAC_PHY7_LFRF_STATUS" },
};

static const struct mac_reg_name rx_mac_phy_1588_cfg_r[] = {
	{ "RX_MAC_PHY0_1588_CFG" },
	{ "RX_MAC_PHY1_1588_CFG" },
	{ "RX_MAC_PHY2_1588_CFG" },
	{ "RX_MAC_PHY3_1588_CFG" },
	{ "RX_MAC_PHY4_1588_CFG" },
	{ "RX_MAC_PHY5_1588_CFG" },
	{ "RX_MAC_PHY6_1588_CFG" },
	{ "RX_MAC_PHY7_1588_CFG" },
	{ "RX_MAC_PORT0_RSTN_LOGIC" },
	{ "RX_MAC_PORT1_RSTN_LOGIC" },
	{ "RX_MAC_PORT2_RSTN_LOGIC" },
	{ "RX_MAC_PORT3_RSTN_LOGIC" },
	{ "RX_MAC_PORT4_RSTN_LOGIC" },
	{ "RX_MAC_PORT5_RSTN_LOGIC" },
	{ "RX_MAC_PORT6_RSTN_LOGIC" },
	{ "RX_MAC_PORT7_RSTN_LOGIC" },
	{ "RX_MAC_PORT0_ENABLE" },
	{ "RX_MAC_PORT1_ENABLE" },
	{ "RX_MAC_PORT2_ENABLE" },
	{ "RX_MAC_PORT3_ENABLE" },
	{ "RX_MAC_PORT4_ENABLE" },
	{ "RX_MAC_PORT5_ENABLE" },
	{ "RX_MAC_PORT6_ENABLE" },
	{ "RX_MAC_PORT7_ENABLE" },
};

static const struct mac_reg_name rx_mac_port_control_r[] = {
	{ "RX_MAC_PORT0_CONTROL" },
	{ "RX_MAC_PORT1_CONTROL" },
	{ "RX_MAC_PORT2_CONTROL" },
	{ "RX_MAC_PORT3_CONTROL" },
	{ "RX_MAC_PORT4_CONTROL" },
	{ "RX_MAC_PORT5_CONTROL" },
	{ "RX_MAC_PORT6_CONTROL" },
	{ "RX_MAC_PORT7_CONTROL" },
	{ "RX_MAC_PORT0_CONTROL1" },
	{ "RX_MAC_PORT1_CONTROL1" },
	{ "RX_MAC_PORT2_CONTROL1" },
	{ "RX_MAC_PORT3_CONTROL1" },
	{ "RX_MAC_PORT4_CONTROL1" },
	{ "RX_MAC_PORT5_CONTROL1" },
	{ "RX_MAC_PORT6_CONTROL1" },
	{ "RX_MAC_PORT7_CONTROL1" },
	{ "RX_MAC_PORT0_PAUSE_CTRL" },
	{ "RX_MAC_PORT1_PAUSE_CTRL" },
	{ "RX_MAC_PORT2_PAUSE_CTRL" },
	{ "RX_MAC_PORT3_PAUSE_CTRL" },
	{ "RX_MAC_PORT4_PAUSE_CTRL" },
	{ "RX_MAC_PORT5_PAUSE_CTRL" },
	{ "RX_MAC_PORT6_PAUSE_CTRL" },
	{ "RX_MAC_PORT7_PAUSE_CTRL" },
};

static const struct mac_reg_name rx_mac_port_1588_ctrl_r[] = {
	{ "RX_MAC_PORT0_1588_CTRL" },
	{ "RX_MAC_PORT1_1588_CTRL" },
	{ "RX_MAC_PORT2_1588_CTRL" },
	{ "RX_MAC_PORT3_1588_CTRL" },
	{ "RX_MAC_PORT4_1588_CTRL" },
	{ "RX_MAC_PORT5_1588_CTRL" },
	{ "RX_MAC_PORT6_1588_CTRL" },
	{ "RX_MAC_PORT7_1588_CTRL" },
	{ "RX_MAC_PORT0_1588_PORT_DELAY" },
	{ "RX_MAC_PORT1_1588_PORT_DELAY" },
	{ "RX_MAC_PORT2_1588_PORT_DELAY" },
	{ "RX_MAC_PORT3_1588_PORT_DELAY" },
	{ "RX_MAC_PORT4_1588_PORT_DELAY" },
	{ "RX_MAC_PORT5_1588_PORT_DELAY" },
	{ "RX_MAC_PORT6_1588_PORT_DELAY" },
	{ "RX_MAC_PORT7_1588_PORT_DELAY" },
};

static const struct mac_reg_name rx_mac_port_diag_curr_status_r[] = {
	{ "RX_MAC_PORT0_DIAG_CURR_STATUS" },
	{ "RX_MAC_PORT1_DIAG_CURR_STATUS" },
	{ "RX_MAC_PORT2_DIAG_CURR_STATUS" },
	{ "RX_MAC_PORT3_DIAG_CURR_STATUS" },
	{ "RX_MAC_PORT4_DIAG_CURR_STATUS" },
	{ "RX_MAC_PORT5_DIAG_CURR_STATUS" },
	{ "RX_MAC_PORT6_DIAG_CURR_STATUS" },
	{ "RX_MAC_PORT7_DIAG_CURR_STATUS" },
	{ "RX_MAC_PORT0_DIAG_HIS_STATUS" },
	{ "RX_MAC_PORT1_DIAG_HIS_STATUS" },
	{ "RX_MAC_PORT2_DIAG_HIS_STATUS" },
	{ "RX_MAC_PORT3_DIAG_HIS_STATUS" },
	{ "RX_MAC_PORT4_DIAG_HIS_STATUS" },
	{ "RX_MAC_PORT5_DIAG_HIS_STATUS" },
	{ "RX_MAC_PORT6_DIAG_HIS_STATUS" },
	{ "RX_MAC_PORT7_DIAG_HIS_STATUS" },
	{ "RX_MAC_PORT0_DIAG_EFD_ERR_CNT" },
	{ "RX_MAC_PORT1_DIAG_EFD_ERR_CNT" },
	{ "RX_MAC_PORT2_DIAG_EFD_ERR_CNT" },
	{ "RX_MAC_PORT3_DIAG_EFD_ERR_CNT" },
	{ "RX_MAC_PORT4_DIAG_EFD_ERR_CNT" },
	{ "RX_MAC_PORT5_DIAG_EFD_ERR_CNT" },
	{ "RX_MAC_PORT6_DIAG_EFD_ERR_CNT" },
	{ "RX_MAC_PORT7_DIAG_EFD_ERR_CNT" },
};

static const struct mac_reg_name rx_mac_port_diag_pre_err_cnt_r[] = {
	{ "RX_MAC_PORT0_DIAG_PRE_ERR_CNT" },
	{ "RX_MAC_PORT1_DIAG_PRE_ERR_CNT" },
	{ "RX_MAC_PORT2_DIAG_PRE_ERR_CNT" },
	{ "RX_MAC_PORT3_DIAG_PRE_ERR_CNT" },
	{ "RX_MAC_PORT4_DIAG_PRE_ERR_CNT" },
	{ "RX_MAC_PORT5_DIAG_PRE_ERR_CNT" },
	{ "RX_MAC_PORT6_DIAG_PRE_ERR_CNT" },
	{ "RX_MAC_PORT7_DIAG_PRE_ERR_CNT" },
};

static const struct mac_reg_name rx_mac_phy_diag_his_status_r[] = {
	{ "RX_MAC_PHY0_DIAG_HIS_STATUS" },
	{ "RX_MAC_PHY1_DIAG_HIS_STATUS" },
	{ "RX_MAC_PHY2_DIAG_HIS_STATUS" },
	{ "RX_MAC_PHY3_DIAG_HIS_STATUS" },
	{ "RX_MAC_PHY4_DIAG_HIS_STATUS" },
	{ "RX_MAC_PHY5_DIAG_HIS_STATUS" },
	{ "RX_MAC_PHY6_DIAG_HIS_STATUS" },
	{ "RX_MAC_PHY7_DIAG_HIS_STATUS" },
	{ "RX_MAC_PHY0_DIAG_1588_REF_GAP_JIT_MAX" },
	{ "RX_MAC_PHY1_DIAG_1588_REF_GAP_JIT_MAX" },
	{ "RX_MAC_PHY2_DIAG_1588_REF_GAP_JIT_MAX" },
	{ "RX_MAC_PHY3_DIAG_1588_REF_GAP_JIT_MAX" },
	{ "RX_MAC_PHY4_DIAG_1588_REF_GAP_JIT_MAX" },
	{ "RX_MAC_PHY5_DIAG_1588_REF_GAP_JIT_MAX" },
	{ "RX_MAC_PHY6_DIAG_1588_REF_GAP_JIT_MAX" },
	{ "RX_MAC_PHY7_DIAG_1588_REF_GAP_JIT_MAX" },
};

static const struct mac_reg_info mac_type_rx_mac_blk[] = {
	{ rx_mac_phy_link_status_r,		HIKP_ARRAY_SIZE(rx_mac_phy_link_status_r) },
	{ rx_mac_phy_1588_cfg_r,		HIKP_ARRAY_SIZE(rx_mac_phy_1588_cfg_r) },
	{ rx_mac_port_control_r,		HIKP_ARRAY_SIZE(rx_mac_port_control_r) },
	{ rx_mac_port_1588_ctrl_r,		HIKP_ARRAY_SIZE(rx_mac_port_1588_ctrl_r) },
	{ rx_mac_port_diag_curr_status_r,	HIKP_ARRAY_SIZE(rx_mac_port_diag_curr_status_r) },
	{ rx_mac_port_diag_pre_err_cnt_r,	HIKP_ARRAY_SIZE(rx_mac_port_diag_pre_err_cnt_r) },
	{ rx_mac_phy_diag_his_status_r,		HIKP_ARRAY_SIZE(rx_mac_phy_diag_his_status_r) },
};

static const struct mac_reg_name rx_mac_port_diag_filter_cnt_r[] = {
	{ "RX_MAC_PORT0_DIAG_FILTER_CNT" },
	{ "RX_MAC_PORT1_DIAG_FILTER_CNT" },
	{ "RX_MAC_PORT2_DIAG_FILTER_CNT" },
	{ "RX_MAC_PORT3_DIAG_FILTER_CNT" },
	{ "RX_MAC_PORT4_DIAG_FILTER_CNT" },
	{ "RX_MAC_PORT5_DIAG_FILTER_CNT" },
	{ "RX_MAC_PORT6_DIAG_FILTER_CNT" },
	{ "RX_MAC_PORT7_DIAG_FILTER_CNT" },
};

static const struct mac_reg_info mac_type_rx_mac_64_blk[] = {
	{ rx_mac_port_diag_filter_cnt_r,	HIKP_ARRAY_SIZE(rx_mac_port_diag_filter_cnt_r) },
};

static const struct mac_reg_name rx_mac_int_status_r[] = {
	{ "RX_MAC_INT_STATUS" },
	{ "RX_MAC_LF_INT_STATUS" },
	{ "RX_MAC_RF_INT_STATUS" },
	{ "RX_MAC_LINK_UP_INT_STATUS" },
	{ "RX_MAC_LINK_DOWN_INT_STATUS" },
	{ "RX_MAC_REF_1588_OVF_INT_STATUS" },
	{ "RX_MAC_PREAMB_ERR_INT_STATUS" },
	{ "RX_MAC_EOP_TIMEOUT_CYC" },
	{ "RX_MAC_DBG_1588_CFG0" },
	{ "RX_MAC_DBG_1588_CFG1" },
	{ "RX_MAC_DBG_1588_CHK0_MAX" },
	{ "RX_MAC_DBG_1588_CHK1_MAX" },
	{ "RX_MAC_DBG_1588_CHK0_MIN" },
	{ "RX_MAC_DBG_1588_CHK1_MIN" },
};

static const struct mac_reg_info mac_type_rx_mac_reg[] = {
	{ rx_mac_int_status_r,		HIKP_ARRAY_SIZE(rx_mac_int_status_r) }
};

static const struct mac_type_name_parse g_mac_rx_mac_name_parse[] = {
	{ mac_type_rx_mac_blk, 		HIKP_ARRAY_SIZE(mac_type_rx_mac_blk),
		HIKP_ARRAY_SIZE(mac_type_rx_mac_blk), true },
	{ mac_type_rx_mac_64_blk, 	HIKP_ARRAY_SIZE(mac_type_rx_mac_64_blk),
		HIKP_ARRAY_SIZE(mac_type_rx_mac_64_blk), true },
	{ mac_type_rx_mac_reg, 		HIKP_ARRAY_SIZE(mac_type_rx_mac_reg),
		ROUND_UP(mac_type_rx_mac_reg[0].reg_num, PER_BLK_DATA_SIZE), false },
};

static const struct mac_reg_name rx_pcs_int_status_r[] = {
	{ "RX_PCS_INT_STATUS" },
	{ "RX_PCS_OVF_INT_STATUS" },
	{ "RX_PCS_SD_INT_STATUS" },
	{ "RX_PCS_SF_INT_STATUS" },
	{ "RX_PCS_RSFEC_TDM_DELAY" },
	{ "RX_PCS_LINK_UP_TYPE_CTRL" },
	{ "RX_PCS_DBG_DEC_CAP" },
	{ "RX_PCS_DBG_DEC_CAP_CMD" },
	{ "RX_PCS_DBG_DEC_CAP_STATUS" },
	{ "RX_PCS_DBG_DEC_CAP_DATA_0" },
	{ "RX_PCS_DBG_DEC_CAP_DATA_1" },
	{ "RX_PCS_DBG_DEC_CAP_DATA_2" },
	{ "RX_PCS_LINK_STATUS" },
};

static const struct mac_reg_info mac_type_rx_pcs_reg[] = {
	{ rx_pcs_int_status_r,		HIKP_ARRAY_SIZE(rx_pcs_int_status_r) },
};

static const struct mac_reg_name rx_pcs_phy0_ctrl_cfg_r[] = {
	{ "RX_PCS_PHY0_CTRL_CFG" },
	{ "RX_PCS_PHY1_CTRL_CFG" },
	{ "RX_PCS_PHY2_CTRL_CFG" },
	{ "RX_PCS_PHY3_CTRL_CFG" },
	{ "RX_PCS_PHY4_CTRL_CFG" },
	{ "RX_PCS_PHY5_CTRL_CFG" },
	{ "RX_PCS_PHY6_CTRL_CFG" },
	{ "RX_PCS_PHY7_CTRL_CFG" },
};

static const struct mac_reg_name rx_pcs_phy0_link_timeout_cfg_r[] = {
	{ "RX_PCS_PHY0_LINK_TIMEOUT_CFG" },
	{ "RX_PCS_PHY1_LINK_TIMEOUT_CFG" },
	{ "RX_PCS_PHY2_LINK_TIMEOUT_CFG" },
	{ "RX_PCS_PHY3_LINK_TIMEOUT_CFG" },
	{ "RX_PCS_PHY4_LINK_TIMEOUT_CFG" },
	{ "RX_PCS_PHY5_LINK_TIMEOUT_CFG" },
	{ "RX_PCS_PHY6_LINK_TIMEOUT_CFG" },
	{ "RX_PCS_PHY7_LINK_TIMEOUT_CFG" },
};

static const struct mac_reg_name rx_pcs_phy0_ber_ctrl_cfg_r[] = {
	{ "RX_PCS_PHY0_BER_CTRL_CFG" },
	{ "RX_PCS_PHY1_BER_CTRL_CFG" },
	{ "RX_PCS_PHY2_BER_CTRL_CFG" },
	{ "RX_PCS_PHY3_BER_CTRL_CFG" },
	{ "RX_PCS_PHY4_BER_CTRL_CFG" },
	{ "RX_PCS_PHY5_BER_CTRL_CFG" },
	{ "RX_PCS_PHY6_BER_CTRL_CFG" },
	{ "RX_PCS_PHY7_BER_CTRL_CFG" },
};

static const struct mac_reg_name rx_pcs_phy0_err_block_cnt_r[] = {
	{ "RX_PCS_PHY0_ERR_BLOCK_CNT" },
	{ "RX_PCS_PHY1_ERR_BLOCK_CNT" },
	{ "RX_PCS_PHY2_ERR_BLOCK_CNT" },
	{ "RX_PCS_PHY3_ERR_BLOCK_CNT" },
	{ "RX_PCS_PHY4_ERR_BLOCK_CNT" },
	{ "RX_PCS_PHY5_ERR_BLOCK_CNT" },
	{ "RX_PCS_PHY6_ERR_BLOCK_CNT" },
	{ "RX_PCS_PHY7_ERR_BLOCK_CNT" },
};

static const struct mac_reg_name rx_pcs_phy0_e_blk_cnt_r[] = {
	{ "RX_PCS_PHY0_E_BLK_CNT" },
	{ "RX_PCS_PHY1_E_BLK_CNT" },
	{ "RX_PCS_PHY2_E_BLK_CNT" },
	{ "RX_PCS_PHY3_E_BLK_CNT" },
	{ "RX_PCS_PHY4_E_BLK_CNT" },
	{ "RX_PCS_PHY5_E_BLK_CNT" },
	{ "RX_PCS_PHY6_E_BLK_CNT" },
	{ "RX_PCS_PHY7_E_BLK_CNT" },
};

static const struct mac_reg_name rx_pcs_phy0_dec_err_blk_cnt_r[] = {
	{ "RX_PCS_PHY0_DEC_ERR_BLK_CNT" },
	{ "RX_PCS_PHY1_DEC_ERR_BLK_CNT" },
	{ "RX_PCS_PHY2_DEC_ERR_BLK_CNT" },
	{ "RX_PCS_PHY3_DEC_ERR_BLK_CNT" },
	{ "RX_PCS_PHY4_DEC_ERR_BLK_CNT" },
	{ "RX_PCS_PHY5_DEC_ERR_BLK_CNT" },
	{ "RX_PCS_PHY6_DEC_ERR_BLK_CNT" },
	{ "RX_PCS_PHY7_DEC_ERR_BLK_CNT" },
};

static const struct mac_reg_name rx_pcs_phy0_link_timeout_status_r[] = {
	{ "RX_PCS_PHY0_LINK_TIMEOUT_STATUS" },
	{ "RX_PCS_PHY1_LINK_TIMEOUT_STATUS" },
	{ "RX_PCS_PHY2_LINK_TIMEOUT_STATUS" },
	{ "RX_PCS_PHY3_LINK_TIMEOUT_STATUS" },
	{ "RX_PCS_PHY4_LINK_TIMEOUT_STATUS" },
	{ "RX_PCS_PHY5_LINK_TIMEOUT_STATUS" },
	{ "RX_PCS_PHY6_LINK_TIMEOUT_STATUS" },
	{ "RX_PCS_PHY7_LINK_TIMEOUT_STATUS" },
};

static const struct mac_reg_name rx_pcs_phy0_mutil_lane_status_r[] = {
	{ "RX_PCS_PHY0_MUTIL_LANE_STATUS" },
	{ "RX_PCS_PHY1_MUTIL_LANE_STATUS" },
	{ "RX_PCS_PHY2_MUTIL_LANE_STATUS" },
	{ "RX_PCS_PHY3_MUTIL_LANE_STATUS" },
	{ "RX_PCS_PHY4_MUTIL_LANE_STATUS" },
	{ "RX_PCS_PHY5_MUTIL_LANE_STATUS" },
	{ "RX_PCS_PHY6_MUTIL_LANE_STATUS" },
	{ "RX_PCS_PHY7_MUTIL_LANE_STATUS" },
};

static const struct mac_reg_name rx_pcs_phy0_dbg_64b66b_curr_status_r[] = {
	{ "RX_PCS_PHY0_DBG_64B66B_CURR_STATUS" },
	{ "RX_PCS_PHY1_DBG_64B66B_CURR_STATUS" },
	{ "RX_PCS_PHY2_DBG_64B66B_CURR_STATUS" },
	{ "RX_PCS_PHY3_DBG_64B66B_CURR_STATUS" },
	{ "RX_PCS_PHY4_DBG_64B66B_CURR_STATUS" },
	{ "RX_PCS_PHY5_DBG_64B66B_CURR_STATUS" },
	{ "RX_PCS_PHY6_DBG_64B66B_CURR_STATUS" },
	{ "RX_PCS_PHY7_DBG_64B66B_CURR_STATUS" },
};

static const struct mac_reg_name rx_pcs_phy0_dbg_curr_status_r[] = {
	{ "RX_PCS_PHY0_DBG_CURR_STATUS" },
	{ "RX_PCS_PHY1_DBG_CURR_STATUS" },
	{ "RX_PCS_PHY2_DBG_CURR_STATUS" },
	{ "RX_PCS_PHY3_DBG_CURR_STATUS" },
	{ "RX_PCS_PHY4_DBG_CURR_STATUS" },
	{ "RX_PCS_PHY5_DBG_CURR_STATUS" },
	{ "RX_PCS_PHY6_DBG_CURR_STATUS" },
	{ "RX_PCS_PHY7_DBG_CURR_STATUS" },
};

static const struct mac_reg_name rx_pcs_phy0_dbg_his_status_r[] = {
	{ "RX_PCS_PHY0_DBG_HIS_STATUS" },
	{ "RX_PCS_PHY1_DBG_HIS_STATUS" },
	{ "RX_PCS_PHY2_DBG_HIS_STATUS" },
	{ "RX_PCS_PHY3_DBG_HIS_STATUS" },
	{ "RX_PCS_PHY4_DBG_HIS_STATUS" },
	{ "RX_PCS_PHY5_DBG_HIS_STATUS" },
	{ "RX_PCS_PHY6_DBG_HIS_STATUS" },
	{ "RX_PCS_PHY7_DBG_HIS_STATUS" },
};

static const struct mac_reg_info mac_type_rx_pcs_phy_blk[] = {
	{ rx_pcs_phy0_ctrl_cfg_r,		HIKP_ARRAY_SIZE(rx_pcs_phy0_ctrl_cfg_r) },
	{ rx_pcs_phy0_link_timeout_cfg_r,	HIKP_ARRAY_SIZE(rx_pcs_phy0_link_timeout_cfg_r) },
	{ rx_pcs_phy0_ber_ctrl_cfg_r,		HIKP_ARRAY_SIZE(rx_pcs_phy0_ber_ctrl_cfg_r) },
	{ rx_pcs_phy0_err_block_cnt_r,		HIKP_ARRAY_SIZE(rx_pcs_phy0_err_block_cnt_r) },
	{ rx_pcs_phy0_e_blk_cnt_r,		HIKP_ARRAY_SIZE(rx_pcs_phy0_e_blk_cnt_r) },
	{ rx_pcs_phy0_dec_err_blk_cnt_r,	HIKP_ARRAY_SIZE(rx_pcs_phy0_dec_err_blk_cnt_r) },
	{ rx_pcs_phy0_link_timeout_status_r,	HIKP_ARRAY_SIZE(rx_pcs_phy0_link_timeout_status_r) },
	{ rx_pcs_phy0_mutil_lane_status_r,	HIKP_ARRAY_SIZE(rx_pcs_phy0_mutil_lane_status_r) },
	{ rx_pcs_phy0_dbg_64b66b_curr_status_r,	HIKP_ARRAY_SIZE(rx_pcs_phy0_dbg_64b66b_curr_status_r) },
	{ rx_pcs_phy0_dbg_curr_status_r,	HIKP_ARRAY_SIZE(rx_pcs_phy0_dbg_curr_status_r) },
	{ rx_pcs_phy0_dbg_his_status_r,		HIKP_ARRAY_SIZE(rx_pcs_phy0_dbg_his_status_r) },
};

static const struct mac_reg_name rx_pcs_phy0_lane_bip_err_cnt_r[] = {
	{ "RX_PCS_PHY0_0_LANE_BIP_ERR_CNT" },
	{ "RX_PCS_PHY0_1_LANE_BIP_ERR_CNT" },
	{ "RX_PCS_PHY0_2_LANE_BIP_ERR_CNT" },
	{ "RX_PCS_PHY0_3_LANE_BIP_ERR_CNT" },
	{ "RX_PCS_PHY0_4_LANE_BIP_ERR_CNT" },
	{ "RX_PCS_PHY0_5_LANE_BIP_ERR_CNT" },
	{ "RX_PCS_PHY0_6_LANE_BIP_ERR_CNT" },
	{ "RX_PCS_PHY0_7_LANE_BIP_ERR_CNT" },
	{ "RX_PCS_PHY0_8_LANE_BIP_ERR_CNT" },
	{ "RX_PCS_PHY0_9_LANE_BIP_ERR_CNT" },
	{ "RX_PCS_PHY0_10_LANE_BIP_ERR_CNT" },
	{ "RX_PCS_PHY0_11_LANE_BIP_ERR_CNT" },
	{ "RX_PCS_PHY0_12_LANE_BIP_ERR_CNT" },
	{ "RX_PCS_PHY0_13_LANE_BIP_ERR_CNT" },
	{ "RX_PCS_PHY0_14_LANE_BIP_ERR_CNT" },
	{ "RX_PCS_PHY0_15_LANE_BIP_ERR_CNT" },
	{ "RX_PCS_PHY0_16_LANE_BIP_ERR_CNT" },
	{ "RX_PCS_PHY0_17_LANE_BIP_ERR_CNT" },
	{ "RX_PCS_PHY0_18_LANE_BIP_ERR_CNT" },
	{ "RX_PCS_PHY0_19_LANE_BIP_ERR_CNT" },
};

static const struct mac_reg_name rx_pcs_phy0_lane_bip_err_cnt_r_offset_2[] = {
	{ "RX_PCS_PHY2_0_LANE_BIP_ERR_CNT" },
	{ "RX_PCS_PHY2_1_LANE_BIP_ERR_CNT" },
	{ "RX_PCS_PHY2_2_LANE_BIP_ERR_CNT" },
	{ "RX_PCS_PHY2_3_LANE_BIP_ERR_CNT" },
	{ "RX_PCS_PHY2_4_LANE_BIP_ERR_CNT" },
	{ "RX_PCS_PHY2_5_LANE_BIP_ERR_CNT" },
	{ "RX_PCS_PHY2_6_LANE_BIP_ERR_CNT" },
	{ "RX_PCS_PHY2_7_LANE_BIP_ERR_CNT" },
	{ "RX_PCS_PHY2_8_LANE_BIP_ERR_CNT" },
	{ "RX_PCS_PHY2_9_LANE_BIP_ERR_CNT" },
	{ "RX_PCS_PHY2_10_LANE_BIP_ERR_CNT" },
	{ "RX_PCS_PHY2_11_LANE_BIP_ERR_CNT" },
	{ "RX_PCS_PHY2_12_LANE_BIP_ERR_CNT" },
	{ "RX_PCS_PHY2_13_LANE_BIP_ERR_CNT" },
	{ "RX_PCS_PHY2_14_LANE_BIP_ERR_CNT" },
	{ "RX_PCS_PHY2_15_LANE_BIP_ERR_CNT" },
	{ "RX_PCS_PHY2_16_LANE_BIP_ERR_CNT" },
	{ "RX_PCS_PHY2_17_LANE_BIP_ERR_CNT" },
	{ "RX_PCS_PHY2_18_LANE_BIP_ERR_CNT" },
	{ "RX_PCS_PHY2_19_LANE_BIP_ERR_CNT" },
};

static const struct mac_reg_name rx_pcs_phy0_lane_bip_err_cnt_r_offset_4[] = {
	{ "RX_PCS_PHY4_0_LANE_BIP_ERR_CNT" },
	{ "RX_PCS_PHY4_1_LANE_BIP_ERR_CNT" },
	{ "RX_PCS_PHY4_2_LANE_BIP_ERR_CNT" },
	{ "RX_PCS_PHY4_3_LANE_BIP_ERR_CNT" },
	{ "RX_PCS_PHY4_4_LANE_BIP_ERR_CNT" },
	{ "RX_PCS_PHY4_5_LANE_BIP_ERR_CNT" },
	{ "RX_PCS_PHY4_6_LANE_BIP_ERR_CNT" },
	{ "RX_PCS_PHY4_7_LANE_BIP_ERR_CNT" },
	{ "RX_PCS_PHY4_8_LANE_BIP_ERR_CNT" },
	{ "RX_PCS_PHY4_9_LANE_BIP_ERR_CNT" },
	{ "RX_PCS_PHY4_10_LANE_BIP_ERR_CNT" },
	{ "RX_PCS_PHY4_11_LANE_BIP_ERR_CNT" },
	{ "RX_PCS_PHY4_12_LANE_BIP_ERR_CNT" },
	{ "RX_PCS_PHY4_13_LANE_BIP_ERR_CNT" },
	{ "RX_PCS_PHY4_14_LANE_BIP_ERR_CNT" },
	{ "RX_PCS_PHY4_15_LANE_BIP_ERR_CNT" },
	{ "RX_PCS_PHY4_16_LANE_BIP_ERR_CNT" },
	{ "RX_PCS_PHY4_17_LANE_BIP_ERR_CNT" },
	{ "RX_PCS_PHY4_18_LANE_BIP_ERR_CNT" },
	{ "RX_PCS_PHY4_19_LANE_BIP_ERR_CNT" },
};

static const struct mac_reg_name rx_pcs_phy0_lane_bip_err_cnt_r_offset_6[] = {
	{ "RX_PCS_PHY6_0_LANE_BIP_ERR_CNT" },
	{ "RX_PCS_PHY6_1_LANE_BIP_ERR_CNT" },
	{ "RX_PCS_PHY6_2_LANE_BIP_ERR_CNT" },
	{ "RX_PCS_PHY6_3_LANE_BIP_ERR_CNT" },
	{ "RX_PCS_PHY6_4_LANE_BIP_ERR_CNT" },
	{ "RX_PCS_PHY6_5_LANE_BIP_ERR_CNT" },
	{ "RX_PCS_PHY6_6_LANE_BIP_ERR_CNT" },
	{ "RX_PCS_PHY6_7_LANE_BIP_ERR_CNT" },
	{ "RX_PCS_PHY6_8_LANE_BIP_ERR_CNT" },
	{ "RX_PCS_PHY6_9_LANE_BIP_ERR_CNT" },
	{ "RX_PCS_PHY6_10_LANE_BIP_ERR_CNT" },
	{ "RX_PCS_PHY6_11_LANE_BIP_ERR_CNT" },
	{ "RX_PCS_PHY6_12_LANE_BIP_ERR_CNT" },
	{ "RX_PCS_PHY6_13_LANE_BIP_ERR_CNT" },
	{ "RX_PCS_PHY6_14_LANE_BIP_ERR_CNT" },
	{ "RX_PCS_PHY6_15_LANE_BIP_ERR_CNT" },
	{ "RX_PCS_PHY6_16_LANE_BIP_ERR_CNT" },
	{ "RX_PCS_PHY6_17_LANE_BIP_ERR_CNT" },
	{ "RX_PCS_PHY6_18_LANE_BIP_ERR_CNT" },
	{ "RX_PCS_PHY6_19_LANE_BIP_ERR_CNT" },
};

static const struct mac_reg_info mac_type_rx_pcs_blk[] = {
	{ rx_pcs_phy0_lane_bip_err_cnt_r,		HIKP_ARRAY_SIZE(rx_pcs_phy0_lane_bip_err_cnt_r) },
	{ rx_pcs_phy0_lane_bip_err_cnt_r_offset_2,	HIKP_ARRAY_SIZE(rx_pcs_phy0_lane_bip_err_cnt_r_offset_2) },
	{ rx_pcs_phy0_lane_bip_err_cnt_r_offset_4,	HIKP_ARRAY_SIZE(rx_pcs_phy0_lane_bip_err_cnt_r_offset_4) },
	{ rx_pcs_phy0_lane_bip_err_cnt_r_offset_6,	HIKP_ARRAY_SIZE(rx_pcs_phy0_lane_bip_err_cnt_r_offset_6) },
};

static const struct mac_type_name_parse g_mac_rx_pcs_name_parse[] = {
	{ mac_type_rx_pcs_reg,		HIKP_ARRAY_SIZE(mac_type_rx_pcs_reg),
		ROUND_UP(mac_type_rx_pcs_reg[0].reg_num, PER_BLK_DATA_SIZE), false},
	{ mac_type_rx_pcs_phy_blk,	HIKP_ARRAY_SIZE(mac_type_rx_pcs_phy_blk),
		HIKP_ARRAY_SIZE(mac_type_rx_pcs_phy_blk), true},
	{ mac_type_rx_pcs_blk,		HIKP_ARRAY_SIZE(mac_type_rx_pcs_blk),
		HIKP_ARRAY_SIZE(mac_type_rx_pcs_blk), true},
};

static const struct mac_reg_name rx_rsfec_int_status_r[] = {
	{ "RX_RSFEC_INT_STATUS" },
	{ "RX_RSFEC_LINK_UP_INT_STATUS" },
	{ "RX_RSFEC_LINK_DOWN_INT_STATUS" },
	{ "RX_RSFEC_TIME_OUT_INT_STATUS" },
	{ "RX_RSFEC_OVF_INT_STATUS" },
	{ "RX_RSFEC_LOC_SER_INT_STATUS" },
	{ "RX_RSFEC_RM_SER_INT_STATUS" },
	{ "RX_RSFEC_HI_BER_INT_STATUS" },
	{ "RX_RSFEC_SD_INT_STATUS" },
	{ "RX_RSFEC_SF_INT_STATUS" },
	{ "RX_RSFEC_PHY_CERR_INT_STATUS" },
	{ "RX_RSFEC_PHY_UERR_INT_STATUS" },
	{ "RX_RSFEC_DEC_DFX_STATUS" },
	{ "RX_RSFEC_PRBS_STATUS" },
};

static const struct mac_reg_info mac_type_rx_rsfec_reg[] = {
	{ rx_rsfec_int_status_r,	HIKP_ARRAY_SIZE(rx_rsfec_int_status_r) },
};

static const struct mac_reg_name rx_rsfec_phy0_control_r[] = {
	{ "RX_RSFEC_PHY0_CONTROL" },
	{ "RX_RSFEC_PHY1_CONTROL" },
	{ "RX_RSFEC_PHY2_CONTROL" },
	{ "RX_RSFEC_PHY3_CONTROL" },
	{ "RX_RSFEC_PHY4_CONTROL" },
	{ "RX_RSFEC_PHY5_CONTROL" },
	{ "RX_RSFEC_PHY6_CONTROL" },
	{ "RX_RSFEC_PHY7_CONTROL" },
};

static const struct mac_reg_name rx_rsfec_phy0_err_sta_cfg_r[] = {
	{ "RX_RSFEC_PHY0_ERR_STA_CFG" },
	{ "RX_RSFEC_PHY1_ERR_STA_CFG" },
	{ "RX_RSFEC_PHY2_ERR_STA_CFG" },
	{ "RX_RSFEC_PHY3_ERR_STA_CFG" },
	{ "RX_RSFEC_PHY4_ERR_STA_CFG" },
	{ "RX_RSFEC_PHY5_ERR_STA_CFG" },
	{ "RX_RSFEC_PHY6_ERR_STA_CFG" },
	{ "RX_RSFEC_PHY7_ERR_STA_CFG" },
};

static const struct mac_reg_name rx_rsfec_phy0_status_r[] = {
	{ "RX_RSFEC_PHY0_STATUS" },
	{ "RX_RSFEC_PHY1_STATUS" },
	{ "RX_RSFEC_PHY2_STATUS" },
	{ "RX_RSFEC_PHY3_STATUS" },
	{ "RX_RSFEC_PHY4_STATUS" },
	{ "RX_RSFEC_PHY5_STATUS" },
	{ "RX_RSFEC_PHY6_STATUS" },
	{ "RX_RSFEC_PHY7_STATUS" },
};

static const struct mac_reg_name rx_rsfec_phy0_rx_his_status_r[] = {
	{ "RX_RSFEC_PHY0_RX_HIS_STATUS" },
	{ "RX_RSFEC_PHY1_RX_HIS_STATUS" },
	{ "RX_RSFEC_PHY2_RX_HIS_STATUS" },
	{ "RX_RSFEC_PHY3_RX_HIS_STATUS" },
	{ "RX_RSFEC_PHY4_RX_HIS_STATUS" },
	{ "RX_RSFEC_PHY5_RX_HIS_STATUS" },
	{ "RX_RSFEC_PHY6_RX_HIS_STATUS" },
	{ "RX_RSFEC_PHY7_RX_HIS_STATUS" },
};

static const struct mac_reg_name rx_rsfec_phy0_idle_chk_err_cnt_r[] = {
	{ "RX_RSFEC_PHY0_IDLE_CHK_ERR_CNT" },
	{ "RX_RSFEC_PHY1_IDLE_CHK_ERR_CNT" },
	{ "RX_RSFEC_PHY2_IDLE_CHK_ERR_CNT" },
	{ "RX_RSFEC_PHY3_IDLE_CHK_ERR_CNT" },
	{ "RX_RSFEC_PHY4_IDLE_CHK_ERR_CNT" },
	{ "RX_RSFEC_PHY5_IDLE_CHK_ERR_CNT" },
	{ "RX_RSFEC_PHY6_IDLE_CHK_ERR_CNT" },
	{ "RX_RSFEC_PHY7_IDLE_CHK_ERR_CNT" },
};

static const struct mac_reg_name rx_rsfec_phy0_rx_inv_block_cnt_r[] = {
	{ "RX_RSFEC_PHY0_RX_INV_BLOCK_CNT" },
	{ "RX_RSFEC_PHY1_RX_INV_BLOCK_CNT" },
	{ "RX_RSFEC_PHY2_RX_INV_BLOCK_CNT" },
	{ "RX_RSFEC_PHY3_RX_INV_BLOCK_CNT" },
	{ "RX_RSFEC_PHY4_RX_INV_BLOCK_CNT" },
	{ "RX_RSFEC_PHY5_RX_INV_BLOCK_CNT" },
	{ "RX_RSFEC_PHY6_RX_INV_BLOCK_CNT" },
	{ "RX_RSFEC_PHY7_RX_INV_BLOCK_CNT" },
};

static const struct mac_reg_name rx_rsfec_phy0_err_bit_cnt_r[] = {
	{ "RX_RSFEC_PHY0_ERR_BIT_CNT" },
	{ "RX_RSFEC_PHY1_ERR_BIT_CNT" },
	{ "RX_RSFEC_PHY2_ERR_BIT_CNT" },
	{ "RX_RSFEC_PHY3_ERR_BIT_CNT" },
	{ "RX_RSFEC_PHY4_ERR_BIT_CNT" },
	{ "RX_RSFEC_PHY5_ERR_BIT_CNT" },
	{ "RX_RSFEC_PHY6_ERR_BIT_CNT" },
	{ "RX_RSFEC_PHY7_ERR_BIT_CNT" },
};

static const struct mac_reg_name rx_rsfec_phy0_err_sym_cnt_r[] = {
	{ "RX_RSFEC_PHY0_ERR_SYM_CNT" },
	{ "RX_RSFEC_PHY1_ERR_SYM_CNT" },
	{ "RX_RSFEC_PHY2_ERR_SYM_CNT" },
	{ "RX_RSFEC_PHY3_ERR_SYM_CNT" },
	{ "RX_RSFEC_PHY4_ERR_SYM_CNT" },
	{ "RX_RSFEC_PHY5_ERR_SYM_CNT" },
	{ "RX_RSFEC_PHY6_ERR_SYM_CNT" },
	{ "RX_RSFEC_PHY7_ERR_SYM_CNT" },
};

static const struct mac_reg_info mac_type_rx_rsfec_phy_blk[] = {
	{ rx_rsfec_phy0_control_r,		HIKP_ARRAY_SIZE(rx_rsfec_phy0_control_r) },
	{ rx_rsfec_phy0_err_sta_cfg_r,		HIKP_ARRAY_SIZE(rx_rsfec_phy0_err_sta_cfg_r) },
	{ rx_rsfec_phy0_status_r,		HIKP_ARRAY_SIZE(rx_rsfec_phy0_status_r) },
	{ rx_rsfec_phy0_rx_his_status_r,	HIKP_ARRAY_SIZE(rx_rsfec_phy0_rx_his_status_r) },
	{ rx_rsfec_phy0_idle_chk_err_cnt_r,	HIKP_ARRAY_SIZE(rx_rsfec_phy0_idle_chk_err_cnt_r) },
	{ rx_rsfec_phy0_rx_inv_block_cnt_r,	HIKP_ARRAY_SIZE(rx_rsfec_phy0_rx_inv_block_cnt_r) },
	{ rx_rsfec_phy0_err_bit_cnt_r,		HIKP_ARRAY_SIZE(rx_rsfec_phy0_err_bit_cnt_r) },
	{ rx_rsfec_phy0_err_sym_cnt_r,		HIKP_ARRAY_SIZE(rx_rsfec_phy0_err_sym_cnt_r) },
};

static const struct mac_reg_name rx_rsfec_phy0_dec_cw_cnt_r[] = {
	{ "RX_RSFEC_PHY0_DEC_CW_CNT_0" },
	{ "RX_RSFEC_PHY0_DEC_CW_CNT_1" },
};

static const struct mac_reg_name rx_rsfec_phy0_dec_cw_cnt_r_offset_2[] = {
	{ "RX_RSFEC_PHY2_DEC_CW_CNT_0" },
	{ "RX_RSFEC_PHY2_DEC_CW_CNT_1" },
};

static const struct mac_reg_name rx_rsfec_phy0_dec_cw_cnt_r_offset_4[] = {
	{ "RX_RSFEC_PHY4_DEC_CW_CNT_0" },
	{ "RX_RSFEC_PHY4_DEC_CW_CNT_1" },
};

static const struct mac_reg_name rx_rsfec_phy0_dec_cw_cnt_r_offset_6[] = {
	{ "RX_RSFEC_PHY6_DEC_CW_CNT_0" },
	{ "RX_RSFEC_PHY6_DEC_CW_CNT_1" },
};

static const struct mac_reg_name rx_rsfec_phy0_corr_lane_sym_cnt_r[] = {
	{ "RX_RSFEC_PHY0_CORR_LANE_SYM_CNT_0" },
	{ "RX_RSFEC_PHY0_CORR_LANE_SYM_CNT_1" },
	{ "RX_RSFEC_PHY0_CORR_LANE_SYM_CNT_2" },
	{ "RX_RSFEC_PHY0_CORR_LANE_SYM_CNT_3" },
	{ "RX_RSFEC_PHY0_CORR_LANE_SYM_CNT_4" },
	{ "RX_RSFEC_PHY0_CORR_LANE_SYM_CNT_5" },
	{ "RX_RSFEC_PHY0_CORR_LANE_SYM_CNT_6" },
	{ "RX_RSFEC_PHY0_CORR_LANE_SYM_CNT_7" },
	{ "RX_RSFEC_PHY0_CORR_LANE_SYM_CNT_8" },
	{ "RX_RSFEC_PHY0_CORR_LANE_SYM_CNT_9" },
	{ "RX_RSFEC_PHY0_CORR_LANE_SYM_CNT_10" },
	{ "RX_RSFEC_PHY0_CORR_LANE_SYM_CNT_11" },
	{ "RX_RSFEC_PHY0_CORR_LANE_SYM_CNT_12" },
	{ "RX_RSFEC_PHY0_CORR_LANE_SYM_CNT_13" },
	{ "RX_RSFEC_PHY0_CORR_LANE_SYM_CNT_14" },
	{ "RX_RSFEC_PHY0_CORR_LANE_SYM_CNT_15" },
};

static const struct mac_reg_name rx_rsfec_phy2_corr_lane_sym_cnt_r[] = {
	{ "RX_RSFEC_PHY2_CORR_LANE_SYM_CNT_0" },
	{ "RX_RSFEC_PHY2_CORR_LANE_SYM_CNT_1" },
	{ "RX_RSFEC_PHY2_CORR_LANE_SYM_CNT_2" },
	{ "RX_RSFEC_PHY2_CORR_LANE_SYM_CNT_3" },
	{ "RX_RSFEC_PHY2_CORR_LANE_SYM_CNT_4" },
	{ "RX_RSFEC_PHY2_CORR_LANE_SYM_CNT_5" },
	{ "RX_RSFEC_PHY2_CORR_LANE_SYM_CNT_6" },
	{ "RX_RSFEC_PHY2_CORR_LANE_SYM_CNT_7" },
};

static const struct mac_reg_name rx_rsfec_phy2_corr_lane_sym_cnt_r_offset_2[] = {
	{ "RX_RSFEC_PHY4_CORR_LANE_SYM_CNT_0" },
	{ "RX_RSFEC_PHY4_CORR_LANE_SYM_CNT_1" },
	{ "RX_RSFEC_PHY4_CORR_LANE_SYM_CNT_2" },
	{ "RX_RSFEC_PHY4_CORR_LANE_SYM_CNT_3" },
	{ "RX_RSFEC_PHY4_CORR_LANE_SYM_CNT_4" },
	{ "RX_RSFEC_PHY4_CORR_LANE_SYM_CNT_5" },
	{ "RX_RSFEC_PHY4_CORR_LANE_SYM_CNT_6" },
	{ "RX_RSFEC_PHY4_CORR_LANE_SYM_CNT_7" },
};

static const struct mac_reg_name rx_rsfec_phy2_corr_lane_sym_cnt_r_offset_4[] = {
	{ "RX_RSFEC_PHY6_CORR_LANE_SYM_CNT_0" },
	{ "RX_RSFEC_PHY6_CORR_LANE_SYM_CNT_1" },
	{ "RX_RSFEC_PHY6_CORR_LANE_SYM_CNT_2" },
	{ "RX_RSFEC_PHY6_CORR_LANE_SYM_CNT_3" },
};

static const struct mac_reg_name rx_rsfec_phy0_lane_cor0_cnt_r[] = {
	{ "RX_RSFEC_PHY0_LANE_COR0_CNT_0" },
	{ "RX_RSFEC_PHY0_LANE_COR0_CNT_1" },
	{ "RX_RSFEC_PHY0_LANE_COR0_CNT_2" },
	{ "RX_RSFEC_PHY0_LANE_COR0_CNT_3" },
	{ "RX_RSFEC_PHY0_LANE_COR0_CNT_4" },
	{ "RX_RSFEC_PHY0_LANE_COR0_CNT_5" },
	{ "RX_RSFEC_PHY0_LANE_COR0_CNT_6" },
	{ "RX_RSFEC_PHY0_LANE_COR0_CNT_7" },
	{ "RX_RSFEC_PHY0_LANE_COR0_CNT_8" },
	{ "RX_RSFEC_PHY0_LANE_COR0_CNT_9" },
	{ "RX_RSFEC_PHY0_LANE_COR0_CNT_10" },
	{ "RX_RSFEC_PHY0_LANE_COR0_CNT_11" },
	{ "RX_RSFEC_PHY0_LANE_COR0_CNT_12" },
	{ "RX_RSFEC_PHY0_LANE_COR0_CNT_13" },
	{ "RX_RSFEC_PHY0_LANE_COR0_CNT_14" },
	{ "RX_RSFEC_PHY0_LANE_COR0_CNT_15" },
};

static const struct mac_reg_name rx_rsfec_phy2_lane_cor0_cnt_r[] = {
	{ "RX_RSFEC_PHY2_LANE_COR0_CNT_0" },
	{ "RX_RSFEC_PHY2_LANE_COR0_CNT_1" },
	{ "RX_RSFEC_PHY2_LANE_COR0_CNT_2" },
	{ "RX_RSFEC_PHY2_LANE_COR0_CNT_3" },
	{ "RX_RSFEC_PHY2_LANE_COR0_CNT_4" },
	{ "RX_RSFEC_PHY2_LANE_COR0_CNT_5" },
	{ "RX_RSFEC_PHY2_LANE_COR0_CNT_6" },
	{ "RX_RSFEC_PHY2_LANE_COR0_CNT_7" },
};

static const struct mac_reg_name rx_rsfec_phy2_lane_cor0_cnt_r_offset_2[] = {
	{ "RX_RSFEC_PHY4_LANE_COR0_CNT_0" },
	{ "RX_RSFEC_PHY4_LANE_COR0_CNT_1" },
	{ "RX_RSFEC_PHY4_LANE_COR0_CNT_2" },
	{ "RX_RSFEC_PHY4_LANE_COR0_CNT_3" },
	{ "RX_RSFEC_PHY4_LANE_COR0_CNT_4" },
	{ "RX_RSFEC_PHY4_LANE_COR0_CNT_5" },
	{ "RX_RSFEC_PHY4_LANE_COR0_CNT_6" },
	{ "RX_RSFEC_PHY4_LANE_COR0_CNT_7" },
};

static const struct mac_reg_name rx_rsfec_phy6_lane_cor0_cnt_r[] = {
	{ "RX_RSFEC_PHY6_LANE_COR0_CNT_0" },
	{ "RX_RSFEC_PHY6_LANE_COR0_CNT_1" },
	{ "RX_RSFEC_PHY6_LANE_COR0_CNT_2" },
	{ "RX_RSFEC_PHY6_LANE_COR0_CNT_3" },
};

static const struct mac_reg_name rx_rsfec_phy0_lane_cor1_cnt_r[] = {
	{ "RX_RSFEC_PHY0_LANE_COR1_CNT_0" },
	{ "RX_RSFEC_PHY0_LANE_COR1_CNT_1" },
	{ "RX_RSFEC_PHY0_LANE_COR1_CNT_2" },
	{ "RX_RSFEC_PHY0_LANE_COR1_CNT_3" },
	{ "RX_RSFEC_PHY0_LANE_COR1_CNT_4" },
	{ "RX_RSFEC_PHY0_LANE_COR1_CNT_5" },
	{ "RX_RSFEC_PHY0_LANE_COR1_CNT_6" },
	{ "RX_RSFEC_PHY0_LANE_COR1_CNT_7" },
	{ "RX_RSFEC_PHY0_LANE_COR1_CNT_8" },
	{ "RX_RSFEC_PHY0_LANE_COR1_CNT_9" },
	{ "RX_RSFEC_PHY0_LANE_COR1_CNT_10" },
	{ "RX_RSFEC_PHY0_LANE_COR1_CNT_11" },
	{ "RX_RSFEC_PHY0_LANE_COR1_CNT_12" },
	{ "RX_RSFEC_PHY0_LANE_COR1_CNT_13" },
	{ "RX_RSFEC_PHY0_LANE_COR1_CNT_14" },
	{ "RX_RSFEC_PHY0_LANE_COR1_CNT_15" },
};

static const struct mac_reg_name rx_rsfec_phy2_lane_cor1_cnt_r[] = {
	{ "RX_RSFEC_PHY2_LANE_COR1_CNT_0" },
	{ "RX_RSFEC_PHY2_LANE_COR1_CNT_1" },
	{ "RX_RSFEC_PHY2_LANE_COR1_CNT_2" },
	{ "RX_RSFEC_PHY2_LANE_COR1_CNT_3" },
	{ "RX_RSFEC_PHY2_LANE_COR1_CNT_4" },
	{ "RX_RSFEC_PHY2_LANE_COR1_CNT_5" },
	{ "RX_RSFEC_PHY2_LANE_COR1_CNT_6" },
	{ "RX_RSFEC_PHY2_LANE_COR1_CNT_7" },
};

static const struct mac_reg_name rx_rsfec_phy2_lane_cor1_cnt_r_offset_2[] = {
	{ "RX_RSFEC_PHY4_LANE_COR1_CNT_0" },
	{ "RX_RSFEC_PHY4_LANE_COR1_CNT_1" },
	{ "RX_RSFEC_PHY4_LANE_COR1_CNT_2" },
	{ "RX_RSFEC_PHY4_LANE_COR1_CNT_3" },
	{ "RX_RSFEC_PHY4_LANE_COR1_CNT_4" },
	{ "RX_RSFEC_PHY4_LANE_COR1_CNT_5" },
	{ "RX_RSFEC_PHY4_LANE_COR1_CNT_6" },
	{ "RX_RSFEC_PHY4_LANE_COR1_CNT_7" },
};

static const struct mac_reg_name rx_rsfec_phy6_lane_cor1_cnt_r[] = {
	{ "RX_RSFEC_PHY6_LANE_COR1_CNT_0" },
	{ "RX_RSFEC_PHY6_LANE_COR1_CNT_1" },
	{ "RX_RSFEC_PHY6_LANE_COR1_CNT_2" },
	{ "RX_RSFEC_PHY6_LANE_COR1_CNT_3" },
};

static const struct mac_reg_info mac_type_rx_rsfec_blk[] = {
	{ rx_rsfec_phy0_dec_cw_cnt_r,			HIKP_ARRAY_SIZE(rx_rsfec_phy0_dec_cw_cnt_r) },
	{ rx_rsfec_phy0_dec_cw_cnt_r_offset_2,		HIKP_ARRAY_SIZE(rx_rsfec_phy0_dec_cw_cnt_r_offset_2) },
	{ rx_rsfec_phy0_dec_cw_cnt_r_offset_4,		HIKP_ARRAY_SIZE(rx_rsfec_phy0_dec_cw_cnt_r_offset_4) },
	{ rx_rsfec_phy0_dec_cw_cnt_r_offset_6,		HIKP_ARRAY_SIZE(rx_rsfec_phy0_dec_cw_cnt_r_offset_6) },
	{ rx_rsfec_phy0_corr_lane_sym_cnt_r,		HIKP_ARRAY_SIZE(rx_rsfec_phy0_corr_lane_sym_cnt_r) },
	{ rx_rsfec_phy2_corr_lane_sym_cnt_r,		HIKP_ARRAY_SIZE(rx_rsfec_phy2_corr_lane_sym_cnt_r) },
	{ rx_rsfec_phy2_corr_lane_sym_cnt_r_offset_2,	HIKP_ARRAY_SIZE(rx_rsfec_phy2_corr_lane_sym_cnt_r_offset_2) },
	{ rx_rsfec_phy2_corr_lane_sym_cnt_r_offset_4,	HIKP_ARRAY_SIZE(rx_rsfec_phy2_corr_lane_sym_cnt_r_offset_4) },
	{ rx_rsfec_phy0_lane_cor0_cnt_r,		HIKP_ARRAY_SIZE(rx_rsfec_phy0_lane_cor0_cnt_r) },
	{ rx_rsfec_phy2_lane_cor0_cnt_r,		HIKP_ARRAY_SIZE(rx_rsfec_phy2_lane_cor0_cnt_r) },
	{ rx_rsfec_phy2_lane_cor0_cnt_r_offset_2,	HIKP_ARRAY_SIZE(rx_rsfec_phy2_lane_cor0_cnt_r_offset_2) },
	{ rx_rsfec_phy6_lane_cor0_cnt_r,		HIKP_ARRAY_SIZE(rx_rsfec_phy6_lane_cor0_cnt_r) },
	{ rx_rsfec_phy0_lane_cor1_cnt_r,		HIKP_ARRAY_SIZE(rx_rsfec_phy0_lane_cor1_cnt_r) },
	{ rx_rsfec_phy2_lane_cor1_cnt_r,		HIKP_ARRAY_SIZE(rx_rsfec_phy2_lane_cor1_cnt_r) },
	{ rx_rsfec_phy2_lane_cor1_cnt_r_offset_2,	HIKP_ARRAY_SIZE(rx_rsfec_phy2_lane_cor1_cnt_r_offset_2) },
	{ rx_rsfec_phy6_lane_cor1_cnt_r,		HIKP_ARRAY_SIZE(rx_rsfec_phy6_lane_cor1_cnt_r) },
};

static const struct mac_reg_name rx_rsfec_cw_dec_all_cnt_r[] = {
	{ "RX_RSFEC_CW_DEC_ALL_CNT" },
	{ "RX_RSFEC_CW_1SYM_ERR_CNT" },
	{ "RX_RSFEC_CW_2SYM_ERR_CNT" },
	{ "RX_RSFEC_CW_3SYM_ERR_CNT" },
	{ "RX_RSFEC_CW_4SYM_ERR_CNT" },
	{ "RX_RSFEC_CW_5SYM_ERR_CNT" },
	{ "RX_RSFEC_CW_6SYM_ERR_CNT" },
	{ "RX_RSFEC_CW_7SYM_ERR_CNT" },
	{ "RX_RSFEC_CW_8SYM_ERR_CNT" },
	{ "RX_RSFEC_CW_9SYM_ERR_CNT" },
	{ "RX_RSFEC_CW_10SYM_ERR_CNT" },
	{ "RX_RSFEC_CW_11SYM_ERR_CNT" },
	{ "RX_RSFEC_CW_12SYM_ERR_CNT" },
	{ "RX_RSFEC_CW_13SYM_ERR_CNT" },
	{ "RX_RSFEC_CW_14SYM_ERR_CNT" },
	{ "RX_RSFEC_CW_15SYM_ERR_CNT" },
	{ "RX_RSFEC_CW_FAIL_ERR_CNT" },
	{ "RX_RSFEC_CW_1SYM_ERR_BUR_CNT" },
	{ "RX_RSFEC_CW_2SYM_ERR_BUR_CNT" },
	{ "RX_RSFEC_CW_3SYM_ERR_BUR_CNT" },
	{ "RX_RSFEC_CW_4SYM_ERR_BUR_CNT" },
	{ "RX_RSFEC_CW_5SYM_ERR_BUR_CNT" },
	{ "RX_RSFEC_CW_6SYM_ERR_BUR_CNT" },
	{ "RX_RSFEC_CW_7SYM_ERR_BUR_CNT" },
	{ "RX_RSFEC_CW_8SYM_ERR_BUR_CNT" },
	{ "RX_RSFEC_CW_9SYM_ERR_BUR_CNT" },
	{ "RX_RSFEC_CW_10SYM_ERR_BUR_CNT" },
	{ "RX_RSFEC_CW_11SYM_ERR_BUR_CNT" },
	{ "RX_RSFEC_CW_12SYM_ERR_BUR_CNT" },
	{ "RX_RSFEC_CW_13SYM_ERR_BUR_CNT" },
};

static const struct mac_reg_name rx_rsfec_cw_14sym_err_bur_cnt_r[] = {
	{ "RX_RSFEC_CW_14SYM_ERR_BUR_CNT" },
	{ "RX_RSFEC_CW_15SYM_ERR_BUR_CNT" },
};

static const struct mac_reg_info mac_type_rx_rsfec_64_reg[] = {
	{ rx_rsfec_cw_dec_all_cnt_r,		HIKP_ARRAY_SIZE(rx_rsfec_cw_dec_all_cnt_r) },
	{ rx_rsfec_cw_14sym_err_bur_cnt_r,	HIKP_ARRAY_SIZE(rx_rsfec_cw_14sym_err_bur_cnt_r) },
};

static const struct mac_type_name_parse g_mac_rx_rsfec_name_parse[] = {
	{ mac_type_rx_rsfec_reg,		HIKP_ARRAY_SIZE(mac_type_rx_rsfec_reg),
		ROUND_UP(mac_type_rx_rsfec_reg[0].reg_num, PER_BLK_DATA_SIZE), false},
	{ mac_type_rx_rsfec_phy_blk,		HIKP_ARRAY_SIZE(mac_type_rx_rsfec_phy_blk),
		HIKP_ARRAY_SIZE(mac_type_rx_rsfec_phy_blk), true},
	{ mac_type_rx_rsfec_blk,		HIKP_ARRAY_SIZE(mac_type_rx_rsfec_blk),
		HIKP_ARRAY_SIZE(mac_type_rx_rsfec_blk), true},
	{ mac_type_rx_rsfec_64_reg,		HIKP_ARRAY_SIZE(mac_type_rx_rsfec_64_reg),
		HIKP_ARRAY_SIZE(mac_type_rx_rsfec_64_reg),  true},
};

static const struct mac_reg_name rx_brfec_int_status_r[] = {
	{ "RX_BRFEC_INT_STATUS" },
	{ "RX_BRFEC_OVF_INT_STATUS" },
	{ "RX_BRFEC_PHY_CERR_INT_STATUS" },
	{ "RX_BRFEC_PHY_UERR_INT_STATUS" },
};

static const struct mac_reg_info mac_type_rx_brfec_reg[] = {
	{ rx_brfec_int_status_r,		HIKP_ARRAY_SIZE(rx_brfec_int_status_r) },
};

static const struct mac_reg_name rx_brfec_phy0_control_r[] = {
	{ "RX_BRFEC_PHY0_CONTROL" },
	{ "RX_BRFEC_PHY1_CONTROL" },
	{ "RX_BRFEC_PHY2_CONTROL" },
	{ "RX_BRFEC_PHY3_CONTROL" },
	{ "RX_BRFEC_PHY4_CONTROL" },
	{ "RX_BRFEC_PHY5_CONTROL" },
	{ "RX_BRFEC_PHY6_CONTROL" },
	{ "RX_BRFEC_PHY7_CONTROL" },
};

static const struct mac_reg_info mac_type_rx_brfec_phy_blk[] = {
	{ rx_brfec_phy0_control_r,		HIKP_ARRAY_SIZE(rx_brfec_phy0_control_r) },
};

static const struct mac_reg_name rx_brfec_phy0_diag_fec_vl_state_r[] = {
	{ "RX_BRFEC_PHY0_DIAG_FEC_VL_STATE_0" },
	{ "RX_BRFEC_PHY0_DIAG_FEC_VL_STATE_1" },
	{ "RX_BRFEC_PHY0_DIAG_FEC_VL_STATE_2" },
	{ "RX_BRFEC_PHY0_DIAG_FEC_VL_STATE_3" },
	{ "RX_BRFEC_PHY0_DIAG_FEC_VL_HIS_STATE_0" },
	{ "RX_BRFEC_PHY0_DIAG_FEC_VL_HIS_STATE_1" },
	{ "RX_BRFEC_PHY0_DIAG_FEC_VL_HIS_STATE_2" },
	{ "RX_BRFEC_PHY0_DIAG_FEC_VL_HIS_STATE_3" },
};

static const struct mac_reg_name rx_brfec_phy0_diag_fec_vl_state_r_offset_2[] = {
	{ "RX_BRFEC_PHY2_DIAG_FEC_VL_STATE_0" },
	{ "RX_BRFEC_PHY2_DIAG_FEC_VL_STATE_1" },
	{ "RX_BRFEC_PHY2_DIAG_FEC_VL_STATE_2" },
	{ "RX_BRFEC_PHY2_DIAG_FEC_VL_STATE_3" },
	{ "RX_BRFEC_PHY2_DIAG_FEC_VL_HIS_STATE_0" },
	{ "RX_BRFEC_PHY2_DIAG_FEC_VL_HIS_STATE_1" },
	{ "RX_BRFEC_PHY2_DIAG_FEC_VL_HIS_STATE_2" },
	{ "RX_BRFEC_PHY2_DIAG_FEC_VL_HIS_STATE_3" },
};

static const struct mac_reg_name rx_brfec_phy0_diag_fec_vl_state_r_offset_4[] = {
	{ "RX_BRFEC_PHY4_DIAG_FEC_VL_STATE_0" },
	{ "RX_BRFEC_PHY4_DIAG_FEC_VL_STATE_1" },
	{ "RX_BRFEC_PHY4_DIAG_FEC_VL_STATE_2" },
	{ "RX_BRFEC_PHY4_DIAG_FEC_VL_STATE_3" },
	{ "RX_BRFEC_PHY4_DIAG_FEC_VL_HIS_STATE_0" },
	{ "RX_BRFEC_PHY4_DIAG_FEC_VL_HIS_STATE_1" },
	{ "RX_BRFEC_PHY4_DIAG_FEC_VL_HIS_STATE_2" },
	{ "RX_BRFEC_PHY4_DIAG_FEC_VL_HIS_STATE_3" },
};

static const struct mac_reg_name rx_brfec_phy0_diag_fec_vl_state_r_offset_6[] = {
	{ "RX_BRFEC_PHY6_DIAG_FEC_VL_STATE_0" },
	{ "RX_BRFEC_PHY6_DIAG_FEC_VL_STATE_1" },
	{ "RX_BRFEC_PHY6_DIAG_FEC_VL_STATE_2" },
	{ "RX_BRFEC_PHY6_DIAG_FEC_VL_STATE_3" },
	{ "RX_BRFEC_PHY6_DIAG_FEC_VL_HIS_STATE_0" },
	{ "RX_BRFEC_PHY6_DIAG_FEC_VL_HIS_STATE_1" },
	{ "RX_BRFEC_PHY6_DIAG_FEC_VL_HIS_STATE_2" },
	{ "RX_BRFEC_PHY6_DIAG_FEC_VL_HIS_STATE_3" },
};

static const struct mac_reg_info mac_type_rx_brfec_blk[] = {
	{ rx_brfec_phy0_diag_fec_vl_state_r,		HIKP_ARRAY_SIZE(rx_brfec_phy0_diag_fec_vl_state_r) },
	{ rx_brfec_phy0_diag_fec_vl_state_r_offset_2,	HIKP_ARRAY_SIZE(rx_brfec_phy0_diag_fec_vl_state_r_offset_2) },
	{ rx_brfec_phy0_diag_fec_vl_state_r_offset_4,	HIKP_ARRAY_SIZE(rx_brfec_phy0_diag_fec_vl_state_r_offset_4) },
	{ rx_brfec_phy0_diag_fec_vl_state_r_offset_6,	HIKP_ARRAY_SIZE(rx_brfec_phy0_diag_fec_vl_state_r_offset_6) },
};

static const struct mac_type_name_parse g_mac_rx_brfec_name_parse[] = {
	{ mac_type_rx_brfec_reg,		HIKP_ARRAY_SIZE(mac_type_rx_brfec_reg),
		ROUND_UP(mac_type_rx_brfec_reg[0].reg_num, PER_BLK_DATA_SIZE), false },
	{ mac_type_rx_brfec_phy_blk,		HIKP_ARRAY_SIZE(mac_type_rx_brfec_phy_blk),
		HIKP_ARRAY_SIZE(mac_type_rx_brfec_phy_blk), true },
	{ mac_type_rx_brfec_blk,		HIKP_ARRAY_SIZE(mac_type_rx_brfec_blk),
		HIKP_ARRAY_SIZE(mac_type_rx_brfec_blk), true },
};

static const struct mac_reg_name rxpma_core_int_status_r[] = {
	{ "RXPMA_CORE_INT_STATUS" },
	{ "RXPMA_CORE_OVF_INT_STATUS" },
	{ "RXPMA_CORE_UDF_INT_STATUS" },
	{ "RXPMA_CORE_CALEN_DEPTH" },
	{ "RXPMA_CORE_CALEN_SEL" },
	{ "RXPMA_CORE_PTP_CLK_FREQ_SEL" },
	{ "RXPMA_CORE_RTC_PERIOD_MAC_CORE_CLK" },
	{ "RXPMA_CORE_DBG_IERR_INSERT" },
};

static const struct mac_reg_info mac_type_rxpma_core_reg[] = {
	{ rxpma_core_int_status_r,		HIKP_ARRAY_SIZE(rxpma_core_int_status_r) },
};

static const struct mac_reg_name rxpma_core_phy0_control_r[] = {
	{ "RXPMA_CORE_PHY0_CONTROL" },
	{ "RXPMA_CORE_PHY1_CONTROL" },
	{ "RXPMA_CORE_PHY2_CONTROL" },
	{ "RXPMA_CORE_PHY3_CONTROL" },
	{ "RXPMA_CORE_PHY4_CONTROL" },
	{ "RXPMA_CORE_PHY5_CONTROL" },
	{ "RXPMA_CORE_PHY6_CONTROL" },
	{ "RXPMA_CORE_PHY7_CONTROL" },
};

static const struct mac_reg_name rxpma_core_phy0_amwin_err_cfg_r[] = {
	{ "RXPMA_CORE_PHY0_AMWIN_ERR_CFG" },
	{ "RXPMA_CORE_PHY1_AMWIN_ERR_CFG" },
	{ "RXPMA_CORE_PHY2_AMWIN_ERR_CFG" },
	{ "RXPMA_CORE_PHY3_AMWIN_ERR_CFG" },
	{ "RXPMA_CORE_PHY4_AMWIN_ERR_CFG" },
	{ "RXPMA_CORE_PHY5_AMWIN_ERR_CFG" },
	{ "RXPMA_CORE_PHY6_AMWIN_ERR_CFG" },
	{ "RXPMA_CORE_PHY7_AMWIN_ERR_CFG" },
};

static const struct mac_reg_name rxpma_core_phy0_rx_1588_cfg_r[] = {
	{ "RXPMA_CORE_PHY0_RX_1588_CFG" },
	{ "RXPMA_CORE_PHY1_RX_1588_CFG" },
	{ "RXPMA_CORE_PHY2_RX_1588_CFG" },
	{ "RXPMA_CORE_PHY3_RX_1588_CFG" },
	{ "RXPMA_CORE_PHY4_RX_1588_CFG" },
	{ "RXPMA_CORE_PHY5_RX_1588_CFG" },
	{ "RXPMA_CORE_PHY6_RX_1588_CFG" },
	{ "RXPMA_CORE_PHY7_RX_1588_CFG" },
};

static const struct mac_reg_name rxpma_core_phy0_rx_ts_dly_r[] = {
	{ "RXPMA_CORE_PHY0_RX_TS_DLY" },
	{ "RXPMA_CORE_PHY1_RX_TS_DLY" },
	{ "RXPMA_CORE_PHY2_RX_TS_DLY" },
	{ "RXPMA_CORE_PHY3_RX_TS_DLY" },
	{ "RXPMA_CORE_PHY4_RX_TS_DLY" },
	{ "RXPMA_CORE_PHY5_RX_TS_DLY" },
	{ "RXPMA_CORE_PHY6_RX_TS_DLY" },
	{ "RXPMA_CORE_PHY7_RX_TS_DLY" },
};

static const struct mac_reg_name rxpma_core_phy0_iso_status_r[] = {
	{ "RXPMA_CORE_PHY0_ISO_STATUS" },
	{ "RXPMA_CORE_PHY1_ISO_STATUS" },
	{ "RXPMA_CORE_PHY2_ISO_STATUS" },
	{ "RXPMA_CORE_PHY3_ISO_STATUS" },
	{ "RXPMA_CORE_PHY4_ISO_STATUS" },
	{ "RXPMA_CORE_PHY5_ISO_STATUS" },
	{ "RXPMA_CORE_PHY6_ISO_STATUS" },
	{ "RXPMA_CORE_PHY7_ISO_STATUS" },
};

static const struct mac_reg_info mac_type_rxpma_core_phy_blk[] = {
	{ rxpma_core_phy0_control_r,		HIKP_ARRAY_SIZE(rxpma_core_phy0_control_r) },
	{ rxpma_core_phy0_amwin_err_cfg_r,	HIKP_ARRAY_SIZE(rxpma_core_phy0_amwin_err_cfg_r) },
	{ rxpma_core_phy0_rx_1588_cfg_r,	HIKP_ARRAY_SIZE(rxpma_core_phy0_rx_1588_cfg_r) },
	{ rxpma_core_phy0_rx_ts_dly_r,		HIKP_ARRAY_SIZE(rxpma_core_phy0_rx_ts_dly_r) },
	{ rxpma_core_phy0_iso_status_r,		HIKP_ARRAY_SIZE(rxpma_core_phy0_iso_status_r) },
};

static const struct mac_reg_name rxpma_core_calendar_table_r[] = {
	{ "RXPMA_CORE_CALENDAR0_TABLE" },
	{ "RXPMA_CORE_CALENDAR1_TABLE" },
	{ "RXPMA_CORE_CALENDAR2_TABLE" },
	{ "RXPMA_CORE_CALENDAR3_TABLE" },
	{ "RXPMA_CORE_CALENDAR4_TABLE" },
	{ "RXPMA_CORE_CALENDAR5_TABLE" },
	{ "RXPMA_CORE_CALENDAR6_TABLE" },
	{ "RXPMA_CORE_CALENDAR7_TABLE" },
	{ "RXPMA_CORE_CALENDAR8_TABLE" },
	{ "RXPMA_CORE_CALENDAR9_TABLE" },
	{ "RXPMA_CORE_CALENDAR10_TABLE" },
	{ "RXPMA_CORE_CALENDAR11_TABLE" },
	{ "RXPMA_CORE_CALENDAR12_TABLE" },
	{ "RXPMA_CORE_CALENDAR13_TABLE" },
	{ "RXPMA_CORE_CALENDAR14_TABLE" },
	{ "RXPMA_CORE_CALENDAR15_TABLE" },
};

static const struct mac_reg_name rxpma_core_phy_amwinsize_r[] = {
	{ "RXPMA_CORE_PHY0_AMWINSIZE" },
	{ "RXPMA_CORE_PHY1_AMWINSIZE" },
	{ "RXPMA_CORE_PHY2_AMWINSIZE" },
	{ "RXPMA_CORE_PHY3_AMWINSIZE" },
	{ "RXPMA_CORE_PHY4_AMWINSIZE" },
	{ "RXPMA_CORE_PHY5_AMWINSIZE" },
	{ "RXPMA_CORE_PHY6_AMWINSIZE" },
	{ "RXPMA_CORE_PHY7_AMWINSIZE" },
	{ "RXPMA_CORE_PHY0_MODE" },
	{ "RXPMA_CORE_PHY1_MODE" },
	{ "RXPMA_CORE_PHY2_MODE" },
	{ "RXPMA_CORE_PHY3_MODE" },
	{ "RXPMA_CORE_PHY4_MODE" },
	{ "RXPMA_CORE_PHY5_MODE" },
	{ "RXPMA_CORE_PHY6_MODE" },
	{ "RXPMA_CORE_PHY7_MODE" },
};

static const struct mac_reg_name rxpma_core_phy0_lane_curr_status_r[] = {
	{ "RXPMA_CORE_PHY0_0_LANE_CURR_STATUS" },
	{ "RXPMA_CORE_PHY0_1_LANE_CURR_STATUS" },
	{ "RXPMA_CORE_PHY0_2_LANE_CURR_STATUS" },
	{ "RXPMA_CORE_PHY0_3_LANE_CURR_STATUS" },
	{ "RXPMA_CORE_PHY0_4_LANE_CURR_STATUS" },
	{ "RXPMA_CORE_PHY0_5_LANE_CURR_STATUS" },
	{ "RXPMA_CORE_PHY0_6_LANE_CURR_STATUS" },
	{ "RXPMA_CORE_PHY0_7_LANE_CURR_STATUS" },
	{ "RXPMA_CORE_PHY0_8_LANE_CURR_STATUS" },
	{ "RXPMA_CORE_PHY0_9_LANE_CURR_STATUS" },
	{ "RXPMA_CORE_PHY0_10_LANE_CURR_STATUS" },
	{ "RXPMA_CORE_PHY0_11_LANE_CURR_STATUS" },
	{ "RXPMA_CORE_PHY0_12_LANE_CURR_STATUS" },
	{ "RXPMA_CORE_PHY0_13_LANE_CURR_STATUS" },
	{ "RXPMA_CORE_PHY0_14_LANE_CURR_STATUS" },
	{ "RXPMA_CORE_PHY0_15_LANE_CURR_STATUS" },
	{ "RXPMA_CORE_PHY0_16_LANE_CURR_STATUS" },
	{ "RXPMA_CORE_PHY0_17_LANE_CURR_STATUS" },
	{ "RXPMA_CORE_PHY0_18_LANE_CURR_STATUS" },
	{ "RXPMA_CORE_PHY0_19_LANE_CURR_STATUS" },
};

static const struct mac_reg_name rxpma_core_phy0_lane_curr_status_r_offset_2[] = {
	{ "RXPMA_CORE_PHY2_0_LANE_CURR_STATUS" },
	{ "RXPMA_CORE_PHY2_1_LANE_CURR_STATUS" },
	{ "RXPMA_CORE_PHY2_2_LANE_CURR_STATUS" },
	{ "RXPMA_CORE_PHY2_3_LANE_CURR_STATUS" },
	{ "RXPMA_CORE_PHY2_4_LANE_CURR_STATUS" },
	{ "RXPMA_CORE_PHY2_5_LANE_CURR_STATUS" },
	{ "RXPMA_CORE_PHY2_6_LANE_CURR_STATUS" },
	{ "RXPMA_CORE_PHY2_7_LANE_CURR_STATUS" },
	{ "RXPMA_CORE_PHY2_8_LANE_CURR_STATUS" },
	{ "RXPMA_CORE_PHY2_9_LANE_CURR_STATUS" },
	{ "RXPMA_CORE_PHY2_10_LANE_CURR_STATUS" },
	{ "RXPMA_CORE_PHY2_11_LANE_CURR_STATUS" },
	{ "RXPMA_CORE_PHY2_12_LANE_CURR_STATUS" },
	{ "RXPMA_CORE_PHY2_13_LANE_CURR_STATUS" },
	{ "RXPMA_CORE_PHY2_14_LANE_CURR_STATUS" },
	{ "RXPMA_CORE_PHY2_15_LANE_CURR_STATUS" },
	{ "RXPMA_CORE_PHY2_16_LANE_CURR_STATUS" },
	{ "RXPMA_CORE_PHY2_17_LANE_CURR_STATUS" },
	{ "RXPMA_CORE_PHY2_18_LANE_CURR_STATUS" },
	{ "RXPMA_CORE_PHY2_19_LANE_CURR_STATUS" },
};

static const struct mac_reg_name rxpma_core_phy0_lane_curr_status_r_offset_4[] = {
	{ "RXPMA_CORE_PHY4_0_LANE_CURR_STATUS" },
	{ "RXPMA_CORE_PHY4_1_LANE_CURR_STATUS" },
	{ "RXPMA_CORE_PHY4_2_LANE_CURR_STATUS" },
	{ "RXPMA_CORE_PHY4_3_LANE_CURR_STATUS" },
	{ "RXPMA_CORE_PHY4_4_LANE_CURR_STATUS" },
	{ "RXPMA_CORE_PHY4_5_LANE_CURR_STATUS" },
	{ "RXPMA_CORE_PHY4_6_LANE_CURR_STATUS" },
	{ "RXPMA_CORE_PHY4_7_LANE_CURR_STATUS" },
	{ "RXPMA_CORE_PHY4_8_LANE_CURR_STATUS" },
	{ "RXPMA_CORE_PHY4_9_LANE_CURR_STATUS" },
	{ "RXPMA_CORE_PHY4_10_LANE_CURR_STATUS" },
	{ "RXPMA_CORE_PHY4_11_LANE_CURR_STATUS" },
	{ "RXPMA_CORE_PHY4_12_LANE_CURR_STATUS" },
	{ "RXPMA_CORE_PHY4_13_LANE_CURR_STATUS" },
	{ "RXPMA_CORE_PHY4_14_LANE_CURR_STATUS" },
	{ "RXPMA_CORE_PHY4_15_LANE_CURR_STATUS" },
	{ "RXPMA_CORE_PHY4_16_LANE_CURR_STATUS" },
	{ "RXPMA_CORE_PHY4_17_LANE_CURR_STATUS" },
	{ "RXPMA_CORE_PHY4_18_LANE_CURR_STATUS" },
	{ "RXPMA_CORE_PHY4_19_LANE_CURR_STATUS" },
};

static const struct mac_reg_name rxpma_core_phy0_lane_curr_status_r_offset_6[] = {
	{ "RXPMA_CORE_PHY6_0_LANE_CURR_STATUS" },
	{ "RXPMA_CORE_PHY6_1_LANE_CURR_STATUS" },
	{ "RXPMA_CORE_PHY6_2_LANE_CURR_STATUS" },
	{ "RXPMA_CORE_PHY6_3_LANE_CURR_STATUS" },
	{ "RXPMA_CORE_PHY6_4_LANE_CURR_STATUS" },
	{ "RXPMA_CORE_PHY6_5_LANE_CURR_STATUS" },
	{ "RXPMA_CORE_PHY6_6_LANE_CURR_STATUS" },
	{ "RXPMA_CORE_PHY6_7_LANE_CURR_STATUS" },
	{ "RXPMA_CORE_PHY6_8_LANE_CURR_STATUS" },
	{ "RXPMA_CORE_PHY6_9_LANE_CURR_STATUS" },
	{ "RXPMA_CORE_PHY6_10_LANE_CURR_STATUS" },
	{ "RXPMA_CORE_PHY6_11_LANE_CURR_STATUS" },
	{ "RXPMA_CORE_PHY6_12_LANE_CURR_STATUS" },
	{ "RXPMA_CORE_PHY6_13_LANE_CURR_STATUS" },
	{ "RXPMA_CORE_PHY6_14_LANE_CURR_STATUS" },
	{ "RXPMA_CORE_PHY6_15_LANE_CURR_STATUS" },
	{ "RXPMA_CORE_PHY6_16_LANE_CURR_STATUS" },
	{ "RXPMA_CORE_PHY6_17_LANE_CURR_STATUS" },
	{ "RXPMA_CORE_PHY6_18_LANE_CURR_STATUS" },
	{ "RXPMA_CORE_PHY6_19_LANE_CURR_STATUS" },
};

static const struct mac_reg_name rxpma_core_phy0_lane_his_status_r[] = {
	{ "RXPMA_CORE_PHY0_0_LANE_HIS_STATUS" },
	{ "RXPMA_CORE_PHY0_1_LANE_HIS_STATUS" },
	{ "RXPMA_CORE_PHY0_2_LANE_HIS_STATUS" },
	{ "RXPMA_CORE_PHY0_3_LANE_HIS_STATUS" },
	{ "RXPMA_CORE_PHY0_4_LANE_HIS_STATUS" },
	{ "RXPMA_CORE_PHY0_5_LANE_HIS_STATUS" },
	{ "RXPMA_CORE_PHY0_6_LANE_HIS_STATUS" },
	{ "RXPMA_CORE_PHY0_7_LANE_HIS_STATUS" },
	{ "RXPMA_CORE_PHY0_8_LANE_HIS_STATUS" },
	{ "RXPMA_CORE_PHY0_9_LANE_HIS_STATUS" },
	{ "RXPMA_CORE_PHY0_10_LANE_HIS_STATUS" },
	{ "RXPMA_CORE_PHY0_11_LANE_HIS_STATUS" },
	{ "RXPMA_CORE_PHY0_12_LANE_HIS_STATUS" },
	{ "RXPMA_CORE_PHY0_13_LANE_HIS_STATUS" },
	{ "RXPMA_CORE_PHY0_14_LANE_HIS_STATUS" },
	{ "RXPMA_CORE_PHY0_15_LANE_HIS_STATUS" },
	{ "RXPMA_CORE_PHY0_16_LANE_HIS_STATUS" },
	{ "RXPMA_CORE_PHY0_17_LANE_HIS_STATUS" },
	{ "RXPMA_CORE_PHY0_18_LANE_HIS_STATUS" },
	{ "RXPMA_CORE_PHY0_19_LANE_HIS_STATUS" },
};

static const struct mac_reg_name rxpma_core_phy0_lane_his_status_r_offset_2[] = {
	{ "RXPMA_CORE_PHY2_0_LANE_HIS_STATUS" },
	{ "RXPMA_CORE_PHY2_1_LANE_HIS_STATUS" },
	{ "RXPMA_CORE_PHY2_2_LANE_HIS_STATUS" },
	{ "RXPMA_CORE_PHY2_3_LANE_HIS_STATUS" },
	{ "RXPMA_CORE_PHY2_4_LANE_HIS_STATUS" },
	{ "RXPMA_CORE_PHY2_5_LANE_HIS_STATUS" },
	{ "RXPMA_CORE_PHY2_6_LANE_HIS_STATUS" },
	{ "RXPMA_CORE_PHY2_7_LANE_HIS_STATUS" },
	{ "RXPMA_CORE_PHY2_8_LANE_HIS_STATUS" },
	{ "RXPMA_CORE_PHY2_9_LANE_HIS_STATUS" },
	{ "RXPMA_CORE_PHY2_10_LANE_HIS_STATUS" },
	{ "RXPMA_CORE_PHY2_11_LANE_HIS_STATUS" },
	{ "RXPMA_CORE_PHY2_12_LANE_HIS_STATUS" },
	{ "RXPMA_CORE_PHY2_13_LANE_HIS_STATUS" },
	{ "RXPMA_CORE_PHY2_14_LANE_HIS_STATUS" },
	{ "RXPMA_CORE_PHY2_15_LANE_HIS_STATUS" },
	{ "RXPMA_CORE_PHY2_16_LANE_HIS_STATUS" },
	{ "RXPMA_CORE_PHY2_17_LANE_HIS_STATUS" },
	{ "RXPMA_CORE_PHY2_18_LANE_HIS_STATUS" },
	{ "RXPMA_CORE_PHY2_19_LANE_HIS_STATUS" },
};

static const struct mac_reg_name rxpma_core_phy0_lane_his_status_r_offset_4[] = {
	{ "RXPMA_CORE_PHY4_0_LANE_HIS_STATUS" },
	{ "RXPMA_CORE_PHY4_1_LANE_HIS_STATUS" },
	{ "RXPMA_CORE_PHY4_2_LANE_HIS_STATUS" },
	{ "RXPMA_CORE_PHY4_3_LANE_HIS_STATUS" },
	{ "RXPMA_CORE_PHY4_4_LANE_HIS_STATUS" },
	{ "RXPMA_CORE_PHY4_5_LANE_HIS_STATUS" },
	{ "RXPMA_CORE_PHY4_6_LANE_HIS_STATUS" },
	{ "RXPMA_CORE_PHY4_7_LANE_HIS_STATUS" },
	{ "RXPMA_CORE_PHY4_8_LANE_HIS_STATUS" },
	{ "RXPMA_CORE_PHY4_9_LANE_HIS_STATUS" },
	{ "RXPMA_CORE_PHY4_10_LANE_HIS_STATUS" },
	{ "RXPMA_CORE_PHY4_11_LANE_HIS_STATUS" },
	{ "RXPMA_CORE_PHY4_12_LANE_HIS_STATUS" },
	{ "RXPMA_CORE_PHY4_13_LANE_HIS_STATUS" },
	{ "RXPMA_CORE_PHY4_14_LANE_HIS_STATUS" },
	{ "RXPMA_CORE_PHY4_15_LANE_HIS_STATUS" },
	{ "RXPMA_CORE_PHY4_16_LANE_HIS_STATUS" },
	{ "RXPMA_CORE_PHY4_17_LANE_HIS_STATUS" },
	{ "RXPMA_CORE_PHY4_18_LANE_HIS_STATUS" },
	{ "RXPMA_CORE_PHY4_19_LANE_HIS_STATUS" },
};

static const struct mac_reg_name rxpma_core_phy0_lane_his_status_r_offset_6[] = {
	{ "RXPMA_CORE_PHY6_0_LANE_HIS_STATUS" },
	{ "RXPMA_CORE_PHY6_1_LANE_HIS_STATUS" },
	{ "RXPMA_CORE_PHY6_2_LANE_HIS_STATUS" },
	{ "RXPMA_CORE_PHY6_3_LANE_HIS_STATUS" },
	{ "RXPMA_CORE_PHY6_4_LANE_HIS_STATUS" },
	{ "RXPMA_CORE_PHY6_5_LANE_HIS_STATUS" },
	{ "RXPMA_CORE_PHY6_6_LANE_HIS_STATUS" },
	{ "RXPMA_CORE_PHY6_7_LANE_HIS_STATUS" },
	{ "RXPMA_CORE_PHY6_8_LANE_HIS_STATUS" },
	{ "RXPMA_CORE_PHY6_9_LANE_HIS_STATUS" },
	{ "RXPMA_CORE_PHY6_10_LANE_HIS_STATUS" },
	{ "RXPMA_CORE_PHY6_11_LANE_HIS_STATUS" },
	{ "RXPMA_CORE_PHY6_12_LANE_HIS_STATUS" },
	{ "RXPMA_CORE_PHY6_13_LANE_HIS_STATUS" },
	{ "RXPMA_CORE_PHY6_14_LANE_HIS_STATUS" },
	{ "RXPMA_CORE_PHY6_15_LANE_HIS_STATUS" },
	{ "RXPMA_CORE_PHY6_16_LANE_HIS_STATUS" },
	{ "RXPMA_CORE_PHY6_17_LANE_HIS_STATUS" },
	{ "RXPMA_CORE_PHY6_18_LANE_HIS_STATUS" },
	{ "RXPMA_CORE_PHY6_19_LANE_HIS_STATUS" },
};

static const struct mac_reg_name rxpma_core_phy0_lane_skew_fifo_info_r[] = {
	{ "RXPMA_CORE_PHY0_0_LANE_SKEW_FIFO_INFO" },
	{ "RXPMA_CORE_PHY0_1_LANE_SKEW_FIFO_INFO" },
	{ "RXPMA_CORE_PHY0_2_LANE_SKEW_FIFO_INFO" },
	{ "RXPMA_CORE_PHY0_3_LANE_SKEW_FIFO_INFO" },
	{ "RXPMA_CORE_PHY0_4_LANE_SKEW_FIFO_INFO" },
	{ "RXPMA_CORE_PHY0_5_LANE_SKEW_FIFO_INFO" },
	{ "RXPMA_CORE_PHY0_6_LANE_SKEW_FIFO_INFO" },
	{ "RXPMA_CORE_PHY0_7_LANE_SKEW_FIFO_INFO" },
	{ "RXPMA_CORE_PHY0_8_LANE_SKEW_FIFO_INFO" },
	{ "RXPMA_CORE_PHY0_9_LANE_SKEW_FIFO_INFO" },
	{ "RXPMA_CORE_PHY0_10_LANE_SKEW_FIFO_INFO" },
	{ "RXPMA_CORE_PHY0_11_LANE_SKEW_FIFO_INFO" },
	{ "RXPMA_CORE_PHY0_12_LANE_SKEW_FIFO_INFO" },
	{ "RXPMA_CORE_PHY0_13_LANE_SKEW_FIFO_INFO" },
	{ "RXPMA_CORE_PHY0_14_LANE_SKEW_FIFO_INFO" },
	{ "RXPMA_CORE_PHY0_15_LANE_SKEW_FIFO_INFO" },
	{ "RXPMA_CORE_PHY0_16_LANE_SKEW_FIFO_INFO" },
	{ "RXPMA_CORE_PHY0_17_LANE_SKEW_FIFO_INFO" },
	{ "RXPMA_CORE_PHY0_18_LANE_SKEW_FIFO_INFO" },
	{ "RXPMA_CORE_PHY0_19_LANE_SKEW_FIFO_INFO" },
};

static const struct mac_reg_name rxpma_core_phy0_lane_skew_fifo_info_r_offset_2[] = {
	{ "RXPMA_CORE_PHY2_0_LANE_SKEW_FIFO_INFO" },
	{ "RXPMA_CORE_PHY2_1_LANE_SKEW_FIFO_INFO" },
	{ "RXPMA_CORE_PHY2_2_LANE_SKEW_FIFO_INFO" },
	{ "RXPMA_CORE_PHY2_3_LANE_SKEW_FIFO_INFO" },
	{ "RXPMA_CORE_PHY2_4_LANE_SKEW_FIFO_INFO" },
	{ "RXPMA_CORE_PHY2_5_LANE_SKEW_FIFO_INFO" },
	{ "RXPMA_CORE_PHY2_6_LANE_SKEW_FIFO_INFO" },
	{ "RXPMA_CORE_PHY2_7_LANE_SKEW_FIFO_INFO" },
	{ "RXPMA_CORE_PHY2_8_LANE_SKEW_FIFO_INFO" },
	{ "RXPMA_CORE_PHY2_9_LANE_SKEW_FIFO_INFO" },
	{ "RXPMA_CORE_PHY2_10_LANE_SKEW_FIFO_INFO" },
	{ "RXPMA_CORE_PHY2_11_LANE_SKEW_FIFO_INFO" },
	{ "RXPMA_CORE_PHY2_12_LANE_SKEW_FIFO_INFO" },
	{ "RXPMA_CORE_PHY2_13_LANE_SKEW_FIFO_INFO" },
	{ "RXPMA_CORE_PHY2_14_LANE_SKEW_FIFO_INFO" },
	{ "RXPMA_CORE_PHY2_15_LANE_SKEW_FIFO_INFO" },
	{ "RXPMA_CORE_PHY2_16_LANE_SKEW_FIFO_INFO" },
	{ "RXPMA_CORE_PHY2_17_LANE_SKEW_FIFO_INFO" },
	{ "RXPMA_CORE_PHY2_18_LANE_SKEW_FIFO_INFO" },
	{ "RXPMA_CORE_PHY2_19_LANE_SKEW_FIFO_INFO" },
};

static const struct mac_reg_name rxpma_core_phy0_lane_skew_fifo_info_r_offset_4[] = {
	{ "RXPMA_CORE_PHY4_0_LANE_SKEW_FIFO_INFO" },
	{ "RXPMA_CORE_PHY4_1_LANE_SKEW_FIFO_INFO" },
	{ "RXPMA_CORE_PHY4_2_LANE_SKEW_FIFO_INFO" },
	{ "RXPMA_CORE_PHY4_3_LANE_SKEW_FIFO_INFO" },
	{ "RXPMA_CORE_PHY4_4_LANE_SKEW_FIFO_INFO" },
	{ "RXPMA_CORE_PHY4_5_LANE_SKEW_FIFO_INFO" },
	{ "RXPMA_CORE_PHY4_6_LANE_SKEW_FIFO_INFO" },
	{ "RXPMA_CORE_PHY4_7_LANE_SKEW_FIFO_INFO" },
	{ "RXPMA_CORE_PHY4_8_LANE_SKEW_FIFO_INFO" },
	{ "RXPMA_CORE_PHY4_9_LANE_SKEW_FIFO_INFO" },
	{ "RXPMA_CORE_PHY4_10_LANE_SKEW_FIFO_INFO" },
	{ "RXPMA_CORE_PHY4_11_LANE_SKEW_FIFO_INFO" },
	{ "RXPMA_CORE_PHY4_12_LANE_SKEW_FIFO_INFO" },
	{ "RXPMA_CORE_PHY4_13_LANE_SKEW_FIFO_INFO" },
	{ "RXPMA_CORE_PHY4_14_LANE_SKEW_FIFO_INFO" },
	{ "RXPMA_CORE_PHY4_15_LANE_SKEW_FIFO_INFO" },
	{ "RXPMA_CORE_PHY4_16_LANE_SKEW_FIFO_INFO" },
	{ "RXPMA_CORE_PHY4_17_LANE_SKEW_FIFO_INFO" },
	{ "RXPMA_CORE_PHY4_18_LANE_SKEW_FIFO_INFO" },
	{ "RXPMA_CORE_PHY4_19_LANE_SKEW_FIFO_INFO" },
};

static const struct mac_reg_name rxpma_core_phy0_lane_skew_fifo_info_r_offset_6[] = {
	{ "RXPMA_CORE_PHY6_0_LANE_SKEW_FIFO_INFO" },
	{ "RXPMA_CORE_PHY6_1_LANE_SKEW_FIFO_INFO" },
	{ "RXPMA_CORE_PHY6_2_LANE_SKEW_FIFO_INFO" },
	{ "RXPMA_CORE_PHY6_3_LANE_SKEW_FIFO_INFO" },
	{ "RXPMA_CORE_PHY6_4_LANE_SKEW_FIFO_INFO" },
	{ "RXPMA_CORE_PHY6_5_LANE_SKEW_FIFO_INFO" },
	{ "RXPMA_CORE_PHY6_6_LANE_SKEW_FIFO_INFO" },
	{ "RXPMA_CORE_PHY6_7_LANE_SKEW_FIFO_INFO" },
	{ "RXPMA_CORE_PHY6_8_LANE_SKEW_FIFO_INFO" },
	{ "RXPMA_CORE_PHY6_9_LANE_SKEW_FIFO_INFO" },
	{ "RXPMA_CORE_PHY6_10_LANE_SKEW_FIFO_INFO" },
	{ "RXPMA_CORE_PHY6_11_LANE_SKEW_FIFO_INFO" },
	{ "RXPMA_CORE_PHY6_12_LANE_SKEW_FIFO_INFO" },
	{ "RXPMA_CORE_PHY6_13_LANE_SKEW_FIFO_INFO" },
	{ "RXPMA_CORE_PHY6_14_LANE_SKEW_FIFO_INFO" },
	{ "RXPMA_CORE_PHY6_15_LANE_SKEW_FIFO_INFO" },
	{ "RXPMA_CORE_PHY6_16_LANE_SKEW_FIFO_INFO" },
	{ "RXPMA_CORE_PHY6_17_LANE_SKEW_FIFO_INFO" },
	{ "RXPMA_CORE_PHY6_18_LANE_SKEW_FIFO_INFO" },
	{ "RXPMA_CORE_PHY6_19_LANE_SKEW_FIFO_INFO" },
};

static const struct mac_reg_info mac_type_rxpma_core_blk[] = {
	{ rxpma_core_calendar_table_r,			HIKP_ARRAY_SIZE(rxpma_core_calendar_table_r) },
	{ rxpma_core_phy_amwinsize_r,			HIKP_ARRAY_SIZE(rxpma_core_phy_amwinsize_r) },
	{ rxpma_core_phy0_lane_curr_status_r,		HIKP_ARRAY_SIZE(rxpma_core_phy0_lane_curr_status_r) },
	{ rxpma_core_phy0_lane_curr_status_r_offset_2,	HIKP_ARRAY_SIZE(rxpma_core_phy0_lane_curr_status_r_offset_2) },
	{ rxpma_core_phy0_lane_curr_status_r_offset_4,	HIKP_ARRAY_SIZE(rxpma_core_phy0_lane_curr_status_r_offset_4) },
	{ rxpma_core_phy0_lane_curr_status_r_offset_6,	HIKP_ARRAY_SIZE(rxpma_core_phy0_lane_curr_status_r_offset_6) },
	{ rxpma_core_phy0_lane_his_status_r,		HIKP_ARRAY_SIZE(rxpma_core_phy0_lane_his_status_r) },
	{ rxpma_core_phy0_lane_his_status_r_offset_2,	HIKP_ARRAY_SIZE(rxpma_core_phy0_lane_his_status_r_offset_2) },
	{ rxpma_core_phy0_lane_his_status_r_offset_4,	HIKP_ARRAY_SIZE(rxpma_core_phy0_lane_his_status_r_offset_4) },
	{ rxpma_core_phy0_lane_his_status_r_offset_6,	HIKP_ARRAY_SIZE(rxpma_core_phy0_lane_his_status_r_offset_6) },
	{ rxpma_core_phy0_lane_skew_fifo_info_r,	HIKP_ARRAY_SIZE(rxpma_core_phy0_lane_skew_fifo_info_r) },
	{ rxpma_core_phy0_lane_skew_fifo_info_r_offset_2,
						HIKP_ARRAY_SIZE(rxpma_core_phy0_lane_skew_fifo_info_r_offset_2) },
	{ rxpma_core_phy0_lane_skew_fifo_info_r_offset_4,
						HIKP_ARRAY_SIZE(rxpma_core_phy0_lane_skew_fifo_info_r_offset_4) },
	{ rxpma_core_phy0_lane_skew_fifo_info_r_offset_6,
						HIKP_ARRAY_SIZE(rxpma_core_phy0_lane_skew_fifo_info_r_offset_6) },
};

static const struct mac_type_name_parse g_mac_rxpma_core_name_parse[] = {
	{ mac_type_rxpma_core_reg,		HIKP_ARRAY_SIZE(mac_type_rxpma_core_reg),
		ROUND_UP(mac_type_rxpma_core_reg[0].reg_num, PER_BLK_DATA_SIZE), false },
	{ mac_type_rxpma_core_phy_blk,		HIKP_ARRAY_SIZE(mac_type_rxpma_core_phy_blk),
		HIKP_ARRAY_SIZE(mac_type_rxpma_core_phy_blk), true },
	{ mac_type_rxpma_core_blk,		HIKP_ARRAY_SIZE(mac_type_rxpma_core_blk),
		HIKP_ARRAY_SIZE(mac_type_rxpma_core_blk), true },
};

static const struct mac_reg_name rxpma_lane_int_status_r[] = {
	{ "RXPMA_LANE_INT_STATUS" },
	{ "RXPMA_LANE_OVF_INT_STATUS" },
	{ "RXPMA_LANE_UDF_INT_STATUS" },
	{ "RXPMA_LANE_DBG_IERR_INSERT" },
};

static const struct mac_reg_info mac_type_rxpma_lane_reg[] = {
	{ rxpma_lane_int_status_r,		HIKP_ARRAY_SIZE(rxpma_lane_int_status_r) },
};

static const struct mac_reg_name rxpma_lane_phy_self_reset_en_r[] = {
	{ "RXPMA_LANE_PHY0_SELF_RESET_EN" },
	{ "RXPMA_LANE_PHY1_SELF_RESET_EN" },
	{ "RXPMA_LANE_PHY2_SELF_RESET_EN" },
	{ "RXPMA_LANE_PHY3_SELF_RESET_EN" },
	{ "RXPMA_LANE_PHY4_SELF_RESET_EN" },
	{ "RXPMA_LANE_PHY5_SELF_RESET_EN" },
	{ "RXPMA_LANE_PHY6_SELF_RESET_EN" },
	{ "RXPMA_LANE_PHY7_SELF_RESET_EN" },
};

static const struct mac_reg_name rxpma_lane_phy_control_r[] = {
	{ "RXPMA_LANE_PHY0_CONTROL" },
	{ "RXPMA_LANE_PHY1_CONTROL" },
	{ "RXPMA_LANE_PHY2_CONTROL" },
	{ "RXPMA_LANE_PHY3_CONTROL" },
	{ "RXPMA_LANE_PHY4_CONTROL" },
	{ "RXPMA_LANE_PHY5_CONTROL" },
	{ "RXPMA_LANE_PHY6_CONTROL" },
	{ "RXPMA_LANE_PHY7_CONTROL" },
	{ "RXPMA_LANE_PHY0_CLK_SKEW_CONTROL" },
	{ "RXPMA_LANE_PHY1_CLK_SKEW_CONTROL" },
	{ "RXPMA_LANE_PHY2_CLK_SKEW_CONTROL" },
	{ "RXPMA_LANE_PHY3_CLK_SKEW_CONTROL" },
	{ "RXPMA_LANE_PHY4_CLK_SKEW_CONTROL" },
	{ "RXPMA_LANE_PHY5_CLK_SKEW_CONTROL" },
	{ "RXPMA_LANE_PHY6_CLK_SKEW_CONTROL" },
	{ "RXPMA_LANE_PHY7_CLK_SKEW_CONTROL" },
	{ "RXPMA_LANE_PHY0_1588_CFG" },
	{ "RXPMA_LANE_PHY1_1588_CFG" },
	{ "RXPMA_LANE_PHY2_1588_CFG" },
	{ "RXPMA_LANE_PHY3_1588_CFG" },
	{ "RXPMA_LANE_PHY4_1588_CFG" },
	{ "RXPMA_LANE_PHY5_1588_CFG" },
	{ "RXPMA_LANE_PHY6_1588_CFG" },
	{ "RXPMA_LANE_PHY7_1588_CFG" },
};

static const struct mac_reg_name rxpma_lane_afifo_ctrl_r[] = {
	{ "RXPMA_LANE_AFIFO_0_CTRL" },
	{ "RXPMA_LANE_AFIFO_1_CTRL" },
	{ "RXPMA_LANE_AFIFO_2_CTRL" },
	{ "RXPMA_LANE_AFIFO_3_CTRL" },
	{ "RXPMA_LANE_AFIFO_4_CTRL" },
	{ "RXPMA_LANE_AFIFO_5_CTRL" },
	{ "RXPMA_LANE_AFIFO_6_CTRL" },
	{ "RXPMA_LANE_AFIFO_7_CTRL" },
};

static const struct mac_reg_name rxpma_lane_afifo_curr_status_r[] = {
	{ "RXPMA_LANE_AFIFO_0_CURR_STATUS" },
	{ "RXPMA_LANE_AFIFO_1_CURR_STATUS" },
	{ "RXPMA_LANE_AFIFO_2_CURR_STATUS" },
	{ "RXPMA_LANE_AFIFO_3_CURR_STATUS" },
	{ "RXPMA_LANE_AFIFO_4_CURR_STATUS" },
	{ "RXPMA_LANE_AFIFO_5_CURR_STATUS" },
	{ "RXPMA_LANE_AFIFO_6_CURR_STATUS" },
	{ "RXPMA_LANE_AFIFO_7_CURR_STATUS" },
	{ "RXPMA_LANE_AFIFO_0_STATUS" },
	{ "RXPMA_LANE_AFIFO_1_STATUS" },
	{ "RXPMA_LANE_AFIFO_2_STATUS" },
	{ "RXPMA_LANE_AFIFO_3_STATUS" },
	{ "RXPMA_LANE_AFIFO_4_STATUS" },
	{ "RXPMA_LANE_AFIFO_5_STATUS" },
	{ "RXPMA_LANE_AFIFO_6_STATUS" },
	{ "RXPMA_LANE_AFIFO_7_STATUS" },
};

static const struct mac_reg_name rxpma_lane_signal_his_status_r[] = {
	{ "RXPMA_LANE_0_SIGNAL_HIS_STATUS" },
	{ "RXPMA_LANE_1_SIGNAL_HIS_STATUS" },
	{ "RXPMA_LANE_2_SIGNAL_HIS_STATUS" },
	{ "RXPMA_LANE_3_SIGNAL_HIS_STATUS" },
	{ "RXPMA_LANE_4_SIGNAL_HIS_STATUS" },
	{ "RXPMA_LANE_5_SIGNAL_HIS_STATUS" },
	{ "RXPMA_LANE_6_SIGNAL_HIS_STATUS" },
	{ "RXPMA_LANE_7_SIGNAL_HIS_STATUS" },
	{ "RXPMA_LANE_0_SIGNAL_STATUS" },
	{ "RXPMA_LANE_1_SIGNAL_STATUS" },
	{ "RXPMA_LANE_2_SIGNAL_STATUS" },
	{ "RXPMA_LANE_3_SIGNAL_STATUS" },
	{ "RXPMA_LANE_4_SIGNAL_STATUS" },
	{ "RXPMA_LANE_5_SIGNAL_STATUS" },
	{ "RXPMA_LANE_6_SIGNAL_STATUS" },
	{ "RXPMA_LANE_7_SIGNAL_STATUS" },
};

static const struct mac_reg_name rxpma_lane_dbg_ctrl_r[] = {
	{ "RXPMA_LANE_DBG_0_CTRL" },
	{ "RXPMA_LANE_DBG_1_CTRL" },
	{ "RXPMA_LANE_DBG_2_CTRL" },
	{ "RXPMA_LANE_DBG_3_CTRL" },
	{ "RXPMA_LANE_DBG_4_CTRL" },
	{ "RXPMA_LANE_DBG_5_CTRL" },
	{ "RXPMA_LANE_DBG_6_CTRL" },
	{ "RXPMA_LANE_DBG_7_CTRL" },
	{ "RXPMA_LANE_DBG_0_CURR_STATUS" },
	{ "RXPMA_LANE_DBG_1_CURR_STATUS" },
	{ "RXPMA_LANE_DBG_2_CURR_STATUS" },
	{ "RXPMA_LANE_DBG_3_CURR_STATUS" },
	{ "RXPMA_LANE_DBG_4_CURR_STATUS" },
	{ "RXPMA_LANE_DBG_5_CURR_STATUS" },
	{ "RXPMA_LANE_DBG_6_CURR_STATUS" },
	{ "RXPMA_LANE_DBG_7_CURR_STATUS" },
	{ "RXPMA_LANE_DBG_0_HIS_STATUS" },
	{ "RXPMA_LANE_DBG_1_HIS_STATUS" },
	{ "RXPMA_LANE_DBG_2_HIS_STATUS" },
	{ "RXPMA_LANE_DBG_3_HIS_STATUS" },
	{ "RXPMA_LANE_DBG_4_HIS_STATUS" },
	{ "RXPMA_LANE_DBG_5_HIS_STATUS" },
	{ "RXPMA_LANE_DBG_6_HIS_STATUS" },
	{ "RXPMA_LANE_DBG_7_HIS_STATUS" },
};

static const struct mac_reg_info mac_type_rxpma_lane_blk[] = {
	{ rxpma_lane_phy_self_reset_en_r,	HIKP_ARRAY_SIZE(rxpma_lane_phy_self_reset_en_r) },
	{ rxpma_lane_phy_control_r,		HIKP_ARRAY_SIZE(rxpma_lane_phy_control_r) },
	{ rxpma_lane_afifo_ctrl_r,		HIKP_ARRAY_SIZE(rxpma_lane_afifo_ctrl_r) },
	{ rxpma_lane_afifo_curr_status_r,	HIKP_ARRAY_SIZE(rxpma_lane_afifo_curr_status_r) },
	{ rxpma_lane_signal_his_status_r,	HIKP_ARRAY_SIZE(rxpma_lane_signal_his_status_r) },
	{ rxpma_lane_dbg_ctrl_r,		HIKP_ARRAY_SIZE(rxpma_lane_dbg_ctrl_r) },
};

static const struct mac_type_name_parse g_mac_rxpma_lane_name_parse[] = {
	{ mac_type_rxpma_lane_reg,		HIKP_ARRAY_SIZE(mac_type_rxpma_lane_reg),
		ROUND_UP(mac_type_rxpma_lane_reg[0].reg_num, PER_BLK_DATA_SIZE), false },
	{ mac_type_rxpma_lane_blk,		HIKP_ARRAY_SIZE(mac_type_rxpma_lane_blk),
		HIKP_ARRAY_SIZE(mac_type_rxpma_lane_blk), true },
};

static const struct mac_reg_name txpma_lane_int_status_r[] = {
	{ "TXPMA_LANE_INT_STATUS" },
	{ "TXPMA_LANE_OVF_INT_STATUS" },
	{ "TXPMA_LANE_UDF_INT_STATUS" },
};

static const struct mac_reg_info mac_type_txpma_lane_reg[] = {
	{ txpma_lane_int_status_r,	HIKP_ARRAY_SIZE(txpma_lane_int_status_r) },
};

static const struct mac_reg_name txpma_lane_phy_self_reset_en_r[] = {
	{ "TXPMA_LANE_PHY0_SELF_RESET_EN" },
	{ "TXPMA_LANE_PHY1_SELF_RESET_EN" },
	{ "TXPMA_LANE_PHY2_SELF_RESET_EN" },
	{ "TXPMA_LANE_PHY3_SELF_RESET_EN" },
	{ "TXPMA_LANE_PHY4_SELF_RESET_EN" },
	{ "TXPMA_LANE_PHY5_SELF_RESET_EN" },
	{ "TXPMA_LANE_PHY6_SELF_RESET_EN" },
	{ "TXPMA_LANE_PHY7_SELF_RESET_EN" },
	{ "TXPMA_LANE_PHY0_LOW_LATENCY_EN" },
	{ "TXPMA_LANE_PHY1_LOW_LATENCY_EN" },
	{ "TXPMA_LANE_PHY2_LOW_LATENCY_EN" },
	{ "TXPMA_LANE_PHY3_LOW_LATENCY_EN" },
	{ "TXPMA_LANE_PHY4_LOW_LATENCY_EN" },
	{ "TXPMA_LANE_PHY5_LOW_LATENCY_EN" },
	{ "TXPMA_LANE_PHY6_LOW_LATENCY_EN" },
	{ "TXPMA_LANE_PHY7_LOW_LATENCY_EN" },
	{ "TXPMA_LANE_PHY0_RD_ALIGN_EN" },
	{ "TXPMA_LANE_PHY1_RD_ALIGN_EN" },
	{ "TXPMA_LANE_PHY2_RD_ALIGN_EN" },
	{ "TXPMA_LANE_PHY3_RD_ALIGN_EN" },
	{ "TXPMA_LANE_PHY4_RD_ALIGN_EN" },
	{ "TXPMA_LANE_PHY5_RD_ALIGN_EN" },
	{ "TXPMA_LANE_PHY6_RD_ALIGN_EN" },
	{ "TXPMA_LANE_PHY7_RD_ALIGN_EN" },
};

static const struct mac_reg_name txpma_lane_phy_rd_delay_th_r[] = {
	{ "TXPMA_LANE_PHY0_RD_DELAY_TH" },
	{ "TXPMA_LANE_PHY1_RD_DELAY_TH" },
	{ "TXPMA_LANE_PHY2_RD_DELAY_TH" },
	{ "TXPMA_LANE_PHY3_RD_DELAY_TH" },
	{ "TXPMA_LANE_PHY4_RD_DELAY_TH" },
	{ "TXPMA_LANE_PHY5_RD_DELAY_TH" },
	{ "TXPMA_LANE_PHY6_RD_DELAY_TH" },
	{ "TXPMA_LANE_PHY7_RD_DELAY_TH" },
	{ "TXPMA_LANE_PHY0_START_TH" },
	{ "TXPMA_LANE_PHY1_START_TH" },
	{ "TXPMA_LANE_PHY2_START_TH" },
	{ "TXPMA_LANE_PHY3_START_TH" },
	{ "TXPMA_LANE_PHY4_START_TH" },
	{ "TXPMA_LANE_PHY5_START_TH" },
	{ "TXPMA_LANE_PHY6_START_TH" },
	{ "TXPMA_LANE_PHY7_START_TH" },
};

static const struct mac_reg_name txpma_lane_phy_control_r[] = {
	{ "TXPMA_LANE_PHY0_CONTROL" },
	{ "TXPMA_LANE_PHY1_CONTROL" },
	{ "TXPMA_LANE_PHY2_CONTROL" },
	{ "TXPMA_LANE_PHY3_CONTROL" },
	{ "TXPMA_LANE_PHY4_CONTROL" },
	{ "TXPMA_LANE_PHY5_CONTROL" },
	{ "TXPMA_LANE_PHY6_CONTROL" },
	{ "TXPMA_LANE_PHY7_CONTROL" },
	{ "TXPMA_LANE_PHY0_1588_EN" },
	{ "TXPMA_LANE_PHY1_1588_EN" },
	{ "TXPMA_LANE_PHY2_1588_EN" },
	{ "TXPMA_LANE_PHY3_1588_EN" },
	{ "TXPMA_LANE_PHY4_1588_EN" },
	{ "TXPMA_LANE_PHY5_1588_EN" },
	{ "TXPMA_LANE_PHY6_1588_EN" },
	{ "TXPMA_LANE_PHY7_1588_EN" },
};

static const struct mac_reg_name txpma_lane_lane_fifo_curr_cnt_r[] = {
	{ "TXPMA_LANE_LANE0_FIFO_CURR_CNT" },
	{ "TXPMA_LANE_LANE1_FIFO_CURR_CNT" },
	{ "TXPMA_LANE_LANE2_FIFO_CURR_CNT" },
	{ "TXPMA_LANE_LANE3_FIFO_CURR_CNT" },
	{ "TXPMA_LANE_LANE4_FIFO_CURR_CNT" },
	{ "TXPMA_LANE_LANE5_FIFO_CURR_CNT" },
	{ "TXPMA_LANE_LANE6_FIFO_CURR_CNT" },
	{ "TXPMA_LANE_LANE7_FIFO_CURR_CNT" },
};

static const struct mac_reg_info mac_type_txpma_lane_blk[] = {
	{ txpma_lane_phy_self_reset_en_r,	HIKP_ARRAY_SIZE(txpma_lane_phy_self_reset_en_r) },
	{ txpma_lane_phy_rd_delay_th_r,		HIKP_ARRAY_SIZE(txpma_lane_phy_rd_delay_th_r) },
	{ txpma_lane_phy_control_r,		HIKP_ARRAY_SIZE(txpma_lane_phy_control_r) },
	{ txpma_lane_lane_fifo_curr_cnt_r,	HIKP_ARRAY_SIZE(txpma_lane_lane_fifo_curr_cnt_r) },
};

static const struct mac_type_name_parse g_mac_txpma_lane_name_parse[] = {
	{ mac_type_txpma_lane_reg,		HIKP_ARRAY_SIZE(mac_type_txpma_lane_reg),
		ROUND_UP(mac_type_txpma_lane_reg[0].reg_num, PER_BLK_DATA_SIZE), false },
	{ mac_type_txpma_lane_blk,		HIKP_ARRAY_SIZE(mac_type_txpma_lane_blk),
		HIKP_ARRAY_SIZE(mac_type_txpma_lane_blk), true },
};

static const struct mac_reg_name txpma_core_int_status_r[] = {
	{ "TXPMA_CORE_INT_STATUS" },
};

static const struct mac_reg_info mac_type_txpma_core_reg[] = {
	{ txpma_core_int_status_r,	HIKP_ARRAY_SIZE(txpma_core_int_status_r) },
};

static const struct mac_reg_name txpma_core_phy_mode_r[] = {
	{ "TXPMA_CORE_PHY0_MODE" },
	{ "TXPMA_CORE_PHY1_MODE" },
	{ "TXPMA_CORE_PHY2_MODE" },
	{ "TXPMA_CORE_PHY3_MODE" },
	{ "TXPMA_CORE_PHY4_MODE" },
	{ "TXPMA_CORE_PHY5_MODE" },
	{ "TXPMA_CORE_PHY6_MODE" },
	{ "TXPMA_CORE_PHY7_MODE" },
};

static const struct mac_reg_info mac_type_txpma_core_blk[] = {
	{ txpma_core_phy_mode_r,	HIKP_ARRAY_SIZE(txpma_core_phy_mode_r) },
};

static const struct mac_type_name_parse g_mac_txpma_core_name_parse[] = {
	{ mac_type_txpma_core_reg,		HIKP_ARRAY_SIZE(mac_type_txpma_core_reg),
		ROUND_UP(mac_type_txpma_core_reg[0].reg_num, PER_BLK_DATA_SIZE), false },
	{ mac_type_txpma_core_blk,		HIKP_ARRAY_SIZE(mac_type_txpma_core_blk),
		HIKP_ARRAY_SIZE(mac_type_txpma_core_blk), true },
};

static const struct mac_reg_name tx_brfec_int_status_r[] = {
	{ "TX_BRFEC_INT_STATUS" },
};

static const struct mac_reg_info mac_type_tx_brfec_reg[] = {
	{ tx_brfec_int_status_r,	HIKP_ARRAY_SIZE(tx_brfec_int_status_r) },
};

static const struct mac_reg_name tx_brfec_phy0_dbg_err_ins_cnt_r[] = {
	{ "TX_BRFEC_PHY0_DBG_ERR_INS_CNT" },
	{ "TX_BRFEC_PHY1_DBG_ERR_INS_CNT" },
	{ "TX_BRFEC_PHY2_DBG_ERR_INS_CNT" },
	{ "TX_BRFEC_PHY3_DBG_ERR_INS_CNT" },
	{ "TX_BRFEC_PHY4_DBG_ERR_INS_CNT" },
	{ "TX_BRFEC_PHY5_DBG_ERR_INS_CNT" },
	{ "TX_BRFEC_PHY6_DBG_ERR_INS_CNT" },
	{ "TX_BRFEC_PHY7_DBG_ERR_INS_CNT" },
};

static const struct mac_reg_name tx_brfec_phy0_dbg_his_st_r[] = {
	{ "TX_BRFEC_PHY0_DBG_HIS_ST" },
	{ "TX_BRFEC_PHY1_DBG_HIS_ST" },
	{ "TX_BRFEC_PHY2_DBG_HIS_ST" },
	{ "TX_BRFEC_PHY3_DBG_HIS_ST" },
	{ "TX_BRFEC_PHY4_DBG_HIS_ST" },
	{ "TX_BRFEC_PHY5_DBG_HIS_ST" },
	{ "TX_BRFEC_PHY6_DBG_HIS_ST" },
	{ "TX_BRFEC_PHY7_DBG_HIS_ST" },
};

static const struct mac_reg_info mac_type_tx_brfec_phy_blk[] = {
	{ tx_brfec_phy0_dbg_err_ins_cnt_r,	HIKP_ARRAY_SIZE(tx_brfec_phy0_dbg_err_ins_cnt_r) },
	{ tx_brfec_phy0_dbg_his_st_r,		HIKP_ARRAY_SIZE(tx_brfec_phy0_dbg_his_st_r) },
};

static const struct mac_type_name_parse g_mac_tx_brfec_name_parse[] = {
	{ mac_type_tx_brfec_reg,		HIKP_ARRAY_SIZE(mac_type_tx_brfec_reg),
		ROUND_UP(mac_type_tx_brfec_reg[0].reg_num, PER_BLK_DATA_SIZE), false },
	{ mac_type_tx_brfec_phy_blk,		HIKP_ARRAY_SIZE(mac_type_tx_brfec_phy_blk),
		HIKP_ARRAY_SIZE(mac_type_tx_brfec_phy_blk), true },
};

static const struct mac_reg_name tx_rsfec_int_status_r[] = {
	{ "TX_RSFEC_INT_STATUS" },
	{ "TX_RSFEC_SCH_SEL" },
	{ "TX_RSFEC_SCH_CFG" },
	{ "TX_RSFEC_DEGRAD_SER_CFG" },
	{ "TX_RSFEC_TX_ERR_CW_CNT0" },
	{ "TX_RSFEC_TX_ERR_CW_CNT1" },
};

static const struct mac_reg_info mac_type_tx_rsfec_reg[] = {
	{ tx_rsfec_int_status_r,		HIKP_ARRAY_SIZE(tx_rsfec_int_status_r) },
};

static const struct mac_reg_name tx_rsfec_phy_err_status_r[] = {
	{ "TX_RSFEC_PHY0_ERR_STATUS" },
	{ "TX_RSFEC_PHY1_ERR_STATUS" },
	{ "TX_RSFEC_PHY2_ERR_STATUS" },
	{ "TX_RSFEC_PHY3_ERR_STATUS" },
	{ "TX_RSFEC_PHY4_ERR_STATUS" },
	{ "TX_RSFEC_PHY5_ERR_STATUS" },
	{ "TX_RSFEC_PHY6_ERR_STATUS" },
	{ "TX_RSFEC_PHY7_ERR_STATUS" },
};

static const struct mac_reg_name tx_rsfec_phy_amwinsize_r[] = {
	{ "TX_RSFEC_PHY0_AMWINSIZE" },
	{ "TX_RSFEC_PHY1_AMWINSIZE" },
	{ "TX_RSFEC_PHY2_AMWINSIZE" },
	{ "TX_RSFEC_PHY3_AMWINSIZE" },
	{ "TX_RSFEC_PHY4_AMWINSIZE" },
	{ "TX_RSFEC_PHY5_AMWINSIZE" },
	{ "TX_RSFEC_PHY6_AMWINSIZE" },
	{ "TX_RSFEC_PHY7_AMWINSIZE" },
};

static const struct mac_reg_name tx_rsfec_tx_inv_block_cnt_r[] = {
	{ "TX_RSFEC_TX_INV_BLOCK_CNT0" },
	{ "TX_RSFEC_TX_INV_BLOCK_CNT1" },
	{ "TX_RSFEC_TX_INV_BLOCK_CNT2" },
	{ "TX_RSFEC_TX_INV_BLOCK_CNT3" },
	{ "TX_RSFEC_TX_INV_BLOCK_CNT4" },
	{ "TX_RSFEC_TX_INV_BLOCK_CNT5" },
	{ "TX_RSFEC_TX_INV_BLOCK_CNT6" },
	{ "TX_RSFEC_TX_INV_BLOCK_CNT7" },
	{ "TX_RSFEC_TS_CTRL_BUF_MAX0" },
	{ "TX_RSFEC_TS_CTRL_BUF_MAX1" },
	{ "TX_RSFEC_TS_CTRL_BUF_MAX2" },
	{ "TX_RSFEC_TS_CTRL_BUF_MAX3" },
	{ "TX_RSFEC_TS_CTRL_BUF_MAX4" },
	{ "TX_RSFEC_TS_CTRL_BUF_MAX5" },
	{ "TX_RSFEC_TS_CTRL_BUF_MAX6" },
	{ "TX_RSFEC_TS_CTRL_BUF_MAX7" },
};

static const struct mac_reg_info mac_type_tx_rsfec_blk[] = {
	{ tx_rsfec_phy_err_status_r,		HIKP_ARRAY_SIZE(tx_rsfec_phy_err_status_r) },
	{ tx_rsfec_phy_amwinsize_r,		HIKP_ARRAY_SIZE(tx_rsfec_phy_amwinsize_r) },
	{ tx_rsfec_tx_inv_block_cnt_r,		HIKP_ARRAY_SIZE(tx_rsfec_tx_inv_block_cnt_r) },
};

static const struct mac_type_name_parse g_mac_tx_rsfec_name_parse[] = {
	{ mac_type_tx_rsfec_reg,		HIKP_ARRAY_SIZE(mac_type_tx_rsfec_reg),
		ROUND_UP(mac_type_tx_rsfec_reg[0].reg_num, PER_BLK_DATA_SIZE), false },
	{ mac_type_tx_rsfec_blk,		HIKP_ARRAY_SIZE(mac_type_tx_rsfec_blk),
		HIKP_ARRAY_SIZE(mac_type_tx_rsfec_blk), true },
};

static const struct mac_reg_name tx_pcs_int_status_r[] = {
	{ "TX_PCS_INT_STATUS" },
};

static const struct mac_reg_info mac_type_tx_pcs_reg[] = {
	{ tx_pcs_int_status_r,		HIKP_ARRAY_SIZE(tx_pcs_int_status_r) },
};

static const struct mac_reg_name tx_pcs_phy_amwinsize_r[] = {
	{ "TX_PCS_PHY0_AMWINSIZE" },
	{ "TX_PCS_PHY1_AMWINSIZE" },
	{ "TX_PCS_PHY2_AMWINSIZE" },
	{ "TX_PCS_PHY3_AMWINSIZE" },
	{ "TX_PCS_PHY4_AMWINSIZE" },
	{ "TX_PCS_PHY5_AMWINSIZE" },
	{ "TX_PCS_PHY6_AMWINSIZE" },
	{ "TX_PCS_PHY7_AMWINSIZE" },
};

static const struct mac_reg_info mac_type_tx_pcs_blk[] = {
	{ tx_pcs_phy_amwinsize_r,		HIKP_ARRAY_SIZE(tx_pcs_phy_amwinsize_r) },
};

static const struct mac_reg_name tx_pcs_phy0_rstn_control_r[] = {
	{ "TX_PCS_PHY0_RSTN_CONTROL" },
	{ "TX_PCS_PHY1_RSTN_CONTROL" },
	{ "TX_PCS_PHY2_RSTN_CONTROL" },
	{ "TX_PCS_PHY3_RSTN_CONTROL" },
	{ "TX_PCS_PHY4_RSTN_CONTROL" },
	{ "TX_PCS_PHY5_RSTN_CONTROL" },
	{ "TX_PCS_PHY6_RSTN_CONTROL" },
	{ "TX_PCS_PHY7_RSTN_CONTROL" },
};

static const struct mac_reg_name tx_pcs_phy0_baser_control_r[] = {
	{ "TX_PCS_PHY0_BASER_CONTROL" },
	{ "TX_PCS_PHY1_BASER_CONTROL" },
	{ "TX_PCS_PHY2_BASER_CONTROL" },
	{ "TX_PCS_PHY3_BASER_CONTROL" },
	{ "TX_PCS_PHY4_BASER_CONTROL" },
	{ "TX_PCS_PHY5_BASER_CONTROL" },
	{ "TX_PCS_PHY6_BASER_CONTROL" },
	{ "TX_PCS_PHY7_BASER_CONTROL" },
};

static const struct mac_reg_name tx_pcs_phy0_am_det_control_r[] = {
	{ "TX_PCS_PHY0_AM_DET_CONTROL" },
	{ "TX_PCS_PHY1_AM_DET_CONTROL" },
	{ "TX_PCS_PHY2_AM_DET_CONTROL" },
	{ "TX_PCS_PHY3_AM_DET_CONTROL" },
	{ "TX_PCS_PHY4_AM_DET_CONTROL" },
	{ "TX_PCS_PHY5_AM_DET_CONTROL" },
	{ "TX_PCS_PHY6_AM_DET_CONTROL" },
	{ "TX_PCS_PHY7_AM_DET_CONTROL" },
};

static const struct mac_reg_name tx_pcs_phy0_am_bip_control_r[] = {
	{ "TX_PCS_PHY0_AM_BIP_CONTROL" },
	{ "TX_PCS_PHY1_AM_BIP_CONTROL" },
	{ "TX_PCS_PHY2_AM_BIP_CONTROL" },
	{ "TX_PCS_PHY3_AM_BIP_CONTROL" },
	{ "TX_PCS_PHY4_AM_BIP_CONTROL" },
	{ "TX_PCS_PHY5_AM_BIP_CONTROL" },
	{ "TX_PCS_PHY6_AM_BIP_CONTROL" },
	{ "TX_PCS_PHY7_AM_BIP_CONTROL" },
};

static const struct mac_reg_name tx_pcs_phy0_dbg_his_status_r[] = {
	{ "TX_PCS_PHY0_DBG_HIS_STATUS" },
	{ "TX_PCS_PHY1_DBG_HIS_STATUS" },
	{ "TX_PCS_PHY2_DBG_HIS_STATUS" },
	{ "TX_PCS_PHY3_DBG_HIS_STATUS" },
	{ "TX_PCS_PHY4_DBG_HIS_STATUS" },
	{ "TX_PCS_PHY5_DBG_HIS_STATUS" },
	{ "TX_PCS_PHY6_DBG_HIS_STATUS" },
	{ "TX_PCS_PHY7_DBG_HIS_STATUS" },
};

static const struct mac_reg_info mac_type_tx_pcs_phy_blk[] = {
	{ tx_pcs_phy0_rstn_control_r,		HIKP_ARRAY_SIZE(tx_pcs_phy0_rstn_control_r) },
	{ tx_pcs_phy0_baser_control_r,		HIKP_ARRAY_SIZE(tx_pcs_phy0_baser_control_r) },
	{ tx_pcs_phy0_am_det_control_r,		HIKP_ARRAY_SIZE(tx_pcs_phy0_am_det_control_r) },
	{ tx_pcs_phy0_am_bip_control_r,		HIKP_ARRAY_SIZE(tx_pcs_phy0_am_bip_control_r) },
	{ tx_pcs_phy0_dbg_his_status_r,		HIKP_ARRAY_SIZE(tx_pcs_phy0_dbg_his_status_r) },
};

static const struct mac_type_name_parse g_mac_tx_pcs_name_parse[] = {
	{ mac_type_tx_pcs_reg,			HIKP_ARRAY_SIZE(mac_type_tx_pcs_reg),
		ROUND_UP(mac_type_tx_pcs_reg[0].reg_num, PER_BLK_DATA_SIZE), false },
	{ mac_type_tx_pcs_phy_blk,		HIKP_ARRAY_SIZE(mac_type_tx_pcs_phy_blk),
		HIKP_ARRAY_SIZE(mac_type_tx_pcs_phy_blk), true },
	{ mac_type_tx_pcs_blk,			HIKP_ARRAY_SIZE(mac_type_tx_pcs_blk),
		HIKP_ARRAY_SIZE(mac_type_tx_pcs_blk), true },
};

static const struct mac_reg_name tx_mac_int_status_r[] = {
	{ "TX_MAC_INT_STATUS" },
	{ "TX_MAC_QUE_OVF_INT_STATUS" },
	{ "TX_MAC_QUE_UDR_INT_STATUS" },
	{ "TX_MAC_REF_1588_OVF_INT_STATUS" },
	{ "TX_MAC_MEM_INIT_START" },
	{ "TX_MAC_MEM_INIT_STATUS" },
	{ "TX_MAC_TX_IDLE_DEL_GAP" },
	{ "TX_MAC_EOP_TIMEOUT_CYC" },
	{ "TX_MAC_GLB_LOW_LAT_CFG" },
	{ "TX_MAC_AM_PERIOD" },
	{ "TX_MAC_GLB_TDM_SCH_CFG" },
	{ "TX_MAC_PHY_DIAG_1588_REF_GAP_JIT_TH" },
	{ "TX_MAC_DIAG_LP_ID" },
	{ "TX_MAC_DIAG_HIS_STATUS" },
};

static const struct mac_reg_info mac_type_tx_mac_reg[] = {
	{ tx_mac_int_status_r,		HIKP_ARRAY_SIZE(tx_mac_int_status_r) },
};

static const struct mac_reg_name tx_mac_calendar_table_r[] = {
	{ "TX_MAC_CALENDAR0_TABLE" },
	{ "TX_MAC_CALENDAR1_TABLE" },
	{ "TX_MAC_CALENDAR2_TABLE" },
	{ "TX_MAC_CALENDAR3_TABLE" },
	{ "TX_MAC_CALENDAR4_TABLE" },
	{ "TX_MAC_CALENDAR5_TABLE" },
	{ "TX_MAC_CALENDAR6_TABLE" },
	{ "TX_MAC_CALENDAR7_TABLE" },
	{ "TX_MAC_CALENDAR8_TABLE" },
	{ "TX_MAC_CALENDAR9_TABLE" },
	{ "TX_MAC_CALENDAR10_TABLE" },
	{ "TX_MAC_CALENDAR11_TABLE" },
	{ "TX_MAC_CALENDAR12_TABLE" },
	{ "TX_MAC_CALENDAR13_TABLE" },
	{ "TX_MAC_CALENDAR14_TABLE" },
	{ "TX_MAC_CALENDAR15_TABLE" },
	{ "TX_MAC_CALENDAR16_TABLE" },
	{ "TX_MAC_CALENDAR17_TABLE" },
	{ "TX_MAC_CALENDAR18_TABLE" },
	{ "TX_MAC_CALENDAR19_TABLE" },
	{ "TX_MAC_CALENDAR20_TABLE" },
	{ "TX_MAC_CALENDAR21_TABLE" },
	{ "TX_MAC_CALENDAR22_TABLE" },
	{ "TX_MAC_CALENDAR23_TABLE" },
	{ "TX_MAC_CALENDAR24_TABLE" },
	{ "TX_MAC_CALENDAR25_TABLE" },
	{ "TX_MAC_CALENDAR26_TABLE" },
	{ "TX_MAC_CALENDAR27_TABLE" },
	{ "TX_MAC_CALENDAR28_TABLE" },
	{ "TX_MAC_CALENDAR29_TABLE" },
};

static const struct mac_reg_name tx_mac_calendar_table_r_0x78[] = {
	{ "TX_MAC_CALENDAR30_TABLE" },
	{ "TX_MAC_CALENDAR31_TABLE" },
	{ "TX_MAC_CALENDAR32_TABLE" },
	{ "TX_MAC_CALENDAR33_TABLE" },
	{ "TX_MAC_CALENDAR34_TABLE" },
	{ "TX_MAC_CALENDAR35_TABLE" },
	{ "TX_MAC_CALENDAR36_TABLE" },
	{ "TX_MAC_CALENDAR37_TABLE" },
	{ "TX_MAC_CALENDAR38_TABLE" },
	{ "TX_MAC_CALENDAR39_TABLE" },
	{ "TX_MAC_CALENDAR40_TABLE" },
	{ "TX_MAC_CALENDAR41_TABLE" },
	{ "TX_MAC_CALENDAR42_TABLE" },
	{ "TX_MAC_CALENDAR43_TABLE" },
	{ "TX_MAC_CALENDAR44_TABLE" },
	{ "TX_MAC_CALENDAR45_TABLE" },
	{ "TX_MAC_CALENDAR46_TABLE" },
	{ "TX_MAC_CALENDAR47_TABLE" },
	{ "TX_MAC_CALENDAR48_TABLE" },
	{ "TX_MAC_CALENDAR49_TABLE" },
	{ "TX_MAC_CALENDAR50_TABLE" },
	{ "TX_MAC_CALENDAR51_TABLE" },
	{ "TX_MAC_CALENDAR52_TABLE" },
	{ "TX_MAC_CALENDAR53_TABLE" },
	{ "TX_MAC_CALENDAR54_TABLE" },
	{ "TX_MAC_CALENDAR55_TABLE" },
	{ "TX_MAC_CALENDAR56_TABLE" },
	{ "TX_MAC_CALENDAR57_TABLE" },
	{ "TX_MAC_CALENDAR58_TABLE" },
	{ "TX_MAC_CALENDAR59_TABLE" },
};

static const struct mac_reg_name tx_mac_calendar_table_r_0xf0[] = {
	{ "TX_MAC_CALENDAR60_TABLE" },
	{ "TX_MAC_CALENDAR61_TABLE" },
	{ "TX_MAC_CALENDAR62_TABLE" },
	{ "TX_MAC_CALENDAR63_TABLE" },
	{ "TX_MAC_CALENDAR64_TABLE" },
	{ "TX_MAC_CALENDAR65_TABLE" },
	{ "TX_MAC_CALENDAR66_TABLE" },
	{ "TX_MAC_CALENDAR67_TABLE" },
	{ "TX_MAC_CALENDAR68_TABLE" },
	{ "TX_MAC_CALENDAR69_TABLE" },
	{ "TX_MAC_CALENDAR70_TABLE" },
	{ "TX_MAC_CALENDAR71_TABLE" },
	{ "TX_MAC_CALENDAR72_TABLE" },
	{ "TX_MAC_CALENDAR73_TABLE" },
	{ "TX_MAC_CALENDAR74_TABLE" },
	{ "TX_MAC_CALENDAR75_TABLE" },
	{ "TX_MAC_CALENDAR76_TABLE" },
	{ "TX_MAC_CALENDAR77_TABLE" },
	{ "TX_MAC_CALENDAR78_TABLE" },
	{ "TX_MAC_CALENDAR79_TABLE" },
};

static const struct mac_reg_name tx_mac_phy_rstn_logic_r[] = {
	{ "TX_MAC_PHY0_RSTN_LOGIC" },
	{ "TX_MAC_PHY1_RSTN_LOGIC" },
	{ "TX_MAC_PHY2_RSTN_LOGIC" },
	{ "TX_MAC_PHY3_RSTN_LOGIC" },
	{ "TX_MAC_PHY4_RSTN_LOGIC" },
	{ "TX_MAC_PHY5_RSTN_LOGIC" },
	{ "TX_MAC_PHY6_RSTN_LOGIC" },
	{ "TX_MAC_PHY7_RSTN_LOGIC" },
	{ "TX_MAC_PHY0_1588_CFG" },
	{ "TX_MAC_PHY1_1588_CFG" },
	{ "TX_MAC_PHY2_1588_CFG" },
	{ "TX_MAC_PHY3_1588_CFG" },
	{ "TX_MAC_PHY4_1588_CFG" },
	{ "TX_MAC_PHY5_1588_CFG" },
	{ "TX_MAC_PHY6_1588_CFG" },
	{ "TX_MAC_PHY7_1588_CFG" },
};

static const struct mac_reg_name tx_mac_port_rstn_logic_r[] = {
	{ "TX_MAC_PORT0_RSTN_LOGIC" },
	{ "TX_MAC_PORT1_RSTN_LOGIC" },
	{ "TX_MAC_PORT2_RSTN_LOGIC" },
	{ "TX_MAC_PORT3_RSTN_LOGIC" },
	{ "TX_MAC_PORT4_RSTN_LOGIC" },
	{ "TX_MAC_PORT5_RSTN_LOGIC" },
	{ "TX_MAC_PORT6_RSTN_LOGIC" },
	{ "TX_MAC_PORT7_RSTN_LOGIC" },
	{ "TX_MAC_PORT0_BUF_ADDR_MAP" },
	{ "TX_MAC_PORT1_BUF_ADDR_MAP" },
	{ "TX_MAC_PORT2_BUF_ADDR_MAP" },
	{ "TX_MAC_PORT3_BUF_ADDR_MAP" },
	{ "TX_MAC_PORT4_BUF_ADDR_MAP" },
	{ "TX_MAC_PORT5_BUF_ADDR_MAP" },
	{ "TX_MAC_PORT6_BUF_ADDR_MAP" },
	{ "TX_MAC_PORT7_BUF_ADDR_MAP" },
	{ "TX_MAC_PORT0_XOFF_TH" },
	{ "TX_MAC_PORT1_XOFF_TH" },
	{ "TX_MAC_PORT2_XOFF_TH" },
	{ "TX_MAC_PORT3_XOFF_TH" },
	{ "TX_MAC_PORT4_XOFF_TH" },
	{ "TX_MAC_PORT5_XOFF_TH" },
	{ "TX_MAC_PORT6_XOFF_TH" },
	{ "TX_MAC_PORT7_XOFF_TH" },
};

static const struct mac_reg_name tx_mac_port_ctrl_th_r[] = {
	{ "TX_MAC_PORT0_CTRL_TH" },
	{ "TX_MAC_PORT1_CTRL_TH" },
	{ "TX_MAC_PORT2_CTRL_TH" },
	{ "TX_MAC_PORT3_CTRL_TH" },
	{ "TX_MAC_PORT4_CTRL_TH" },
	{ "TX_MAC_PORT5_CTRL_TH" },
	{ "TX_MAC_PORT6_CTRL_TH" },
	{ "TX_MAC_PORT7_CTRL_TH" },
	{ "TX_MAC_PORT0_ENABLE" },
	{ "TX_MAC_PORT1_ENABLE" },
	{ "TX_MAC_PORT2_ENABLE" },
	{ "TX_MAC_PORT3_ENABLE" },
	{ "TX_MAC_PORT4_ENABLE" },
	{ "TX_MAC_PORT5_ENABLE" },
	{ "TX_MAC_PORT6_ENABLE" },
	{ "TX_MAC_PORT7_ENABLE" },
	{ "TX_MAC_PORT0_CONTROL" },
	{ "TX_MAC_PORT1_CONTROL" },
	{ "TX_MAC_PORT2_CONTROL" },
	{ "TX_MAC_PORT3_CONTROL" },
	{ "TX_MAC_PORT4_CONTROL" },
	{ "TX_MAC_PORT5_CONTROL" },
	{ "TX_MAC_PORT6_CONTROL" },
	{ "TX_MAC_PORT7_CONTROL" },
};

static const struct mac_reg_name tx_mac_port_control1_r[] = {
	{ "TX_MAC_PORT0_CONTROL1" },
	{ "TX_MAC_PORT1_CONTROL1" },
	{ "TX_MAC_PORT2_CONTROL1" },
	{ "TX_MAC_PORT3_CONTROL1" },
	{ "TX_MAC_PORT4_CONTROL1" },
	{ "TX_MAC_PORT5_CONTROL1" },
	{ "TX_MAC_PORT6_CONTROL1" },
	{ "TX_MAC_PORT7_CONTROL1" },
	{ "TX_MAC_PORT0_PAUSE_CTRL" },
	{ "TX_MAC_PORT1_PAUSE_CTRL" },
	{ "TX_MAC_PORT2_PAUSE_CTRL" },
	{ "TX_MAC_PORT3_PAUSE_CTRL" },
	{ "TX_MAC_PORT4_PAUSE_CTRL" },
	{ "TX_MAC_PORT5_PAUSE_CTRL" },
	{ "TX_MAC_PORT6_PAUSE_CTRL" },
	{ "TX_MAC_PORT7_PAUSE_CTRL" },
	{ "TX_MAC_PORT0_PAUSE_CTRL1" },
	{ "TX_MAC_PORT1_PAUSE_CTRL1" },
	{ "TX_MAC_PORT2_PAUSE_CTRL1" },
	{ "TX_MAC_PORT3_PAUSE_CTRL1" },
	{ "TX_MAC_PORT4_PAUSE_CTRL1" },
	{ "TX_MAC_PORT5_PAUSE_CTRL1" },
	{ "TX_MAC_PORT6_PAUSE_CTRL1" },
	{ "TX_MAC_PORT7_PAUSE_CTRL1" },
};

static const struct mac_reg_name tx_mac_port_pause_local_mac_r[] = {
	{ "TX_MAC_PORT0_PAUSE_LOCAL_MAC" },
	{ "" },
	{ "TX_MAC_PORT1_PAUSE_LOCAL_MAC" },
	{ "" },
	{ "TX_MAC_PORT2_PAUSE_LOCAL_MAC" },
	{ "" },
	{ "TX_MAC_PORT3_PAUSE_LOCAL_MAC" },
	{ "" },
	{ "TX_MAC_PORT4_PAUSE_LOCAL_MAC" },
	{ "" },
	{ "TX_MAC_PORT5_PAUSE_LOCAL_MAC" },
	{ "" },
	{ "TX_MAC_PORT6_PAUSE_LOCAL_MAC" },
	{ "" },
	{ "TX_MAC_PORT7_PAUSE_LOCAL_MAC" },
	{ "" },
};

static const struct mac_reg_name tx_mac_port_pause_peer_mac_r[] = {
	{ "TX_MAC_PORT0_PAUSE_PEER_MAC" },
	{ "" },
	{ "TX_MAC_PORT1_PAUSE_PEER_MAC" },
	{ "" },
	{ "TX_MAC_PORT2_PAUSE_PEER_MAC" },
	{ "" },
	{ "TX_MAC_PORT3_PAUSE_PEER_MAC" },
	{ "" },
	{ "TX_MAC_PORT4_PAUSE_PEER_MAC" },
	{ "" },
	{ "TX_MAC_PORT5_PAUSE_PEER_MAC" },
	{ "" },
	{ "TX_MAC_PORT6_PAUSE_PEER_MAC" },
	{ "" },
	{ "TX_MAC_PORT7_PAUSE_PEER_MAC" },
	{ "" },
};

static const struct mac_reg_name tx_mac_port_1588_ctrl_r[] = {
	{ "TX_MAC_PORT0_1588_CTRL" },
	{ "TX_MAC_PORT1_1588_CTRL" },
	{ "TX_MAC_PORT2_1588_CTRL" },
	{ "TX_MAC_PORT3_1588_CTRL" },
	{ "TX_MAC_PORT4_1588_CTRL" },
	{ "TX_MAC_PORT5_1588_CTRL" },
	{ "TX_MAC_PORT6_1588_CTRL" },
	{ "TX_MAC_PORT7_1588_CTRL" },
	{ "TX_MAC_PORT0_1588_PORT_DELAY" },
	{ "TX_MAC_PORT1_1588_PORT_DELAY" },
	{ "TX_MAC_PORT2_1588_PORT_DELAY" },
	{ "TX_MAC_PORT3_1588_PORT_DELAY" },
	{ "TX_MAC_PORT4_1588_PORT_DELAY" },
	{ "TX_MAC_PORT5_1588_PORT_DELAY" },
	{ "TX_MAC_PORT6_1588_PORT_DELAY" },
	{ "TX_MAC_PORT7_1588_PORT_DELAY" },
	{ "TX_MAC_PORT0_1588_ASYM_DELAY" },
	{ "TX_MAC_PORT1_1588_ASYM_DELAY" },
	{ "TX_MAC_PORT2_1588_ASYM_DELAY" },
	{ "TX_MAC_PORT3_1588_ASYM_DELAY" },
	{ "TX_MAC_PORT4_1588_ASYM_DELAY" },
	{ "TX_MAC_PORT5_1588_ASYM_DELAY" },
	{ "TX_MAC_PORT6_1588_ASYM_DELAY" },
	{ "TX_MAC_PORT7_1588_ASYM_DELAY" },
};

static const struct mac_reg_name tx_mac_port_test_ctrl_r[] = {
	{ "TX_MAC_PORT0_TEST_CTRL" },
	{ "TX_MAC_PORT1_TEST_CTRL" },
	{ "TX_MAC_PORT2_TEST_CTRL" },
	{ "TX_MAC_PORT3_TEST_CTRL" },
	{ "TX_MAC_PORT4_TEST_CTRL" },
	{ "TX_MAC_PORT5_TEST_CTRL" },
	{ "TX_MAC_PORT6_TEST_CTRL" },
	{ "TX_MAC_PORT7_TEST_CTRL" },
	{ "TX_MAC_PORT0_TEST_CTRL1" },
	{ "TX_MAC_PORT1_TEST_CTRL1" },
	{ "TX_MAC_PORT2_TEST_CTRL1" },
	{ "TX_MAC_PORT3_TEST_CTRL1" },
	{ "TX_MAC_PORT4_TEST_CTRL1" },
	{ "TX_MAC_PORT5_TEST_CTRL1" },
	{ "TX_MAC_PORT6_TEST_CTRL1" },
	{ "TX_MAC_PORT7_TEST_CTRL1" },
};

static const struct mac_reg_name tx_mac_port_diag_drop_octet_cnt_r[] = {
	{ "TX_MAC_PORT0_DIAG_DROP_OCTET_CNT" },
	{ "TX_MAC_PORT1_DIAG_DROP_OCTET_CNT" },
	{ "TX_MAC_PORT2_DIAG_DROP_OCTET_CNT" },
	{ "TX_MAC_PORT3_DIAG_DROP_OCTET_CNT" },
	{ "TX_MAC_PORT4_DIAG_DROP_OCTET_CNT" },
	{ "TX_MAC_PORT5_DIAG_DROP_OCTET_CNT" },
	{ "TX_MAC_PORT6_DIAG_DROP_OCTET_CNT" },
	{ "TX_MAC_PORT7_DIAG_DROP_OCTET_CNT" },
	{ "TX_MAC_PORT0_DIAG_RUNT_PKT_CNT" },
	{ "TX_MAC_PORT1_DIAG_RUNT_PKT_CNT" },
	{ "TX_MAC_PORT2_DIAG_RUNT_PKT_CNT" },
	{ "TX_MAC_PORT3_DIAG_RUNT_PKT_CNT" },
	{ "TX_MAC_PORT4_DIAG_RUNT_PKT_CNT" },
	{ "TX_MAC_PORT5_DIAG_RUNT_PKT_CNT" },
	{ "TX_MAC_PORT6_DIAG_RUNT_PKT_CNT" },
	{ "TX_MAC_PORT7_DIAG_RUNT_PKT_CNT" },
	{ "TX_MAC_PORT0_DIAG_LFRF_TERM_PKT_CNT" },
	{ "TX_MAC_PORT1_DIAG_LFRF_TERM_PKT_CNT" },
	{ "TX_MAC_PORT2_DIAG_LFRF_TERM_PKT_CNT" },
	{ "TX_MAC_PORT3_DIAG_LFRF_TERM_PKT_CNT" },
	{ "TX_MAC_PORT4_DIAG_LFRF_TERM_PKT_CNT" },
	{ "TX_MAC_PORT5_DIAG_LFRF_TERM_PKT_CNT" },
	{ "TX_MAC_PORT6_DIAG_LFRF_TERM_PKT_CNT" },
	{ "TX_MAC_PORT7_DIAG_LFRF_TERM_PKT_CNT" },
};

static const struct mac_reg_name tx_mac_port_diag_his_status_r[] = {
	{ "TX_MAC_PORT0_DIAG_HIS_STATUS" },
	{ "TX_MAC_PORT1_DIAG_HIS_STATUS" },
	{ "TX_MAC_PORT2_DIAG_HIS_STATUS" },
	{ "TX_MAC_PORT3_DIAG_HIS_STATUS" },
	{ "TX_MAC_PORT4_DIAG_HIS_STATUS" },
	{ "TX_MAC_PORT5_DIAG_HIS_STATUS" },
	{ "TX_MAC_PORT6_DIAG_HIS_STATUS" },
	{ "TX_MAC_PORT7_DIAG_HIS_STATUS" },
};

static const struct mac_reg_name tx_mac_phy_diag_1588_ref_gap_jit_max_r[] = {
	{ "TX_MAC_PHY0_DIAG_1588_REF_GAP_JIT_MAX" },
	{ "TX_MAC_PHY1_DIAG_1588_REF_GAP_JIT_MAX" },
	{ "TX_MAC_PHY2_DIAG_1588_REF_GAP_JIT_MAX" },
	{ "TX_MAC_PHY3_DIAG_1588_REF_GAP_JIT_MAX" },
	{ "TX_MAC_PHY4_DIAG_1588_REF_GAP_JIT_MAX" },
	{ "TX_MAC_PHY5_DIAG_1588_REF_GAP_JIT_MAX" },
	{ "TX_MAC_PHY6_DIAG_1588_REF_GAP_JIT_MAX" },
	{ "TX_MAC_PHY7_DIAG_1588_REF_GAP_JIT_MAX" },
};

static const struct mac_reg_info mac_type_tx_mac_blk[] = {
	{ tx_mac_calendar_table_r,			HIKP_ARRAY_SIZE(tx_mac_calendar_table_r) },
	{ tx_mac_calendar_table_r_0x78 ,		HIKP_ARRAY_SIZE(tx_mac_calendar_table_r_0x78 ) },
	{ tx_mac_calendar_table_r_0xf0,			HIKP_ARRAY_SIZE(tx_mac_calendar_table_r_0xf0) },
	{ tx_mac_phy_rstn_logic_r,			HIKP_ARRAY_SIZE(tx_mac_phy_rstn_logic_r) },
	{ tx_mac_port_rstn_logic_r,			HIKP_ARRAY_SIZE(tx_mac_port_rstn_logic_r) },
	{ tx_mac_port_ctrl_th_r,			HIKP_ARRAY_SIZE(tx_mac_port_ctrl_th_r) },
	{ tx_mac_port_control1_r,			HIKP_ARRAY_SIZE(tx_mac_port_control1_r) },
	{ tx_mac_port_pause_local_mac_r,		HIKP_ARRAY_SIZE(tx_mac_port_pause_local_mac_r) },
	{ tx_mac_port_pause_peer_mac_r,			HIKP_ARRAY_SIZE(tx_mac_port_pause_peer_mac_r) },
	{ tx_mac_port_1588_ctrl_r,			HIKP_ARRAY_SIZE(tx_mac_port_1588_ctrl_r) },
	{ tx_mac_port_test_ctrl_r,			HIKP_ARRAY_SIZE(tx_mac_port_test_ctrl_r) },
	{ tx_mac_port_diag_drop_octet_cnt_r,		HIKP_ARRAY_SIZE(tx_mac_port_diag_drop_octet_cnt_r) },
	{ tx_mac_port_diag_his_status_r,		HIKP_ARRAY_SIZE(tx_mac_port_diag_his_status_r) },
	{ tx_mac_phy_diag_1588_ref_gap_jit_max_r,	HIKP_ARRAY_SIZE(tx_mac_phy_diag_1588_ref_gap_jit_max_r) },
};

static const struct mac_type_name_parse g_mac_tx_mac_name_parse[] = {
	{ mac_type_tx_mac_reg,			HIKP_ARRAY_SIZE(mac_type_tx_mac_reg),
		ROUND_UP(mac_type_tx_mac_reg[0].reg_num, PER_BLK_DATA_SIZE), false },
	{ mac_type_tx_mac_blk,			HIKP_ARRAY_SIZE(mac_type_tx_mac_blk),
		HIKP_ARRAY_SIZE(mac_type_tx_mac_blk), true },
};

static const struct mac_reg_name mib_int_status_r[] = {
	{ "MIB_INT_STATUS" },
	{ "MIB_MEM_INIT_START" },
};

static const struct mac_reg_info mac_type_mib_reg[] = {
	{ mib_int_status_r,			HIKP_ARRAY_SIZE(mib_int_status_r) },
};

static const struct mac_reg_name mib_port_mib_control_r[] = {
	{ "MIB_PORT0_MIB_CONTROL" },
	{ "MIB_PORT1_MIB_CONTROL" },
	{ "MIB_PORT2_MIB_CONTROL" },
	{ "MIB_PORT3_MIB_CONTROL" },
	{ "MIB_PORT4_MIB_CONTROL" },
	{ "MIB_PORT5_MIB_CONTROL" },
	{ "MIB_PORT6_MIB_CONTROL" },
	{ "MIB_PORT7_MIB_CONTROL" },
};

static const struct mac_reg_info mac_type_mib_blk[] = {
	{ mib_port_mib_control_r,		HIKP_ARRAY_SIZE(mib_port_mib_control_r) },
};

static const struct mac_type_name_parse g_mac_mib_name_parse[] = {
	{ mac_type_mib_reg,			HIKP_ARRAY_SIZE(mac_type_mib_reg),
		ROUND_UP(mac_type_mib_reg[0].reg_num, PER_BLK_DATA_SIZE), false },
	{ mac_type_mib_blk,			HIKP_ARRAY_SIZE(mac_type_mib_blk),
		HIKP_ARRAY_SIZE(mac_type_mib_blk), true },
};

static const struct mac_reg_name com_tx_phy_mode_r[] = {
	{ "COM_TX_PHY0_MODE" },
	{ "COM_TX_PHY1_MODE" },
	{ "COM_TX_PHY2_MODE" },
	{ "COM_TX_PHY3_MODE" },
	{ "COM_TX_PHY4_MODE" },
	{ "COM_TX_PHY5_MODE" },
	{ "COM_TX_PHY6_MODE" },
	{ "COM_TX_PHY7_MODE" },
	{ "COM_RX_PHY0_MODE" },
	{ "COM_RX_PHY1_MODE" },
	{ "COM_RX_PHY2_MODE" },
	{ "COM_RX_PHY3_MODE" },
	{ "COM_RX_PHY4_MODE" },
	{ "COM_RX_PHY5_MODE" },
	{ "COM_RX_PHY6_MODE" },
	{ "COM_RX_PHY7_MODE" },
};

static const struct mac_reg_info mac_type_com_blk[] = {
	{ com_tx_phy_mode_r,		HIKP_ARRAY_SIZE(com_tx_phy_mode_r) },
};

static const struct mac_type_name_parse g_mac_com_name_parse[] = {
	{ mac_type_com_blk, HIKP_ARRAY_SIZE(mac_type_com_blk), HIKP_ARRAY_SIZE(mac_type_com_blk), true },
};

static const struct mac_reg_name ge_common_int_status_r[] = {
	{ "GE_COMMON_INT_STATUS" },
	{ "GE_COMMON_INT_ENABLE" },
	{ "GE_COMMON_INT_SET" },
	{ "GE_COMMON_IERR_U_INFO" },
	{ "GE_COMMON_OVF_INFO" },
	{ "GE_COMMON_UDF_INFO" },
	{ "GE_COMMON_RESET" },
	{ "GE_COMMON_LINK_CONTROL" },
	{ "GE_COMMON_PORT_MODE" },
	{ "GE_COMMON_PORT_SPEED" },
	{ "GE_COMMON_DUPLEX_MODE" },
	{ "GE_COMMON_DUPLEX_SPEED_PAUSE_SEL" },

	{ "GE_MAC_MAC_ENABLE" },
	{ "GE_MAC_MAC_CONTROL" },
	{ "GE_MAC_MAC_IPG" },
	{ "GE_MAC_MAC_PAD_SIZE" },
	{ "GE_MAC_MAC_MIN_PKT_SIZE" },
	{ "GE_MAC_MAC_MAX_PKT_SIZE" },
	{ "GE_MAC_MAC_PAUSE_CTRL" },
	{ "GE_MAC_MAC_PAUSE_TIME" },
	{ "GE_MAC_MAC_PAUSE_GAP" },
	{ "GE_MAC_MAC_PAUSE_LOCAL_MAC_H" },
	{ "GE_MAC_MAC_PAUSE_LOCAL_MAC_L" },
	{ "GE_MAC_MAC_PAUSE_PEER_MAC_H" },
	{ "GE_MAC_MAC_PAUSE_PEER_MAC_L" },
	{ "GE_MAC_MAC_PFC_PRI_EN" },
	{ "GE_MAC_MAC_1588_CTRL" },
	{ "GE_MAC_MAC_1588_TX_PORT_DLY" },
	{ "GE_MAC_MAC_1588_RX_PORT_DLY" },
	{ "GE_MAC_MAC_1588_ASYM_DLY" },
	{ "GE_MAC_MAC_Y1731_ETH_TYPE" },
	{ "GE_MAC_MAC_MIB_CONTROL" },
	{ "GE_MAC_MAC_TX_ERR_MARK" },
	{ "GE_MAC_MAC_DBG_CTRL" },
	{ "GE_MAC_MAC_EEE_CONTROL" },
	{ "GE_MAC_MAC_EEE_WAIT_TIMER" },
	{ "GE_MAC_MAC_EEE_SLEEP_TIMER" },
	{ "GE_MAC_MAC_EEE_WAKE_TIMER" },
	{ "GE_MAC_MAC_RX_RUNT_PKT_CNT" },
	{ "GE_MAC_MAC_TX_SN_MISMATCH_PKT_CNT" },
	{ "GE_MAC_MAC_ERR_INFO" },
	{ "GE_MAC_MAC_MII_CLK_SEL" },
	{ "GE_MAC_MAC_GMII_LOOPBACK" },
	{ "GE_MAC_MAC_GMII_RX_FIFO_CONTROL" },
	{ "GE_MAC_MAC_GMII_RX_FIFO_STATUS" },
	{ "GE_MAC_MAC_TX_FSM_STATUS" },
	{ "GE_MAC_MAC_RX_STATUS" },
	{ "GE_MAC_MAC_RX_FSM_STATUS" },

	{ "GE_PCS_TX_PCS_TX_FSM_STATUS0" },
	{ "GE_PCS_TX_PCS_TX_FSM_STATUS1" },

	{ "GE_PCS_RX_PCS_RX_CONTROL" },
	{ "GE_PCS_RX_PCS_RX_AN_ADV" },
	{ "GE_PCS_RX_PCS_RX_AN_NEXT_PAGE" },
	{ "GE_PCS_RX_PCS_RX_NEXT_PAGE_LOAD" },
	{ "GE_PCS_RX_PCS_RX_LINK_TIME" },
	{ "GE_PCS_RX_PCS_RX_FE_LINK_STABLE" },
	{ "GE_PCS_RX_PCS_RX_AN_SGMII_RESULT" },
	{ "GE_PCS_RX_PCS_RX_LP_ABILITY" },
	{ "GE_PCS_RX_PCS_RX_LP_NEXT_PAGE" },
	{ "GE_PCS_RX_PCS_RX_AN_RESULT" },
	{ "GE_PCS_RX_PCS_RX_AN_EXPANSION" },
	{ "GE_PCS_RX_PCS_RX_STATUS" },
	{ "GE_PCS_RX_PCS_RX_FSM_STATUS" },
	{ "GE_PCS_RX_PCS_ERR_INFO" },
	{ "GE_PCS_RX_PCS_RX_DEBUG" },

	{ "GE_PMA_PMA_ENABLE" },
	{ "GE_PMA_PMA_CONTROL" },
	{ "GE_PMA_PMA_SIGNAL_STATUS" },
};

static const struct mac_reg_info mac_type_ge_reg[] = {
	{ ge_common_int_status_r,		HIKP_ARRAY_SIZE(ge_common_int_status_r) },
};

static const struct mac_type_name_parse g_mac_ge_name_parse[] = {
	{ mac_type_ge_reg,			HIKP_ARRAY_SIZE(mac_type_ge_reg),
		ROUND_UP(mac_type_ge_reg[0].reg_num, PER_BLK_DATA_SIZE), false },
};

static const struct mac_reg_name an_tran_rc_control_addr[] = {
	{ "MAC_RD_CLR" },
	{ "GE_CLK_EN" },
	{ "CFG_REG_ACS_TIMEOUT_TH" },
	{ "CFG_APP_LOOP_AE_TH" },
	{ "CFGIF_RSTART" },
	{ "CFG_COEFUP_TIMER" },
	{ "CFG_LT_REMOTE_REJECT" },
	{ "MAC_RAM_TMOD_HIGH" },
	{ "MAC_RAM_TMOD_LOW" },
	{ "CFG_RTC_TIME_EN" },
	{ "CFG_RTC_TIME_SEL" },
};

static const struct mac_reg_info mac_type_mac_comm_reg[] = {
	{ an_tran_rc_control_addr,		HIKP_ARRAY_SIZE(an_tran_rc_control_addr) },
};

static const struct mac_reg_name spi2msgbus_dfx_addr[] = {
	{ "SPI2MSGBUS_DFX_spi2msg_num_0" },
	{ "SPI2MSGBUS_DFX_spi2msg_num_1" },
	{ "SPI2MSGBUS_DFX_spi2msg_num_2" },
	{ "SPI2MSGBUS_DFX_spi2msg_num_3" },
	{ "SPI2MSGBUS_DFX_spi2msg_num_4" },
	{ "SPI2MSGBUS_DFX_spi2msg_num_5" },
	{ "SPI2MSGBUS_DFX_spi2msg_num_6" },
	{ "SPI2MSGBUS_DFX_spi2msg_num_7" },
	{ "SPI2MSGBUS_DFX_spi2msg_num_8" },
	{ "SPI2MSGBUS_DFX_spi2msg_num_9" },
	{ "SPI2MSGBUS_DFX_spi2msg_num_10" },
	{ "SPI2MSGBUS_DFX_spi2msg_num_11" },

	{ "SPI2MSGBUS_DFX_SEL" },
	{ "MAC_CNT_CLR_CE" },
	{ "MAG_INT_ENABLE" },
	{ "MAG_INT_TYPE" },
	{ "MAG_INT_SET" },
	{ "MAG_INT_SRC" },
	{ "MAG_INT_MSIX" },
	{ "MAG_INT_CE" },
	{ "MAG_INT_NFE" },
	{ "MAG_INT_FE" },
	{ "MAG_DATABUS_DFX" },
	{ "MAG_RAM_TMOD" },
	{ "HIMAC_IF_TDM_TAB" },
	{ "MAC_ETH_ECO0" },
	{ "MAC_ETH_ECO1" },
};

static const struct mac_reg_info mac_type_mac_comm_blk[] = {
	{ spi2msgbus_dfx_addr,			HIKP_ARRAY_SIZE(spi2msgbus_dfx_addr) },
};

static const struct mac_reg_name eg_lge_credit_addr[] = {
	{ "EG_LGE_CREDIT" },
	{ "LGE_EGU_FIFO_DFX" },
	{ "CGE_EGU_AFIFO_DFX" },
	{ "AFIFO_TNL_INT_ENABLE" },
	{ "AFIFO_TNL_INT_MSIX" },
	{ "AFIFO_TNL_INT_SRC" },
	{ "MAC_LOOP_RX2TX" },
	{ "MAC_SERDES_LOS_MSK" },
	{ "AFIFO_TNL_INT_SET" },
	{ "AFIFO_TNL_INT_TYPE" },
	{ "AFIFO_TNL_INT_CE" },
	{ "AFIFO_TNL_INT_NFE" },
	{ "AFIFO_TNL_INT_FE" },
	{ "MAC_COMMON_TNL_INT_CE_TH" },
	{ "MAC_AN_ENABLE" },
	{ "MAC_TRAIN_ENABLE" },
	{ "AN_INT_ENABLE" },
	{ "AN_INT_STS" },
	{ "AN_INT_R" },
	{ "GE_IGU_AFIFO_DFX" },
	{ "GE_EGU_AFIFO_DFX" },
	{ "GE_FIFO_TH_CFG" },
	{ "GE_PAUSE_PFC_SEL" },
	{ "CFG_LGE_IGU_ASYNC_FIFO_AF_TH" },
	{ "LGE_TXFIF0_CFG" },
};

static const struct mac_reg_info mac_type_mac_comm_mac_reg[] = {
	{ eg_lge_credit_addr,			HIKP_ARRAY_SIZE(eg_lge_credit_addr) },
};

static const struct mac_type_name_parse g_mac_mac_comm_name_parse[] = {
	{ mac_type_mac_comm_reg,		HIKP_ARRAY_SIZE(mac_type_mac_comm_reg),
		ROUND_UP(mac_type_mac_comm_reg[0].reg_num, PER_BLK_DATA_SIZE), false },
	{ mac_type_mac_comm_blk,		HIKP_ARRAY_SIZE(mac_type_mac_comm_blk),
		HIKP_ARRAY_SIZE(mac_type_mac_comm_blk), true },
	{ mac_type_mac_comm_mac_reg,		HIKP_ARRAY_SIZE(mac_type_mac_comm_mac_reg),
		ROUND_UP(mac_type_mac_comm_mac_reg[0].reg_num, PER_BLK_DATA_SIZE), false },
};

static const struct mac_reg_name an_int_status_addr[] = {
	{ "AN_INT_STATUS" },
	{ "AN_INT_ENABLE" },
	{ "AN_INT_SET" },
	{ "AN_MODE_CFG" },
	{ "AN_CTRL_CFG" },
	{ "AN_TIMER_0_CFG" },
	{ "AN_TIMER_1_CFG" },
	{ "AN_TIMER_2_CFG" },
	{ "AN_TIMER_3_CFG" },
	{ "AN_TIMER_UNIT_CFG" },
	{ "AN_BP_ABILITY0_CFG" },
	{ "AN_BP_ABILITY1_CFG" },
	{ "AN_XNP_ABILITY0_CFG" },
	{ "AN_XNP_ABILITY1_CFG" },
	{ "AN_XNP_ABILITY2_CFG" },
	{ "AN_XNP_ABILITY3_CFG" },
	{ "AN_LINK_STATUS_CFG" },
	{ "AN_RX_SAMPLE_TOLERANCE_CFG" },
	{ "AN_STATUS" },
	{ "AN_LINK_CONTROL_STATUS" },
	{ "AN_LP_BP_ABILITY0_STATUS" },
	{ "AN_LP_BP_ABILITY1_STATUS" },
	{ "AN_LP_XNP_ABILITY0_STATUS" },
	{ "AN_LP_XNP_ABILITY1_STATUS" },
	{ "AN_LP_XNP_ABILITY2_STATUS" },
	{ "AN_LP_XNP_ABILITY3_STATUS" },
	{ "AN_RX_DME_STAT_CNT" },
	{ "AN_IERR_C_CNT" },
	{ "AN_IERR_U_CNT" },
	{ "AN_DBG_INFO" },
};

static const struct mac_reg_name an_dbg_timer_addr[] = {
	{ "AN_DBG_TIMER" },
	{ "AN_SPARE0" },
	{ "AN_SPARE1" },
	{ "AN_SPARE_CNT0" },
	{ "AN_SPARE_CNT1" },
};

static const struct mac_reg_info mac_type_an_reg[] = {
	{ an_int_status_addr,			HIKP_ARRAY_SIZE(an_int_status_addr) },
	{ an_dbg_timer_addr,			HIKP_ARRAY_SIZE(an_dbg_timer_addr) },
};

static const struct mac_type_name_parse g_mac_an_name_parse[] = {
	{ mac_type_an_reg, HIKP_ARRAY_SIZE(mac_type_an_reg), HIKP_ARRAY_SIZE(mac_type_an_reg), true },
};

static const struct mac_reg_name lt_int_status_addr[] = {
	{ "LT_INT_STATUS" },
	{ "LT_INT_ENABLE" },
	{ "LT_INT_SET" },
	{ "LT_CONTROL_CFG1" },
	{ "LT_CONTROL_CFG2" },
	{ "LT_SOFT_TRAIN_CFG" },
	{ "LT_HARD_TRAIN_CFG1" },
	{ "LT_HARD_TRAIN_CFG2" },
	{ "LT_HARD_TRAIN_CFG3" },
	{ "LT_MARKER_THRESHOLD_CFG" },
	{ "LT_PRE_CURSOR_CFG" },
	{ "LT_POST_CURSOR_CFG" },
	{ "LT_MAIN_CURSOR_CFG" },
	{ "LT_PRBS_CFG" },
	{ "LT_SPI_CFG" },
	{ "LT_MS_CNT_CFG" },
	{ "LT_TIMER_CFG" },
	{ "LT_DME_STATUS" },
	{ "LT_CUR_EYE_MARGIN_STATUS" },
	{ "LT_RMT_STATUS" },
	{ "LT_CUR_TAP_WEIGHT_STATUS" },
	{ "LT_TXEQ_LIMIT_STATUS" },
	{ "LT_STATUS" },
	{ "LT_DBG_INFO0" },
	{ "LT_DBG_INFO1" },
	{ "LT_DBG_INFO2" },
	{ "LT_DBG_INFO3" },
	{ "LT_DBG_INFO4" },
	{ "LT_DBG_INFO5" },
	{ "LT_DBG_INFO6" },
};

static const struct mac_reg_name lt_dme_info_addr[] = {
	{ "LT_DME_INFO" },
	{ "LT_FRAME_LOCK_FSM_INFO" },
	{ "LT_TX_CMD_INFO" },
	{ "LT_DME_CNT" },
	{ "LT_SM_DURATION0_CNT" },
	{ "LT_SM_DURATION1_CNT" },
	{ "LT_SM_TRANSFER_CNT" },
	{ "LT_SDS_TRAIN_INFO0" },
	{ "LT_SDS_TRAIN_INFO1" },
	{ "LT_SPARE0" },
	{ "LT_SPARE1" },
	{ "LT_SPARE_CNT0" },
	{ "LT_SPARE_CNT1" },
};

static const struct mac_reg_info mac_type_lt_reg[] = {
	{ lt_int_status_addr,			HIKP_ARRAY_SIZE(lt_int_status_addr) },
	{ lt_dme_info_addr,			HIKP_ARRAY_SIZE(lt_dme_info_addr) },
};

static const struct mac_type_name_parse g_mac_lt_name_parse[] = {
	{ mac_type_lt_reg, HIKP_ARRAY_SIZE(mac_type_lt_reg), HIKP_ARRAY_SIZE(mac_type_lt_reg), true },
	{ mac_type_lt_reg, HIKP_ARRAY_SIZE(mac_type_lt_reg), HIKP_ARRAY_SIZE(mac_type_lt_reg), true },
	{ mac_type_lt_reg, HIKP_ARRAY_SIZE(mac_type_lt_reg), HIKP_ARRAY_SIZE(mac_type_lt_reg), true },
	{ mac_type_lt_reg, HIKP_ARRAY_SIZE(mac_type_lt_reg), HIKP_ARRAY_SIZE(mac_type_lt_reg), true },
	{ mac_type_lt_reg, HIKP_ARRAY_SIZE(mac_type_lt_reg), HIKP_ARRAY_SIZE(mac_type_lt_reg), true },
	{ mac_type_lt_reg, HIKP_ARRAY_SIZE(mac_type_lt_reg), HIKP_ARRAY_SIZE(mac_type_lt_reg), true },
	{ mac_type_lt_reg, HIKP_ARRAY_SIZE(mac_type_lt_reg), HIKP_ARRAY_SIZE(mac_type_lt_reg), true },
	{ mac_type_lt_reg, HIKP_ARRAY_SIZE(mac_type_lt_reg), HIKP_ARRAY_SIZE(mac_type_lt_reg), true },
};

static const struct mac_dump_mod_proc g_dump_mod_proc[] = {
	{MOD_RX_MAC,        MAC_DUMP_RX_MAC_REG,        "RX_MAC",
		g_mac_rx_mac_name_parse,	HIKP_ARRAY_SIZE(g_mac_rx_mac_name_parse) },
	{MOD_RX_PCS,        MAC_DUMP_RX_PCS_REG,        "RX_PCS",
		g_mac_rx_pcs_name_parse,	HIKP_ARRAY_SIZE(g_mac_rx_pcs_name_parse) },
	{MOD_RX_RSFEC,      MAC_DUMP_RX_RSFEC_REG,      "RX_RSFEC",
		g_mac_rx_rsfec_name_parse,	HIKP_ARRAY_SIZE(g_mac_rx_rsfec_name_parse) },
	{MOD_RX_BRFEC,      MAC_DUMP_RX_BRFEC_REG,      "RX_BRFEC",
		g_mac_rx_brfec_name_parse,	HIKP_ARRAY_SIZE(g_mac_rx_brfec_name_parse) },
	{MOD_RXPMA_CORE,    MAC_DUMP_RXPMA_CORE_REG,    "RXPMA_CORE",
		g_mac_rxpma_core_name_parse,	HIKP_ARRAY_SIZE(g_mac_rxpma_core_name_parse) },
	{MOD_RXPMA_LANE,    MAC_DUMP_RXPMA_LANE_REG,    "RXPMA_LANE",
		g_mac_rxpma_lane_name_parse,	HIKP_ARRAY_SIZE(g_mac_rxpma_lane_name_parse) },
	{MOD_TXPMA_LANE,    MAC_DUMP_TXPMA_LANE_REG,    "TXPMA_LANE",
		g_mac_txpma_lane_name_parse,	HIKP_ARRAY_SIZE(g_mac_txpma_lane_name_parse) },
	{MOD_TXPMA_CORE,    MAC_DUMP_TXPMA_CORE_REG,    "TXPMA_CORE",
		g_mac_txpma_core_name_parse,	HIKP_ARRAY_SIZE(g_mac_txpma_core_name_parse) },
	{MOD_TX_BRFEC,      MAC_DUMP_TX_BRFEC_REG,      "TX_BRFEC",
		g_mac_tx_brfec_name_parse,	HIKP_ARRAY_SIZE(g_mac_tx_brfec_name_parse) },
	{MOD_TX_RSFEC,      MAC_DUMP_TX_RSFEC_REG,      "TX_RSFEC",
		g_mac_tx_rsfec_name_parse,	HIKP_ARRAY_SIZE(g_mac_tx_rsfec_name_parse) },
	{MOD_TX_PCS,        MAC_DUMP_TX_PCS_REG,        "TX_PCS",
		g_mac_tx_pcs_name_parse,	HIKP_ARRAY_SIZE(g_mac_tx_pcs_name_parse) },
	{MOD_TX_MAC,        MAC_DUMP_TX_MAC_REG,        "TX_MAC",
		g_mac_tx_mac_name_parse,	HIKP_ARRAY_SIZE(g_mac_tx_mac_name_parse) },
	{MOD_MIB,           MAC_DUMP_MIB_REG,           "MIB",
		g_mac_mib_name_parse,		HIKP_ARRAY_SIZE(g_mac_mib_name_parse) },
	{MOD_COM,           MAC_DUMP_COM_REG,           "COM",
		g_mac_com_name_parse,		HIKP_ARRAY_SIZE(g_mac_com_name_parse) },
	{MOD_GE,            MAC_DUMP_GE_REG,            "GE",
		g_mac_ge_name_parse,		HIKP_ARRAY_SIZE(g_mac_ge_name_parse) },
	{MOD_MAC_COMM,      MAC_DUMP_MAC_COMM_REG,      "MAC_COMM",
		g_mac_mac_comm_name_parse,	HIKP_ARRAY_SIZE(g_mac_mac_comm_name_parse) },
	{MOD_AN,            MAC_DUMP_AN_REG,            "AN",
		g_mac_an_name_parse,		HIKP_ARRAY_SIZE(g_mac_an_name_parse) },
	{MOD_LT,            MAC_DUMP_LT_REG,            "LT",
		g_mac_lt_name_parse,		HIKP_ARRAY_SIZE(g_mac_lt_name_parse) },
};

static const struct mac_reg_name *hikp_nic_mac_get_reg_list(uint32_t cur_blk_id, uint32_t reg_num,
							    const struct mac_dump_mod_proc *mac_dump_info,
							    uint32_t *lane_index)
{
	const struct mac_reg_name *reg_list;
	uint32_t reg_index = 0;
	uint32_t cur_cnt = 0;
	uint32_t hikp_reg_num;
	uint32_t i;

	for (i = 0; i < mac_dump_info->mac_name_parse_size; i++) {
		cur_cnt += mac_dump_info->mac_name_parse[i].blk_size;
		if (cur_blk_id >= cur_cnt)
			continue;
		reg_index = cur_blk_id + mac_dump_info->mac_name_parse[i].blk_size - cur_cnt;
		*lane_index = i;

		if (mac_dump_info->mac_name_parse[i].is_blk == true) {
			reg_list = mac_dump_info->mac_name_parse[i].reg_info_list[reg_index].reg_name;
			hikp_reg_num = mac_dump_info->mac_name_parse[i].reg_info_list[reg_index].reg_num;
			return hikp_reg_num == reg_num ? reg_list : NULL;
		} else {
			reg_list = mac_dump_info->mac_name_parse[i].reg_info_list[0].reg_name
					+ PER_BLK_DATA_SIZE * reg_index;
			hikp_reg_num = mac_dump_info->mac_name_parse[i].reg_info_list[0].reg_num;
			return hikp_reg_num - PER_BLK_DATA_SIZE * reg_index >= reg_num ? reg_list : NULL;
		}
	}
	return NULL;
}

static int mac_dump_module_reg(struct major_cmd_ctrl *self, uint32_t cur_blk_id, uint32_t sub_code, uint32_t module_id)
{
	const struct mac_reg_name *reg_list;
	struct reg_rsp_info *rsp_data = NULL;
	struct dump_reg_req req_data = {0};
	struct hikp_cmd_ret *cmd_ret = NULL;
	struct hikp_cmd_header req_header = {0};
	uint32_t lane_index = 0;
	uint32_t rsp_reg_num;
	uint32_t i;

	req_data.bdf = g_dump_reg_info.target.bdf;
	req_data.blk_id = cur_blk_id;

	hikp_cmd_init(&req_header, MAC_MOD, MAC_CMD_DUMP_REG, sub_code);
	cmd_ret = hikp_cmd_alloc(&req_header, &req_data, sizeof(req_data));
	if (!cmd_ret) {
		HIKP_ERROR_PRINT("dump mac sub_code:0x%x reg failed.\n", sub_code);
		self->err_no = -ENOSPC;
		return self->err_no;
	}
	if (cmd_ret->status != 0) {
		hikp_cmd_free(&cmd_ret);
		self->err_no = -EINVAL;
		return self->err_no;
	}

	rsp_data = (struct reg_rsp_info *)(cmd_ret->rsp_data);
	rsp_reg_num = cmd_ret->rsp_data_num >> 1U;
	reg_list = hikp_nic_mac_get_reg_list(cur_blk_id, rsp_reg_num, &g_dump_mod_proc[module_id], &lane_index);

	for (i = 0; i < rsp_reg_num; i++) {
		if (reg_list == NULL) {
			printf("%-40s\t[0x%04x] :\t0x%012lx\n", "", rsp_data->addr, (uint64_t)rsp_data->val);
		} else if (sub_code == MAC_DUMP_LT_REG) {
			printf("%s%d %-33s\t[0x%04x] :\t0x%012lx\n", "lane_", lane_index, reg_list->name,
				rsp_data->addr,(uint64_t)rsp_data->val);
			reg_list++;
		} else {
			printf("%-40s\t[0x%04x] :\t0x%012lx\n", reg_list->name, rsp_data->addr,
				(uint64_t)rsp_data->val);
			reg_list++;
		}

		rsp_data++;
	}

	hikp_cmd_free(&cmd_ret);

	return 0;
}

static int mac_cmd_dump_mod(struct major_cmd_ctrl *self, const char *name,
			    uint32_t sub_code, uint32_t blk_num, uint32_t module_id)
{
	uint32_t i;
	int ret;

	if (blk_num == 0) {
		printf("%s module is not support dump.\n", name);
		return 0;
	}

	printf("================================= %10s REG INFO =================================\n", name);
	printf("%-40s\t %s  :\t%10s\n", "name", "offset", "value");

	for (i = 0; i < blk_num; i++) {
		ret = mac_dump_module_reg(self, i, sub_code, module_id);
		if (ret != 0)
			return ret;
	}

	return 0;
}

static void mac_cmd_dump_all(struct major_cmd_ctrl *self)
{
	size_t size = HIKP_ARRAY_SIZE(g_dump_mod_proc);
	size_t i;
	int ret;

	for (i = 0; i < size; i++) {
		ret = mac_cmd_dump_mod(self, g_dump_mod_proc[i].name, g_dump_mod_proc[i].sub_cmd,
				       g_dump_reg_info.blk_num[g_dump_mod_proc[i].module_id],
				       g_dump_mod_proc[i].module_id);
		if (ret != 0) {
			HIKP_ERROR_PRINT("dump module name:%s reg failed.\n",
					 g_dump_mod_proc[i].name);
			return;
		}
	}
}

static void mac_cmd_dump_module(struct major_cmd_ctrl *self, const char *module_name)
{
	size_t size = HIKP_ARRAY_SIZE(g_dump_mod_proc);
	size_t i;
	int ret;

	for (i = 0; i < size; i++) {
		if (strcmp(g_dump_mod_proc[i].name, module_name) != 0)
			continue;

		ret = mac_cmd_dump_mod(self, g_dump_mod_proc[i].name, g_dump_mod_proc[i].sub_cmd,
				       g_dump_reg_info.blk_num[g_dump_mod_proc[i].module_id],
				       g_dump_mod_proc[i].module_id);
		if (ret != 0)
			HIKP_ERROR_PRINT("dump module name:%s reg failed.\n", module_name);
		return;
	}

	HIKP_ERROR_PRINT("invalid module name:%s.\n", module_name);
}

static int mac_cmd_get_dump_blk_num(struct major_cmd_ctrl *self)
{
	struct hikp_cmd_ret *cmd_ret = NULL;
	struct hikp_cmd_header req_header = {0};

	hikp_cmd_init(&req_header, MAC_MOD, MAC_CMD_DUMP_REG, MAC_DUMP_GET_BLK_NUM);
	cmd_ret = hikp_cmd_alloc(&req_header, &g_dump_reg_info.target.bdf,
				 sizeof(g_dump_reg_info.target.bdf));
	if (!cmd_ret) {
		HIKP_ERROR_PRINT("nic_mac get reg blk num failed.\n");
		self->err_no = -ENOSPC;
		return self->err_no;
	}

	if (cmd_ret->status != 0 || cmd_ret->rsp_data_num < MOD_ID_MAX) {
		HIKP_ERROR_PRINT("nic_mac reg blk num error, rsp_num:%u\n", cmd_ret->rsp_data_num);
		self->err_no = -EINVAL;
		hikp_cmd_free(&cmd_ret);
		return self->err_no;
	}

	memcpy(g_dump_reg_info.blk_num, cmd_ret->rsp_data, sizeof(g_dump_reg_info.blk_num));

	hikp_cmd_free(&cmd_ret);

	return 0;
}

static int mac_cmd_dump_para_check(struct major_cmd_ctrl *self)
{
	if (!g_dump_reg_info.port_flag) {
		self->err_no = -EINVAL;
		snprintf(self->err_str, sizeof(self->err_str), "Need port id.");
		return self->err_no;
	}

	if (!g_dump_reg_info.module_name) {
		self->err_no = -EINVAL;
		snprintf(self->err_str, sizeof(self->err_str), "Need module name.");
		return self->err_no;
	}

	return 0;
}

void mac_cmd_dump_execute(struct major_cmd_ctrl *self)
{
	int ret;

	ret = mac_cmd_dump_para_check(self);
	if (ret != 0)
		return;

	/* first:get blk num */
	ret = mac_cmd_get_dump_blk_num(self);
	if (ret != 0)
		return;

	if (strcmp(g_dump_reg_info.module_name, "ALL") == 0)
		mac_cmd_dump_all(self);
	else
		mac_cmd_dump_module(self, g_dump_reg_info.module_name);
}

int mac_cmd_dump_reg_target(struct major_cmd_ctrl *self, const char *argv)
{
	self->err_no = tool_check_and_get_valid_bdf_id(argv, &g_dump_reg_info.target);
	if (self->err_no) {
		snprintf(self->err_str, sizeof(self->err_str), "Unknown device %s.", argv);
		return self->err_no;
	}
	g_dump_reg_info.port_flag = true;

	return 0;
}

static int mac_cmd_dump_reg_help(struct major_cmd_ctrl *self, const char *argv)
{
	HIKP_SET_USED(argv);

	printf("\n  Usage: %s %s\n", self->cmd_ptr->name, "-i <interface> -m <module>\n");
	printf("\n         %s\n", self->cmd_ptr->help_info);
	printf("  Options:\n\n");
	printf("    %s, %-25s %s\n", "-h", "--help", "display this help and exit");
	printf("    %s, %-25s %s\n", "-i", "--interface=<interface>", "device target, e.g. eth0~3");

	printf("    %s, %-25s %s\n", "-m", "--module=<module>",
	       "input the mac key reg module name, e.g:");
	printf("                                  %s\n",
	       "ALL/RX_MAC/RX_PCS/RX_RSFEC/RX_BRFEC/RXPMA_CORE/RXPMA_LANE/TXPMA_LANE");
	printf("                                  %s\n",
	       "TXPMA_CORE/TX_BRFEC/TX_RSFEC/TX_PCS/TX_MAC/MIB/COM/GE/MAC_COMM/AN/LT");

	return 0;
}

int mac_cmd_dump_module_cfg(struct major_cmd_ctrl *self, const char *argv)
{
	HIKP_SET_USED(self);

	g_dump_reg_info.module_name = argv;

	return 0;
}

int hikp_info_collect_nic_mac(void *data)
{
	struct nic_mac_collect_param *param = (struct nic_mac_collect_param *)data;
	struct major_cmd_ctrl *major_cmd = get_major_cmd();
	int ret;

	memset(&g_dump_reg_info, 0, sizeof(g_dump_reg_info));

	ret = mac_cmd_dump_reg_target(major_cmd, param->net_dev_name);
	if (ret)
		return ret;

	ret = mac_cmd_dump_module_cfg(major_cmd, param->module_name);
	if (ret)
		return ret;

	printf("hikptool nic_mac -i %s -m %s\n", param->net_dev_name, param->module_name);
	mac_cmd_dump_execute(major_cmd);

	return ret;
}

static void cmd_mac_dump_reg_init(void)
{
	struct major_cmd_ctrl *major_cmd = get_major_cmd();

	g_dump_reg_info.port_flag = false;

	major_cmd->option_count = 0;
	major_cmd->execute = mac_cmd_dump_execute;

	cmd_option_register("-h", "--help",       false,    mac_cmd_dump_reg_help);
	cmd_option_register("-i", "--interface",  true,     mac_cmd_dump_reg_target);
	cmd_option_register("-m", "--module",     true,     mac_cmd_dump_module_cfg);
}

HIKP_CMD_DECLARE("nic_mac", "dump mac module reg information", cmd_mac_dump_reg_init);
