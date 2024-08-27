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
#include "hikp_nic_log.h"
#include "hikp_nic_mac_dump.h"
#include "hikp_nic_port.h"
#include "hikp_nic_xsfp.h"
#include "hikp_nic_ncsi.h"

static int hikp_collect_nic_log_info(void *dev_name)
{
	struct info_collect_cmd nic_log_cmd = {
		.group = GROUP_IMP,
		.log_name = NULL,
		.args = {"mv", NULL, NULL},
	};
	struct nic_log_collect_param param = {0};
	char log_name[MAX_LOG_NAME_LEN] = {0};
	int ret;

	param.net_dev_name = (const char *)dev_name;
	ret = snprintf(log_name, MAX_LOG_NAME_LEN, "%s_nic_log", (char *)dev_name);
	if (ret < 0 || (uint32_t)ret >= MAX_LOG_NAME_LEN)
		return -EINVAL;

	ret = hikp_collect_log(GROUP_IMP, log_name, hikp_info_collect_nic_log, (void *)&param);
	if (ret) {
		HIKP_ERROR_PRINT("collect %s log failed: %d\n", log_name, ret);
		return ret;
	}

	nic_log_cmd.args[ARGS_IDX1] = (char *)hikp_info_collect_get_log_path();
	ret = hikp_move_files(&nic_log_cmd);
	if (ret)
		HIKP_ERROR_PRINT("collect imp log failed, %d\n", ret);

	return ret;
}

static int hikp_collect_nic_port_info(void *dev_name)
{
	struct nic_port_collect_param param = {0};
	char log_name[MAX_LOG_NAME_LEN] = {0};
	int ret;

	param.net_dev_name = (const char *)dev_name;
	ret = snprintf(log_name, MAX_LOG_NAME_LEN, "%s_nic_port", (char *)dev_name);
	if (ret < 0 || (uint32_t)ret >= MAX_LOG_NAME_LEN)
		return -EINVAL;

	ret = hikp_collect_log(GROUP_IMP, log_name, hikp_info_collect_nic_port, (void *)&param);
	if (ret)
		HIKP_ERROR_PRINT("collect %s log failed: %d\n", log_name, ret);

	return ret;
}

static int hikp_collect_nic_xsfp_info(void *dev_name)
{
	struct nic_xsfp_collect_param param = {0};
	char log_name[MAX_LOG_NAME_LEN] = {0};
	int ret;

	param.net_dev_name = (const char *)dev_name;
	ret = snprintf(log_name, MAX_LOG_NAME_LEN, "%s_nic_xsfp", (char *)dev_name);
	if (ret < 0 || (uint32_t)ret >= MAX_LOG_NAME_LEN)
		return -EINVAL;

	ret = hikp_collect_log(GROUP_IMP, log_name, hikp_info_collect_nic_xsfp, (void *)&param);
	if (ret)
		HIKP_ERROR_PRINT("collect %s log failed: %d\n", log_name, ret);

	return ret;
}

static int hikp_collect_nic_mac_info(void *dev_name)
{
	struct nic_mac_collect_param param = {0};
	char log_name[MAX_LOG_NAME_LEN] = {0};
	int ret;

	param.net_dev_name = (const char *)dev_name;
	param.module_name = "ALL";
	ret = snprintf(log_name, MAX_LOG_NAME_LEN, "%s_nic_mac", (char *)dev_name);
	if (ret < 0 || (uint32_t)ret >= MAX_LOG_NAME_LEN)
		return -EINVAL;

	ret = hikp_collect_log(GROUP_IMP, log_name, hikp_info_collect_nic_mac, (void *)&param);
	if (ret)
		HIKP_ERROR_PRINT("collect %s log failed: %d\n", log_name, ret);

	return ret;
}

static int hikp_collect_nic_ncsi_info(void *dev_name)
{
	struct nic_ncsi_collect_param param = {0};
	char log_name[MAX_LOG_NAME_LEN] = {0};
	int ret;

	param.net_dev_name = (const char *)dev_name;
	ret = snprintf(log_name, MAX_LOG_NAME_LEN, "%s_nic_ncsi", (char *)dev_name);
	if (ret < 0 || (uint32_t)ret >= MAX_LOG_NAME_LEN)
		return -EINVAL;

	ret = hikp_collect_log(GROUP_IMP, log_name, hikp_info_collect_nic_ncsi, (void *)&param);
	if (ret)
		HIKP_ERROR_PRINT("collect %s log failed: %d\n", log_name, ret);

	return ret;
}

void collect_imp_log(void)
{
	hikp_collect_all_nic_cmd_log(hikp_collect_nic_log_info);
	hikp_collect_all_nic_cmd_log(hikp_collect_nic_port_info);
	hikp_collect_all_nic_cmd_log(hikp_collect_nic_xsfp_info);
	hikp_collect_all_nic_cmd_log(hikp_collect_nic_mac_info);
	hikp_collect_all_nic_cmd_log(hikp_collect_nic_ncsi_info);
}
