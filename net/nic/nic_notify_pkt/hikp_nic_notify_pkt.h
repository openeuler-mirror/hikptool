/*
 * Copyright (c) 2023 Hisilicon Technologies Co., Ltd.
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

#ifndef HIKP_NIC_NOTIFY_PKT_H
#define HIKP_NIC_NOTIFY_PKT_H

#include "hikp_net_lib.h"

enum nic_notify_pkt_sub_cmd_type {
	NIC_NOTIFY_PKT_DUMP = 0,
};

struct nic_notify_pkt_req_para {
	struct bdf_t bdf;
	uint32_t block_id;
};

struct nic_notify_pkt_rsp_head {
	uint8_t total_blk_num;
	uint8_t curr_blk_size; /* real data size, not contain head size. */
	uint16_t rsv;
};

#define NIC_NOTIFY_PKT_MAX_RSP_DATA    18
struct nic_notify_pkt_rsp {
	struct nic_notify_pkt_rsp_head head;
	uint32_t data[NIC_NOTIFY_PKT_MAX_RSP_DATA];
};

#define NIC_NOTIFY_PKT_DATA_LEN 64
struct nic_notify_pkt_info {
	uint32_t cfg;
	uint32_t ipg;
	uint8_t data[NIC_NOTIFY_PKT_DATA_LEN];
};

#define HIKP_NOTIFY_PKT_CFG_PKT_EN		0
#define HIKP_NOTIFY_PKT_CFG_START_EN		1
#define HIKP_NOTIFY_PKT_CFG_PKT_NUM_M		GENMASK(5, 2)
#define HIKP_NOTIFY_PKT_CFG_PKT_NUM_S		2

int hikp_nic_notify_pkt_get_target(struct major_cmd_ctrl *self, const char *argv);
void hikp_nic_notify_pkt_cmd_execute(struct major_cmd_ctrl *self);
#endif /* HIKP_NIC_NOTIFY_PKT_H */
