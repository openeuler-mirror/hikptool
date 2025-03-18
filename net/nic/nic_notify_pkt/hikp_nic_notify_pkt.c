/*
 * Copyright (c) 2023 Hisilicon Technologies Co., Ltd.
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
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include "hikp_nic_notify_pkt.h"

static struct tool_target g_notify_pkt_target;

static int hikp_nic_notify_pkt_query(struct major_cmd_ctrl *self, const struct bdf_t *bdf,
				     struct nic_notify_pkt_info *info)
{
	struct nic_notify_pkt_req_para req = { 0 };
	struct hikp_cmd_header header = { 0 };
	struct nic_notify_pkt_rsp *rsp;
	struct hikp_cmd_ret *cmd_resp;

	req.bdf = *bdf;
	hikp_cmd_init(&header, NIC_MOD, GET_NOTIFY_PKT_CMD, GET_NOTIFY_PKT_CMD);
	cmd_resp = hikp_cmd_alloc(&header, &req, sizeof(req));
	self->err_no = hikp_rsp_normal_check(cmd_resp);
	if (self->err_no) {
		snprintf(self->err_str, sizeof(self->err_str), "get notify pkt failed.");
		hikp_cmd_free(&cmd_resp);
		return self->err_no;
	}

	rsp = (struct nic_notify_pkt_rsp *)cmd_resp->rsp_data;
	*info = *(struct nic_notify_pkt_info *)rsp->data;
	hikp_cmd_free(&cmd_resp);

	return 0;
}

static void hikp_nic_notify_pkt_show(const struct nic_notify_pkt_info *info)
{
#define HIKP_NIC_NOFITY_PKT_DATA_PEER_LINE_MAX_CNT 16

	uint32_t pkt_start_en = hikp_get_bit(info->cfg, HIKP_NOTIFY_PKT_CFG_START_EN);
	uint32_t pkt_num = hikp_get_field(info->cfg, HIKP_NOTIFY_PKT_CFG_PKT_NUM_M,
					  HIKP_NOTIFY_PKT_CFG_PKT_NUM_S);
	uint32_t pkt_en = hikp_get_bit(info->cfg, HIKP_NOTIFY_PKT_CFG_PKT_EN);
	uint32_t i;

	printf("################ NIC notify pkt info ##################\n");
	printf("pkt_en       : %u\n", pkt_en);
	printf("pkt_start_en : %u\n", pkt_start_en);
	printf("pkt_num      : %u\n", pkt_num);
	printf("pkt_ipg      : %u %s\n", info->ipg, info->ipg > 1 ?
		"clock cycles" : "clock cycle");

	printf("pkt_data:\n");
	for (i = 1; i <= NIC_NOTIFY_PKT_DATA_LEN; i++) {
		printf("%02x ", info->data[i - 1]);
		if (i % HIKP_NIC_NOFITY_PKT_DATA_PEER_LINE_MAX_CNT == 0)
			printf("\n");
	}
	printf("####################### END ###########################\n");
}

void hikp_nic_notify_pkt_cmd_execute(struct major_cmd_ctrl *self)
{
	struct bdf_t *bdf = &g_notify_pkt_target.bdf;
	struct nic_notify_pkt_info info = {0};

	self->err_no = hikp_nic_notify_pkt_query(self, bdf, &info);
	if (self->err_no)
		return;

	hikp_nic_notify_pkt_show(&info);
}

static int hikp_nic_notify_pkt_cmd_help(struct major_cmd_ctrl *self, const char *argv)
{
	HIKP_SET_USED(argv);

	printf("\n  Usage: %s %s\n", self->cmd_ptr->name, "-i <device>");
	printf("\n         %s\n", self->cmd_ptr->help_info);
	printf("\n  Options:\n\n");
	printf("    %s, %-25s %s\n", "-h", "--help", "display this help and exit");
	printf("    %s, %-25s %s\n", "-i", "--interface=<interface>",
	       "device target or bdf id, e.g. eth0~7 or 0000:35:00.0");

	return 0;
}

int hikp_nic_notify_pkt_get_target(struct major_cmd_ctrl *self, const char *argv)
{
	self->err_no = tool_check_and_get_valid_bdf_id(argv, &g_notify_pkt_target);
	if (self->err_no != 0) {
		snprintf(self->err_str, sizeof(self->err_str), "unknown device!");
		return self->err_no;
	}

	if (g_notify_pkt_target.bdf.dev_id != 0) {
		snprintf(self->err_str, sizeof(self->err_str), "VF does not support query!");
		self->err_no = -EOPNOTSUPP;
		return self->err_no;
	}

	return 0;
}

static void cmd_nic_notify_pkt_init(void)
{
	struct major_cmd_ctrl *major_cmd = get_major_cmd();

	major_cmd->option_count = 0;
	major_cmd->execute = hikp_nic_notify_pkt_cmd_execute;

	cmd_option_register("-h", "--help", false, hikp_nic_notify_pkt_cmd_help);
	cmd_option_register("-i", "--interface", true, hikp_nic_notify_pkt_get_target);
}

HIKP_CMD_DECLARE("nic_notify_pkt", "dump notify pkt info of nic!", cmd_nic_notify_pkt_init);
