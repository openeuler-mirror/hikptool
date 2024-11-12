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
#include "os_common.h"
#include "pcie_common.h"
#include "hikptdev_plug.h"
#include "pcie_link_ltssm.h"

union ltssm_state_reg {
	struct {
		uint64_t ltssm_state : 6;                  /* [0:5] */
		uint64_t duration_counter : 4;             /* [6:9] */
		uint64_t mac_rate : 2;                     /* [10:11] */
		uint64_t train_bit_map1 : 8;               /* [12:19] */
		uint64_t rxl0s_st : 2;                     /* [20:21] */
		uint64_t any_change_pipe_req : 1;          /* [22] */
		uint64_t rcv_eios : 1;                     /* [23] */
		uint64_t dl_retrain : 1;                   /* [24] */
		uint64_t all_phy_rxeleidle : 1;            /* [25] */
		uint64_t directed_speed_change : 1;        /* [26] */
		uint64_t any_det_eieos_ts : 1;             /* [27] */
		uint64_t rxl0s_to_recovery : 1;            /* [28] */
		uint64_t any_lane_rcv_speed_change : 1;    /* [29] */
		uint64_t changed_speed_recovery : 1;       /* [30] */
		uint64_t successful_speed_negotiation : 1; /* [31] */
		uint64_t train_bit_map2 : 16;              /* [32:47] */
		uint64_t txdetrx : 16;                     /* [48:63] */
	} bits;
	uint64_t val;
};

union pm_state_reg {
	struct {
		uint64_t pm_state : 6;                      /* [0:5] */
		uint64_t pm_clock : 18;                     /* [6:23] */
		uint64_t reserved1 : 8;                     /* [24:31] */
		uint64_t refclk_stable_vld : 1;             /* [32] */
		uint64_t enter_l12_case : 1;                /* [33] */
		uint64_t pm_t_dl_l2_gnt_timeout : 1;        /* [34] */
		uint64_t pm_t_dl_l1_gnt_timeout : 1;        /* [35] */
		uint64_t pm_t_dl_l0s_gnt_timeout : 1;       /* [36] */
		uint64_t pm_t_dl_lastack_timeout : 1;       /* [37] */
		uint64_t pme_turn_off_vld_hold : 1;         /* [38] */
		uint64_t pm_blk_tlp_timeout : 1;            /* [39] */
		uint64_t aspm_nak_vld : 1;                  /* [40] */
		uint64_t retrain_link_vld : 1;              /* [41] */
		uint64_t pending_dllp_vld : 1;              /* [42] */
		uint64_t pm_wakeup_tol0_en : 1;             /* [43] */
		uint64_t mac2pm_rx_data_vld : 1;            /* [44] */
		uint64_t dfe_req : 1;                       /* [45] */
		uint64_t pm_t_dfe_time_meet : 1;            /* [46] */
		uint64_t reserved2 : 17;                    /* [47:63] */
	} bits;
	uint64_t val;
};

static int pcie_get_ltssm_trace(uint32_t port_id, uint64_t *ltssm_status, uint32_t *ltssm_num)
{
	struct hikp_cmd_header req_header;
	struct hikp_cmd_ret *cmd_ret = NULL;
	struct pcie_trace_req_para req_data = { 0 };
	size_t src_size, dst_size;
	int ret;

	req_data.port_id = port_id;
	hikp_cmd_init(&req_header, PCIE_MOD, PCIE_TRACE, TRACE_SHOW);
	cmd_ret = hikp_cmd_alloc(&req_header, &req_data, sizeof(req_data));
	ret = hikp_rsp_normal_check(cmd_ret);
	if (ret) {
		Err("pcie trace cmd_ret check failed, ret: %d.\n", ret);
		goto free_cmd_ret;
	}

	if (cmd_ret->rsp_data_num == 0) {
		Err("without rsp data.\n");
		ret = -EINVAL;
		goto free_cmd_ret;
	}
	/* 0: First uint32_t is ltssm trace num received from TF */
	*ltssm_num = cmd_ret->rsp_data[0];
	src_size = (*ltssm_num) * sizeof(uint64_t);
	dst_size = TRACER_DEPTH * sizeof(uint64_t);
	if (src_size > dst_size) {
		Err("size check failed, %u > %u.\n", src_size, dst_size);
		ret = -EINVAL;
		goto free_cmd_ret;
	}

	if ((cmd_ret->rsp_data_num - 1) * sizeof(uint32_t) != (*ltssm_num) * sizeof(uint64_t)) {
		Err("rsp data number check failed, rsp_data_num: %u, ltssm_num: %u.\n",
		    cmd_ret->rsp_data_num, *ltssm_num);
		ret = -EINVAL;
		goto free_cmd_ret;
	}

	memcpy(ltssm_status, (cmd_ret->rsp_data + 1), src_size);

free_cmd_ret:
	free(cmd_ret);
	return ret;
}

struct pcie_ltssm_num_string g_ltssm_string_table[] = {
	{0x0, "ltssm_init"},
	{0x1, "ltssm_reset_pipe_afifo"},
	{0x2, "ltssm_detect_quiet"},
	{0x3, "ltssm_detect_active"},
	{0x4, "ltssm_detect_wait"},
	{0x5, "ltssm_detect_pwr_p0"},
	{0x6, "ltssm_poll_active"},
	{0x7, "ltssm_poll_config"},
	{0x8, "ltssm_poll_comp"},
	{0x9, "ltssm_config_lw_str"},
	{0xa, "ltssm_config_lw_acc"},
	{0xb, "ltssm_config_ln_wait"},
	{0xc, "ltssm_config_ln_acc"},
	{0xd, "ltssm_config_complete"},
	{0xe, "ltssm_config_idle1"},
	{0xf, "ltssm_config_idle2"},
	{0x10, "ltssm_l0"},
	{0x11, "ltssm_rx_l0s"},
	{0x14, "ltssm_tx_l0s_entry"},
	{0x15, "ltssm_tx_l0s_idle"},
	{0x16, "ltssm_tx_l0s_fts"},
	{0x17, "ltssm_l1"},
	{0x18, "ltssm_l2"},
	{0x19, "ltssm_tx_beacon_begin"},
	{0x1a, "ltssm_tx_beacon_end"},
	{0x30, "ltssm_tx_eios_st"},
	{0x31, "ltssm_chg_rate_gen1"},
	{0x32, "ltssm_change_power"},
	{0x33, "ltssm_hot_reset"},
	{0x34, "ltssm_disable_p1"},
	{0x35, "ltssm_disable_p2"},
	{0x38, "ltssm_loopback_entry"},
	{0x39, "ltssm_loopback_active"},
	{0x3a, "ltssm_loopback_exit"},
	{0x20, "ltssm_recovery_rcvlock"},
	{0x21, "ltssm_recovery_rcvcfg"},
	{0x22, "ltssm_recovery_speed"},
	{0x23, "ltssm_recovery_idle1"},
	{0x24, "ltssm_recovery_idle2"},
	{0x25, "ltssm_recovery_eq_p0"},
	{0x26, "ltssm_recovery_eq_p1"},
	{0x27, "ltssm_recovery_eq_p2"},
	{0x28, "ltssm_recovery_eq_p3"},
	{-1, "unknown"} /* end of array */
};

static char *hisi_pcie_ltssm_string_get(uint32_t ltssm)
{
	int i = 0;

	while (g_ltssm_string_table[i].ltssm >= 0) {
		if ((uint32_t)g_ltssm_string_table[i].ltssm != ltssm) {
			i++;
			continue;
		}
		break;
	}

	return g_ltssm_string_table[i].ltssm_c;
}

static int pcie_print_ltssm_trace(const uint64_t *ltssm_input, uint32_t ltssm_num)
{
	uint32_t i;
	char *ltssm_c = NULL;
	union ltssm_state_reg ltssm_val;

	if (ltssm_num > TRACER_DEPTH || ltssm_num == 0) {
		Err("ltssm_num(%u) is over range or zero\n", ltssm_num);
		return -EINVAL;
	}
	Info("ltssm tracer:\n");
	Info("\ttrace mode: %llx\n", *ltssm_input);
	Info("\tltssm[ii]:  63:48 47:32 31 30 29 28 27 26 25 24 23 22 21:"
	     "20 19:12 11:10 9:6 5:0  ltssm\n");
	for (i = 1; i < ltssm_num; i++) {
		ltssm_val.val = ltssm_input[i];
		ltssm_c = hisi_pcie_ltssm_string_get((uint32_t)ltssm_val.bits.ltssm_state);
		Info("\tltssm[%02u]: 0x%04x %04x   %x  %x  %x  %x  %x  %x  %x  "
			"%x  %x  %x  %x     %02x    %x     %x   %02x  %s\n",
			i,
			(uint32_t)ltssm_val.bits.txdetrx,
			(uint32_t)ltssm_val.bits.train_bit_map2,
			(uint32_t)ltssm_val.bits.successful_speed_negotiation,
			(uint32_t)ltssm_val.bits.changed_speed_recovery,
			(uint32_t)ltssm_val.bits.any_lane_rcv_speed_change,
			(uint32_t)ltssm_val.bits.rxl0s_to_recovery,
			(uint32_t)ltssm_val.bits.any_det_eieos_ts,
			(uint32_t)ltssm_val.bits.directed_speed_change,
			(uint32_t)ltssm_val.bits.all_phy_rxeleidle,
			(uint32_t)ltssm_val.bits.dl_retrain,
			(uint32_t)ltssm_val.bits.rcv_eios,
			(uint32_t)ltssm_val.bits.any_change_pipe_req,
			(uint32_t)ltssm_val.bits.rxl0s_st,
			(uint32_t)ltssm_val.bits.train_bit_map1,
			(((uint32_t)ltssm_val.bits.rxl0s_st) << GEN5_BIT_OFFEST) |
			((uint32_t)ltssm_val.bits.mac_rate),
			(uint32_t)ltssm_val.bits.duration_counter,
			(uint32_t)ltssm_val.bits.ltssm_state,
			ltssm_c);
	}

	return 0;
}

int pcie_ltssm_trace_show(uint32_t port_id)
{
	int ret;
	uint32_t ltssm_num = 0;
	uint64_t ltssm_st_save[TRACER_DEPTH];

	ret = pcie_get_ltssm_trace(port_id, ltssm_st_save, &ltssm_num);
	if (ret)
		return ret;

	return pcie_print_ltssm_trace(ltssm_st_save, ltssm_num);
}

int pcie_ltssm_trace_mode_set(uint32_t port_id, uint32_t mode)
{
	struct hikp_cmd_header req_header;
	struct hikp_cmd_ret *cmd_ret = NULL;
	struct pcie_trace_req_para req_data = { 0 };
	int ret;

	req_data.port_id = port_id;
	req_data.trace_mode = mode;
	hikp_cmd_init(&req_header, PCIE_MOD, PCIE_TRACE, TRACE_MODE);
	cmd_ret = hikp_cmd_alloc(&req_header, &req_data, sizeof(req_data));
	ret = hikp_rsp_normal_check(cmd_ret);
	free(cmd_ret);

	return ret;
}

int pcie_ltssm_trace_clear(uint32_t port_id)
{
	struct hikp_cmd_header req_header;
	struct hikp_cmd_ret *cmd_ret = NULL;
	struct pcie_trace_req_para req_data = { 0 };
	int ret;

	req_data.port_id = port_id;
	hikp_cmd_init(&req_header, PCIE_MOD, PCIE_TRACE, TRACE_CLEAR);
	cmd_ret = hikp_cmd_alloc(&req_header, &req_data, sizeof(req_data));
	ret = hikp_rsp_normal_check(cmd_ret);
	free(cmd_ret);

	return ret;
}

int pcie_ltssm_link_status_get(uint32_t port_id)
{
	union pcie_link_info reg_val;
	struct hikp_cmd_header req_header;
	struct hikp_cmd_ret *cmd_ret = NULL;
	struct pcie_trace_req_para req_data = { 0 };
	char *ltssm_sts = NULL;
	int ret;

	req_data.port_id = port_id;
	hikp_cmd_init(&req_header, PCIE_MOD, PCIE_TRACE, TRACE_INFO);
	cmd_ret = hikp_cmd_alloc(&req_header, &req_data, sizeof(req_data));
	ret = hikp_rsp_normal_check(cmd_ret);
	if (ret)
		goto free_cmd_ret;

	if (cmd_ret->rsp_data_num == 0) {
		Err("rsp data number check failed, rsp_data_num: %u.\n",
			cmd_ret->rsp_data_num);
		ret = -EINVAL;
		goto free_cmd_ret;
	}
	reg_val.u32 = cmd_ret->rsp_data[0];
	ltssm_sts = hisi_pcie_ltssm_string_get(reg_val.bits.mac_ltssm_st);

	Info("Port[%u] mac link information:\n", port_id);
	Info("    ltssm status: %s\n", ltssm_sts);
	Info("    speed: Gen%u\n", reg_val.bits.mac_cur_link_speed);
	Info("    width: X%u\n", reg_val.bits.mac_cur_link_width);
	Info("    link_up: %u\n", reg_val.bits.mac_link_up);
	Info("    lane_reverse: %u\n", reg_val.bits.lane_reverse);
free_cmd_ret:
	free(cmd_ret);

	return ret;
}

static int pcie_get_pm_trace(uint32_t port_id, uint64_t *pm_status, uint32_t *pm_num)
{
	struct hikp_cmd_header req_header;
	struct hikp_cmd_ret *cmd_ret = NULL;
	struct pcie_trace_req_para req_data = { 0 };
	size_t src_size, dst_size;
	int ret;

	req_data.port_id = port_id;
	hikp_cmd_init(&req_header, PCIE_MOD, PCIE_TRACE, TRACE_PM);
	cmd_ret = hikp_cmd_alloc(&req_header, &req_data, sizeof(req_data));
	ret = hikp_rsp_normal_check(cmd_ret);
	if (ret) {
		Err("pcie pm trace cmd_ret check failed, ret: %d.\n", ret);
		goto free_cmd_ret;
	}

	if (cmd_ret->rsp_data_num == 0) {
		Err("without rsp data.\n");
		ret = -EINVAL;
		goto free_cmd_ret;
	}
	/* 0: First uint32_t is pm trace num received from TF */
	*pm_num = cmd_ret->rsp_data[0];
	src_size = (*pm_num) * sizeof(uint64_t);
	dst_size = TRACER_DEPTH * sizeof(uint64_t);
	if (src_size > dst_size) {
		Err("size check failed, %u > %u.\n", src_size, dst_size);
		ret = -EINVAL;
		goto free_cmd_ret;
	}

	if ((cmd_ret->rsp_data_num - 1) * sizeof(uint32_t) != (*pm_num) * sizeof(uint64_t)) {
		Err("rsp data number check failed, rsp_data_num: %u, pm_num: %u.\n",
		    cmd_ret->rsp_data_num, *pm_num);
		ret = -EINVAL;
		goto free_cmd_ret;
	}

	memcpy(pm_status, (cmd_ret->rsp_data + 1), src_size);

free_cmd_ret:
	free(cmd_ret);
	return ret;
}

struct pcie_pm_num_string g_pm_string_table[] = {
	{0x0,	"pm_pme_idle"},
	{0x1,	"pm_wait_dc_pme_msg_send_out"},
	{0x2,	"pm_wait_dc_tl_enter_l2"},
	{0x3,	"pm_wait_dc_dl_enter_l2"},
	{0x4,	"pm_wait_dc_mac_enter_l2"},
	{0x5,	"pm_dc_enter_l2"},
	{0x6,	"pm_wait_dc_tl_enter_pcipm_l1"},
	{0x7,	"pm_wait_dc_dl_enter_pcipm_l1"},
	{0x8,	"pm_wait_dc_tl_enter_aspm_l1"},
	{0x9,	"pm_wait_dc_dl_enter_aspm_l1"},
	{0xa,	"pm_wait_tl_enter_aspm_l0"},
	{0xb,	"pm_wait_dl_enter_aspm_l0"},
	{0xc,	"pm_wait_dc_mac_enter_l1"},
	{0xd,	"pm_wait_mac_enter_l0s"},
	{0xe,	"pm_device_in_l0s"},
	{0xf,	"pm_dc_device_in_l1"},
	{0x10,	"pm_wait_dc_enter_l0"},
	{0x11,	"pm_wait_uc_tl_enter_l2"},
	{0x12,	"pm_wait_uc_dl_enter_l2"},
	{0x13,	"pm_wait_uc_mac_enter_l2"},
	{0x15,	"pm_wait_uc_tl_enter_pcipm_l1"},
	{0x17,	"pm_wait_uc_dl_enter_aspm_l1"},
	{0x18,	"pm_wait_uc_tl_enter_aspm_l1"},
	{0x1a,	"pm_wait_uc_dl_enter_pcipm_l1"},
	{0x1c,	"pm_wait_uc_mac_enter_l1"},
	{0x1d,	"pm_wait_uc_pme_enter_l1_nak_sent_out"},
	{0x1e,	"pm_wait_uc_enter_l0"},
	{0x20,	"pm_device_will_enter_l1_substate"},
	{0x21,	"pm_device_in_l1_1"},
	{0x22,	"pm_device_will_exit_l1_substate"},
	{0x23,	"pm_device_in_l1_2_entry"},
	{0x24,	"pm_device_in_l1_2_idle"},
	{0x25,	"pm_device_in_l1_2_exit"},
	{-1,	"unknown"} /* end of array */
};

static char *hisi_pcie_pm_string_get(uint32_t pm)
{
	int i = 0;

	while (g_pm_string_table[i].pm >= 0) {
		if ((uint32_t)g_pm_string_table[i].pm != pm) {
			i++;
			continue;
		}
		break;
	}

	return g_pm_string_table[i].pm_c;
}

static int pcie_print_pm_trace(const uint64_t *pm_status, uint32_t pm_num)
{
	uint32_t i;
	char *pm_c = NULL;
	union pm_state_reg pm_val;

	if (pm_num > TRACER_DEPTH || pm_num == 0) {
		Err("pm_num(%u) is over range or zero\n", pm_num);
		return -EINVAL;
	}
	Info("pm tracer:\n");
	Info("\ttrace state: %llx\n", pm_status[0]);
	Info("\tpm[ii]: BE8: 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0 "
	     "BD8:   23:6   5:0 :  pm state\n");
	for (i = 1; i < pm_num; i++) {
		pm_val.val = pm_status[i];
		pm_c = hisi_pcie_pm_string_get((uint32_t)pm_val.bits.pm_state);
		Info("\tpm[%02u]:\t     %x  %x  %x  %x  %x  %x %x %x %x %x %x "
		     "%x %x %x %x     0x%06x  0x%02x   %s\n",
			i,
			(uint32_t)pm_val.bits.pm_t_dfe_time_meet,
			(uint32_t)pm_val.bits.dfe_req,
			(uint32_t)pm_val.bits.mac2pm_rx_data_vld,
			(uint32_t)pm_val.bits.pm_wakeup_tol0_en,
			(uint32_t)pm_val.bits.pending_dllp_vld,
			(uint32_t)pm_val.bits.retrain_link_vld,
			(uint32_t)pm_val.bits.aspm_nak_vld,
			(uint32_t)pm_val.bits.pm_blk_tlp_timeout,
			(uint32_t)pm_val.bits.pme_turn_off_vld_hold,
			(uint32_t)pm_val.bits.pm_t_dl_lastack_timeout,
			(uint32_t)pm_val.bits.pm_t_dl_l0s_gnt_timeout,
			(uint32_t)pm_val.bits.pm_t_dl_l1_gnt_timeout,
			(uint32_t)pm_val.bits.pm_t_dl_l2_gnt_timeout,
			(uint32_t)pm_val.bits.enter_l12_case,
			(uint32_t)pm_val.bits.refclk_stable_vld,
			(uint32_t)pm_val.bits.pm_clock,
			(uint32_t)pm_val.bits.pm_state,
			pm_c);
	}

	return 0;
}

int pcie_pm_trace(uint32_t port_id)
{
	int ret;
	uint32_t pm_num = 0;
	uint64_t pm_st_save[TRACER_DEPTH];

	ret = pcie_get_pm_trace(port_id, pm_st_save, &pm_num);
	if (ret)
		return ret;

	return pcie_print_pm_trace(pm_st_save, pm_num);
}