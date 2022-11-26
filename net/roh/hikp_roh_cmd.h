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

#ifndef __HIKP_ROH_CMD_H__
#define __HIKP_ROH_CMD_H__

#include "hikp_net_lib.h"

#define RESPONSE_DATA_NUMBER_MAX 60

enum roh_subcmd {
	CMD_SHOW_MAC_TYPE = 0,
	CMD_SHOW_CAM = 1,
	CMD_SHOW_CREDIT = 2,
	CMD_GET_CAM_REG_NUM = 3,
	CMD_BUILD_CAM_TABLE = 4,
};

enum roh_show_mib_subcmd {
	CMD_SHOW_MIB_FILL_CNT = 0,
};

enum roh_show_bp_subcmd {
	CMD_SHOW_BP = 0,
};

int hikp_roh_get_mac_type(struct major_cmd_ctrl *self, struct bdf_t bdf);
#endif /* __HIKP_ROH_CMD_H__ */
