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

#include <dirent.h>
#include <unistd.h>

#include "hikp_collect_lib.h"
#include "hikp_collect.h"
#include "tool_lib.h"

struct info_collect_cmd acc_cmd_arr[] = {
	{
		.group = GROUP_ACC,
		.log_name = "uadk_version",
		.args = {"uadk_tool", "dfx", "--version", NULL},
	},
	{
		.group = GROUP_ACC,
		.log_name = "openssl_version",
		.args = {"openssl", "version", NULL},
	},
	{
		.group = GROUP_ACC,
		.log_name = "openssl3_version",
		.args = {"openssl3", "version", NULL},
	},
	{
		.group = GROUP_ACC,
		.log_name = "qemu_version",
		.args = {"qemu-system-aarch64", "-version", NULL},
	},
};

struct info_collect_cmd acc_cmd_copy[] = {
	{
		.group = GROUP_ACC,
		.log_name = "uadk",
		.args = {"cp", "-rf", "/var/log/uadk.log", NULL},
	},
	{
		.group = GROUP_ACC,
		.log_name = "acc_sec",
		.args = {"cp", "-rf", "/sys/kernel/debug/hisi_sec2/", NULL},
	},
	{
		.group = GROUP_ACC,
		.log_name = "acc_sec",
		.args = {"cp", "-rf", "/sys/module/hisi_sec2/parameters/", NULL},
	},
	{
		.group = GROUP_ACC,
		.log_name = "acc_hpre",
		.args = {"cp", "-rf", "/sys/kernel/debug/hisi_hpre/", NULL},
	},
	{
		.group = GROUP_ACC,
		.log_name = "acc_hpre",
		.args = {"cp", "-rf", "/sys/module/hisi_hpre/parameters/", NULL},
	},
	{
		.group = GROUP_ACC,
		.log_name = "acc_zip",
		.args = {"cp", "-rf", "/sys/kernel/debug/hisi_zip/", NULL},
	},
	{
		.group = GROUP_ACC,
		.log_name = "acc_zip",
		.args = {"cp", "-rf", "/sys/module/hisi_zip/parameters/", NULL},
	},
	{
		.group = GROUP_ACC,
		.log_name = "acc_trng",
		.args = {"cp", "-rf", "/sys/module/hisi_trng_v2/parameters/", NULL},
	},
};

struct info_collect_cmd acc_copy_link[] = {
	{
		.group = GROUP_ACC,
		.log_name = "uacce",
		.args = {"cp", "-rf", "/sys/class/uacce", NULL},
	}
};

static int acc_cmd_mkdir(char *root_path, char *src_patch)
{
	char dir_path[LOG_FILE_PATH_MAX_LEN] = {0};
	int ret;

	/* mkdir for log sub source */
	ret = snprintf(dir_path, LOG_FILE_PATH_MAX_LEN, "%s/%s",
				   root_path, src_patch);
	if (ret < 0 || (uint32_t)(ret) >= LOG_FILE_PATH_MAX_LEN) {
		HIKP_ERROR_PRINT("create dir path failed: %d\n", ret);
		return -EINVAL;
	}

	ret = tool_mk_dir((const char *)dir_path);
	if (ret)
		return ret;

	return 0;
}

static int acc_collect_file(struct info_collect_cmd *acc_cmd, char *root_path, char *dev_path)
{
	char sub_src_path[LOG_FILE_PATH_MAX_LEN] = {0};
	char save_path[LOG_FILE_PATH_MAX_LEN] = {0};
	struct info_collect_cmd tmp_cmd = {0};
	char *source_path = acc_cmd->args[ARGS_IDX2];
	char *sub_group = acc_cmd->log_name;
	int ret;

	ret = snprintf(sub_src_path, LOG_FILE_PATH_MAX_LEN, "%s/%s/",
				   source_path, dev_path);
	if (ret < 0 || (uint32_t)(ret) >= LOG_FILE_PATH_MAX_LEN) {
		HIKP_ERROR_PRINT("create sub source path failed: %d\n", ret);
		return -EINVAL;
	}
	if (access(sub_src_path, F_OK) != 0) {
		HIKP_ERROR_PRINT("Can't access sub source path: %s\n", sub_src_path);
		return -ENOENT;
	}

	ret = snprintf(save_path, LOG_FILE_PATH_MAX_LEN, "%s/%s/%s/",
				   root_path, sub_group, dev_path);
	if (ret < 0 || (uint32_t)(ret) >= LOG_FILE_PATH_MAX_LEN) {
		HIKP_ERROR_PRINT("create save path failed: %d\n", ret);
		return -EINVAL;
	}

	tmp_cmd.group = acc_cmd->group;
	tmp_cmd.log_name = acc_cmd->log_name;
	tmp_cmd.args[ARGS_IDX0] = acc_cmd->args[ARGS_IDX0];
	tmp_cmd.args[ARGS_IDX1] = acc_cmd->args[ARGS_IDX1];
	tmp_cmd.args[ARGS_IDX2] = sub_src_path;
	tmp_cmd.args[ARGS_IDX3] = save_path;
	tmp_cmd.args[ARGS_IDX4] = NULL;

	ret = hikp_collect_exec((void *)&tmp_cmd);
	if (ret)
		return ret;

	return 0;
}

static int acc_save_link_files(struct info_collect_cmd *link_cmd)
{
	char root_path[LOG_FILE_PATH_MAX_LEN] = {0};
	char *source_path = link_cmd->args[ARGS_IDX2];
	char *sub_group = link_cmd->log_name;
	struct dirent *dev_dir;
	DIR *link_dir;
	int ret;

	if (strcmp(link_cmd->args[ARGS_IDX0], "cp") != 0) {
		HIKP_ERROR_PRINT("input cmd failed: %s.\n", link_cmd->args[ARGS_IDX0]);
		return -EINVAL;
	}

	ret = hikp_get_file_path(root_path, LOG_FILE_PATH_MAX_LEN, link_cmd->group);
	if (ret < 0) {
		HIKP_ERROR_PRINT("get save path fail: %d\n", ret);
		return ret;
	}

	if (source_path == NULL || access(source_path, F_OK) != 0) {
		HIKP_ERROR_PRINT("Can't access source path: %s\n", source_path);
		return -ENOENT;
	}

	ret = acc_cmd_mkdir(root_path, sub_group);
	if (ret)
		return ret;

	link_dir = opendir(source_path);
	if (!link_dir) {
		HIKP_ERROR_PRINT("input source file dir is error!\n");
		return -ENOENT;
	}

	/* 1 is sizeof ".", 2 is sizeof ".." */
	while ((dev_dir = readdir(link_dir)) != NULL) {
		if (!strncmp(dev_dir->d_name, ".", 1) ||
		    !strncmp(dev_dir->d_name, "..", sizeof("..")))
			continue;

		ret = acc_collect_file(link_cmd, root_path, dev_dir->d_name);
		if (ret)
			goto free_dir;
	}

	closedir(link_dir);
	return 0;

free_dir:
	closedir(link_dir);
	return ret;
}

void collect_acc_log(void)
{
	int i, asize;
	int ret;

	asize = (int)HIKP_ARRAY_SIZE(acc_cmd_arr);
	for (i = 0; i < asize; i++) {
		ret = hikp_collect_log(acc_cmd_arr[i].group, acc_cmd_arr[i].log_name,
							   hikp_collect_exec, (void *)&acc_cmd_arr[i]);
		if (ret)
			HIKP_ERROR_PRINT("collect_acc_log arr failed: %d\n", ret);
	}

	asize = (int)HIKP_ARRAY_SIZE(acc_copy_link);
	for (i = 0; i < asize; i++) {
		ret = acc_save_link_files(&acc_copy_link[i]);
		if (ret)
			HIKP_ERROR_PRINT("collect_acc_log link copy failed: %d\n", ret);
	}

	asize = (int)HIKP_ARRAY_SIZE(acc_cmd_copy);
	for (i = 0; i < asize; i++) {
		ret = hikp_save_files(&acc_cmd_copy[i]);
		if (ret)
			HIKP_ERROR_PRINT("collect_acc_log copy failed: %d\n", ret);
	}
}
