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

#include "hikp_scc_version.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <inttypes.h>
#include "hikptdev_plug.h"

static int hikp_scc_version_get_data(struct hikp_cmd_ret **cmd_ret, struct scc_cmd_cfg *cmd_cfg)
{
	struct hikp_cmd_header req_header = {0};
	struct scc_version_req_para log_req = {0};

	log_req.chip = cmd_cfg->chip;
	log_req.die = cmd_cfg->die;
	hikp_cmd_init(&req_header, SCC_MOD, SCC_VERSION_CMD, SCC_VERSION_DUMP);
	*cmd_ret = hikp_cmd_alloc(&req_header, &log_req, sizeof(log_req));
	return hikp_rsp_normal_check(*cmd_ret);
}

void hikp_scc_dump_version(struct major_cmd_ctrl *self, struct scc_cmd_cfg *cmd_cfg)
{
	struct hikp_cmd_ret *cmd_ret = NULL;
	struct scc_ver *scc_version = NULL;

	self->err_no = hikp_scc_version_get_data(&cmd_ret, cmd_cfg);
	if (self->err_no) {
		snprintf(self->err_str, sizeof(self->err_str), "get scc version data failed.");
		goto err_out;
	}

	scc_version = (struct scc_ver *)(cmd_ret->rsp_data);
	printf("scc firmware version: %02u.%02u.%02u.%02u\n", scc_version->ver_major,
	       scc_version->ver_minor, scc_version->ver_release, scc_version->ver_patch);

err_out:
	hikp_cmd_free(&cmd_ret);
}
