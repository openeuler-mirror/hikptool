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

#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include "tool_lib.h"
#include "hikptdev_plug.h"
#include "ubus_common.h"
#include "ubus_err_info.h"

static int err_show_rsp_data_check(struct hikp_cmd_ret *cmd_ret)
{
	if (cmd_ret->rsp_data_num != sizeof(struct ubus_err_data) / sizeof(uint32_t))
		return -EINVAL;

	return 0;
}

static int err_clear_rsp_data_check(struct hikp_cmd_ret *cmd_ret)
{
	HIKP_SET_USED(cmd_ret);
	/* Not supported in the current version. The value 0 is returned. */
	return 0;
}

static void err_info_print(struct hikp_cmd_ret *cmd_ret)
{
	struct ubus_err_data *err_data = (struct ubus_err_data *)(cmd_ret->rsp_data);

	printf("===========ubus err show===============\n");
	printf("ubus err info:\n");
	printf("\terr info detail:\n");
	printf("    bit err cnt: 0x%08x\n", err_data->bit_err_cnt);
	printf("    frm cnt: 0x%08x\n", err_data->frm_cnt);
	printf("    frm cnt err 1symb: 0x%08x\n", err_data->frm_cnt_err_1symb);
	printf("    frm cnt err 2symb: 0x%08x\n", err_data->frm_cnt_err_2symb);
	printf("    frm cnt err 3symb: 0x%08x\n", err_data->frm_cnt_err_3symb);
	printf("    frm cnt err 4symb: 0x%08x\n", err_data->frm_cnt_err_4symb);
	printf("    frm cnt err 5symb: 0x%08x\n", err_data->frm_cnt_err_5symb);
	printf("    frm cnt err 6symb: 0x%08x\n", err_data->frm_cnt_err_6symb);
	printf("    frm cnt err 7symb: 0x%08x\n", err_data->frm_cnt_err_7symb);
	printf("    frm cnt err 8symb: 0x%08x\n", err_data->frm_cnt_err_8symb);
	printf("    frm cnt err more symb: 0x%08x\n", err_data->frm_cnt_err_more_symb);
	printf("    frm cnt err fail symb t2: 0x%08x\n", err_data->frm_cnt_err_fail_symb_t2);
	printf("    frm cnt err fail symb t4: 0x%08x\n", err_data->frm_cnt_err_fail_symb_t4);
	printf("===========ubus err end================\n");
}

int ubus_err_show_execute(uint32_t port_id)
{
	struct ubus_err_info_req_param req_param = { 0 };
	struct hikp_cmd_header req_header = { 0 };
	struct hikp_cmd_ret *cmd_ret;
	int ret;

	hikp_cmd_init(&req_header, UBUS_MOD, UBUS_ERR_INFO, ERR_INFO_SHOW);
	req_param.port_id = port_id;

	cmd_ret = hikp_cmd_alloc(&req_header, &req_param, sizeof(req_param));
	ret = hikp_rsp_normal_check(cmd_ret);
	if (ret) {
		printf("check cmd ret failed, ret: %d.\n", ret);
		goto free_cmd_ret;
	}

	ret = err_show_rsp_data_check(cmd_ret);
	if (ret) {
		printf("check err show rsp data failed, ret: %d.\n", ret);
		goto free_cmd_ret;
	}

	err_info_print(cmd_ret);

free_cmd_ret:
	hikp_cmd_free(&cmd_ret);
	return ret;
}

int ubus_err_clear_execute(uint32_t port_id)
{
	struct ubus_err_info_req_param req_param = { 0 };
	struct hikp_cmd_header req_header = { 0 };
	struct hikp_cmd_ret *cmd_ret;
	int ret;

	hikp_cmd_init(&req_header, UBUS_MOD, UBUS_ERR_INFO, ERR_INFO_CLEAR);
	req_param.port_id = port_id;

	cmd_ret = hikp_cmd_alloc(&req_header, &req_param, sizeof(req_param));
	ret = hikp_rsp_normal_check(cmd_ret);
	if (ret) {
		printf("check cmd ret failed, ret: %d.\n", ret);
		goto free_cmd_ret;
	}

	ret = err_clear_rsp_data_check(cmd_ret);
	if (ret) {
		printf("ubus err info clear failed with ret: %d.\n", ret);
	} else {
		printf("ubus err info clear success.\n");
	}

free_cmd_ret:
	hikp_cmd_free(&cmd_ret);
	return ret;
}
