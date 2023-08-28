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

#ifndef HIKP_NIC_PORT_FAULT_H
#define HIKP_NIC_PORT_FAULT_H

#include "hikp_net_lib.h"

enum nic_port_fault_sub_cmd_type {
	NIC_PORT_FAULT_INFO_DUMP,
};

enum nic_port_fault_info_type {
	NIC_PORT_FAULT_OK,
	NIC_PORT_FAULT_ERR,
	NIC_PORT_FAULT_NOTSUP,
	NIC_PORT_FAULT_INVALID
};

struct nic_port_fault_req_para {
	struct bdf_t bdf;
	uint8_t block_id;
};

struct nic_port_fault_rsp_head {
	uint8_t total_blk_num;
	uint8_t curr_blk_size;
	uint16_t rsv;
};

#define NIC_PORT_FAULT_MAX_RSP_DATA    1
struct nic_port_fault_rsp {
	struct nic_port_fault_rsp_head head;
	uint32_t data[NIC_PORT_FAULT_MAX_RSP_DATA];
};

struct nic_port_fault_status {
	uint8_t cdr_flash_status;
	uint8_t fault_9545_status;
	uint8_t cdr_core_status;
	uint8_t hilink_ref_status;
};

#endif /* HIKP_NIC_PORT_FAULT_H */
