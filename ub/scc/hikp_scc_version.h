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
#ifndef HIKP_SCC_VERSION_H
#define HIKP_SCC_VERSION_H
#include "tool_cmd.h"
#include "hikp_scc_cmd.h"

enum scc_version_sub_cmd {
	SCC_VERSION_DUMP = 1,
};

struct scc_ver {
	uint8_t ver_patch;
	uint8_t ver_release;
	uint8_t ver_minor;
	uint8_t ver_major;
};

struct scc_version_req_para {
	uint8_t chip;
	uint8_t die;
	uint8_t rsv[2];
};

void hikp_scc_dump_version(struct major_cmd_ctrl *self, struct scc_cmd_cfg *cmd_cfg);

#endif /* HIKP_SCC_VERSION_H */
