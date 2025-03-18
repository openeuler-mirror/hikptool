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

#ifndef HIKP_NIC_QOS_H
#define HIKP_NIC_QOS_H

#include "hikp_net_lib.h"

enum nic_qos_sub_cmd_type {
	NIC_PACKET_BUFFER_DUMP = 0,
	NIC_DCB_DUMP,
	NIC_PAUSE_DUMP,
	NIC_PFC_STORM_PARA_DUMP,
};

struct nic_buf_waterline {
	uint32_t low;
	uint32_t high;
};

struct nic_shared_buf {
	struct nic_buf_waterline tc_thrd[HIKP_NIC_MAX_TC_NUM];
	struct nic_buf_waterline comm_wl;
	uint32_t buf_size;  /* share packet buffer size. */
};

struct nic_priv_buf {
	struct nic_buf_waterline wl; /* high and low waterline for the TC Rx private buffer. */
	uint32_t rx_buf_size;        /* TC Rx private buffer size. */
	uint32_t tx_buf_size;        /* TC Tx private buffer size. */
};

struct nic_pkt_buf_info {
	struct nic_priv_buf priv_buf[HIKP_NIC_MAX_TC_NUM];
	struct nic_shared_buf share_buf;
};

struct nic_pfc_info {
	uint8_t hw_tc_map; /* enabled TC map in hardware */
	uint8_t pfc_en;  /* enable pfc bitmap for UP */
	uint8_t rsv[2];
};

struct nic_pfc_storm_para {
	uint32_t dir;
	uint32_t enable;
	uint32_t period_ms;
	uint32_t times;
	uint32_t recovery_period_ms;
	uint32_t storm_count;
};

struct nic_ets_info {
	uint8_t prio_tc[HIKP_NIC_MAX_USER_PRIO_NUM];
	uint8_t tc_bw[HIKP_NIC_MAX_TC_NUM];
	uint8_t sch_mode[HIKP_NIC_MAX_TC_NUM]; /* 0: sp 1: dwrr */
	uint8_t max_tc; /* supported max TC number */
	uint8_t rsv[3];
};

struct nic_dcb_info {
	struct nic_pfc_info pfc;
	struct nic_ets_info ets;
};

enum hikp_pause_type {
	HIKP_NONE_PAUSE = 0x0,
	HIKP_MAC_PAUSE,
	HIKP_PFC,
};

struct nic_pause_info {
	uint8_t type;
	uint8_t pause_rx; /* 1: enable 0: disable */
	uint8_t pause_tx; /* 1: enable 0: disable */
	uint8_t pause_gap;
	uint16_t pause_time;
	uint16_t rsv;
};

struct qos_cmd_info {
	uint32_t length;
	union nic_qos_feature_info {
		struct nic_pkt_buf_info pkt_buf;
		struct nic_dcb_info dcb;
		struct nic_pause_info pause;
		struct nic_pfc_storm_para pfc_storm_para;
	} info;
};

struct nic_qos_rsp_head {
	uint8_t total_blk_num;
	uint8_t cur_blk_size;  /* real data size, not contain head size. */
	uint16_t rsv;
};

#define NIC_QOS_MAX_RSP_DATA  59
struct nic_qos_rsp {
	struct nic_qos_rsp_head rsp_head; /* 4 Byte */
	uint32_t rsp_data[NIC_QOS_MAX_RSP_DATA];
};

struct nic_qos_req_para {
	struct bdf_t bdf;
	uint8_t block_id;
	uint8_t dir;
	uint8_t rsv[2];
};

enum nic_pfc_dir {
	NIC_RX_QOS = 0,
	NIC_TX_QOS,
	NIC_QOS_DIR_NONE,
};

struct nic_qos_param {
	struct tool_target target;
	int feature_idx;
	enum nic_pfc_dir dir;
	char revision_id[MAX_PCI_ID_LEN + 1];
};

#define HIKP_QOS_MAX_FEATURE_NAME_LEN 20
struct qos_feature_cmd {
	const char feature_name[HIKP_QOS_MAX_FEATURE_NAME_LEN];
	uint32_t sub_cmd_code;
	void (*show)(const void *data);
};

int hikp_nic_cmd_get_qos_target(struct major_cmd_ctrl *self, const char *argv);
void hikp_nic_qos_cmd_execute(struct major_cmd_ctrl *self);
void hikp_nic_qos_set_cmd_feature_idx(int feature_idx);
void hikp_nic_qos_set_cmd_direction(enum nic_pfc_dir dir);
#endif /* HIKP_NIC_QOS_H */
