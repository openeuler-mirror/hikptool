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

#include <stdint.h>
#include "tool_lib.h"
#include "tool_cmd.h"
#include "ubus_common.h"
#include "ubus_msgq_dump.h"
#include "hikptdev_plug.h"

struct tool_ubus_msgq_cmd {
	enum ubus_msgq_dump_cmd_type cmd_type;
	uint32_t controller_idx;
	uint32_t msgq_idx;
	uint32_t queue_type;
	uint32_t entry_idx;
};

static struct tool_ubus_msgq_cmd g_ubus_msgq_cmd = {
	.cmd_type = MSGQ_DUMP_HELP,
	.controller_idx = (uint32_t)(-1),
	.msgq_idx = (uint32_t)(-1),
	.queue_type = UBUS_MSGQ_UNKNOWN,
	.entry_idx = (uint32_t)(-1),
};

static int ubus_msgq_dump_help(struct major_cmd_ctrl *self, const char *argv)
{
	HIKP_SET_USED(argv);

	printf("\n  ubus msgq dump\n");
	printf("\n  Usage: %s --msgq [OPTION] [PARA...]\n", self->cmd_ptr->name);
	printf("\n  Options:\n");
	printf("    %s, %-25s %s\n", "-h", "--help", "display this help and exit");
	printf("    %s, %-25s %s\n", "-c", "--controller", "select the controller first");
	printf("    %s, %-25s %s\n", "-i", "--msgqidx", "input msgq index");
	printf("    %s, %-25s %s\n", "-r", "--register",
	       "only dump registers of the specified msgq");
	printf("    %s, %-25s %s\n", "-e", "--entry", "select the entry to be dumped");
	printf("        usage: -e [queue_type]e[entry_idx], e.g. -e sqe0/-e rqe0/-e cqe0\n");
	printf("\n");

	return 0;
}

static int ubus_controller_set(struct major_cmd_ctrl *self, const char *argv)
{
	uint32_t val;
	int ret;

	HIKP_SET_USED(self);

	ret = string_toui(argv, &val);
	if (ret) {
		printf("ubus set controller err %d\n", ret);
		return ret;
	}
	g_ubus_msgq_cmd.controller_idx = val;

	return 0;
}

static int ubus_msgq_idx_set(struct major_cmd_ctrl *self, const char *argv)
{
	uint32_t val;
	int ret;

	HIKP_SET_USED(self);

	ret = string_toui(argv, &val);
	if (ret) {
		printf("ubus set msgq idx err %d\n", ret);
		return ret;
	}
	g_ubus_msgq_cmd.msgq_idx = val;

	return 0;
}

static int ubus_entry_set(struct major_cmd_ctrl *self, const char *argv)
{
#define MAX_ENTRY_STR_SIZE 256
	char str[MAX_ENTRY_STR_SIZE] = { 0 };
	char *save_str, *tmp_str;
	uint32_t val;
	size_t len;
	int ret;

	HIKP_SET_USED(self);

	len = strlen(argv);
	if (len >= MAX_ENTRY_STR_SIZE || len == 0 ) {
		printf("ubus entry queue type string err length: %zu\n", len);
		return -EPERM;
	}
	memcpy(str, argv, len);

	tmp_str = strtok_r(str, "e", &save_str);
	if (tmp_str == NULL) {
		printf("ubus set entry queue type(NULL) err\n");
		return -EPERM;
	}

	if (!strcmp(tmp_str, "sq")) {
		g_ubus_msgq_cmd.queue_type = UBUS_MSGQ_SQ;
	} else if (!strcmp(tmp_str, "rq")) {
		g_ubus_msgq_cmd.queue_type = UBUS_MSGQ_RQ;
	} else if (!strcmp(tmp_str, "cq")) {
		g_ubus_msgq_cmd.queue_type = UBUS_MSGQ_CQ;
	} else {
		printf("ubus set entry queue type(%s) err\n", tmp_str);
		return -EPERM;
	}

	ret = string_toui(save_str, &val);
	if (ret) {
		printf("ubus set entry idx err %d\n", ret);
		return ret;
	}
	g_ubus_msgq_cmd.entry_idx = val;
	g_ubus_msgq_cmd.cmd_type = MSGQ_DUMP_ENTRY;

	return 0;
}

static int ubus_msgq_reg_dump(struct major_cmd_ctrl *self, const char *argv)
{
	HIKP_SET_USED(self);
	HIKP_SET_USED(argv);

	g_ubus_msgq_cmd.cmd_type = MSGQ_DUMP_REG;

	return 0;
}

static int ubus_msgq_dump_execute_process(void)
{
	uint32_t cmd_type = g_ubus_msgq_cmd.cmd_type;
	uint32_t controller_idx = g_ubus_msgq_cmd.controller_idx;
	uint32_t msgq_idx = g_ubus_msgq_cmd.msgq_idx;
	uint32_t queue_type = g_ubus_msgq_cmd.queue_type;
	uint32_t entry_idx = g_ubus_msgq_cmd.entry_idx;

	if (cmd_type == MSGQ_DUMP_REG) {
		return ubus_msgq_dump_reg_execute(controller_idx, msgq_idx);
	} else if (cmd_type == MSGQ_DUMP_ENTRY) {
		return ubus_msgq_dump_entry_execute(controller_idx, msgq_idx,
						    queue_type, entry_idx);
	}

	return -EPERM;
}

static void ubus_msgq_dump_execute(struct major_cmd_ctrl *self)
{
	int ret;
	uint32_t cmd_type;
	static const char *ubus_msgq_dump_succ_msg[] = {
		"",
		"ubus_msgq_reg_dump success!",
		"ubus_msgq_entry_dump success!",
	};
	static const char *ubus_msgq_dump_err_msg[] = {
		"Error : unknown param_type!",
		"ubus_msgq_reg_dump failed!",
		"ubus_msgq_entry_dump failed!",
	};

	ret = ubus_msgq_dump_execute_process();
	cmd_type = g_ubus_msgq_cmd.cmd_type;
	if (ret) {
		/* In error branches, errors are printed and copied, and no check is required. */
		snprintf(self->err_str, sizeof(self->err_str), "%s",
			 ubus_msgq_dump_err_msg[cmd_type]);
	} else {
		printf("%s\n", ubus_msgq_dump_succ_msg[cmd_type]);
	}

	self->err_no = ret;
}

void cmd_ubus_msgq_dump_init(void)
{
	struct major_cmd_ctrl *major_cmd = get_major_cmd();

	major_cmd->execute = ubus_msgq_dump_execute;
	cmd_option_register("-h", "--help", false, ubus_msgq_dump_help);
	cmd_option_register("-r", "--register", false, ubus_msgq_reg_dump);
	cmd_option_register("-c", "--controller", true, ubus_controller_set);
	cmd_option_register("-i", "--msgqidx", true, ubus_msgq_idx_set);
	cmd_option_register("-e", "--entry", true, ubus_entry_set);
}
