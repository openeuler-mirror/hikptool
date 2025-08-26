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
#include "op_logs.h"
#include "hikptdev_plug.h"

/* hikptool command adapter */
struct cmd_adapter g_tool = { 0 };

static const char *g_cxl_cmd_list[] = {
	"cxl_cpa",          "cxl_dl",        "cxl_membar",     "cxl_rcrb",
};

static const char *g_hccs_cmd_list[] = {
	"hccs",
};

static const char *g_log_collect_cmd_list[] = {
	"info_collect",
};

static const char *g_nic_cmd_list[] = {
	"nic_dfx",          "nic_fd",        "nic_fec",        "nic_gro",          "nic_info",
	"nic_log",          "nic_mac",       "nic_ncsi",       "nic_notify_pkt",   "nic_port",
	"nic_port_fault",   "nic_ppp",       "nic_qos",        "nic_queue",        "nic_rss",
	"nic_torus",        "nic_xsfp",
};

static const char *g_pcie_cmd_list[] = {
	"pcie_dumpreg",     "pcie_info",     "pcie_regrd",     "pcie_trace",
};

static const char *g_roce_cmd_list[] = {
	"roce_bond",        "roce_caep",     "roce_dfx_sta",   "roce_global_cfg",  "roce_gmv",
	"roce_mdb",         "roce_pkt",      "roce_qmm",       "roce_rst",         "roce_scc",
	"roce_timer",       "roce_trp",      "roce_tsp",
};

static const char *g_roh_cmd_list[] = {
	"roh_mac",          "roh_show_bp",   "roh_show_mib",
};

static const char *g_sas_cmd_list[] = {
	"sas_anacq", "sas_anadq", "sas_dev", "sas_dqe", "sas_dump", "sas_errcode",
};

static const char *g_sata_cmd_list[] = {
	"sata_dump",
};

static const char *g_serdes_cmd_list[] = {
	"serdes_dump",      "serdes_info",
};

static const char *g_socip_cmd_list[] = {
	"socip_dumpreg",
};

static const char *g_core_ring_cmd_list[] = {
	"cpu_ring",
};

static const char *g_sdma_cmd_list[] = {
	"sdma_dump",
};

static const char *g_ub_cmd_list[] = {
	"ub_bp", "ub_crd", "ub_dfx", "ub_info", "ub_link", "unic_ppp",
};

static const char *g_ras_cmd_list[] = {
	"bbox_export",
};

const struct cmd_list_info g_chip_hip09_hip10_cmd_list[] = {
	{g_cxl_cmd_list,            HIKP_ARRAY_SIZE(g_cxl_cmd_list)},
	{g_hccs_cmd_list,           HIKP_ARRAY_SIZE(g_hccs_cmd_list)},
	{g_log_collect_cmd_list,    HIKP_ARRAY_SIZE(g_log_collect_cmd_list)},
	{g_nic_cmd_list,            HIKP_ARRAY_SIZE(g_nic_cmd_list)},
	{g_pcie_cmd_list,           HIKP_ARRAY_SIZE(g_pcie_cmd_list)},
	{g_roce_cmd_list,           HIKP_ARRAY_SIZE(g_roce_cmd_list)},
	{g_roh_cmd_list,            HIKP_ARRAY_SIZE(g_roh_cmd_list)},
	{g_sas_cmd_list,            HIKP_ARRAY_SIZE(g_sas_cmd_list)},
	{g_sata_cmd_list,           HIKP_ARRAY_SIZE(g_sata_cmd_list)},
	{g_serdes_cmd_list,         HIKP_ARRAY_SIZE(g_serdes_cmd_list)},
	{g_socip_cmd_list,          HIKP_ARRAY_SIZE(g_socip_cmd_list)},
};

const struct cmd_list_info g_chip_hip11_cmd_list[] = {
	{g_core_ring_cmd_list,      HIKP_ARRAY_SIZE(g_core_ring_cmd_list)},
	{g_log_collect_cmd_list,    HIKP_ARRAY_SIZE(g_log_collect_cmd_list)},
	{g_nic_cmd_list,            HIKP_ARRAY_SIZE(g_nic_cmd_list)},
	{g_pcie_cmd_list,           HIKP_ARRAY_SIZE(g_pcie_cmd_list)},
	{g_roce_cmd_list,           HIKP_ARRAY_SIZE(g_roce_cmd_list)},
	{g_roh_cmd_list,            HIKP_ARRAY_SIZE(g_roh_cmd_list)},
	{g_sata_cmd_list,           HIKP_ARRAY_SIZE(g_sata_cmd_list)},
	{g_sdma_cmd_list,           HIKP_ARRAY_SIZE(g_sdma_cmd_list)},
	{g_serdes_cmd_list,         HIKP_ARRAY_SIZE(g_serdes_cmd_list)},
	{g_socip_cmd_list,          HIKP_ARRAY_SIZE(g_socip_cmd_list)},
	{g_ub_cmd_list,             HIKP_ARRAY_SIZE(g_ub_cmd_list)},
};

const struct cmd_list_info g_chip_hip12_cmd_list[] = {
	{g_ras_cmd_list,            HIKP_ARRAY_SIZE(g_ras_cmd_list)},
	{g_hccs_cmd_list,           HIKP_ARRAY_SIZE(g_hccs_cmd_list)},
	{g_log_collect_cmd_list,    HIKP_ARRAY_SIZE(g_log_collect_cmd_list)},
	{g_pcie_cmd_list,           HIKP_ARRAY_SIZE(g_pcie_cmd_list)},
	{g_serdes_cmd_list,         HIKP_ARRAY_SIZE(g_serdes_cmd_list)},
	{g_socip_cmd_list,          HIKP_ARRAY_SIZE(g_socip_cmd_list)},
};

static bool cmd_is_support(const char *cmd_name, struct cmd_list_info *cmd_info, size_t len)
{
	for (size_t i = 0; i < len; i++)
		for (size_t j = 0; j < cmd_info[i].list_len; j++)
			if (strcmp(cmd_info[i].cmd_list[j], cmd_name) == 0)
				return true;

	return false;
}

static bool check_cmd_is_support(const char *cmd_name)
{
	uint32_t chip_type = get_chip_type();
	bool is_support = true;

	switch (chip_type) {
	case CHIP_HIP09:
	case CHIP_HIP10:
	case CHIP_HIP10C:
		is_support = cmd_is_support(cmd_name, g_chip_hip09_hip10_cmd_list,
					    HIKP_ARRAY_SIZE(g_chip_hip09_hip10_cmd_list));
		break;
	case CHIP_HIP11:
		is_support = cmd_is_support(cmd_name, g_chip_hip11_cmd_list,
					    HIKP_ARRAY_SIZE(g_chip_hip11_cmd_list));
		break;
	case CHIP_HIP12:
		is_support = cmd_is_support(cmd_name, g_chip_hip12_cmd_list,
					    HIKP_ARRAY_SIZE(g_chip_hip12_cmd_list));
		break;
	default:
		break;
	}

	/* Unrecognized hardware type, default display supported */
	return is_support;
}

static void show_tool_version(const struct cmd_adapter *adapter)
{
	printf("%s version %s Huawei HW(%u)\n", adapter->name, adapter->version, get_chip_type());
}

static int cmp(const void *a, const void *b)
{
	struct hikp_cmd_type x = *(struct hikp_cmd_type *)a;
	struct hikp_cmd_type y = *(struct hikp_cmd_type *)b;

	return strncmp(x.name, y.name, MAX_CMD_LEN);
}

static void show_tool_help(const struct cmd_adapter *adapter)
{
	struct hikp_cmd_type *start_cmd_ptr = (struct hikp_cmd_type *)&_s_cmd_data;
	struct hikp_cmd_type *end_cmd_ptr = (struct hikp_cmd_type *)&_e_cmd_data;
	struct hikp_cmd_type *cmd_ptr = NULL;

	if (adapter == NULL)
		return;

	printf("\n  Usage: %s <major_cmd> [option]\n\n", adapter->name);
	printf("    -h, --help    show help information\n");
	printf("    -v, --version show version information\n");
	printf("\n  Major Commands:\n\n");

	/* We should first sort by dictionary to
	 * avoid the confusion of multi-process compilation.
	 */
	qsort(start_cmd_ptr, end_cmd_ptr - start_cmd_ptr,
	      sizeof(struct hikp_cmd_type), (const void *)cmp);
	for (cmd_ptr = start_cmd_ptr; cmd_ptr < end_cmd_ptr; cmd_ptr++)
		if (check_cmd_is_support(cmd_ptr->name))
			printf("    %-23s  %s\n", cmd_ptr->name, cmd_ptr->help_info);

	printf("\n");
}

static bool is_help_version(struct cmd_adapter *adapter, const int argc, const char **argv)
{
#define ARG_NUM_FOR_HELP_VER 2
	const char *arg = NULL;

	if (argc == 1) {
		show_tool_help(adapter);
		return true;
	}
	if (argc != ARG_NUM_FOR_HELP_VER)
		return false;

	arg = argv[1];
	if (is_specified_option(arg, "-h", "--help")) {
		show_tool_help(adapter);
		return true;
	}
	if (is_specified_option(arg, "-v", "--version")) {
		show_tool_version(adapter);
		return true;
	}

	return false;
}

static int parse_and_init_cmd(const char *arg)
{
	struct hikp_cmd_type *start_cmd_ptr = (struct hikp_cmd_type *)&_s_cmd_data;
	struct hikp_cmd_type *end_cmd_ptr = (struct hikp_cmd_type *)&_e_cmd_data;
	struct hikp_cmd_type *cmd_ptr = NULL;

	for (cmd_ptr = start_cmd_ptr; cmd_ptr < end_cmd_ptr; cmd_ptr++) {
		if (strnlen(cmd_ptr->name, MAX_CMD_LEN) != strnlen(arg, MAX_CMD_LEN))
			continue;

		if (!check_cmd_is_support(cmd_ptr->name))
			continue;

		if ((strncmp(arg, cmd_ptr->name,
		    strnlen(cmd_ptr->name, MAX_CMD_LEN - 1) + 1) == 0) && cmd_ptr->cmd_init) {
			g_tool.p_major_cmd.cmd_ptr = cmd_ptr;
			cmd_ptr->cmd_init();
			return 0;
		}
	}

	return -EINVAL;
}

int main(const int argc, const char **argv)
{
	struct major_cmd_ctrl *major_cmd = get_major_cmd();
	int ret;

	ret = op_log_initialise(HIKP_LOG_DIR_PATH);
	if (ret != 0)
		return ret;

	/* Pre-record user input log */
	op_log_record_input(argc, argv);

	sig_init();
	command_mechanism_init(&g_tool, get_tool_name());

	if (is_help_version(&g_tool, argc, argv))
		return 0;

	ret = hikp_dev_init();
	if (ret != 0) {
		HIKP_ERROR_PRINT("Failed to init RCiEP\n");
		major_cmd->err_no = ret;
		goto IEP_INIT_FAIL;
	}

	ret = parse_and_init_cmd(argv[1]);
	if (ret != 0) {
		major_cmd->err_no = ret;
		HIKP_ERROR_PRINT("Unknown major command, try '%s -h' for help.\n", g_tool.name);
	} else {
		command_parse_and_excute(argc, argv);
	}

	hikp_dev_uninit();

IEP_INIT_FAIL:
	op_log_record_result(major_cmd->err_no, get_tool_name(), HIKP_LOG_DIR_PATH);

	return major_cmd->err_no;
}
