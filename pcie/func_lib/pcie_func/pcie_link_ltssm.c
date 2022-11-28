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
		Err("PCIe Base", "pcie trace cmd_ret check failed, ret: %d.\n", ret);
		goto free_cmd_ret;
	}

	if (cmd_ret->rsp_data_num == 0) {
		Err("PCIe Base", "without rsp data.\n");
		ret = -EINVAL;
		goto free_cmd_ret;
	}
	/* 0: First uint32_t is ltssm trace num received from TF */
	*ltssm_num = cmd_ret->rsp_data[0];

	if ((cmd_ret->rsp_data_num - 1) * sizeof(uint32_t) != (*ltssm_num) * sizeof(uint64_t)) {
		Err("PCIe Base", "rsp data number check failed, rsp_data_num: %u, ltssm_num: %u.\n",
		    cmd_ret->rsp_data_num, *ltssm_num);
		ret = -EINVAL;
		goto free_cmd_ret;
	}

	src_size = (*ltssm_num) * sizeof(uint64_t);
	dst_size = TRACER_DEPTH * sizeof(uint64_t);
	if (src_size > dst_size) {
		Err("PCIe Base", "size check failed, %u > %u.\n", src_size, dst_size);
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
		Err("PCIe Base", "ltssm_num(%u) is over range or zero\n", ltssm_num);
		return -EINVAL;
	}
	Info("PCIe Base", "ltssm tracer:\n");
	Info("PCIe Base", "\ttrace mode: %llx\n", *ltssm_input);
	Info("PCIe Base",
	     "\tltssm[ii]:  63:48 47:32 31 30 29 28 27 26 25 24 23 22 21:"
	     "20 19:12 11:10 9:6 5:0  ltssm\n");
	for (i = 1; i < ltssm_num; i++) {
		ltssm_val.val = ltssm_input[i];
		ltssm_c = hisi_pcie_ltssm_string_get((uint32_t)ltssm_val.bits.ltssm_state);
		Info("PCIe Base",
			"\tltssm[%02u]: 0x%04x %04x   %x  %x  %x  %x  %x  %x  %x  "
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
			(uint32_t)ltssm_val.bits.mac_rate,
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
	struct hikp_cmd_ret *cmd_ret;
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
	struct hikp_cmd_ret *cmd_ret;
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
	uint32_t i;
	union pcie_link_info reg_val;
	struct hikp_cmd_header req_header;
	struct hikp_cmd_ret *cmd_ret;
	struct pcie_trace_req_para req_data = { 0 };
	char *ltssm_sts;
	int ret;

	req_data.port_id = port_id;
	hikp_cmd_init(&req_header, PCIE_MOD, PCIE_TRACE, TRACE_INFO);
	cmd_ret = hikp_cmd_alloc(&req_header, &req_data, sizeof(req_data));
	ret = hikp_rsp_normal_check(cmd_ret);
	if (ret)
		goto free_cmd_ret;

	if (cmd_ret->rsp_data_num == 0) {
		Err("PCIe Base", "rsp data number check failed, rsp_data_num: %u.\n",
			cmd_ret->rsp_data_num);
		ret = -EINVAL;
		goto free_cmd_ret;
	}
	reg_val.u32 = cmd_ret->rsp_data[0];
	ltssm_sts = hisi_pcie_ltssm_string_get(reg_val.bits.mac_ltssm_st);

	Info(LOG_PCIE, "Port[%u] mac link information:\n", port_id);
	Info(LOG_PCIE, "    ltssm status: %s\n", ltssm_sts);
	Info(LOG_PCIE, "    speed: Gen%u\n", reg_val.bits.mac_cur_link_speed);
	Info(LOG_PCIE, "    width: X%u\n", reg_val.bits.mac_cur_link_width);
	Info(LOG_PCIE, "    link_up: %u\n", reg_val.bits.mac_link_up);
	Info(LOG_PCIE, "    lane_reverse: %u\n", reg_val.bits.lane_reverse);
free_cmd_ret:
	free(cmd_ret);

	return ret;
}
