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

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <inttypes.h>
#include "hikp_nic_fec.h"

static struct tool_target g_fec_target;

static int hikp_nic_fec_err_query(const struct bdf_t *bdf, struct nic_fec_err_info *info)
{
	struct hikp_cmd_header header = { 0 };
	struct nic_fec_req_para req = { 0 };
	struct hikp_cmd_ret *cmd_ret;
	struct nic_fec_rsp *rsp;
	int ret;

	req.bdf = *bdf;
	hikp_cmd_init(&header, NIC_MOD, GET_FEC_INFO_CMD, NIC_FEC_ERR_INFO_DUMP);
	cmd_ret = hikp_cmd_alloc(&header, &req, sizeof(req));
	if (cmd_ret == NULL || cmd_ret->status != 0) {
		ret = cmd_ret ? (int)(-cmd_ret->status) : -EIO;
		HIKP_ERROR_PRINT("fail to get fec info, retcode: %d\n", ret);
		hikp_cmd_free(&cmd_ret);
		return ret;
	}

	rsp = (struct nic_fec_rsp *)cmd_ret->rsp_data;
	*info = *(struct nic_fec_err_info *)rsp->data;
	hikp_cmd_free(&cmd_ret);

	if (info->fec_mode >= NIC_FEC_MODE_BUTT) {
		HIKP_ERROR_PRINT("unknown fec mode: %u\n", info->fec_mode);
		return -EINVAL;
	}

	return 0;
}

static const char *hikp_nic_fec_mode_name(const uint32_t mode)
{
	static const char *name[] = { "Off", "BaseR", "RS", "LLRS" };

	if (mode < HIKP_ARRAY_SIZE(name))
		return name[mode];
	else
		return "unknown";
}

static void hikp_nic_fec_err_show_basefec(const struct nic_fec_err_info *info)
{
	uint32_t lane_num = HIKP_MIN(info->basefec.lane_num, NIC_FEC_MAX_LANES);
	uint64_t total;
	uint32_t i;

	printf("Statistics:\n");

	/* show corrected blocks */
	for (total = 0, i = 0; i < lane_num; i++)
		total += info->basefec.lane_corr_block_cnt[i];

	printf(" corrected_blocks: %" PRIu64 "\n", total);
	for (i = 0; i < lane_num; i++)
		printf("   Lane %u: %u\n", i, info->basefec.lane_corr_block_cnt[i]);

	/* show uncorrected blocks */
	for (total = 0, i = 0; i < lane_num; i++)
		total += info->basefec.lane_uncorr_block_cnt[i];

	printf(" uncorrectable_blocks: %" PRIu64 "\n", total);
	for (i = 0; i < lane_num; i++)
		printf("   Lane %u: %u\n", i, info->basefec.lane_uncorr_block_cnt[i]);
}

static void hikp_nic_fec_err_show_rsfec(const struct nic_fec_err_info *info)
{
	printf("Statistics:\n");
	printf(" corrected_blocks: %u\n", info->rsfec.corr_cw_cnt);
	printf(" uncorrectable_blocks: %u\n", info->rsfec.uncorr_cw_cnt);
}

static void hikp_nic_fec_err_show(const struct nic_fec_err_info *info)
{
	printf("############## NIC FEC err info ################\n");
	/* The output format similar to 'ethtool -I --show-fec ethx' */
	printf("Active FEC encoding: %s\n", hikp_nic_fec_mode_name(info->fec_mode));
	if (info->fec_mode == NIC_FEC_MODE_BASEFEC) {
		hikp_nic_fec_err_show_basefec(info);
	} else if (info->fec_mode == NIC_FEC_MODE_RSFEC ||
		   info->fec_mode == NIC_FEC_MODE_LLRSFEC) {
		hikp_nic_fec_err_show_rsfec(info);
	} else {
		/* Do nothing */
	}
	printf("#################### END #######################\n");
}

static void hikp_nic_fec_cmd_execute(struct major_cmd_ctrl *self)
{
	struct bdf_t *bdf = &g_fec_target.bdf;
	struct nic_fec_err_info info = { 0 };
	int ret;

	ret = hikp_nic_fec_err_query(bdf, &info);
	if (ret != 0) {
		snprintf(self->err_str, sizeof(self->err_str), "fail to obtain fec err info.");
		self->err_no = ret;
		return;
	}

	hikp_nic_fec_err_show(&info);
}

static int hikp_nic_fec_cmd_help(struct major_cmd_ctrl *self, const char *argv)
{
	printf("\n  Usage: %s %s\n", self->cmd_ptr->name, "-i <device>");
	printf("\n         %s\n", self->cmd_ptr->help_info);
	printf("\n  Options:\n\n");
	printf("    %s, %-25s %s\n", "-h", "--help", "display this help and exit");
	printf("    %s, %-25s %s\n", "-i", "--interface=<interface>",
	       "device target or bdf id, e.g. eth0~7 or 0000:35:00.0");

	return 0;
}

static int hikp_nic_fec_get_target(struct major_cmd_ctrl *self, const char *argv)
{
	self->err_no = tool_check_and_get_valid_bdf_id(argv, &g_fec_target);
	if (self->err_no != 0) {
		snprintf(self->err_str, sizeof(self->err_str), "unknown device!");
		return self->err_no;
	}

	if (g_fec_target.bdf.dev_id != 0) {
		snprintf(self->err_str, sizeof(self->err_str), "VF is not supported!");
		self->err_no = -EINVAL;
		return self->err_no;
	}

	return 0;
}

static void cmd_nic_fec_init(void)
{
	struct major_cmd_ctrl *major_cmd = get_major_cmd();

	major_cmd->option_count = 0;
	major_cmd->execute = hikp_nic_fec_cmd_execute;

	cmd_option_register("-h", "--help", false, hikp_nic_fec_cmd_help);
	cmd_option_register("-i", "--interface", true, hikp_nic_fec_get_target);
}

HIKP_CMD_DECLARE("nic_fec", "dump fec info of nic!", cmd_nic_fec_init);
