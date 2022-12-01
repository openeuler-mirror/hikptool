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

#ifndef __SAS_TOOLS_INCLUDE_H_
#define __SAS_TOOLS_INCLUDE_H_

#include "hikptdev_plug.h"
#include "tool_lib.h"
#include "tool_cmd.h"

#define MAX_PARA_LENTH 10
#define SAS_UNKNOW_CMD 0xff

struct tool_sas_cmd {
	uint32_t sas_cmd_type;
	uint32_t chip_id;
	uint32_t phy_id;
	uint32_t die_id;
	uint32_t dev_id;
	uint32_t que_id;
	uint32_t dqe_id;
};

int sas_set_cmd_type(int cmd_type);
int sas_get_cmd_type(void);
int sas_get_phy_id(void);
int sas_get_dev_id(void);
int sas_get_que_id(void);
struct tool_sas_cmd *sas_get_cmd_p(void);
int sas_set_chip_id(struct major_cmd_ctrl *self, const char *argv);
int sas_set_phy_id(struct major_cmd_ctrl *self, const char *argv);
int sas_set_die_id(struct major_cmd_ctrl *self, const char *argv);
int sas_set_dev_id(struct major_cmd_ctrl *self, const char *argv);
int sas_set_que_id(struct major_cmd_ctrl *self, const char *argv);
int sas_set_dqe_id(struct major_cmd_ctrl *self, const char *argv);

#endif
