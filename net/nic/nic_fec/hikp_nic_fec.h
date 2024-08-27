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

#ifndef HIKP_NIC_FEC_H
#define HIKP_NIC_FEC_H

#include "hikp_net_lib.h"

enum nic_fec_sub_cmd_type {
	NIC_FEC_ERR_INFO_DUMP,
};

enum nic_fec_mode_type {
	NIC_FEC_MODE_NOFEC = 0,
	NIC_FEC_MODE_BASEFEC,
	NIC_FEC_MODE_RSFEC,
	NIC_FEC_MODE_LLRSFEC,
	NIC_FEC_MODE_BUTT
};

struct nic_fec_req_para {
	struct bdf_t bdf;
	uint8_t block_id;
	uint8_t rsv[3];
};

struct nic_fec_rsp_head {
	uint8_t total_blk_num;
	uint8_t curr_blk_size; /* real data size, not contain head size. */
	uint16_t rsv;
};

#define NIC_FEC_MAX_RSP_DATA    59
struct nic_fec_rsp {
	struct nic_fec_rsp_head head;
	uint32_t data[NIC_FEC_MAX_RSP_DATA];
};

#define NIC_FEC_MAX_LANES    8
struct nic_basefec_stats {
	uint32_t corr_block_cnt;
	uint32_t uncorr_block_cnt;
	uint32_t lane_num;
	uint32_t lane_corr_block_cnt[NIC_FEC_MAX_LANES];
	uint32_t lane_uncorr_block_cnt[NIC_FEC_MAX_LANES];
};

struct nic_rsfec_stats {
	uint32_t corr_cw_cnt;
	uint32_t uncorr_cw_cnt;
	uint32_t err_cw_cnt;
};

struct nic_fec_err_info {
	uint32_t fec_mode;
	union {
		struct nic_basefec_stats basefec;
		struct nic_rsfec_stats rsfec;
	};
};

int hikp_nic_fec_get_target(struct major_cmd_ctrl *self, const char *argv);
void hikp_nic_fec_cmd_execute(struct major_cmd_ctrl *self);
#endif /* HIKP_NIC_FEC_H */
