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
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "tool_cmd.h"
#include "hikp_net_lib.h"
#include "op_logs.h"
#include "hikp_nic_log.h"

static struct log_param g_log_param = { 0 };

static int hikp_nic_cmd_log_help(struct major_cmd_ctrl *self, const char *argv)
{
	printf("\n  Usage: %s %s\n", self->cmd_ptr->name, "-i <interface>\n");
	printf("\n         %s\n", self->cmd_ptr->help_info);
	printf("  Options:\n\n");
	printf("    %s, %-25s %s\n", "-h", "--help", "display this help and exit");
	printf("    %s, %-25s %s\n", "-i", "--interface=<interface>",
	       "device target or bdf id, e.g. eth0~7 or 0000:35:00.0");

	return 0;
}

static int hikp_nic_cmd_log_target(struct major_cmd_ctrl *self, const char *argv)
{
	self->err_no = tool_check_and_get_valid_bdf_id(argv, &(g_log_param.target));
	if (self->err_no != 0) {
		snprintf(self->err_str, sizeof(self->err_str), "Unknown device %s.", argv);
		return self->err_no;
	}

	return 0;
}

static int hikp_nic_write_data_to_file(uint8_t *data, uint32_t len)
{
	uint8_t file_path[OP_LOG_FILE_PATH_MAXLEN] = { 0 };
	uint8_t file_name[MAX_LOG_NAME_LEN] = { 0 };
	size_t write_cnt;
	FILE *fp = NULL;
	int ret;

	ret = generate_file_name(file_name, MAX_LOG_NAME_LEN, (const unsigned char *)"m7");
	if (ret < 0)
		return ret;

	ret = snprintf((char *)file_path, sizeof(file_path), HIKP_LOG_DIR_PATH"%s", file_name);
	if (ret < 0) {
		HIKP_ERROR_PRINT("creat log file path fail.\n");
		return -EIO;
	}
	(void)remove((const char *)file_path);
	fp = fopen((char *)file_path, "w+");
	if (fp == NULL) {
		HIKP_ERROR_PRINT("open %s failed, errno is %d\n", file_path, errno);
		return -errno;
	}
	write_cnt = fwrite(data, 1, len, fp);
	if (write_cnt != len)
		HIKP_ERROR_PRINT("write %s failed, write cnt %lu.\n", file_path, write_cnt);

	printf("dump m7 log completed, log file: %s.\n", file_path);
	(void)chmod((char *)file_path, 0440);
	(void)fclose(fp);
	return 0;
}

static int hikp_nic_get_blk_log(struct hikp_cmd_ret **cmd_ret, uint32_t blk_id)
{
	struct nic_log_req_para req_data = { 0 };
	struct hikp_cmd_header req_header = { 0 };

	req_data.bdf = g_log_param.target.bdf;
	req_data.block_id = blk_id;
	hikp_cmd_init(&req_header, NIC_MOD, GET_FW_LOG_INFO_CMD, FW_LOG_DUMP);
	*cmd_ret = hikp_cmd_alloc(&req_header, &req_data, sizeof(req_data));
	return hikp_rsp_normal_check(*cmd_ret);
}

static int hikp_nic_get_first_blk_info(uint32_t *total_blk_num,
				       uint32_t *cur_blk_size, uint8_t **log_data)
{
	struct nic_log_rsp_data *log_rsp = NULL;
	struct hikp_cmd_ret *cmd_ret = NULL;
	uint32_t log_size;
	int ret;

	ret = hikp_nic_get_blk_log(&cmd_ret, 0);
	if (ret < 0)
		goto err_out;

	log_rsp = (struct nic_log_rsp_data *)(cmd_ret->rsp_data);
	log_size = (uint32_t)(log_rsp->total_blk_num * MAX_LOG_DATA_NUM * sizeof(uint32_t));
	if (log_rsp->cur_blk_size == 0 ||
	    log_size < log_rsp->cur_blk_size ||
	    log_rsp->cur_blk_size > sizeof(log_rsp->log_data)) {
		HIKP_ERROR_PRINT("log size must bigger than current block size.\n");
		ret = -EINVAL;
		goto err_out;
	}
	*log_data = (uint8_t *)calloc(1, log_size);
	if (*log_data == NULL) {
		HIKP_ERROR_PRINT("calloc log memory 0x%x failed.", log_size);
		ret = -ENOMEM;
		goto err_out;
	}

	*total_blk_num = (uint32_t)log_rsp->total_blk_num;
	*cur_blk_size = (uint32_t)log_rsp->cur_blk_size;
	memcpy(*log_data, log_rsp->log_data, log_rsp->cur_blk_size);
err_out:
	hikp_cmd_free(&cmd_ret);

	return ret;
}

static int hikp_nic_get_log_info(uint32_t blk_id, uint32_t *cur_blk_size, uint8_t *log_data,
				 uint32_t max_log_size, uint32_t *blk_num)
{
	struct nic_log_rsp_data *log_rsp = NULL;
	struct hikp_cmd_ret *cmd_ret = NULL;
	int ret;

	ret = hikp_nic_get_blk_log(&cmd_ret, blk_id);
	if (ret)
		goto err_out;

	log_rsp = (struct nic_log_rsp_data *)(cmd_ret->rsp_data);
	*cur_blk_size = (uint32_t)log_rsp->cur_blk_size;
	*blk_num = (uint32_t)log_rsp->total_blk_num;
	if (max_log_size < *cur_blk_size ||
	    *cur_blk_size > sizeof(log_rsp->log_data)) {
		HIKP_ERROR_PRINT("log size must bigger than current block(%u) size.\n", blk_id);
		ret = -EINVAL;
		goto err_out;
	}
	memcpy(log_data, log_rsp->log_data, log_rsp->cur_blk_size);
err_out:
	hikp_cmd_free(&cmd_ret);

	return ret;
}

static int hikp_nic_dump_m7_log(struct major_cmd_ctrl *self)
{
	uint32_t real_log_size = 0;
	uint32_t total_blk_num = 0;
	uint8_t *log_data = NULL;
	uint32_t cur_blk_size;
	uint32_t max_log_size;
	uint32_t blk_num;
	uint32_t i;

	self->err_no = hikp_nic_get_first_blk_info(&total_blk_num, &cur_blk_size, &log_data);
	if (self->err_no) {
		snprintf(self->err_str, sizeof(self->err_str), "get the first block log fail.");
		return self->err_no;
	}
	max_log_size = (uint32_t)(total_blk_num * MAX_LOG_DATA_NUM * sizeof(uint32_t) -
		       cur_blk_size);
	real_log_size += cur_blk_size;
	for (i = 1; i < total_blk_num; i++) {
		self->err_no = hikp_nic_get_log_info(i, &cur_blk_size, log_data + real_log_size,
						     max_log_size, &blk_num);
		if (self->err_no) {
			snprintf(self->err_str, sizeof(self->err_str),
				 "getting block%u log fail.", i);
			free(log_data);
			return self->err_no;
		}
		max_log_size -= cur_blk_size;
		real_log_size += cur_blk_size;
		if (blk_num == 0 || cur_blk_size == 0)
			break;
	}
	self->err_no = hikp_nic_write_data_to_file(log_data, real_log_size);
	free(log_data);

	return self->err_no;
}

static void hikp_nic_log_cmd_execute(struct major_cmd_ctrl *self)
{
	self->err_no = hikp_nic_dump_m7_log(self);
}

static void cmd_nic_log_init(void)
{
	struct major_cmd_ctrl *major_cmd = get_major_cmd();

	major_cmd->option_count = 0;
	major_cmd->execute = hikp_nic_log_cmd_execute;

	cmd_option_register("-h", "--help", false, hikp_nic_cmd_log_help);
	cmd_option_register("-i", "--interface", true, hikp_nic_cmd_log_target);
}

HIKP_CMD_DECLARE("nic_log", "dump m7 log info.", cmd_nic_log_init);
