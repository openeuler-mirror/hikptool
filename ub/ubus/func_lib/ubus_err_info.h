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

#ifndef UBUS_ERR_INFO_H
#define UBUS_ERR_INFO_H

#include <stdint.h>

struct ubus_err_info_req_param {
	uint32_t port_id;
};

struct ubus_err_data {
	uint32_t bit_err_cnt;
	uint32_t frm_cnt;
	uint32_t frm_cnt_err_1symb;
	uint32_t frm_cnt_err_2symb;
	uint32_t frm_cnt_err_3symb;
	uint32_t frm_cnt_err_4symb;
	uint32_t frm_cnt_err_5symb;
	uint32_t frm_cnt_err_6symb;
	uint32_t frm_cnt_err_7symb;
	uint32_t frm_cnt_err_8symb;
	uint32_t frm_cnt_err_more_symb;
	uint32_t frm_cnt_err_fail_symb_t2;
	uint32_t frm_cnt_err_fail_symb_t4;
};

int ubus_err_show_execute(uint32_t port_id);
int ubus_err_clear_execute(uint32_t port_id);
#endif /* UBUS_ERR_INFO_H */
