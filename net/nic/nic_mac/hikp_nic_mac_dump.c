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

#include <unistd.h>
#include "tool_cmd.h"
#include "hikp_net_lib.h"
#include "hikptdev_plug.h"
#include "hikp_mac_cmd.h"
#include "hikp_nic_mac_dump.h"

static struct cmd_mac_dump g_dump_reg_info = {0};

static int mac_dump_module_reg(struct major_cmd_ctrl *self, uint32_t cur_blk_id, uint32_t sub_code)
{
	struct reg_rsp_info *rsp_data = NULL;
	struct dump_reg_req req_data = {0};
	struct hikp_cmd_ret *cmd_ret = NULL;
	struct hikp_cmd_header req_header = {0};
	uint32_t rsp_reg_num;
	uint32_t i;

	req_data.bdf = g_dump_reg_info.target.bdf;
	req_data.blk_id = cur_blk_id;

	hikp_cmd_init(&req_header, MAC_MOD, MAC_CMD_DUMP_REG, sub_code);
	cmd_ret = hikp_cmd_alloc(&req_header, &req_data, sizeof(req_data));
	if (!cmd_ret) {
		HIKP_ERROR_PRINT("dump mac sub_code:0x%x reg failed.\n", sub_code);
		self->err_no = -ENOSPC;
		return self->err_no;
	}
	if (cmd_ret->status != 0) {
		hikp_cmd_free(&cmd_ret);
		self->err_no = -EINVAL;
		return self->err_no;
	}

	rsp_data = (struct reg_rsp_info *)(cmd_ret->rsp_data);
	rsp_reg_num = cmd_ret->rsp_data_num >> 1U;

	for (i = 0; i < rsp_reg_num; i++) {
		printf("\t[0x%04x] :\t0x%012lx\n", rsp_data->addr, (uint64_t)rsp_data->val);
		rsp_data++;
	}

	hikp_cmd_free(&cmd_ret);

	return 0;
}

static int mac_cmd_dump_mod(struct major_cmd_ctrl *self, const char *name,
			    uint32_t sub_code, uint32_t blk_num)
{
	uint32_t i;
	int ret;

	if (blk_num == 0) {
		printf("%s module is not support dump.\n", name);
		return 0;
	}

	printf("============ %10s REG INFO ==============\n", name);
	printf("\t %s  :\t%10s\n", "offset", "value");

	for (i = 0; i < blk_num; i++) {
		ret = mac_dump_module_reg(self, i, sub_code);
		if (ret != 0)
			return ret;
	}

	return 0;
}

struct mac_dump_mod_proc g_dump_mod_proc[] = {
	{MOD_RX_MAC,        MAC_DUMP_RX_MAC_REG,        "RX_MAC"      },
	{MOD_RX_PCS,        MAC_DUMP_RX_PCS_REG,        "RX_PCS"      },
	{MOD_RX_RSFEC,      MAC_DUMP_RX_RSFEC_REG,      "RX_RSFEC"    },
	{MOD_RX_BRFEC,      MAC_DUMP_RX_BRFEC_REG,      "RX_BRFEC"    },
	{MOD_RXPMA_CORE,    MAC_DUMP_RXPMA_CORE_REG,    "RXPMA_CORE"  },
	{MOD_RXPMA_LANE,    MAC_DUMP_RXPMA_LANE_REG,    "RXPMA_LANE"  },
	{MOD_TXPMA_LANE,    MAC_DUMP_TXPMA_LANE_REG,    "TXPMA_LANE"  },
	{MOD_TXPMA_CORE,    MAC_DUMP_TXPMA_CORE_REG,    "TXPMA_CORE"  },
	{MOD_TX_BRFEC,      MAC_DUMP_TX_BRFEC_REG,      "TX_BRFEC"    },
	{MOD_TX_RSFEC,      MAC_DUMP_TX_RSFEC_REG,      "TX_RSFEC"    },
	{MOD_TX_PCS,        MAC_DUMP_TX_PCS_REG,        "TX_PCS"      },
	{MOD_TX_MAC,        MAC_DUMP_TX_MAC_REG,        "TX_MAC"      },
	{MOD_MIB,           MAC_DUMP_MIB_REG,           "MIB"         },
	{MOD_COM,           MAC_DUMP_COM_REG,           "COM"         },
	{MOD_GE,            MAC_DUMP_GE_REG,            "GE",         },
	{MOD_MAC_COMM,      MAC_DUMP_MAC_COMM_REG,      "MAC_COMM"    },
	{MOD_AN,            MAC_DUMP_AN_REG,            "AN"          },
	{MOD_LT,            MAC_DUMP_LT_REG,            "LT"          },
};

static void mac_cmd_dump_all(struct major_cmd_ctrl *self)
{
	size_t size = HIKP_ARRAY_SIZE(g_dump_mod_proc);
	size_t i;
	int ret;

	for (i = 0; i < size; i++) {
		ret = mac_cmd_dump_mod(self, g_dump_mod_proc[i].name, g_dump_mod_proc[i].sub_cmd,
				       g_dump_reg_info.blk_num[g_dump_mod_proc[i].module_id]);
		if (ret != 0) {
			HIKP_ERROR_PRINT("dump module name:%s reg failed.\n",
					 g_dump_mod_proc[i].name);
			return;
		}
	}
}

static void mac_cmd_dump_module(struct major_cmd_ctrl *self, const char *module_name)
{
	size_t size = HIKP_ARRAY_SIZE(g_dump_mod_proc);
	size_t i;
	int ret;

	for (i = 0; i < size; i++) {
		if (strcmp(g_dump_mod_proc[i].name, module_name) != 0)
			continue;

		ret = mac_cmd_dump_mod(self, g_dump_mod_proc[i].name, g_dump_mod_proc[i].sub_cmd,
				       g_dump_reg_info.blk_num[g_dump_mod_proc[i].module_id]);
		if (ret != 0)
			HIKP_ERROR_PRINT("dump module name:%s reg failed.\n", module_name);
		return;
	}

	HIKP_ERROR_PRINT("invalid module name:%s.\n", module_name);
}

static int mac_cmd_get_dump_blk_num(struct major_cmd_ctrl *self)
{
	struct hikp_cmd_ret *cmd_ret = NULL;
	struct hikp_cmd_header req_header = {0};

	hikp_cmd_init(&req_header, MAC_MOD, MAC_CMD_DUMP_REG, MAC_DUMP_GET_BLK_NUM);
	cmd_ret = hikp_cmd_alloc(&req_header, &g_dump_reg_info.target.bdf,
				 sizeof(g_dump_reg_info.target.bdf));
	if (!cmd_ret) {
		HIKP_ERROR_PRINT("nic_mac get reg blk num failed.\n");
		self->err_no = -ENOSPC;
		return self->err_no;
	}

	if (cmd_ret->status != 0 || cmd_ret->rsp_data_num < MOD_ID_MAX) {
		HIKP_ERROR_PRINT("nic_mac reg blk num error, rsp_num:%u\n", cmd_ret->rsp_data_num);
		self->err_no = -EINVAL;
		hikp_cmd_free(&cmd_ret);
		return self->err_no;
	}

	memcpy(g_dump_reg_info.blk_num, cmd_ret->rsp_data, sizeof(g_dump_reg_info.blk_num));

	hikp_cmd_free(&cmd_ret);

	return 0;
}

static int mac_cmd_dump_para_check(struct major_cmd_ctrl *self)
{
	if (!g_dump_reg_info.port_flag) {
		self->err_no = -EINVAL;
		snprintf(self->err_str, sizeof(self->err_str), "Need port id.");
		return self->err_no;
	}

	if (!g_dump_reg_info.module_name) {
		self->err_no = -EINVAL;
		snprintf(self->err_str, sizeof(self->err_str), "Need module name.");
		return self->err_no;
	}

	return 0;
}

void mac_cmd_dump_execute(struct major_cmd_ctrl *self)
{
	int ret;

	ret = mac_cmd_dump_para_check(self);
	if (ret != 0)
		return;

	/* first:get blk num */
	ret = mac_cmd_get_dump_blk_num(self);
	if (ret != 0)
		return;

	if (strcmp(g_dump_reg_info.module_name, "ALL") == 0)
		mac_cmd_dump_all(self);
	else
		mac_cmd_dump_module(self, g_dump_reg_info.module_name);
}

int mac_cmd_dump_reg_target(struct major_cmd_ctrl *self, const char *argv)
{
	self->err_no = tool_check_and_get_valid_bdf_id(argv, &g_dump_reg_info.target);
	if (self->err_no) {
		snprintf(self->err_str, sizeof(self->err_str), "Unknown device %s.", argv);
		return self->err_no;
	}
	g_dump_reg_info.port_flag = true;

	return 0;
}

static int mac_cmd_dump_reg_help(struct major_cmd_ctrl *self, const char *argv)
{
	HIKP_SET_USED(argv);

	printf("\n  Usage: %s %s\n", self->cmd_ptr->name, "-i <interface> -m <module>\n");
	printf("\n         %s\n", self->cmd_ptr->help_info);
	printf("  Options:\n\n");
	printf("    %s, %-25s %s\n", "-h", "--help", "display this help and exit");
	printf("    %s, %-25s %s\n", "-i", "--interface=<interface>", "device target, e.g. eth0~3");

	printf("    %s, %-25s %s\n", "-m", "--module=<module>",
	       "input the mac key reg module name, e.g:");
	printf("                                  %s\n",
	       "ALL/RX_MAC/RX_PCS/RX_RSFEC/RX_BRFEC/RXPMA_CORE/RXPMA_LANE/TXPMA_LANE");
	printf("                                  %s\n",
	       "TXPMA_CORE/TX_BRFEC/TX_RSFEC/TX_PCS/TX_MAC/MIB/COM/GE/MAC_COMM/AN/LT");

	return 0;
}

int mac_cmd_dump_module_cfg(struct major_cmd_ctrl *self, const char *argv)
{
	HIKP_SET_USED(self);

	g_dump_reg_info.module_name = argv;

	return 0;
}

int hikp_info_collect_nic_mac(void *data)
{
	struct nic_mac_collect_param *param = (struct nic_mac_collect_param *)data;
	struct major_cmd_ctrl *major_cmd = get_major_cmd();
	int ret;

	memset(&g_dump_reg_info, 0, sizeof(g_dump_reg_info));

	ret = mac_cmd_dump_reg_target(major_cmd, param->net_dev_name);
	if (ret)
		return ret;

	ret = mac_cmd_dump_module_cfg(major_cmd, param->module_name);
	if (ret)
		return ret;

	printf("hikptool nic_mac -i %s -m %s\n", param->net_dev_name, param->module_name);
	mac_cmd_dump_execute(major_cmd);

	return ret;
}

static void cmd_mac_dump_reg_init(void)
{
	struct major_cmd_ctrl *major_cmd = get_major_cmd();

	g_dump_reg_info.port_flag = false;

	major_cmd->option_count = 0;
	major_cmd->execute = mac_cmd_dump_execute;

	cmd_option_register("-h", "--help",       false,    mac_cmd_dump_reg_help);
	cmd_option_register("-i", "--interface",  true,     mac_cmd_dump_reg_target);
	cmd_option_register("-m", "--module",     true,     mac_cmd_dump_module_cfg);
}

HIKP_CMD_DECLARE("nic_mac", "dump mac module reg information", cmd_mac_dump_reg_init);
