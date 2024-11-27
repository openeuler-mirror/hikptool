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

#ifndef HIKP_NIC_LOG_H
#define HIKP_NIC_LOG_H

#include "hikp_net_lib.h"

#define MAX_LOG_NAME_LEN 128
#define MAX_LOG_DATA_NUM 59

enum nic_log_cmd_type {
	FW_LOG_DUMP = 0,
};

struct nic_log_req_para {
	struct bdf_t bdf;
	uint32_t block_id;
};

struct nic_log_rsp_data {
	uint16_t total_blk_num;
	uint16_t cur_blk_size;
	uint32_t log_data[MAX_LOG_DATA_NUM];
};

struct log_param {
	struct tool_target target;
};

#endif /* HIKP_NIC_LOG_H */
