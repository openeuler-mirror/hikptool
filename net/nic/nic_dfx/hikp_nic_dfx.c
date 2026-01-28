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
#include <sys/mman.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <unistd.h>
#include <inttypes.h>
#include "tool_cmd.h"
#include "hikp_net_lib.h"
#include "hikp_nic_dfx.h"

#define dfx_get_max_reg_bffer_size(rsp_head) \
	(uint32_t)((rsp_head)->total_blk_num * MAX_DFX_DATA_NUM * sizeof(uint32_t))

struct nic_dfx_param g_dfx_param = { 0 };

static const struct dfx_type_parse g_dfx_type_parse[] = {
	{INCORRECT_REG_TYPE, WIDTH_32_BIT, "INCORRECT TYPE"},
	{TYPE_32_RX_ERROR_STATS, WIDTH_32_BIT, "32 bit RX error statistics"},
	{TYPE_32_RX_DROP_STATS, WIDTH_32_BIT, "32 bit RX drop statistics"},
	{TYPE_32_RX_NORMAL_STATS, WIDTH_32_BIT, "32 bit RX statistics"},
	{TYPE_32_TX_ERROR_STATS, WIDTH_32_BIT, "32 bit TX error statistics"},
	{TYPE_32_TX_DROP_STATS, WIDTH_32_BIT, "32 bit TX drop statistics"},
	{TYPE_32_TX_NORMAL_STATS, WIDTH_32_BIT, "32 bit TX statistics"},
	{TYPE_32_ERROR_STATUS, WIDTH_32_BIT, "32 bit error status"},
	{TYPE_32_RUNNING_STATUS, WIDTH_32_BIT, "32 bit running status"},
	{TYPE_32_CFG_STATUS, WIDTH_32_BIT, "32 bit config status"},
	{TYPE_32_RX_PORT_ERROR_STATS, WIDTH_32_BIT, "32 bit RX port error statistics"},
	{TYPE_32_RX_PORT_DROP_STATS, WIDTH_32_BIT, "32 bit RX port drop statistics"},
	{TYPE_32_RX_PORT_NORMAL_STATS, WIDTH_32_BIT, "32 bit RX port statistics"},
	{TYPE_32_TX_PORT_ERROR_STATS, WIDTH_32_BIT, "32 bit TX port error statistics"},
	{TYPE_32_TX_PORT_DROP_STATS, WIDTH_32_BIT, "32 bit TX port drop statistics"},
	{TYPE_32_TX_PORT_NORMAL_STATS, WIDTH_32_BIT, "32 bit TX port statistics"},
	{TYPE_32_PORT_ERROR_STATUS, WIDTH_32_BIT, "32 bit port error status"},
	{TYPE_32_PORT_RUNNING_STATUS, WIDTH_32_BIT, "32 bit port running status"},
	{TYPE_32_PORT_CFG_STATUS, WIDTH_32_BIT, "32 bit port config status"},
	{TYPE_32_COMM_STATS, WIDTH_32_BIT, "32 bit common statistics"},
	{TYPE_32_COMM_DROP_STATS, WIDTH_32_BIT, "32 bit common drop statistics"},
	{TYPE_32_COMM_ERROR_STATS, WIDTH_32_BIT, "32 bit common error statistics"},
	{TYPE_64_RX_ERROR_STATS, WIDTH_64_BIT, "64 bit RX error statistics"},
	{TYPE_64_RX_DROP_STATS, WIDTH_64_BIT, "64 bit RX drop statistics"},
	{TYPE_64_RX_NORMAL_STATS, WIDTH_64_BIT, "64 bit RX statistics"},
	{TYPE_64_TX_ERROR_STATS, WIDTH_64_BIT, "64 bit TX error statistics"},
	{TYPE_64_TX_DROP_STATS, WIDTH_64_BIT, "64 bit TX drop statistics"},
	{TYPE_64_TX_NORMAL_STATS, WIDTH_64_BIT, "64 bit TX statistics"},
	{TYPE_64_RX_PORT_ERROR_STATS, WIDTH_64_BIT, "64 bit RX port error statistics"},
	{TYPE_64_RX_PORT_DROP_STATS, WIDTH_64_BIT, "64 bit RX port drop statistics"},
	{TYPE_64_RX_PORT_NORMAL_STATS, WIDTH_64_BIT, "64 bit RX port statistics"},
	{TYPE_64_TX_PORT_ERROR_STATS, WIDTH_64_BIT, "64 bit TX port error statistics"},
	{TYPE_64_TX_PORT_DROP_STATS, WIDTH_64_BIT, "64 bit TX port drop statistics"},
	{TYPE_64_TX_PORT_NORMAL_STATS, WIDTH_64_BIT, "64 bit TX port statistics"},
	{TYPE_64_COMM_STATS, WIDTH_64_BIT, "64 bit common statistics"},
	{TYPE_64_COMM_DROP_STATS, WIDTH_64_BIT, "64 bit common drop statistics"},
	{TYPE_64_COMM_ERROR_STATS, WIDTH_64_BIT, "64 bit common error statistics"},
	{TYPE_64_TX_PF_ERROR_STATS, WIDTH_64_BIT, "64 bit TX pf error statistics"},
	{TYPE_64_TX_PF_DROP_STATS, WIDTH_64_BIT, "64 bit TX pf drop statistics"},
	{TYPE_64_TX_PF_NORMAL_STATS, WIDTH_64_BIT, "64 bit TX pf statistics"},
};

static const struct dfx_reg_name dfx_ppp_type_32_common_drop_stats1[] = {
	{ "DROP_FROM_PRT_PKT_CNT"},
	{ "DROP_FROM_HOST_PKT_CNT"},
	{ "DROP_TX_VLAN_PROC_CNT"},
	{ "DROP_MNG_CNT"},
	{ "DROP_FD_CNT"},
	{ "DROP_NO_DST_CNT"},
	{ "DROP_MC_MBID_FULL_CNT"},
	{ "DROP_SC_FILTERED"},
};

static const struct dfx_reg_name dfx_ppp_type_32_common_drop_stats2[] = {
	{ "PPP_MC_DROP_PKT_CNT"},
	{ "DROP_PT_CNT"},
	{ "DROP_MAC_ANTI_SPOOF_CNT"},
	{ "DROP_IG_VFV_CNT"},
	{ "DROP_IG_PRTV_CNT"},
	{ "DROP_CNM_PFC_PAUSE_CNT"},
	{ "DROP_TORUS_TC_CNT"},
	{ "DROP_TORUS_LPBK_CNT"},
	{ "FWD_BONDING_PRT_EG_VLAN_DROP_CNT"},
	{ "UMV_UPLINK_EG_VLAN_DROP_CNT"},
	{ "BONDING_UPLINK_VLAN_FILTER_FAIL_CNT"},
	{ "PROMIS_PRUNE_DROP_CNT"},
	{ "UMV_UC_SRC_PRUNE_DROP_CNT"},
	{ "PPP_GRO_DROP_CNT"},
};

static const struct dfx_reg_name dfx_ppp_type_32_rx_normal_stats[] = {
	{ "PPP_GRO_KEY_CNT" },
	{ "PPP_GRO_INFO_CNT" },
	{ "PPP_GRO_OUT_CNT" },
	{ "PPP_GRO_KEY_MATCH_DATA_CNT" },
	{ "PPP_GRO_KEY_MATCH_TCAM_CNT" },
	{ "PPP_GRO_INFO_MATCH_CNT" },
};

static const struct dfx_reg_name dfx_ppp_type_64_rx_normal_stats1[] = {
	{ "GET_RX_PKT_CNT" },
	{ "SEND_UC_PRT2HOST_PKT_CNT" },
	{ "SEND_UC_PRT2PRT_PKT_CNT" },
	{ "SEND_MC_FROM_PRT_CNT" },
	{ "SSU_MC_RD_CNT" },
	{ "SSU_MC_DROP_CNT" },
	{ "SSU_MC_RD_PKT_CNT" },
	{ "PPP_MC_2HOST_PKT_CNT" },
	{ "NR_PKT_CNT" },
	{ "RR_PKT_CNT" },
	{ "MNG_TBL_HIT_CNT" },
	{ "" },
	{ "FD_TBL_HIT_CNT" },
	{ "UM_TBL_UC_HIT_CNT" },
	{ "UM_TBL_MC_HIT_CNT" },
	{ "UM_TBL_MC_HIT_PKT_CNT" },
	{ "UM_TBL_VMDQ1_HIT_CNT" },
	{ "MTA_TBL_HIT_CNT" },
	{ "MTA_TBL_HIT_PKT_CNT" },
	{ "FWD_BONDING_HIT_CNT" },
	{ "GET_BMC_PKT_CNT" },
	{ "" },
	{ "SEND_UC_PRT2BMC_PKT_CNT" },
	{ "PROMIS_TBL_HIT_CNT" },
	{ "PROMIS_TBL_HIT_PKT_CNT" },
	{ "" },
};

static const struct dfx_reg_name dfx_ppp_type_64_rx_normal_stats2[] = {
	{ "SEND_UC_BMC2PRT_PKT_CNT" },
	{ "RX_DEFAULT_HOST_HIT_CNT" },
	{ "LAN_PAIR_CNT" },
};

static const struct dfx_reg_name dfx_ppp_type_64_tx_normal_stats1[] = {
	{ "GET_TX_PKT_CNT" },
	{ "SEND_UC_HOST2HOST_PKT_CNT" },
	{ "SEND_UC_HOST2PRT_PKT_CNT" },
	{ "SEND_MC_FROM_HOST_CNT" },
	{ "PPP_MC_2PRT_PKT_CNT" },
	{ "NTSNOS_PKT_CNT" },
	{ "NTUP_PKT_CNT" },
	{ "NTLCL_PKT_CNT" },
	{ "NTTGT_PKT_CNT" },
	{ "RTNS_PKT_CNT" },
	{ "RTLPBK_PKT_CNT" },
	{ "BC_HIT_CNT" },
	{ "SEND_UC_HOST2BMC_PKT_CNT" },
};

static const struct dfx_reg_name dfx_ppp_type_64_tx_normal_stats2[] = {
	{ "PPP_MC_2BMC_PKT_CNT" },
};

static const struct dfx_reg_name dfx_ppp_type_64_tx_drop_stats[] = {
	{ "PPP_TX_TAG_DROP_CNT_0" },
	{ "PPP_TX_TAG_DROP_CNT_1" },
	{ "PPP_TX_TAG_DROP_CNT_2" },
	{ "PPP_TX_TAG_DROP_CNT_3" },
};

static const struct dfx_reg_name dfx_ppp_type_32_running_stats[] = {
	{ "PPP_HFS_STS" },
	{ "PPP_MC_RSLT_STS" },
	{ "PPP_RSLT_DESCR_STS" },
	{ "PPP_UMV_STS_0" },
	{ "PPP_UMV_STS_1" },
	{ "PPP_VFV_STS" },
};

static const struct dfx_type_name_parse g_dfx_ppp_name_parse[] = {
	{ TYPE_32_COMM_DROP_STATS, dfx_ppp_type_32_common_drop_stats1,
		HIKP_ARRAY_SIZE(dfx_ppp_type_32_common_drop_stats1), NULL},
	{ TYPE_32_COMM_DROP_STATS, dfx_ppp_type_32_common_drop_stats2,
		HIKP_ARRAY_SIZE(dfx_ppp_type_32_common_drop_stats2), NULL},
	{ TYPE_32_RX_NORMAL_STATS, dfx_ppp_type_32_rx_normal_stats,
		HIKP_ARRAY_SIZE(dfx_ppp_type_32_rx_normal_stats), NULL},
	{ TYPE_64_RX_NORMAL_STATS, dfx_ppp_type_64_rx_normal_stats1,
		HIKP_ARRAY_SIZE(dfx_ppp_type_64_rx_normal_stats1), NULL},
	{ TYPE_64_RX_NORMAL_STATS, dfx_ppp_type_64_rx_normal_stats2,
		HIKP_ARRAY_SIZE(dfx_ppp_type_64_rx_normal_stats2), NULL},
	{ TYPE_64_TX_NORMAL_STATS, dfx_ppp_type_64_tx_normal_stats1,
		HIKP_ARRAY_SIZE(dfx_ppp_type_64_tx_normal_stats1), NULL},
	{ TYPE_64_TX_NORMAL_STATS, dfx_ppp_type_64_tx_normal_stats2,
		HIKP_ARRAY_SIZE(dfx_ppp_type_64_tx_normal_stats2), NULL},
	{ TYPE_64_TX_DROP_STATS, dfx_ppp_type_64_tx_drop_stats,
		HIKP_ARRAY_SIZE(dfx_ppp_type_64_tx_drop_stats), NULL},
	{ TYPE_32_RUNNING_STATUS, dfx_ppp_type_32_running_stats,
		HIKP_ARRAY_SIZE(dfx_ppp_type_32_running_stats), NULL},
};

static const struct dfx_reg_name dfx_ssu_type_32_rx_drop_stats[] = {
	{ "RX_OQ_DROP_PKT_CNT" },
	{ "RX_OQ_GLB_DROP_PKT_CNT" },
};

static const struct dfx_reg_name dfx_ssu_type_32_rx_normal_stats[] = {
	{ "NCSI_RX_PACKET_IN_CNT" },
};

static const struct dfx_reg_name dfx_ssu_type_32_tx_drop_stats[] = {
	{ "TX_OQ_DROP_PKT_CNT" },
};

static const struct dfx_reg_name dfx_ssu_type_32_tx_normal_stats[] = {
	{ "NCSI_TX_PACKET_OUT_CNT" },
};

static const struct dfx_reg_name dfx_ssu_type_32_rx_port_drop_stats1[] = {
	{ "RX_FULL_DROP_NUM" },
	{ "RX_PART_DROP_NUM" },
	{ "ROCE_RX_BYPASS_5NS_DROP_NUM" },
};

static const struct dfx_reg_name dfx_ssu_type_32_rx_port_drop_stats2[] = {
	{ "RX_OQ_GLB_DROP_PKT_CNT_PORT" },
};

static const struct dfx_reg_name dfx_ssu_type_32_tx_port_drop_stats1[] = {
	{ "TX_FULL_DROP_NUM" },
	{ "TX_PART_DROP_NUM" },
};

static const struct dfx_reg_name dfx_ssu_type_32_tx_port_drop_stats2[] = {
	{ "TX_OQ_GLB_DROP_PKT_CNT_PORT" },
};

static const struct dfx_reg_name dfx_ssu_type_32_running_stats[] = {
	{ "SSU_BP_STATUS_0" },
	{ "SSU_BP_STATUS_1" },
	{ "SSU_BP_STATUS_2" },
	{ "SSU_BP_STATUS_3" },
	{ "SSU_BP_STATUS_4" },
	{ "SSU_BP_STATUS_5" },
	{ "SSU_MAC_TX_PFC_IND" },
	{ "MAC_SSU_RX_PFC_IND" },
	{ "ROH_SSU_PFC" },
	{ "SSU_ETS_PORT_STATUS" },
	{ "SSU_ETS_TCG_STATUS" },
	{ "BTMP_AGEING_ST" },
};

static const struct dfx_reg_name dfx_ssu_type_32_port_running_stats[] = {
	{ "PACKET_TC_0_CURR_BUFFER_CNT" },
	{ "PACKET_TC_1_CURR_BUFFER_CNT" },
	{ "PACKET_TC_2_CURR_BUFFER_CNT" },
	{ "PACKET_TC_3_CURR_BUFFER_CNT" },
	{ "PACKET_TC_4_CURR_BUFFER_CNT" },
	{ "PACKET_TC_5_CURR_BUFFER_CNT" },
	{ "PACKET_TC_6_CURR_BUFFER_CNT" },
	{ "PACKET_TC_7_CURR_BUFFER_CNT" },
};

static const struct dfx_reg_name dfx_ssu_type_32_comm_stats[] = {
	{ "LO_PRI_UNICAST_CUR_CNT" },
	{ "HI_PRI_MULTICAST_CUR_CNT" },
	{ "LO_PRI_MULTICAST_CUR_CNT" },
};

static const struct dfx_reg_name dfx_ssu_type_32_comm_drop_stats[] = {
	{ "FULL_DROP_NUM" },
	{ "PART_DROP_NUM" },
	{ "PPP_KEY_DROP_NUM" },
	{ "PPP_RLT_DROP_NUM" },
	{ "MB_UNCOPY_NUM" },
	{ "LO_PRI_UNICAST_RLT_DROP_NUM" },
	{ "HI_PRI_MULTICAST_RLT_DROP_NUM" },
	{ "LO_PRI_MULTICAST_RLT_DROP_NUM" },
	{ "NIC_L2_ERR_DROP_PKT_CNT" },
	{ "ROC_L2_ERR_DROP_PKT_CNT" },
	{ "BANK_UNBALANCE_DROP_CNT" },
	{ "SSU_MB_RD_RLT_DROP_CNT" },
	{ "NCSI_FULL_DROP_NUM" },
	{ "NCSI_PART_DROP_NUM" },
	{ "NCSI_OQ_GLB_DROP_PKT_CNT_PORT" },
	{ "LO_MB_DROP_FOR_CNT_MEM_EMPTY" },
	{ "HI_MB_DROP_FOR_CNT_MEM_EMPTY" },
	{ "SSU_OVERSIZE_DROP_NUM" },
};

static const struct dfx_reg_name dfx_ssu_type_32_comm_err_stats[] = {
	{ "SSU_ECC_1BIT_ERR_CNT" },
	{ "SSU_ECC_MULTI_BIT_ERR_CNT" },
};

static const struct dfx_reg_name dfx_ssu_type_64_rx_port_drop_stats[] = {
	{ "RX_PACKET_IN_ERR_CNT" },
	{ "RX_PACKET_OUT_ERR_CNT" },
};

static const struct dfx_reg_name dfx_ssu_type_64_tx_port_drop_stats[] = {
	{ "TX_PACKET_IN_ERR_CNT" },
	{ "TX_PACKET_OUT_ERR_CNT" },
};

static const struct dfx_reg_name dfx_ssu_type_64_rx_port_normal_stats[] = {
	{ "RX_PACKET_IN_CNT" },
	{ "RX_PACKET_OUT_CNT" },
	{ "RX_PACKET_TC_0_IN_CNT" },
	{ "RX_PACKET_TC_1_IN_CNT" },
	{ "RX_PACKET_TC_2_IN_CNT" },
	{ "RX_PACKET_TC_3_IN_CNT" },
	{ "RX_PACKET_TC_4_IN_CNT" },
	{ "RX_PACKET_TC_5_IN_CNT" },
	{ "RX_PACKET_TC_6_IN_CNT" },
	{ "RX_PACKET_TC_7_IN_CNT" },
	{ "RX_PACKET_TC_0_OUT_CNT" },
	{ "RX_PACKET_TC_1_OUT_CNT" },
	{ "RX_PACKET_TC_2_OUT_CNT" },
	{ "RX_PACKET_TC_3_OUT_CNT" },
	{ "RX_PACKET_TC_4_OUT_CNT" },
	{ "RX_PACKET_TC_5_OUT_CNT" },
	{ "RX_PACKET_TC_6_OUT_CNT" },
	{ "RX_PACKET_TC_7_OUT_CNT" },
	{ "ROC_RX_PACKET_IN_CNT" },
};

static const struct dfx_reg_name dfx_ssu_type_64_tx_port_normal_stats[] = {
	{ "TX_PACKET_IN_CNT" },
	{ "TX_PACKET_OUT_CNT" },
	{ "TX_PACKET_TC_0_IN_CNT" },
	{ "TX_PACKET_TC_1_IN_CNT" },
	{ "TX_PACKET_TC_2_IN_CNT" },
	{ "TX_PACKET_TC_3_IN_CNT" },
	{ "TX_PACKET_TC_4_IN_CNT" },
	{ "TX_PACKET_TC_5_IN_CNT" },
	{ "TX_PACKET_TC_6_IN_CNT" },
	{ "TX_PACKET_TC_7_IN_CNT" },
	{ "TX_PACKET_TC_0_OUT_CNT" },
	{ "TX_PACKET_TC_1_OUT_CNT" },
	{ "TX_PACKET_TC_2_OUT_CNT" },
	{ "TX_PACKET_TC_3_OUT_CNT" },
	{ "TX_PACKET_TC_4_OUT_CNT" },
	{ "TX_PACKET_TC_5_OUT_CNT" },
	{ "TX_PACKET_TC_6_OUT_CNT" },
	{ "TX_PACKET_TC_7_OUT_CNT" },
	{ "ROC_TX_PACKET_OUT_CNT" },
};

static const struct dfx_reg_name dfx_ssu_type_64_tx_comm_stats[] = {
	{ "SSU_PPP_MAC_KEY_NUM" },
	{ "SSU_PPP_HOST_KEY_NUM" },
	{ "PPP_SSU_MAC_RLT_NUM" },
	{ "PPP_SSU_HOST_RLT_NUM" },
};

static const struct dfx_type_name_parse g_dfx_ssu_name_parse[] = {
	{ TYPE_32_RX_DROP_STATS, dfx_ssu_type_32_rx_drop_stats,
		HIKP_ARRAY_SIZE(dfx_ssu_type_32_rx_drop_stats), NULL},
	{ TYPE_32_RX_NORMAL_STATS, dfx_ssu_type_32_rx_normal_stats,
		HIKP_ARRAY_SIZE(dfx_ssu_type_32_rx_normal_stats), NULL},
	{ TYPE_32_TX_DROP_STATS, dfx_ssu_type_32_tx_drop_stats,
		HIKP_ARRAY_SIZE(dfx_ssu_type_32_tx_drop_stats), NULL},
	{ TYPE_32_TX_NORMAL_STATS, dfx_ssu_type_32_tx_normal_stats,
		HIKP_ARRAY_SIZE(dfx_ssu_type_32_tx_normal_stats), NULL},
	{ TYPE_32_RX_PORT_DROP_STATS, dfx_ssu_type_32_rx_port_drop_stats1,
		HIKP_ARRAY_SIZE(dfx_ssu_type_32_rx_port_drop_stats1), NULL},
	{ TYPE_32_RX_PORT_DROP_STATS, dfx_ssu_type_32_rx_port_drop_stats2,
		HIKP_ARRAY_SIZE(dfx_ssu_type_32_rx_port_drop_stats2), NULL},
	{ TYPE_32_TX_PORT_DROP_STATS, dfx_ssu_type_32_tx_port_drop_stats1,
		HIKP_ARRAY_SIZE(dfx_ssu_type_32_tx_port_drop_stats1), NULL},
	{ TYPE_32_TX_PORT_DROP_STATS, dfx_ssu_type_32_tx_port_drop_stats2,
		HIKP_ARRAY_SIZE(dfx_ssu_type_32_tx_port_drop_stats2), NULL},
	{ TYPE_32_RUNNING_STATUS, dfx_ssu_type_32_running_stats,
		HIKP_ARRAY_SIZE(dfx_ssu_type_32_running_stats), NULL},
	{ TYPE_32_PORT_RUNNING_STATUS, dfx_ssu_type_32_port_running_stats,
		HIKP_ARRAY_SIZE(dfx_ssu_type_32_port_running_stats), NULL},
	{ TYPE_32_COMM_STATS, dfx_ssu_type_32_comm_stats,
		HIKP_ARRAY_SIZE(dfx_ssu_type_32_comm_stats), NULL},
	{ TYPE_32_COMM_DROP_STATS, dfx_ssu_type_32_comm_drop_stats,
		HIKP_ARRAY_SIZE(dfx_ssu_type_32_comm_drop_stats), NULL},
	{ TYPE_32_COMM_ERROR_STATS, dfx_ssu_type_32_comm_err_stats,
		HIKP_ARRAY_SIZE(dfx_ssu_type_32_comm_err_stats), NULL},
	{ TYPE_64_RX_PORT_DROP_STATS, dfx_ssu_type_64_rx_port_drop_stats,
		HIKP_ARRAY_SIZE(dfx_ssu_type_64_rx_port_drop_stats), NULL},
	{ TYPE_64_TX_PORT_DROP_STATS, dfx_ssu_type_64_tx_port_drop_stats,
		HIKP_ARRAY_SIZE(dfx_ssu_type_64_tx_port_drop_stats), NULL},
	{ TYPE_64_RX_PORT_NORMAL_STATS, dfx_ssu_type_64_rx_port_normal_stats,
		HIKP_ARRAY_SIZE(dfx_ssu_type_64_rx_port_normal_stats), NULL},
	{ TYPE_64_TX_PORT_NORMAL_STATS, dfx_ssu_type_64_tx_port_normal_stats,
		HIKP_ARRAY_SIZE(dfx_ssu_type_64_tx_port_normal_stats), NULL},
	{ TYPE_64_COMM_STATS, dfx_ssu_type_64_tx_comm_stats,
		HIKP_ARRAY_SIZE(dfx_ssu_type_64_tx_comm_stats), NULL},
};

static const struct dfx_reg_name dfx_type_egu_32_error_stats[] = {
	{ "EGU_TX_ECC_1B_ERR_CNT" },
	{ "EGU_TX_ECC_2B_ERR_CNT" },
};

static const struct dfx_reg_name dfx_type_pa_32_rx_port_nor_stats[] = {
	{ "PA_OUT_NON_TUN_L2_PKT" },
	{ "PA_OUT_NON_TUN_L3_PKT" },
	{ "PA_OUT_NON_TUN_L4_PKT" },
	{ "PA_OUT_TUN_L2_PKT" },
	{ "PA_OUT_TUN_L3_PKT" },
	{ "PA_OUT_TUN_L4_PKT" },
	{ "PA_OUT_ROCEE_PKT" },
	{ "PA_OUT_OUTER_UDP0_PKT" },
	{ "PA_OUT_INNER_UDP0_PKT" },
};

static const struct dfx_reg_name dfx_type_igu_32_rx_port_error_stats[] = {
	{ "IGU_RX_ERR_PKT" },
	{ "IGU_RX_NO_SOF_PKT" },
	{ "IGU_RX_OVERSIZE_PKT" },
	{ "IGU_RX_UNDERSIZE_PKT" },
};

static const struct dfx_reg_name dfx_type_pa_32_rx_port_error_stats[] = {
	{ "PA_OUT_EL3E_PKT" },
	{ "PA_OUT_EL4E_PKT" },
	{ "PA_OUT_L3E_PKT" },
	{ "PA_OUT_L4E_PKT" },
};

static const struct dfx_reg_name dfx_type_egu_32_tx_port_nor_stats[] = {
	{ "EGU_TX_1588_PKT" },
	{ "TX_TNL_NOTE_PKT" },
};

static const struct dfx_reg_name dfx_type_egu_32_tx_port_error_stats[] = {
	{ "EGU_TX_SHORT_PKT" },
	{ "EGU_TX_ERR_PKT" },
};

static const struct dfx_reg_name dfx_type_igu_64_rx_port_nor_stats[] = {
	{ "IGU_RX_OUT_ALL_PKT" },
	{ "IGU_RX_UNI_PKT" },
	{ "IGU_RX_MULTI_PKT" },
	{ "IGU_RX_BROAD_PKT" },
};

static const struct dfx_reg_name dfx_type_pa_64_rx_port_nor_stats[] = {
	{ "PA_OUT_NON_TUN_PKT" },
	{ "PA_OUT_TUN_PKT" },
};

static const struct dfx_reg_name dfx_type_egu_64_tx_port_nor_status[] = {
	{ "EGU_TX_OUT_ALL_PKT" },
	{ "EGU_TX_UNI_PKT" },
	{ "EGU_TX_MULTI_PKT" },
	{ "EGU_TX_BROAD_PKT" },
};

static const struct dfx_reg_name dfx_type_igu_32_error_status[] = {
	{ "IGU_OUTER_ERR_STS" },
	{ "IGU_INNER_ERR_STS" },
};

static const struct dfx_reg_name dfx_type_pa_64_tx_port_nor_stats[] = {
	{ "PA_OUT_ALL_PKT" },
};

static bool igu_egu_32_rx_port_err_stats_match(uint16_t offset_0)
{
	return offset_0 == IGU_RX_ERR_PKT_OFFSET;
}

static const struct dfx_type_name_parse g_dfx_igu_egu_name_parse[] = {
	{ TYPE_32_ERROR_STATUS, dfx_type_egu_32_error_stats,
		HIKP_ARRAY_SIZE(dfx_type_egu_32_error_stats), NULL},
	{ TYPE_32_RX_PORT_NORMAL_STATS, dfx_type_pa_32_rx_port_nor_stats,
		HIKP_ARRAY_SIZE(dfx_type_pa_32_rx_port_nor_stats), NULL},
	{ TYPE_32_RX_PORT_ERROR_STATS, dfx_type_igu_32_rx_port_error_stats,
		HIKP_ARRAY_SIZE(dfx_type_igu_32_rx_port_error_stats), igu_egu_32_rx_port_err_stats_match},
	{ TYPE_32_RX_PORT_ERROR_STATS, dfx_type_pa_32_rx_port_error_stats,
		HIKP_ARRAY_SIZE(dfx_type_pa_32_rx_port_error_stats), NULL},
	{ TYPE_32_TX_PORT_NORMAL_STATS, dfx_type_egu_32_tx_port_nor_stats,
		HIKP_ARRAY_SIZE(dfx_type_egu_32_tx_port_nor_stats), NULL},
	{ TYPE_32_TX_PORT_ERROR_STATS, dfx_type_egu_32_tx_port_error_stats,
		HIKP_ARRAY_SIZE(dfx_type_egu_32_tx_port_error_stats), NULL},
	{ TYPE_64_RX_PORT_NORMAL_STATS, dfx_type_igu_64_rx_port_nor_stats,
		HIKP_ARRAY_SIZE(dfx_type_igu_64_rx_port_nor_stats), NULL},
	{ TYPE_64_RX_PORT_NORMAL_STATS, dfx_type_pa_64_rx_port_nor_stats,
		HIKP_ARRAY_SIZE(dfx_type_pa_64_rx_port_nor_stats), NULL},
	{ TYPE_64_TX_PORT_NORMAL_STATS, dfx_type_egu_64_tx_port_nor_status,
		HIKP_ARRAY_SIZE(dfx_type_egu_64_tx_port_nor_status), NULL},
	{ TYPE_32_PORT_ERROR_STATUS, dfx_type_igu_32_error_status,
		HIKP_ARRAY_SIZE(dfx_type_igu_32_error_status), NULL},
	{ TYPE_64_TX_PORT_NORMAL_STATS, dfx_type_pa_64_tx_port_nor_stats,
		HIKP_ARRAY_SIZE(dfx_type_pa_64_tx_port_nor_stats), NULL},
};

static const struct dfx_reg_name dfx_type_ncsi_32_cfg_status[] = {
	{ "NCSI_CTRL_ETH_CFG" },
	{ "NCSI_PKG_ID_CFG" },
	{ "NCSI_CTRL_CKS_CFG" },
	{ "NCSI_HARD_ARB_CFG" },
	{ "NCSI_CTRL_IND_CFG" },
	{ "NCSI_CNT_CLR_CE" },
	{ "NCSI_MEM_INIT_CFG" },
	{ "NCSI_IMP_INT_REG" },
	{ "NCSI_FLUSH_CFG" },
	{ "NCSI_PAUSE_EN_CFG" },
};

static const struct dfx_reg_name dfx_type_ncsi_32_mac_cfg_status[] = {
	{ "MAX_FRM_SIZE" },
	{ "PORT_MODE" },
	{ "PORT_EN" },
	{ "PAUSE_EN" },
	{ "RESERVED" },
	{ "SHORT_RUNTS_THR" },
	{ "RESERVED" },
	{ "AN_NEG_STATE" },
	{ "TX_LOCAL_PAGE" },
	{ "TRANSMIT_CONTROL" },
	{ "REC_FILT_CONTROL" },
};

static const struct dfx_reg_name dfx_type_ncsi_32_run_status[] = {
	{ "NCSI_EGU_TX_FIFO_STS" },
	{ "NCSI_PAUSE_STATUS" },
};

static const struct dfx_reg_name dfx_type_ncsi_32_rx_error_stats[] = {
	{ "NCSI_RX_CTRL_DMAC_ERR_CNT" },
	{ "NCSI_RX_CTRL_SMAC_ERR_CNT" },
	{ "NCSI_RX_CTRL_CKS_ERR_CNT" },
	{ "NCSI_RX_PT_DMAC_ERR_CNT" },
	{ "NCSI_RX_PT_SMAC_ERR_CNT" },
	{ "NCSI_RX_FCS_ERR_CNT" },
	{ "NCSI_RX_CTRL_PKT_TRUN_CNT" },
	{ "NCSI_RX_CTRL_PKT_CFLIT_CNT" },
};

static const struct dfx_reg_name dfx_type_ncsi_32_mac_rx_error_stats[] = {
	{ "RX_OCTETS_BAD" },
	{ "RX_FCS_ERRORS" },
	{ "RX_DATA_ERR" },
	{ "RX_ALIGN_ERRORS" },
	{ "RX_LONG_ERRORS" },
	{ "RX_JABBER_ERRORS" },
	{ "RX_VERY_LONG_ERR_CNT" },
	{ "RX_RUNT_ERR_CNT" },
	{ "RX_SHORT_ERR_CNT" },
	{ "RX_OVERRUN_CNT" },
	{ "RX_LENGTHFIELD_ERR_CNT" },
	{ "RX_FAIL_COMMA_CNT" },
};

static const struct dfx_reg_name dfx_type_ncsi_32_tx_error_stats[] = {
	{ "NCSI_TX_CTRL_DMAC_ERR_CNT" },
	{ "NCSI_TX_CTRL_SMAC_ERR_CNT" },
	{ "NCSI_TX_PT_DMAC_ERR_CNT" },
	{ "NCSI_TX_PT_SMAC_ERR_CNT" },
	{ "NCSI_TX_PT_PKT_TRUN_CNT" },
	{ "NCSI_TX_PT_PKT_ERR_CNT" },
	{ "NCSI_TX_CTRL_PKT_ERR_CNT" },
	{ "NCSI_RX_CTRL_PKT_TRUN_CNT" },
	{ "NCSI_RX_CTRL_PKT_CFLIT_CNT" },
};

static const struct dfx_reg_name dfx_type_ncsi_32_mac_tx_error_stats[] = {
	{ "OCTETS_TRANSMITTED_BAD" },
	{ "TX_UNDERRUN" },
	{ "TX_CRC_ERROR" },
};

static const struct dfx_reg_name dfx_type_ncsi_32_rx_stats[] = {
	{ "NCSI_RX_CTRL_PKT_CNT" },
	{ "NCSI_RX_PT_PKT_CNT" },
	{ "RMII_NCSI_PKT_CNT" },
};

static const struct dfx_reg_name dfx_type_ncsi_32_mac_rx_stats[] = {
	{ "RX_OCTETS_TOTAL_OK" },
	{ "RX_UC_PKTS" },
	{ "RX_MC_PKTS" },
	{ "RX_BC_PKTS" },
	{ "RX_PKTS_64OCTETS" },
	{ "RX_PKTS_65TO127OCTETS" },
	{ "RX_PKTS_128TO255OCTETS" },
	{ "RX_PKTS_255TO511OCTETS" },
	{ "RX_PKTS_512TO1023OCTETS" },
	{ "RX_PKTS_1024TO1518OCTETS" },
	{ "RX_PKTS_1519TOMAXOCTETS" },
	{ "RX_TAGGED" },
	{ "RX_PAUSE_MACCONTROL_FRAMCOUNTER" },
	{ "RX_UNKNOWN_MACCONTROL_FRAMCOUNTER" },
};

static const struct dfx_reg_name dfx_type_ncsi_32_tx_stats[] = {
	{ "NCSI_TX_CTRL_PKT_CNT" },
	{ "NCSI_TX_PT_PKT_CNT" },
	{ "NCSI_RMII_PKT_CNT" },
};

static const struct dfx_reg_name dfx_type_ncsi_32_mac_tx_stats[] = {
	{ "OCTETS_TRANSMITTED_OK" },
	{ "TX_UC_PKTS" },
	{ "TX_MC_PKTS" },
	{ "TX_BC_PKTS" },
	{ "TX_PKTS_64OCTETS" },
	{ "TX_PKTS_65TO127OCTETS" },
	{ "TX_PKTS_128TO255OCTETS" },
	{ "TX_PKTS_255TO511OCTETS" },
	{ "TX_PKTS_512TO1023OCTETS" },
	{ "TX_PKTS_1024TO1518OCTETS" },
	{ "TX_PKTS_1519TOMAXOCTETS" },
	{ "TX_TAGGED" },
	{ "TX_PAUSE_FRAMES" },
};

static const struct dfx_reg_name dfx_type_ncsi_32_error_cnt_stats[] = {
	{ "NCSI_TX_SINGLE_ECC_ERR_CNT" },
	{ "NCSI_TX_MULTI_ECC_ERR_CNT" },
	{ "NCSI_TX_BUF_PT_DMAC_ERR_CNT" },
	{ "" },
	{ "NCSI_TX_BUF_PT_PKT_ERR_CNT" },
	{ "" },
	{ "" },
	{ "" },
	{ "" },
	{ "" },
	{ "" },
	{ "NCSI_UBRX_PT_PKT_ERR_CNT" },
	{ "" },
	{ "NCSI_RX_OVERLONG_TRUN_CNT" },
	{ "" },
	{ "" },
	{ "" },
	{ "" },
	{ "NCSI_UBRX_PT_BUF_OVERWRITE_CNT" },
	{ "" },
	{ "" },
	{ "" },
	{ "" },
	{ "" },
	{ "" },
	{ "" },
	{ "" },
};

static const struct dfx_reg_name dfx_type_ncsi_32_cnt_stats[] = {
	{ "NCSI_TX_BUF_PT_PKT_CNT" },
	{ "NCSI_UBRX_PT_PKT_CNT" },
	{ "NCSI_RX_ARP_PKT_CNT" },
	{ "NCSI_RX_BUF_RW_CNT" },
	{ "" },
	{ "" },
	{ "" },
	{ "" },
	{ "" },
	{ "" },
	{ "" },
};

static const struct dfx_type_name_parse g_dfx_ncsi_name_parse[] = {
	{ TYPE_32_CFG_STATUS, dfx_type_ncsi_32_cfg_status,
		HIKP_ARRAY_SIZE(dfx_type_ncsi_32_cfg_status), NULL},
	{ TYPE_32_CFG_STATUS, dfx_type_ncsi_32_mac_cfg_status,
		HIKP_ARRAY_SIZE(dfx_type_ncsi_32_mac_cfg_status), NULL},
	{ TYPE_32_RUNNING_STATUS, dfx_type_ncsi_32_run_status,
		HIKP_ARRAY_SIZE(dfx_type_ncsi_32_run_status), NULL},
	{ TYPE_32_RX_ERROR_STATS, dfx_type_ncsi_32_rx_error_stats,
		HIKP_ARRAY_SIZE(dfx_type_ncsi_32_rx_error_stats), NULL},
	{ TYPE_32_RX_ERROR_STATS, dfx_type_ncsi_32_mac_rx_error_stats,
		HIKP_ARRAY_SIZE(dfx_type_ncsi_32_mac_rx_error_stats), NULL},
	{ TYPE_32_TX_ERROR_STATS, dfx_type_ncsi_32_tx_error_stats,
		HIKP_ARRAY_SIZE(dfx_type_ncsi_32_tx_error_stats), NULL},
	{ TYPE_32_TX_ERROR_STATS, dfx_type_ncsi_32_mac_tx_error_stats,
		HIKP_ARRAY_SIZE(dfx_type_ncsi_32_mac_tx_error_stats), NULL},
	{ TYPE_32_RX_NORMAL_STATS, dfx_type_ncsi_32_rx_stats,
		HIKP_ARRAY_SIZE(dfx_type_ncsi_32_rx_stats), NULL},
	{ TYPE_32_RX_NORMAL_STATS, dfx_type_ncsi_32_mac_rx_stats,
		HIKP_ARRAY_SIZE(dfx_type_ncsi_32_mac_rx_stats), NULL},
	{ TYPE_32_TX_NORMAL_STATS, dfx_type_ncsi_32_tx_stats,
		HIKP_ARRAY_SIZE(dfx_type_ncsi_32_tx_stats), NULL},
	{ TYPE_32_TX_NORMAL_STATS, dfx_type_ncsi_32_mac_tx_stats,
		HIKP_ARRAY_SIZE(dfx_type_ncsi_32_mac_tx_stats), NULL},
	{ TYPE_32_COMM_ERROR_STATS, dfx_type_ncsi_32_error_cnt_stats,
		HIKP_ARRAY_SIZE(dfx_type_ncsi_32_error_cnt_stats), NULL},
	{ TYPE_32_COMM_STATS, dfx_type_ncsi_32_cnt_stats,
		HIKP_ARRAY_SIZE(dfx_type_ncsi_32_cnt_stats), NULL},
};

static const struct dfx_reg_name dfx_type_bios_32_run_status[] = {
	{ "DFX_MSIX_INFO_NIC_0" },
	{ "DFX_MSIX_INFO_NIC_1" },
	{ "DFX_MSIX_INFO_NIC_2" },
	{ "DFX_MSIX_INFO_NIC_3" },
	{ "DFX_MSIX_INFO_ROC_0" },
	{ "DFX_MSIX_INFO_ROC_1" },
	{ "DFX_MSIX_INFO_ROC_2" },
	{ "DFX_MSIX_INFO_ROC_3" },
	{ "DFX_MSIX_INFO_ROH_0" },
	{ "DFX_MSIX_INFO_ROH_1" },
	{ "DFX_MSIX_INFO_ROH_2" },
	{ "DFX_MSIX_INFO_ROH_3" },
	{ "FUN_RST_META_DATA0_PF0" },
	{ "FUN_RST_META_DATA0_PF1" },
	{ "FUN_RST_META_DATA0_PF2" },
	{ "FUN_RST_META_DATA0_PF3" },
	{ "FUN_RST_META_DATA0_PF4" },
	{ "FUN_RST_META_DATA0_PF5" },
	{ "FUN_RST_META_DATA0_PF6" },
	{ "FUN_RST_META_DATA0_PF7" },
	{ "FUN_RST_META_DATA1" },
	{ "SRIOV_CAPABILITY_STATUS_PF0" },
	{ "SRIOV_CAPABILITY_STATUS_PF1" },
	{ "SRIOV_CAPABILITY_STATUS_PF2" },
	{ "SRIOV_CAPABILITY_STATUS_PF3" },
	{ "SRIOV_CAPABILITY_STATUS_PF4" },
	{ "SRIOV_CAPABILITY_STATUS_PF5" },
	{ "SRIOV_CAPABILITY_STATUS_PF6" },
	{ "SRIOV_CAPABILITY_STATUS_PF7" },
	{ "BP_CPU_STATE" },
	{ "FUN_RST_STATE" },
	{ "IMP_BIOS_OS_STS_REG" },
	{ "IMP_DBG_RSV_REG_0" },
	{ "IMP_DBG_RSV_REG_1" },
	{ "IMP_DBG_RSV_REG_2" },
	{ "IMP_DBG_RSV_REG_3" },
	{ "IMP_DBG_RSV_REG_4" },
	{ "IMP_DBG_RSV_REG_5" },
	{ "IMP_DBG_RSV_REG_6" },
	{ "IMP_DBG_RSV_REG_7" },
	{ "IMP_DBG_RSV_REG_8" },
	{ "IMP_DBG_RSV_REG_9" },
	{ "IMP_DBG_RSV_REG_10" },
	{ "IMP_DBG_RSV_REG_11" },
	{ "IMP_DBG_RSV_REG_12" },
	{ "IMP_DBG_RSV_REG_13" },
	{ "IMP_DBG_RSV_REG_14" },
	{ "IMP_DBG_RSV_REG_15" },
	{ "IMP_DBG_RSV_REG_16" },
	{ "IMP_DBG_RSV_REG_17" },
	{ "IMP_DBG_RSV_REG_18" },
	{ "IMP_DBG_RSV_REG_19" },
	{ "IMP_DBG_RSV_REG_20" },
	{ "IMP_DBG_RSV_REG_21" },
	{ "IMP_DBG_RSV_REG_22" },
	{ "IMP_DBG_RSV_REG_23" },
	{ "IMP_DBG_RSV_REG_24" },
	{ "IMP_DBG_RSV_REG_25" },
	{ "IMP_DBG_RSV_REG_26" },
	{ "IMP_DBG_RSV_REG_27" },
	{ "IMP_DBG_RSV_REG_28" },
	{ "IMP_DBG_RSV_REG_29" },
	{ "IMP_DBG_RSV_REG_30" },
	{ "IMP_DBG_RSV_REG_31" },
};

static const struct dfx_reg_name dfx_type_bios_comm_stats[] = {
	{ "MSIX_IRQ_CNT_NIC_0" },
	{ "MSIX_IRQ_CNT_NIC_1" },
	{ "MSIX_IRQ_CNT_ROC_0" },
	{ "MSIX_IRQ_CNT_ROC_1" },
	{ "MSIX_IRQ_CNT_ROH_0" },
	{ "MSIX_IRQ_CNT_ROH_1" },
	{ "MSIX_IRQ_CNT_NIC_2" },
	{ "MSIX_IRQ_CNT_ROC_2" },
	{ "MSIX_IRQ_CNT_ROH_2" },
};

static const struct dfx_type_name_parse g_dfx_bios_name_parse[] = {
	{ TYPE_32_RUNNING_STATUS, dfx_type_bios_32_run_status, HIKP_ARRAY_SIZE(dfx_type_bios_32_run_status), NULL},
	{ TYPE_32_COMM_STATS, dfx_type_bios_comm_stats, HIKP_ARRAY_SIZE(dfx_type_bios_comm_stats), NULL},
};

static const struct dfx_reg_name dfx_type_rcb_32_run_status[] = {
	{ "FSM_DFX_ST0" },
	{ "FSM_DFX_ST1" },
	{ "FSM_DFX_ST2" },
	{ "FIFO_DFX_ST4" },
	{ "FIFO_DFX_ST6" },
	{ "FIFO_DFX_ST8" },
	{ "FIFO_DFX_ST10" },
	{ "TPE_FIFO_DFX_0" },
	{ "TPE_FIFO_DFX_1" },
	{ "RCB_TX_QUEUE_RST_DFX" },
};

static const struct dfx_reg_name dfx_type_rcb_32_tx_error_stats[] = {
	{ "RCB_TX_MEM_SERR_CNT" },
	{ "RCB_TX_MEM_MERR_CNT" },
	{ "RCB_TX_RCV_PUSH_BD_CMD_ERR_CNT" },
};

static const struct dfx_reg_name dfx_type_rcb_32_rx_error_stats[] = {
	{ "RCB_RX_RING_SERR_CNT" },
	{ "RCB_RX_EBD_SERR_CNT" },
	{ "RCB_RX_STASH_CFG_SERR_CNT" },
	{ "RCB_RX_INT_INFO_SERR_CNT" },
	{ "RPU_GRO_BD_SERR_CNT" },
	{ "RPU_GRO_CONTEXT_SERR_CNT" },
	{ "RPU_RX_PKT_SERR_CNT_RXPKTMEM0" },
	{ "RPU_RX_PKT_SERR_CNT_RXPKTMEM1" },
	{ "RPE_PFC_INFO_SERR_CNT" },
	{ "RPU_FUNC_DCNT_ODDERR_CNT" },
	{ "RPU_BD_MEM_ECC_SERR_CNT" },
	{ "RCB_RX_RING_MERR_CNT" },
	{ "RCB_RX_EBD_MERR_CNT" },
	{ "RCB_RX_STASH_CFG_MERR_CNT" },
	{ "RCB_RX_INT_INFO_MERR_CNT" },
	{ "RPU_GRO_BD_MERR_CNT" },
	{ "RPU_GRO_CONTEXT_MERR_CNT" },
	{ "RPU_RX_PKT_MERR_CNT_RXPKTMEM0" },
	{ "RPU_RX_PKT_MERR_CNT_RXPKTMEM1" },
	{ "RPE_PFC_INFO_MERR_CNT" },
	{ "RPU_BD_MEM_ECC_MERR_CNT" },
};

static const struct dfx_reg_name dfx_type_rcb_64_tx_pf_drop_stats[] = {
	{ "RCB_TX_RCV_PUSH_BD_FULL_DRP_CNT" },
	{ "RCB_TX_RCV_PUSH_BD_PULL_DRP_CNT" },
	{ "RCB_TX_RCV_PUSH_BD_CRDT_VLD_DRP_CNT" },
	{ "RCB_TX_RCV_PUSH_BD_TC_BP_DRP_CNT" },
	{ "RCB_TX_RCV_PUSH_BD_FLUSH_DRP_CNT" },
};

static const struct dfx_reg_name dfx_type_rcb_64_tx_pf_nor_stats[] = {
	{ "RCB_TX_RCV_PUSH_BD_CNT" },
	{ "RCB_TX_RCV_CMB_PUSH_BD_CNT" },
	{ "RCB_TX_RCV_PUSH_BD_SUCCESS_CNT" },
};

static const struct dfx_type_name_parse g_dfx_rcb_name_parse[] = {
	{ TYPE_32_RUNNING_STATUS, dfx_type_rcb_32_run_status,
		HIKP_ARRAY_SIZE(dfx_type_rcb_32_run_status), NULL},
	{ TYPE_32_TX_DROP_STATS, dfx_type_rcb_32_tx_error_stats,
		HIKP_ARRAY_SIZE(dfx_type_rcb_32_tx_error_stats), NULL},
	{ TYPE_32_RX_DROP_STATS, dfx_type_rcb_32_rx_error_stats,
		HIKP_ARRAY_SIZE(dfx_type_rcb_32_rx_error_stats), NULL},
	{ TYPE_64_TX_PF_DROP_STATS, dfx_type_rcb_64_tx_pf_drop_stats,
		HIKP_ARRAY_SIZE(dfx_type_rcb_64_tx_pf_drop_stats), NULL},
	{ TYPE_64_TX_PF_NORMAL_STATS, dfx_type_rcb_64_tx_pf_nor_stats,
		HIKP_ARRAY_SIZE(dfx_type_rcb_64_tx_pf_nor_stats), NULL},
};

static const struct dfx_reg_name dfx_type_txdma_64_tx_port_stats[] = {
	{ "NIC_TX_IN_PKT_CNT" },
	{ "ROCE_TX_IN_PKT_CNT" },
	{ "NIC_TX_OUT_PKT_CNT" },
	{ "ROCE_TX_OUT_PKT_CNT" },
	{ "NIC_TX_IN_AR_CNT" },
	{ "ROCE_TX_IN_AR_CNT" },
	{ "AXI_DATA_NIC_CNT" },
	{ "AXI_DATA_ROCE_CNT" },
};

static const struct dfx_reg_name dfx_type_txdma_64_tx_nor_stats[] = {
	{ "TXDMA_AR_CNT_MSTER0" },
	{ "TXDMA_AR_CNT_MSTER1" },
	{ "TXDMA_R_CNT_MSTER0" },
	{ "TXDMA_R_CNT_MSTER1" },
	{ "TXDMA_AXI_TX_OUT_CNT" },
};

static const struct dfx_reg_name dfx_type_txdma_32_run_status[] = {
	{ "ROCE_TPP_DATA_FIFO_ST" },
	{ "ROCE_TPP_DATA_ALM" },
	{ "ROCE_TPP_DATA_STA0" },
	{ "ROCE_TPP_DATA_STA1" },
	{ "TXDMA_BP_STATUS" },
	{ "" },
	{ "TXDMA_FIFO_FULL_STATUS" },
	{ "TXDMA_QUEUE_RESET_ST" },
};

static const struct dfx_reg_name dfx_type_txdma_32_tx_port_stats[] = {
	{ "TXDMA_OST_CNT_MASTER0" },
	{ "TXDMA_OST_CNT_MASTER1" },
};

static const struct dfx_reg_name dfx_type_txdma_32_tx_port_err_stats[] = {
	{ "TXDMA_ROCE_POISON_NAK_CNT" },
};

static const struct dfx_reg_name dfx_type_txdma_32_port_run_status[] = {
	{ "TPU_RDATA_CTRL_FIFO_DFX" },
};

static const struct dfx_type_name_parse g_dfx_txdma_name_parse[] = {
	{ TYPE_64_TX_PORT_NORMAL_STATS, dfx_type_txdma_64_tx_port_stats,
		HIKP_ARRAY_SIZE(dfx_type_txdma_64_tx_port_stats), NULL},
	{ TYPE_64_TX_NORMAL_STATS, dfx_type_txdma_64_tx_nor_stats,
		HIKP_ARRAY_SIZE(dfx_type_txdma_64_tx_nor_stats), NULL},
	{ TYPE_32_RUNNING_STATUS, dfx_type_txdma_32_run_status,
		HIKP_ARRAY_SIZE(dfx_type_txdma_32_run_status), NULL},
	{ TYPE_32_TX_PORT_NORMAL_STATS, dfx_type_txdma_32_tx_port_stats,
		HIKP_ARRAY_SIZE(dfx_type_txdma_32_tx_port_stats), NULL},
	{ TYPE_32_TX_PORT_ERROR_STATS, dfx_type_txdma_32_tx_port_err_stats,
		HIKP_ARRAY_SIZE(dfx_type_txdma_32_tx_port_err_stats), NULL},
	{ TYPE_32_PORT_RUNNING_STATUS, dfx_type_txdma_32_port_run_status,
		HIKP_ARRAY_SIZE(dfx_type_txdma_32_port_run_status), NULL},
};

static const struct dfx_reg_name dfx_type_master_32_run_status[] = {
	{ "OUTSATNDING_NUM_SCH0" },
	{ "OUTSATNDING_NUM_SCH1" },
	{ "OUTSATNDING_NUM_W_DFX0" },
	{ "OUTSATNDING_NUM_W_DFX1" },
	{ "OUTSATNDING_NUM_W_DFX2" },
	{ "OUTSATNDING_NUM_R_DFX0" },
};

static const struct dfx_type_name_parse g_dfx_master_name_parse[] = {
	{ TYPE_32_RUNNING_STATUS, dfx_type_master_32_run_status, HIKP_ARRAY_SIZE(dfx_type_master_32_run_status), NULL},
};

static const struct dfx_module_cmd g_dfx_module_parse[] = {
	{"SSU", SSU_DFX_REG_DUMP, g_dfx_ssu_name_parse, HIKP_ARRAY_SIZE(g_dfx_ssu_name_parse)},
	{"IGU_EGU", IGU_EGU_DFX_REG_DUMP, g_dfx_igu_egu_name_parse, HIKP_ARRAY_SIZE(g_dfx_igu_egu_name_parse)},
	{"PPP", PPP_DFX_REG_DUMP, g_dfx_ppp_name_parse, HIKP_ARRAY_SIZE(g_dfx_ppp_name_parse)},
	{"NCSI", NCSI_DFX_REG_DUMP, g_dfx_ncsi_name_parse, HIKP_ARRAY_SIZE(g_dfx_ncsi_name_parse)},
	{"BIOS", BIOS_COMM_DFX_REG_DUMP, g_dfx_bios_name_parse, HIKP_ARRAY_SIZE(g_dfx_bios_name_parse)},
	{"RCB", RCB_DFX_REG_DUMP, g_dfx_rcb_name_parse, HIKP_ARRAY_SIZE(g_dfx_rcb_name_parse)},
	{"TXDMA", TXDMA_DFX_REG_DUMP, g_dfx_txdma_name_parse, HIKP_ARRAY_SIZE(g_dfx_txdma_name_parse)},
	{"MASTER", MASTER_DFX_REG_DUMP, g_dfx_master_name_parse, HIKP_ARRAY_SIZE(g_dfx_master_name_parse)},
};

void hikp_nic_dfx_set_cmd_para(int idx)
{
	if (idx < (int)HIKP_ARRAY_SIZE(g_dfx_module_parse)) {
		g_dfx_param.sub_cmd_code = g_dfx_module_parse[idx].sub_cmd_code;
		g_dfx_param.module_idx = idx;
		g_dfx_param.flag |= MODULE_SET_FLAG;
	}
}

static void dfx_help_info(const struct major_cmd_ctrl *self)
{
	printf("\n  Usage: %s %s\n", self->cmd_ptr->name, "-i <interface>\n");
	printf("\n         %s\n", self->cmd_ptr->help_info);
	printf("  Options:\n\n");
	printf("    %s, %-25s %s\n", "-h", "--help", "display this help and exit");
	printf("    %s, %-25s %s\n", "-i", "--interface=<interface>", "device target, e.g. eth0~7");
	printf("    %s\n", "	[-m/--module SSU/IGU_EGU/PPP/NCSI/BIOS/RCB/TXDMA/MASTER] :"
	       "this is necessary param\n");
}

static int hikp_cmd_dfx_help(struct major_cmd_ctrl *self, const char *argv)
{
	HIKP_SET_USED(argv);

	dfx_help_info(self);
	return 0;
}

int hikp_nic_cmd_dfx_target(struct major_cmd_ctrl *self, const char *argv)
{
	self->err_no = tool_check_and_get_valid_bdf_id(argv, &(g_dfx_param.target));
	if (self->err_no != 0) {
		snprintf(self->err_str, sizeof(self->err_str), "Unknown device %s.", argv);
		return self->err_no;
	}

	return 0;
}

static int hikp_nic_dfx_get_blk(struct hikp_cmd_ret **cmd_ret,
				uint32_t blk_id, uint32_t sub_cmd_code)
{
	struct nic_dfx_req_para req_data = { 0 };
	struct hikp_cmd_header req_header = { 0 };

	req_data.bdf = g_dfx_param.target.bdf;
	req_data.block_id = blk_id;
	hikp_cmd_init(&req_header, NIC_MOD, GET_DFX_INFO_CMD, sub_cmd_code);
	*cmd_ret = hikp_cmd_alloc(&req_header, &req_data, sizeof(req_data));

	return hikp_rsp_normal_check(*cmd_ret);
}

static int hikp_nic_get_first_blk_dfx(struct nic_dfx_rsp_head_t *rsp_head, uint32_t **reg_data,
				      uint32_t *max_dfx_size, uint32_t *version)
{
	uint32_t reg_data_size = MAX_DFX_DATA_NUM * sizeof(uint32_t);
	struct nic_dfx_rsp_t *dfx_rsp = NULL;
	struct hikp_cmd_ret *cmd_ret = NULL;
	int ret;

	ret = hikp_nic_dfx_get_blk(&cmd_ret, 0, g_dfx_param.sub_cmd_code);
	if (ret < 0)
		goto err_out;

	dfx_rsp = (struct nic_dfx_rsp_t *)(cmd_ret->rsp_data);
	*version = cmd_ret->version;
	*rsp_head = dfx_rsp->rsp_head;
	if (rsp_head->total_blk_num == 0) {
		/* if total block number is zero, set total type number to zero anyway */
		rsp_head->total_type_num = 0;
		goto err_out;
	}
	*max_dfx_size = dfx_get_max_reg_bffer_size(rsp_head);
	*reg_data = (uint32_t *)calloc(1, *max_dfx_size);
	if (*reg_data == NULL) {
		HIKP_ERROR_PRINT("malloc log memory 0x%x failed.\n", *max_dfx_size);
		ret = -ENOMEM;
		goto err_out;
	}

	if (rsp_head->cur_blk_size > *max_dfx_size || rsp_head->cur_blk_size > reg_data_size) {
		free(*reg_data);
		*reg_data = NULL;
		HIKP_ERROR_PRINT("blk0 reg_data copy size error"
				 "data size: 0x%x, max size: 0x%x, reg_data_size: 0x%x\n",
				 rsp_head->cur_blk_size, *max_dfx_size, reg_data_size);
		ret = -EINVAL;
		goto err_out;
	}
	memcpy(*reg_data, dfx_rsp->reg_data, rsp_head->cur_blk_size);

	*max_dfx_size -= (uint32_t)rsp_head->cur_blk_size;
err_out:
	hikp_cmd_free(&cmd_ret);

	return ret;
}

static int hikp_nic_get_blk_dfx(struct nic_dfx_rsp_head_t *rsp_head, uint32_t blk_id,
				uint32_t *reg_data, uint32_t *max_dfx_size)
{
	struct nic_dfx_rsp_t *dfx_rsp = NULL;
	struct hikp_cmd_ret *cmd_ret = NULL;
	int ret;

	ret = hikp_nic_dfx_get_blk(&cmd_ret, blk_id, g_dfx_param.sub_cmd_code);
	if (ret < 0)
		goto err_out;

	dfx_rsp = (struct nic_dfx_rsp_t *)(cmd_ret->rsp_data);
	*rsp_head = dfx_rsp->rsp_head;
	if (rsp_head->cur_blk_size > *max_dfx_size) {
		HIKP_ERROR_PRINT("blk%u reg_data copy size error, "
				 "data size: 0x%x, max size: 0x%x\n",
				 blk_id, rsp_head->cur_blk_size, *max_dfx_size);
		ret = -EINVAL;
		goto err_out;
	}
	memcpy(reg_data, dfx_rsp->reg_data, rsp_head->cur_blk_size);
	*max_dfx_size -= (uint32_t)rsp_head->cur_blk_size;

err_out:
	hikp_cmd_free(&cmd_ret);

	return ret;
}

static int cmd_dfx_module_select(struct major_cmd_ctrl *self, const char *argv)
{
	size_t arr_size = HIKP_ARRAY_SIZE(g_dfx_module_parse);
	bool is_found;
	size_t i;

	for (i = 0; i < arr_size; i++) {
		is_found = strncmp(argv, (const char *)g_dfx_module_parse[i].module_name,
						   sizeof(g_dfx_module_parse[i].module_name)) == 0;
		if (is_found) {
			g_dfx_param.sub_cmd_code = g_dfx_module_parse[i].sub_cmd_code;
			g_dfx_param.module_idx = i;
			g_dfx_param.flag |= MODULE_SET_FLAG;
			return 0;
		}
	}
	dfx_help_info(self);
	snprintf(self->err_str, sizeof(self->err_str), "-m/--module param error!!!");
	self->err_no = -EINVAL;

	return -EINVAL;
}

static const struct dfx_reg_name *hikp_nic_dfx_get_reg_list(uint8_t type_id, uint32_t reg_num, uint16_t offset_0,
							    const struct dfx_module_cmd *dfx_module_info)
{
	uint32_t i;

	for (i = 0; i < dfx_module_info->dfx_name_parse_size; i++) {
		if (dfx_module_info->dfx_name_parse[i].type_id != type_id)
			continue;
		if (dfx_module_info->dfx_name_parse[i].reg_num != reg_num)
			continue;
		if (dfx_module_info->dfx_name_parse[i].match && !dfx_module_info->dfx_name_parse[i].match(offset_0))
			continue;
		return dfx_module_info->dfx_name_parse[i].reg_list;
	}
	return NULL;
}

static void hikp_nic_dfx_print_b32(struct nic_dfx_type_head *type_head, uint32_t *reg_data)
{
	uint32_t num = (uint32_t)type_head->reg_num;
	uint32_t word_num = num * WORD_NUM_PER_REG;
	const struct dfx_reg_name *reg_list;
	uint32_t sub_cmd_code;
	uint16_t offset_0;
	uint16_t offset;
	uint32_t value;
	uint32_t index;
	uint32_t i;

	sub_cmd_code = g_dfx_module_parse[g_dfx_param.module_idx].sub_cmd_code;
	offset_0 = (uint16_t)HI_GET_BITFIELD(reg_data[0], 0, DFX_REG_ADDR_MASK);
	reg_list = hikp_nic_dfx_get_reg_list(type_head->type_id, num, offset_0, &g_dfx_module_parse[sub_cmd_code]);

	for (i = 0, index = 1; i < word_num; i = i + WORD_NUM_PER_REG, index++) {
		offset = (uint16_t)HI_GET_BITFIELD(reg_data[i], 0, DFX_REG_ADDR_MASK);
		value = reg_data[i + 1];
		if (reg_list != NULL) {
			printf("%-35s\t0x%04x\t0x%08x\n", reg_list->name, offset, value);
			reg_list++;
		} else {
			printf("%-35s\t0x%04x\t0x%08x\n", "", offset, value);
		}
	}
}

static void hikp_nic_dfx_print_b64(struct nic_dfx_type_head *type_head, uint32_t *reg_data)
{
	uint32_t num = (uint32_t)type_head->reg_num;
	uint32_t word_num = num * WORD_NUM_PER_REG;
	const struct dfx_reg_name *reg_list;
	uint32_t sub_cmd_code;
	uint16_t offset_0;
	uint16_t offset;
	uint64_t value;
	uint32_t index;
	uint32_t i;

	sub_cmd_code = g_dfx_module_parse[g_dfx_param.module_idx].sub_cmd_code;
	offset_0 = (uint16_t)HI_GET_BITFIELD(reg_data[0], 0, DFX_REG_ADDR_MASK);
	reg_list = hikp_nic_dfx_get_reg_list(type_head->type_id, num, offset_0, &g_dfx_module_parse[sub_cmd_code]);

	for (i = 0, index = 1; i < word_num; i = i + WORD_NUM_PER_REG, index++) {
		offset = (uint16_t)HI_GET_BITFIELD(reg_data[i], 0, DFX_REG_ADDR_MASK);
		value = (uint64_t)reg_data[i + 1] |
			(HI_GET_BITFIELD((uint64_t)reg_data[i], DFX_REG_VALUE_OFF,
			DFX_REG_VALUE_MASK) << BIT_NUM_OF_WORD);
		if (reg_list != NULL) {
			printf("%-35s\t0x%04x\t0x%" PRIx64 "\n", reg_list->name, offset, value);
			reg_list++;
		} else {
			printf("%-35s\t0x%04x\t0x%" PRIx64 "\n", "", offset, value);
		}
	}
}

static bool is_type_found(uint16_t type_id, uint32_t *index)
{
	size_t arr_size = HIKP_ARRAY_SIZE(g_dfx_type_parse);
	size_t i;

	for (i = 0; i < arr_size; i++) {
		if (g_dfx_type_parse[i].type_id == type_id) {
			*index = i;
			return true;
		}
	}

	return false;
}

static void hikp_nic_dfx_print_type_head(uint8_t type_id, uint8_t *last_type_id)
{
	uint32_t index = 0;

	if (type_id != *last_type_id) {
		printf("-----------------------------------------------------\n");
		if (is_type_found(type_id, &index))
			printf("type name: %s\n\n", g_dfx_type_parse[index].type_name);
		else
			HIKP_WARN_PRINT("type name: unknown type, type id is %u\n\n", type_id);

		*last_type_id = type_id;
	}
}

static void hikp_nic_dfx_print(const struct nic_dfx_rsp_head_t *rsp_head, uint32_t *reg_data)
{
	struct nic_dfx_type_head *type_head;
	uint8_t last_type_id = 0;
	uint32_t *ptr = reg_data;
	uint32_t max_size;
	uint32_t num_u32;
	bool show_title;
	uint8_t i;

	max_size = dfx_get_max_reg_bffer_size(rsp_head);
	for (i = 0; i < rsp_head->total_type_num; i++) {
		type_head = (struct nic_dfx_type_head *)ptr;
		num_u32 = type_head->reg_num * WORD_NUM_PER_REG + 1; /* including type_head */
		if (max_size < num_u32 * sizeof(uint32_t)) {
			HIKP_ERROR_PRINT("register real size exceeds the max size\n");
			return;
		}
		ptr += num_u32;
		max_size -= num_u32 * sizeof(uint32_t);
	}

	ptr = reg_data;
	printf("****************** module %s reg dump start ********************\n",
		g_dfx_module_parse[g_dfx_param.module_idx].module_name);
	for (i = 0; i < rsp_head->total_type_num; i++) {
		type_head = (struct nic_dfx_type_head *)ptr;
		if (type_head->type_id == INCORRECT_REG_TYPE) {
			HIKP_ERROR_PRINT("No.%u type is incorrect reg type\n", i + 1u);
			break;
		}
		show_title = type_head->type_id != last_type_id;
		hikp_nic_dfx_print_type_head(type_head->type_id, &last_type_id);
		ptr++;
		if (show_title)
			printf("%-35s\t%s\t%s\n", "name", "offset", "value");
		if (type_head->bit_width == WIDTH_32_BIT) {
			hikp_nic_dfx_print_b32(type_head, ptr);
		} else if (type_head->bit_width == WIDTH_64_BIT) {
			hikp_nic_dfx_print_b64(type_head, ptr);
		} else {
			HIKP_ERROR_PRINT("type%u's bit width error.\n", type_head->type_id);
			break;
		}
		ptr += (uint32_t)type_head->reg_num * WORD_NUM_PER_REG;
	}
	printf("################### ====== dump end ====== ######################\n");
}

void hikp_nic_dfx_cmd_execute(struct major_cmd_ctrl *self)
{
	struct nic_dfx_rsp_head_t rsp_head = { 0 };
	struct nic_dfx_rsp_head_t tmp_head = { 0 };
	uint32_t *reg_data = NULL;
	uint32_t real_reg_size;
	uint32_t max_dfx_size;
	uint32_t version;
	uint32_t i;

	if (!(g_dfx_param.flag & MODULE_SET_FLAG)) {
		self->err_no = -EINVAL;
		snprintf(self->err_str, sizeof(self->err_str), "Please specify a module.");
		dfx_help_info(self);
		return;
	}
	self->err_no = hikp_nic_get_first_blk_dfx(&rsp_head, &reg_data, &max_dfx_size, &version);
	if (self->err_no != 0) {
		snprintf(self->err_str, sizeof(self->err_str), "get the first block dfx fail.");
		return;
	}
	real_reg_size = (uint32_t)rsp_head.cur_blk_size;
	for (i = 1; i < rsp_head.total_blk_num; i++) {
		self->err_no = hikp_nic_get_blk_dfx(&tmp_head, i,
						    reg_data + (real_reg_size / sizeof(uint32_t)),
						    &max_dfx_size);
		if (self->err_no != 0) {
			snprintf(self->err_str, sizeof(self->err_str),
				 "getting block%u reg fail.", i);
			free(reg_data);
			return;
		}
		real_reg_size += (uint32_t)tmp_head.cur_blk_size;
		memset(&tmp_head, 0, sizeof(struct nic_dfx_rsp_head_t));
	}

	printf("DFX cmd version: 0x%x\n\n", version);
	hikp_nic_dfx_print((const struct nic_dfx_rsp_head_t *)&rsp_head, reg_data);
	free(reg_data);
}

static void cmd_nic_dfx_init(void)
{
	struct major_cmd_ctrl *major_cmd = get_major_cmd();

	major_cmd->option_count = 0;
	major_cmd->execute = hikp_nic_dfx_cmd_execute;

	cmd_option_register("-h", "--help", false, hikp_cmd_dfx_help);
	cmd_option_register("-i", "--interface", true, hikp_nic_cmd_dfx_target);
	cmd_option_register("-m", "--module", true, cmd_dfx_module_select);
}

HIKP_CMD_DECLARE("nic_dfx", "dump dfx info of hardware", cmd_nic_dfx_init);
