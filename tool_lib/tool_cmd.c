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
#include "tool_cmd.h"

/* save the tool real name pointer, it was update when enter main function */
static const char *g_tool_name = TOOL_NAME;

const char *get_tool_name(void)
{
	return g_tool_name;
}

void command_mechanism_init(struct cmd_adapter *adapter, const char *name)
{
	if ((adapter == NULL) || (name == NULL)) {
		HIKP_ERROR_PRINT("Invalid input parameter.\n");
		return;
	}

	adapter->name = name;
	adapter->version = TOOL_VER;
}

static int major_command_check_param(const struct cmd_option *option, const char *arg)
{
	if (option == NULL)
		return -EFAULT;

	if (option->have_param) {
		if ((!arg) || (arg[0] == '-'))
			return -EINVAL;
		return 0;
	}

	return -EPERM;
}

static int check_command_length(const int argc, const char **argv)
{
	unsigned long long str_len = 0;
	int i;

	for (i = 0; i < argc; i++) {
		str_len += (unsigned long long)strlen(argv[i]);
		if (str_len > COMMAND_MAX_STRING)
			return -EINVAL;
	}

	return 0;
}

struct major_cmd_ctrl *get_major_cmd(void)
{
	return &g_tool.p_major_cmd;
}

static int sub_command_record(struct major_cmd_ctrl *major_cmd,
			      struct cmd_option *option, const char *arg)
{
	major_cmd->err_no = option->record(major_cmd, arg);

	if (((strlen((const char *)option->little) + 1) != sizeof("-h")) &&
		((strlen((const char *)option->large) + 1) != sizeof("--help"))) {
		return major_cmd->err_no;
	}
	if ((strncmp((const char *)option->little, "-h", sizeof("-h")) == 0) ||
		(strncmp((const char *)option->large, "--help", sizeof("--help")) == 0)) {
		return -ESRCH;
	}

	return major_cmd->err_no;
}

bool is_specified_option(const char *arg, const char *little, const char *large)
{
	bool flag;

	flag = (little == NULL) || (large == NULL) || (arg == NULL);
	if (flag)
		return false;

	flag = (strlen(arg) != strlen(little)) && (strlen(arg) != strlen(large));
	if (flag)
		return false;

	return (strncmp(arg, little, strlen(little) + 1) == 0) ||
	       (strncmp(arg, large, strlen(large) + 1) == 0);
}

static int major_command_parse(struct major_cmd_ctrl *major_cmd, const int argc, const char **argv)
{
	struct cmd_option *option = NULL;
	int intermediate_var;
	const char *param = NULL;
	int i, j;
	int ret;

	for (i = 0; i < argc; i++) {
		/* Traverse all registered subcommands */
		for (j = 0; j < major_cmd->option_count; j++) {
			option = &major_cmd->options[j];
			if (!is_specified_option(argv[i], option->little, option->large))
				continue;

			/* Prevent duplicate input */
			if (major_cmd->options_repeat_flag[j] != 0) {
				major_cmd->err_no = -EINVAL;
				snprintf(major_cmd->err_str, sizeof(major_cmd->err_str),
					 "Repeated option %s.", option->little);
				return -EINVAL;
			}
			major_cmd->options_repeat_flag[j] = 1;

			/* Determine whether the subcommand is related to the parameter */
			intermediate_var = i + 1;
			param = intermediate_var < argc ? argv[intermediate_var] : NULL;
			ret = major_command_check_param(option, param);
			if (ret == 0) {
				i++;
			} else if (ret == -EINVAL) {
				major_cmd->err_no = -EINVAL;
				snprintf(major_cmd->err_str, sizeof(major_cmd->err_str),
					 "%s option need parameter.", option->little);
				return -EINVAL;
			}

			/* Record the option identifier and parameter
			 * information to provide parameters
			 * for subsequent execute processing.
			 */
			ret = sub_command_record(major_cmd, option, param);
			if (ret)
				return ret;

			break;
		}

		if (j == major_cmd->option_count) {
			major_cmd->err_no = -EINVAL;
			snprintf(major_cmd->err_str, sizeof(major_cmd->err_str),
				 "%s is not option needed.", argv[i]);
			return -EINVAL;
		}
	}

	return 0;
}

void cmd_option_register(const char *little, const char *large,
			 uint8_t have_param, command_record_t record)
{
	struct major_cmd_ctrl *major_cmd = get_major_cmd();
	struct cmd_option *option = NULL;

	if (((little == NULL) && (large == NULL)) || (record == NULL)) {
		HIKP_ERROR_PRINT("Invalid input parameter.\n");
		return;
	}

	if (major_cmd->option_count >= COMMAND_MAX_OPTIONS) {
		HIKP_ERROR_PRINT("Do not support more than %d options\n", COMMAND_MAX_OPTIONS);
		return;
	}

	option = &major_cmd->options[major_cmd->option_count];
	major_cmd->options_repeat_flag[major_cmd->option_count] = 0;
	major_cmd->option_count++;

	option->record = record;
	option->little = little;
	option->large = large;
	option->have_param = have_param;
}

void command_parse_and_excute(const int argc, const char **argv)
{
	struct major_cmd_ctrl *major_cmd = get_major_cmd();
	int lock_fd;
	int check;
	int ret;

	major_cmd->err_no = check_command_length(argc, argv);
	if (major_cmd->err_no) {
		snprintf(major_cmd->err_str, sizeof(major_cmd->err_str), "Command input too long.");
		goto PARSE_OUT;
	}

	/* More than 2 means major command need to be parsed */
	if (argc > 2) {
		/* 2: Start index of the execution content */
		if (major_command_parse(major_cmd, argc - 2, argv + 2))
			goto PARSE_OUT;
	}
	ret = tool_flock(CMD_EXECUTE_LOCK_NAME, UDA_FLOCK_BLOCK, &lock_fd, HIKP_LOG_DIR_PATH);
	if (ret) {
		major_cmd->err_no = ret < 0 ? ret : -ret;
		snprintf(major_cmd->err_str, sizeof(major_cmd->err_str), "locking failed.");
		goto PARSE_OUT;
	}
	major_cmd->execute(major_cmd);
	tool_unlock(&lock_fd, UDA_FLOCK_BLOCK);
PARSE_OUT:
	if (major_cmd->err_no)
		HIKP_ERROR_PRINT("%s command error(%d): %s\n",
				 g_tool.name, major_cmd->err_no, major_cmd->err_str);
}
