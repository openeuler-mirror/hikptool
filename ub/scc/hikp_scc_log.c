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

#include "hikp_scc_log.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <inttypes.h>
#include "op_logs.h"
#include "hikptdev_plug.h"

static struct scc_information *g_scc_info = NULL;

static void hikp_scc_log_header_free(void)
{
	if (g_scc_info) {
		free(g_scc_info);
		g_scc_info = NULL;
	}
}

static int hikp_scc_log_header_alloc(uint8_t *data, uint32_t start_addr)
{
	struct scc_information *fixed_part = (struct scc_information *)(data + start_addr);

	if (fixed_part == NULL) {
		HIKP_ERROR_PRINT("get fixed part failed\n");
		return -EINVAL;
	}

	if (fixed_part->header_size < sizeof(struct scc_information)) {
		HIKP_ERROR_PRINT("invalid log header size, size %u\n", fixed_part->header_size);
		return -EINVAL;
	}

	if (fixed_part->log_size <= sizeof(struct scc_log_detail)) {
		HIKP_ERROR_PRINT("invalid log size, size %u\n", fixed_part->log_size);
		return -EINVAL;
	}

	if (fixed_part->head >= LOG_AREA_TOTAL_SIZE) {
		HIKP_ERROR_PRINT("invalid head position, position %u\n", fixed_part->head);
		return -EINVAL;
	}

	g_scc_info = (struct scc_information *)calloc(1, fixed_part->header_size);
	if (g_scc_info == NULL) {
		HIKP_ERROR_PRINT("calloc log header memory size 0x%x failed\n",
				 fixed_part->header_size);
		return -ENOMEM;
	}

	memcpy(g_scc_info, data + start_addr, fixed_part->header_size);

	return 0;
}

static void hikp_scc_parse_log_header(FILE *dst_fp)
{
	fprintf(dst_fp, "Firmware Version    : 0x%X\n", g_scc_info->scc_firmware_version);
	fprintf(dst_fp, "Magic Number        : 0x%X\n", g_scc_info->magic_num);
	fprintf(dst_fp, "Header Size         : %u\n", g_scc_info->header_size);
	fprintf(dst_fp, "Log Size            : %u\n", g_scc_info->log_size);
	fprintf(dst_fp, "Log Area Size       : %u\n", g_scc_info->log_area_size);
	fprintf(dst_fp, "Head                : %u\n", g_scc_info->head);
	fprintf(dst_fp, "Tail                : %u\n", g_scc_info->tail);
	fprintf(dst_fp, "Log Count           : %u\n\n", g_scc_info->log_cnt);
}

static int hikp_scc_parse_remain_log(uint8_t *data, FILE *dst_fp, uint32_t start_addr)
{
	struct scc_log_detail *scc_log = NULL;
	uint32_t current = g_scc_info->head;
	uint32_t log_area_start = start_addr + g_scc_info->header_size;
	uint32_t parsed_size = 0;
	uint16_t max_len = g_scc_info->log_size - sizeof(struct scc_log_detail);
	uint16_t i, len;

	if (g_scc_info->head == g_scc_info->tail)
		return 0;

	scc_log = (struct scc_log_detail *)calloc(1, g_scc_info->log_size);
	if (scc_log == NULL) {
		HIKP_ERROR_PRINT("calloc log body memory size 0x%x failed\n", g_scc_info->log_size);
		return -ENOMEM;
	}

	while (true) {
		if ((current == g_scc_info->tail))
			break;

		if ((parsed_size >= LOG_AREA_TOTAL_SIZE) || ((current + g_scc_info->header_size +
		   g_scc_info->log_size) >= LOG_AREA_TOTAL_SIZE)) {
			HIKP_WARN_PRINT("log area is invalid, please check the log header info\n");
			break;
		}

		memcpy(scc_log, data + log_area_start + current, g_scc_info->log_size);

		fprintf(dst_fp, "tpn: %u, timestamp: %u, type: 0x%X, len: %u, value: ",
			scc_log->tpn, scc_log->timestamp, scc_log->type, scc_log->len);

		if (scc_log->type == TYPE_STR) {
			scc_log->value[max_len - 1] = '\0';
			fprintf(dst_fp, "%s\n", scc_log->value);
		} else {
			fprintf(dst_fp, "0x ");
			len = scc_log->len < max_len ? scc_log->len : max_len;
			for (i = 0; i < len; i++)
				fprintf(dst_fp, "%02X ", scc_log->value[i]);
			fprintf(dst_fp, "\n");
		}

		parsed_size += g_scc_info->log_size;
		current += g_scc_info->log_size;
		if (current >= g_scc_info->log_area_size)
			current = 0;
	}

	free(scc_log);
	scc_log = NULL;

	return 0;
}

static int hikp_scc_parse_log_one_core(uint8_t *data, FILE *dst_fp, uint32_t start_addr,
				       uint32_t core_id)
{
	int ret = 0;

	ret = hikp_scc_log_header_alloc(data, start_addr);
	if (ret != 0)
		return ret;

	fprintf(dst_fp, "\n\n========= CORE %u =========\n\n", core_id);
	hikp_scc_parse_log_header(dst_fp);
	ret = hikp_scc_parse_remain_log(data, dst_fp, start_addr);

	hikp_scc_log_header_free();

	return ret;
}

static int hikp_scc_parse_log(uint8_t *data, uint32_t len, FILE *fp)
{
	int ret = 0;

	if (len != ((LOG_AREA_TOTAL_SIZE) * (MAX_CORE_NUM))) {
		HIKP_ERROR_PRINT("log file must be exactly 4MB\n");
		return -EINVAL;
	}

	ret = hikp_scc_parse_log_one_core(data, fp, 0, SCC_MCU_CORE0);
	if (ret != 0)
		return ret;

	return hikp_scc_parse_log_one_core(data, fp, LOG_AREA_TOTAL_SIZE, SCC_MCU_CORE1);
}

static int hikp_scc_write_data_to_file(uint8_t *data, uint32_t len)
{
	uint8_t file_path[OP_LOG_FILE_PATH_MAXLEN] = { 0 };
	uint8_t file_name[MAX_LOG_NAME_LEN] = { 0 };
	FILE *fp = NULL;
	int ret;
	int rc;

	ret = generate_file_name(file_name, MAX_LOG_NAME_LEN, (const unsigned char *)"scc");
	if (ret < 0)
		return ret;

	rc = snprintf((char *)file_path, sizeof(file_path), HIKP_LOG_DIR_PATH"%s", file_name);
	if (rc < 0) {
		HIKP_ERROR_PRINT("create log file path fail.\n");
		return -EIO;
	}

	if (access((const char *)file_path, F_OK) == 0) {
		if (remove((const char *)file_path)) {
			HIKP_ERROR_PRINT("remove %s failed, errno is %d\n", file_path, errno);
			return -errno;
		}
	}

	fp = fopen((char *)file_path, "w+");
	if (fp == NULL) {
		HIKP_ERROR_PRINT("open %s failed, errno is %d\n", file_path, errno);
		return -errno;
	}

	ret = hikp_scc_parse_log(data, len, fp);

	printf("dump scc log completed, log file: %s\n", file_path);
	/* Set the file permission to 0440 */
	if (chmod((char *)file_path, 0440))
		HIKP_ERROR_PRINT("chmod %s failed, errno is %d\n", file_path, errno);

	if (fclose(fp))
		HIKP_ERROR_PRINT("close %s failed, errno is %d\n", file_path, errno);

	return ret;
}

static int hikp_scc_log_get_data(struct hikp_cmd_ret **cmd_ret, uint32_t blk_id,
				 struct scc_cmd_cfg *cmd_cfg)
{
	struct hikp_cmd_header req_header = {0};
	struct scc_log_req_para log_req = {0};

	log_req.chip = cmd_cfg->chip;
	log_req.die = cmd_cfg->die;
	log_req.block_id = blk_id;
	hikp_cmd_init(&req_header, SCC_MOD, SCC_LOG_CMD, SCC_LOG_DUMP);
	*cmd_ret = hikp_cmd_alloc(&req_header, &log_req, sizeof(log_req));
	return hikp_rsp_normal_check(*cmd_ret);
}

static int hikp_scc_get_first_blk(struct scc_log_blk_ctrl *blk_ctrl, uint8_t **log_data,
				  struct scc_cmd_cfg *cmd_cfg)
{
	struct scc_log_rsp_data *log_resp = NULL;
	struct hikp_cmd_ret *cmd_ret = NULL;
	uint32_t log_size;
	int ret;

	ret = hikp_scc_log_get_data(&cmd_ret, 0, cmd_cfg);
	if (ret)
		goto err_out;

	log_resp = (struct scc_log_rsp_data *)(cmd_ret->rsp_data);
	log_size = log_resp->total_blk_num * LOG_DATA_BLK_SIZE;
	if (log_resp->cur_blk_size == 0 ||
	    log_size < log_resp->cur_blk_size ||
	    log_resp->cur_blk_size > sizeof(log_resp->log_data)) {
		HIKP_ERROR_PRINT("log size(%u) must bigger than current block size(%hu).\n",
				 log_size, log_resp->cur_blk_size);
		ret = -EINVAL;
		goto err_out;
	}

	*log_data = (uint8_t *)calloc(1, log_size);
	if (*log_data == NULL) {
		HIKP_ERROR_PRINT("calloc log memory size 0x%x failed.\n", log_size);
		ret = -ENOMEM;
		goto err_out;
	}

	blk_ctrl->total_blk_num = log_resp->total_blk_num;
	blk_ctrl->resp_blk_size = log_resp->cur_blk_size;
	memcpy(*log_data, log_resp->log_data, log_resp->cur_blk_size);

err_out:
	hikp_cmd_free(&cmd_ret);

	return ret;
}

static int hikp_scc_get_remain_log(struct scc_log_blk_ctrl *blk_ctrl,
				   uint8_t *log_data, uint32_t data_len,
				   struct scc_cmd_cfg *cmd_cfg)
{
	struct scc_log_rsp_data *log_resp = NULL;
	struct hikp_cmd_ret *cmd_ret = NULL;
	int ret;

	ret = hikp_scc_log_get_data(&cmd_ret, blk_ctrl->blk_id, cmd_cfg);
	if (ret)
		goto err_out;

	log_resp = (struct scc_log_rsp_data *)(cmd_ret->rsp_data);
	blk_ctrl->resp_blk_size = log_resp->cur_blk_size;
	blk_ctrl->total_blk_num = log_resp->total_blk_num;
	if (log_resp->cur_blk_size == 0 || log_resp->cur_blk_size > data_len ||
	    log_resp->cur_blk_size > sizeof(log_resp->log_data)) {
		HIKP_ERROR_PRINT("blk%u data size(0x%x) bigger than remain data(0x%x) size.\n",
				 blk_ctrl->blk_id, log_resp->cur_blk_size, data_len);
		ret = -EINVAL;
		goto err_out;
	}
	memcpy(log_data, log_resp->log_data, log_resp->cur_blk_size);

err_out:
	hikp_cmd_free(&cmd_ret);

	return ret;
}

void hikp_scc_dump_log(struct major_cmd_ctrl *self, struct scc_cmd_cfg *cmd_cfg)
{
	struct scc_log_blk_ctrl blk_ctrl = {};
	uint8_t *log_data = NULL;
	uint32_t log_size = 0;
	uint32_t remain_size;

	self->err_no = hikp_scc_get_first_blk(&blk_ctrl, &log_data, cmd_cfg);
	if (self->err_no) {
		snprintf(self->err_str, sizeof(self->err_str), "get first block data failed.");
		return;
	}

	remain_size = blk_ctrl.total_blk_num * LOG_DATA_BLK_SIZE - blk_ctrl.resp_blk_size;
	log_size += blk_ctrl.resp_blk_size;
	for (uint32_t i = 1; i < blk_ctrl.total_blk_num; i++) {
		blk_ctrl.blk_id = i;
		self->err_no = hikp_scc_get_remain_log(&blk_ctrl, log_data + log_size,
						       remain_size, cmd_cfg);
		if (self->err_no) {
			snprintf(self->err_str, sizeof(self->err_str),
				 "getting block%u log data failed.", i);
			free(log_data);
			return;
		}

		remain_size -= blk_ctrl.resp_blk_size;
		log_size += blk_ctrl.resp_blk_size;

		if (blk_ctrl.total_blk_num == 0 || blk_ctrl.resp_blk_size == 0)
			break;
		/* Wait 1000 us to allow the CPU interrupt to be scheduled */
		usleep(1000);
	}

	self->err_no = hikp_scc_write_data_to_file(log_data, log_size);

	free(log_data);
}
