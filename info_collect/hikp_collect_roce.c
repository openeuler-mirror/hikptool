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
#include "hikp_roce_bond.h"
#include "hikp_roce_global_cfg.h"
#include "hikp_roce_rst.h"
#include "hikp_roce_timer.h"
#include "hikp_roce_pkt.h"
#include "hikp_roce_mdb.h"
#include "hikp_roce_caep.h"
#include "hikp_roce_qmm.h"
#include "hikp_roce_trp.h"
#include "hikp_roce_tsp.h"
#include "hikp_roce_scc.h"
#include "hikp_roce_gmv.h"
#include "hikp_roce_dfx_sta.h"

static void collect_roce_devinfo_log(void)
{
	const struct info_collect_cmd roce_devinfo_cmd = {
		.group = GROUP_ROCE,
		.log_name = "ibv_devinfo",
		.args = {"ibv_devinfo", NULL},
	};
	int ret;

	ret = hikp_collect_log(roce_devinfo_cmd.group,
			       roce_devinfo_cmd.log_name,
			       hikp_collect_exec,
			       (void *)&roce_devinfo_cmd);
	if (ret)
		HIKP_ERROR_PRINT("collect %s log failed: %d\n",
				 roce_devinfo_cmd.log_name, ret);
}

static void collect_roce_cc_param_log(void)
{
	struct info_collect_cmd roce_cc_param_cmd = {
		.group = GROUP_ROCE,
		.log_name = "cc_param",
		.args = {"cp", "-rf",
			 "/sys/class/infiniband/*/ports/1/cc_param", NULL},
	};
	int ret;

	ret = hikp_collect_log(roce_cc_param_cmd.group,
			       roce_cc_param_cmd.log_name,
			       hikp_collect_cp_glob_exec,
			       (void *)&roce_cc_param_cmd);
	if (ret)
		HIKP_ERROR_PRINT("collect %s log failed: %d\n",
				 roce_cc_param_cmd.log_name, ret);
}

static void collect_roce_sw_stats_log(void)
{
	struct info_collect_cmd roce_sw_stats_cmd = {
		.group = GROUP_ROCE,
		.log_name = "sw_stat",
		.args = {"cat", "/sys/kernel/debug/hns_roce/*/sw_stat/sw_stat",
			 NULL},
	};
	int ret;

	ret = hikp_collect_log(roce_sw_stats_cmd.group,
			       roce_sw_stats_cmd.log_name,
			       hikp_collect_cat_glob_exec,
			       (void *)&roce_sw_stats_cmd);
	if (ret)
		HIKP_ERROR_PRINT("collect %s log failed: %d\n",
				 roce_sw_stats_cmd.log_name, ret);
}

static void collect_roce_res_stats_log(void)
{
	const struct info_collect_cmd roce_res_stats_cmd = {
		.group = GROUP_ROCE,
		.log_name = "rdma_res_show",
		.args = {"rdma", "res", "show", NULL},
	};
	int ret;

	ret = hikp_collect_log(roce_res_stats_cmd.group,
			       roce_res_stats_cmd.log_name,
			       hikp_collect_exec,
			       (void *)&roce_res_stats_cmd);
	if (ret)
		HIKP_ERROR_PRINT("collect %s log failed: %d\n",
				 roce_res_stats_cmd.log_name, ret);
}

static int collect_hikp_roce_gmv_log(void *nic_name)
{
	struct major_cmd_ctrl self = {0};
	struct hikp_cmd_type type = {0};
	uint32_t gmv_index;
	int ret;

	self.cmd_ptr = &type;
	ret = hikp_roce_set_gmv_bdf((char *)nic_name);
	if (ret) {
		HIKP_ERROR_PRINT("failed to set roce_gmv bdf for %s.\n",
				 (char *)nic_name);
		return ret;
	}

	for (gmv_index = 0; gmv_index < ROCE_MAX_HIKPTOOL_GMV; gmv_index++) {
		printf("hikptool roce_gmv -i %s -x %u\n", (char *)nic_name, gmv_index);
		hikp_roce_set_gmv_index(gmv_index);
		hikp_roce_gmv_execute(&self);
	}

	return 0;
}

static int collect_hikp_roce_dfx_sta_log(void *nic_name)
{
	struct major_cmd_ctrl self = {0};
	struct hikp_cmd_type type = {0};
	int ret;

	self.cmd_ptr = &type;
	ret = hikp_roce_set_dfx_sta_bdf((char *)nic_name);
	if (ret) {
		HIKP_ERROR_PRINT("failed to set roce_dfx_sta bdf for %s.\n",
				 (char *)nic_name);
		return ret;
	}

	printf("hikptool roce_dfx_sta -i %s\n", (char *)nic_name);
	hikp_roce_dfx_sta_execute(&self);

	return 0;
}

static int collect_hikp_roce_scc_log(void *nic_name)
{
	struct major_cmd_ctrl self = {0};
	struct hikp_cmd_type type = {0};
	int ret;

	self.cmd_ptr = &type;
	ret = hikp_roce_set_scc_bdf((char *)nic_name);
	if (ret) {
		HIKP_ERROR_PRINT("failed to set roce_scc bdf for %s.\n",
				 (char *)nic_name);
		return ret;
	}

	printf("hikptool roce_scc -i %s -m COMMON\n", (char *)nic_name);
	hikp_roce_set_scc_submodule(SCC_COMMON);
	hikp_roce_scc_execute(&self);

	printf("hikptool roce_scc -i %s -m DCQCN\n", (char *)nic_name);
	hikp_roce_set_scc_submodule(DCQCN);
	hikp_roce_scc_execute(&self);

	printf("hikptool roce_scc -i %s -m DIP\n", (char *)nic_name);
	hikp_roce_set_scc_submodule(DIP);
	hikp_roce_scc_execute(&self);

	printf("hikptool roce_scc -i %s -m HC3\n", (char *)nic_name);
	hikp_roce_set_scc_submodule(HC3);
	hikp_roce_scc_execute(&self);

	printf("hikptool roce_scc -i %s -m LDCP\n", (char *)nic_name);
	hikp_roce_set_scc_submodule(LDCP);
	hikp_roce_scc_execute(&self);

	printf("hikptool roce_scc -i %s -m CFG\n", (char *)nic_name);
	hikp_roce_set_scc_submodule(CFG);
	hikp_roce_scc_execute(&self);

	return 0;
}

static int collect_hikp_roce_tsp_log(void *nic_name)
{
	struct major_cmd_ctrl self = {0};
	struct hikp_cmd_type type = {0};
	uint32_t bankid;
	int ret;

	self.cmd_ptr = &type;
	ret = hikp_roce_set_tsp_bdf((char *)nic_name);
	if (ret) {
		HIKP_ERROR_PRINT("failed to set roce_tsp bdf for %s.\n",
				 (char *)nic_name);
		return ret;
	}

	hikp_roce_set_tsp_submodule(TSP_COMMON);
	for (bankid = 0; bankid <= MAX_TSP_BANK_NUM; bankid++) {
		hikp_roce_set_tsp_bankid(bankid);

		printf("hikptool roce_tsp -i %s -m COMMON -b %u\n", (char *)nic_name, bankid);
		hikp_roce_tsp_execute(&self);
	}

	hikp_roce_set_tsp_submodule(TGP_TMP);
	for (bankid = 0; bankid <= MAX_TGP_TMP_BANK_NUM; bankid++) {
		hikp_roce_set_tsp_bankid(bankid);

		printf("hikptool roce_tsp -i %s -m TGP_TMP -b %u\n", (char *)nic_name, bankid);
		hikp_roce_tsp_execute(&self);
	}

	printf("hikptool roce_tsp -i %s -m TDP\n", (char *)nic_name);
	hikp_roce_set_tsp_submodule(TDP);
	hikp_roce_tsp_execute(&self);

	return 0;
}

static int collect_hikp_roce_trp_log(void *nic_name)
{
	struct major_cmd_ctrl self = {0};
	struct hikp_cmd_type type = {0};
	uint32_t bankid;
	int ret;

	self.cmd_ptr = &type;
	ret = hikp_roce_set_trp_bdf((char *)nic_name);
	if (ret) {
		HIKP_ERROR_PRINT("failed to set roce_trp bdf for %s.\n",
				 (char *)nic_name);
		return ret;
	}

	hikp_roce_set_trp_submodule(TRP_COMMON);
	for (bankid = 0; bankid <= TRP_MAX_BANK_NUM; bankid++) {
		hikp_roce_set_trp_bankid(bankid);

		printf("hikptool roce_trp -i %s -m COMMON -b %u\n", (char *)nic_name, bankid);
		hikp_roce_trp_execute(&self);
	}

	printf("hikptool roce_trp -i %s -m TRP_RX\n", (char *)nic_name);
	hikp_roce_set_trp_submodule(TRP_RX);
	hikp_roce_trp_execute(&self);

	hikp_roce_set_trp_submodule(GEN_AC);
	for (bankid = 0; bankid <= GAC_MAX_BANK_NUM; bankid++) {
		hikp_roce_set_trp_bankid(bankid);

		printf("hikptool roce_trp -i %s -m GEN_AC -b %u\n", (char *)nic_name, bankid);
		hikp_roce_trp_execute(&self);
	}

	hikp_roce_set_trp_submodule(PAYL);
	for (bankid = 0; bankid <= PAYL_MAX_BANK_NUM; bankid++) {
		hikp_roce_set_trp_bankid(bankid);

		printf("hikptool roce_trp -i %s -m PAYL -b %u\n", (char *)nic_name, bankid);
		hikp_roce_trp_execute(&self);
	}

	return 0;
}

static int collect_hikp_roce_qmm_log(void *nic_name)
{
	struct major_cmd_ctrl self = {0};
	struct hikp_cmd_type type = {0};
	uint32_t bankid;
	int ret;

	self.cmd_ptr = &type;
	ret = hikp_roce_set_qmm_bdf((char *)nic_name);
	if (ret) {
		HIKP_ERROR_PRINT("failed to set roce_qmm bdf for %s.\n",
				 (char *)nic_name);
		return ret;
	}

	for (bankid = 0; bankid <= QMM_BANK_NUM; bankid++) {
		hikp_roce_set_qmm_bankid(bankid);

		printf("hikptool roce_qmm -i %s -b %u\n", (char *)nic_name, bankid);
		hikp_roce_set_qmm_ext_flag(false);
		hikp_roce_qmm_execute(&self);

		printf("hikptool roce_qmm -i %s -b %u -e\n", (char *)nic_name, bankid);
		hikp_roce_set_qmm_ext_flag(true);
		hikp_roce_qmm_execute(&self);
	}

	return 0;
}

static int collect_hikp_roce_caep_log(void *nic_name)
{
	struct major_cmd_ctrl self = {0};
	struct hikp_cmd_type type = {0};
	int ret;

	self.cmd_ptr = &type;
	ret = hikp_roce_set_caep_bdf((char *)nic_name);
	if (ret) {
		HIKP_ERROR_PRINT("failed to set roce_caep bdf for %s.\n",
				 (char *)nic_name);
		return ret;
	}

	printf("hikptool roce_caep -i %s\n", (char *)nic_name);
	hikp_roce_set_caep_mode(CAEP_ORIGIN);
	hikp_roce_caep_execute(&self);

	printf("hikptool roce_caep -i %s -e\n", (char *)nic_name);
	hikp_roce_set_caep_mode(CAEP_EXT);
	hikp_roce_caep_execute(&self);

	return 0;
}

static int collect_hikp_roce_mdb_log(void *nic_name)
{
	struct major_cmd_ctrl self = {0};
	struct hikp_cmd_type type = {0};
	int ret;

	self.cmd_ptr = &type;
	ret = hikp_roce_set_mdb_bdf((char *)nic_name);
	if (ret) {
		HIKP_ERROR_PRINT("failed to set roce_mdb bdf for %s.\n",
				 (char *)nic_name);
		return ret;
	}

	printf("hikptool roce_mdb -i %s\n", (char *)nic_name);
	hikp_roce_set_mdb_mode(ROCE_MDB_CMD);
	hikp_roce_mdb_execute(&self);

	printf("hikptool roce_mdb -i %s -e\n", (char *)nic_name);
	hikp_roce_set_mdb_mode(ROCE_MDB_CMD_EXT);
	hikp_roce_mdb_execute(&self);

	return 0;
}

static int collect_hikp_roce_pkt_log(void *nic_name)
{
	struct major_cmd_ctrl self = {0};
	struct hikp_cmd_type type = {0};
	int ret;

	self.cmd_ptr = &type;
	ret = hikp_roce_set_pkt_bdf((char *)nic_name);
	if (ret) {
		HIKP_ERROR_PRINT("failed to set roce_pkt bdf for %s.\n",
				 (char *)nic_name);
		return ret;
	}

	printf("hikptool roce_pkt -i %s\n", (char *)nic_name);
	hikp_roce_pkt_execute(&self);

	return 0;
}

static int collect_hikp_roce_timer_log(void *nic_name)
{
	struct major_cmd_ctrl self = {0};
	struct hikp_cmd_type type = {0};
	int ret;

	self.cmd_ptr = &type;
	ret = hikp_roce_set_timer_bdf((char *)nic_name);
	if (ret) {
		HIKP_ERROR_PRINT("failed to set roce_timer bdf for %s.\n",
				 (char *)nic_name);
		return ret;
	}

	printf("hikptool roce_timer -i %s\n", (char *)nic_name);
	hikp_roce_timer_execute(&self);

	return 0;
}

static int collect_hikp_roce_rst_log(void *nic_name)
{
	struct major_cmd_ctrl self = {0};
	struct hikp_cmd_type type = {0};
	int ret;

	self.cmd_ptr = &type;
	ret = hikp_roce_set_rst_bdf((char *)nic_name);
	if (ret) {
		HIKP_ERROR_PRINT("failed to set roce_rst bdf for %s.\n",
				 (char *)nic_name);
		return ret;
	}

	printf("hikptool roce_rst -i %s\n", (char *)nic_name);
	hikp_roce_rst_execute(&self);

	return 0;
}

static int collect_hikp_roce_global_cfg_log(void *nic_name)
{
	struct major_cmd_ctrl self = {0};
	struct hikp_cmd_type type = {0};
	int ret;

	self.cmd_ptr = &type;
	ret = hikp_roce_set_global_cfg_bdf((char *)nic_name);
	if (ret) {
		HIKP_ERROR_PRINT("failed to set roce global_cfg bdf for %s.\n",
				 (char *)nic_name);
		return ret;
	}

	printf("hikptool roce_global_cfg -i %s\n", (char *)nic_name);
	hikp_roce_global_cfg_execute(&self);

	return 0;
}

static int collect_hikp_roce_bond_log(void *nic_name)
{
	struct major_cmd_ctrl self = {0};
	struct hikp_cmd_type type = {0};
	int ret;

	self.cmd_ptr = &type;
	ret = hikp_roce_set_bond_bdf((char *)nic_name);
	if (ret) {
		HIKP_ERROR_PRINT("failed to set roce bond bdf for %s.\n",
				 (char *)nic_name);
		return ret;
	}

	printf("hikptool roce_bond -i %s\n", (char *)nic_name);
	hikp_roce_bond_execute(&self);

	return 0;
}

static void collect_one_roce_hikp_log_compact(char *net_name, char *module,
					      collect_cmd_handler_t hikp_pfn)
{
	char log_name[LOG_FILE_PATH_MAX_LEN] = {0};
	int ret;

	ret = snprintf(log_name, LOG_FILE_PATH_MAX_LEN, "%s_%s", net_name,
		       module);
	if (ret < 0 || (uint32_t)ret >= LOG_FILE_PATH_MAX_LEN) {
		HIKP_ERROR_PRINT("failed to set %s path %d\n", net_name, ret);
		return;
	}

	ret = hikp_collect_log(GROUP_ROCE, log_name, hikp_pfn, (void *)net_name);
	if (ret)
		HIKP_ERROR_PRINT("failed to get %s info %d\n", net_name, ret);
}

static int collect_one_roce_hikp_log(void *net_name)
{
	struct collect_roce_hikp_log_meta {
		const char *module_name;
		collect_cmd_handler_t hikp_pfn;
	} roce_hikp_log_meta[] = {
		{ "roce_bond", collect_hikp_roce_bond_log },
		{ "roce_global_cfg", collect_hikp_roce_global_cfg_log },
		{ "roce_rst", collect_hikp_roce_rst_log },
		{ "roce_timer", collect_hikp_roce_timer_log },
		{ "roce_pkt", collect_hikp_roce_pkt_log },
		{ "roce_mdb", collect_hikp_roce_mdb_log },
		{ "roce_caep", collect_hikp_roce_caep_log },
		{ "roce_qmm", collect_hikp_roce_qmm_log },
		{ "roce_trp", collect_hikp_roce_trp_log },
		{ "roce_tsp", collect_hikp_roce_tsp_log },
		{ "roce_scc", collect_hikp_roce_scc_log },
		{ "roce_gmv", collect_hikp_roce_gmv_log },
		{ "roce_dfx_sta", collect_hikp_roce_dfx_sta_log },
	};
	size_t i;

	for (i = 0; i < HIKP_ARRAY_SIZE(roce_hikp_log_meta); ++i) {
		collect_one_roce_hikp_log_compact((char *)net_name,
					roce_hikp_log_meta[i].module_name,
					roce_hikp_log_meta[i].hikp_pfn);
	}

	return 0;
}

void collect_roce_log(void)
{
	collect_roce_devinfo_log();
	collect_roce_cc_param_log();
	collect_roce_sw_stats_log();
	collect_roce_res_stats_log();
	hikp_collect_all_nic_cmd_log(collect_one_roce_hikp_log);
}
