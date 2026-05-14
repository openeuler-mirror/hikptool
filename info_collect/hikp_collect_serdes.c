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
#include "hikp_serdes.h"

#define MAX_CHIP_NUM_SUPPORT 8

struct macro_info_msg g_hip10 = {
	14, /* 14: macro_num */
	{
		LANE_NUM_4, LANE_NUM_4, LANE_NUM_8, LANE_NUM_8, LANE_NUM_8, LANE_NUM_8, LANE_NUM_8,
		LANE_NUM_4, LANE_NUM_4, LANE_NUM_8, LANE_NUM_8, LANE_NUM_8, LANE_NUM_8, LANE_NUM_8,
	}
};

struct macro_info_msg g_hip11 = {
	16, /* 16: macro_num */
	{
		LANE_NUM_4, LANE_NUM_8, LANE_NUM_4, LANE_NUM_4,
		LANE_NUM_4, LANE_NUM_8, LANE_NUM_4, LANE_NUM_4,
		LANE_NUM_4, LANE_NUM_8, LANE_NUM_4, LANE_NUM_4,
		LANE_NUM_4, LANE_NUM_8, LANE_NUM_4, LANE_NUM_4,
	}
};

static struct macro_info_msg *get_macro_info_from_firmware(void)
{
	struct chip_info_msg *chip_info = NULL;
	struct cmd_serdes_param cmd = {0};

	chip_info = hikp_serdes_get_chip_info(&cmd);
	if (chip_info != NULL) {
		return &chip_info->macro_info;
	}

	return NULL;
}

static struct macro_info_msg *serdes_get_macro_info(void)
{
	uint32_t chip_type = get_chip_type();

	switch (chip_type) {
		case CHIP_HIP09:
		case CHIP_HIP10:
		case CHIP_HIP10C:
			return &g_hip10;
		case CHIP_HIP11:
			return &g_hip11;
		case CHIP_HIP12:
			return get_macro_info_from_firmware();
		default:
			return NULL;
	}

	return NULL;
}

static int collect_serdes_info_process(void *data)
{
	struct cmd_serdes_param *cmd = (struct cmd_serdes_param *)data;
	struct macro_info_msg *tmp_macro_info = NULL;
	struct macro_info_msg macro_info;
	const char *info_cmd_str[] = {"", "-k"};
	/* 0, 1: brief info, detail info */
	unsigned char subcmd_list[] = {0, 1};
	unsigned char k, p;
	int ret;

	tmp_macro_info = serdes_get_macro_info();
	if (tmp_macro_info == NULL) {
		return -EINVAL;
	}
	memcpy(&macro_info, tmp_macro_info, sizeof(struct macro_info_msg));

	for (k = 0; k < macro_info.macro_num && k < SERDES_MACRO_NUM_MAX; k++) {
		cmd->macro_id = k;
		cmd->start_sds_id = 0;
		cmd->sds_num = macro_info.lane_num[k];
		for (p = 0; p < sizeof(subcmd_list) / sizeof(subcmd_list[0]); p++) {
			cmd->sub_cmd = subcmd_list[p];
			printf("hikptool serdes_info -i %u -s m%ud%u -n %u %s\n",
			       cmd->chip_id, cmd->macro_id, cmd->start_sds_id,
			       cmd->sds_num, info_cmd_str[cmd->sub_cmd]);
			ret = hikp_serdes_get_reponse(cmd);
			if (ret) {
				HIKP_ERROR_PRINT("collect chip%u macro%u "
						 "serdes_info%u failed: %d\n",
						 cmd->chip_id, k, subcmd_list[p], ret);
				return ret;
			}
		}
	}

	return 0;
}

static void collect_serdes_info_log(void)
{
	struct cmd_serdes_param serdes_info_cmd = {0};
	unsigned char chip_num = MAX_CHIP_NUM_SUPPORT;
	char log_name[MAX_LOG_NAME_LEN] = {0};
	unsigned char i;
	int ret;

	serdes_info_cmd.cmd_type = SERDES_KEY_INFO;
	for (i = 0; i < chip_num; i++) {
		serdes_info_cmd.chip_id = i;
		ret = snprintf(log_name, MAX_LOG_NAME_LEN, "serdes_info_c%u", i);
		if (ret < 0 || (uint32_t)(ret) >= MAX_LOG_NAME_LEN) {
			HIKP_ERROR_PRINT("create serdes_info log name failed\n");
			break;
		}
		ret = hikp_collect_log(GROUP_SERDES, log_name,
				       collect_serdes_info_process,
				       (void *)&serdes_info_cmd);
		if (ret) {
			HIKP_ERROR_PRINT("%s chip%u failed: %d\n", __func__, i, ret);
			break;
		}
	}
}

static int collect_serdes_dump_process(void *data)
{
	const char *dump_cmd_str[HILINK_DUMP_TYPE_END] = {"cs", "ds", "csds", "ram", "subctrl"};
	struct cmd_serdes_param *cmd = (struct cmd_serdes_param *)data;
	struct macro_info_msg *tmp_macro_info = NULL;
	struct macro_info_msg macro_info;
	unsigned char subcmd_list[] = {0, 1, 4}; /* 0, 1, 4: cs, ds, subctrl reg */
	unsigned char k, p, q;
	int ret;

	tmp_macro_info = serdes_get_macro_info();
	if (tmp_macro_info == NULL) {
		return -EINVAL;
	}
	memcpy(&macro_info, tmp_macro_info, sizeof(struct macro_info_msg));

	for (k = 0; k < macro_info.macro_num && k < SERDES_MACRO_NUM_MAX; k++) {
		cmd->macro_id = k;
		for (q = 0; q < macro_info.lane_num[k]; q++) {
			cmd->start_sds_id = q;
			cmd->sds_num = 1;
			for (p = 0; p < sizeof(subcmd_list) / sizeof(subcmd_list[0]); p++) {
				cmd->sub_cmd = subcmd_list[p];
				printf("hikptool serdes_dump -i %u -s m%ud%u -c %s\n",
				       cmd->chip_id, cmd->macro_id, cmd->start_sds_id,
				       dump_cmd_str[cmd->sub_cmd]);
				ret = hikp_serdes_get_reponse(cmd);
				if (ret) {
					HIKP_ERROR_PRINT("collect chip%u macro%u lane%u "
							 "serdes_dump%u failed: %d\n",
							 cmd->chip_id, k, q,
							 subcmd_list[p], ret);
					return ret;
				}
			}
		}
	}

	return 0;
}

static void collect_serdes_dump_log(void)
{
	struct cmd_serdes_param serdes_dump_cmd = {0};
	unsigned char chip_num = MAX_CHIP_NUM_SUPPORT;
	char log_name[MAX_LOG_NAME_LEN] = {0};
	unsigned char i;
	int ret;

	serdes_dump_cmd.cmd_type = SERDES_DUMP_REG;
	for (i = 0; i < chip_num; i++) {
		serdes_dump_cmd.chip_id = i;
		ret = snprintf(log_name, MAX_LOG_NAME_LEN, "serdes_dump_c%u", i);
		if (ret < 0 || (uint32_t)(ret) >= MAX_LOG_NAME_LEN) {
			HIKP_ERROR_PRINT("create serdes_dump log name failed\n");
			break;
		}
		ret = hikp_collect_log(GROUP_SERDES, log_name,
				       collect_serdes_dump_process,
				       (void *)&serdes_dump_cmd);
		if (ret) {
			HIKP_ERROR_PRINT("%s chip%u failed: %d\n", __func__, i, ret);
			break;
		}
	}
}

void collect_serdes_log(void)
{
	collect_serdes_info_log();
	collect_serdes_dump_log();
}
