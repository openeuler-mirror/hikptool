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

#include "hikp_imp_log.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <inttypes.h>
#include "ossl_user_linux.h"
#include "hikptdev_plug.h"
#include "op_logs.h"

static int hikp_imp_log_write_to_file(uint8_t *data, uint32_t len)
{
	uint8_t file_path[OP_LOG_FILE_PATH_MAXLEN] = {0};
	uint8_t file_name[MAX_LOG_NAME_LEN] = {0};
	size_t write_cnt;
	FILE *fp = NULL;
	int ret;
	int rc;

	ret = generate_file_name(file_name, MAX_LOG_NAME_LEN, (const unsigned char *)"imp");
	if (ret < 0)
		return ret;

	rc = snprintf((char *)file_path, sizeof(file_path), HIKP_LOG_DIR_PATH"%s", file_name);
	if (rc < 0) {
		HIKP_ERROR_PRINT("creat log file path fail.\n");
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

	write_cnt = fwrite(data, 1, len, fp);
	if (write_cnt != len) {
		HIKP_ERROR_PRINT("write %s failed, write cnt %zu.\n", file_path, write_cnt);
		ret = -EAGAIN;
	}

	printf("dump imp log completed, log file: %s.\n", file_path);
	/* Set the file permission to 0440 */
	if (chmod((char *)file_path, 0440))
		HIKP_ERROR_PRINT("chmod %s failed, errno is %d\n", file_path, errno);

	if (fclose(fp))
		HIKP_ERROR_PRINT("close %s failed, errno is %d\n", file_path, errno);

	return ret;
}

static int hikp_imp_log_get_blk_data(struct hikp_cmd_ret **cmd_ret, uint32_t blk_id,
				     struct imp_cmd_cfg *cmd_cfg)
{
	struct hikp_cmd_header req_header = {0};
	struct imp_log_req_para log_req = {0};

	log_req.chip = cmd_cfg->chip;
	log_req.die = cmd_cfg->die;
	log_req.block_id = blk_id;
	hikp_cmd_init(&req_header, IMP_MOD, IMP_MOD_DUMP_LOG_CMD, DUMP_LOG_DATA);
	*cmd_ret = hikp_cmd_alloc(&req_header, &log_req, sizeof(log_req));
	return hikp_rsp_normal_check(*cmd_ret);
}

static int hikp_imp_log_get_first_blk(struct imp_log_blk_ctrl *blk_ctrl,
				      uint8_t **log_data, struct imp_cmd_cfg *cmd_cfg)
{
	struct imp_log_rsp_data *log_rsp = NULL;
	struct hikp_cmd_ret *cmd_ret = NULL;
	uint32_t log_size = 0;
	int ret;

	ret = hikp_imp_log_get_blk_data(&cmd_ret, 0, cmd_cfg);
	if (ret)
		goto err_out;

	log_rsp = (struct imp_log_rsp_data *)(cmd_ret->rsp_data);
	log_size = log_rsp->total_blk_num * LOG_DATA_BLK_SIZE;
	if (log_rsp->cur_blk_size == 0 ||
	    log_size < log_rsp->cur_blk_size ||
	    log_rsp->cur_blk_size > sizeof(log_rsp->log_data)) {
		HIKP_ERROR_PRINT("log size must bigger than current block size.\n");
		ret = -EINVAL;
		goto err_out;
	}

	*log_data = (uint8_t *)calloc(1, log_size);
	if (*log_data == NULL) {
		HIKP_ERROR_PRINT("calloc log memory size 0x%x failed.\n", log_size);
		ret = -ENOMEM;
		goto err_out;
	}

	blk_ctrl->total_blk_num = log_rsp->total_blk_num;
	blk_ctrl->resp_blk_size = log_rsp->cur_blk_size;
	memcpy(*log_data, log_rsp->log_data, log_rsp->cur_blk_size);
err_out:
	hikp_cmd_free(&cmd_ret);

	return ret;
}

static int hikp_imp_log_get_remain_data(struct imp_log_blk_ctrl *blk_ctrl,
					uint8_t *log_data, uint32_t data_len,
					struct imp_cmd_cfg *cmd_cfg)
{
	struct imp_log_rsp_data *log_rsp = NULL;
	struct hikp_cmd_ret *cmd_ret = NULL;
	int ret;

	ret = hikp_imp_log_get_blk_data(&cmd_ret, blk_ctrl->blk_id, cmd_cfg);
	if (ret)
		goto err_out;

	log_rsp = (struct imp_log_rsp_data *)(cmd_ret->rsp_data);
	blk_ctrl->resp_blk_size = log_rsp->cur_blk_size;
	blk_ctrl->total_blk_num = log_rsp->total_blk_num;
	if (log_rsp->cur_blk_size > data_len ||
	    log_rsp->cur_blk_size > sizeof(log_rsp->log_data)) {
		HIKP_ERROR_PRINT("blk%u data size(0x%x) bigger than remain data(0x%x) size.\n",
				 blk_ctrl->blk_id, log_rsp->cur_blk_size, data_len);
		ret = -EINVAL;
		goto err_out;
	}
	memcpy(log_data, log_rsp->log_data, log_rsp->cur_blk_size);

err_out:
	hikp_cmd_free(&cmd_ret);

	return ret;
}

void hikp_imp_dump_log(struct major_cmd_ctrl *self, struct imp_cmd_cfg *cmd_cfg)
{
	struct imp_log_blk_ctrl blk_ctrl = {};
	uint32_t log_size = 0;
	uint8_t *log_data = NULL;
	uint32_t remain_size;
	uint32_t i;

	self->err_no = hikp_imp_log_get_first_blk(&blk_ctrl, &log_data, cmd_cfg);
	if (self->err_no) {
		snprintf(self->err_str, sizeof(self->err_str), "get the first block log fail.");
		return;
	}

	remain_size = blk_ctrl.total_blk_num * LOG_DATA_BLK_SIZE - blk_ctrl.resp_blk_size;
	log_size += blk_ctrl.resp_blk_size;
	for (i = 1; i < blk_ctrl.total_blk_num; i++) {
		blk_ctrl.blk_id = i;
		self->err_no = hikp_imp_log_get_remain_data(&blk_ctrl, log_data + log_size,
							    remain_size, cmd_cfg);
		if (self->err_no) {
			snprintf(self->err_str, sizeof(self->err_str),
				 "getting block%u log fail.", i);
			free(log_data);
			return;
		}

		remain_size -= blk_ctrl.resp_blk_size;
		log_size += blk_ctrl.resp_blk_size;

		if (blk_ctrl.total_blk_num == 0 || blk_ctrl.resp_blk_size == 0)
			break;
	}

	self->err_no = hikp_imp_log_write_to_file(log_data, log_size);
	free(log_data);
}
