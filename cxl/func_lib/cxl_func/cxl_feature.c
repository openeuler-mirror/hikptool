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
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "hikptdev_plug.h"
#include "cxl_feature.h"

static void cxl_cpa_err_print(const struct cxl_data_unit *data, uint32_t data_unit_len)
{
	uint32_t i;

	printf("  cxl_cpa err msg info:\n");
	for (i = 0; i < data_unit_len; i++)
		printf("    [0x%04x] : 0x%08x\n", data[i].data_addr, data[i].data);
}

static void cxl_cpa_config_print(const struct cxl_data_unit *data, uint32_t data_unit_len)
{
	uint32_t i;

	printf("  cxl_cpa key cfg info:\n");
	for (i = 0; i < data_unit_len; i++)
		printf("    [0x%04x] : 0x%08x\n", data[i].data_addr, data[i].data);
}

static void cxl_cpa_mmrg_window_print(const struct cxl_data_unit *data, uint32_t data_unit_len)
{
	uint32_t i, j, offset, data_each_port;

	data_each_port = data_unit_len / CXL_HDM_CNT_EACH_PORT;
	if (data_unit_len % CXL_HDM_CNT_EACH_PORT != 0 ||
		data_each_port != CPA_MMRG_MSG_INFO_CNT) {
		printf("  data alignment alarm: data len[%u]\n", data_unit_len);
		goto nonformat_print;
	}

	printf("  cxl_cpa mmrg info:\n");
	for (i = 0; i < CXL_HDM_CNT_EACH_PORT; i++) {
		printf("    Hdm%u mmrg window show list:\n", i);
		offset = i * data_each_port;
		for (j = 0; j < data_each_port; j++)
			printf("      [0x%04x] : 0x%08x\n",
			       data[j + offset].data_addr, data[j + offset].data);
	}
	return;
nonformat_print:
	printf("  cxl_cpa mmrg info:\n");
	for (i = 0; i < data_unit_len; i++)
		printf("    [0x%04x] : 0x%08x\n", data[i].data_addr, data[i].data);
}

static void cxl_cpa_dump_reg_prt(const struct cxl_data_unit *data, uint32_t data_unit_len)
{
	uint32_t i;

	printf("  cxl_cpa reg dump list:\n");
	printf("    Addr        Value\n");
	for (i = 0; i < data_unit_len; i++)
		printf("    0x%04x      0x%08x\n", data[i].data_addr, data[i].data);
}

static void cxl_dl_fsm_str_get(struct cxl_fsm_state_str *fsm_str_table,
			       char **fsm_s, uint32_t fsm_state)
{
	int i = 0;

	while (fsm_str_table[i].fsm_state >= 0 &&
	       (uint32_t)fsm_str_table[i].fsm_state != fsm_state)
		i++;

	*fsm_s = fsm_str_table[i].fsm_str;
}

static void cxl_dl_fsm_state_print(const struct cxl_data_unit *data, uint32_t data_unit_len)
{
	union cxl_dl_fsm_state_reg reg;
	char *fsm_s = NULL;
	struct cxl_fsm_state_str rrsm_state[] = {
		{0x0, "retry_remote_normal"},
		{0x1, "retry_llrack"},
		{-1, "unknown"}
	};
	struct cxl_fsm_state_str lrsm_state[] = {
		{0x1, "retry_local_normal"},
		{0x2, "retry_llrreq"},
		{0x4, "retry_phy_reinit"},
		{0x8, "retry_local_idle"},
		{0x10, "retry_abort"},
		{-1, "unknown"}
	};
	struct cxl_fsm_state_str init_fsm_state[] = {
		{0x1, "inactive"},
		{0x2, "retry"},
		{0x4, "param"},
		{0x8, "active"},
		{-1, "unknown"}
	};

	if (data_unit_len == 0) {
		printf("cxl dump data size is wrong.\n");
		return;
	}
	reg.val = data[0].data;
	cxl_dl_fsm_str_get(rrsm_state, &fsm_s, reg.bits.rrsm_state);
	printf("  %-25s : %s[%04x]\n", "cxl_dl_rrsm_state", fsm_s, reg.bits.rrsm_state);
	cxl_dl_fsm_str_get(lrsm_state, &fsm_s, reg.bits.lrsm_state);
	printf("  %-25s : %s[%04x]\n", "cxl_dl_lrsm_state", fsm_s, reg.bits.lrsm_state);
	cxl_dl_fsm_str_get(init_fsm_state, &fsm_s, reg.bits.init_fsm_state);
	printf("  %-25s : %s[%04x]\n", "cxl_dl_init_fsm_state", fsm_s, reg.bits.init_fsm_state);
}

static void cxl_dl_dfx_print(const struct cxl_data_unit *data, uint32_t data_unit_len)
{
	uint32_t i;

	printf("  cxl_dl dfx info:\n");
	for (i = 0; i < data_unit_len; i++)
		printf("    [0x%04x] : [0x%08x]\n", data[i].data_addr, data[i].data);
}

static void cxl_dl_dump_reg_prt(const struct cxl_data_unit *data, uint32_t data_unit_len)
{
	uint32_t i;

	printf("  cxl_dl reg dump list:\n");
	printf("    Addr        Value\n");
	for (i = 0; i < data_unit_len; i++)
		printf("    0x%04x      0x%08x\n", data[i].data_addr, data[i].data);
}

static void cxl_dl_error_info_prt(const struct cxl_data_unit *data, uint32_t data_unit_len)
{
	uint32_t i;

	printf("  cxl_dl err info:\n");
	for (i = 0; i < data_unit_len; i++)
		printf("    [0x%04x] : 0x%08x\n", data[i].data_addr, data[i].data);
}

static void cxl_link_info0_prt(uint32_t data)
{
	union cxl_rcrb_vendor_spec_header reg;

	reg.val = data;
	printf("    %-20s : 0x%x\n", "cxl.cache capable", reg.bits.cache_capable);
	printf("    %-20s : 0x%x\n", "cxl.io capable", reg.bits.io_capable);
	printf("    %-20s : 0x%x\n", "cxl.mem capable", reg.bits.mem_capable);
}

static void cxl_link_info1_prt(uint32_t data)
{
	union cxl_rcrb_flex_bus_ctrl reg;

	reg.val = data;
	printf("    %-20s : 0x%x\n", "cxl.cache enable", reg.bits.cache_enable);
	printf("    %-20s : 0x%x\n", "cxl.io enable", reg.bits.io_enable);
	printf("    %-20s : 0x%x\n", "cxl.mem enable", reg.bits.mem_enable);
	printf("\n  %s:\n", "cxl link status");
	printf("    %-20s : %s\n", "cxl.cache",
	       (reg.bits.cache_enabled == 1) ? "link up" : "link down");
	printf("    %-20s : %s\n", "cxl.io", (reg.bits.io_enabled == 1) ? "link up" : "link down");
	printf("    %-20s : %s\n", "cxl.mem",
	       (reg.bits.mem_enabled == 1) ? "link up" : "link down");
}

static void cxl_rcrb_link_info_prt(const struct cxl_data_unit *data, uint32_t data_unit_len)
{
	uint32_t i;
	struct cxl_info_desc cxl_link_info[CXL_RCRB_LINK_INFO_CNT] = {
		{"cxl_rcrb_link_info0", cxl_link_info0_prt},
		{"cxl_rcrb_link_info1", cxl_link_info1_prt},
	};

	printf("\n  %s list:\n", "cxl link cfg");
	for (i = 0; i < data_unit_len && i < CXL_RCRB_LINK_INFO_CNT; i++)
		cxl_link_info[i].info_prt(data[i].data);
}

static void cxl_rcrb_cfg_header_prt(const struct cxl_data_unit *data, uint32_t data_unit_len)
{
	uint32_t i;
	const char *cxl_rcrb_hdr_msg[CXL_RCRB_CFG_HEADER_INFO_CNT] = {
		"cxl_rcrb_hdr_id",
		"cxl_rcrb_hdr_cmd",
		"cxl_rcrb_hdr_clserv",
		"cxl_rcrb_hdr_misc",
		"cxl_rcrb_hdr_bar0",
		"cxl_rcrb_hdr_bar1",
		"cxl_rcrb_hdr_busnum",
		"cxl_rcrb_hdr_membase_limit",
		"cxl_rcrb_hdr_pre_membase_limit",
		"cxl_rcrb_hdr_pre_membase_32upadr",
		"cxl_rcrb_hdr_pre_memlimit_32upadr",
		"cxl_rcrb_hdr_int_info"
	};

	printf("  cxl_rcrb cfg_header info:\n");
	for (i = 0; i < data_unit_len && i < CXL_RCRB_CFG_HEADER_INFO_CNT; i++)
		printf("    %-40s : 0x%x\n", cxl_rcrb_hdr_msg[i], data[i].data);
}

static void cxl_rcrb_dump_reg_prt(const struct cxl_data_unit *data, uint32_t data_unit_len)
{
	uint32_t i;

	printf("  cxl_rcrb reg dump list:\n");
	printf("    Addr        Value\n");
	for (i = 0; i < data_unit_len; i++)
		printf("    0x%04x      0x%08x\n", data[i].data_addr, data[i].data);
}

static void cxl_mem_uncorrect_err_prt(uint32_t data)
{
	union cxl_mem_uncorrect_err reg;

	reg.val = data;
	printf("    %-25s : 0x%x\n", "cache_data_parity", reg.bits.cache_data_parity);
	printf("    %-25s : 0x%x\n", "cache_addr_parity", reg.bits.cache_addr_parity);
	printf("    %-25s : 0x%x\n", "cache_be_parity", reg.bits.cache_be_parity);
	printf("    %-25s : 0x%x\n", "cache_data_ecc", reg.bits.cache_data_ecc);
	printf("    %-25s : 0x%x\n", "mem_data_parity", reg.bits.mem_data_parity);
	printf("    %-25s : 0x%x\n", "mem_address_parity", reg.bits.mem_address_parity);
	printf("    %-25s : 0x%x\n", "mem_be_parity", reg.bits.mem_be_parity);
	printf("    %-25s : 0x%x\n", "mem_data_ecc", reg.bits.mem_data_ecc);
	printf("    %-25s : 0x%x\n", "reinit_threshold", reg.bits.reinit_threshold);
	printf("    %-25s : 0x%x\n", "rsvd_encoding_violation", reg.bits.rsvd_encoding_violation);
	printf("    %-25s : 0x%x\n", "poison_received", reg.bits.poison_received);
	printf("    %-25s : 0x%x\n", "receiver_overflow", reg.bits.receiver_overflow);
}

static void cxl_mem_correct_err_prt(uint32_t data)
{
	union cxl_mem_correct_err reg;

	reg.val = data;
	printf("    %-25s : 0x%x\n", "cache_data_ecc", reg.bits.cache_data_ecc);
	printf("    %-25s : 0x%x\n", "mem_data_ecc", reg.bits.mem_data_ecc);
	printf("    %-25s : 0x%x\n", "crc_threshold", reg.bits.crc_threshold);
	printf("    %-25s : 0x%x\n", "retry_threshold", reg.bits.retry_threshold);
	printf("    %-25s : 0x%x\n", "cache_poison_received", reg.bits.cache_poison_received);
	printf("    %-25s : 0x%x\n", "mem_poison_received", reg.bits.mem_poison_received);
	printf("    %-25s : 0x%x\n", "physical_layer_error", reg.bits.physical_layer_error);
}

static void cxl_mem_err_ctrl_prt(uint32_t data)
{
	union cxl_mem_error_ctrl reg;

	reg.val = data;
	printf("    %-25s : 0x%x\n", "first_error_pointer", reg.bits.first_error_pointer);
	printf("    %-25s : 0x%x\n", "multiple_header_recording_capability",
	       reg.bits.multiple_header_recording_capability);
	printf("    %-25s : 0x%x\n", "poison_enable", reg.bits.poison_enable);
}

static void cxl_membar_err_info_prt(const struct cxl_data_unit *data, uint32_t data_unit_len)
{
	uint32_t i;
	struct cxl_info_desc cxl_membar_err[CXL_MEMBAR_ERR_INFO_CNT] = {
		{"cxl_mem_uncorrect_err_status", cxl_mem_uncorrect_err_prt},
		{"cxl_mem_uncorrect_err_mask", cxl_mem_uncorrect_err_prt},
		{"cxl_mem_uncorrect_err_severity", cxl_mem_uncorrect_err_prt},
		{"cxl_mem_correct_err_status", cxl_mem_correct_err_prt},
		{"cxl_mem_correct_err_mask", cxl_mem_correct_err_prt},
		{"cxl_mem_err_ctrl", cxl_mem_err_ctrl_prt}
	};

	for (i = 0; i < data_unit_len && i < CXL_MEMBAR_ERR_INFO_CNT; i++) {
		printf("\n  %s list:\n", cxl_membar_err[i].info_msg);
		cxl_membar_err[i].info_prt(data[i].data);
	}

	if (data_unit_len < CXL_MEM_HEADER_LOG_UNIT) {
		printf("dump cxl_mem headerlog size invalid, data_len is %u\n", data_unit_len);
		return;
	}
	printf("\n  HeaderLog :\n");
	for (; i <= data_unit_len - CXL_MEM_HEADER_LOG_UNIT; i += CXL_MEM_HEADER_LOG_UNIT) {
		printf("  [%04x] :  0x%08x  0x%08x  0x%08x  0x%08x\n",
		       i - CXL_MEMBAR_ERR_INFO_CNT, data[i + CXL_HEADER_LOG0].data,
		       data[i + CXL_HEADER_LOG1].data, data[i + CXL_HEADER_LOG2].data,
		       data[i + CXL_HEADER_LOG3].data);
	}
}

static void cxl_membar_dump_reg_prt(const struct cxl_data_unit *data, uint32_t data_unit_len)
{
	uint32_t i;

	printf("  cxl_membar reg dump list:\n");
	printf("    Addr        Value\n");
	for (i = 0; i < data_unit_len; i++)
		printf("    0x%04x      0x%08x\n", data[i].data_addr, data[i].data);
}

static int cxl_data_unit_buf_check(uint32_t data_offset,
	size_t data_len, uint32_t rsp_data_num)
{
	size_t expect_end_pos;
	size_t rsp_data_end_pos;

	expect_end_pos = data_offset * sizeof(uint32_t) + data_len;
	rsp_data_end_pos = rsp_data_num * sizeof(uint32_t);
	if (expect_end_pos > rsp_data_end_pos)
		return -EINVAL;

	return 0;
}

static struct cxl_prt_func g_prtf[] = {
	{CXL_CPA, CPA_ERR, cxl_cpa_err_print},
	{CXL_CPA, CPA_MMRG, cxl_cpa_mmrg_window_print},
	{CXL_CPA, CPA_DUMP, cxl_cpa_dump_reg_prt},
	{CXL_CPA, CPA_CONFIG, cxl_cpa_config_print},
	{CXL_DL, CXL_DL_FSM_STATE, cxl_dl_fsm_state_print},
	{CXL_DL, CXL_DL_DFX, cxl_dl_dfx_print},
	{CXL_DL, CXL_DL_DUMP, cxl_dl_dump_reg_prt},
	{CXL_DL, CXL_DL_ERR, cxl_dl_error_info_prt},
	{CXL_RCRB, CXL_RCRB_LINK, cxl_rcrb_link_info_prt},
	{CXL_RCRB, CXL_RCRB_HDR, cxl_rcrb_cfg_header_prt},
	{CXL_RCRB, CXL_RCRB_DUMP, cxl_rcrb_dump_reg_prt},
	{CXL_MEMBAR, CXL_MEMBAR_ERR, cxl_membar_err_info_prt},
	{CXL_MEMBAR, CXL_MEMBAR_DUMP, cxl_membar_dump_reg_prt},
};

int cxl_reg_show_execute(uint32_t port_id, uint32_t mode_code, uint32_t cmd_type)
{
	int ret;
	uint32_t i;
	size_t data_unit_len;
	struct hikp_cmd_header req_header;
	struct hikp_cmd_ret *cmd_ret;
	struct cxl_cmd_paras_in req_para;
	struct cxl_out_data *data_head = NULL;
	struct cxl_data_unit *data_unit_buf = NULL;

	req_para.port_id = port_id;
	hikp_cmd_init(&req_header, CXL_MOD, mode_code, cmd_type);
	cmd_ret = hikp_cmd_alloc(&req_header, &req_para, sizeof(req_para));
	ret = hikp_rsp_normal_check(cmd_ret);
	if (ret) {
		printf("cxl_cmd mode_code: %u cmd_type: %u, hikp_get_data_proc err, ret : %d\n",
		       mode_code, cmd_type, ret);
		if (cmd_ret)
			free(cmd_ret);
		return ret;
	}

	data_head = (struct cxl_out_data *)cmd_ret->rsp_data;
	data_unit_buf = (struct cxl_data_unit *)(cmd_ret->rsp_data + data_head->data_offset);
	data_unit_len = data_head->length / sizeof(struct cxl_data_unit);

	ret = cxl_data_unit_buf_check(data_head->data_offset, data_unit_len, cmd_ret->rsp_data_num);
	if (ret) {
		free(cmd_ret);
		return ret;
	}

	for (i = 0; i < (sizeof(g_prtf) / sizeof((g_prtf)[0])); i++) {
		if (mode_code == g_prtf[i].mode_code &&
		    cmd_type == g_prtf[i].cmd_type && g_prtf[i].cxl_prt_handle) {
			g_prtf[i].cxl_prt_handle(data_unit_buf, data_unit_len);
			free(cmd_ret);
			return 0;
		}
	}

	free(cmd_ret);
	return -EINVAL;
}
