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

#ifndef __HIKP_ROCE_MDB_H__
#define __HIKP_ROCE_MDB_H__

#include "hikp_net_lib.h"

#define ROCE_HIKP_MDB_REG_NUM 22
#define ROCE_HIKP_REG_SWICTH 2

struct cmd_roce_mdb_param {
	struct tool_target target;
	int reset_flag;
};

struct roce_mdb_req_para {
	struct bdf_t bdf;
};

struct roce_mdb_rsp_data {
	uint32_t reg_offset[ROCE_HIKP_MDB_REG_NUM];
	uint32_t reg_data[ROCE_HIKP_MDB_REG_NUM];
};

enum roce_mdb_cmd_type {
	MDB_SHOW = 0x0,
	MDB_CLEAR,
};

#endif /* __HIKP_ROCE_MDB_H__ */
