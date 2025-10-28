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

#ifndef UBUS_TRACE_SHOW_H
#define UBUS_TRACE_SHOW_H

#include <stdint.h>

struct ubus_trace_show_req_para {
	uint32_t port_id;
};

/* ubus trace data struct */
struct ubus_trace_data {
	/* 待芯片给出具体域段信息，当前暂时用trace的两个data reg */
	uint32_t data_l;
	uint32_t data_2;
};

int ubus_trace_show_execute(uint32_t port_id);

#endif /* UBUS_TRACE_SHOW_H */
