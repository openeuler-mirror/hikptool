/*
 * Copyright (c) 2025 Hisilicon Technologies Co., Ltd.
 * Hikptool is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS; WITHOUT WARRANTIES OF ANY KIND;
 * EITHER EXPRESS OR IMPLIED; INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT;
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 *
 * See the Mulan PSL v2 for more details.
 */

#ifndef UMMU_REG_DUMP_H
#define UMMU_REG_DUMP_H

#include <stdint.h>

#define MAX_KCMD_ENTRY_NO 15
#define MAX_RR_WIN_NUM 6
#define MAX_CACHE_IDX 15
#define MAX_SYNC_TIMEOUT_VAL 1
#define MAX_UMMU_NUM 8

struct ummu_reg_dump_para {
	uint8_t ummu_id;
	uint8_t cache_idx;
	uint8_t sync_timeout_open;
	uint8_t rr_win_num;
	uint8_t kcmd_entry_no;
};

struct ummu_kcmd_reg {
	/* USI */
	uint32_t ummu_glb_err;
	/* DFX */
	uint32_t ummu_swif_kcmdq_dfx_kcmd_status;
	uint32_t ummu_swif_kcmdq_dfx_kcmd_err_status;
	uint32_t ummu_swif_kcmdq_dfx_cmd_entry_status;
	uint32_t ummu_swif_kcmdq_dfx_snp_err_cnt;
	uint32_t ummu_swif_kcmdq_dfx_snp_status;
	uint32_t ummu_swif_kcmdq_dfx_cmd_ctrl_status_1;
	uint32_t ummu_swif_kcmdq_dfx_cmd_ctrl_status_2;
	uint32_t ummu_dvm_receive_req_cnt;
	uint32_t ummu_dvm_send_req_cnt;
	uint32_t ummu_dvm_req_info0;
	uint32_t ummu_dvm_req_info1;
	uint32_t ummu_mcmd_que_ci;
	uint32_t ummu_sync_timeout_info;
	uint32_t ummu_swif_eventq_dfx_drop_cnt;
	/* RAS */
	uint32_t ummu_err_status_0;
	uint32_t ummu_err_status_1;
	uint32_t ummu_err_addr_0;
	uint32_t ummu_err_addr_1;
};

struct ummu_umcmd_reg {
	/* USI */
	uint32_t ummu_glb_err;
	/* DFX */
	uint32_t ummu_swif_umcmd_dfx_0;
	uint32_t ummu_swif_umcmd_dfx_1;
	uint32_t ummu_swif_umcmd_dfx_2;
	uint32_t ummu_swif_umcmd_dfx_3;
	uint32_t ummu_swif_umcmd_rr_win_dfx_0;
	uint32_t ummu_swif_umcmd_rr_win_dfx_1;
	uint32_t ummu_swif_umcmd_rr_win_dfx_2;
	uint32_t ummu_swif_umcmd_cache_dfx_1;
	uint32_t ummu_swif_umcmd_cache_dfx_2;
	uint32_t ummu_swif_umcmd_cache_dfx_3;
	uint32_t ummu_swif_umcmd_cache_dfx_4;
	uint32_t ummu_swif_umcmd_cache_dfx_5;
	uint32_t ummu_swif_umcmd_cache_dfx_6;
	/* RAS */
	uint32_t ummu_err_status_0;
	uint32_t ummu_err_addr_0;
	uint32_t ummu_err_addr_1;
};

struct ummu_ubif_reg {
	uint32_t ummu_ubif_sync_dfx;
	uint32_t ummu_ubif_dfx_0;
	uint32_t ummu_ubif_dfx_1;
	uint32_t ummu_ubif_dfx_2;
	uint32_t ummu_ubif_dsteid_dfx;
	uint32_t ummu_ubif_kv_cache_ns_nse_mismatch_dfx_0;
	uint32_t ummu_ubif_kv_cache_ns_nse_mismatch_dfx_1;
	uint32_t ummu_ubif_kv_cache_ns_nse_mismatch_dfx_2;
	uint32_t ummu_ubif_kv_cache_ns_nse_mismatch_dfx_3;
	uint32_t ummu_ubif_kv_cache_ns_nse_mismatch_dfx_4;
	/* RAS */
	uint32_t ummu_err_status_0;
	uint32_t ummu_err_addr_0;
	uint32_t ummu_err_addr_1;
};

struct ummu_tbu_reg {
	uint32_t ummu_tbu_tlb_lkup_proc;
	uint32_t ummu_tbu_tlb_stat;
	uint32_t ummu_tbu_tlb_fault_cnt;
	uint32_t ummu_tbu_plb_lkup_proc;
	uint32_t ummu_tbu_plb_stat;
	uint32_t ummu_tbu_plb_fault_cnt;
	uint32_t ummu_tbu_invld_mg_info;
	uint32_t ummu_tbu_rab_stat;
	uint32_t ummu_tbu_rab_entry_info_0;
	uint32_t ummu_tbu_cnt;
	uint32_t ummu_dfx_tbu_perm_err_cnt;
	uint32_t ummu_tbu_dfx_0;
	uint32_t ummu_dfx_ecc_monitor_0;
	/* RAS */
	uint32_t ummu_err_status_0;
	uint32_t ummu_err_misc1_0;
};

struct ummu_tcu_reg {
	uint32_t ummu_tcu_ptw_queue_stat;
	uint32_t ummu_tcu_pptw_queue_stat;
	uint32_t ummu_dfx_ecc_monitor_1;
	/* RAS */
	uint32_t ummu_err_status_0;
	uint32_t ummu_err_misc1_0;
};

struct ummu_sky_reg {
	/* RAS */
	uint32_t ummu_err_status_0;
	uint32_t ummu_err_addr_0;
	uint32_t ummu_err_addr_1;
};

int ummu_dump_kcmd_execute(uint8_t kcmd_entry_no, uint8_t ummu_id);
int ummu_dump_umcmd_execute(uint8_t cache_idx, uint8_t rr_win_num, uint8_t ummu_id);
int ummu_dump_ubif_execute(uint8_t ummu_id);
int ummu_dump_tbu_execute(uint8_t ummu_id);
int ummu_dump_tcu_execute(uint8_t ummu_id);
int ummu_dump_sky_execute(uint8_t ummu_id);
int ummu_get_valid_cnt(uint32_t *cnt);
int ummu_dump_cnt_execute(void);
int ummu_set_sync_timeout(uint8_t sync_timeout_open, uint8_t ummu_id);

#endif /* UMMU_REG_DUMP_H */
