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
#include "hikp_nic_fd.h"
#include "hikp_nic_gro.h"
#include "hikp_nic_ppp.h"
#include "hikp_nic_qos.h"
#include "hikp_nic_queue.h"
#include "hikp_nic_rss.h"
#include "hikp_nic_torus.h"
#include "hikp_nic_fec.h"
#include "hikp_nic_dfx.h"
#include "hikp_nic_info.h"
#include "hikp_nic_notify_pkt.h"
#include "hikp_nic_port_fault.h"
#include "hikp_nic_mac_dump.h"

static void collect_nic_debugfs_log(void)
{
	struct info_collect_cmd nic_cmd_copy = {
		.group = GROUP_NIC,
		.log_name = "debugfs",
		.args = {"cp", "-rf", "/sys/kernel/debug/hns3/", NULL},
	};
	int ret;

	ret = hikp_save_files(&nic_cmd_copy);
	if (ret)
		HIKP_ERROR_PRINT("collect_nic_log debugfs failed, %d\n", ret);
}

static int collect_hikp_nic_fd_log(void *nic_name)
{
	struct major_cmd_ctrl self = {0};
	struct hikp_cmd_type type = {0};
	int ret;

	/* collect nic_fd hw_info */
	printf("hikptool nic_fd -i %s -du hw_info\n", (char *)nic_name);
	hikp_nic_set_fd_idx(NIC_FD_HW_INFO_DUMP, -1);
	self.cmd_ptr = &type;
	ret = hikp_nic_cmd_get_fd_target(&self, (char *)nic_name);
	if (ret) {
		HIKP_ERROR_PRINT("failed to set bdf for %s.\n", (char *)nic_name);
		return ret;
	}
	hikp_nic_fd_cmd_execute(&self);

	/* collect nic_fd rules and counters */
	printf("hikptool nic_fd -i %s -du rules -st 1\n", (char *)nic_name);
	hikp_nic_set_fd_idx(NIC_FD_RULES_INFO_DUMP, 1);
	hikp_nic_fd_cmd_execute(&self);
	printf("hikptool nic_fd -i %s -du counter -st 1\n", (char *)nic_name);
	hikp_nic_set_fd_idx(NIC_FD_COUNTER_STATS_DUMP, 1);
	hikp_nic_fd_cmd_execute(&self);

	return 0;
}

static int collect_hikp_nic_gro_log(void *nic_name)
{
	struct major_cmd_ctrl self = {0};
	struct hikp_cmd_type type = {0};
	int ret;

	printf("hikptool nic_gro -i %s\n", (char *)nic_name);
	self.cmd_ptr = &type;
	ret = hikp_nic_gro_get_target(&self, (char *)nic_name);
	if (ret) {
		HIKP_ERROR_PRINT("failed to get bdf for %s.\n", (char *)nic_name);
		return ret;
	}

	hikp_nic_gro_cmd_execute(&self);
	return 0;
}

static int collect_hikp_nic_ppp_log(void *nic_name)
{
	const char *sub_cmd_name[] = {"mac", "vlan", "mng", "promisc", "vlan_offload"};
	struct major_cmd_ctrl self = {0};
	struct hikp_cmd_type type = {0};
	int i, ret;

	self.cmd_ptr = &type;
	ret = hikp_nic_cmd_get_ppp_target(&self, (char *)nic_name);
	if (ret) {
		HIKP_ERROR_PRINT("failed to get bdf for %s.\n", (char *)nic_name);
		return ret;
	}

	for (i = NIC_MAC_TBL_DUMP; i <= NIC_VLAN_OFFLOAD_DUMP; ++i) {
		printf("hikptool nic_ppp -i %s -du %s\n", (char *)nic_name,
		       sub_cmd_name[i - NIC_MAC_TBL_DUMP]);
		hikp_nic_ppp_set_cmd_param(i-1);
		hikp_nic_ppp_cmd_execute(&self);
	}

	return 0;
}

static int collect_hikp_nic_qos_log(void *nic_name)
{
	const char *sub_cmd_name[] = {"pkt_buf", "dcb", "pause"};
	const char *dir_name[] = {"rx", "tx"};
	struct major_cmd_ctrl self = {0};
	struct hikp_cmd_type type = {0};
	int i, ret;

	self.cmd_ptr = &type;
	ret = hikp_nic_cmd_get_qos_target(&self, (char *)nic_name);
	if (ret) {
		HIKP_ERROR_PRINT("failed to get bdf for %s.\n", (char *)nic_name);
		return ret;
	}

	for (i = NIC_PACKET_BUFFER_DUMP; i <= NIC_PAUSE_DUMP; ++i) {
		printf("hikptool nic_qos -i %s -g %s\n", (char *)nic_name, sub_cmd_name[i]);
		hikp_nic_qos_set_cmd_feature_idx(i);
		hikp_nic_qos_cmd_execute(&self);
	}

	hikp_nic_qos_set_cmd_feature_idx(NIC_PFC_STORM_PARA_DUMP);
	for (i = NIC_RX_QOS; i <= NIC_TX_QOS; ++i) {
		printf("hikptool nic_qos -i %s -g pfc_storm_para -d %s\n", (char *)nic_name,
		       dir_name[i]);
		hikp_nic_qos_set_cmd_direction(i);
		hikp_nic_qos_cmd_execute(&self);
	}

	return 0;
}

static int collect_hikp_nic_queue_log(void *nic_name)
{
	const char *dir_name[] = {"tx", "rx"};
	struct major_cmd_ctrl self = {0};
	struct hikp_cmd_type type = {0};
	int j, ret;

	self.cmd_ptr = &type;
	ret = hikp_nic_cmd_get_queue_target(&self, (char *)nic_name);
	if (ret) {
		HIKP_ERROR_PRINT("failed to get bdf for %s.\n", (char *)nic_name);
		return ret;
	}

	printf("hikptool nic_queue -i %s -du queue_en -a on\n", (char *)nic_name);
	hikp_nic_queue_cmd_set_param(QUEUE_EN_INFO, -1, NIC_QUEUE_DIR_UNKNOWN);
	hikp_nic_queue_cmd_execute(&self);
	printf("hikptool nic_queue -i %s -du func_map\n", (char *)nic_name);
	hikp_nic_queue_cmd_set_param(QUEUE_FUNC_MAP, -1, NIC_QUEUE_DIR_UNKNOWN);
	hikp_nic_queue_cmd_execute(&self);

	for (j = NIC_TX_QUEUE; j <= NIC_RX_QUEUE; ++j) {
		printf("hikptool nic_queue -i %s -du basic_info -d %s -q 0\n", (char *)nic_name,
		       dir_name[j]);
		hikp_nic_queue_cmd_set_param(QUEUE_BASIC_INFO, 0, j);
		hikp_nic_queue_cmd_execute(&self);
		printf("hikptool nic_queue -i %s -du intr_map -d %s -a on\n", (char *)nic_name,
		       dir_name[j]);
		hikp_nic_queue_cmd_set_param(QUEUE_INTR_MAP, -1, j);
		hikp_nic_queue_cmd_execute(&self);
	}

	return 0;
}

static int collect_hikp_nic_rss_log(void *nic_name)
{
	const char *sub_cmd_name[] = {"algo", "key", "tuple", "reta", "tc_mode"};
	struct major_cmd_ctrl self = {0};
	struct hikp_cmd_type type = {0};
	int i, ret;

	self.cmd_ptr = &type;
	ret = hikp_nic_cmd_get_rss_target(&self, (char *)nic_name);
	if (ret) {
		HIKP_ERROR_PRINT("failed to get bdf for %s.\n", (char *)nic_name);
		return ret;
	}

	for (i = RSS_ALGO_DUMP; i <= RSS_TC_MODE_DUMP; ++i) {
		printf("hikptool nic_rss -i %s -g %s\n", (char *)nic_name, sub_cmd_name[i]);
		hikp_nic_rss_cmd_set_feature_idx(i);
		hikp_nic_rss_cmd_execute(&self);
	}

	return 0;
}

static int collect_hikp_nic_torus_log(void *nic_name)
{
	struct major_cmd_ctrl self = {0};
	struct hikp_cmd_type type = {0};
	int ret;

	self.cmd_ptr = &type;
	ret = hikp_nic_torus_get_target(&self, (char *)nic_name);
	if (ret) {
		HIKP_ERROR_PRINT("failed to get bdf for %s.\n", (char *)nic_name);
		return ret;
	}

	printf("hikptool nic_torus -i %s\n", (char *)nic_name);
	hikp_nic_torus_cmd_execute(&self);
	return 0;
}

static int collect_hikp_nic_fec_log(void *nic_name)
{
	struct major_cmd_ctrl self = {0};
	struct hikp_cmd_type type = {0};
	int ret;

	self.cmd_ptr = &type;
	ret = hikp_nic_fec_get_target(&self, (char *)nic_name);
	if (ret) {
		HIKP_ERROR_PRINT("failed to get bdf for %s.\n", (char *)nic_name);
		return ret;
	}

	printf("hikptool nic_fec -i %s\n", (char *)nic_name);
	hikp_nic_fec_cmd_execute(&self);
	return 0;
}

static int collect_hikp_nic_dfx_log(void *nic_name)
{
	const char *sub_cmd_name[] = {"SSU", "IGU_EGU", "PPP", "NCSI", "BIOS", "RCB", "TXDMA",
				      "MASTER"};
	struct major_cmd_ctrl self = {0};
	struct hikp_cmd_type type = {0};
	int i, ret;

	self.cmd_ptr = &type;
	ret = hikp_nic_cmd_dfx_target(&self, (char *)nic_name);
	if (ret) {
		HIKP_ERROR_PRINT("failed to get bdf for %s.\n", (char *)nic_name);
		return ret;
	}
	for (i = SSU_DFX_REG_DUMP; i <= MASTER_DFX_REG_DUMP; ++i) {
		printf("hikptool nic_dfx -i %s -m %s\n", (char *)nic_name, sub_cmd_name[i]);
		hikp_nic_dfx_set_cmd_para(i);
		hikp_nic_dfx_cmd_execute(&self);
	}

	return 0;
}

static int collect_hikp_nic_info_log(void *nic_name)
{
	struct major_cmd_ctrl self = {0};
	struct hikp_cmd_type type = {0};
	int ret;

	printf("hikptool nic_info -i %s\n", (char *)nic_name);
	self.cmd_ptr = &type;
	ret = hikp_nic_cmd_get_info_target(&self, (char *)nic_name);
	if (ret) {
		HIKP_ERROR_PRINT("failed to get bdf for %s.\n", (char *)nic_name);
		return ret;
	}

	hikp_nic_info_cmd_execute(&self);
	return 0;
}

static int collect_hikp_nic_notify_pkt_log(void *nic_name)
{
	struct major_cmd_ctrl self = {0};
	struct hikp_cmd_type type = {0};
	int ret;

	printf("hikptool nic_notify_pkt -i %s\n", (char *)nic_name);
	self.cmd_ptr = &type;
	ret = hikp_nic_notify_pkt_get_target(&self, (char *)nic_name);
	if (ret) {
		HIKP_ERROR_PRINT("failed to get bdf for %s.\n", (char *)nic_name);
		return ret;
	}

	hikp_nic_notify_pkt_cmd_execute(&self);
	return 0;
}

static int collect_hikp_nic_port_fault_log(void *nic_name)
{
	struct major_cmd_ctrl self = {0};
	struct hikp_cmd_type type = {0};
	int ret;

	printf("hikptool nic_port_fault -i %s\n", (char *)nic_name);
	self.cmd_ptr = &type;
	ret = hikp_nic_port_fault_get_target(&self, (char *)nic_name);
	if (ret) {
		HIKP_ERROR_PRINT("failed to get bdf for %s.\n", (char *)nic_name);
		return ret;
	}

	hikp_nic_port_fault_cmd_execute(&self);
	return 0;
}

static int collect_one_nic_hikp_log_compact(char *net_name, char *module, collect_cmd_handler_t hikp_pfn)
{
	char log_name[LOG_FILE_PATH_MAX_LEN] = {0};
	int ret;

	ret = snprintf(log_name, LOG_FILE_PATH_MAX_LEN, "%s_%s", net_name, module);
	if (ret < 0 || (uint32_t)ret >= LOG_FILE_PATH_MAX_LEN)
		return -EINVAL;

	return hikp_collect_log(GROUP_NIC, log_name, hikp_pfn, (void *)net_name);
}

static int collect_one_nic_hikp_log(void *net_name)
{
	struct collect_nic_hikp_log_meta {
		const char *module_name;
		collect_cmd_handler_t hikp_pfn;
	} nic_hikp_log_meta[] = {
		{ "nic_fd", collect_hikp_nic_fd_log },
		{ "nic_gro", collect_hikp_nic_gro_log },
		{ "nic_ppp", collect_hikp_nic_ppp_log },
		{ "nic_qos", collect_hikp_nic_qos_log },
		{ "nic_queue", collect_hikp_nic_queue_log },
		{ "nic_rss", collect_hikp_nic_rss_log },
		{ "nic_torus", collect_hikp_nic_torus_log },
		{ "nic_fec", collect_hikp_nic_fec_log },
		{ "nic_dfx", collect_hikp_nic_dfx_log },
		{ "nic_info", collect_hikp_nic_info_log },
		{ "nic_notify_pkt", collect_hikp_nic_notify_pkt_log },
		{ "nic_port_fault", collect_hikp_nic_port_fault_log },
	};
	size_t i;
	int ret;

	for (i = 0; i < HIKP_ARRAY_SIZE(nic_hikp_log_meta); ++i) {
		ret = collect_one_nic_hikp_log_compact((char *)net_name,
					nic_hikp_log_meta[i].module_name,
					nic_hikp_log_meta[i].hikp_pfn);
		if (ret)
			HIKP_ERROR_PRINT("collect %s log failed: %d\n",
					 nic_hikp_log_meta[i].module_name, ret);
	}

	return 0;
}

void collect_nic_log(void)
{
	collect_nic_debugfs_log();
	hikp_collect_all_nic_cmd_log(collect_one_nic_hikp_log);
}
