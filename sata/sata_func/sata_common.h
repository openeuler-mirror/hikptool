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

#ifndef __SATA_COMMON_H_
#define __SATA_COMMON_H_

#define SATA_MAX_PORT_NUM 1
#define RESP_MAX_NUM 60

/* SATA command code */
enum sata_cmd_type {
	SATA_DUMP = 0,
};

enum sata_dump_cmd_type {
	DUMP_UNKNOWN = 0,
	DUMP_GLOBAL,
	DUMP_PORTX,
};

#endif
