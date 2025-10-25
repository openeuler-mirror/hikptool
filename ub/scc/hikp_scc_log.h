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

#ifndef HIKP_SCC_LOG_H
#define HIKP_SCC_LOG_H
#include "tool_cmd.h"
#include "hikp_scc_cmd.h"

#define LOG_AREA_TOTAL_SIZE 0x200000
#define SCC_MCU_CORE0 0
#define SCC_MCU_CORE1 1
#define MAX_CORE_NUM 2

enum scc_log_sub_cmd {
	SCC_LOG_DUMP = 1,
};

struct scc_log_req_para {
	uint8_t chip;
	uint8_t die;
	uint8_t rsv[2];
	uint32_t block_id;
};

#define LOG_DATA_BLK_SIZE	236    /* A maximum of 240 bytes can be transmitted at a time. */
struct scc_log_rsp_data {
	uint16_t total_blk_num;
	uint16_t cur_blk_size;
	uint8_t log_data[LOG_DATA_BLK_SIZE];
};

/* SCC log information: 32B; size must be a multiple of 8B */
struct scc_information {
	uint32_t scc_firmware_version;
	uint32_t magic_num;
	uint16_t header_size;
	uint16_t log_size;
	uint32_t log_area_size;
	uint32_t head;
	uint32_t tail;
	uint32_t log_cnt;
	uint32_t log_area_info_start;
};

typedef enum {
	TYPE_DEFAULT = 0,
	TYPE_STR,
	TYPE_OTHER = 0x80,
	TYPE_ERR,
	TYPE_RETRANS,
} scc_log_type;

struct scc_log_detail {
	uint32_t tpn;
	uint32_t timestamp;
	uint16_t type;
	uint16_t len;
	uint8_t value[];
};

struct scc_log_blk_ctrl {
	uint32_t blk_id;
	uint32_t resp_blk_size;
	uint32_t total_blk_num;
};

void hikp_scc_dump_log(struct major_cmd_ctrl *self, struct scc_cmd_cfg *cmd_cfg);

#endif /* HIKP_SCC_LOG_H */
