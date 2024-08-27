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

#ifndef HIKP_NIC_QUEUE_H
#define HIKP_NIC_QUEUE_H

#include "hikp_net_lib.h"

enum nic_queue_sub_cmd_type {
	QUEUE_BASIC_INFO = 0,
	QUEUE_EN_INFO,
	QUEUE_INTR_MAP,
	QUEUE_FUNC_MAP,
};

enum nic_queue_dir {
	NIC_TX_QUEUE = 0,
	NIC_RX_QUEUE,
	NIC_QUEUE_DIR_UNKNOWN,
};

#define HIKP_NIC_MAX_QUEUE_NUM 2048

struct rx_queue_info {
	uint32_t rx_nb_desc;
	uint32_t rx_buf_len;
	uint32_t rx_ring_tcid;
	uint32_t rx_merge_en;
	uint32_t rx_tail;
	uint32_t rx_head;
	uint32_t rx_fbd_num;
	uint32_t rx_ring_offset;
	uint32_t rx_fbd_offset;
	uint32_t rx_bd_err;
	uint32_t rx_stash_lpid;
};

struct tx_queue_info {
	uint32_t tx_nb_desc;
	uint32_t tx_ring_pri; /* 0-low prioritym, 1-high priority */
	uint32_t tx_ring_tc;
	uint32_t tx_merge_en;
	uint32_t tx_tail;
	uint32_t tx_head;
	uint32_t tx_fbd_num;
	uint32_t tx_ring_offset;
	uint32_t tx_ebd_num;
	uint32_t tx_ring_ebd_offset;
	uint32_t tx_ring_bd_err;
	uint32_t tx_ring_fbd_prf_num;
};

union nic_queue_info {
	struct rx_queue_info rxq;
	struct tx_queue_info txq;
};

struct nic_queue_en_cfg {
	uint8_t rcb_en; /* common switch, 0-disable, 1-enable */
	uint8_t rcb_rx_en; /* 0-disable, 1-enable */
	uint8_t rcb_tx_en; /* 0-disable, 1-enable */
	uint8_t rsv;
};

struct nic_queue_en_cfg_info {
	uint16_t tqp_num; /* max queue number in the function */
	uint16_t rsv;
	struct nic_queue_en_cfg q_en_cfg[HIKP_NIC_MAX_QUEUE_NUM];
};

struct rcb_intr_ctl_cfg {
	uint8_t intr_dis; /* 1-disable, 0-enable */
	uint8_t int_gl_idx;
	uint16_t intr_vector_id;
};

struct queue_intr_cfg {
	uint8_t tqp_intr_en; /* intr enable in func common. */
	uint8_t rsv[3];
	struct rcb_intr_ctl_cfg rcb_intr;
};

/* Rx/Tx queue intr map */
struct nic_queue_intr_map {
	uint16_t tqp_num;
	uint16_t rsv;
	struct queue_intr_cfg intr_cfg[HIKP_NIC_MAX_QUEUE_NUM];
};

struct tqp_func_map {
	uint16_t q_id; /* relative queue id in the func */
	uint16_t abs_q_id; /* absolute queue id relative to PF */
};

struct nic_queue_func_map {
	uint16_t tqp_num;
	uint16_t rsv;
	struct tqp_func_map map[HIKP_NIC_MAX_QUEUE_NUM];
};

union nic_queue_feature_info {
	union nic_queue_info q_info; /* one Rx/Tx queue info */
	struct nic_queue_en_cfg_info q_en_info;
	struct nic_queue_intr_map q_intr_map;
	struct nic_queue_func_map q_func_map;
};

struct nic_queue_rsp_head {
	uint8_t total_blk_num;
	uint8_t cur_blk_size;  /* real data size, not contain head size. */
	uint16_t rsv;
};

#define NIC_QUEUE_MAX_RSP_DATA  59
struct nic_queue_rsp {
	struct nic_queue_rsp_head rsp_head; /* 4 Byte */
	uint32_t rsp_data[NIC_QUEUE_MAX_RSP_DATA];
};

struct nic_queue_req_para {
	struct bdf_t bdf;
	uint8_t block_id;
	uint8_t is_rx; /* 1: Rx queue, 0: Tx queue */
	uint16_t q_id;
};

struct nic_queue_param {
	struct tool_target target;
	int feature_idx;
	enum nic_queue_dir dir;
	int qid;

	/* Control if display all queues including disabled or unused, defaulty
	 * only display enabled and used queues.
	 */
	bool is_display_all;
};

#define HIKP_QUEUE_FEATURE_MAX_NAME_LEN 20
struct queue_feature_cmd {
	const char feature_name[HIKP_QUEUE_FEATURE_MAX_NAME_LEN];
	uint32_t sub_cmd_code;
	void (*show)(const void *data);
};

int hikp_nic_cmd_get_queue_target(struct major_cmd_ctrl *self, const char *argv);
void hikp_nic_queue_cmd_execute(struct major_cmd_ctrl *self);
void hikp_nic_queue_cmd_set_param(int feature_idx, int qid, enum nic_queue_dir dir);
#endif /* HIKP_NIC_QUEUE_H */
