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

#ifndef __CXL_FEATURE_H_
#define __CXL_FEATURE_H_

#include "tool_cmd.h"

#define CPA_ERR_MSG_INFO_CNT 8
#define CPA_CFG_MSG_INFO_CNT 5
#define CPA_MMRG_MSG_INFO_CNT 10
#define CXL_DL_DFX_MSG_INFO_CNT 6
#define CXL_DL_ERR_MSG_INFO_CNT 6
#define CXL_RCRB_LINK_INFO_CNT 2
#define CXL_RCRB_CFG_HEADER_INFO_CNT 12
#define CXL_MEMBAR_ERR_INFO_CNT 6
#define CXL_DL_FSM_STATE_STR_NUM 0x20
#define CXL_HDM_CNT_EACH_PORT 2
#define CXL_DATA_UNIT_SIZE 2  // reg addr + data
#define CXL_MEM_HEADER_LOG_UNIT 4

enum cxl_cmd_type {
	CXL_CPA = 0,
	CXL_DL,
	CXL_RCRB,
	CXL_MEMBAR
};

enum cxl_cpa_type {
	CPA_UNKNOWN_TYPE = 0,
	CPA_ERR,
	CPA_MMRG,
	CPA_DUMP,
	CPA_CONFIG
};

enum cxl_dl_type {
	CXL_DL_UNKNOWN_TYPE = 0,
	CXL_DL_FSM_STATE,
	CXL_DL_DFX,
	CXL_DL_DUMP,
	CXL_DL_ERR
};

enum cxl_rcrb_type {
	CXL_RCRB_UNKNOWN_TYPE = 0,
	CXL_RCRB_LINK,
	CXL_RCRB_HDR,
	CXL_RCRB_DUMP
};

enum cxl_membar_type {
	CXL_MEMBAR_UNKNOWN_TYPE = 0,
	CXL_MEMBAR_ERR,
	CXL_MEMBAR_DUMP
};

enum cxl_membar_header_log {
	CXL_HEADER_LOG0 = 0,
	CXL_HEADER_LOG1,
	CXL_HEADER_LOG2,
	CXL_HEADER_LOG3
};

struct cxl_cmd_paras_in {
	uint32_t port_id;
};

struct cxl_out_data {
	uint32_t length;
	uint32_t data_offset;
};

struct cxl_data_unit {
	uint32_t data_addr;
	uint32_t data;
};

struct cxl_info_desc {
	const char *info_msg;
	void (*info_prt)(uint32_t data);
};

struct cxl_fsm_state_str {
	int fsm_state;
	char fsm_str[CXL_DL_FSM_STATE_STR_NUM];
};

struct cxl_prt_func {
	uint32_t mode_code;
	uint32_t cmd_type;
	void (*cxl_prt_handle)(const struct cxl_data_unit *data, uint32_t data_size);
};

int cxl_reg_show_execute(uint32_t port_id, uint32_t mode_code, uint32_t cmd_type);

union cpa_cfg_key {
	struct {
		uint32_t cfg_cpa_port_en : 4;             /* [3..0] */
		uint32_t cfg_cpa_dmt_disable : 1;         /* [4] */
		uint32_t cfg_cpa_send_32byte : 1;         /* [5] */
		uint32_t cfg_cpa_wcud_send_en : 1;        /* [6] */
		uint32_t cfg_cpa_icg_en : 1;              /* [7] */
		uint32_t cfg_cpa_mee_ns_sel : 1;          /* [8] */
		uint32_t cfg_cpa_pmicg_en : 1;            /* [9] */
		uint32_t cfg_cpa_rxdatreq_cango_th : 9;   /* [18..10] */
		uint32_t cfg_cpa_port_remap : 1;          /* [19] */
		uint32_t reserved_0 : 1;                  /* [20] */
		uint32_t reserved_1 : 1;                  /* [21] */
		uint32_t cfg_cpa_m2s_req_a5_tie0 : 1;     /* [22] */
		uint32_t cfg_cpa_pcie_err_rpt_enable : 1; /* [23] */
		uint32_t cfg_cpa_retry_switch : 2;        /* [25..24] */
		uint32_t reserved_2 : 6;                  /* [31..26] */
	} bits;

	uint32_t val;
};

union cpa_ctl_cfg {
	struct {
		uint32_t cpa_csp_en : 1;             /* [0] */
		uint32_t cpa_comp_blk_en : 1;        /* [1] */
		uint32_t cpa_wnsp_early_cmp_en : 1;  /* [2] */
		uint32_t cpa_wtnsnp_cmpdbid_en : 1;  /* [3] */
		uint32_t cpa_ptl_access_en : 1;      /* [4] */
		uint32_t cpa_reqrsdata_bps_en : 1;   /* [5] */
		uint32_t cpa_reqrsp_intrl_en : 1;    /* [6] */
		uint32_t cpa_rspdat_intrl_en : 1;    /* [7] */
		uint32_t cpa_wtevict_drop_en : 1;    /* [8] */
		uint32_t cpa_resp_sc_drop_en : 1;    /* [9] */
		uint32_t cfg_be_azero_drop_en : 1;   /* [10] */
		uint32_t cpa_timeout_poison_en : 1;  /* [11] */
		uint32_t cpa_err_reqcmd_send_en : 1; /* [12] */
		uint32_t cpa_req_err_rsp_en : 1;     /* [13] */
		uint32_t cpa_rsp_exok_en : 1;        /* [14] */
		uint32_t reserved_0 : 1;             /* [15] */
		uint32_t cpa_rwd_pend_crd : 1;       /* [16] */
		uint32_t cpa_wr_metavl : 2;          /* [18..17] */
		uint32_t cpa_wr_metafd : 2;          /* [20..19] */
		uint32_t cpa_wr_snptype : 3;         /* [23..21] */
		uint32_t cpa_rd_metavl : 2;          /* [25..24] */
		uint32_t cpa_rd_metafd : 2;          /* [27..26] */
		uint32_t cpa_rd_snptype : 3;         /* [30..28] */
		uint32_t cpa_drs_pend_crd : 1;       /* [31] */
	} bits;

	uint32_t val;
};

union cpa_err_send_ctl {
	struct {
		uint32_t acc2core_resperr_en : 1;     /* [0] */
		uint32_t acc2core_poison_en : 1;      /* [1] */
		uint32_t core2acc_resperr_en : 1;     /* [2] */
		uint32_t core2acc_poison_en : 1;      /* [3] */
		uint32_t cfg_cpa_rspcmd_halt_en : 1;  /* [4] */
		uint32_t cfg_cpa_rspcmd_halt_clr : 1; /* [5] */
		uint32_t cfg_cpa_reqcmd_halt_en : 1;  /* [6] */
		uint32_t cfg_cpa_reqcmd_halt_clr : 1; /* [7] */
		uint32_t reserved_0 : 24;             /* [31..8] */
	} bits;

	uint32_t val;
};

union cpa_timeout_ctl {
	struct {
		uint32_t cfg_timeout_rsp_fb : 1; /* [0] */
		uint32_t reserved_0 : 3;         /* [3..1] */
		uint32_t cfg_timeout_th : 16;    /* [19..4] */
		uint32_t reserved_1 : 12;        /* [31..20] */
	} bits;

	uint32_t val;
};

union cpa_cfg_mee_enable {
	struct {
		uint32_t p0_cfg_cpa_mee_enable : 1; /* [0] */
		uint32_t p1_cfg_cpa_mee_enable : 1; /* [1] */
		uint32_t reserved_0 : 30;           /* [31..2] */
	} bits;

	uint32_t val;
};

union cxl_dl_fsm_state_reg {
	struct {
		uint32_t init_fsm_state : 4;        /* [3..0] */
		uint32_t lrsm_state : 5;            /* [8..4] */
		uint32_t reserved_0 : 3;            /* [11..9] */
		uint32_t rrsm_state : 1;            /* [12] */
		uint32_t reserved_1 : 19;           /* [31..13] */
	} bits;

	uint32_t val;
};

union cxl_dl_retry_buffer_status {
	struct {
		uint32_t dl_cxl_retry_buffer_w_ptr : 10; /* [9..0] */
		uint32_t dl_cxl_retry_buffer_r_ptr : 10; /* [19..10] */
		uint32_t dl_cxl_retry_buffer_full : 1;   /* [20] */
		uint32_t dl_cxl_retry_buffer_empty : 1;  /* [21] */
		uint32_t reserved_0 : 10;                /* [31..22] */
	} bits;

	uint32_t val;
};

union cxl_dl_dfx_rx_status {
	struct {
		uint32_t numack : 8;                    /* [7..0] */
		uint32_t rcv_eseq : 8;                  /* [15..8] */
		uint32_t g0_num_going : 5;              /* [20..16] */
		uint32_t g0_num_going_rollover : 1;     /* [21] */
		uint32_t g0_num_going_rollover_clr : 1; /* [22] */
		uint32_t rxflit_err_ind : 1;            /* [23] */
		uint32_t rcv_empty : 1;                 /* [24] */
		uint32_t rcv_viral : 1;                 /* [25] */
		uint32_t reserved_0 : 6;                /* [31..26] */
	} bits;

	uint32_t val;
};

union cxl_dl_dfx_llr_var {
	struct {
		uint32_t eseq : 8;          /* [7..0] */
		uint32_t llrb_wrptr : 8;    /* [15..8] */
		uint32_t llrb_rdptr : 8;    /* [23..16] */
		uint32_t numfreebuf : 8;    /* [31..24] */
	} bits;

	uint32_t val;
};

union cxl_dl_dfx_alm_err_num {
	struct {
		uint32_t alm_err_num : 8; /* [7..0] */
		uint32_t reserved_0 : 24; /* [31..8] */
	} bits;

	uint32_t val;
};

union cxl_dl_init_signal {
	struct {
		uint32_t rcv_vld_flit_flag : 1;        /* [0] */
		uint32_t rcv_init_param_flit_flag : 1; /* [1] */
		uint32_t init_flit_send_flag : 1;      /* [2] */
		uint32_t cpa_receive_flit_rdy : 1;     /* [3] */
		uint32_t alm_sch_en : 1;               /* [4] */
		uint32_t reserved_0 : 27;              /* [31..5] */
	} bits;

	uint32_t val;
};

union cxl_dl_dfx_retry_num {
	struct {
		uint32_t num_retry : 5;      /* [4..0] */
		uint32_t num_phy_reinit : 5; /* [9..5] */
		uint32_t reserved_0 : 22;    /* [31..10] */
	} bits;

	uint32_t val;
};

union cxl_dl_int_info {
	struct {
		uint32_t init_timeout_int : 1;              /* [0] */
		uint32_t cxldl_down_int : 1;                /* [1] */
		uint32_t reach_max_retry_num_int : 1;       /* [2] */
		uint32_t retry_abort_int : 1;               /* [3] */
		uint32_t rcv_retry_ack_error_int : 1;       /* [4] */
		uint32_t rcv_static_error_int : 1;          /* [5] */
		uint32_t ecc_1b_err_cxldl_int : 1;          /* [6] */
		uint32_t ecc_2b_err_cxldl_int : 1;          /* [7] */
		uint32_t rcv_non_retry_before_init_int : 1; /* [8] */
		uint32_t rcv_dup_init_param_int : 1;        /* [9] */
		uint32_t rcv_viral_retry_ack_int : 1;       /* [10] */
		uint32_t cpa_indicate_viral_int : 1;        /* [11] */
		uint32_t reserved_0 : 4;                    /* [15..12] */
		uint32_t cxldl_up_int : 1;                  /* [16] */
		uint32_t reserved_1 : 15;                   /* [31..17] */
	} bits;

	uint32_t val;
};

union cxl_dl_ecc_err_cnt {
	struct {
		uint32_t ram0_dl_ecc_err_cnt : 5; /* [4..0] */
		uint32_t ram1_dl_ecc_err_cnt : 5; /* [9..5] */
		uint32_t reserved_0 : 22;         /* [31..10] */
	} bits;

	uint32_t val;
};

union cxl_dl_ram_ecc_err_addr {
	struct {
		uint32_t dl_ecc_1bit_err_addr : 10; /* [9..0] */
		uint32_t reserved_0 : 6;            /* [15..10] */
		uint32_t dl_ecc_2bit_err_addr : 10; /* [25..16] */
		uint32_t reserved_1 : 6;            /* [31..26] */
	} bits;

	uint32_t val;
};

union cxl_dl_ram_ecc_state {
	struct {
		uint32_t ram0_1bit_ecc_err_status : 1; /* [0]  */
		uint32_t ram0_2bit_ecc_err_status : 1; /* [1]  */
		uint32_t ram1_1bit_ecc_err_status : 1; /* [2]  */
		uint32_t ram1_2bit_ecc_err_status : 1; /* [3]  */
		uint32_t reserved_0 : 28;              /* [31..4]  */
	} bits;

	uint32_t val;
};

union cxl_rcrb_vendor_spec_header {
	struct {
		uint32_t dvsec_id : 16;         /* [15..0] */
		uint32_t cache_capable : 1;     /* [16] */
		uint32_t io_capable : 1;        /* [17] */
		uint32_t mem_capable : 1;       /* [18] */
		uint32_t reserved_0 : 13;       /* [31..19] */
	} bits;

	uint32_t val;
};

union cxl_rcrb_flex_bus_ctrl {
	struct {
		uint32_t cache_enable : 1;                 /* [0] */
		uint32_t io_enable : 1;                    /* [1] */
		uint32_t mem_enable : 1;                   /* [2] */
		uint32_t reserved_0 : 13;                  /* [15..3] */
		uint32_t cache_enabled : 1;                /* [16] */
		uint32_t io_enabled : 1;                   /* [17] */
		uint32_t mem_enabled : 1;                  /* [18] */
		uint32_t reserved_1 : 13;                  /* [31..19] */
	} bits;

	uint32_t val;
};

union cxl_mem_uncorrect_err {
	struct {
		uint32_t cache_data_parity : 1;       /* [0] */
		uint32_t cache_addr_parity : 1;       /* [1] */
		uint32_t cache_be_parity : 1;         /* [2] */
		uint32_t cache_data_ecc : 1;          /* [3] */
		uint32_t mem_data_parity : 1;         /* [4] */
		uint32_t mem_address_parity : 1;      /* [5] */
		uint32_t mem_be_parity : 1;           /* [6] */
		uint32_t mem_data_ecc : 1;            /* [7] */
		uint32_t reinit_threshold : 1;        /* [8] */
		uint32_t rsvd_encoding_violation : 1; /* [9] */
		uint32_t poison_received : 1;         /* [10] */
		uint32_t receiver_overflow : 1;       /* [11] */
		uint32_t reserved_0 : 20;             /* [31..12] */
	} bits;

	uint32_t val;
};

union cxl_mem_correct_err {
	struct {
		uint32_t cache_data_ecc : 1;           /* [0] */
		uint32_t mem_data_ecc : 1;             /* [1] */
		uint32_t crc_threshold : 1;            /* [2] */
		uint32_t retry_threshold : 1;          /* [3] */
		uint32_t cache_poison_received : 1;    /* [4] */
		uint32_t mem_poison_received : 1;      /* [5] */
		uint32_t physical_layer_error : 1;     /* [6] */
		uint32_t reserved_0 : 25;              /* [31..7] */
	} bits;

	uint32_t val;
};

union cxl_mem_error_ctrl {
	struct {
		uint32_t first_error_pointer : 4;                  /* [3..0]  */
		uint32_t reserved_0 : 5;                           /* [8..4]  */
		uint32_t multiple_header_recording_capability : 1; /* [9]  */
		uint32_t reserved_1 : 3;                           /* [12..10]  */
		uint32_t poison_enable : 1;                        /* [13]  */
		uint32_t reserved_2 : 18;                          /* [31..14]  */
	} bits;

	uint32_t val;
};

#endif
