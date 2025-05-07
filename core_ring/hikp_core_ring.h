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

#ifndef HIKP_CORE_RING_H
#define HIKP_CORE_RING_H
#include <stdint.h>
#include "tool_lib.h"

#define PARAM_DUMP_MASK		HI_BIT(0)

enum core_ring_cmd_type {
	CORE_RING_DUMP = 1,
};

enum core_ring_sub_cmd_type {
	RING_INFO_DUMP = 1,
};

#define RING_DATA_MAX		29 /* A maximum of 240 bytes can be transmitted at a time. */
struct core_ring_info {
	uint8_t chip_num;
	uint8_t per_cluster_num;
	uint8_t rsv0[2];
	uint32_t rsv1;
	uint64_t ring_data[RING_DATA_MAX];
};

struct core_ring_req {
	uint32_t cmd_flag; /* Reserved in the current version */
};

#endif /* HIKP_CORE_RING_H */
