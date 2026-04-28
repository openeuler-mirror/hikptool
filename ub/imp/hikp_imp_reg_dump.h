/*
 * Copyright (c) 2026 Hisilicon Technologies Co., Ltd.
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

#ifndef HIKP_IMP_REG_DUMP_H
#define HIKP_IMP_REG_DUMP_H
#include "tool_cmd.h"
#include "hikp_imp_cmd.h"

enum imp_dump_sub_cmd {
	DUMP_DFX_REG = 1,
};

enum reg_width_type {
	WIDTH_32_BIT,
	WIDTH_64_BIT,
};

/* A maximum of 128 bytes is supported. */
struct imp_dump_req_para {
	uint8_t chip;
	uint8_t die;
	uint8_t rsv[2];
	uint32_t block_id;
	uint32_t dump_rsvd[6];
};

#define REG_DATA_BLK_SIZE	232    /* A maximum of 232 bytes can be transmitted at a time. */
struct imp_dump_reg_rsp_data {
	uint8_t hw_version;
	uint8_t ret_code;
	uint8_t reg_width : 1;
	uint8_t reg_rsv   : 7;
	uint8_t rsvd;
	uint16_t total_blk_num;
	uint16_t cur_blk_size;
	uint8_t reg_data[REG_DATA_BLK_SIZE];
};

void hikp_imp_dump_dfx_reg(struct major_cmd_ctrl *self, struct imp_cmd_cfg *cmd_cfg);

#endif /* HIKP_IMP_REG_DUMP_H */
