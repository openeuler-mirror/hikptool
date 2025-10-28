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

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <inttypes.h>
#include "hikptdev_plug.h"
#include "ubus_common.h"
#include "ubus_msgq_dump.h"

static int msgq_dump_reg_rsp_data_check(struct hikp_cmd_ret *cmd_ret)
{
	size_t rsp_data_size;
	int ret;

	ret = hikp_rsp_normal_check(cmd_ret);
	if (ret) {
		printf("dump registers failed, ret: %d.\n", ret);
		return ret;
	}

	rsp_data_size = cmd_ret->rsp_data_num * sizeof(uint32_t);
	if (rsp_data_size < sizeof(struct ubus_msgq_reg)) {
		printf("msgq dump reg rsp size check failed, rsp size: %zu, expect size:%zu.\n",
		       rsp_data_size, sizeof(struct ubus_msgq_reg));
		return -EINVAL;
	}

	return 0;
}

static void msgq_dump_reg_parse(struct ubus_msgq_reg *reg)
{
#define UBUS_MSGQ_ADDR_MASK 0xffffffULL
#define UBUS_MSGQ_ADDR_PACK(addr_h, addr_l) \
	((((uint64_t)(addr_h) << 32) | (addr_l)) / (uint64_t)(UBUS_MSGQ_ADDR_MASK + 1))

	/* "******" is size corresponding to the UBUS_MSGQ_ADDR_MASK */
	printf("sq base addr = 0x%" PRIx64 "******\n",
	       UBUS_MSGQ_ADDR_PACK(reg->sq_base_addr_h, reg->sq_base_addr_l));
	printf("sq pi = %u\n", reg->sq_pi);
	printf("sq ci = %u\n", reg->sq_ci);
	printf("sq depth = %u\n", reg->sq_dep);
	printf("sq status = 0x%x\n", reg->sq_status);
	printf("sq int mask = 0x%x\n", reg->sq_int_mask);
	printf("sq int status = 0x%x\n", reg->sq_int_status);
	printf("sq int ro = 0x%x\n", reg->sq_int_ro);

	printf("rq base addr = 0x%" PRIx64 "******\n",
	       UBUS_MSGQ_ADDR_PACK(reg->rq_base_addr_h, reg->rq_base_addr_l));
	printf("rq pi = %u\n", reg->rq_pi);
	printf("rq ci = %u\n", reg->rq_ci);
	printf("rq depth = %u\n", reg->rq_dep);
	printf("rq entry block size = %u\n", reg->rq_entry_blk_sz);
	printf("rq status = 0x%x\n", reg->rq_status);

	printf("cq base addr = 0x%" PRIx64 "******\n",
	       UBUS_MSGQ_ADDR_PACK(reg->cq_base_addr_h, reg->cq_base_addr_l));
	printf("cq pi = %u\n", reg->cq_pi);
	printf("cq ci = %u\n", reg->cq_ci);
	printf("cq depth = %u\n", reg->cq_dep);
	printf("cq status = 0x%x\n", reg->cq_status);
	printf("cq int mask = 0x%x\n", reg->cq_int_mask);
	printf("cq int status = 0x%x\n", reg->cq_int_status);
	printf("cq int ro = 0x%x\n", reg->cq_int_ro);
}

int ubus_msgq_dump_reg_execute(uint32_t controller_idx, uint32_t msgq_idx)
{
	struct ubus_msgq_dump_req_para req_para = { 0 };
	struct hikp_cmd_header req_header = { 0 };
	struct hikp_cmd_ret *cmd_ret;
	int ret;

	hikp_cmd_init(&req_header, UBUS_MOD, UBUS_MSGQ_DUMP, MSGQ_DUMP_REG);
	req_para.controller_idx = controller_idx;
	req_para.msgq_idx = msgq_idx;

	cmd_ret = hikp_cmd_alloc(&req_header, &req_para, sizeof(req_para));
	ret = msgq_dump_reg_rsp_data_check(cmd_ret);
	if (ret)
		goto free_cmd_ret;

	msgq_dump_reg_parse((struct ubus_msgq_reg *)cmd_ret->rsp_data);

free_cmd_ret:
	hikp_cmd_free(&cmd_ret);
	return ret;
}

static int msgq_dump_entry_rsp_data_check(struct hikp_cmd_ret *cmd_ret,
					  uint32_t queue_type,
					  uint32_t *entry_data_size)
{
	size_t rsp_data_size;
	size_t rqe_size;
	size_t expect_size;
	int ret;

	*entry_data_size = 0;
	ret = hikp_rsp_normal_check(cmd_ret);
	if (ret) {
		printf("dump entry failed, ret: %d.\n", ret);
		return ret;
	}

	rsp_data_size = cmd_ret->rsp_data_num * sizeof(uint32_t);
	if (rsp_data_size == 0) {
		printf("msgq dump entry zero rsp data size.\n");
		return -EIO;
	}

	switch (queue_type) {
	case UBUS_MSGQ_SQ:
		expect_size = sizeof(struct ubus_sq_entry) + sizeof(uint32_t);
		*entry_data_size = sizeof(struct ubus_sq_entry);
		break;
	case UBUS_MSGQ_RQ:
#define ROUND_UP(x, n) (((x) + (n) - 1) / (n) * (n))
		/* MSGQ RQ response first u32 for "Entry Block Size" which value
		   maybe not 4-byte aligned, other u32 for "RQ Entry Data" */
		rqe_size = ROUND_UP(cmd_ret->rsp_data[0], sizeof(uint32_t));
		expect_size = rqe_size + sizeof(uint32_t);
		*entry_data_size = rqe_size;
		break;
	case UBUS_MSGQ_CQ:
		expect_size = sizeof(struct ubus_cq_entry) + sizeof(uint32_t);
		*entry_data_size = sizeof(struct ubus_cq_entry);
		break;
	default:
		printf("msgq dump entry unknown type: %u.\n", queue_type);
		return -EINVAL;
	}

#define SIZE_2K 2048
	if (rsp_data_size < expect_size || *entry_data_size > SIZE_2K) {
		printf("rsp size %zu, expected size %zu, entry size %u\n",
		       rsp_data_size, expect_size, *entry_data_size);
		*entry_data_size = 0;
		return -EINVAL;
	}

	return 0;
}

static void msgq_dump_entry_parse(void *data, uint32_t entry_data_size,
				  uint32_t queue_type)
{
	struct ubus_sq_entry *sqe_data;
	struct ubus_cq_entry *cqe_data;
	uint8_t *rqe_data;
	uint32_t i, j;

	switch (queue_type) {
	case UBUS_MSGQ_SQ:
		sqe_data = (struct ubus_sq_entry *)(data + sizeof(uint32_t));
		printf("SQ ENTRY:\n");
		printf("\tmsg code = %u\n", sqe_data->msg_code);
		printf("\tsub msg code = %u\n", sqe_data->sub_msg_code);
		printf("\tmsg id = %u\n", sqe_data->msg_id);
		printf("\tp len = %u\n", sqe_data->p_len);
		printf("\tvl = %u\n", sqe_data->vl);
		printf("\tlocal = %u\n", sqe_data->local);
		printf("\ticrc = %u\n", sqe_data->icrc);
		break;
	case UBUS_MSGQ_RQ:
		rqe_data = (uint8_t *)(data + sizeof(uint32_t));
		printf("RQ ENTRY:\n");
		for (i = 0; i < entry_data_size; i += sizeof(uint32_t)) {
			printf("\t[0x%04x]\t", i);
			for (j = sizeof(uint32_t); j > 0; j--) {
				i + j - 1 >= entry_data_size ?
					printf("-- ") :
					printf("%02x ", rqe_data[i + j - 1]);
			}
			printf("\n");
		}
		break;
	case UBUS_MSGQ_CQ:
		cqe_data = (struct ubus_cq_entry *)(data + sizeof(uint32_t));
		printf("CQ ENTRY:\n");
		printf("\tmsg code = %u\n", cqe_data->msg_code);
		printf("\tsub msg code = %u\n", cqe_data->sub_msg_code);
		printf("\tmsg id = %u\n", cqe_data->msg_id);
		printf("\tp len = %u\n", cqe_data->p_len);
		printf("\tstatus = %u\n", cqe_data->status);
		printf("\trq pi = %u\n", cqe_data->rq_pi);
		break;
	default:
		return;
	}
}

int ubus_msgq_dump_entry_execute(uint32_t controller_idx, uint32_t msgq_idx,
				 uint32_t queue_type, uint32_t entry_idx)
{
	struct ubus_msgq_dump_req_para req_para = { 0 };
	struct hikp_cmd_header req_header = { 0 };
	struct hikp_cmd_ret *cmd_ret;
	uint32_t entry_data_size;
	int ret;

	hikp_cmd_init(&req_header, UBUS_MOD, UBUS_MSGQ_DUMP, MSGQ_DUMP_ENTRY);
	req_para.controller_idx = controller_idx;
	req_para.msgq_idx = msgq_idx;
	req_para.queue_type = queue_type;
	req_para.entry_idx = entry_idx;

	cmd_ret = hikp_cmd_alloc(&req_header, &req_para, sizeof(req_para));
	ret = msgq_dump_entry_rsp_data_check(cmd_ret, queue_type, &entry_data_size);
	if (ret)
		goto free_cmd_ret;

	msgq_dump_entry_parse(cmd_ret->rsp_data, entry_data_size, queue_type);

free_cmd_ret:
	hikp_cmd_free(&cmd_ret);
	return ret;
}