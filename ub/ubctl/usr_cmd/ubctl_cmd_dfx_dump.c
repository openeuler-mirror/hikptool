/*
 * Copyright (c) 2025 Hisilicon Technologies Co., Ltd.
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
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "tool_lib.h"
#include "tool_cmd.h"
#include "hikptdev_plug.h"

#define UBCTL_ARGS_MAX_NUM    128
#define UBCTL_ARGV_MAX_LEN    1024

static char *g_args[UBCTL_ARGS_MAX_NUM];
static char g_argv_copy[UBCTL_ARGV_MAX_LEN] = {0};

static int ubctl_dfx_dump_help(struct major_cmd_ctrl *self, const char *argv)
{
	HIKP_SET_USED(argv);

	printf("\n  Usage: %s -p \"params\"\n", self->cmd_ptr->name);
	printf("\n         %s\n", self->cmd_ptr->help_info);
	printf("\n  Options:\n\n");
	printf("    %s, %-25s %s\n", "-h", "--help", "display this help and exit");
	printf("    %s, %-25s %s\n", "-p", "--params", "please input the all params of ubctl");
	printf("\n  Examples: hikptool ubctl -p \"-c 0 -d 1 -m ta\"\n");
	printf("\n");

	return 0;
}

static int ubctl_check_arg(struct major_cmd_ctrl *self, const char *argv)
{
	int i;

	if ((argv == NULL) || (strlen(argv) >= UBCTL_ARGV_MAX_LEN)) {
		snprintf(self->err_str, sizeof(self->err_str), "Params is invalid.\n");
		self->err_no = -EINVAL;
		return -EINVAL;
	}

	for (i = 0; argv[i] != '\0'; i++) {
		if ((isalnum(argv[i]) == 0) && argv[i] != '_' && argv[i] != '-' && argv[i] != ' ') {
			snprintf(self->err_str, sizeof(self->err_str), "Params is illegal string.\n");
			self->err_no = -EINVAL;
			return -EINVAL;
		}
	}

	return 0;
}

static int ubctl_params_set(struct major_cmd_ctrl *self, const char *argv)
{
	char *token = NULL;
	int i = 1;

	HIKP_SET_USED(self);

	if (ubctl_check_arg(self, argv) != 0)
		return -EINVAL;

	(void)strncpy(g_argv_copy, argv, sizeof(g_argv_copy) - 1);

	g_args[0] = "ubctl";
	token = strtok(g_argv_copy, " ");
	while (token != NULL && i < UBCTL_ARGS_MAX_NUM - 1) {
		g_args[i++] = token;
		token = strtok(NULL, " ");
	}
	g_args[i] = NULL;

	if (g_args[1] == NULL) {
		snprintf(self->err_str, sizeof(self->err_str), "Parameter parsing error.\n");
		self->err_no = -EINVAL;
		return -EINVAL;
	}

	return 0;
}

static bool ubctl_process_ubctl_file(struct major_cmd_ctrl *self, const char *ubctl_path)
{
	struct stat buffer;

	if (lstat(ubctl_path, &buffer) != 0) {
		snprintf(self->err_str, sizeof(self->err_str),
			 "ubctl does not exist in the hikptool directory.\n");
		self->err_no = -ENOENT;
		return false;
	}

	if (!S_ISREG(buffer.st_mode)) {
		snprintf(self->err_str, sizeof(self->err_str), "ubctl is not a regular file.\n");
		self->err_no = -EPERM;
		return false;
	}

	if (!(buffer.st_mode & S_IXUSR)) {
		snprintf(self->err_str, sizeof(self->err_str), "ubctl has no execute permission.\n");
		self->err_no = -EACCES;
		return false;
	}

	return true;
}

static void ubctl_execute(struct major_cmd_ctrl *self, const char *current_dir)
{
#define PATH_SEPARATOR_LENGTH 2  // 1 for '/' and 1 for null terminator

	size_t ubctl_path_length = strlen(current_dir) + strlen("ubctl") + PATH_SEPARATOR_LENGTH;
	size_t path_env_length = strlen(current_dir) + 1;
	ssize_t bytes_written = 0;
	char *ubctl_path = NULL;
	char *path_env = NULL;
	char *envp[] = {path_env, NULL};
	int ret = 0;

	ubctl_path = (char *)malloc(ubctl_path_length);
	if (ubctl_path == NULL) {
		snprintf(self->err_str, sizeof(self->err_str),
			 "Failed to allocate memory for ubctl path.\n");
		self->err_no = -ENOMEM;
		return;
	}

	bytes_written = snprintf(ubctl_path, ubctl_path_length, "%s/ubctl", current_dir);
	if (bytes_written < 0 || (size_t)bytes_written >= ubctl_path_length) {
		snprintf(self->err_str, sizeof(self->err_str), "Failed to copy ubctl path.\n");
		self->err_no = -EINVAL;
		goto cleanup_ubctl_path;
	}

	if (!ubctl_process_ubctl_file(self, ubctl_path)) {
		goto cleanup_ubctl_path;
	}

	path_env = (char *)malloc(path_env_length);
	if (path_env == NULL) {
		snprintf(self->err_str, sizeof(self->err_str),
			 "Failed to allocate memory for path env.\n");
		self->err_no = -ENOMEM;
		goto cleanup_ubctl_path;
	}

	bytes_written = snprintf(path_env, path_env_length, "%s", current_dir);
	if (bytes_written < 0 || (size_t)bytes_written >= path_env_length) {
		snprintf(self->err_str, sizeof(self->err_str), "Failed to copy path env.\n");
		self->err_no = -EINVAL;
		goto cleanup_path_env;
	}

	ret = execve(ubctl_path, g_args, envp);
	if (ret != 0) {
		snprintf(self->err_str, sizeof(self->err_str),
			 "ubctl execute failed: %s.\n", strerror(errno));
	}
	self->err_no = ret;

cleanup_path_env:
	free(path_env);
cleanup_ubctl_path:
	free(ubctl_path);
}

static void ubctl_dfx_dump_execute(struct major_cmd_ctrl *self)
{
	char *program_path = NULL;
	char *current_dir = NULL;
	char *last_slash = NULL;

	program_path = realpath("/proc/self/exe", NULL);
	if (program_path == NULL) {
		snprintf(self->err_str, sizeof(self->err_str),
			 "Failed to get program path: %s.\n", strerror(errno));
		self->err_no = -errno;
		return;
	}

	current_dir = strdup(program_path);
	if (current_dir == NULL) {
		snprintf(self->err_str, sizeof(self->err_str),
			 "Failed to allocate memory for current dir.\n");
		self->err_no = -ENOMEM;
		goto cleanup_program_path;
	}

	last_slash = strrchr(current_dir, '/');
	if (last_slash == NULL) {
		snprintf(self->err_str, sizeof(self->err_str), "Invalid program path.\n");
		self->err_no = -EINVAL;
		goto cleanup_current_dir;
	}
	*last_slash = '\0';

	ubctl_execute(self, current_dir);

cleanup_current_dir:
	free(current_dir);
cleanup_program_path:
	free(program_path);
}

static void ubctl_cmd_dfx_init(void)
{
	struct major_cmd_ctrl *major_cmd = get_major_cmd();

	major_cmd->option_count = 0;
	major_cmd->execute = ubctl_dfx_dump_execute;

	cmd_option_register("-h", "--help", false, ubctl_dfx_dump_help);
	cmd_option_register("-p", "--params", true, ubctl_params_set);
}

HIKP_CMD_DECLARE("ubctl", "ubctl dfx dump", ubctl_cmd_dfx_init);
