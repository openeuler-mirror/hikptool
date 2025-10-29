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
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include "ummu_common.h"
#include "hikptdev_plug.h"
#include "ummu_reg_dump.h"

/* ummu_dfx_reg default value */
#define DV_UMMU_SWIF_KCMDQ_DFX_CMD_CTRL_STATUS2 0x80828800
#define DV_UMMU_SWIF_UMCMD_CACHE_DFX2 0x010000
#define DV_UMMU_SWIF_UMCMD_CACHE_DFX4 0x01
#define DV_UMMU_UBIF_DFX0 0x0A00002A
#define DV_UMMU_UBIF_DSTEID_DFX 0x0410
#define DV_UMMU_TBU_TLB_LKUP_PROC 0x0155
#define DV_UMMU_TBU_PLB_LKUP_PROC 0x0155
#define DV_UMMU_TBU_PLB_STAT 0x1000
#define DV_UMMU_TBU_INVLD_MG_INFO 0x03F
#define DV_UMMU_TBU_RAB_STAT 0x0214000

static int ummu_dump_reg_rsp_data_check(struct hikp_cmd_ret *cmd_ret, size_t expected_size)
{
	size_t rsp_data_size;
	int ret;

	ret = hikp_rsp_normal_check(cmd_ret);
	if (ret) {
		printf("dump registers failed, ret: %d.\n", ret);
		return ret;
	}

	rsp_data_size = cmd_ret->rsp_data_num * sizeof(uint32_t);
	if (rsp_data_size < expected_size) {
		printf("ummu dump reg rsp size check failed, rsp size: %zu, expect size:%zu.\n",
		       rsp_data_size, expected_size);
		return -EINVAL;
	}

	return 0;
}

static int get_ummu_data_cnt(struct hikp_cmd_ret *cmd_ret, size_t expected_size)
{
	size_t rsp_data_size, cnt;

	rsp_data_size = cmd_ret->rsp_data_num * sizeof(uint32_t);

	if (rsp_data_size % expected_size != 0) {
		printf("get ummu data cnt failed\n");
		return -EINVAL;
	}

	cnt = rsp_data_size / expected_size;

	if (cnt > MAX_UMMU_NUM)
		return -EINVAL;

	return cnt;
}

static void print_ummu_dfx_title(void)
{
	printf("UMMU DFX Register\t\t\t\tCurrent Value\t\t\t"
	       "Default Value\t\t\tUMMU ID\n");
	printf("-------------------------------------------------------------"
	       "------------------------------------------------------------\n");
}

static void dump_kcmd_reg_parse(struct ummu_kcmd_reg *reg, int id)
{
	printf("ummu_glb_err\t\t\t\t\t0x%08x\t\t\t0x%08x\t\t\t%d\n",
	       reg->ummu_glb_err, 0, id);
	printf("ummu_swif_kcmdq_dfx_kcmd_status\t\t\t0x%08x\t\t\t0x%08x\t\t\t%d\n",
	       reg->ummu_swif_kcmdq_dfx_kcmd_status, 0, id);
	printf("ummu_swif_kcmdq_dfx_kcmd_err_status\t\t0x%08x\t\t\t0x%08x\t\t\t%d\n",
	       reg->ummu_swif_kcmdq_dfx_kcmd_err_status, 0, id);
	printf("ummu_swif_kcmdq_dfx_cmd_entry_status\t\t0x%08x\t\t\t0x%08x\t\t\t%d\n",
	       reg->ummu_swif_kcmdq_dfx_cmd_entry_status, 0, id);
	printf("ummu_swif_kcmdq_dfx_snp_err_cnt\t\t\t0x%08x\t\t\t0x%08x\t\t\t%d\n",
	       reg->ummu_swif_kcmdq_dfx_snp_err_cnt, 0, id);
	printf("ummu_swif_kcmdq_dfx_snp_status\t\t\t0x%08x\t\t\t0x%08x\t\t\t%d\n",
	       reg->ummu_swif_kcmdq_dfx_snp_status, 0, id);
	printf("ummu_swif_kcmdq_dfx_cmd_ctrl_status_1\t\t0x%08x\t\t\t0x%08x\t\t\t%d\n",
	       reg->ummu_swif_kcmdq_dfx_cmd_ctrl_status_1, 0, id);
	printf("ummu_swif_kcmdq_dfx_cmd_ctrl_status_2\t\t0x%08x\t\t\t0x%08x\t\t\t%d\n",
	       reg->ummu_swif_kcmdq_dfx_cmd_ctrl_status_2,
	       DV_UMMU_SWIF_KCMDQ_DFX_CMD_CTRL_STATUS2, id);
	printf("ummu_dvm_receive_req_cnt\t\t\t0x%08x\t\t\t0x%08x\t\t\t%d\n",
	       reg->ummu_dvm_receive_req_cnt, 0, id);
	printf("ummu_dvm_send_req_cnt\t\t\t\t0x%08x\t\t\t0x%08x\t\t\t%d\n",
	       reg->ummu_dvm_send_req_cnt, 0, id);
	printf("ummu_dvm_req_info0\t\t\t\t0x%08x\t\t\t0x%08x\t\t\t%d\n",
	       reg->ummu_dvm_req_info0, 0, id);
	printf("ummu_dvm_req_info1\t\t\t\t0x%08x\t\t\t0x%08x\t\t\t%d\n",
	       reg->ummu_dvm_req_info1, 0, id);
	printf("ummu_mcmd_que_ci\t\t\t\t0x%08x\t\t\t0x%08x\t\t\t%d\n",
	       reg->ummu_mcmd_que_ci, 0, id);
	printf("ummu_sync_timeout_info\t\t\t\t0x%08x\t\t\t0x%08x\t\t\t%d\n",
	       reg->ummu_sync_timeout_info, 0, id);
	printf("ummu_swif_eventq_dfx_drop_cnt\t\t\t0x%08x\t\t\t0x%08x\t\t\t%d\n",
	       reg->ummu_swif_eventq_dfx_drop_cnt, 0, id);
	printf("ummu_err_status_0\t\t\t\t0x%08x\t\t\t0x%08x\t\t\t%d\n",
	       reg->ummu_err_status_0, 0, id);
	printf("ummu_err_status_1\t\t\t\t0x%08x\t\t\t0x%08x\t\t\t%d\n",
	       reg->ummu_err_status_1, 0, id);
	printf("ummu_err_addr_0\t\t\t\t\t0x%08x\t\t\t0x%08x\t\t\t%d\n",
	       reg->ummu_err_addr_0, 0, id);
	printf("ummu_err_addr_1\t\t\t\t\t0x%08x\t\t\t0x%08x\t\t\t%d\n",
	       reg->ummu_err_addr_1, 0, id);
	printf("\n");
}

int ummu_dump_kcmd_execute(uint8_t kcmd_entry_no, uint8_t ummu_id)
{
	struct ummu_reg_dump_para req_para = { 0 };
	struct hikp_cmd_header req_header = { 0 };
	struct ummu_kcmd_reg *kcmd_reg;
	struct hikp_cmd_ret *cmd_ret;
	size_t expected_size;
	int ret, cnt, idx;

	req_para.kcmd_entry_no = kcmd_entry_no;
	req_para.ummu_id = ummu_id;

	hikp_cmd_init(&req_header, UMMU_MOD, UMMU_CMD_DUMP, UMMU_KCMD_DUMP);
	cmd_ret = hikp_cmd_alloc(&req_header, &req_para, sizeof(req_para));
	if (!cmd_ret)
		return -ENOSPC;
	expected_size = sizeof(struct ummu_kcmd_reg);
	ret = ummu_dump_reg_rsp_data_check(cmd_ret, expected_size);
	if (ret)
		goto free_cmd_ret;

	kcmd_reg = (struct ummu_kcmd_reg *)cmd_ret->rsp_data;
	if (ummu_id != MAX_UMMU_NUM) {
		print_ummu_dfx_title();
		dump_kcmd_reg_parse(kcmd_reg, ummu_id);
		goto free_cmd_ret;
	}

	cnt = get_ummu_data_cnt(cmd_ret, expected_size);
	if (cnt < 0) {
		ret = cnt;
		goto free_cmd_ret;
	}

	print_ummu_dfx_title();
	for (idx = 0; idx < cnt; idx++)
		dump_kcmd_reg_parse(kcmd_reg + idx, idx);

free_cmd_ret:
	hikp_cmd_free(&cmd_ret);
	return ret;
}

static void dump_umcmd_reg_parse(struct ummu_umcmd_reg *reg, int id)
{
	printf("ummu_glb_err\t\t\t\t\t0x%08x\t\t\t0x%08x\t\t\t%d\n",
	       reg->ummu_glb_err, 0, id);
	printf("ummu_swif_umcmd_dfx_0\t\t\t\t0x%08x\t\t\t0x%08x\t\t\t%d\n",
	       reg->ummu_swif_umcmd_dfx_0, 0, id);
	printf("ummu_swif_umcmd_dfx_1\t\t\t\t0x%08x\t\t\t0x%08x\t\t\t%d\n",
	       reg->ummu_swif_umcmd_dfx_1, 0, id);
	printf("ummu_swif_umcmd_dfx_2\t\t\t\t0x%08x\t\t\t0x%08x\t\t\t%d\n",
	       reg->ummu_swif_umcmd_dfx_2, 0, id);
	printf("ummu_swif_umcmd_dfx_3\t\t\t\t0x%08x\t\t\t0x%08x\t\t\t%d\n",
	       reg->ummu_swif_umcmd_dfx_3, 0, id);
	printf("ummu_swif_umcmd_rr_win_dfx_0\t\t\t0x%08x\t\t\t0x%08x\t\t\t%d\n",
	       reg->ummu_swif_umcmd_rr_win_dfx_0, 0, id);
	printf("ummu_swif_umcmd_rr_win_dfx_1\t\t\t0x%08x\t\t\t0x%08x\t\t\t%d\n",
	       reg->ummu_swif_umcmd_rr_win_dfx_1, 0, id);
	printf("ummu_swif_umcmd_rr_win_dfx_2\t\t\t0x%08x\t\t\t0x%08x\t\t\t%d\n",
	       reg->ummu_swif_umcmd_rr_win_dfx_2, 0, id);
	printf("ummu_swif_umcmd_cache_dfx_1\t\t\t0x%08x\t\t\t0x%08x\t\t\t%d\n",
	       reg->ummu_swif_umcmd_cache_dfx_1, 0, id);
	printf("ummu_swif_umcmd_cache_dfx_2\t\t\t0x%08x\t\t\t0x%08x\t\t\t%d\n",
	       reg->ummu_swif_umcmd_cache_dfx_2,
	       DV_UMMU_SWIF_UMCMD_CACHE_DFX2, id);
	printf("ummu_swif_umcmd_cache_dfx_3\t\t\t0x%08x\t\t\t0x%08x\t\t\t%d\n",
	       reg->ummu_swif_umcmd_cache_dfx_3, 0, id);
	printf("ummu_swif_umcmd_cache_dfx_4\t\t\t0x%08x\t\t\t0x%08x\t\t\t%d\n",
	       reg->ummu_swif_umcmd_cache_dfx_4,
	       DV_UMMU_SWIF_UMCMD_CACHE_DFX4, id);
	printf("ummu_swif_umcmd_cache_dfx_5\t\t\t0x%08x\t\t\t0x%08x\t\t\t%d\n",
	       reg->ummu_swif_umcmd_cache_dfx_5, 0, id);
	printf("ummu_swif_umcmd_cache_dfx_6\t\t\t0x%08x\t\t\t0x%08x\t\t\t%d\n",
	       reg->ummu_swif_umcmd_cache_dfx_6, 0, id);
	printf("ummu_err_status_0\t\t\t\t0x%08x\t\t\t0x%08x\t\t\t%d\n",
	       reg->ummu_err_status_0, 0, id);
	printf("ummu_err_addr_0\t\t\t\t\t0x%08x\t\t\t0x%08x\t\t\t%d\n",
	       reg->ummu_err_addr_0, 0, id);
	printf("ummu_err_addr_1\t\t\t\t\t0x%08x\t\t\t0x%08x\t\t\t%d\n",
	       reg->ummu_err_addr_1, 0, id);
	printf("\n");
}

int ummu_dump_umcmd_execute(uint8_t cache_idx, uint8_t rr_win_num, uint8_t ummu_id)
{
	struct ummu_reg_dump_para req_para = { 0 };
	struct hikp_cmd_header req_header = { 0 };
	struct ummu_umcmd_reg *umcmd_reg;
	struct hikp_cmd_ret *cmd_ret;
	size_t expected_size;
	int ret, idx, cnt;

	req_para.cache_idx = cache_idx;
	req_para.rr_win_num = rr_win_num;
	req_para.ummu_id = ummu_id;

	hikp_cmd_init(&req_header, UMMU_MOD, UMMU_CMD_DUMP, UMMU_UMCMD_DUMP);
	cmd_ret = hikp_cmd_alloc(&req_header, &req_para, sizeof(req_para));
	if (!cmd_ret)
		return -ENOSPC;
	expected_size = sizeof(struct ummu_umcmd_reg);
	ret = ummu_dump_reg_rsp_data_check(cmd_ret, expected_size);
	if (ret)
		goto free_cmd_ret;

	umcmd_reg = (struct ummu_umcmd_reg *)cmd_ret->rsp_data;
	if (ummu_id != MAX_UMMU_NUM) {
		print_ummu_dfx_title();
		dump_umcmd_reg_parse(umcmd_reg, ummu_id);
		goto free_cmd_ret;
	}

	cnt = get_ummu_data_cnt(cmd_ret, expected_size);
	if (cnt < 0) {
		ret = cnt;
		goto free_cmd_ret;
	}

	print_ummu_dfx_title();
	for (idx = 0; idx < cnt; idx++)
		dump_umcmd_reg_parse(umcmd_reg + idx, idx);

free_cmd_ret:
	hikp_cmd_free(&cmd_ret);
	return ret;
}

static void dump_ubif_reg_parse(struct ummu_ubif_reg *reg, int id)
{
	printf("ummu_ubif_sync_dfx\t\t\t\t0x%08x\t\t\t0x%08x\t\t\t%d\n",
	       reg->ummu_ubif_sync_dfx, 0, id);
	printf("ummu_ubif_dfx_0\t\t\t\t\t0x%08x\t\t\t0x%08x\t\t\t%d\n",
	       reg->ummu_ubif_dfx_0, DV_UMMU_UBIF_DFX0, id);
	printf("ummu_ubif_dfx_1\t\t\t\t\t0x%08x\t\t\t0x%08x\t\t\t%d\n",
	       reg->ummu_ubif_dfx_1, 0, id);
	printf("ummu_ubif_dfx_2\t\t\t\t\t0x%08x\t\t\t0x%08x\t\t\t%d\n",
	       reg->ummu_ubif_dfx_2, 0, id);
	printf("ummu_ubif_dsteid_dfx\t\t\t\t0x%08x\t\t\t0x%08x\t\t\t%d\n",
	       reg->ummu_ubif_dsteid_dfx, DV_UMMU_UBIF_DSTEID_DFX, id);
	printf("ummu_ubif_kv_cache_ns_nse_mismatch_dfx_0\t0x%08x\t\t\t0x%08x\t\t\t%d\n",
	       reg->ummu_ubif_kv_cache_ns_nse_mismatch_dfx_0, 0, id);
	printf("ummu_ubif_kv_cache_ns_nse_mismatch_dfx_1\t0x%08x\t\t\t0x%08x\t\t\t%d\n",
	       reg->ummu_ubif_kv_cache_ns_nse_mismatch_dfx_1, 0, id);
	printf("ummu_ubif_kv_cache_ns_nse_mismatch_dfx_2\t0x%08x\t\t\t0x%08x\t\t\t%d\n",
	       reg->ummu_ubif_kv_cache_ns_nse_mismatch_dfx_2, 0, id);
	printf("ummu_ubif_kv_cache_ns_nse_mismatch_dfx_3\t0x%08x\t\t\t0x%08x\t\t\t%d\n",
	       reg->ummu_ubif_kv_cache_ns_nse_mismatch_dfx_3, 0, id);
	printf("ummu_ubif_kv_cache_ns_nse_mismatch_dfx_4\t0x%08x\t\t\t0x%08x\t\t\t%d\n",
	       reg->ummu_ubif_kv_cache_ns_nse_mismatch_dfx_4, 0, id);
	printf("ummu_err_status_0\t\t\t\t0x%08x\t\t\t0x%08x\t\t\t%d\n",
	       reg->ummu_err_status_0, 0, id);
	printf("ummu_err_addr_0\t\t\t\t\t0x%08x\t\t\t0x%08x\t\t\t%d\n",
	       reg->ummu_err_addr_0, 0, id);
	printf("ummu_err_addr_1\t\t\t\t\t0x%08x\t\t\t0x%08x\t\t\t%d\n",
	       reg->ummu_err_addr_1, 0, id);
	printf("\n");
}

int ummu_dump_ubif_execute(uint8_t ummu_id)
{
	struct ummu_reg_dump_para req_para = { 0 };
	struct hikp_cmd_header req_header = { 0 };
	struct ummu_ubif_reg *ubif_reg;
	struct hikp_cmd_ret *cmd_ret;
	size_t expected_size;
	int ret, cnt, idx;

	req_para.ummu_id = ummu_id;

	hikp_cmd_init(&req_header, UMMU_MOD, UMMU_CMD_DUMP, UMMU_UBIF_DUMP);
	cmd_ret = hikp_cmd_alloc(&req_header, &req_para, sizeof(req_para));
	if (!cmd_ret)
		return -ENOSPC;
	expected_size = sizeof(struct ummu_ubif_reg);
	ret = ummu_dump_reg_rsp_data_check(cmd_ret, expected_size);
	if (ret)
		goto free_cmd_ret;

	ubif_reg = (struct ummu_ubif_reg *)cmd_ret->rsp_data;
	if (ummu_id != MAX_UMMU_NUM) {
		print_ummu_dfx_title();
		dump_ubif_reg_parse(ubif_reg, ummu_id);
		goto free_cmd_ret;
	}

	cnt = get_ummu_data_cnt(cmd_ret, expected_size);
	if (cnt < 0) {
		ret = cnt;
		goto free_cmd_ret;
	}

	print_ummu_dfx_title();
	for (idx = 0; idx < cnt; idx++)
		dump_ubif_reg_parse(ubif_reg + idx, idx);

free_cmd_ret:
	hikp_cmd_free(&cmd_ret);
	return ret;
}

static void dump_tbu_reg_parse(struct ummu_tbu_reg *reg, int id)
{
	printf("ummu_tbu_tlb_lkup_proc\t\t\t\t0x%08x\t\t\t0x%08x\t\t\t%d\n",
	       reg->ummu_tbu_tlb_lkup_proc,
	       DV_UMMU_TBU_TLB_LKUP_PROC, id);
	printf("ummu_tbu_tlb_stat\t\t\t\t0x%08x\t\t\t0x%08x\t\t\t%d\n",
	       reg->ummu_tbu_tlb_stat, 0, id);
	printf("ummu_tbu_tlb_fault_cnt\t\t\t\t0x%08x\t\t\t0x%08x\t\t\t%d\n",
	       reg->ummu_tbu_tlb_fault_cnt, 0, id);
	printf("ummu_tbu_plb_lkup_proc\t\t\t\t0x%08x\t\t\t0x%08x\t\t\t%d\n",
	       reg->ummu_tbu_plb_lkup_proc,
	       DV_UMMU_TBU_PLB_LKUP_PROC, id);
	printf("ummu_tbu_plb_stat\t\t\t\t0x%08x\t\t\t0x%08x\t\t\t%d\n",
	       reg->ummu_tbu_plb_stat,
	       DV_UMMU_TBU_PLB_STAT, id);
	printf("ummu_tbu_plb_fault_cnt\t\t\t\t0x%08x\t\t\t0x%08x\t\t\t%d\n",
	       reg->ummu_tbu_plb_fault_cnt, 0, id);
	printf("ummu_tbu_invld_mg_info\t\t\t\t0x%08x\t\t\t0x%08x\t\t\t%d\n",
	       reg->ummu_tbu_invld_mg_info,
	       DV_UMMU_TBU_INVLD_MG_INFO, id);
	printf("ummu_tbu_rab_stat\t\t\t\t0x%08x\t\t\t0x%08x\t\t\t%d\n",
	       reg->ummu_tbu_rab_stat,
	       DV_UMMU_TBU_RAB_STAT, id);
	printf("ummu_tbu_rab_entry_info_0\t\t\t0x%08x\t\t\t0x%08x\t\t\t%d\n",
	       reg->ummu_tbu_rab_entry_info_0, 0, id);
	printf("ummu_tbu_cnt\t\t\t\t\t0x%08x\t\t\t0x%08x\t\t\t%d\n",
	       reg->ummu_tbu_cnt, 0, id);
	printf("ummu_dfx_tbu_perm_err_cnt\t\t\t0x%08x\t\t\t0x%08x\t\t\t%d\n",
	       reg->ummu_dfx_tbu_perm_err_cnt, 0, id);
	printf("ummu_tbu_dfx_0\t\t\t\t\t0x%08x\t\t\t0x%08x\t\t\t%d\n",
	       reg->ummu_tbu_dfx_0, 0, id);
	printf("ummu_dfx_ecc_monitor_0\t\t\t\t0x%08x\t\t\t0x%08x\t\t\t%d\n",
	       reg->ummu_dfx_ecc_monitor_0, 0, id);
	printf("ummu_err_status_0\t\t\t\t0x%08x\t\t\t0x%08x\t\t\t%d\n",
	       reg->ummu_err_status_0, 0, id);
	printf("ummu_err_misc1_0\t\t\t\t0x%08x\t\t\t0x%08x\t\t\t%d\n",
	       reg->ummu_err_misc1_0, 0, id);
	printf("\n");
}

int ummu_dump_tbu_execute(uint8_t ummu_id)
{
	struct ummu_reg_dump_para req_para = { 0 };
	struct hikp_cmd_header req_header = { 0 };
	struct ummu_tbu_reg *tbu_reg;
	struct hikp_cmd_ret *cmd_ret;
	size_t expected_size;
	int ret, cnt, idx;

	req_para.ummu_id = ummu_id;

	hikp_cmd_init(&req_header, UMMU_MOD, UMMU_CMD_DUMP, UMMU_TBU_DUMP);
	cmd_ret = hikp_cmd_alloc(&req_header, &req_para, sizeof(req_para));
	if (!cmd_ret)
		return -ENOSPC;
	expected_size = sizeof(struct ummu_tbu_reg);
	ret = ummu_dump_reg_rsp_data_check(cmd_ret, expected_size);
	if (ret)
		goto free_cmd_ret;

	tbu_reg = (struct ummu_tbu_reg *)cmd_ret->rsp_data;
	if (ummu_id != MAX_UMMU_NUM) {
		print_ummu_dfx_title();
		dump_tbu_reg_parse(tbu_reg, ummu_id);
		goto free_cmd_ret;
	}

	cnt = get_ummu_data_cnt(cmd_ret, expected_size);
	if (cnt < 0) {
		ret = cnt;
		goto free_cmd_ret;
	}

	print_ummu_dfx_title();
	for (idx = 0; idx < cnt; idx++)
		dump_tbu_reg_parse(tbu_reg + idx, idx);

free_cmd_ret:
	hikp_cmd_free(&cmd_ret);
	return ret;
}

static void dump_tcu_reg_parse(struct ummu_tcu_reg *reg, int id)
{
	printf("ummu_tcu_ptw_queue_stat\t\t\t\t0x%08x\t\t\t0x%08x\t\t\t%d\n",
	       reg->ummu_tcu_ptw_queue_stat, 0, id);
	printf("ummu_tcu_pptw_queue_stat\t\t\t0x%08x\t\t\t0x%08x\t\t\t%d\n",
	       reg->ummu_tcu_pptw_queue_stat, 0, id);
	printf("ummu_dfx_ecc_monitor_1\t\t\t\t0x%08x\t\t\t0x%08x\t\t\t%d\n",
	       reg->ummu_dfx_ecc_monitor_1, 0, id);
	printf("ummu_err_status_0\t\t\t\t0x%08x\t\t\t0x%08x\t\t\t%d\n",
	       reg->ummu_err_status_0, 0, id);
	printf("ummu_err_misc1_0\t\t\t\t0x%08x\t\t\t0x%08x\t\t\t%d\n",
	       reg->ummu_err_misc1_0, 0, id);
	printf("\n");
}

int ummu_dump_tcu_execute(uint8_t ummu_id)
{
	struct ummu_reg_dump_para req_para = { 0 };
	struct hikp_cmd_header req_header = { 0 };
	struct ummu_tcu_reg *tcu_reg;
	struct hikp_cmd_ret *cmd_ret;
	size_t expected_size;
	int ret, cnt, idx;

	req_para.ummu_id = ummu_id;

	hikp_cmd_init(&req_header, UMMU_MOD, UMMU_CMD_DUMP, UMMU_TCU_DUMP);
	cmd_ret = hikp_cmd_alloc(&req_header, &req_para, sizeof(req_para));
	if (!cmd_ret)
		return -ENOSPC;
	expected_size = sizeof(struct ummu_tcu_reg);
	ret = ummu_dump_reg_rsp_data_check(cmd_ret, expected_size);
	if (ret)
		goto free_cmd_ret;

	tcu_reg = (struct ummu_tcu_reg *)cmd_ret->rsp_data;
	if (ummu_id != MAX_UMMU_NUM) {
		print_ummu_dfx_title();
		dump_tcu_reg_parse(tcu_reg, ummu_id);
		goto free_cmd_ret;
	}

	cnt = get_ummu_data_cnt(cmd_ret, expected_size);
	if (cnt < 0) {
		ret = cnt;
		goto free_cmd_ret;
	}

	print_ummu_dfx_title();
	for (idx = 0; idx < cnt; idx++)
		dump_tcu_reg_parse(tcu_reg + idx, idx);

free_cmd_ret:
	hikp_cmd_free(&cmd_ret);
	return ret;
}

static void dump_sky_reg_parse(struct ummu_sky_reg *reg, int id)
{
	printf("ummu_err_status_0\t\t\t\t0x%08x\t\t\t0x%08x\t\t\t%d\n",
	       reg->ummu_err_status_0, 0, id);
	printf("ummu_err_addr_0\t\t\t\t\t0x%08x\t\t\t0x%08x\t\t\t%d\n",
	       reg->ummu_err_addr_0, 0, id);
	printf("ummu_err_addr_1\t\t\t\t\t0x%08x\t\t\t0x%08x\t\t\t%d\n",
	       reg->ummu_err_addr_1, 0, id);
	printf("\n");
}

int ummu_dump_sky_execute(uint8_t ummu_id)
{
	struct ummu_reg_dump_para req_para = { 0 };
	struct hikp_cmd_header req_header = { 0 };
	struct ummu_sky_reg *sky_reg;
	struct hikp_cmd_ret *cmd_ret;
	size_t expected_size;
	int ret, cnt, idx;

	req_para.ummu_id = ummu_id;

	hikp_cmd_init(&req_header, UMMU_MOD, UMMU_CMD_DUMP, UMMU_SKY_DUMP);
	cmd_ret = hikp_cmd_alloc(&req_header, &req_para, sizeof(req_para));
	if (!cmd_ret)
		return -ENOSPC;
	expected_size = sizeof(struct ummu_sky_reg);
	ret = ummu_dump_reg_rsp_data_check(cmd_ret, expected_size);
	if (ret)
		goto free_cmd_ret;

	sky_reg = (struct ummu_sky_reg *)cmd_ret->rsp_data;
	if (ummu_id != MAX_UMMU_NUM) {
		print_ummu_dfx_title();
		dump_sky_reg_parse(sky_reg, ummu_id);
		goto free_cmd_ret;
	}

	cnt = get_ummu_data_cnt(cmd_ret, expected_size);
	if (cnt < 0) {
		ret = cnt;
		goto free_cmd_ret;
	}

	print_ummu_dfx_title();
	for (idx = 0; idx < cnt; idx++)
		dump_sky_reg_parse(sky_reg + idx, idx);

free_cmd_ret:
	hikp_cmd_free(&cmd_ret);
	return ret;
}

int ummu_get_valid_cnt(uint32_t *cnt)
{
	struct ummu_reg_dump_para req_para = { 0 };
	struct hikp_cmd_header req_header = { 0 };
	struct hikp_cmd_ret *cmd_ret;
	size_t expected_size;
	int ret;

	hikp_cmd_init(&req_header, UMMU_MOD, UMMU_CMD_DUMP, UMMU_CNT_DUMP);
	cmd_ret = hikp_cmd_alloc(&req_header, &req_para, sizeof(req_para));
	if (!cmd_ret)
		return -ENOSPC;
	expected_size = sizeof(uint32_t);
	ret = ummu_dump_reg_rsp_data_check(cmd_ret, expected_size);
	if (ret)
		goto free_cmd_ret;
	*cnt = cmd_ret->rsp_data[0];
free_cmd_ret:
	hikp_cmd_free(&cmd_ret);
	return ret;
}

int ummu_dump_cnt_execute(void)
{
	uint32_t cnt;
	int ret;

	ret = ummu_get_valid_cnt(&cnt);
	if (ret)
		return ret;
	printf("ummu cnt = %u\n", cnt);
	return 0;
}

int ummu_set_sync_timeout(uint8_t sync_timeout_open, uint8_t ummu_id)
{
	struct ummu_reg_dump_para req_para = { 0 };
	struct hikp_cmd_header req_header = { 0 };
	struct hikp_cmd_ret *cmd_ret;
	uint32_t *sync_timeout_reg;
	size_t expected_size;
	int ret, idx, cnt;

	req_para.sync_timeout_open = sync_timeout_open;
	req_para.ummu_id = ummu_id;

	hikp_cmd_init(&req_header, UMMU_MOD, UMMU_CMD_SYNC_TIMEOUT, 0);
	cmd_ret = hikp_cmd_alloc(&req_header, &req_para, sizeof(req_para));
	if (!cmd_ret)
		return -ENOSPC;
	expected_size = sizeof(uint32_t);
	ret = ummu_dump_reg_rsp_data_check(cmd_ret, expected_size);
	if (ret)
		goto free_cmd_ret;

	sync_timeout_reg = cmd_ret->rsp_data;
	if (ummu_id != MAX_UMMU_NUM) {
		printf("UMMU id is %hhu\n", ummu_id);
		printf("ummu_sync_timeout_open = %u\n", *sync_timeout_reg);
		goto free_cmd_ret;
	}

	cnt = get_ummu_data_cnt(cmd_ret, expected_size);
	if (cnt < 0) {
		ret = cnt;
		goto free_cmd_ret;
	}

	for (idx = 0; idx < cnt; idx++) {
		printf("UMMU id is %d\n", idx);
		printf("ummu_sync_timeout_open = %u\n", *(sync_timeout_reg + idx));
	}

free_cmd_ret:
	hikp_cmd_free(&cmd_ret);
	return ret;
}
