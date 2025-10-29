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

#ifndef UMMU_COMMON_H
#define UMMU_COMMON_H

enum ummu_cmd_type {
	UMMU_CMD_HELP = 0,
	UMMU_CMD_DUMP = 1,
	UMMU_CMD_SYNC_TIMEOUT = 2
};

enum ummu_dump_cmd_type {
	UMMU_HELP = 0,
	UMMU_KCMD_DUMP = 1,
	UMMU_UMCMD_DUMP = 2,
	UMMU_UBIF_DUMP = 3,
	UMMU_TBU_DUMP = 4,
	UMMU_TCU_DUMP = 5,
	UMMU_SKY_DUMP = 6,
	UMMU_CNT_DUMP = 7
};

#endif /* UMMU_COMMON_H */
