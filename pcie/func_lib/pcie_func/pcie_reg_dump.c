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

#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include "hikptdev_plug.h"
#include "os_common.h"
#include "pcie_common.h"
#include "pcie_reg_dump.h"

FILE *g_pcie_dumpreg_fd = NULL;
char dumpreg_log_file[MAX_LOG_NAME_LEN + 1] = {0};

struct pcie_dumpreg_info g_reg_table_tl[] = {
	{0, "TL_ASPM_IDLE_CNT"},
	{0, "TL_ASPM_IDLE_EN"},
	{0, "TL_PM_DC_CTRL"},
	{0, "TL_PM_STATE"},
	{0, "TL_PM_UC_CTRL"},
	{0, "TL_ENTER_L0_CTRL"},
	{0, "TL_PM_TIMEOUT_CTRL"},
	{0, "TL_INT_STATUS0"},
	{0, "TL_INT_STATUS1"},
	{0, "TL_RX_POSTED_CREDIT_DF"},
	{0, "TL_RX_NON_POSTED_CREDIT_DF"},
	{0, "TL_RX_CPL_CREDIT_DF"},
	{0, "TL_RX_CDT_INI_UP_DF"},
	{0, "RX_ASYN_STONE_FIFO_STATUS"},
	{0, "TL_RX_ERR_STATUS"},
	{0, "TL_TX_VC0_P_FC_LEFT"},
	{0, "TL_TX_VC0_NP_FC_LEFT"},
	{0, "TL_TX_VC0_CPL_FC_LEFT"},
	{0, "TL_TX_VC1_P_FC_LEFT"},
	{0, "TL_TX_ORDER_P_NUM"},
	{0, "TL_TX_ORDER_NP_CPL_NUM"},
	{0, "TL_RX_VC0_POST_CREDIT_LEFT"},
	{0, "TL_RX_VC0_NONPOST_CREDIT_LEFT"},
	{0, "TL_RX_VC0_CPL_CREDIT_LEFT"},
	{0, "RX_RX_BUFFER_STATUS"},
	{0, "TL_RX_POSTED_CREDIT"},
	{0, "TL_RX_NON_POSTED_CREDIT"},
	{0, "TL_RX_CPL_CREDIT"},
	{0, "TL_RX_ERR_CNT"},
	{0, "TL_RX_NULL_CNT"},
	{0, "TL_RX_UR_TLP_CNT"},
	{0, "TL_RX_TOTAL_CNT"},
	{0, "TL_RX_RCV_PNP_CNT"},
	{0, "TL_RX_RCV_CPL_CNT"},
	{0, "TL_RX_POST_CNT"},
	{0, "TL_RX_NONPOST_CNT"},
	{0, "TL_RX_CPL_CNT"},
	{0, "TL_RX_LOC_TLP_CNT"},
	{0, "TL_CFGSPACE_BDF"},
	{0, "TL_TX_UR_CNT"},
};

struct pcie_dumpreg_info g_reg_table_dl[] = {
	{0, "DFX_LCRC_ERR_NUM"},
	{0, "DFX_DCRC_ERR_NUM"},
	{0, "DFX_FSM_STATE"},
	{0, "DL_INT_STATUS"},
	{0, "DFX_MAC_BP_TIMER"},
	{0, "DFX_RETRY_CNT"},
	{0, "DL_MAC_RETRAIN_CNT"},
	{0, "DFX_DLLP_RX_COUNT_NUM"},
	{0, "DFX_DLLP_TX_COUNT_NUM"},
	{0, "DFX_RX_NAK_COUNT"},
	{0, "DFX_RX_BAD_DLLP_TYPE"},
	{0, "DL_FC_INIT_ERR_STATUS"},
};

struct pcie_dumpreg_info g_reg_table_mac[] = {
	{0, "MAC_REQ_TX_LINK_NUM"},
	{0, "MAC_REG_MAC_INT_STATUS"},
	{0, "MAC_REG_MAC_INT_MASK"},
	{0, "MAC_REG_TEST_COUNTER"},
	{0, "MAC_REG_LINK_INFO"},
	{0, "MAC_REG_SYMBOL_UNLOCL_COUNTER"},
	{0, "MAC_REG_MAC_INT_RO"},
	{0, "MAC_REG_ENTER_L1L2_TIMEOUT_VAL"},
	{0, "MAC_PCS_RX_ERR_CNT"},
	{0, "MAC_RX_ERR_CHECK_EN"},
	{0, "MAC_TRACE_2BIT_ECC_CNT"},
	{0, "MAC_TRACE_1BIT_ECC_CNT"},
	{0, "MAC_LTSSM_TIMEOUT_ENABLE"},
	{0, "MAC_FRAMING_ERR_CNT"},
	{0, "MAC_FRAMING_ERR_CTRL"},
	{0, "MAC_LEAVE_L0_INFO"},
	{0, "MAC_INT_CE_NFE_SEL"},
	{0, "MAC_REG_NI_INT_RO"},
	{0, "MAC_REG_FE_INT_RO"},
	{0, "MAC_REG_CE_INT_RO"},
	{0, "MAC_REG_NFE_INT_RO"},
	{0, "MAC_REG_EQ_FIX_LP_TX_PRESET"},
	{0, "MAC_REG_ESM_32G_EQ_FIX_LP_TX_PRESET"},
	{0, "MAC_REG_ADJ_HILINK_MODE_EN"},
	{0, "MAC_REG_EQ_OPT_TX_PRESET_1"},
	{0, "MAC_REG_LP_GEN3_TX_PRESET_P1_2"},
	{0, "MAC_REG_GEN3_EQ_OPT_TX_PRESET_2"},
	{0, "MAC_REG_GEN4_EQ_OPT_TX_PRESET_1"},
	{0, "MAC_REG_GEN4_EQ_OPT_TX_PRESET_2"},
	{0, "MAC_REG_LP_GEN4_TX_PRESET_P1_1"},
	{0, "MAC_REG_LP_GEN4_TX_PRESET_P1_2"},
	{0, "MAC_REG_DEBUG_PIPE1"},
	{0, "MAC_REG_DEBUG_PIPE2"},
	{0, "MAC_REG_DEBUG_PIPE3"},
	{0, "MAC_REG_DEBUG_PIPE5"},
	{0, "MAC_REG_DEBUG_PIPE7"},
	{0, "MAC_REG_DEBUG_PIPE8"},
	{0, "MAC_REG_DEBUG_PIPE9"},
	{0, "MAC_REG_DEBUG_PIPE10"},
	{0, "MAC_REG_DEBUG_PIPE11"},
	{0, "DFX_APB_LANE_ERROR_STATUS_0"},
	{0, "DFX_APB_LANE_ERROR_STATUS_1"},
	{0, "MAC_REG_PHY_RXDATA_TS_REG"},
	{0, "MAC_LTSSM_TRACER_CFG0_REG"},
	{0, "MAC_POWERDOWN_VALUE_REG"},
};

struct pcie_dumpreg_info g_reg_table_iob_tx[] = {
	{0, "IOB_TX_ECAM_CONTROL0"},
	{0, "IOB_TX_ECAM_CONTROL1"},
	{0, "IOB_TX_ECAM_BASE_ADDR_L"},
	{0, "IOB_TX_ECAM_BASE_ADDR_H"},
	{0, "IOB_TX_CXL_BASE_BUS_0"},
	{0, "IOB_TX_CXL_RCRB_BASE_L_0"},
	{0, "IOB_TX_CXL_RCRB_BASE_H_0"},
	{0, "IOB_TX_INT_STATUS0"},
	{0, "IOB_TX_INT_STATUS1"},
	{0, "IOB_TX_INT_STATUS2"},
	{0, "IOB_TX_INT_STATUS3"},
	{0, "IOB_TX_INT_STATUS4"},
	{0, "IOB_TX_INT_STATUS5"},
	{0, "IOB_TX_INT_RO0"},
	{0, "IOB_TX_INT_RO1"},
	{0, "IOB_TX_INT_RO2"},
	{0, "IOB_TX_INT_RO3"},
	{0, "IOB_TX_INT_RO4"},
	{0, "IOB_TX_INT_RO5"},
	{0, "IOB_TX_INT_SEVERITY0"},
	{0, "IOB_TX_INT_SEVERITY1"},
	{0, "IOB_TX_INT_SEVERITY2"},
	{0, "IOB_TX_INT_SEVERITY3"},
	{0, "IOB_TX_INT_SEVERITY4"},
	{0, "IOB_TX_INT_SEVERITY5"},
	{0, "IOB_TX_TCS_DEC_ERR_INFO_L"},
	{0, "IOB_TX_TCS_DEC_ERR_INFO_H"},
	{0, "DFX_IOB_TX_P_CNT_0"},
	{0, "DFX_IOB_TX_P_CNT_1"},
	{0, "DFX_IOB_TX_P_CNT_2"},
	{0, "DFX_IOB_TX_P_CNT_3"},
	{0, "DFX_IOB_TX_P_CNT_4"},
	{0, "DFX_IOB_TX_NP_CNT_0"},
	{0, "DFX_IOB_TX_NP_CNT_1"},
	{0, "DFX_IOB_TX_NP_CNT_2"},
	{0, "DFX_IOB_TX_NP_CNT_3"},
	{0, "DFX_IOB_TX_NP_CNT_4"},
	{0, "DFX_IOB_TX_NP_CNT_5"},
	{0, "DFX_IOB_TX_NP_CNT_6"},
	{0, "DFX_IOB_TX_NP_CNT_7"},
	{0, "DFX_IOB_TX_NP_CNT_8"},
	{0, "DFX_IOB_TX_NP_CNT_9"},
	{0, "DFX_IOB_TX_NP_CNT_10"},
	{0, "DFX_IOB_TX_NP_CNT_11"},
	{0, "DFX_IOB_TX_CPL_CNT_0"},
	{0, "DFX_IOB_TX_CPL_CNT_1"},
	{0, "DFX_IOB_TX_CPL_CNT_2"},
	{0, "DFX_IOB_TX_REQ_CNT"},
	{0, "DFX_IOB_TX_STATUS0"},
	{0, "DFX_IOB_TX_STATUS1"},
	{0, "DFX_IOB_TX_STATUS2"},
	{0, "DFX_IOB_TX_STATUS3"},
	{0, "DFX_IOB_TX_STATUS4"},
	{0, "DFX_IOB_TX_STATUS5"},
	{0, "DFX_IOB_TX_ABNORMAL_CNT_0"},
	{0, "DFX_IOB_TX_ABNORMAL_CNT_1"},
	{0, "DFX_IOB_TX_ABNORMAL_CNT_2"},
	{0, "DFX_IOB_TX_ABNORMAL_CNT_3"},
	{0, "DFX_IOB_TX_ERROR0"},
	{0, "DFX_IOB_TX_ERROR1"},
	{0, "DFX_IOB_TX_TCS_NORMAL_CNT_0"},
	{0, "DFX_IOB_TX_TCS_NORMAL_CNT_1"},
	{0, "DFX_IOB_TX_TCS_NORMAL_CNT_2"},
	{0, "DFX_IOB_TX_TCS_NORMAL_CNT_3"},
	{0, "DFX_IOB_TX_TCS_NORMAL_CNT_4"},
	{0, "DFX_IOB_TX_TCS_NORMAL_CNT_5"},
	{0, "DFX_IOB_TX_TCS_NORMAL_CNT_6"},
	{0, "DFX_IOB_TX_TCS_NORMAL_CNT_7"},
	{0, "DFX_IOB_TX_TCS_NORMAL_CNT_8"},
	{0, "DFX_IOB_TX_TCS_NORMAL_CNT_9"},
	{0, "DFX_IOB_TX_TCS_NORMAL_CNT_10"},
	{0, "DFX_IOB_TX_TCS_NORMAL_CNT_11"},
	{0, "DFX_IOB_TX_TCS_NORMAL_CNT_12"},
	{0, "DFX_IOB_TX_TCS_NORMAL_CNT_13"},
	{0, "DFX_IOB_TX_TCS_NORMAL_CNT_14"},
	{0, "DFX_IOB_TX_TCS_NORMAL_CNT_15"},
	{0, "DFX_IOB_TX_TCS_P2P_CNT_0"},
	{0, "DFX_IOB_TX_TCS_P2P_CNT_1"},
	{0, "DFX_IOB_TX_TCS_P2P_CNT_2"},
	{0, "DFX_IOB_TX_TCS_P2P_CNT_3"},
	{0, "DFX_IOB_TX_TCS_P2P_CNT_4"},
	{0, "DFX_IOB_TX_TCS_P2P_CNT_5"},
	{0, "DFX_IOB_TX_TCS_P2P_CNT_6"},
	{0, "DFX_IOB_TX_TCS_P2P_CNT_7"},
	{0, "DFX_IOB_TX_TCS_ARNORAML_CNT_0"},
	{0, "DFX_IOB_TX_TCS_ARNORAML_CNT_1"},
	{0, "DFX_IOB_TX_TCS_STATUS0"},
	{0, "DFX_IOB_TX_TCS_STATUS1"},
	{0, "DFX_IOB_TX_TCS_STATUS2"},
	{0, "DFX_IOB_TX_TCS_IDLE"},
};

struct pcie_dumpreg_info g_reg_table_iob_rx[] = {
	{0, "IOB_RX_INT_STATUS"},
	{0, "IOB_RX_INT_RO"},
	{0, "IOB_RX_INT_SEVERITY"},
	{0, "IOB_RX_MSI_MSIX_CTRL_0"},
	{0, "IOB_RX_MSI_MSIX_ADDR_HIGH_0"},
	{0, "IOB_RX_MSI_MSIX_ADDR_LOW_0"},
	{0, "DFX_IOB_RX_CNT_RX_REQ"},
	{0, "DFX_IOB_RX_CNT_LOC_REQ"},
	{0, "DFX_IOB_RX_CNT_SEND_AM"},
	{0, "DFX_IOB_RX_CNT_SEND_LOC"},
	{0, "DFX_IOB_RX_CNT_RESP_RX"},
	{0, "DFX_IOB_RX_CNT_RESP_LOC"},
	{0, "DFX_IOB_RX_CNT_RESP_RECV"},
	{0, "DFX_IOB_RX_AMB_WR_CNT_0"},
	{0, "DFX_IOB_RX_AMB_WR_CNT_1"},
	{0, "DFX_IOB_RX_AMB_RD_CNT_0"},
	{0, "DFX_IOB_RX_AMB_RD_CNT_1"},
	{0, "DFX_IOB_RX_AMB_INT_NUM"},
};

struct pcie_dumpreg_info g_reg_table_ap_glb[] = {
	{0, "PCIE_ERR_MAPPING"},
	{0, "PCIE_CXL_ERR_MAPPING"},
	{0, "PCIE_CE_ENA"},
	{0, "PCIE_CE_MASK"},
	{0, "PCIE_UNF_ENA"},
	{0, "PCIE_UNF_MASK"},
	{0, "PCIE_UF_ENA"},
	{0, "PCIE_UF_MASK"},
	{0, "PCIE_CE_STATUS"},
	{0, "PCIE_UNF_STATUS"},
	{0, "PCIE_UF_STATUS"},
	{0, "PCIE_CXL_CE_ENA"},
	{0, "PCIE_CXL_CE_MASK"},
	{0, "PCIE_CXL_UNF_ENA"},
	{0, "PCIE_CXL_UNF_MASK"},
	{0, "PCIE_CXL_UF_ENA"},
	{0, "PCIE_CXL_UF_MASK"},
	{0, "PCIE_CXL_CE_STATUS"},
	{0, "PCIE_CXL_UNF_STATUS"},
	{0, "PCIE_CXL_UF_STATUS"},
	{0, "PCIE_MSI_MASK"},
	{0, "PCIE_MSI_STATUS"},
	{0, "PCIE_AP_NI_ENA"},
	{0, "PCIE_AP_CE_ENA"},
	{0, "PCIE_AP_UNF_ENA"},
	{0, "PCIE_AP_UF_ENA"},
	{0, "PCIE_AP_NI_MASK"},
	{0, "PCIE_AP_CE_MASK"},
	{0, "PCIE_AP_UNF_MASK"},
	{0, "PCIE_AP_UF_MASK"},
	{0, "PCIE_AP_NI_STATUS"},
	{0, "PCIE_AP_CE_STATUS"},
	{0, "PCIE_AP_UNF_STATUS"},
	{0, "PCIE_AP_UF_STATUS"},
	{0, "PCIE_CORE_NI_ENA"},
	{0, "PCIE_CORE_CE_ENA"},
	{0, "PCIE_CORE_UNF_ENA"},
	{0, "PCIE_CORE_UF_ENA"},
	{0, "PCIE_CORE_NI_MASK"},
	{0, "PCIE_CORE_CE_MASK"},
	{0, "PCIE_CORE_UNF_MASK"},
	{0, "PCIE_CORE_UF_MASK"},
	{0, "PCIE_CORE_NI_STATUS"},
	{0, "PCIE_CORE_CE_STATUS"},
	{0, "PCIE_CORE_UNF_STATUS"},
	{0, "PCIE_CORE_UF_STATUS"},
	{0, "PORT_INTX_MAPPING"},
	{0, "PORT_INTX_INTERRUPT_MODE"},
	{0, "PORT_INTX_ASSERT_MASK"},
	{0, "PORT_INTX_DEASSERT_MASK"},
	{0, "PORT_INTX_ASSERT_STATUS"},
	{0, "PORT_INTX_DEASSERT_STATUS"},
	{0, "AP_P2P_PORT_BITMAP_0"},
	{0, "AP_P2P_CTRL"},
	{0, "AP_P2P_INFO"},
	{0, "AP_PORT_EN"},
};

struct pcie_dumpreg_info g_reg_table_core_glb[] = {
	{0, "PORT_RESET"},
	{0, "PHY_RESET"},
	{0, "PORT_EN"},
	{0, "PORT_RESET_CFG"},
	{0, "GLB_PCIEC_MODE_SEL"},
	{0, "HILINK_INT_MASK"},
	{0, "HILINK_INT_RO"},
	{0, "HILINK_INT_STATUS"},
	{0, "CORE_INT_NI_MSK_0"},
	{0, "CORE_INT_NI_STATUS_0"},
	{0, "CORE_INT_NI_RO_0"},
	{0, "CORE_INT_NI_MSK_1"},
	{0, "CORE_INT_NI_STATUS_1"},
	{0, "CORE_INT_NI_RO_1"},
	{0, "CORE_INT_CE_MSK_0"},
	{0, "CORE_INT_CE_STATUS_0"},
	{0, "CORE_INT_CE_RO_0"},
	{0, "CORE_INT_CE_MSK_1"},
	{0, "CORE_INT_CE_STATUS_1"},
	{0, "CORE_INT_CE_RO_1"},
	{0, "CORE_INT_NFE_MSK_0"},
	{0, "CORE_INT_NFE_STATUS_0"},
	{0, "CORE_INT_NFE_RO_0"},
	{0, "CORE_INT_NFE_MSK_1"},
	{0, "CORE_INT_NFE_STATUS_1"},
	{0, "CORE_INT_NFE_RO_1"},
	{0, "CORE_INT_FE_MSK_0"},
	{0, "CORE_INT_FE_STATUS_0"},
	{0, "CORE_INT_FE_RO_0"},
	{0, "CORE_INT_FE_MSK_1"},
	{0, "CORE_INT_FE_STATUS_1"},
	{0, "CORE_INT_FE_RO_1"},
	{0, "CORE_INT_NI_MSK_2"},
	{0, "CORE_INT_NI_STATUS_2"},
	{0, "CORE_INT_NI_RO_2"},
	{0, "CORE_INT_CE_MSK_2"},
	{0, "CORE_INT_CE_STATUS_2"},
	{0, "CORE_INT_CE_RO_2"},
	{0, "CORE_INT_NFE_MSK_2"},
	{0, "CORE_INT_NFE_STATUS_2"},
	{0, "CORE_INT_NFE_RO_2"},
	{0, "CORE_INT_FE_MSK_2"},
	{0, "CORE_INT_FE_STATUS_2"},
	{0, "CORE_INT_FE_RO_2"},
	{0, "PORT07_LINK_MODE"},
	{0, "PORT815_LINK_MODE"},
	{0, "PCIE_LINK_DOWN_CLR_PORT_EN"},
	{0, "CORE_CLK_FLG"},
};

struct pcie_dumpreg_info g_reg_table_core_tl[] = {
	{0, "TL_PM_AUTO_EXIT_TIME_VALUE"},
	{0, "TL_DFX_PM_CORE_FUNC_EN"},
	{0, "TL_PM_DFE_TIME_VALUE"},
};

struct pcie_dumpreg_info g_reg_table_dfx_core_tl[] = {
	{0, "TL_TX_ASYN_FIFO_ST"},
	{0, "TL_TX_INGRESS_CNT"},
	{0, "TL_TX_CTRL_EGRESS_CNT"},
	{0, "TL_TX_CFG_CNT"},
	{0, "TL_TX_MEM_RD_CNT"},
	{0, "TL_TX_MEM_WR_CNT"},
	{0, "TL_TX_IO_RD_CNT"},
	{0, "TL_TX_IO_WR_CNT"},
	{0, "TL_TX_MSG_CNT"},
	{0, "TL_TX_CPL_CNT"},
	{0, "TL_TX_ATOMIC_CNT"},
	{0, "TL_TX_CFG_TX_CNT"},
	{0, "TL_TX_GEN_CPL_CNT"},
};

static int pcie_create_dumpreg_log_file(uint32_t port_id, uint32_t dump_level)
{
	char file_name[MAX_LOG_NAME_LEN + 1] = { 0 };
	char info_str[MAX_LOG_NAME_LEN + 1] = { 0 };
	FILE *fd_file = NULL;
	int ret;

	ret = snprintf(info_str, sizeof(info_str), "%s_port%u_level%u",
		       PCIE_DUMPREG_LOGFILE_NAME, port_id, dump_level);
	if (ret < 0)
		return -EINVAL;

	ret = generate_file_name((unsigned char *)file_name, MAX_LOG_NAME_LEN,
		(const unsigned char *)info_str);
	if (ret)
		return -EINVAL;

	memset(dumpreg_log_file, 0, sizeof(dumpreg_log_file));
	(void)strncpy((char *)dumpreg_log_file, file_name, MAX_LOG_NAME_LEN + 1);

	(void)remove((const char *)file_name);
	/* Add write permission to the file */
	fd_file = fopen(file_name, "w+");
	if (fd_file == NULL) {
		Err("open %s failed.\n", file_name);
		return -EPERM;
	}
	g_pcie_dumpreg_fd = fd_file;

	return 0;
}

static void pcie_close_dumpreg_log_file(void)
{
	fclose(g_pcie_dumpreg_fd);
	/* Revoke write permission of file  */
	chmod(dumpreg_log_file, 0400);
	g_pcie_dumpreg_fd = NULL;
}

static void pcie_dumpreg_write_value_to_file(const char *reg_name, uint32_t val)
{
	char str[MAX_STR_LEN] = { 0 };
	size_t wr_ret;
	int ret;

	ret = snprintf(str, sizeof(str), "    %-40s : 0x%x\n", reg_name, val);
	if (ret < 0 || ret >= MAX_STR_LEN) {
		Err("pcie dumpreg write info to logfile failed.\n");
	} else {
		wr_ret = fwrite(str, 1, strlen(str), g_pcie_dumpreg_fd);
		if (wr_ret != strlen(str))
			Err("write info to logfile failed.\n");
	}
}

struct pcie_dumpreg_table g_dump_info_glb[] = {
	{HIKP_ARRAY_SIZE(g_reg_table_iob_tx), g_reg_table_iob_tx},
	{HIKP_ARRAY_SIZE(g_reg_table_iob_rx), g_reg_table_iob_rx},
	{HIKP_ARRAY_SIZE(g_reg_table_ap_glb), g_reg_table_ap_glb},
	{HIKP_ARRAY_SIZE(g_reg_table_core_glb), g_reg_table_core_glb},
	{HIKP_ARRAY_SIZE(g_reg_table_core_tl), g_reg_table_core_tl},
	{HIKP_ARRAY_SIZE(g_reg_table_dfx_core_tl), g_reg_table_dfx_core_tl},
};

struct pcie_dumpreg_table g_dump_info_port[] = {
	{HIKP_ARRAY_SIZE(g_reg_table_tl), g_reg_table_tl},
	{HIKP_ARRAY_SIZE(g_reg_table_dl), g_reg_table_dl},
	{HIKP_ARRAY_SIZE(g_reg_table_mac), g_reg_table_mac},
};

static void pcie_dumpreg_save_analysis_log(const uint32_t *data, uint32_t data_num,
	 struct pcie_dumpreg_table *table, uint32_t size)
{
	uint32_t i, j, data_i = 0;

	for (i = 0; i < size; i++) {
		struct pcie_dumpreg_info *info = table[i].dump_info;
		for (j = 0; j < table[i].size && data_i < data_num; j++, data_i++) {
			info[j].val = data[data_i];
			pcie_dumpreg_write_value_to_file(info[j].name, info[j].val);
		}
	}
}

static int pcie_dumpreg_write_header_to_file(uint32_t version,
					     const struct pcie_dump_req_para *req_data)
{
	char str[MAX_STR_LEN] = {0};
	size_t wr_ret;
	int ret;

	ret = snprintf(str, sizeof(str), "Command Version[%u], dump_level[%u], port_id[%u]\n\n",
		version, req_data->level, req_data->port_id);
	if (ret < 0) {
		Err("pcie dumpreg write header to logfile failed.\n");
		return -EIO;
	}

	wr_ret = fwrite(str, 1, strlen(str), g_pcie_dumpreg_fd);
	if (wr_ret != strlen(str)) {
		Err("write header to logfile failed.\n");
		return -EIO;
	}

	return 0;
}

static int pcie_dumpreg_save_log(uint32_t *data, uint32_t data_num,
				 uint32_t version, struct pcie_dump_req_para *req_data)
{
	size_t expect_data_num = 0;
	char reg_name[PCIE_REG_NAME_LEN];
	uint32_t i;
	int ret;

	ret = pcie_dumpreg_write_header_to_file(version, req_data);
	if (ret < 0)
		return ret;

	switch (req_data->level) {
	case DUMP_GLOBAL_LEVEL:
		for (i = 0; i < HIKP_ARRAY_SIZE(g_dump_info_glb); i++) {
			expect_data_num += g_dump_info_glb[i].size;
		}
		break;
	case DUMP_PORT_LEVEL:
		for (i = 0; i < HIKP_ARRAY_SIZE(g_dump_info_port); i++) {
			expect_data_num += g_dump_info_port[i].size;
		}
		break;
	default:
		Err("check dump level failed.\n");
		return -EINVAL;
	}

	if (expect_data_num != data_num || version != ORIGINAL_VERSION) {
		for (i = 0; i < data_num; i++) {
			ret = snprintf(reg_name, sizeof(reg_name), "REG_%03u", i);
			if (ret < 0)
				Err("save log snprintf failed.\n");
			pcie_dumpreg_write_value_to_file(reg_name, data[i]);
		}
	} else if (req_data->level == DUMP_GLOBAL_LEVEL) {
		pcie_dumpreg_save_analysis_log(data, data_num,
		 g_dump_info_glb, HIKP_ARRAY_SIZE(g_dump_info_glb));
	} else {
		pcie_dumpreg_save_analysis_log(data, data_num,
		 g_dump_info_port, HIKP_ARRAY_SIZE(g_dump_info_port));
	}

	return 0;
}

int pcie_dumpreg_do_dump(uint32_t port_id, uint32_t dump_level)
{
	struct hikp_cmd_header req_header;
	struct hikp_cmd_ret *cmd_ret = NULL;
	struct pcie_dump_req_para req_data = { 0 };
	int ret = 0;

	Info("hikptool pcie_dumpreg -i %u -l %u -d\n", port_id, dump_level);

	req_data.port_id = port_id;
	req_data.level = dump_level;
	hikp_cmd_init(&req_header, PCIE_MOD, PCIE_DUMP, DUMPREG_DUMP);
	cmd_ret = hikp_cmd_alloc(&req_header, &req_data, sizeof(req_data));
	ret = hikp_rsp_normal_check(cmd_ret);
	if (ret) {
		Err("pcie dump cmd_ret check failed, ret: %d.\n", ret);
		goto free_cmd_ret;
	}
	ret = pcie_create_dumpreg_log_file(port_id, dump_level);
	if (ret)
		goto free_cmd_ret;

	ret = pcie_dumpreg_save_log(cmd_ret->rsp_data,
				    cmd_ret->rsp_data_num, cmd_ret->version, &req_data);
	if (ret) {
		Err("pcie dump save log failed, ret: %d.\n", ret);
		goto close_file_ret;
	}

	Info("pcie reg dump finish.\n");
close_file_ret:
	pcie_close_dumpreg_log_file();
free_cmd_ret:
	hikp_cmd_free(&cmd_ret);

	return ret;
}
