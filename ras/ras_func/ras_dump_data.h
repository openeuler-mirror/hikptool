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

#ifndef RAS_DUMP_REG_H
#define RAS_DUMP_REG_H

#include "ras_tools_include.h"
#include "hikpt_rciep.h"
#include "tool_lib.h"

#define RAS_FILE_HEAD_BUF_LEN 256
#define MAX_DFX_PACKET_LEN 256
#define RAS_REQ_DATA_LEN 4
#define DFX_REG_DUMP_HEADER_LEN 6
#define DFX_REG_PACKET_HEAD_LEN 3

struct dfx_reg_dump_header {
	uint32_t head_magic;  // 文件头的magic数字，特定值表示有效记录。
	uint32_t version;     // 存储格式版本
	uint32_t cap_bits;    // bit0表示是否开启crc，其余bit保留。
	uint32_t pkt_num;     // packet数量
	uint32_t pkt_length;  // 单个packet占用内存空间，单位bytes
	uint32_t reserved;
};

struct file_seq {
	char *buffer;
	uint32_t buffer_size;
	int len;
	char file_name[MAX_LOG_NAME_LEN];
};

struct ras_rsp {
	uint32_t rsp_data[HIKP_RSP_ALL_DATA_MAX];
	uint32_t first_pkt_begin;
	uint32_t last_pkt_end;
	uint32_t rsp_data_num;
	uint32_t packet_buffer[MAX_DFX_PACKET_LEN];
	uint32_t packet_buffer_len;
};

struct ras_dump_req_para {
	uint32_t cmd_id;
};

enum reg_dump_header_index {
	HEAD_MAGIC,
	VERSION,
	CAP_BITS,
	PKT_NUM,
	PKT_LENGTH
};

enum dfx_packet_index {
	DFX_HEAD_INFO_DW0,
	DFX_HEAD_INFO_DW1,
	DFX_COMMON_MAIN_TEXT_BEGIN = 3
};

#define DFX_HEAD_VERSION_OFF 0
#define DFX_HEAD_SOC_ID_OFF 8
#define DFX_HEAD_SKT_ID_OFF 16
#define DFX_HEAD_DIE_ID_OFF 24
#define DFX_HEAD_MODULE_ID_OFF 0
#define DFX_HEAD_SUBMODULE_ID_OFF 8
#define DFX_HEAD_SEQUENCE_NUM_OFF 16
#define DFX_HEAD_REG_COUNT_OFF 24

#define DFX_DATA_IS_CLEARED 0

#define DFX_FILE_SINGLE_PACKET_HEAD_SIZE 256
#define DFX_FILE_SINGLE_REG_SIZE 10


int ras_data_dump(struct tool_ras_cmd *cmd);
int ras_data_clear(struct tool_ras_cmd *cmd);

#endif /* RAS_DUMP_REG_H */
