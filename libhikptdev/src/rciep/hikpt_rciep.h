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

#ifndef __KPT_RCIEP_H__
#define __KPT_RCIEP_H__

#include <stdint.h>
#include "hikptdev_plug.h"

#define MAX_PCI_REVISION_LEN 4

#define HIKP_PCI_DEV_DIR "/device"
#define HIKP_PCI_VENDOR_DIR "/vendor"

#define HIKP_IEP_DEV_ID "0xa12f"
#define HIKP_IEP_VENDOR_ID "0x19e5"
#define HIKP_IEP_REVISION "0x30"
#define HIKP_PCI_RESOURCE "/resource0"
#define HIKP_PCI_CONFIG "/config"

#define RCIEP_FAIL (-1)

#define PCI_COMMAND_REG 0x4

#define HIKP_REQ_DATA_MAX 32
#define HIKP_RSP_DATA_MAX 60

#define HIKP_RSP_CYCLE_MAX 10
#define HIKP_RSP_ALL_DATA_MAX (HIKP_RSP_DATA_MAX * HIKP_RSP_CYCLE_MAX)
#define MAX_LOCK_COUNT 5
#define LOCK_CHECK_GAP_US 1000
#define CPL_CHECK_GAP_US 1000
#define WAIT_CPL_MAX_MS 5000

enum {
	HIKP_RESOURCE_DIR,
	HIKP_CONFIG_DIR,
};

enum rciep_cpl_status {
	HIKP_INIT_STAT = 0, /* Initial state */
	HIKP_CPL_BY_TF = 1, /* TF successfully executed */
	HIKP_CPL_BY_IMU = 2, /* IMU successfully executed. */
	HIKP_INV_REQ = 3, /* Invalid request (command not found) */
	HIKP_INV_REQ_PARA = 4, /* Invalid request parameter */
	HIKP_REQ_PARA_LACK = 5, /* Lack of request parameters */
	HIKP_EXE_FAILED = 6, /* Execution failed */
	HIKP_APP_WAIT_TIMEOUT = 7, /* APP wait Firmware timeout */
	HIKP_IMU_WAIT_IMP_TIMEOUT = 8, /* IMU wait IMP timeout */
	HIKP_IMU_WAIT_APP_TIMEOUT = 9, /* IMU wait APP timeout */
};

#pragma pack(1)

struct iep_doorbell {
	uint32_t db_trig; /* Doorbell interrupt generation register, WO */
	uint32_t db_clr; /* Doorbell interrupt clear register, WC */
	uint32_t db_mask; /* Doorbell interrupt mask register, WR */
	uint32_t db_sta; /* Doorbell interrupt status register, RO */
	uint32_t db_flag; /* Doorbell flag register, WR */
};

union hikp_space_rsp {
	struct {
		uint32_t version; /* 000h */
		uint32_t rsp_para_num; /* 004h */
		uint32_t data[HIKP_RSP_DATA_MAX]; /* 008h */
		uint32_t rsv; /* 0F8h */
		uint32_t cpl_status; /* 0FCh */
		struct iep_doorbell sw_db; /* 100h. Fixed and unmodifiable */
	} field;

	uint32_t dw[69];
};

union hikp_space_req {
	struct {
		uint32_t rsv0[4]; /* 000h */
		struct hikp_cmd_header req_header; /* 010h */
		uint32_t req_para_num; /* 020h */
		uint32_t exe_round; /* 024h */
		uint32_t data[HIKP_REQ_DATA_MAX]; /* 028h */
		uint32_t checksum; /* 0A8h */
		uint32_t rsv1[19]; /* 0ACh */
		uint32_t pid_record; /* 0F8h */
		uint32_t cpl_status; /* 0FCh */
		struct iep_doorbell sw_db; /* 100h. Fixed and unmodifiable */
	} field;

	uint32_t dw[69];
};
#pragma pack()

#endif
