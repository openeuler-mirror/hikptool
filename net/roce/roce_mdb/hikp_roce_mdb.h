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

#ifndef HIKP_ROCE_MDB_H
#define HIKP_ROCE_MDB_H

#include "hikp_roce_ext_common.h"

#define ROCE_HIKP_MDB_REG_NUM 22
#define ROCE_HIKP_REG_SWICTH 2

#define ROCE_MDB_CMD_CLEAR (1 << 0)
#define ROCE_MDB_CMD_EXT (1 << 1)

struct cmd_roce_mdb_param {
	struct tool_target target;
	uint32_t sub_cmd;
	uint8_t flag;
};

struct roce_mdb_req_para {
	struct bdf_t bdf;
};

struct roce_mdb_req_param_ext {
	struct roce_mdb_req_para origin_param;
	uint32_t block_id;
};

struct roce_mdb_rsp_data {
	uint32_t reg_offset[ROCE_HIKP_MDB_REG_NUM];
	uint32_t reg_data[ROCE_HIKP_MDB_REG_NUM];
};

enum roce_mdb_cmd_type {
	MDB_SHOW = 0x0,
	MDB_CLEAR,
	MDB_EXT,
	MDB_CLEAR_EXT,
};

#endif /* HIKP_ROCE_MDB_H */
