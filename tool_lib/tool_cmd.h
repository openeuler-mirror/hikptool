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

#ifndef TOOL_CMD_H
#define TOOL_CMD_H
#include "tool_lib.h"

#define CMD_EXECUTE_LOCK_NAME "op_execute_lock"

/* The maximum number of this command supported by a single main command */
#define COMMAND_MAX_OPTIONS 64
/* Full command maximum length */
#define COMMAND_MAX_STRING 512
/* Length of command error description */
#define COMMANDER_ERR_MAX_STRING 128

/* Define driver type which hikptool needed, defined as bit mask value */
enum {
	DRIVER_TYPE_OF_NONE = 0,
	DRIVER_TYPE_OF_OTHERS = HI_BIT(0),
	DRIVER_TYPE_OF_NIC_KERNEL = HI_BIT(1),
	DRIVER_TYPE_OF_NIC_DPDK = HI_BIT(2),
};

struct major_cmd_ctrl;
struct cmd_adapter;

/* Callback function prototype for minor command parameter records */
typedef int (*command_record_t)(struct major_cmd_ctrl *major, const char *param);
/* Callback function prototype for command execution */
typedef void (*command_executeute_t)(struct major_cmd_ctrl *major);

/* Subcommand description information */
struct cmd_option {
	const char *little;
	const char *large;
	uint8_t have_param;
	command_record_t record;
};

/* Main command description */
struct major_cmd_ctrl {
	int option_count;
	struct cmd_option options[COMMAND_MAX_OPTIONS];
	uint32_t options_repeat_flag[COMMAND_MAX_OPTIONS];
	command_executeute_t execute;

	int err_no;
	char err_str[COMMANDER_ERR_MAX_STRING + 1];
	struct hikp_cmd_type *cmd_ptr;
	void *cmd_param;
};

struct cmd_adapter {
	const char *name;
	const char *version;
	struct major_cmd_ctrl p_major_cmd;
};

extern int _s_cmd_data;
extern int _e_cmd_data;
extern struct cmd_adapter g_tool;

extern const char *get_tool_name(void);
extern void command_mechanism_init(struct cmd_adapter *adapter, const char *name);
extern bool is_specified_option(const char *arg, const char *little, const char *large);
extern void cmd_option_register(const char *little, const char *large, uint8_t have_param,
				command_record_t record);
extern void command_parse_and_excute(const int argc, const char **argv);
extern struct major_cmd_ctrl *get_major_cmd(void);

#endif
