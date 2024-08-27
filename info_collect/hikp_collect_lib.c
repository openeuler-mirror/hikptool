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
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <time.h>
#include <glob.h>
#include <dirent.h>
#include "tool_lib.h"

static char log_save_path[LOG_FILE_PATH_MAX_LEN] = {0};
static char g_collect_name[MAX_LOG_NAME_LEN] = {0};

static bool hikp_nic_drv_check(char *nic_name)
{
	char drv_path[LOG_FILE_PATH_MAX_LEN] = {0};
	struct dirent *ptr = NULL;
	bool hns3_if = false;
	DIR *dir = NULL;
	int ret;

	ret = snprintf(drv_path, LOG_FILE_PATH_MAX_LEN, "%s%s/%s", HIKP_NIC_NAME_DIR,
		       nic_name, HIKP_NIC_DRV_DIR);
	if (ret < 0 || (uint32_t)(ret) >= LOG_FILE_PATH_MAX_LEN)
		return false;

	if ((dir = opendir(drv_path)) == NULL)
		return false;

	while ((ptr = readdir(dir)) != NULL) {
		if (strcmp(ptr->d_name, HIKP_NIC_DRV_NAME) == 0) {
			hns3_if = true;
			break;
		}
	}

	closedir(dir);
	return hns3_if;
}

void hikp_collect_all_nic_cmd_log(collect_cmd_handler_t hikp_collect_one_nic_log)
{
	struct dirent *ptr = NULL;
	DIR *dir = NULL;
	int ret;

	if (!hikp_collect_one_nic_log) {
		HIKP_ERROR_PRINT("Invalid parameters!\n");
		return;
	}

	dir = opendir(HIKP_NIC_NAME_DIR);
	if (!dir) {
		HIKP_ERROR_PRINT("failed to open path!\n");
		return;
	}

	while ((ptr = readdir(dir)) != NULL) {
		if ((strcmp(ptr->d_name, ".") == 0) || (strcmp(ptr->d_name, "..") == 0)) {
			continue;
		} else if (ptr->d_type == DT_LNK) {
			if (hikp_nic_drv_check(ptr->d_name)) {
				ret = hikp_collect_one_nic_log((void *)ptr->d_name);
				if (ret) {
					HIKP_WARN_PRINT("failed to collect log for: %s", ptr->d_name);
					break;
				}
			}
		}
	}

	closedir(dir);
}

static bool is_cmd_valid(struct info_collect_cmd *cmd, unsigned int args_num)
{
	unsigned int i;

	if (cmd == NULL || cmd->log_name == NULL || args_num == 0 || args_num >= ARGS_MAX_NUM)
		return false;

	for (i = 0; i < args_num; i++)
		if (cmd->args[i] == NULL)
			return false;

	return true;
}

int hikp_create_save_path(const char *name)
{
	char collect_name[MAX_LOG_NAME_LEN] = {0};
	time_t time_seconds = time(0);
	struct tm timeinfo;
	int ret;

	localtime_r(&time_seconds, &timeinfo);
	if (name != NULL)
		(void)snprintf((char *)collect_name, MAX_LOG_NAME_LEN,
			"collect_%s_%04d%02d%02d%02d%02d%02d",
			name, timeinfo.tm_year + START_YEAR,
			timeinfo.tm_mon + 1, timeinfo.tm_mday,
			timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
	else
		(void)snprintf((char *)collect_name, MAX_LOG_NAME_LEN,
			"collect_%04d%02d%02d%02d%02d%02d",
			timeinfo.tm_year + START_YEAR, timeinfo.tm_mon + 1,
			timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min,
			timeinfo.tm_sec);

	ret = snprintf((char *)log_save_path, LOG_FILE_PATH_MAX_LEN,
			HIKP_COLLECT_LOG_DIR_PATH"%s", collect_name);
	if (ret < 0 || (uint32_t)(ret) >= LOG_FILE_PATH_MAX_LEN)
		return -EINVAL;

	if (!is_dir_exist((const char*)log_save_path)) {
		if (tool_mk_dir((const char*)log_save_path)) {
			HIKP_ERROR_PRINT("mkdir %s failed, errno is %d\n",
					log_save_path, errno);
			memset(log_save_path, 0, LOG_FILE_PATH_MAX_LEN);
			return -ENOENT;
		}
	}

	ret = snprintf((char *)g_collect_name, MAX_LOG_NAME_LEN, "%s", collect_name);
	if (ret < 0 || (uint32_t)(ret) >= MAX_LOG_NAME_LEN)
		return -EINVAL;

	return 0;
}

static int hikp_get_save_path(char *file_path, unsigned int file_path_len)
{
	int ret;

	if (access(log_save_path, F_OK) != 0)
		return -ENOENT;

	ret = snprintf((char *)file_path, file_path_len, "%s", log_save_path);
	if (ret < 0 || (uint32_t)(ret) >= LOG_FILE_PATH_MAX_LEN)
		return -EINVAL;

	return 0;
}

int hikp_get_file_path(char *file_path, uint32_t file_path_len,
		char *group)
{
	char tmp_path[LOG_FILE_PATH_MAX_LEN] = {0};
	int ret;

	ret = hikp_get_save_path(tmp_path, LOG_FILE_PATH_MAX_LEN);
	if (ret < 0) {
		HIKP_ERROR_PRINT("get file save path failed: %d\n", ret);
		return ret;
	}

	if (group != NULL)
		ret = snprintf(file_path, file_path_len, "%s/%s",
				tmp_path, group);
	else
		ret = snprintf(file_path, file_path_len, "%s",
				tmp_path);

	if (ret < 0 || (uint32_t)(ret) >= LOG_FILE_PATH_MAX_LEN) {
		HIKP_ERROR_PRINT("create file path fail: %d\n", ret);
		return -EINVAL;
	}

	if (!is_dir_exist((const char*)file_path)) {
		if (tool_mk_dir((const char*)file_path)) {
			HIKP_ERROR_PRINT("mkdir %s failed: %d\n",
					file_path, errno);
			return -ENOENT;
		}
	}

	return 0;
}

static int hikp_collect_cmd_exec(const struct info_collect_cmd *cmd)
{
	pid_t pid;
	int status;

	pid = fork();
	if (pid == 0) {
		/*
		 * When the command execution fails, exit the child
		 * process just like when it succeeds.
		 * */
		if (execvp(cmd->args[ARGS_IDX0], cmd->args) < 0) {
			HIKP_ERROR_PRINT("execvp failed: %d\n", errno);
			exit(EXIT_FAILURE);
		}
	} else if (pid > 0) {
		/* Parent process */
		waitpid(pid, &status, 0);
	} else {
		HIKP_ERROR_PRINT("fork failed!\n");
		return -ECHILD;
	}

	return 0;
}

int hikp_collect_exec(void *data)
{
	struct info_collect_cmd *cmd = (struct info_collect_cmd *)data;
	struct info_collect_cmd echo_cmd = {0};
	char result[MAX_LOG_NAME_LEN] = {0};
	int len = 0;
	int ret;
	int i;

	echo_cmd.args[ARGS_IDX0] = "echo";
	echo_cmd.args[ARGS_IDX1] = result;
	echo_cmd.args[ARGS_IDX2] = NULL;
	for (i = 0; i < ARGS_MAX_NUM && cmd->args[i] != NULL; i++) {
		ret = snprintf(result + len,
				MAX_LOG_NAME_LEN - len - 1,
				"%s ", cmd->args[i]);
		len += ret;
		if (ret < 0 || len >= (MAX_LOG_NAME_LEN - 1)) {
			HIKP_INFO_PRINT("Error getting command args");
			break;
		}
	}

	ret = hikp_collect_cmd_exec(&echo_cmd);
	if (ret)
		return ret;

	return hikp_collect_cmd_exec(cmd);
}

int hikp_collect_log(char *group, char *log_name, collect_cmd_handler_t func, void *data)
{
	unsigned char file_name[MAX_LOG_NAME_LEN] = {0};
	char file_dir[LOG_FILE_PATH_MAX_LEN] = {0};
	char file_path[LOG_FILE_PATH_MAX_LEN] = {0};
	int stdout_fd = dup(STDOUT_FILENO);
	int stderr_fd = dup(STDERR_FILENO);
	FILE *log_file;
	int ret;

	if (log_name == NULL) {
		HIKP_ERROR_PRINT("log name is NULL");
		return -EINVAL;
	}

	ret = generate_file_name(file_name, MAX_LOG_NAME_LEN,
			(const unsigned char*)log_name);
	if (ret < 0)
		return ret;

	ret = hikp_get_file_path(file_dir, LOG_FILE_PATH_MAX_LEN, group);
	if (ret < 0)
		return ret;

	ret = snprintf(file_path, LOG_FILE_PATH_MAX_LEN, "%s/%s", file_dir, file_name);
	if (ret < 0 || (uint32_t)(ret) >= LOG_FILE_PATH_MAX_LEN) {
                HIKP_ERROR_PRINT("create log file path fail: %d\n", ret);
                return -EINVAL;
	}

	log_file = fopen(file_path, "a");
	if (log_file == NULL) {
		HIKP_ERROR_PRINT("open %s failed.", file_path);
		return -ENOENT;
	 }

	(void)chmod(file_path, S_IRUSR | S_IWUSR | S_IRGRP);

	fflush(stdout);
	fflush(stderr);

	/* Copy the standard output and error information to log_file */
	dup2(fileno(log_file), STDOUT_FILENO);
	dup2(fileno(log_file), STDERR_FILENO);

	ret = func(data);

	fflush(stdout);
	fflush(stderr);

	/* Redirect stdout and stderr back to the terminal */
	dup2(stdout_fd, STDOUT_FILENO);
	dup2(stderr_fd, STDERR_FILENO);

	fclose(log_file);
	close(stdout_fd);
	close(stderr_fd);

	return ret;
}

int hikp_compress_log(void)
{
	char file_path[LOG_FILE_PATH_MAX_LEN] = {0};
	char tar_name[LOG_FILE_PATH_MAX_LEN] = {0};
	struct info_collect_cmd tar_cmd = {0};
	struct info_collect_cmd rm_cmd = {0};
	int ret;

	ret = hikp_get_save_path(file_path, LOG_FILE_PATH_MAX_LEN);
	if (ret < 0) {
		HIKP_ERROR_PRINT("get save path fail: %d\n", ret);
		return ret;
	}

	ret = snprintf((char *)tar_name, LOG_FILE_PATH_MAX_LEN, "%s.tar.gz",
			file_path);
	if (ret < 0 || (uint32_t)(ret) >= LOG_FILE_PATH_MAX_LEN) {
		HIKP_ERROR_PRINT("create tar path fail: %d\n", ret);
		return -EINVAL;
	}

	tar_cmd.args[ARGS_IDX0] = "tar";
	tar_cmd.args[ARGS_IDX1] = "-zPcf";
	tar_cmd.args[ARGS_IDX2] = tar_name;
	tar_cmd.args[ARGS_IDX3] = "-C";
	tar_cmd.args[ARGS_IDX4] = HIKP_COLLECT_LOG_DIR_PATH;
	tar_cmd.args[ARGS_IDX5] = g_collect_name;
	tar_cmd.args[ARGS_IDX6] = NULL;

	ret = hikp_collect_exec(&tar_cmd);
	if (ret)
		return ret;

	/* Delete the original log after the log is packaged. */
	rm_cmd.args[ARGS_IDX0] = "rm";
	rm_cmd.args[ARGS_IDX1] = "-rf";
	rm_cmd.args[ARGS_IDX2] = file_path;
	rm_cmd.args[ARGS_IDX3] = NULL;

	return hikp_collect_exec(&rm_cmd);
}

int hikp_move_files(struct info_collect_cmd *mv_cmd)
{
	char dest_path[LOG_FILE_PATH_MAX_LEN] = {0};
	char tmp_path[LOG_FILE_PATH_MAX_LEN] = {0};
	char *src_path = mv_cmd->args[ARGS_IDX1];
	char *sub_group = mv_cmd->log_name;
	int ret;

	if (strcmp(mv_cmd->args[ARGS_IDX0], "mv") != 0) {
		HIKP_ERROR_PRINT("input cmd failed: %s.\n", mv_cmd->args[ARGS_IDX0]);
		return -EINVAL;
	}

	if (src_path == NULL || access(src_path, F_OK) != 0) {
		HIKP_ERROR_PRINT("Can't access source path: %s\n", src_path);
		return -ENOENT;
	}

	ret = hikp_get_file_path(tmp_path, LOG_FILE_PATH_MAX_LEN, mv_cmd->group);
	if (ret < 0) {
		HIKP_ERROR_PRINT("get destination path fail: %d\n", ret);
		return ret;
	}

	if (sub_group == NULL)
		ret = snprintf(dest_path, LOG_FILE_PATH_MAX_LEN, "%s", tmp_path);
	else
		ret = snprintf(dest_path, LOG_FILE_PATH_MAX_LEN, "%s/%s",
			       tmp_path, sub_group);
	if (ret < 0 || (uint32_t)(ret) >= LOG_FILE_PATH_MAX_LEN) {
		HIKP_ERROR_PRINT("create destination path failed: %d\n", ret);
		return -EINVAL;
	}

	mv_cmd->args[ARGS_IDX2] = dest_path;
	mv_cmd->args[ARGS_IDX3] = NULL;

	return hikp_collect_exec((void *)mv_cmd);
}

int hikp_save_files(struct info_collect_cmd *save_cmd)
{
	char save_path[LOG_FILE_PATH_MAX_LEN] = {0};
	char tmp_path[LOG_FILE_PATH_MAX_LEN] = {0};
	char *source_path = save_cmd->args[ARGS_IDX2];
	char *sub_group = save_cmd->log_name;
	int ret;

	if (strcmp(save_cmd->args[ARGS_IDX0], "cp") != 0) {
		HIKP_ERROR_PRINT("input cmd failed: %s.\n", save_cmd->args[ARGS_IDX0]);
		return -EINVAL;
	}

	if (source_path == NULL || access(source_path, F_OK) != 0) {
		HIKP_ERROR_PRINT("Can't access source path: %s\n", source_path);
		return -ENOENT;
	}

	ret = hikp_get_file_path(tmp_path, LOG_FILE_PATH_MAX_LEN, save_cmd->group);
	if (ret < 0) {
		HIKP_ERROR_PRINT("get save path fail: %d\n", ret);
		return ret;
	}

	if (sub_group == NULL)
		ret = snprintf(save_path, LOG_FILE_PATH_MAX_LEN, "%s", tmp_path);
	else
		ret = snprintf(save_path, LOG_FILE_PATH_MAX_LEN, "%s/%s",
					   tmp_path, sub_group);
	if (ret < 0 || (uint32_t)(ret) >= LOG_FILE_PATH_MAX_LEN) {
		HIKP_ERROR_PRINT("create save path failed: %d\n", ret);
		return -EINVAL;
	}

	save_cmd->args[ARGS_IDX3] = save_path;
	save_cmd->args[ARGS_IDX4] = NULL;

	return hikp_collect_exec((void *)save_cmd);
}

int hikp_collect_cat_glob_exec(void *data)
{
	struct info_collect_cmd *cmd = (struct info_collect_cmd *)data;
	glob_t glb_buff;
	size_t i;
	int ret;

	if (!is_cmd_valid(cmd, ARGS_IDX2))
		return -EINVAL;

	if (strcmp(cmd->args[ARGS_IDX0], "cat"))
		return -EINVAL;

	ret = glob(cmd->args[ARGS_IDX1], GLOB_TILDE, NULL, &glb_buff);
	if (ret) {
		HIKP_ERROR_PRINT("failed to generate cat paths: %s\n", cmd->args[ARGS_IDX1]);
		return ret;
	}

	for (i = 0; i < glb_buff.gl_pathc; i++) {
		cmd->args[ARGS_IDX1] = glb_buff.gl_pathv[i];
		ret = hikp_collect_exec((void *)cmd);
		if(ret)
			HIKP_INFO_PRINT("cat %s failed\n", glb_buff.gl_pathv[i]);
	}

	globfree(&glb_buff);

	return 0;
}

int hikp_collect_cp_glob_exec(void *data)
{
	struct info_collect_cmd *cmd = (struct info_collect_cmd *)data;
	char log_name[MAX_LOG_NAME_LEN] = {0};
	char tmp[MAX_LOG_NAME_LEN] = {0};
	glob_t glb_buff;
	size_t i;
	int ret;

	if (!is_cmd_valid(cmd, ARGS_IDX3))
		return -EINVAL;

	if (strcmp(cmd->args[ARGS_IDX0], "cp"))
		return -EINVAL;

	ret = glob(cmd->args[ARGS_IDX2], GLOB_TILDE, NULL, &glb_buff);
	if (ret) {
		HIKP_ERROR_PRINT("failed to generate cp paths: %s\n", cmd->args[ARGS_IDX2]);
		return ret;
	}

	ret = snprintf(tmp, MAX_LOG_NAME_LEN, "%s", cmd->log_name);
	if (ret < 0 || (uint32_t)(ret) >= MAX_LOG_NAME_LEN) {
		HIKP_ERROR_PRINT("log name is invalid\n");
		globfree(&glb_buff);
		return -EINVAL;
	}

	for (i = 0; i < glb_buff.gl_pathc; i++) {
		ret = snprintf(log_name, MAX_LOG_NAME_LEN, "%s_%zu", tmp, i);
		if (ret < 0 || (uint32_t)(ret) >= MAX_LOG_NAME_LEN) {
			HIKP_ERROR_PRINT("create log name failed\n");
			globfree(&glb_buff);
			return -EINVAL;
		}

		cmd->log_name = log_name;
		cmd->args[ARGS_IDX2] = glb_buff.gl_pathv[i];
		ret = hikp_save_files(cmd);
		if(ret)
			HIKP_INFO_PRINT("cp %s failed\n", glb_buff.gl_pathv[i]);
	}

	globfree(&glb_buff);

	return 0;
}
