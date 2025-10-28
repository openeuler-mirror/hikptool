/*
 * Copyright (c) 2025 Hisilicon Technologies Co., Ltd.
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

#ifndef UBUS_MSGQ_DUMP_H
#define UBUS_MSGQ_DUMP_H

#include <stdint.h>

struct ubus_msgq_dump_req_para {
	uint32_t controller_idx;
	uint32_t msgq_idx;
	uint32_t queue_type;
	uint32_t entry_idx;
};

struct ubus_msgq_reg {
	/* SQ information */
	uint32_t sq_base_addr_l;
	uint32_t sq_base_addr_h;
	uint32_t sq_pi;
	uint32_t sq_ci;
	uint32_t sq_dep;
	uint32_t sq_status;
	uint32_t sq_int_mask;
	uint32_t sq_int_status;
	uint32_t sq_int_ro;
	/* RQ information */
	uint32_t rq_base_addr_l;
	uint32_t rq_base_addr_h;
	uint32_t rq_pi;
	uint32_t rq_ci;
	uint32_t rq_dep;
	uint32_t rq_entry_blk_sz;
	uint32_t rq_status;
	/* CQ information */
	uint32_t cq_base_addr_l;
	uint32_t cq_base_addr_h;
	uint32_t cq_pi;
	uint32_t cq_ci;
	uint32_t cq_dep;
	uint32_t cq_status;
	uint32_t cq_int_mask;
	uint32_t cq_int_status;
	uint32_t cq_int_ro;
};

struct ubus_sq_entry {
	uint32_t msg_code : 4;
	uint32_t sub_msg_code : 4;
	uint32_t msg_id : 8;
	uint32_t p_len : 16;
	uint32_t rsvd_0 : 8;
	uint32_t vl : 4;
	uint32_t local : 1;
	uint32_t icrc : 1;
	uint32_t rsvd_1 : 18;
	uint32_t p_addr;
};

struct ubus_cq_entry {
	uint32_t msg_code : 4;
	uint32_t sub_msg_code : 4;
	uint32_t msg_id : 8;
	uint32_t p_len : 16;
	uint32_t status : 1;
	uint32_t rsvd_0 : 7;
	uint32_t rq_pi : 10;
	uint32_t rsvd_1 : 14;
};

enum ubus_msgq_type {
	UBUS_MSGQ_SQ = 0,
	UBUS_MSGQ_RQ = 1,
	UBUS_MSGQ_CQ = 2,
	UBUS_MSGQ_UNKNOWN
};

int ubus_msgq_dump_reg_execute(uint32_t controller_idx, uint32_t msgq_idx);
int ubus_msgq_dump_entry_execute(uint32_t controller_idx, uint32_t msgq_idx,
								 uint32_t queue_type, uint32_t entry_idx);

#endif /* UBUS_MSGQ_DUMP_H */