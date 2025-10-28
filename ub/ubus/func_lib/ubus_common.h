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

#ifndef UBUS_COMMON_H
#define UBUS_COMMON_H

enum ubus_cmd_type {
	UBUS_MSGQ_HELP = 0,
	UBUS_MSGQ_DUMP = 1,
	UBUS_TRACE_SHOW = 2,
	UBUS_DFX_DUMP = 3,
	UBUS_ERR_INFO = 4,
};

enum ubus_msgq_dump_cmd_type {
	MSGQ_DUMP_HELP = 0,
	MSGQ_DUMP_REG = 1,
	MSGQ_DUMP_ENTRY = 2,
};

enum ubus_trace_show_cmd_type {
	TRACE_HELP = 0,
	TRACE_SHOW = 1,
};

enum ubus_dfx_dump_cmd_type {
	DFX_DUMP_HELP = 0,
	DFX_DUMP_SHOW = 1,
};

enum ubus_err_info_cmd_type {
	ERR_INFO_HELP = 0,
	ERR_INFO_SHOW = 1,
	ERR_INFO_CLEAR = 2,
};

void cmd_ubus_dfx_init(void);
void cmd_ubus_trace_init(void);
void cmd_ubus_msgq_dump_init(void);
void cmd_ubus_err_init(void);

#endif /* UBUS_COMMON_H */
