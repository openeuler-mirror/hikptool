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

#ifndef HIKP_ROH_SHOW_MIB_H
#define HIKP_ROH_SHOW_MIB_H

#include "hikp_net_lib.h"

#define ROH_NAME_MAX 50
#define ROH_CMD_MAX 15
#define FLAG_ROH_MAC_PMU_RC 1
#define FIRST_PMU_CUT 0
#define SECOND_PMU_CUT 1
#define THIRD_PMU_CUT 2
#define FOURTH_PMU_CUT 3
#define PMU_OFFSET 32
#define MIB_EVENT_COUNT 128
#define BLOCK_SIZE 15

struct cmd_roh_show_mib_param {
	struct tool_target target;
	uint8_t flag;
};
union cfg_pmu_val {
	uint32_t val;
	struct {
		uint32_t cfg_pmu_en : 1;
		uint32_t resv1 : 3;
		uint32_t cfg_pmu_clr : 1;
		uint32_t resv2 : 3;
		uint32_t cfg_pmu_rd_en : 1;
		uint32_t resv3 : 3;
		uint32_t cfg_pmu_rd_num : 7;
		uint32_t resv4 : 1;
		uint32_t cfg_pmu_mem : 1;
		uint32_t resv5 : 3;
		uint32_t mem_init_done : 1;
		uint32_t resv6 : 7;
	} _val;
};
union st_pmu_4 {
	uint32_t val;
	struct {
		uint32_t pmu_cnt_rd_vld_0 : 1;
		uint32_t resv1 : 3;
		uint32_t pmu_cnt_rd_vld_1 : 1;
		uint32_t resv2 : 27;
	} _val;
};
struct roh_show_mib_req_paras {
	struct bdf_t bdf;
	uint32_t round;
};
struct roh_show_mib_rsp_t {
	uint64_t reg_data[BLOCK_SIZE][2];
};

struct roh_show_mib_reg_num {
	uint32_t reg_num;
};

enum target_addr {
	CFG_PMU0 = 0,
	CFG_PMU1 = 1,
	ICG_EN = 2,
	ST_PMU0 = 3,
	ST_PMU1 = 4,
	ST_PMU2 = 5,
	ST_PMU3 = 6,
	MAC_TYPE = 7,
};

#define ROH_CMD_SHOW_MIB (1 << 0)
#define RESPONSE_MIB_NUMBER_MAX 15

#endif /* HIKP_ROH_SHOW_MIB_H */
