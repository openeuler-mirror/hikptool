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

#include "hikp_nic_queue.h"

static struct nic_queue_param g_queue_param = { 0 };

static void hikp_nic_queue_show_basic_info(const void *data);
static void hikp_nic_queue_show_enable_info(const void *data);
static void hikp_nic_queue_show_intr_map(const void *data);
static void hikp_nic_queue_show_func_map(const void *data);

static const struct queue_feature_cmd g_queue_feature_cmd[] = {
	{"basic_info", QUEUE_BASIC_INFO, hikp_nic_queue_show_basic_info},
	{"queue_en",   QUEUE_EN_INFO,    hikp_nic_queue_show_enable_info},
	{"intr_map",   QUEUE_INTR_MAP,   hikp_nic_queue_show_intr_map},
	{"func_map",   QUEUE_FUNC_MAP,   hikp_nic_queue_show_func_map},
};

static int hikp_nic_queue_cmd_help(struct major_cmd_ctrl *self, const char *argv)
{
	printf("\n  Usage: %s %s\n", self->cmd_ptr->name, "-i <device>");
	printf("\n         %s\n", self->cmd_ptr->help_info);
	printf("\n  Options:\n\n");
	printf("    %s, %-25s %s\n", "-h", "--help", "display this help and exit");
	printf("	%s, %-25s %s\n", "-i", "--interface=<interface>",
	       "device target or bdf id, e.g. eth0~7 or 0000:35:00.0");
	printf("%s\n",
	       "      [-du/--dump basic_info -d/--dir <rx/tx> -q/--qid <q_id>]\n"
	       "              dump a Rx/Tx queue basic info.\n"
	       "      [-du/--dump queue_en -a/--all <on/off>]\n"
	       "              dump Rx & Tx queue enable info\n"
	       "      [-du/--dump intr_map -d/--dir <rx/tx> -a/--all <on/off>]\n"
	       "              dump all Rx/Tx queue intr map.\n"
	       "      [-du/--dump func_map]\n"
	       "              display the queue mapping between the function and "
	       "the global queue on the port.\n");
	printf("%s\n",
	       "  Note: '-a/--all' means if display all queues including disabled or unused, "
	       "defaultly only display\n        enabled and used queues.\n");

	return 0;
}

#define HIKP_NIC_GET_DESC_NUM(num) (((num) + 1) * 8)

#define HIKP_NIC_BD_SIZE_512_TYPE       0
#define HIKP_NIC_BD_SIZE_1024_TYPE      1
#define HIKP_NIC_BD_SIZE_2048_TYPE      2
#define HIKP_NIC_BD_SIZE_4096_TYPE      3

#define HIKP_NIC_BUF_LEN_512B           512
#define HIKP_NIC_BUF_LEN_1024B          1024
#define HIKP_NIC_BUF_LEN_2048B          2048
#define HIKP_NIC_BUF_LEN_4096B          4096
static uint32_t hikp_nic_buf_type_to_buf_len(uint32_t type)
{
	uint32_t rx_buf_len;

	switch (type) {
	case HIKP_NIC_BD_SIZE_512_TYPE:
		rx_buf_len = HIKP_NIC_BUF_LEN_512B;
		break;
	case HIKP_NIC_BD_SIZE_1024_TYPE:
		rx_buf_len = HIKP_NIC_BUF_LEN_1024B;
		break;
	case HIKP_NIC_BD_SIZE_2048_TYPE:
		rx_buf_len = HIKP_NIC_BUF_LEN_2048B;
		break;
	case HIKP_NIC_BD_SIZE_4096_TYPE:
		rx_buf_len = HIKP_NIC_BUF_LEN_4096B;
		break;
	default:
		rx_buf_len = 0;
		break;
	}

	return rx_buf_len;
}

static void hikp_nic_queue_show_basic_info(const void *data)
{
	union nic_queue_info *q_info = (union nic_queue_info *)data;
	struct rx_queue_info *rxq;
	struct tx_queue_info *txq;

	printf("%s queue id-%d basic info:\n",
	       g_queue_param.dir == NIC_RX_QUEUE ? "Rx" : "Tx", g_queue_param.qid);
	if (g_queue_param.dir == NIC_RX_QUEUE) {
		rxq = &q_info->rxq;
		printf("  nb_desc = %u\n", HIKP_NIC_GET_DESC_NUM(rxq->rx_nb_desc));
		printf("  tail = %u\n", rxq->rx_tail);
		printf("  head = %u\n", rxq->rx_head);
		printf("  fbd_num = %u\n", rxq->rx_fbd_num);
		printf("  ring_offset = %u\n", rxq->rx_ring_offset);
		printf("  fbd_offset = %u\n", rxq->rx_fbd_offset);
		printf("  rx_buf_len = %u\n", hikp_nic_buf_type_to_buf_len(rxq->rx_buf_len));
		printf("  ring_tcid = %u\n", rxq->rx_ring_tcid);
		printf("  merge_en = %s\n", rxq->rx_merge_en ? "enable" : "disable");
		printf("  bd_err = %u\n", rxq->rx_bd_err);
		printf("  rx_stash_lpid = 0x%x\n", rxq->rx_stash_lpid);
		return;
	}

	txq = &q_info->txq;
	printf("  nb_desc = %u\n", HIKP_NIC_GET_DESC_NUM(txq->tx_nb_desc));
	printf("  tail = %u\n", txq->tx_tail);
	printf("  head = %u\n", txq->tx_head);
	printf("  fbd_num = %u\n", txq->tx_fbd_num);
	printf("  ring_offset = %u\n", txq->tx_ring_offset);
	printf("  ebd_num = %u\n", txq->tx_ebd_num);
	printf("  ebd_offset = %u\n", txq->tx_ring_ebd_offset);
	printf("  tx_ring_pri = %u\n", txq->tx_ring_pri);
	printf("  tx_ring_tc = %u\n", txq->tx_ring_tc);
	printf("  merge_en = %s\n", txq->tx_merge_en ? "enable" : "disable");
	printf("  bd_err = %u\n", txq->tx_ring_bd_err);
	printf("  ring_fbd_prf_num = 0x%x\n", txq->tx_ring_fbd_prf_num);
}

static void hikp_nic_queue_show_enable_info(const void *data)
{
	struct nic_queue_en_cfg_info *q_en_info = (struct nic_queue_en_cfg_info *)data;
	struct nic_queue_en_cfg *q_en_cfg;
	uint16_t qid;
	bool rx_en;
	bool tx_en;

	printf("Rx & Tx Queue enable info[tqp_num=%u display_all=%s]:\n", q_en_info->tqp_num,
	       g_queue_param.is_display_all ? "On" : "Off");
	for (qid = 0; qid < q_en_info->tqp_num; qid++) {
		if (qid >= HIKP_NIC_MAX_QUEUE_NUM) {
			HIKP_ERROR_PRINT("The cmd data is truncated.\n");
			break;
		}
		q_en_cfg = &q_en_info->q_en_cfg[qid];
		rx_en = q_en_cfg->rcb_en && q_en_cfg->rcb_rx_en;
		tx_en = q_en_cfg->rcb_en && q_en_cfg->rcb_tx_en;
		if (rx_en || tx_en || g_queue_param.is_display_all) {
			printf(" qid-%u: tqp_en:%u rcb_rx_en:%u rcb_tx_en:%u Rx:%s Tx:%s\n",
			       qid, q_en_cfg->rcb_en, q_en_cfg->rcb_rx_en, q_en_cfg->rcb_tx_en,
			       rx_en ? "enable" : "disable", tx_en ? "enable" : "disable");
		}
	}
}

static void hikp_nic_queue_show_intr_map(const void *data)
{
	struct nic_queue_intr_map *map = (struct nic_queue_intr_map *)data;
	struct rcb_intr_ctl_cfg *rcb_intr;
	struct queue_intr_cfg *intr_cfg;
	bool intr_en;
	uint16_t qid;

	printf("%s queue intr mapping info[tqp_num=%u display_all=%s]:\n",
	       g_queue_param.dir == NIC_RX_QUEUE ? "Rx" : "Tx",
	       map->tqp_num, g_queue_param.is_display_all ? "On" : "Off");
	for (qid = 0; qid < map->tqp_num; qid++) {
		if (qid >= HIKP_NIC_MAX_QUEUE_NUM) {
			HIKP_ERROR_PRINT("The cmd data is truncated.\n");
			break;
		}
		intr_cfg = &map->intr_cfg[qid];
		rcb_intr = &intr_cfg->rcb_intr;
		intr_en = intr_cfg->tqp_intr_en && rcb_intr->intr_dis == 0;
		if (intr_en || g_queue_param.is_display_all) {
			printf("  qid-%u: vec_id=%u gl_cfg=%u intr_dis:%s "
			       "intr_coal:%s que_intr:%s\n", qid, rcb_intr->intr_vector_id,
			       rcb_intr->int_gl_idx, rcb_intr->intr_dis ? "disable" : "enable",
			       intr_cfg->tqp_intr_en ? "enable" : "disable",
			       intr_en ? "enable" : "disable");
		}
	}
}

static void hikp_nic_queue_show_func_map(const void *data)
{
	struct nic_queue_func_map *q_func_map = (struct nic_queue_func_map *)data;
	struct tqp_func_map *map;
	uint16_t qid;

	printf("TQP function mapping info:\n");
	printf("  local_qid        |        global_qid\n");
	for (qid = 0; qid < q_func_map->tqp_num; qid++) {
		if (qid >= HIKP_NIC_MAX_QUEUE_NUM) {
			HIKP_ERROR_PRINT("The cmd data is truncated.\n");
			break;
		}

		map = &q_func_map->map[qid];
		printf("  %4u             |        %4u\n", map->q_id, map->abs_q_id);
	}
}

static void hikp_nic_queue_req_para_init(struct nic_queue_req_para *req_data,
					 const struct bdf_t *bdf,
					 const struct nic_queue_param *queue_param)
{
	const struct queue_feature_cmd *queue_cmd = &g_queue_feature_cmd[queue_param->feature_idx];

	req_data->bdf = *bdf;

	switch (queue_cmd->sub_cmd_code) {
	case QUEUE_BASIC_INFO:
		req_data->is_rx = queue_param->dir == NIC_RX_QUEUE ? 1 : 0;
		req_data->q_id = (uint16_t)queue_param->qid;
		break;
	case QUEUE_EN_INFO:
		break;
	case QUEUE_INTR_MAP:
		req_data->is_rx = queue_param->dir == NIC_RX_QUEUE ? 1 : 0;
		break;
	case QUEUE_FUNC_MAP:
	default:
		break;
	}
}

static int hikp_nic_queue_get_blk(struct hikp_cmd_header *req_header,
				  const struct nic_queue_req_para *req_data,
				  void *buf, size_t buf_len, struct nic_queue_rsp_head *rsp_head)
{
	struct hikp_cmd_ret *cmd_ret;
	struct nic_queue_rsp *rsp;
	int ret = 0;

	cmd_ret = hikp_cmd_alloc(req_header, req_data, sizeof(*req_data));
	ret = hikp_rsp_normal_check(cmd_ret);
	if (ret) {
		HIKP_ERROR_PRINT("failed to get block-%u context.\n", req_data->block_id);
		goto out;
	}

	rsp = (struct nic_queue_rsp *)cmd_ret->rsp_data;
	if (rsp->rsp_head.cur_blk_size > buf_len) {
		HIKP_ERROR_PRINT("nic_queue block-%u copy size error, "
				 "buffer size=%u, data size=%u.\n",
				 req_data->block_id, buf_len, rsp->rsp_head.cur_blk_size);
		ret = -EINVAL;
		goto out;
	}
	memcpy(buf, rsp->rsp_data, rsp->rsp_head.cur_blk_size);
	rsp_head->total_blk_num = rsp->rsp_head.total_blk_num;
	rsp_head->cur_blk_size = rsp->rsp_head.cur_blk_size;

out:
	free(cmd_ret);
	return ret;
}

static int hikp_nic_query_queue_feature(struct hikp_cmd_header *req_header, const struct bdf_t *bdf,
					union nic_queue_feature_info *data)
{
	struct nic_queue_rsp_head rsp_head = {0};
	struct nic_queue_req_para req_data;
	size_t buf_len = sizeof(*data);
	uint32_t total_blk_size;
	uint8_t total_blk_num;
	uint8_t blk_id = 0;
	int ret;

	hikp_nic_queue_req_para_init(&req_data, bdf, &g_queue_param);

	req_data.block_id = blk_id;
	ret = hikp_nic_queue_get_blk(req_header, &req_data, data, buf_len, &rsp_head);
	if (ret != 0)
		return ret;

	total_blk_num = rsp_head.total_blk_num;
	total_blk_size = rsp_head.cur_blk_size;

	/* Copy the remaining block content if total block number is greater than 1. */
	for (blk_id = 1; blk_id < total_blk_num; blk_id++) {
		req_data.block_id = blk_id;
		ret = hikp_nic_queue_get_blk(req_header, &req_data,
					     (uint8_t *)data + total_blk_size,
					     buf_len - total_blk_size, &rsp_head);
		if (ret != 0)
			return ret;
		total_blk_size += rsp_head.cur_blk_size;
	}

	return ret;
}

static bool hikp_nic_queue_check_feature_para_vaild(const struct queue_feature_cmd *cmd)
{
	bool valid = true;

	switch (cmd->sub_cmd_code) {
	case QUEUE_BASIC_INFO:
		if (g_queue_param.qid == -1 ||
		    g_queue_param.dir == NIC_QUEUE_DIR_UNKNOWN) {
			HIKP_ERROR_PRINT("please select rx or tx and qid "
					 "by '-d/--dir' and '-q/--qid'.\n");
			valid = false;
		}
		break;
	case QUEUE_EN_INFO:
	case QUEUE_FUNC_MAP:
		if (g_queue_param.qid != -1 ||
		    g_queue_param.dir != NIC_QUEUE_DIR_UNKNOWN) {
			HIKP_ERROR_PRINT("%s sub cmd no need '-q/--qid' and '-d/--dir'.\n",
					 cmd->feature_name);
			valid = false;
		}
		break;
	case QUEUE_INTR_MAP:
		if (g_queue_param.dir == NIC_QUEUE_DIR_UNKNOWN) {
			HIKP_ERROR_PRINT("please select rx or tx by '-d/--dir'.\n");
			valid = false;
		}
		if (g_queue_param.qid != -1) {
			HIKP_ERROR_PRINT("%s sub cmd no need -q/--qid.\n", cmd->feature_name);
			valid = false;
		}
		break;
	default:
		HIKP_ERROR_PRINT("unknown feature parameter.\n");
		valid = false;
		break;
	}

	return valid;
}

static void hikp_nic_queue_cmd_execute(struct major_cmd_ctrl *self)
{
	struct bdf_t *bdf = &g_queue_param.target.bdf;
	const struct queue_feature_cmd *queue_cmd;
	union nic_queue_feature_info *queue_data;
	struct hikp_cmd_header req_header = {0};
	int ret;

	if (g_queue_param.feature_idx == -1) {
		hikp_nic_queue_cmd_help(self, NULL);
		snprintf(self->err_str, sizeof(self->err_str), "-du/--dump param error!");
		self->err_no = -EINVAL;
		return;
	}

	queue_cmd = &g_queue_feature_cmd[g_queue_param.feature_idx];
	if (!hikp_nic_queue_check_feature_para_vaild(queue_cmd)) {
		hikp_nic_queue_cmd_help(self, NULL);
		snprintf(self->err_str, sizeof(self->err_str), "option parameters error!");
		self->err_no = -EINVAL;
		return;
	}

	queue_data = (union nic_queue_feature_info *)calloc(1,
		     sizeof(union nic_queue_feature_info));
	if (queue_data == NULL) {
		snprintf(self->err_str, sizeof(self->err_str),
			 "failed to allocate queue_data memory!");
		self->err_no = -ENOMEM;
		return;
	}
	hikp_cmd_init(&req_header, NIC_MOD, GET_QUEUE_INFO_CMD, queue_cmd->sub_cmd_code);
	ret = hikp_nic_query_queue_feature(&req_header, &g_queue_param.target.bdf, queue_data);
	if (ret != 0) {
		snprintf(self->err_str, sizeof(self->err_str), "failed to query %s, ret = %d.",
			 queue_cmd->feature_name, ret);
		self->err_no = ret;
		goto out;
	}

	printf("############## NIC Queue: %s info ############\n", queue_cmd->feature_name);
	queue_cmd->show(queue_data);
	printf("#################### END #######################\n");

out:
	free(queue_data);
}

static int hikp_nic_cmd_get_queue_target(struct major_cmd_ctrl *self, const char *argv)
{
	self->err_no = tool_check_and_get_valid_bdf_id(argv, &(g_queue_param.target));
	if (self->err_no != 0) {
		snprintf(self->err_str, sizeof(self->err_str), "unknown device!");
		return self->err_no;
	}

	return 0;
}

static int hikp_nic_cmd_queue_select_dir(struct major_cmd_ctrl *self, const char *argv)
{
	if (strcmp(argv, "rx") == 0) {
		g_queue_param.dir = NIC_RX_QUEUE;
		return 0;
	} else if (strcmp(argv, "tx") == 0) {
		g_queue_param.dir = NIC_TX_QUEUE;
		return 0;
	}

	snprintf(self->err_str, sizeof(self->err_str), "--d/--dir option is incorrect.");
	self->err_no = -EINVAL;

	return self->err_no;
}

static int hikp_nic_cmd_queue_get_qid(struct major_cmd_ctrl *self, const char *argv)
{
	uint32_t qid;

	self->err_no = string_toui(argv, &qid);
	if (self->err_no) {
		snprintf(self->err_str, sizeof(self->err_str), "parse qid failed.");
		return self->err_no;
	}

	if (qid >= HIKP_NIC_MAX_QUEUE_NUM) {
		snprintf(self->err_str, sizeof(self->err_str), "the qid should be less than %u.",
			 HIKP_NIC_MAX_QUEUE_NUM);
		self->err_no = -EINVAL;
		return self->err_no;
	}

	g_queue_param.qid = (uint16_t)qid;

	return 0;
}

static int hikp_nic_cmd_queue_get_all_switch(struct major_cmd_ctrl *self, const char *argv)
{
	if (strcmp(argv, "on") == 0) {
		g_queue_param.is_display_all = true;
		return 0;
	} else if (strcmp(argv, "off") == 0) {
		g_queue_param.is_display_all = false;
		return 0;
	}

	snprintf(self->err_str, sizeof(self->err_str), "parse -a/--all option failed.");
	self->err_no = -EINVAL;

	return self->err_no;
}

static int hikp_nic_cmd_queue_feature_select(struct major_cmd_ctrl *self, const char *argv)
{
	size_t feat_size = HIKP_ARRAY_SIZE(g_queue_feature_cmd);
	size_t i;

	for (i = 0; i < feat_size; i++) {
		if (strcmp(argv, g_queue_feature_cmd[i].feature_name) == 0) {
			g_queue_param.feature_idx = i;
			return 0;
		}
	}

	hikp_nic_queue_cmd_help(self, NULL);
	snprintf(self->err_str, sizeof(self->err_str),
		 "please specify the subfunction to be queried.");
	self->err_no = -EINVAL;

	return self->err_no;
}

static void cmd_nic_get_queue_init(void)
{
	struct major_cmd_ctrl *major_cmd = get_major_cmd();

	g_queue_param.feature_idx = -1;
	g_queue_param.qid = -1;
	g_queue_param.dir = NIC_QUEUE_DIR_UNKNOWN;
	g_queue_param.is_display_all = false;

	major_cmd->option_count = 0;
	major_cmd->execute = hikp_nic_queue_cmd_execute;

	cmd_option_register("-h", "--help", false, hikp_nic_queue_cmd_help);
	cmd_option_register("-i", "--interface", true, hikp_nic_cmd_get_queue_target);
	cmd_option_register("-du", "--dump", true, hikp_nic_cmd_queue_feature_select);
	cmd_option_register("-d", "--dir", true, hikp_nic_cmd_queue_select_dir);
	cmd_option_register("-q", "--qid", true, hikp_nic_cmd_queue_get_qid);
	cmd_option_register("-a", "--all", true, hikp_nic_cmd_queue_get_all_switch);
}

HIKP_CMD_DECLARE("nic_queue", "dump queue info of nic!", cmd_nic_get_queue_init);
