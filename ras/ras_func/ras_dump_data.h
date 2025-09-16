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

#ifndef RAS_DUMP_DATA_H
#define RAS_DUMP_DATA_H

#include <stdint.h>

struct rasdfx_file_header {
	uint32_t head_magic;  // 文件头的magic数字，特定值表示有效记录。
	uint32_t version;     // 存储格式版本
	uint32_t cap_bits;    // bit0表示是否开启crc，其余bit保留。
	uint32_t pkt_num;     // packet数量
	uint32_t pkt_size_dwords;  // 单个packet内DWord个数，单位4bytes
	uint32_t reserved;
};

struct rasdfx_pkt_header_dw0 {
	uint32_t version : 8;
	uint32_t soc_id : 8;
	uint32_t skt_id : 8;
	uint32_t die_id : 8;
};

struct rasdfx_pkt_header_dw1 {
	uint32_t module_id : 8;
	uint32_t submodule_id : 8;
	uint32_t sequence_num : 8;
	uint32_t reg_count : 8;
};

struct rasdfx_pkt {
	struct rasdfx_pkt_header_dw0 dw0;
	struct rasdfx_pkt_header_dw1 dw1;
	uint32_t reserved;
	uint32_t reg_base[0];
};

struct file_seq {
	int fd;
	char *buf;
	size_t buf_size;
	size_t buf_offs;
};

enum ras_dump_cmd_type {
	DUMP_DFX,
	DUMP_CLEAR
 };

struct ras_dump_cmd {
	enum ras_dump_cmd_type cmd_type;
	uint32_t cmd_id; /* 0: get header info, 1-n: get packet data */
};

int ras_data_dump(void);
int ras_data_clear(void);

#endif /* RAS_DUMP_DATA_H */

