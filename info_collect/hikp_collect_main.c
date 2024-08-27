/*
 * Copyright (c) 2024 Hisilicon Technologies Co., Ltd.
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

#include "hikp_collect_lib.h"
#include "hikp_collect.h"
#include "tool_lib.h"
#include "tool_cmd.h"

static enum info_collect_type g_info_collect_type = COLLECT_UNKNOWN_TYPE;
static enum info_collect_type get_info_collect_type(void)
{
	return g_info_collect_type;
}

static void set_info_collect_type(enum info_collect_type type)
{
	g_info_collect_type = type;
}

static int info_collect_acc(struct major_cmd_ctrl *self, const char *argv)
{
	HIKP_SET_USED(self);
	HIKP_SET_USED(argv);

	set_info_collect_type(COLLECT_ACC);
	return 0;
}

static int info_collect_imp(struct major_cmd_ctrl *self, const char *argv)
{
	HIKP_SET_USED(self);
	HIKP_SET_USED(argv);

	set_info_collect_type(COLLECT_IMP);
	return 0;
}

static int info_collect_nic(struct major_cmd_ctrl *self, const char *argv)
{
	HIKP_SET_USED(self);
	HIKP_SET_USED(argv);

	set_info_collect_type(COLLECT_NIC);
	return 0;
}

static int info_collect_pcie(struct major_cmd_ctrl *self, const char *argv)
{
	HIKP_SET_USED(self);
	HIKP_SET_USED(argv);

	set_info_collect_type(COLLECT_PCIE);
	return 0;
}

static int info_collect_roce(struct major_cmd_ctrl *self, const char *argv)
{
	HIKP_SET_USED(self);
	HIKP_SET_USED(argv);

	set_info_collect_type(COLLECT_ROCE);
	return 0;
}

static int info_collect_sas(struct major_cmd_ctrl *self, const char *argv)
{
	HIKP_SET_USED(self);
	HIKP_SET_USED(argv);

	set_info_collect_type(COLLECT_SAS);
	return 0;
}

static int info_collect_sata(struct major_cmd_ctrl *self, const char *argv)
{
	HIKP_SET_USED(self);
	HIKP_SET_USED(argv);

	set_info_collect_type(COLLECT_SATA);
	return 0;
}

static int info_collect_serdes(struct major_cmd_ctrl *self, const char *argv)
{
	HIKP_SET_USED(self);
	HIKP_SET_USED(argv);

	set_info_collect_type(COLLECT_SERDES);
	return 0;
}

static int info_collect_socip(struct major_cmd_ctrl *self, const char *argv)
{
	HIKP_SET_USED(self);
	HIKP_SET_USED(argv);

	set_info_collect_type(COLLECT_SOCIP);
	return 0;
}

static int info_collect_all(struct major_cmd_ctrl *self, const char *argv)
{
	HIKP_SET_USED(self);
	HIKP_SET_USED(argv);

	set_info_collect_type(COLLECT_ALL);
	return 0;
}

static void collect_all_log(void)
{
	collect_pcie_info();
	collect_acc_log();
	collect_imp_log();
	collect_nic_log();
	collect_roce_log();
	collect_sas_log();
	collect_sata_log();
	collect_serdes_log();
	collect_socip_log();
}

static int info_collect_excute_funs_call(uint32_t collect_type)
{
	const char *type_name[] = {"acc", "imp", "nic", "pcie", "roce", "sas",
		"sata", "serdes", "socip", "all"};
	int ret;

	if (collect_type == COLLECT_UNKNOWN_TYPE)
		return -EINVAL;

	ret = hikp_create_save_path(type_name[collect_type]);
	if (ret) {
		HIKP_ERROR_PRINT("create save path fail: %d\n", ret);
		return ret;
	}

	switch (collect_type) {
	case COLLECT_ACC:
		collect_acc_log();
		break;
	case COLLECT_IMP:
		collect_imp_log();
		break;
	case COLLECT_NIC:
		collect_nic_log();
		break;
	case COLLECT_PCIE:
		collect_pcie_info();
		break;
	case COLLECT_ROCE:
		collect_roce_log();
		break;
	case COLLECT_SAS:
		collect_sas_log();
		break;
	case COLLECT_SATA:
		collect_sata_log();
		break;
	case COLLECT_SERDES:
		collect_serdes_log();
		break;
	case COLLECT_SOCIP:
		collect_socip_log();
		break;
	case COLLECT_ALL:
		collect_all_log();
		break;
	default:
		return -EINVAL;
	}

	collect_common_log();
	ret = hikp_compress_log();

	return ret;
}


static void info_collect_execute(struct major_cmd_ctrl *self)
{
	const char *suc_msg[] = {
		"collect acc info success.",
		"collect imp info success.",
		"collect nic info success.",
		"collect pcie info success.",
		"collect roce info success.",
		"collect sas info success.",
		"collect sata info success.",
		"collect serdes info success.",
		"collect socip info success.",
		"collect all info success.",
	};
	const char *err_msg[] = {
		"collect acc info error.",
		"collect imp info error.",
		"collect nic info error.",
		"collect pcie info error.",
		"collect roce info error.",
		"collect sas info error.",
		"collect sata info error.",
		"collect serdes info error.",
		"collect socip info error.",
		"collect all info error.",
		"collect info failed, unknown type.",
	};
	enum info_collect_type type;
	int ret;

	type = get_info_collect_type();
	ret = info_collect_excute_funs_call(type);
	set_info_collect_type(COLLECT_UNKNOWN_TYPE);
	if (ret == 0) {
		printf("%s\n", suc_msg[type]);
	} else {
		(void)snprintf(self->err_str, sizeof(self->err_str), "%s\n", err_msg[type]);
		self->err_no = ret;
	}
}

static int info_collect_help(struct major_cmd_ctrl *self, const char *argv)
{
	HIKP_SET_USED(argv);

	printf("\n  Usage: %s\n", self->cmd_ptr->name);
	printf("\n         %s\n", self->cmd_ptr->help_info);
	printf("\n  Options:\n\n");
	printf("    %s, %-25s %s\n", "-h", "--help", "display this help and exit\n");
	printf("    %s, %-25s %s\n", "-acc", "--acc", "collect acc info\n");
	printf("    %s, %-25s %s\n", "-imp", "--imp", "collect imp info\n");
	printf("    %s, %-25s %s\n", "-nic", "--nic", "collect nic info\n");
	printf("    %s, %-25s %s\n", "-pcie", "--pcie", "collect pcie info\n");
	printf("    %s, %-25s %s\n", "-roce", "--roce", "collect roce info\n");
	printf("    %s, %-25s %s\n", "-sas", "--sas", "collect sas info\n");
	printf("    %s, %-25s %s\n", "-sata", "--sata", "collect sata info\n");
	printf("    %s, %-25s %s\n", "-serdes", "--serdes", "collect serdes info\n");
	printf("    %s, %-25s %s\n", "-socip", "--socip", "collect socip info\n");
	printf("    %s, %-25s %s\n", "-all", "--all", "collect all info\n");
	printf("\n");

	return 0;
}

static void cmd_info_collect_init(void)
{
	struct major_cmd_ctrl *major_cmd = get_major_cmd();

	major_cmd->option_count = 0;
	major_cmd->execute = info_collect_execute;

	cmd_option_register("-h", "--help", false, info_collect_help);
	cmd_option_register("-acc", "--acc", false, info_collect_acc);
	cmd_option_register("-imp", "--imp", false, info_collect_imp);
	cmd_option_register("-nic", "--nic", false, info_collect_nic);
	cmd_option_register("-pcie", "--pcie", false, info_collect_pcie);
	cmd_option_register("-roce", "--roce", false, info_collect_roce);
	cmd_option_register("-sas", "--sas", false, info_collect_sas);
	cmd_option_register("-sata", "--sata", false, info_collect_sata);
	cmd_option_register("-serdes", "--serdes", false, info_collect_serdes);
	cmd_option_register("-socip", "--socip", false, info_collect_socip);
	cmd_option_register("-all", "--all", false, info_collect_all);
}

HIKP_CMD_DECLARE("info_collect", "information collect", cmd_info_collect_init);
