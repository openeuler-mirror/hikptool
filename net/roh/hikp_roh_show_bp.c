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
#include "hikp_roh_show_bp.h"
#include "hikp_roh_cmd.h"

static struct cmd_roh_show_bp_param g_roh_show_bp_param = { 0 };

static int hikp_roh_show_bp_help(struct major_cmd_ctrl *self, const char *argv)
{
	HIKP_SET_USED(argv);

	printf("\n  Usage: %s %s\n", self->cmd_ptr->name, "-i <interface> -s\n");
	printf("\n         %s\n", self->cmd_ptr->help_info);
	printf("  Options:\n\n");
	printf("    %s, %-25s %s\n", "-h", "--help", "display this help and exit");
	printf("    %s, %-25s %s\n", "-i", "--interface=<interface>", "device target, e.g. eth0");
	printf("    %s, %-25s %s\n", "-s", "--show", "show backpressure");
	printf("\n");
	return 0;
}

static int hikp_roh_show_bp_target(struct major_cmd_ctrl *self, const char *argv)
{
	self->err_no = tool_check_and_get_valid_bdf_id(argv, &(g_roh_show_bp_param.target));
	if (self->err_no) {
		snprintf(self->err_str, sizeof(self->err_str), "Unknown device %s.", argv);
		return self->err_no;
	}
	return 0;
}

static int hikp_roh_show_bp(struct major_cmd_ctrl *self)
{
	uint8_t pfc[BP_SIZE] = { 0 };
	uint8_t egu_tx_bp;
	union bp_val res;
	uint8_t flit_bp;
	int mac_id;
	int ret = 0;

	struct hikp_cmd_ret *cmd_ret = NULL;
	struct hikp_cmd_header req_header = { 0 };
	struct roh_show_bp_req_paras req_data = { 0 };
	struct roh_show_bp_rsp_t *bp_rsp = NULL;

	hikp_cmd_init(&req_header, ROH_MOD, HIKP_ROH_SHOW_BP, CMD_SHOW_BP);
	req_data.bdf = g_roh_show_bp_param.target.bdf;
	cmd_ret = hikp_cmd_alloc(&req_header, &req_data, sizeof(req_data));
	if (cmd_ret == NULL || cmd_ret->status != 0) {
		HIKP_ERROR_PRINT("fail to get bp info, retcode: %u\n",
				 cmd_ret ? cmd_ret->status : EIO);
		self->err_no = -EIO;
		ret = -EIO;
		goto out;
	}
	bp_rsp = (struct roh_show_bp_rsp_t *)(cmd_ret->rsp_data);
	res.val = bp_rsp->bp_val;
	mac_id = bp_rsp->mac_id;
	if (mac_id % VERIFY_MAC_ID != 0) {
		HIKP_ERROR_PRINT("Invalid mac id\n");
		self->err_no = -EINVAL;
		ret = -EINVAL;
		goto out;
	}

	for (int i = 0; i < BP_SIZE; i++) {
		if (mac_id == 0)
			pfc[i] = ((res._val.st_ssu_pfc_mac0)>>i) & 0x1;
		else
			pfc[i] = ((res._val.st_ssu_pfc_mac1)>>i) & 0x1;

		printf("MAC%d_pfc_%d : 0x%x\n", mac_id, i, pfc[i]);
	}
	egu_tx_bp = mac_id == 0 ? res._val.st_roh_egu_tx_bp_mac0 : res._val.st_roh_egu_tx_bp_mac1;
	flit_bp = mac_id == 0 ? res._val.st_hllc_roh_flit_bp_mac0 :
		  res._val.st_hllc_roh_flit_bp_mac1;
	printf("MAC%d_egu_tx_bp : 0x%x\n", mac_id, egu_tx_bp);
	printf("MAC%d_flit_bp : 0x%x\n", mac_id, flit_bp);

out:
	hikp_cmd_free(&cmd_ret);
	return ret;
}

static void hikp_roh_show_bp_execute(struct major_cmd_ctrl *self)
{
	int mac_type = hikp_roh_get_mac_type(self, g_roh_show_bp_param.target.bdf);

	if (g_roh_show_bp_param.flag & ROH_CMD_SHOW_BP) {
		if (mac_type == -EIO) {
			self->err_no = -EIO;
			return;
		}

		if (mac_type == 0) {
			HIKP_ERROR_PRINT("Current mac is not roh type, don't support\n");
			self->err_no = -EINVAL;
			return;
		}

		hikp_roh_show_bp(self);
	} else {
		self->err_no = -EINVAL;
		snprintf(self->err_str, sizeof(self->err_str), "Invalid operation\n");
	}
}

static int hikp_roh_show_bp_parse(struct major_cmd_ctrl *self, const char *argv)
{
	HIKP_SET_USED(self);
	HIKP_SET_USED(argv);

	g_roh_show_bp_param.flag |= ROH_CMD_SHOW_BP;
	return 0;
}

static void cmd_roh_show_bp_init(void)
{
	struct major_cmd_ctrl *major_cmd = get_major_cmd();

	major_cmd->option_count = 0;
	major_cmd->execute = hikp_roh_show_bp_execute;

	cmd_option_register("-h", "--help", false, hikp_roh_show_bp_help);
	cmd_option_register("-i", "--interface", true, hikp_roh_show_bp_target);
	cmd_option_register("-s", "--show", false, hikp_roh_show_bp_parse);
}

HIKP_CMD_DECLARE("roh_show_bp", "get roh bp information", cmd_roh_show_bp_init);
