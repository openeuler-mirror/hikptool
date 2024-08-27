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
#include "hikpt_rciep.h"
#include "hikp_nic_qos.h"

static struct nic_qos_param g_qos_param = { 0 };

static void hikp_nic_qos_show_pkt_buf(const void *data);
static void hikp_nic_qos_show_dcb_info(const void *data);
static void hikp_nic_qos_show_pause_info(const void *data);
static void hikp_nic_qos_show_pfc_storm_para(const void *data);

static const struct qos_feature_cmd g_qos_feature_cmd[] = {
	{"pkt_buf", NIC_PACKET_BUFFER_DUMP, hikp_nic_qos_show_pkt_buf},
	{"dcb",     NIC_DCB_DUMP,       hikp_nic_qos_show_dcb_info},
	{"pause",   NIC_PAUSE_DUMP,         hikp_nic_qos_show_pause_info},
	{"pfc_storm_para", NIC_PFC_STORM_PARA_DUMP,
	 hikp_nic_qos_show_pfc_storm_para},
};

void hikp_nic_qos_set_cmd_feature_idx(int feature_idx)
{
	g_qos_param.feature_idx = feature_idx;
}

void hikp_nic_qos_set_cmd_direction(enum nic_pfc_dir dir)
{
	g_qos_param.dir = dir;
}

static int hikp_nic_qos_cmd_help(struct major_cmd_ctrl *self, const char *argv)
{
	printf("\n  Usage: %s %s\n", self->cmd_ptr->name, "-i <device>");
	printf("\n         %s\n", self->cmd_ptr->help_info);
	printf("\n  Options:\n\n");
	printf("    %s, %-25s %s\n", "-h", "--help", "display this help and exit");
	printf("	%s, %-25s %s\n", "-i", "--interface=<interface>",
	       "device target or bdf id, e.g. eth0~7 or 0000:35:00.0");
	printf("      %s\n",
	       "[-g/--get <options>]\n"
	       "          pkt_buf : get nic packet buffer.\n"
	       "          dcb     : get dcb information.\n"
	       "          pause   : get pause information\n"
	       "          pfc_storm_para : get pfc storm configuration parameters\n");
	printf("      %s\n",
	       "[-d/--dir <options>]\n"
	       "          tx : transmit.\n"
	       "          rx : receive.\n");
	return 0;
}

static void hikp_nic_qos_show_pkt_buf(const void *data)
{
	struct qos_cmd_info *qos_info_pkt = (struct qos_cmd_info *)data;
	struct nic_pkt_buf_info *pkt_buf = (struct nic_pkt_buf_info *)&qos_info_pkt->info;
	struct nic_shared_buf *share_buf = &pkt_buf->share_buf;
	struct nic_priv_buf *priv_buf = pkt_buf->priv_buf;
	uint16_t tc_no;

	printf("Rx Shared packet buffer configuration\n");
	printf("  > buffer size: 0x%x\n", share_buf->buf_size);
	printf("  > common waterline high: 0x%x  low: 0x%x\n",
	       share_buf->comm_wl.high, share_buf->comm_wl.low);
	printf("  > common threshold: 0x%x  low: 0x%x\n",
	       share_buf->comm_wl.high, share_buf->comm_wl.low);
	for (tc_no = 0; tc_no < HIKP_NIC_MAX_TC_NUM; tc_no++) {
		printf("    - tc%u high: 0x%x  low: 0x%x\n", tc_no, share_buf->tc_thrd[tc_no].high,
		       share_buf->tc_thrd[tc_no].low);
	}

	printf("\nRx Privated buffer waterline\n");
	for (tc_no = 0; tc_no < HIKP_NIC_MAX_TC_NUM; tc_no++)
		printf("  > tc%u high: 0x%x  low: 0x%x\n", tc_no, priv_buf[tc_no].wl.high,
		       priv_buf[tc_no].wl.low);

	printf("\nRx Privated packet buffer configuration\n");
	for (tc_no = 0; tc_no < HIKP_NIC_MAX_TC_NUM; tc_no++)
		printf("  > tc%u Rx buffer size: 0x%x\n", tc_no, priv_buf[tc_no].rx_buf_size);

	printf("\nTx Privated packet buffer configuration\n");
	for (tc_no = 0; tc_no < HIKP_NIC_MAX_TC_NUM; tc_no++)
		printf("  > tc%u Tx buffer size: 0x%x\n", tc_no, priv_buf[tc_no].tx_buf_size);
}

static void hikp_nic_qos_show_dcb_info(const void *data)
{
	struct qos_cmd_info *qos_info_dcb = (struct qos_cmd_info *)data;
	struct nic_dcb_info *dcb = (struct nic_dcb_info *)&qos_info_dcb->info;
	struct nic_pfc_info *pfc = &dcb->pfc;
	struct nic_ets_info *ets = &dcb->ets;
	uint16_t tc_no;
	uint16_t up;

	printf("PFC configuration\n");
	printf("  PFC enable:");
	for (up = 0; up < HIKP_NIC_MAX_USER_PRIO_NUM; up++)
		printf(" %u", HI_BIT(up) & pfc->pfc_en ? 1 : 0);

	printf("\n");
	printf("  TC enable:");
	for (tc_no = 0; tc_no < HIKP_NIC_MAX_TC_NUM; tc_no++)
		printf(" %u", HI_BIT(tc_no) & pfc->hw_tc_map ? 1 : 0);

	printf("\n");
	printf("ETS configuration\n");
	printf("  max_tc_cap: %u\n", ets->max_tc);
	printf("  up2tc:");
	for (up = 0; up < HIKP_NIC_MAX_USER_PRIO_NUM; up++)
		printf(" %u:%u", up, ets->prio_tc[up]);

	printf("\n");
	printf("  tc_bw:");
	for (tc_no = 0; tc_no < HIKP_NIC_MAX_TC_NUM; tc_no++)
		printf(" %u:%u%%", tc_no, ets->tc_bw[tc_no]);

	printf("\n");
	printf("  tsa_map:");
	for (tc_no = 0; tc_no < HIKP_NIC_MAX_TC_NUM; tc_no++)
		printf(" %u:%s", tc_no, ets->sch_mode[tc_no] == 0 ? "strict" : "ets");
	printf("\n");
}

static void hikp_nic_qos_show_pause_info(const void *data)
{
	struct qos_cmd_info *qos_info_pause = (struct qos_cmd_info *)data;
	struct nic_pause_info *pause = (struct nic_pause_info *)&qos_info_pause->info;

	printf("PAUSE Information\n");
	if (pause->type == HIKP_NONE_PAUSE)
		printf("pause type: none\n");
	else if (pause->type == HIKP_MAC_PAUSE)
		printf("pause type: MAC pause\n");
	else if (pause->type == HIKP_PFC)
		printf("pause type: PFC\n");
	printf("  pause_rx: %s\n", pause->pause_rx ? "On" : "Off");
	printf("  pause_tx: %s\n", pause->pause_tx ? "On" : "Off");

	printf("pause time: 0x%x\n", pause->pause_time);
	printf("pause gap: 0x%x\n", pause->pause_gap);
}

static void hikp_nic_qos_show_pfc_storm_para(const void *data)
{
	struct qos_cmd_info *qos_info_pfc = (struct qos_cmd_info *)data;
	struct nic_pfc_storm_para *pfc_storm_para =
		(struct nic_pfc_storm_para *)&qos_info_pfc->info;
	uint32_t length = qos_info_pfc->length;

	printf("PFC STORM Information:\n");
	printf("direction: %s\n", pfc_storm_para->dir ? "tx" : "rx");
	printf("enabled: %s\n", pfc_storm_para->enable ? "on" : "off");
	printf("period: %ums\n", pfc_storm_para->period_ms);
	strncmp(g_qos_param.revision_id, HIKP_IEP_REVISION,
		MAX_PCI_REVISION_LEN) ?
		printf("check times: %u\n", pfc_storm_para->times) :
		printf("pfc threshold: %ums\n", pfc_storm_para->times);
	printf("recovery period: %ums\n", pfc_storm_para->recovery_period_ms);

	if (length < sizeof(struct nic_pfc_storm_para))
		return;

	printf("storm count: %u\n", pfc_storm_para->storm_count);
}

static int hikp_nic_qos_get_blk(struct hikp_cmd_header *req_header,
				const struct nic_qos_req_para *req_data,
				void *buf, size_t buf_len, struct nic_qos_rsp_head *rsp_head)
{
	struct hikp_cmd_ret *cmd_ret;
	struct nic_qos_rsp *rsp;
	int ret = 0;

	cmd_ret = hikp_cmd_alloc(req_header, req_data, sizeof(*req_data));
	ret = hikp_rsp_normal_check(cmd_ret);
	if (ret) {
		HIKP_ERROR_PRINT("failed to get block-%u context.\n", req_data->block_id);
		goto out;
	}

	rsp = (struct nic_qos_rsp *)cmd_ret->rsp_data;
	if (rsp->rsp_head.cur_blk_size > buf_len ||
	    rsp->rsp_head.cur_blk_size > sizeof(rsp->rsp_data)) {
		HIKP_ERROR_PRINT("nic_qos block-%u copy size error, "
				 "dst buffer size=%zu, src buffer size=%zu, "
				 "data size=%u.\n", req_data->block_id, buf_len,
				 sizeof(rsp->rsp_data), rsp->rsp_head.cur_blk_size);
		ret = -EINVAL;
		goto out;
	}
	memcpy(buf, rsp->rsp_data, rsp->rsp_head.cur_blk_size);
	rsp_head->total_blk_num = rsp->rsp_head.total_blk_num;
	rsp_head->cur_blk_size = rsp->rsp_head.cur_blk_size;

out:
	hikp_cmd_free(&cmd_ret);
	return ret;
}

static int hikp_nic_query_qos_feature(struct hikp_cmd_header *req_header, const struct bdf_t *bdf,
				      struct qos_cmd_info *qcmd_info)
{
	size_t buf_len = sizeof(qcmd_info->info);
	struct nic_qos_rsp_head rsp_head = {0};
	struct nic_qos_req_para req_data;
	uint32_t total_blk_size;
	uint8_t total_blk_num;
	uint8_t blk_id = 0;
	int ret;

	req_data.bdf = *bdf;

	req_data.block_id = blk_id;
	req_data.dir = g_qos_param.dir;

	ret = hikp_nic_qos_get_blk(req_header, &req_data, &qcmd_info->info, buf_len, &rsp_head);
	if (ret != 0)
		return ret;

	total_blk_num = rsp_head.total_blk_num;
	total_blk_size = rsp_head.cur_blk_size;

	/* Copy the remaining block content if total block number is greater than 1. */
	for (blk_id = 1; blk_id < total_blk_num; blk_id++) {
		req_data.block_id = blk_id;
		req_data.dir = g_qos_param.dir;

		ret = hikp_nic_qos_get_blk(req_header, &req_data,
					   (uint8_t *)&qcmd_info->info + total_blk_size,
					   buf_len - total_blk_size, &rsp_head);
		if (ret != 0)
			return ret;
		total_blk_size += rsp_head.cur_blk_size;
	}

	qcmd_info->length = total_blk_size;

	return ret;
}

void hikp_nic_qos_cmd_execute(struct major_cmd_ctrl *self)
{
	char *revision_id = g_qos_param.revision_id;
	struct bdf_t *bdf = &g_qos_param.target.bdf;
	struct hikp_cmd_header req_header = {0};
	const struct qos_feature_cmd *qos_cmd;
	struct qos_cmd_info qos_data = {0};
	int ret;

	if (bdf->dev_id != 0) {
		snprintf(self->err_str, sizeof(self->err_str), "VF does not support query!");
		self->err_no = -EINVAL;
		return;
	}

	if (g_qos_param.feature_idx == -1) {
		hikp_nic_qos_cmd_help(self, NULL);
		snprintf(self->err_str, sizeof(self->err_str), "-g/--get param error!");
		self->err_no = -EINVAL;
		return;
	}

	if (g_qos_param.feature_idx == NIC_PFC_STORM_PARA_DUMP &&
	    g_qos_param.dir == NIC_QOS_DIR_NONE) {
		hikp_nic_qos_cmd_help(self, NULL);
		snprintf(self->err_str, sizeof(self->err_str),
			 "-d/--dir param error!");
		self->err_no = -EINVAL;
		return;
	}

	qos_cmd = &g_qos_feature_cmd[g_qos_param.feature_idx];
	hikp_cmd_init(&req_header, NIC_MOD, GET_QOS_INFO_CMD, qos_cmd->sub_cmd_code);
	ret = hikp_nic_query_qos_feature(&req_header, &g_qos_param.target.bdf, &qos_data);
	if (ret != 0) {
		snprintf(self->err_str, sizeof(self->err_str), "failed to query %s, ret = %d.",
			 qos_cmd->feature_name, ret);
		self->err_no = ret;
		return;
	}

	memset(revision_id, 0, MAX_PCI_ID_LEN + 1);
	ret = get_revision_id_by_bdf(bdf, revision_id, sizeof(g_qos_param.revision_id));
	// show pfc threshold as default if get revision_id error
	if (ret) {
		strncpy(g_qos_param.revision_id, HIKP_IEP_REVISION, MAX_PCI_REVISION_LEN);
		g_qos_param.revision_id[MAX_PCI_ID_LEN] = '\0';
	}

	printf("############## NIC QOS: %s info ############\n", qos_cmd->feature_name);
	qos_cmd->show(&qos_data);
	printf("#################### END #######################\n");
}

int hikp_nic_cmd_get_qos_target(struct major_cmd_ctrl *self, const char *argv)
{
	self->err_no = tool_check_and_get_valid_bdf_id(argv, &(g_qos_param.target));
	if (self->err_no != 0) {
		snprintf(self->err_str, sizeof(self->err_str), "unknown device!");
		return self->err_no;
	}

	return 0;
}

static int hikp_nic_cmd_qos_feature_select(struct major_cmd_ctrl *self, const char *argv)
{
	size_t feat_size = HIKP_ARRAY_SIZE(g_qos_feature_cmd);
	size_t i;

	for (i = 0; i < feat_size; i++) {
		if (strcmp(argv, g_qos_feature_cmd[i].feature_name) == 0) {
			g_qos_param.feature_idx = i;
			return 0;
		}
	}

	hikp_nic_qos_cmd_help(self, NULL);
	snprintf(self->err_str, sizeof(self->err_str), "-g/--get param error!!!");
	self->err_no = -EINVAL;

	return self->err_no;
}

static int hikp_nic_cmd_qos_direct(struct major_cmd_ctrl *self,
				   const char *argv)
{
	if (strcmp(argv, "rx") == 0) {
		g_qos_param.dir = NIC_RX_QOS;
		return 0;
	}
	if (strcmp(argv, "tx") == 0) {
		g_qos_param.dir = NIC_TX_QOS;
		return 0;
	}

	snprintf(self->err_str, sizeof(self->err_str),
		 "-d/--dir option is invalid.");
	self->err_no = -EINVAL;

	return self->err_no;
}

static void cmd_nic_get_qos_init(void)
{
	struct major_cmd_ctrl *major_cmd = get_major_cmd();

	g_qos_param.feature_idx = -1;
	g_qos_param.dir = NIC_QOS_DIR_NONE;

	major_cmd->option_count = 0;
	major_cmd->execute = hikp_nic_qos_cmd_execute;

	cmd_option_register("-h", "--help", false, hikp_nic_qos_cmd_help);
	cmd_option_register("-i", "--interface", true, hikp_nic_cmd_get_qos_target);
	cmd_option_register("-g", "--get", true, hikp_nic_cmd_qos_feature_select);
	cmd_option_register("-d", "--dir", true, hikp_nic_cmd_qos_direct);
}

HIKP_CMD_DECLARE("nic_qos", "show qos info of nic!", cmd_nic_get_qos_init);
