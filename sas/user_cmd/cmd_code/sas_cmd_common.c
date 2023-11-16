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

#include "sas_tools_include.h"

static struct tool_sas_cmd g_sas_cmd = {
	.sas_cmd_type = SAS_UNKNOW_CMD,
	.chip_id = (uint32_t)(-1),
	.phy_id = (uint32_t)(-1),
	.die_id = (uint32_t)(-1),
	.dev_id = (uint32_t)(-1),
	.que_id = (uint32_t)(-1),
	.dqe_id = (uint32_t)(-1),
};

int sas_set_id(struct major_cmd_ctrl *self, const char *argv, uint32_t *id)
{
	int ret;
	uint32_t val = 0;

	ret = string_toui(argv, &val);
	if (ret) {
		snprintf(self->err_str, sizeof(self->err_str), "Invalid id.");
		self->err_no = ret;
		return ret;
	}
	*id = val;
	return ret;
}

int sas_set_cmd_type(int cmd_type)
{
	g_sas_cmd.sas_cmd_type = cmd_type;
	return 0;
}

struct tool_sas_cmd *sas_get_cmd_p(void)
{
	return &g_sas_cmd;
}

int sas_get_phy_id(void)
{
	return g_sas_cmd.phy_id;
}

int sas_get_que_id(void)
{
	return g_sas_cmd.que_id;
}

int sas_get_cmd_type(void)
{
	return g_sas_cmd.sas_cmd_type;
}

int sas_set_chip_id(struct major_cmd_ctrl *self, const char *argv)
{
	return sas_set_id(self, argv, &g_sas_cmd.chip_id);
}

int sas_set_phy_id(struct major_cmd_ctrl *self, const char *argv)
{
	return sas_set_id(self, argv, &g_sas_cmd.phy_id);
}

int sas_set_die_id(struct major_cmd_ctrl *self, const char *argv)
{
	return sas_set_id(self, argv, &g_sas_cmd.die_id);
}

int sas_set_que_id(struct major_cmd_ctrl *self, const char *argv)
{
	return sas_set_id(self, argv, &g_sas_cmd.que_id);
}

int sas_set_dqe_id(struct major_cmd_ctrl *self, const char *argv)
{
	return sas_set_id(self, argv, &g_sas_cmd.dqe_id);
}
