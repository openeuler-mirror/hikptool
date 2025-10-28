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

#ifndef UBUS_DFX_DUMP_H
#define UBUS_DFX_DUMP_H

#include <stdint.h>

#define UBUS_REG_NAME_LEN 60

struct ubus_dump_req_para {
	uint32_t port_id;
};

struct ubus_dumpreg_info {
	uint32_t val;
	char name[UBUS_REG_NAME_LEN];
};

struct ubus_dumpreg_table {
	uint32_t size;
	struct ubus_dumpreg_info *dump_info;
};

int ubus_dfx_dump_show_execute(uint32_t port_id);
#endif /* UBUS_DFX_DUMP_H */
