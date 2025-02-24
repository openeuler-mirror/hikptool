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

#ifndef SDMA_COMMON_H
#define SDMA_COMMON_H

#define RESP_MAX_NUM 160

/* SDMA command code */
enum sdma_cmd_type {
	SDMA_DUMP = 0,
};

enum sdma_dump_cmd_type {
	DUMP_UNKNOWN = 0,
	DUMP_CHN_STATUS,
	DUMP_CHN_PC,
	DUMP_CHN_VC,
};

#endif /* SDMA_COMMON_H */
