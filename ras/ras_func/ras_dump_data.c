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
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include "hikptdev_plug.h"
#include "op_logs.h"
#include "ras_common.h"
#include "ras_dump_data.h"

static struct dfx_reg_dump_header header;

static int ras_get_data(uint32_t ras_cmd_type, struct ras_dump_req_para *req_data,
                        struct ras_rsp *ras_rsp_data)
{
	uint32_t i;
	struct hikp_cmd_ret *cmd_ret;
	struct hikp_cmd_header req_header;

	hikp_cmd_init(&req_header, RAS_MOD, RAS_DUMP, ras_cmd_type);
	cmd_ret = hikp_cmd_alloc(&req_header, req_data, RAS_REQ_DATA_LEN);
	if (cmd_ret == NULL || cmd_ret->status != 0 ||
			cmd_ret->rsp_data_num > HIKP_RSP_ALL_DATA_MAX) {
		printf("hikp_data_proc err\n");
		hikp_cmd_free(&cmd_ret);
		return -1;
	}

	ras_rsp_data->rsp_data_num = cmd_ret->rsp_data_num;
	for (i = 0; i < ras_rsp_data->rsp_data_num; i++) {
		ras_rsp_data->rsp_data[i] = cmd_ret->rsp_data[i];
	}

	hikp_cmd_free(&cmd_ret);
	return 0;
}

static void ras_print_time(struct file_seq *s)
{
	time_t time_seconds = time(0);
	struct tm timeinfo;

	(void)localtime_r(&time_seconds, &timeinfo);
	s->len += snprintf(s->buffer + s->len, s->buffer_size - s->len, "Time: %d-%d-%d %d:%d:%d\n",
		       timeinfo.tm_year + START_YEAR, timeinfo.tm_mon + 1, timeinfo.tm_mday,
		       timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
}

static int ras_parse_data(uint32_t *reg_save, uint32_t reg_num, uint32_t reg_off, struct file_seq *s)
{
	uint32_t i, j;
	uint32_t cycle;
	uint32_t reg_count, pkt_reg_num;
	uint32_t off = reg_off;

	pkt_reg_num = header.pkt_length / sizeof(uint32_t);
	cycle = reg_num / pkt_reg_num;
	if (!cycle)
		return -1;

	for (i = 0; i < cycle; i++) {
		if ((off + pkt_reg_num) > HIKP_RSP_ALL_DATA_MAX) {
			HIKP_ERROR_PRINT("off is %u, pkt_reg_num is %u,\
				reg_save index will exceed max reg_save length\n",
				off, pkt_reg_num);
			return -1;
		}

		ras_print_time(s);
		s->len += snprintf(s->buffer + s->len, s->buffer_size - s->len, "Socket: 0X%hhX\t",
				(reg_save[off + DFX_HEAD_INFO_DW0] >> DFX_HEAD_SKT_ID_OFF) & 0xff);
		s->len += snprintf(s->buffer + s->len, s->buffer_size - s->len, "DIE: 0X%hhX\t",
				(reg_save[off + DFX_HEAD_INFO_DW0] >> DFX_HEAD_DIE_ID_OFF) & 0xff);
		s->len += snprintf(s->buffer + s->len, s->buffer_size - s->len, "Module: 0X%hhX\t",
				(reg_save[off + DFX_HEAD_INFO_DW1] >> DFX_HEAD_MODULE_ID_OFF) & 0xff);
		s->len += snprintf(s->buffer + s->len, s->buffer_size - s->len, "Sub Module: 0X%hhX\t",
				(reg_save[off + DFX_HEAD_INFO_DW1] >> DFX_HEAD_SUBMODULE_ID_OFF) & 0xff);
		s->len += snprintf(s->buffer + s->len, s->buffer_size - s->len, "SequenceNum: 0X%hhX\t",
				(reg_save[off + DFX_HEAD_INFO_DW1] >> DFX_HEAD_SEQUENCE_NUM_OFF) & 0xff);
		s->len += snprintf(s->buffer + s->len, s->buffer_size - s->len, "Version: 0X%hhX\n",
				(reg_save[off + DFX_HEAD_INFO_DW0] >> DFX_HEAD_VERSION_OFF) & 0xff);
		s->len += snprintf(s->buffer + s->len, s->buffer_size - s->len,
				"----------------------- DFX REGISTER DUMP -----------------------\n");

		reg_count = (reg_save[off + DFX_HEAD_INFO_DW1] >> DFX_HEAD_REG_COUNT_OFF) & 0xff;
		if (!reg_count || reg_count > pkt_reg_num - DFX_REG_PACKET_HEAD_LEN) {
			HIKP_ERROR_PRINT("reg_count is %u, value is not within the reasonable range(1-%u).\n",
				reg_count, pkt_reg_num - DFX_REG_PACKET_HEAD_LEN);
			return -1;
		}

		for (j = 0; j < reg_count; j++)
			s->len += snprintf(s->buffer + s->len, s->buffer_size - s->len, "0X%X\n",
				reg_save[off + DFX_COMMON_MAIN_TEXT_BEGIN + j]);

		s->len += snprintf(s->buffer + s->len, s->buffer_size - s->len, "\n");
		off += pkt_reg_num;
	}

	return 0;
}

static int ras_generate_file_name(struct file_seq *s)
{
	time_t time_seconds = time(0);
	struct tm timeinfo;
	int ret;

	(void)localtime_r(&time_seconds, &timeinfo);
	ret = snprintf(s->file_name, MAX_LOG_NAME_LEN, "rasdfx_%d_%d_%d_%d_%d_%d.log",
		       timeinfo.tm_year + START_YEAR, timeinfo.tm_mon + 1, timeinfo.tm_mday,
		       timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
	if (ret < 0 || (uint32_t)ret >= MAX_LOG_NAME_LEN) {
		HIKP_ERROR_PRINT("generate file name failed, errno is %d\n", errno);
		return -errno;
	}

	return 0;
}

static int ras_store_data(struct file_seq *s)
{
	char file_path[OP_LOG_FILE_PATH_MAXLEN];
	size_t write_cnt;
	FILE *fp;
	int rc;

	rc = snprintf(file_path, sizeof(file_path), HIKP_LOG_DIR_PATH"%s", s->file_name);
	if (rc < 0) {
		HIKP_ERROR_PRINT("creat log file path fail.\n");
		return -EIO;
	}

	fp = fopen(file_path, "a");
	if (fp == NULL) {
		HIKP_ERROR_PRINT("open %s failed, errno is %d\n", file_path, errno);
		return -errno;
	}

	write_cnt = fwrite(s->buffer, 1, s->len, fp);
	if (write_cnt != (uint32_t)s->len) {
		fclose(fp);
		HIKP_ERROR_PRINT("write %s failed, write cnt %zu.\n", file_path, write_cnt);
		return -EAGAIN;
	}

	printf("dump imp log completed, log file: %s.\n", file_path);
	/* Set the file permission to 0440 */
	if (chmod(file_path, 0440))
		HIKP_ERROR_PRINT("chmod %s failed, errno is %d\n", file_path, errno);

	if (fclose(fp)) {
		HIKP_ERROR_PRINT("close %s failed, errno is %d\n", file_path, errno);
		return -errno;
	}

	s->len = 0;

	return 0;
}

static int file_seq_init(struct file_seq *s, uint32_t size)
{
	if (!size)
		return -1;

	s->buffer_size = size;
	s->len = 0;
	s->buffer = (char*)malloc(s->buffer_size);
	if (!s->buffer)
		return -1;

	return 0;
}

static void file_seq_destroy(struct file_seq *s)
{
	free(s->buffer);
	s->buffer = NULL;
}

static void ras_rsp_init(struct ras_rsp *ras_rsp_data)
{
	ras_rsp_data->first_pkt_begin = 0;
	ras_rsp_data->last_pkt_end = 0;
	ras_rsp_data->rsp_data_num = 0;
	ras_rsp_data->packet_buffer_len = 0;

	memset(ras_rsp_data->rsp_data, 0, sizeof(ras_rsp_data->rsp_data));
	memset(ras_rsp_data->packet_buffer, 0, sizeof(ras_rsp_data->packet_buffer));
}

static int parse_packet_buffer_data(struct ras_rsp *ras_rsp_data,
				uint32_t pkt_reg_num, struct file_seq *s)
{
	int ret;

	if (pkt_reg_num > MAX_DFX_PACKET_LEN) {
		HIKP_ERROR_PRINT("pkt_reg_num is %u, has exceeded max packet length\n", pkt_reg_num);
		return -1;
	}

	if (ras_rsp_data->packet_buffer_len) {
		uint32_t rest_pkt_length;

		rest_pkt_length = pkt_reg_num - ras_rsp_data->packet_buffer_len;
		memcpy(ras_rsp_data->packet_buffer + ras_rsp_data->packet_buffer_len,
			ras_rsp_data->rsp_data, rest_pkt_length);

		ras_rsp_data->first_pkt_begin = rest_pkt_length;
		ret = ras_parse_data(ras_rsp_data->packet_buffer, pkt_reg_num, 0, s);
		if (ret) {
			HIKP_ERROR_PRINT("ras parse packet_buffer_data is failed\n");
			return ret;
		}
	} else {
		ras_rsp_data->first_pkt_begin = 0;
	}

	if (ras_rsp_data->first_pkt_begin == ras_rsp_data->rsp_data_num)
		return 0;

	ras_rsp_data->packet_buffer_len =
		(ras_rsp_data->rsp_data_num - ras_rsp_data->first_pkt_begin) % pkt_reg_num;
	ras_rsp_data->last_pkt_end = ras_rsp_data->rsp_data_num - ras_rsp_data->packet_buffer_len - 1;
	ras_rsp_data->rsp_data_num = ras_rsp_data->last_pkt_end - ras_rsp_data->first_pkt_begin + 1;

	memcpy(ras_rsp_data->packet_buffer, ras_rsp_data->rsp_data + ras_rsp_data->last_pkt_end + 1,
		ras_rsp_data->packet_buffer_len);

	return 0;
}

static int ras_dump_pkt_pre(struct ras_rsp *ras_rsp_data, struct file_seq *s)
{
	int ret;
	uint32_t reg_num, max_pkt_num, s_buffer_size;

	max_pkt_num = (HIKP_RSP_DATA_SIZE_MAX / header.pkt_length) + 1;
	reg_num = header.pkt_length / sizeof(uint32_t) - DFX_REG_PACKET_HEAD_LEN;
	s_buffer_size = max_pkt_num *
		(reg_num * DFX_FILE_SINGLE_REG_SIZE + DFX_FILE_SINGLE_PACKET_HEAD_SIZE);
	ras_rsp_data->rsp_data_num = 0;

	ret = file_seq_init(s, s_buffer_size);
	if (ret) {
		HIKP_ERROR_PRINT("malloc file_seq buffer is failed\n");
		return ret;
	}

	ret = ras_generate_file_name(s);
	if (ret) {
		HIKP_ERROR_PRINT("ras generate file name is failed\n");
		file_seq_destroy(s);
		return ret;
	}

	return ret;
}

static int ras_dump_packet(struct tool_ras_cmd *cmd, struct ras_rsp *ras_rsp_data,
			   struct ras_dump_req_para *req_data)
{
	int ret;
	uint32_t i, cmd_num;
	bool has_printed_socid = false;
	struct file_seq s;

	ret = ras_dump_pkt_pre(ras_rsp_data, &s);
	if (ret)
		return ret;

	cmd_num = (header.pkt_num * header.pkt_length +
		HIKP_RSP_DATA_SIZE_MAX - 1) / HIKP_RSP_DATA_SIZE_MAX;
	/* 0: get header info;  1-n: get packet data */
	for (i = 0; i < cmd_num; i++) {
		req_data->cmd_id = i + 1;
		ret = ras_get_data(cmd->ras_cmd_type, req_data, ras_rsp_data);
		if (ret) {
			HIKP_ERROR_PRINT("ras dump cmd %u is failed\n", req_data->cmd_id);
			goto err_out_free;
		}

		if (!has_printed_socid) {
			s.len += snprintf(s.buffer + s.len, s.buffer_size - s.len, "SocID: %hhX\n",
			(ras_rsp_data->rsp_data[DFX_HEAD_INFO_DW0] >> DFX_HEAD_SOC_ID_OFF) & 0xff);
			s.len += snprintf(s.buffer + s.len, s.buffer_size - s.len, "\n");

			has_printed_socid = true;
		}

		ret = parse_packet_buffer_data(ras_rsp_data, header.pkt_length / sizeof(uint32_t), &s);
		if (ret) {
			HIKP_ERROR_PRINT("ras parse packet buffer data is failed\n");
			goto err_out_free;
		}

		if (ras_rsp_data->first_pkt_begin != ras_rsp_data->rsp_data_num) {
			ret = ras_parse_data(ras_rsp_data->rsp_data, ras_rsp_data->rsp_data_num,
				ras_rsp_data->first_pkt_begin, &s);
			if (ret) {
				HIKP_ERROR_PRINT("ras parse rsp_data is failed\n");
				goto err_out_free;
			}
		}

		ret = ras_store_data(&s);
		if (ret) {
			HIKP_ERROR_PRINT("ras store rsp_data is failed\n");
			goto err_out_free;
		}
	}

err_out_free:
	file_seq_destroy(&s);
	return ret;
}

int ras_data_dump(struct tool_ras_cmd *cmd)
{
	int ret;
	struct ras_rsp ras_rsp_data;
	struct ras_dump_req_para req_data = {0};

	if (cmd == NULL)
		return -ENOSPC;

	ras_rsp_init(&ras_rsp_data);
	ret = ras_get_data(cmd->ras_cmd_type, &req_data, &ras_rsp_data);
	if (ret || (ras_rsp_data.rsp_data_num != DFX_REG_DUMP_HEADER_LEN)) {
		HIKP_ERROR_PRINT("ras dump header is failed, rsp_data_num is %u\n",
			ras_rsp_data.rsp_data_num);
		return -1;
	}

	if (!ras_rsp_data.rsp_data[HEAD_MAGIC]) {
		HIKP_ERROR_PRINT("ras dfx dump is failed, data does not exist or has been cleared.\n");
		return -1;
	}

	header.pkt_num = ras_rsp_data.rsp_data[PKT_NUM];
	header.pkt_length = ras_rsp_data.rsp_data[PKT_LENGTH];
	if (header.pkt_num == 0 || header.pkt_length < DFX_REG_PACKET_HEAD_LEN) {
		HIKP_ERROR_PRINT("ras dfx dump is failed, pkt_num is %u, pkt_length is %u\n",
			header.pkt_num, header.pkt_length);
		return -1;
	}

	ret = ras_dump_packet(cmd, &ras_rsp_data, &req_data);
	if (ret)
		HIKP_ERROR_PRINT("ras dump packet is failed\n");

	return ret;
}

int ras_data_clear(struct tool_ras_cmd *cmd)
{
	int ret;
	struct ras_rsp ras_rsp_data;
	struct ras_dump_req_para req_data = { 0 };

	if (cmd == NULL)
		return -ENOSPC;

	ras_rsp_init(&ras_rsp_data);
	ret = ras_get_data(cmd->ras_cmd_type, &req_data, &ras_rsp_data);
	if (ret || ras_rsp_data.rsp_data_num != DFX_REG_DUMP_HEADER_LEN ||
		ras_rsp_data.rsp_data[HEAD_MAGIC] != DFX_DATA_IS_CLEARED) {
		HIKP_ERROR_PRINT("ras dfx data clear is failed\n");
		return -1;
	}

	return 0;
}

