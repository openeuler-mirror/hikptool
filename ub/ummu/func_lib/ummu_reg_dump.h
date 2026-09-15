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
#define MAX_DFX_PTW_QUEUE_PROBE_ID 39
#define MAX_DFX_PPTW_QUEUE_PROBE_ID 31
#define MAX_DFX_GPC_QUEUE_PROBE_ID 15
#define MAX_CCT_QUE_SEL_DFX 31
#define MAX_DFX_SKY_QUEUE_PROBE_ID_SP 63
#define MAX_SYNC_TIMEOUT_VAL 1
#define MAX_UMMU_NUM 8

struct ummu_reg_dump_para {
	uint8_t ummu_id;
	uint8_t cache_idx;
	uint8_t sync_timeout_open;
	uint8_t rr_win_num;
	uint8_t kcmd_entry_no;
	uint8_t dfx_ptw_queue_probe_id;
	uint8_t dfx_pptw_queue_probe_id;
	uint8_t dfx_gpc_queue_probe_id;
	uint8_t cct_que_sel_dfx;
	uint8_t dfx_sky_queue_probe_id_sp;
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
	uint32_t ummu_swif_fsm_status_dfx0;
	uint32_t ummu_swif_fsm_status_dfx1;
	uint32_t ummu_swif_fsm_status_dfx2;
	uint32_t ummu_swif_fsm_status_dfx3;
	uint32_t ummu_swif_fsm_status_dfx4;
	uint32_t ummu_swif_dfx_qid_status0;
	uint32_t ummu_swif_dfx_qid_status1;
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
	uint32_t ummu_swif_umcmd_no_ready;
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
	uint32_t ummu_ubif_queue_dfx;
	uint32_t ummu_ubif_kv_cache_dfx;
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
	uint32_t ummu_tbu_iopf_sts0;
	uint32_t ummu_tbu_iopf_sts1;
	uint32_t ummu_tbu_iopf_sts2;
	uint32_t ummu_tbu_iopf_sts3;
	uint32_t ummu_tbu_rab_tect_tag_info;
};

struct ummu_tcu_reg {
	uint32_t ummu_tcu_ptw_queue_stat;
	uint32_t ummu_tcu_pptw_queue_pointer;
	uint32_t ummu_tcu_pptw_queue_stat;
	uint32_t ummu_dfx_ecc_monitor_1;
	/* RAS */
	uint32_t ummu_err_status_0;
	uint32_t ummu_err_misc1_0;
	uint32_t ummu_dfx_ecc_monitor_2;
	uint32_t ummu_tcu_ptw_queue_pointer;
	uint32_t ummu_tcu_pptw_queue_stat1;
	uint32_t ummu_gpc_queue_pointer;
	uint32_t ummu_gpc_queue_stat;
	uint32_t ummu_tcu_itf_stat0;
	uint32_t ummu_cct_req_que_sel_dfx;
	uint32_t ummu_cct_req_que_info0_dfx;
	uint32_t ummu_cct_req_que_info1_dfx;
	uint32_t ummu_cct_req_que_info2_dfx;
	uint32_t ummu_cct_req_que_info3_dfx;
	uint32_t ummu_cct_req_que_info4_dfx;
	uint32_t ummu_tcu_queue_stat;
};

struct ummu_sky_reg {
	/* RAS */
	uint32_t ummu_err_status_0;
	uint32_t ummu_err_addr_0;
	uint32_t ummu_err_addr_1;
	uint32_t ummu_sky_queue_pointer_sp;
	uint32_t ummu_sky_queue_addr_low_sp;
	uint32_t ummu_sky_queue_addr_high_sp;
	uint32_t ummu_sky_queue_stat0_sp;
	uint32_t ummu_sky_queue_stat1_sp;
	uint32_t ummu_sky_queue_stat2_sp;
	uint32_t ummu_sky_queue_stat3_sp;
};

int ummu_dump_kcmd_execute(uint8_t kcmd_entry_no, uint8_t ummu_id);
int ummu_dump_umcmd_execute(uint8_t cache_idx, uint8_t rr_win_num, uint8_t ummu_id);
int ummu_dump_ubif_execute(uint8_t ummu_id);
int ummu_dump_tbu_execute(uint8_t ummu_id);
int ummu_dump_tcu_execute(uint8_t dfx_ptw_queue_probe_id, uint8_t dfx_pptw_queue_probe_id,
			  uint8_t dfx_gpc_queue_probe_id, uint8_t cct_que_sel_dfx,
			  uint8_t ummu_id);
int ummu_dump_sky_execute(uint8_t dfx_sky_queue_probe_id_sp, uint8_t ummu_id);
int ummu_get_valid_cnt(uint32_t *cnt);
int ummu_dump_cnt_execute(void);

int ummu_set_sync_timeout(uint8_t sync_timeout_open, uint8_t ummu_id);

#endif /* UMMU_REG_DUMP_H */
