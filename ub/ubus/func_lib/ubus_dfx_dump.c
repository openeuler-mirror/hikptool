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

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "hikptdev_plug.h"
#include "tool_lib.h"
#include "tool_cmd.h"
#include "ubus_common.h"
#include "ubus_dfx_dump.h"

static struct ubus_dumpreg_info g_reg_table_rxdma[] = {
	{0, "RXDMA_RAW_PKT_L2E_CNT"},
	{0, "RXDMA_RWP_PKT_IN_CNT"},
	{0, "RXDMA_RWP_PKT_OUT_CNT0"},
	{0, "RXDMA_RWP_PKT_OUT_CNT1"},
	{0, "RXDMA_INTF_STATUS"},
	{0, "RXDMA_UBMRX_PKT_OUT_CNT"},
	{0, "RXDMA_UBMRX_PKT_IN_CNT"},
	{0, "RXDMA_UBMRX_CB_PKT_OUT_CNT"},
	{0, "RXDMA_UBMRX_CB_PKT_IN_CNT"},
};

static struct ubus_dumpreg_info g_reg_table_txdma[] = {
	{0, "TXDMA_AR_CNT_MSTER1"},
	{0, "TXDMA_R_CNT_MSTER0"},
	{0, "TXDMA_R_CNT_MSTER1"},
	{0, "UB_TX_IN_PKT_CNT_G"},
	{0, "NIC_TX_IN_PKT_CNT_G"},
	{0, "UB_TX_OUT_PKT_CNT_G"},
	{0, "NIC_TX_OUT_PKT_CNT_G"},
	{0, "UB_C_OUT_PKT_CNT_G"},
	{0, "UB_G_OUT_PKT_CNT_G"},
	{0, "UB_C_IN_PKT_CNT_G"},
	{0, "UB_G_IN_PKT_CNT_G"},
	{0, "LSA_IN_PKT_CNT_G"},
	{0, "LSA_OUT_PKT_CNT_G"},
	{0, "PFA_OUT_PKT_CNT"},
	{0, "MAR_IN_PKT_CNT"},
	{0, "MAR_OUT_PKT_CNT"},
	{0, "PA_OUT_PKT_ERR_CNT"},
	{0, "AXI_MEM_CNT_0"},
	{0, "AXI_MEM_CNT_1"},
	{0, "AXI_MEM_CNT_2"},
	{0, "AXI_MEM_CNT_3"},
	{0, "P2P_BYPASS_CNT"},
	{0, "TPP_HDR_CREDIT_PORY0"},
	{0, "TPP_DATA_CREDIT_PORY0"},
	{0, "TPP_HDR_CREDIT_PORY2"},
	{0, "TPP_DATA_CREDIT_PORY2"},
};

static struct ubus_dumpreg_info g_reg_table_tx_cnt[] = {
	{0, "UB_TX_IN_PKT_CNT"},
	{0, "NIC_TX_IN_PKT_CNT"},
	{0, "UB_TX_OUT_PKT_CNT"},
	{0, "NIC_TX_OUT_PKT_CNT"},
	{0, "UB_C_OUT_PKT_CNT"},
	{0, "UB_G_OUT_PKT_CNT"},
	{0, "UB_C_IN_PKT_CNT"},
	{0, "UB_G_IN_PKT_CNT"},
	{0, "LSA_IN_PKT_CNT"},
	{0, "LSA_OUT_PKT_CNT"},
};

static struct ubus_dumpreg_info g_reg_table_ba_dfx[] = {
	{0, "DFX_MASTER_P2P_STATUS0"},
	{0, "DFX_MASTER_P2P_STATUS1"},
	{0, "DFX_MASTER_TLB_LKUP_REQ_CNT"},
	{0, "DFX_MASTER_TLB_RETRIED_REQ_CNT"},
	{0, "DFX_MASTER_TLB_RSP_SUCCESS_CNT"},
	{0, "DFX_MASTER_TLB_RSP_FAULT_CNT"},
	{0, "DFX_MASTER_DTLB_HIT_CNT0"},
	{0, "MASTER_ECO_REG"},
	{0, "MASTER_REQ_DAT_COMB_EN"},
	{0, "DFX_MASTER_SYNC_CNT"},
	{0, "DFX_MASTER_INV_CNT0"},
	{0, "DFX_MASTER_INV_CNT1"},
	{0, "DFX_MASTER_INV_CNT2"},
	{0, "DFX_MASTER_INV_CNT3"},
	{0, "DFX_MASTER_DTLB_HIT_CNT1"},
	{0, "DFX_MASTER_DTLB_STATUS0"},
	{0, "DFX_MASTER_DTLB_STATUS1"},
	{0, "DFX_MASTER_DTLB_STATUS2"},
	{0, "DFX_UIMMU_FIRST_FAULT_STS"},
	{0, "DFX_UIMMU_FIRST_FAULT_VAH"},
	{0, "DFX_UIMMU_FIRST_FAULT_VAL"},
	{0, "DFX_MASTER_TP_WR_REQ_CNT"},
	{0, "DFX_MASTER_TP_RD_REQ_CNT"},
	{0, "DFX_MASTER_TA_WR_REQ_CNT"},
	{0, "DFX_MASTER_TA_RD_REQ_CNT"},
	{0, "DFX_MASTER_TXDMA_WR_REQ_CNT"},
	{0, "DFX_MASTER_TXDMA_RD_REQ_CNT"},
	{0, "DFX_MASTER_RXDMA_WR_REQ_CNT"},
	{0, "DFX_MASTER_RXDMA_RD_REQ_CNT"},
	{0, "DFX_MASTER_AMB0_WR_REQ_CNT"},
	{0, "DFX_MASTER_AMB0_RD_REQ_CNT"},
	{0, "DFX_MASTER_AMB0_AWACK_CNT"},
	{0, "DFX_MASTER_AMB0_WLAST_CNT"},
	{0, "DFX_MASTER_AMB0_BRESP_CNT"},
	{0, "DFX_MASTER_AMB0_RD_OUTSTANDING"},
	{0, "DFX_MASTER_AMB0_LAST_WR_ADDR_H"},
	{0, "DFX_MASTER_AMB0_LAST_WR_ADDR_L"},
	{0, "DFX_MASTER_AMB0_LAST_RD_ADDR_H"},
	{0, "DFX_MASTER_AMB0_LAST_RD_ADDR_L"},
	{0, "DFX_MASTER_AMB0_BKPR_STS"},
	{0, "DFX_MASTER_AMB1_WR_REQ_CNT"},
	{0, "DFX_MASTER_AMB1_RD_REQ_CNT"},
	{0, "DFX_MASTER_AMB1_AWACK_CNT"},
	{0, "DFX_MASTER_AMB1_WLAST_CNT"},
	{0, "DFX_MASTER_AMB1_BRESP_CNT"},
	{0, "DFX_MASTER_AMB1_RD_OUTSTANDING"},
	{0, "DFX_MASTER_AMB1_LAST_WR_ADDR_H"},
	{0, "DFX_MASTER_AMB1_LAST_WR_ADDR_L"},
	{0, "DFX_MASTER_AMB1_LAST_RD_ADDR_H"},
	{0, "DFX_MASTER_AMB1_LAST_RD_ADDR_L"},
	{0, "DFX_MASTER_AMB1_BKPR_STS"},
	{0, "DFX_MASTER_LKP_ODR0_CTX_ADDR"},
	{0, "DFX_MASTER_LKP_ODR0_CTX0"},
	{0, "DFX_MASTER_LKP_ODR0_CTX1"},
	{0, "DFX_MASTER_LKP_ODR0_CTX2"},
	{0, "DFX_MASTER_LKP_ODR0_CTX3"},
	{0, "DFX_MASTER_LKP_ODR0_OUTSTANDING"},
	{0, "DFX_MASTER_LKP_ODR0_IDLE"},
	{0, "DFX_MASTER_LKP_ODR1_CTX_ADDR"},
	{0, "DFX_MASTER_LKP_ODR1_CTX0"},
	{0, "DFX_MASTER_LKP_ODR1_CTX1"},
	{0, "DFX_MASTER_LKP_ODR1_CTX2"},
	{0, "DFX_MASTER_LKP_ODR1_CTX3"},
	{0, "DFX_MASTER_LKP_ODR1_OUTSTANDING"},
	{0, "DFX_MASTER_LKP_ODR1_IDLE"},
	{0, "DFX_MASTER_AMB0_CTX_ADDR"},
	{0, "DFX_MASTER_AMB0_CTX0"},
	{0, "DFX_MASTER_AMB0_CTX1"},
	{0, "DFX_MASTER_AMB0_CTX2"},
	{0, "DFX_MASTER_AMB0_CTX3"},
	{0, "DFX_MASTER_AMB0_IDLE"},
	{0, "DFX_MASTER_AMB1_CTX_ADDR"},
	{0, "DFX_MASTER_AMB1_CTX0"},
	{0, "DFX_MASTER_AMB1_CTX1"},
	{0, "DFX_MASTER_AMB1_CTX2"},
	{0, "DFX_MASTER_AMB1_CTX3"},
	{0, "DFX_MASTER_AMB1_IDLE"},
	{0, "DFX_MASTER_ICG_EN"},
	{0, "DFX_MASTER_US_TIME_SET"},
	{0, "DFX_MASTER_AMB0_RD_MAX_DELAY"},
	{0, "DFX_MASTER_AMB0_RD_MAX_DELAY_CLR"},
	{0, "DFX_MASTER_AMB0_WR_MAX_DELAY"},
	{0, "DFX_MASTER_AMB0_WR_MAX_DELAY_CLR"},
	{0, "DFX_MASTER_AMB1_RD_MAX_DELAY"},
	{0, "DFX_MASTER_AMB1_RD_MAX_DELAY_CLR"},
	{0, "DFX_MASTER_AMB1_WR_MAX_DELAY"},
	{0, "DFX_MASTER_AMB1_WR_MAX_DELAY_CLR"},
	{0, "DFX_AMB0_ENTRY_MAX_OTSD"},
	{0, "DFX_AMB0_CURR_AVA_LANTENCY"},
	{0, "DFX_AMB1_ENTRY_MAX_OTSD"},
	{0, "DFX_AMB1_CURR_AVA_LANTENCY"},
};

static struct ubus_dumpreg_info g_reg_table_dl_mac[] = {
	{0, "UB_DLMAC_GLB.LQC_CORE_CLK_FLG"},
};

static struct ubus_dumpreg_info g_reg_table_dl[] = {
	{0, "UB_DL_PORT_REG.ST_LINK_STATE"},
	{0, "UB_DL_PORT_REG.ST_CRD_0"},
	{0, "UB_DL_PORT_REG.ST_CRD_1"},
	{0, "UB_DL_PORT_REG.ST_CRD_2"},
	{0, "UB_DL_PORT_REG.ST_CRD_3"},
	{0, "UB_DL_PORT_REG.ST_CRD_4"},
	{0, "UB_DL_PORT_REG.ST_CRD_5"},
	{0, "UB_DL_PORT_REG.ST_CRD_6"},
	{0, "UB_DL_PORT_REG.ST_CRD_7"},
	{0, "UB_DL_PORT_REG.ST_CRD_8"},
	{0, "UB_DL_PORT_REG.ST_CRD_9"},
	{0, "UB_DL_PORT_REG.ST_CRD_10"},
	{0, "UB_DL_PORT_REG.ST_CRD_11"},
	{0, "UB_DL_PORT_REG.ST_CRD_12"},
};

static struct ubus_dumpreg_info g_reg_table_nl_pa[] = {
	{0, "PA_OUT_ALL_PKT"},
	{0, "PA_OUT_KEY_NUM"},
	{0, "IGU_OUTER_ERR_STS"},
	{0, "IGU_INNER_ERR_STS"},
	{0, "PA_OUT_NON_TUN_L2_PKT"},
	{0, "PA_OUT_NON_TUN_L3_PKT"},
	{0, "PA_OUT_NON_TUN_L4_PKT"},
	{0, "PA_OUT_TUN_L2_PKT"},
	{0, "PA_OUT_TUN_L3_PKT"},
	{0, "PA_OUT_TUN_L4_PKT"},
	{0, "PA_OUT_NON_TUN_PKT"},
	{0, "PA_OUT_TUN_PKT"},
	{0, "PA_OUT_EL3E_PKT"},
	{0, "PA_OUT_EL4E_PKT"},
	{0, "PA_OUT_L3E_PKT"},
	{0, "PA_OUT_L4E_PKT"},
	{0, "PA_OUT_UB_PKT"},
	{0, "PA_OUT_OUTER_UDP0_PKT"},
	{0, "PA_OUT_INNER_UDP0_PKT"},
	{0, "PA_UB_COM_ERR_STS"},
	{0, "PA_UB_FLD_ERR_STS"},
	{0, "PA_UB_LEN_ERR_STS"},
	{0, "PA_UB_GLB_IPV4_PKT_CNT"},
	{0, "PA_UB_GLB_IPV6_PKT_CNT"},
	{0, "PA_UNIC_IPV4_PKT_CNT"},
	{0, "PA_UNIC_IPV6_PKT_CNT"},
	{0, "PA_UNIC_NCP_PKT_CNT"},
	{0, "PA_UB_CLAN_PKT_CNT"},
	{0, "PA_UB_UMOC_CTPH_CNT"},
	{0, "PA_UB_UMOC_NTPH_CNT"},
	{0, "PA_UB_MEM_PKT_CNT"},
	{0, "PA_UNKNOWN_PKT_CNT"},
	{0, "PA_DROP_IND_CNT"},
	{0, "PA_ERR_PKT_CNT"},
	{0, "PA_NOC_IND_CNT"},
	{0, "PA_LPBK_CNT"},
	{0, "PA_OUT_ERR_CNT"},
	{0, "PA_LEN_ERR_CNT"},
	{0, "PA_IN_ERR_CNT"},
};

static struct ubus_dumpreg_info g_reg_table_nl_pb[] = {
	{0, "PA_OUT_ALL_PKT"},
	{0, "PA_OUT_KEY_NUM"},
	{0, "IGU_OUTER_ERR_STS"},
	{0, "IGU_INNER_ERR_STS"},
	{0, "PA_OUT_NON_TUN_L2_PKT"},
	{0, "PA_OUT_NON_TUN_L3_PKT"},
	{0, "PA_OUT_NON_TUN_L4_PKT"},
	{0, "PA_OUT_TUN_L2_PKT"},
	{0, "PA_OUT_TUN_L3_PKT"},
	{0, "PA_OUT_TUN_L4_PKT"},
	{0, "PA_OUT_NON_TUN_PKT"},
	{0, "PA_OUT_TUN_PKT"},
	{0, "PA_OUT_EL3E_PKT"},
	{0, "PA_OUT_EL4E_PKT"},
	{0, "PA_OUT_L3E_PKT"},
	{0, "PA_OUT_L4E_PKT"},
	{0, "PA_OUT_UB_PKT"},
	{0, "PA_OUT_OUTER_UDP0_PKT"},
	{0, "PA_OUT_INNER_UDP0_PKT"},
	{0, "PA_UB_COM_ERR_STS"},
	{0, "PA_UB_FLD_ERR_STS"},
	{0, "PA_UB_LEN_ERR_STS"},
	{0, "PA_UB_GLB_IPV4_PKT_CNT"},
	{0, "PA_UB_GLB_IPV6_PKT_CNT"},
	{0, "PA_UNIC_IPV4_PKT_CNT"},
	{0, "PA_UNIC_IPV6_PKT_CNT"},
	{0, "PA_UNIC_NCP_PKT_CNT"},
	{0, "PA_UB_CLAN_PKT_CNT"},
	{0, "PA_UB_UMOC_CTPH_CNT"},
	{0, "PA_UB_UMOC_NTPH_CNT"},
	{0, "PA_UB_MEM_PKT_CNT"},
	{0, "PA_UNKNOWN_PKT_CNT"},
	{0, "PA_DROP_IND_CNT"},
	{0, "PA_ERR_PKT_CNT"},
	{0, "PA_PACK_PKT_CNT"},
	{0, "PA_HOST_PKT_CNT"},
	{0, "PA_IMP_PKT_CNT"},
	{0, "PA_MAR_PKT_CNT"},
	{0, "PA_LINK_PKT_CNT"},
	{0, "PA_NOC_PKT_CNT"},
	{0, "PA_RT_ERR_CNT"},
	{0, "PA_OUT_ERR_CNT"},
	{0, "PA_LEN_ERR_CNT"},
	{0, "PA_IN_ERR_CNT"},
};

static struct ubus_dumpreg_info g_reg_table_tpp[] = {
	{0, "TPP_STA"},
	{0, "TPP_ALM"},
	{0, "TPP_ALM1"},
	{0, "TPP_MAR_ALLOC_CELL_NUM_BA0"},
	{0, "TPP_MAR_ALLOC_CELL_NUM_BA1"},
	{0, "TPP_MAR_ALLOC_CELL_NUM_BA2"},
	{0, "TPP_MAR_ALLOC_CELL_NUM_BA3"},
	{0, "TPP_MAR_ALLOC_CELL_NUM_BA4"},
	{0, "TPP_TAI_ALLOC_CELL_NUM"},
	{0, "TPP_LSAR_ALLOC_CELL_NUM"},
	{0, "TPP_LSAR_ALLOC_CELL_NUM_BA1"},
	{0, "TPP_LSAR_ALLOC_CELL_NUM_BA2"},
	{0, "TPP_LSAR_ALLOC_CELL_NUM_BA3"},
	{0, "TPP_LSAR_ALLOC_CELL_NUM_BA4"},
	{0, "TPP_SSU_BACK_CELL_NUM_NL0"},
	{0, "TPP_SSU_BACK_CELL_NUM_NL1"},
	{0, "TPP_SSU_BACK_CELL_NUM_NL2"},
	{0, "TPP_SSU_BACK_CELL_NUM_NL3"},
	{0, "TPP_SSU_BACK_CELL_NUM_NL4"},
	{0, "TPP_LSAD_BACK_CELL_NUM_NL0"},
	{0, "TPP_LSAD_BACK_CELL_NUM_NL1"},
	{0, "TPP_LSAD_BACK_CELL_NUM_NL2"},
	{0, "TPP_LSAD_BACK_CELL_NUM_NL3"},
	{0, "TPP_LSAD_BACK_CELL_NUM_NL4"},
	{0, "TPP_TPP_TQS_SGE_RLS_CNT"},
	{0, "TPP_TRANS_GLB_PKTN"},
	{0, "TPP_TRANS_GLB_PKTN_BA23"},
	{0, "TPP_TRANS_GLB_PKTN_BA4"},
	{0, "TPP_TRANS_CLAN_PKTN"},
	{0, "TPP_TRANS_CLAN_PKTN_BA23"},
	{0, "TPP_TRANS_CLAN_PKTN_BA4"},
	{0, "TPP_TRANS_LSA_PKTN"},
	{0, "TPP_TRANS_LSA_PKTN_BA23"},
	{0, "TPP_TRANS_LSA_PKTN_BA4"},
	{0, "TPP_DFX0"},
	{0, "TPP_DFX1"},
	{0, "TPP_DFX1_1"},
	{0, "TPP_DFX2"},
	{0, "TPP_DFX3"},
	{0, "TPP_DFX100"},
	{0, "TPP_DFX4"},
	{0, "TPP_DFX5"},
	{0, "TPP_DFX6"},
	{0, "TPP_DFX7"},
	{0, "TPP_DFX8"},
	{0, "TPP_DFX9"},
	{0, "TPP_DFX10"},
	{0, "TPP_DFX11"},
	{0, "TPP_DFX12"},
	{0, "TPP_DFX13"},
	{0, "TPP_DFX14"},
	{0, "TPP_DFX15"},
	{0, "TPP_DFX16"},
	{0, "TPP_DFX17"},
	{0, "TPP_DFX18"},
	{0, "TPP_DFX19"},
	{0, "TPP_DFX20"},
	{0, "TPP_DFX21"},
	{0, "TPP_DFX22"},
	{0, "TPP_DFX23"},
	{0, "TPP_DFX24"},
	{0, "TPP_DFX25"},
	{0, "TPP_DFX26"},
	{0, "TPP_DFX27"},
	{0, "TPP_DFX28"},
	{0, "TPP_DFX29"},
	{0, "TPP_DFX30"},
	{0, "TPP_DFX31"},
	{0, "TPP_DFX32"},
	{0, "TPP_DFX33"},
	{0, "TPP_DFX34"},
	{0, "TPP_DFX35"},
	{0, "TPP_DFX36"},
	{0, "TPP_DFX37"},
	{0, "TPP_DFX38"},
	{0, "TPP_DFX39"},
	{0, "TPP_DFX40"},
	{0, "TPP_DFX41"},
	{0, "TPP_DFX42"},
	{0, "TPP_DFX43"},
	{0, "TPP_DFX44"},
	{0, "TPP_DFX45"},
	{0, "TPP_DFX46"},
	{0, "TPP_DFX47"},
	{0, "TPP_DFX48"},
	{0, "TPP_DFX49"},
	{0, "TPP_DFX50"},
	{0, "TPP_DFX51"},
	{0, "TPP_DFX52"},
	{0, "TPP_DFX53"},
	{0, "TPP_DFX54"},
	{0, "TPP_DFX55"},
	{0, "TPP_DFX56"},
	{0, "TPP_DFX57"},
	{0, "TPP_DFX58"},
	{0, "TPP_DFX59"},
	{0, "TPP_DFX60"},
	{0, "TPP_DFX61"},
	{0, "TPP_DFX62"},
	{0, "TPP_DFX63"},
	{0, "TPP_DFX64"},
	{0, "TPP_DFX65"},
	{0, "TPP_DFX66"},
	{0, "TPP_DFX67"},
	{0, "TPP_DFX68"},
	{0, "TPP_DFX69"},
	{0, "TPP_DFX70"},
	{0, "TPP_DFX71"},
	{0, "TPP_DFX72"},
	{0, "TPP_DFX73"},
	{0, "TPP_DFX74"},
	{0, "TPP_DFX75"},
	{0, "TPP_DFX76"},
	{0, "TPP_DFX77"},
	{0, "TPP_DFX78"},
	{0, "TPP_DFX79"},
	{0, "TPP_DFX80"},
	{0, "TPP_DFX81"},
	{0, "TPP_DFX82"},
	{0, "TPP_DFX83"},
	{0, "TPP_DFX84"},
	{0, "TPP_DFX85"},
	{0, "TPP_DFX86"},
	{0, "TPP_DFX87"},
	{0, "TPP_DFX88"},
	{0, "TPP_DFX89"},
	{0, "TPP_DFX90"},
	{0, "TPP_DFX91"},
	{0, "TPP_DFX92"},
	{0, "TPP_DFX93"},
	{0, "TPP_DFX94"},
	{0, "TPP_DFX95"},
	{0, "TPP_DFX96"},
	{0, "TPP_DFX97"},
	{0, "TPP_DFX98"},
	{0, "TPP_DFX99"},
	{0, "TPP_DFX101"},
	{0, "TPP_DFX102"},
	{0, "TPP_DFX103"},
	{0, "TPP_DFX104"},
	{0, "TPP_DFX105"},
	{0, "TPP_DFX106"},
	{0, "TPP_DFX107"},
	{0, "TPP_DFX108"},
	{0, "TPP_DFX109"},
	{0, "TPP_DFX110"},
	{0, "TPP_DFX112"},
	{0, "TPP_DFX113"},
	{0, "TPP_DFX114"},
	{0, "TPP_DFX115"},
	{0, "TPP_DFX116"},
	{0, "TPP_DFX117"},
	{0, "TPP_DFX118"},
	{0, "TPP_DFX119"},
	{0, "TPP_DFX120"},
	{0, "TPP_DFX121"},
};

static struct ubus_dumpreg_info g_reg_table_tp[] = {
	{0, "TP_RHP_BA0_PORT0_GLB_RM_PKT_CNT"},
	{0, "TP_RHP_BA0_PORT2_GLB_RM_PKT_CNT"},
	{0, "TP_RHP_BA1_PORT0_GLB_RM_PKT_CNT"},
	{0, "TP_RHP_BA1_PORT2_GLB_RM_PKT_CNT"},
	{0, "TP_RHP_BA2_PORT0_GLB_RM_PKT_CNT"},
	{0, "TP_RHP_BA2_PORT2_GLB_RM_PKT_CNT"},
	{0, "TP_RHP_BA3_PORT0_GLB_RM_PKT_CNT"},
	{0, "TP_RHP_BA3_PORT2_GLB_RM_PKT_CNT"},
	{0, "TP_RHP_BA4_PORT0_GLB_RM_PKT_CNT"},
	{0, "TP_RHP_BA0_PORT0_GLB_RC_PKT_CNT"},
	{0, "TP_RHP_BA0_PORT2_GLB_RC_PKT_CNT"},
	{0, "TP_RHP_BA1_PORT0_GLB_RC_PKT_CNT"},
	{0, "TP_RHP_BA1_PORT2_GLB_RC_PKT_CNT"},
	{0, "TP_RHP_BA2_PORT0_GLB_RC_PKT_CNT"},
	{0, "TP_RHP_BA2_PORT2_GLB_RC_PKT_CNT"},
	{0, "TP_RHP_BA3_PORT0_GLB_RC_PKT_CNT"},
	{0, "TP_RHP_BA3_PORT2_GLB_RC_PKT_CNT"},
	{0, "TP_RHP_BA4_PORT0_GLB_RC_PKT_CNT"},
	{0, "TP_RHP_BA0_PORT0_CLAN_RM_PKT_CNT"},
	{0, "TP_RHP_BA0_PORT2_CLAN_RM_PKT_CNT"},
	{0, "TP_RHP_BA1_PORT0_CLAN_RM_PKT_CNT"},
	{0, "TP_RHP_BA1_PORT2_CLAN_RM_PKT_CNT"},
	{0, "TP_RHP_BA2_PORT0_CLAN_RM_PKT_CNT"},
	{0, "TP_RHP_BA2_PORT2_CLAN_RM_PKT_CNT"},
	{0, "TP_RHP_BA3_PORT0_CLAN_RM_PKT_CNT"},
	{0, "TP_RHP_BA3_PORT2_CLAN_RM_PKT_CNT"},
	{0, "TP_RHP_BA4_PORT0_CLAN_RM_PKT_CNT"},
	{0, "TP_RHP_BA0_PORT0_CLAN_RC_PKT_CNT"},
	{0, "TP_RHP_BA0_PORT2_CLAN_RC_PKT_CNT"},
	{0, "TP_RHP_BA1_PORT0_CLAN_RC_PKT_CNT"},
	{0, "TP_RHP_BA1_PORT2_CLAN_RC_PKT_CNT"},
	{0, "TP_RHP_BA2_PORT0_CLAN_RC_PKT_CNT"},
	{0, "TP_RHP_BA2_PORT2_CLAN_RC_PKT_CNT"},
	{0, "TP_RHP_BA3_PORT0_CLAN_RC_PKT_CNT"},
	{0, "TP_RHP_BA3_PORT2_CLAN_RC_PKT_CNT"},
	{0, "TP_RHP_BA4_PORT0_CLAN_RC_PKT_CNT"},
	{0, "TP_RHP_BA0_PORT0_UNIC_PKT_CNT"},
	{0, "TP_RHP_BA0_PORT2_UNIC_PKT_CNT"},
	{0, "TP_RHP_BA1_PORT0_UNIC_PKT_CNT"},
	{0, "TP_RHP_BA1_PORT2_UNIC_PKT_CNT"},
	{0, "TP_RHP_BA2_PORT0_UNIC_PKT_CNT"},
	{0, "TP_RHP_BA2_PORT2_UNIC_PKT_CNT"},
	{0, "TP_RHP_BA3_PORT0_UNIC_PKT_CNT"},
	{0, "TP_RHP_BA3_PORT2_UNIC_PKT_CNT"},
	{0, "TP_RHP_BA4_PORT0_UNIC_PKT_CNT"},
	{0, "TP_RHP_BA0_PORT0_UD_PKT_CNT"},
	{0, "TP_RHP_BA0_PORT2_UD_PKT_CNT"},
	{0, "TP_RHP_BA1_PORT0_UD_PKT_CNT"},
	{0, "TP_RHP_BA1_PORT2_UD_PKT_CNT"},
	{0, "TP_RHP_BA2_PORT0_UD_PKT_CNT"},
	{0, "TP_RHP_BA2_PORT2_UD_PKT_CNT"},
	{0, "TP_RHP_BA3_PORT0_UD_PKT_CNT"},
	{0, "TP_RHP_BA3_PORT2_UD_PKT_CNT"},
	{0, "TP_RHP_BA4_PORT0_UD_PKT_CNT"},
	{0, "TP_RHP_BA4_PORT2_GLB_RM_PKT_CNT"},
	{0, "TP_RHP_BA4_PORT2_GLB_RC_PKT_CNT"},
	{0, "TP_RHP_BA4_PORT2_CLAN_RM_PKT_CNT"},
	{0, "TP_RHP_BA4_PORT2_CLAN_RC_PKT_CNT"},
	{0, "TP_RHP_BA4_PORT2_UNIC_PKT_CNT"},
	{0, "TP_RHP_BA4_PORT2_UD_PKT_CNT"},

};

static struct ubus_dumpreg_info g_reg_table_lqc_ta[] = {
	{0, "LQC_TA_MRD_ERR"},
	{0, "LQC_TA_MRD_DB_TIMEOUT_DROP_CNT"},
	{0, "LQC_TA_MRD_MB_ISSUE_CNT"},
	{0, "LQC_TA_MRD_MB_EXEC_CNT"},
	{0, "LQC_TA_MRD_DSQE_ISSUE_CNT"},
	{0, "LQC_TA_MRD_DSQE_EXEC_CNT"},
	{0, "LQC_TA_MRD_DSQE_DROP_CNT"},
	{0, "LQC_TA_MRD_JFSDB_ISSUE_CNT"},
	{0, "LQC_TA_MRD_JFSDB_EXEC_CNT"},
	{0, "LQC_TA_MRD_JFCDB_ISSUE_CNT"},
	{0, "LQC_TA_MRD_JFCDB_EXEC_CNT"},
	{0, "LQC_TA_MRD_EQDB_ISSUE_CNT"},
	{0, "LQC_TA_MRD_EQDB_EXEC_CNT"},
	{0, "LQC_TA_MRD_FLR_CNT"},
	{0, "LQC_TA_MRD_DESTROY_CNT"},
	{0, "LQC_TA_MRD_DB_CFG_ADDR_ERR_CNT"},
	{0, "LQC_TA_MRD_RW_ADDR_ERR_CNT"},
	{0, "LQC_TA_MRD_MB_CMD_ERR_CNT"},
	{0, "LQC_TA_MRD_MB_AXI_ERR_INFO"},
	{0, "LQC_TA_MRD_TP_FLUSH_ISSUE_CNT"},
	{0, "LQC_TA_MRD_TA_FLUSH_ISSUE_CNT"},
	{0, "LQC_TA_MRD_TP_FLUSH_COMP_CNT"},
	{0, "LQC_TA_MRD_TA_FLUSH_COMP_CNT"},
	{0, "LQC_TA_MRD_BYPASS_WQE_EXEC_CNT"},
	{0, "LQC_TA_MRD_DSQE_INTR_CNT"},
	{0, "LQC_TA_MRD_JFSDB_INTR_CNT"},
	{0, "LQC_TA_MRD_JFCDB_INTR_CNT"},
	{0, "LQC_TA_MRD_EQDB_INTR_CNT"},
	{0, "LQC_TA_MRD_FE0_NPA_MB_CMD_CNT"},
	{0, "LQC_TA_MRD_DB_SEC_AUTH_FAIL_CNT"},
	{0, "LQC_TA_MRD_DSQE_DB_ISSUE_CNT"},
	{0, "LQC_TA_MRD_MB_BUFF_FULL"},
	{0, "LQC_TA_MRD_MB_BUFF_EMPTY"},
	{0, "LQC_TA_MRD_DYNAMIC_CLK_STATUS"},
	{0, "LQC_TA_MRD_MB_STATE"},
	{0, "LQC_TA_MRD_MB_CMD_INPIPE_NUM"},
	{0, "LQC_TA_MRD_AXI_OST_NUM"},
};

static struct ubus_dumpreg_info g_reg_table_eip[] = {
	{0, "EIP_STA0"},
	{0, "EIP_STA1"},
	{0, "EIP_STA2"},
	{0, "EIP_STA3"},
	{0, "EIP_AXI_OUTSTANDING"},
	{0, "EIP_TP_AE_CNT_I"},
	{0, "EIP_CQM_AE_CNT_I"},
	{0, "EIP_TQEP_AE_CNT_I"},
	{0, "EIP_MRD_AE_CNT_I"},
	{0, "EIP_CQCB_CE_CNT_I"},
	{0, "EIP_CQM_CE_VLD_CNT_I"},
	{0, "EIP_CE_OVERFLOW_CNT_O"},
	{0, "EIP_DB_ERROR_CNT_O"},
	{0, "EIP_MB_ERROR_CNT_O"},
	{0, "EIP_ACE_DISCARD_CNT_O"},
	{0, "EIP_AE_OVERFLOW_CNT_O"},
	{0, "EIP_AE_VLD_CNT_O"},
	{0, "EIP_IRQ_CNT"},
};

static struct ubus_dumpreg_table g_dump_info_glb[] = {
	{HIKP_ARRAY_SIZE(g_reg_table_rxdma), g_reg_table_rxdma},
	{HIKP_ARRAY_SIZE(g_reg_table_txdma), g_reg_table_txdma},
	{HIKP_ARRAY_SIZE(g_reg_table_tx_cnt), g_reg_table_tx_cnt},
	{HIKP_ARRAY_SIZE(g_reg_table_ba_dfx), g_reg_table_ba_dfx},
	{HIKP_ARRAY_SIZE(g_reg_table_dl_mac), g_reg_table_dl_mac},
	{HIKP_ARRAY_SIZE(g_reg_table_nl_pa), g_reg_table_nl_pa},
	{HIKP_ARRAY_SIZE(g_reg_table_nl_pb), g_reg_table_nl_pb},
	{HIKP_ARRAY_SIZE(g_reg_table_tpp), g_reg_table_tpp},
	{HIKP_ARRAY_SIZE(g_reg_table_tp), g_reg_table_tp},
	{HIKP_ARRAY_SIZE(g_reg_table_lqc_ta), g_reg_table_lqc_ta},
	{HIKP_ARRAY_SIZE(g_reg_table_eip), g_reg_table_eip},
	{HIKP_ARRAY_SIZE(g_reg_table_dl), g_reg_table_dl},
};

static int dfx_rsp_data_check(struct hikp_cmd_ret *cmd_ret)
{
	int ret;

	ret = hikp_rsp_normal_check(cmd_ret);
	if (ret) {
		printf("dfx dump respond data check failed, ret: %d.\n", ret);
		return ret;
	}

	return 0;
}

static void ubus_dumpreg_print(const uint32_t *data, uint32_t data_num,
			       struct ubus_dumpreg_table *table, uint32_t size)
{
	struct ubus_dumpreg_info *info;
	uint32_t i, j, data_i = 0;

	for (i = 0; i < size; i++) {
		info = table[i].dump_info;
		printf("  reg table[%u]:\n", i);
		for (j = 0; j < table[i].size && data_i < data_num; j++, data_i++) {
			info[j].val = data[data_i];
			printf("    reg name: %-40s\tvalue: 0x%08x\n",
			       info[j].name, info[j].val);
		}
	}
}

static void ubus_dump_reg(uint32_t *data, uint32_t data_num)
{
	uint32_t expect_data_num = 0;
	uint32_t i;

	for (i = 0; i < HIKP_ARRAY_SIZE(g_dump_info_glb); i++)
		expect_data_num += g_dump_info_glb[i].size;

	if (expect_data_num != data_num) {
		printf("data num check failed.\n");
		return;
	}

	ubus_dumpreg_print(data, data_num, g_dump_info_glb,
			   HIKP_ARRAY_SIZE(g_dump_info_glb));
}

static void dfx_info_print(struct hikp_cmd_ret *cmd_ret, uint32_t port_id)
{
	printf("===========ubus dfx dump===============\n");
	printf("Command Version[%u], port_id[%u]\n\n", cmd_ret->version, port_id);
	printf("ubus dfx info detail:\n");
	ubus_dump_reg(cmd_ret->rsp_data, cmd_ret->rsp_data_num);
	printf("===========ubus dfx dump end===========\n");
}

int ubus_dfx_dump_show_execute(uint32_t port_id)
{
	struct ubus_dump_req_para req_data = { 0 };
	struct hikp_cmd_ret *cmd_ret = NULL;
	struct hikp_cmd_header req_header;
	int ret;

	req_data.port_id = port_id;
	hikp_cmd_init(&req_header, UBUS_MOD, UBUS_DFX_DUMP, DFX_DUMP_SHOW);
	cmd_ret = hikp_cmd_alloc(&req_header, &req_data, sizeof(req_data));
	ret = dfx_rsp_data_check(cmd_ret);
	if (ret) {
		printf("ubus dfx dump cmd_ret check failed, ret: %d.\n", ret);
		goto free_cmd_ret;
	}

	dfx_info_print(cmd_ret, port_id);
free_cmd_ret:
	hikp_cmd_free(&cmd_ret);

	return ret;
}
