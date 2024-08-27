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
#define HIP10_DIE_NUM        2
#define HIP11_DIE_NUM        4
#define HIP10_DIE_MACRO_NUM  7
#define HIP11_DIE_MACRO_NUM  4

struct serdes_macro_info {
	uint8_t macro_id;
	uint8_t ds_num;
};

struct serdes_log_cmd {
	struct cmd_serdes_param *cmd;
	unsigned char die_id;
};

struct serdes_macro_info g_hip10[] = {
	{0, 4}, /* 0, 4: macro_id, ds_num */
	{1, 4}, /* 1, 4: macro_id, ds_num */
	{2, 8}, /* 2, 8: macro_id, ds_num */
	{3, 8}, /* 3, 8: macro_id, ds_num */
	{4, 8}, /* 4, 8: macro_id, ds_num */
	{5, 8}, /* 5, 8: macro_id, ds_num */
	{6, 8}, /* 6, 8: macro_id, ds_num */
};

struct serdes_macro_info g_hip11[] = {
	{0, 4}, /* 0, 4: macro_id, ds_num */
	{1, 8}, /* 1, 8: macro_id, ds_num */
	{2, 4}, /* 2, 4: macro_id, ds_num */
	{3, 4}, /* 3, 4: macro_id, ds_num */
};

static int is_chip_hip11(void)
{
	return get_chip_type() == CHIP_HIP11;
}

static unsigned char serdes_get_die_num(void)
{
	return is_chip_hip11() ? HIP11_DIE_NUM : HIP10_DIE_NUM;
}

static unsigned char serdes_get_die_macro_num(void)
{
	return is_chip_hip11() ? HIP11_DIE_MACRO_NUM : HIP10_DIE_MACRO_NUM;
}

static struct serdes_macro_info *serdes_get_macro_info(void)
{
	return is_chip_hip11() ? g_hip11 : g_hip10;
}

static int collect_serdes_info_process(void *data)
{
	struct serdes_log_cmd *log_cmd = (struct serdes_log_cmd *)data;
	struct serdes_macro_info *macro_info = serdes_get_macro_info();
	unsigned char die_macro_num = serdes_get_die_macro_num();
	struct cmd_serdes_param *cmd = log_cmd->cmd;
	const char *info_cmd_str[] = {"", "-k"};
	unsigned char die_id = log_cmd->die_id;
	/* 0, 1: brief info, detail info */
	unsigned char subcmd_list[] = {0, 1};
	unsigned char k, p;
	int ret;

	for (k = 0; k < die_macro_num; k++) {
		cmd->macro_id = die_id * die_macro_num + k;
		cmd->start_sds_id = 0;
		cmd->sds_num = macro_info[k].ds_num;
		for (p = 0; p < sizeof(subcmd_list) / sizeof(subcmd_list[0]); p++) {
			cmd->sub_cmd = subcmd_list[p];
			printf("hikptool serdes_info -i %u -s m%ud%u -n %u %s\n",
			       cmd->chip_id, cmd->macro_id, cmd->start_sds_id,
			       cmd->sds_num, info_cmd_str[cmd->sub_cmd]);
			ret = hikp_serdes_get_reponse(cmd);
			if (ret) {
				HIKP_ERROR_PRINT("collect chip%u die%u macro%u "
						 "serdes_info%u failed: %d\n",
						 cmd->chip_id, die_id, k, subcmd_list[p], ret);

				if (ret == -EINVAL)
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
	unsigned char die_num = serdes_get_die_num();
	char log_name[MAX_LOG_NAME_LEN] = {0};
	struct serdes_log_cmd log_cmd = {0};
	bool stop = false;
	unsigned char i, j;
	int ret;

	serdes_info_cmd.cmd_type = SERDES_KEY_INFO;
	for (i = 0; i < chip_num; i++) {
		for (j = 0; j < die_num; j++) {
			serdes_info_cmd.chip_id = i;
			log_cmd.cmd = &serdes_info_cmd;
			log_cmd.die_id = j;
			ret = snprintf(log_name, MAX_LOG_NAME_LEN, "serdes_info_c%ud%u", i, j);
			if (ret < 0 || (uint32_t)(ret) >= MAX_LOG_NAME_LEN) {
				HIKP_ERROR_PRINT("create serdes_info log name failed\n");
				break;
			}
			ret = hikp_collect_log(GROUP_SERDES, log_name,
					       collect_serdes_info_process,
					       (void *)&log_cmd);
			if (ret) {
				/*
				 * Stop collection when the die id is 0 and ret is -EINVAL,
				 * indicating that the current chip id is not supported.
				 */
				if (j == 0 && ret == -EINVAL)
					stop = true;

				if (ret == -EINVAL)
					break;

				HIKP_ERROR_PRINT("%s chip%u die%u failed: %d\n", __func__,
						 i, j, ret);
			}
		}

		if (stop)
			break;
	}
}

static int collect_serdes_dump_process(void *data)
{
	const char *dump_cmd_str[HILINK_DUMP_TYPE_END] = {"cs", "ds", "csds", "ram", "subctrl"};
	struct serdes_log_cmd *log_cmd = (struct serdes_log_cmd *)data;
	struct serdes_macro_info *macro_info = serdes_get_macro_info();
	unsigned char die_macro_num = serdes_get_die_macro_num();
	struct cmd_serdes_param *cmd = log_cmd->cmd;
	unsigned char die_id = log_cmd->die_id;
	unsigned char subcmd_list[] = {0, 1, 4}; /* 0, 1, 4: cs, ds, subctrl reg */
	unsigned char k, p, q;
	int ret;

	for (k = 0; k < die_macro_num; k++) {
		cmd->macro_id = die_id * die_macro_num + k;
		for (q = 0; q < macro_info[k].ds_num; q++) {
			cmd->start_sds_id = q;
			cmd->sds_num = 1;
			for (p = 0; p < sizeof(subcmd_list) / sizeof(subcmd_list[0]); p++) {
				cmd->sub_cmd = subcmd_list[p];
				printf("hikptool serdes_dump -i %u -s m%ud%u -c %s\n",
				       cmd->chip_id, cmd->macro_id, cmd->start_sds_id,
				       dump_cmd_str[cmd->sub_cmd]);
				ret = hikp_serdes_get_reponse(cmd);
				if (ret) {
					HIKP_ERROR_PRINT("collect chip%u die%u macro%u lane%u "
							 "serdes_dump%u failed: %d\n",
							 cmd->chip_id, die_id, k, q,
							 subcmd_list[p], ret);

					if (ret == -EINVAL)
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
	unsigned char die_num = serdes_get_die_num();
	struct serdes_log_cmd log_cmd = {0};
	char log_name[MAX_LOG_NAME_LEN] = {0};
	unsigned char i, j;
	bool stop = false;
	int ret;

	serdes_dump_cmd.cmd_type = SERDES_DUMP_REG;
	for (i = 0; i < chip_num; i++) {
		for (j = 0; j < die_num; j++) {
			serdes_dump_cmd.chip_id = i;
			log_cmd.cmd = &serdes_dump_cmd;
			log_cmd.die_id = j;
			ret = snprintf(log_name, MAX_LOG_NAME_LEN, "serdes_dump_c%ud%u", i, j);
			if (ret < 0 || (uint32_t)(ret) >= MAX_LOG_NAME_LEN) {
				HIKP_ERROR_PRINT("create serdes_info log name failed\n");
				break;
			}
			ret = hikp_collect_log(GROUP_SERDES, log_name,
					       collect_serdes_dump_process,
					       (void *)&log_cmd);
			if (ret) {
				/*
				 * Stop collection when the die id is 0 and ret is -EINVAL,
				 * indicating that the current chip id is not supported.
				 */
				if (j == 0 && ret == -EINVAL)
					stop = true;

				if (ret == -EINVAL)
					break;

				HIKP_ERROR_PRINT("%s chip%u die%u failed: %d\n", __func__,
						 i, j, ret);
			}
		}

		if (stop)
			break;
	}
}

void collect_serdes_log(void)
{
	collect_serdes_info_log();
	collect_serdes_dump_log();
}
