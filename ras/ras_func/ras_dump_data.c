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
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include "hikptdev_plug.h"
#include "op_logs.h"
#include "hikpt_rciep.h"
#include "tool_lib.h"
#include "ras_dump_data.h"

static void __attribute__((format(printf, 2, 3))) __THROWNL
rasdfx_wr2buf(struct file_seq *s, const char *fmt, ...);

#define RAS_DUMP	0

#define RASDFX_FILE_HEADER_LEN		6
#define MAX_DFX_PACKET_LEN		256
#define DFX_REG_PACKET_HEAD_LEN		3U

#define DFX_DATA_CLEARED_MAGIC		0
#define DFX_DATA_DUMPED_MAGIC		0x5aa5a55a

#define RASDFX_PACKET_HEAD_SIZE		256
#define RASDFX_PACKET_SIZE(reg_num)	(RASDFX_PACKET_HEAD_SIZE + (reg_num) * 10)
#define RASDFX_PACKET_NUM_MAX		1000000

static void __THROWNL rasdfx_wr2buf(struct file_seq *s, const char *fmt, ...)
{
	size_t size = s->buf_size - s->buf_offs;
	va_list argp;
	int len;

	va_start(argp, fmt);
	len = vsnprintf(s->buf + s->buf_offs, size, fmt, argp);
	va_end(argp);

	if (len < 0 || (size_t)len >= size)
		HIKP_WARN_PRINT("rasdfx_wr2buf failed, the dfx data is incomplete\n");
	else
		s->buf_offs += (size_t)len;
}

static struct hikp_cmd_ret *ras_get_rsp_data(struct ras_dump_cmd *cmd)
{
	struct hikp_cmd_header req_header;
	struct hikp_cmd_ret *cmd_ret;

	hikp_cmd_init(&req_header, RAS_MOD, RAS_DUMP, cmd->cmd_type);
	cmd_ret = hikp_cmd_alloc(&req_header, &cmd->cmd_id, sizeof(cmd->cmd_id));
	if (!cmd_ret) {
		HIKP_ERROR_PRINT("alloc cmd failed, cmd: %u\n", cmd->cmd_id);
		return NULL;
	}

	if (cmd_ret->status) {
		HIKP_ERROR_PRINT("hikp_data_proc err, status: %u\n", cmd_ret->status);
		hikp_cmd_free(&cmd_ret);
		return NULL;
	}

	return cmd_ret;
}

static int ras_get_rasdfx_header(struct ras_dump_cmd *cmd, struct rasdfx_file_header *f_header)
{
	struct hikp_cmd_ret *cmd_ret;

	cmd->cmd_id = 0;
	cmd_ret = ras_get_rsp_data(cmd);
	if (!cmd_ret)
		return -ENOMEM;

	if (cmd_ret->rsp_data_num != RASDFX_FILE_HEADER_LEN) {
		HIKP_ERROR_PRINT("invalid number of response data: %u\n", cmd_ret->rsp_data_num);
		hikp_cmd_free(&cmd_ret);
		return -1;
	}

	memcpy(f_header, cmd_ret->rsp_data, sizeof(struct rasdfx_file_header));
	hikp_cmd_free(&cmd_ret);

	return 0;
}

static bool ras_check_header(struct rasdfx_file_header *f_header)
{
	if (f_header->pkt_size_dwords % REP_DATA_BLK_SIZE) {
		HIKP_ERROR_PRINT("packet size is not aligned: %u\n", f_header->pkt_size_dwords);
		return false;
	}

	/* Converted to DWORD units to simplify subsequent calculations */
	f_header->pkt_size_dwords = f_header->pkt_size_dwords / REP_DATA_BLK_SIZE;
	if (f_header->pkt_size_dwords < DFX_REG_PACKET_HEAD_LEN ||
	    f_header->pkt_size_dwords > MAX_DFX_PACKET_LEN) {
		HIKP_ERROR_PRINT("packet size is out of bounds: %u\n", f_header->pkt_size_dwords);
		return false;
	}

	if (f_header->pkt_num == 0 || f_header->pkt_num > RASDFX_PACKET_NUM_MAX) {
		HIKP_ERROR_PRINT("packet number is out of bounds: %u\n", f_header->pkt_num);
		return false;
	}

	return true;
}

static int ras_open_rasdfx_file_seq(struct file_seq *s, struct rasdfx_file_header *f_header)
{
	char file_path[OP_LOG_FILE_PATH_MAXLEN];
	time_t time_seconds = time(0);
	struct tm timeinfo;

	s->buf_offs = 0;
	s->buf_size = RASDFX_PACKET_SIZE(f_header->pkt_size_dwords);
	s->buf = (char *)malloc(s->buf_size);
	if (!s->buf) {
		HIKP_ERROR_PRINT("malloc file_seq buffer is failed\n");
		return -ENOMEM;
	}

	(void)localtime_r(&time_seconds, &timeinfo);
	snprintf(file_path, sizeof(file_path), "%srasdfx_%04d_%02d_%02d_%02d_%02d_%02d.log",
		 HIKP_LOG_DIR_PATH, timeinfo.tm_year + START_YEAR, timeinfo.tm_mon + 1,
		 timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);

	// creat and open file, set file permissiion 0440
	s->fd = open(file_path, O_WRONLY | O_CREAT, S_IRUSR | S_IRGRP);
	if (s->fd < 0) {
		HIKP_ERROR_PRINT("open %s failed: %s\n", file_path, strerror(errno));
		free(s->buf);
		return -errno;
	}

	printf("dump dfx log start, log file: %s\n", file_path);
	return 0;
}

static void ras_close_rasdfx_file_seq(struct file_seq *s)
{
	(void)close(s->fd);
	free(s->buf);
}

static void ras_parse_rasdfx_pkt_header(struct file_seq *s, struct rasdfx_pkt *pkt)
{
	time_t time_seconds = time(0);
	struct tm timeinfo;

	(void)localtime_r(&time_seconds, &timeinfo);
	rasdfx_wr2buf(s, "Time: %d-%d-%d %d:%d:%d\n",
		      timeinfo.tm_year + START_YEAR, timeinfo.tm_mon + 1, timeinfo.tm_mday,
		      timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
	rasdfx_wr2buf(s, "Socket: 0X%hhX    ", pkt->dw0.skt_id);
	rasdfx_wr2buf(s, "DIE: 0X%hhX    ", pkt->dw0.die_id);
	rasdfx_wr2buf(s, "Module: 0X%hhX    ", pkt->dw1.module_id);
	rasdfx_wr2buf(s, "Sub Module: 0X%hhX    ", pkt->dw1.submodule_id);
	rasdfx_wr2buf(s, "SequenceNum: 0X%hhX    ", pkt->dw1.sequence_num);
	rasdfx_wr2buf(s, "Version: 0X%hhX\n", pkt->dw0.version);
	rasdfx_wr2buf(s, "----------------------- DFX REGISTER DUMP -----------------------\n");
}

static int ras_parse_rasdfx_payload(struct file_seq *s, uint32_t *buf,
				    struct rasdfx_file_header *f_header)
{
	struct rasdfx_pkt *pkt = (struct rasdfx_pkt *)buf;
	uint32_t reg_offs, i;
	ssize_t write_cnt;

	rasdfx_wr2buf(s, "SocID: %u\n\n", pkt->dw0.soc_id);

	for (i = 0; i < f_header->pkt_num; i++) {
		ras_parse_rasdfx_pkt_header(s, pkt);
		if (pkt->dw1.reg_count > f_header->pkt_size_dwords - DFX_REG_PACKET_HEAD_LEN) {
			HIKP_ERROR_PRINT("ras dfx register number is incorrect\n");
			return -1;
		}

		for (reg_offs = 0; reg_offs < pkt->dw1.reg_count; reg_offs++)
			rasdfx_wr2buf(s, "0X%08X\n", pkt->reg_base[reg_offs]);
		rasdfx_wr2buf(s, "\n");

		write_cnt = write(s->fd, s->buf, s->buf_offs);
		if (write_cnt != (ssize_t)s->buf_offs) {
			HIKP_ERROR_PRINT("write rasdfx file failed: %s\n", strerror(errno));
			return -1;
		}

		s->buf_offs = 0;
		pkt = (struct rasdfx_pkt *)((uint32_t *)pkt + f_header->pkt_size_dwords);
	}

	return 0;
}

static int ras_dump_data_into_buf(struct ras_dump_cmd *cmd, uint32_t *buf, uint32_t buf_max)
{
	struct hikp_cmd_ret *cmd_ret;
	uint32_t copy_len = 0;
	uint32_t data_num;

	while (copy_len < buf_max) {
		cmd->cmd_id++;
		cmd_ret = ras_get_rsp_data(cmd);
		if (!cmd_ret)
			return -ENOMEM;

		data_num = cmd_ret->rsp_data_num;
		if (data_num == 0 || data_num > HIKP_RSP_ALL_DATA_MAX) {
			HIKP_ERROR_PRINT("invalid response data number: %u\n", data_num);
			hikp_cmd_free(&cmd_ret);
			return -1;
		}

		if (copy_len + data_num > buf_max) {
			HIKP_ERROR_PRINT("response data is more than expected\n");
			hikp_cmd_free(&cmd_ret);
			return -1;
		}

		memcpy(buf + copy_len, cmd_ret->rsp_data, data_num * REP_DATA_BLK_SIZE);
		copy_len += data_num;
		hikp_cmd_free(&cmd_ret);
	}

	return 0;
}

static int ras_get_rasdfx_payload(struct ras_dump_cmd *cmd, struct rasdfx_file_header *f_header)
{
	uint32_t buf_max = f_header->pkt_size_dwords * f_header->pkt_num;
	uint32_t *total_buf;
	struct file_seq s;
	int ret;

	total_buf = (uint32_t *)malloc(buf_max * REP_DATA_BLK_SIZE);
	if (!total_buf) {
		HIKP_ERROR_PRINT("malloc total_buf failed\n");
		return -ENOMEM;
	}

	ret = ras_dump_data_into_buf(cmd, total_buf, buf_max);
	if (ret)
		goto release_total_buf;

	ret = ras_open_rasdfx_file_seq(&s, f_header);
	if (ret)
		goto release_total_buf;

	ret = ras_parse_rasdfx_payload(&s, total_buf, f_header);

	ras_close_rasdfx_file_seq(&s);
release_total_buf:
	free(total_buf);
	return ret;
}

int ras_data_dump(void)
{
	struct ras_dump_cmd cmd = { .cmd_type = DUMP_DFX };
	struct rasdfx_file_header f_header;
	int ret;

	ret = ras_get_rasdfx_header(&cmd, &f_header);
	if (ret)
		return ret;

	if (f_header.head_magic != DFX_DATA_DUMPED_MAGIC) {
		HIKP_ERROR_PRINT("data does not exist or has been cleared.\n");
		return -1;
	}

	if (!ras_check_header(&f_header))
		return -1;

	return ras_get_rasdfx_payload(&cmd, &f_header);
}

int ras_data_clear(void)
{
	struct ras_dump_cmd cmd = { .cmd_type = DUMP_CLEAR };
	struct rasdfx_file_header f_header;
	int ret;

	ret = ras_get_rasdfx_header(&cmd, &f_header);
	if (ret)
		return ret;

	if (f_header.head_magic != DFX_DATA_CLEARED_MAGIC) {
		HIKP_ERROR_PRINT("ras dfx data clear is failed\n");
		return -1;
	}

	return 0;
}
