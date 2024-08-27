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
#include "hikp_socip.h"
#include "tool_lib.h"
#include "tool_cmd.h"
#include "hikptdev_plug.h"

enum {
	CHIP_ID_INDEX = 0,
	DIE_ID_INDEX = 1,
	MODULE_ID_INDEX = 2,
	CONTROLLER_ID_INDEX = 3,
};
#define SOCIP_DUMP_REG_PARAM_NUM 4

struct dump_reg_param_t {
	uint8_t val;
	bool is_vaild;
};

static const char *g_param_desc[SOCIP_DUMP_REG_PARAM_NUM] = {
	"chip id",
	"die id",
	"module id",
	"controller id",
};

static struct dump_reg_param_t g_dump_reg_param[SOCIP_DUMP_REG_PARAM_NUM] = {0};

static int cmd_socip_dump_help(struct major_cmd_ctrl *self, const char *argv)
{
	printf("\n  Usage: %s %s\n", self->cmd_ptr->name,
	       "-c <chipid> -d <dieid> -m <module_id> -i <controller_id>");
	printf("\n         %s\n", self->cmd_ptr->help_info);
	printf("\n  Options:\n\n");
	printf("    -h, %-15s %s\n", "--help", "the help info");
	printf("    -c, %-15s %s\n", "--chip id", "the chip id");
	printf("    -d, %-15s %s\n", "--die id", "the die id");
	printf("    -m, %-15s %s\n", "--module id", "e.g. 1:i2c 2:gpio 3:spi 4:sfc 5:btc");
	printf("    -i, %-15s %s\n", "--controller id",
	       "e.g. while the module is i2c, 0:i2c0 1:i2c1 ...");

	return 0;
}

static int get_param(struct major_cmd_ctrl *self, const char *argv, uint32_t index)
{
	struct dump_reg_param_t *param = &g_dump_reg_param[index];

	if (string_toub(argv, &param->val)) {
		snprintf(self->err_str, sizeof(self->err_str), "%s=%s\n",
			 g_param_desc[index], argv);
		self->err_no = -EINVAL;
		return -EINVAL;
	}
	param->is_vaild = true;

	return 0;
}

static int cmd_chip_id(struct major_cmd_ctrl *self, const char *argv)
{
	return get_param(self, argv, CHIP_ID_INDEX);
}

static int cmd_die_id(struct major_cmd_ctrl *self, const char *argv)
{
	return get_param(self, argv, DIE_ID_INDEX);
}

static int cmd_module_id(struct major_cmd_ctrl *self, const char *argv)
{
	return get_param(self, argv, MODULE_ID_INDEX);
}

static int cmd_controller_id(struct major_cmd_ctrl *self, const char *argv)
{
	return get_param(self, argv, CONTROLLER_ID_INDEX);
}

static bool check_socip_dumpreg_param(void)
{
	struct dump_reg_param_t *param = &g_dump_reg_param[0];
	bool ret = true;
	uint32_t i;

	for (i = 0; i < HIKP_ARRAY_SIZE(g_dump_reg_param); i++) {
		if (!param->is_vaild) {
			ret = false;
			HIKP_ERROR_PRINT("%s is not set\n", g_param_desc[i]);
		}
		param++;
	}

	return ret;
}

void dump_reg_info(const uint32_t *reg_data, uint32_t data_num)
{
#define ONE_LINE_PRINT_DATA_NUM 4
	uint32_t i;

	for (i = 0; i < data_num; i++) {
		if (!(i % ONE_LINE_PRINT_DATA_NUM)) {
			if (i)
				printf("\n");
			printf("%08x:", i); // print offset
		}

		printf("%08x ", reg_data[i]);
	}
	printf("\n");
}

static void hikp_socip_dumpreg_execute(struct major_cmd_ctrl *self)
{
	struct dump_reg_param_t *param = &g_dump_reg_param[0];
	struct hikp_cmd_header req_header = {0};
	struct socip_dump_reg_req_data_t req_data = {0};
	struct hikp_cmd_ret *cmd_ret;

	if (!check_socip_dumpreg_param()) {
		self->err_no = -EINVAL;
		cmd_socip_dump_help(self, NULL);
		return;
	}

	req_data.chip_id  = param[CHIP_ID_INDEX].val;
	req_data.die_id = param[DIE_ID_INDEX].val;
	req_data.controller_id = param[CONTROLLER_ID_INDEX].val;
	hikp_cmd_init(&req_header, SOCIP_MOD, HIKP_SOCIP_CMD_DUMPREG, param[MODULE_ID_INDEX].val);
	cmd_ret = hikp_cmd_alloc(&req_header, &req_data, sizeof(req_data));
	if (!cmd_ret || cmd_ret->status != 0) {
		self->err_no = -EINVAL;
		HIKP_ERROR_PRINT("hikp_cmd_alloc\n");
		hikp_cmd_free(&cmd_ret);
		return;
	}

	dump_reg_info(&cmd_ret->rsp_data[0], cmd_ret->rsp_data_num);
	hikp_cmd_free(&cmd_ret);
}

static void cmd_socip_dump_reg_init(void)
{
	struct major_cmd_ctrl *major_cmd = get_major_cmd();

	major_cmd->option_count = 0;
	major_cmd->execute = hikp_socip_dumpreg_execute;

	cmd_option_register("-h", "--help", false, cmd_socip_dump_help);
	cmd_option_register("-c", "--chip", true, cmd_chip_id);
	cmd_option_register("-d", "--die", true, cmd_die_id);
	cmd_option_register("-m", "--module", true, cmd_module_id);
	cmd_option_register("-i", "--controller", true, cmd_controller_id);
}

HIKP_CMD_DECLARE("socip_dumpreg", "Dump SoCIP registers", cmd_socip_dump_reg_init);
