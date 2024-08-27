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

#ifndef HIKP_NIC_GRO_H
#define HIKP_NIC_GRO_H

#include "hikp_net_lib.h"

enum nic_gro_sub_cmd_type {
	NIC_GRO_INFO_DUMP,
};

struct nic_gro_req_para {
	struct bdf_t bdf;
	uint8_t block_id;
	uint8_t rsv[3];
};

struct nic_gro_rsp_head {
	uint8_t total_blk_num;
	uint8_t curr_blk_size; /* real data size, not contain head size. */
	uint16_t rsv;
};

#define NIC_GRO_MAX_RSP_DATA    59
struct nic_gro_rsp {
	struct nic_gro_rsp_head head;
	uint32_t data[NIC_GRO_MAX_RSP_DATA];
};

struct nic_gro_info {
	uint32_t gro_en;
	uint32_t max_coal_bd_num;
};

int hikp_nic_gro_get_target(struct major_cmd_ctrl *self, const char *argv);
void hikp_nic_gro_cmd_execute(struct major_cmd_ctrl *self);
#endif /* HIKP_NIC_GRO_H */
