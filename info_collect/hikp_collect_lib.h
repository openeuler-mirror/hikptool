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

#ifndef HIKP_COLLECT_LIB_H
#define HIKP_COLLECT_LIB_H

#ifndef NULL
#define NULL	((void *)0)
#endif

#define ARGS_IDX0	0
#define ARGS_IDX1	1
#define ARGS_IDX2	2
#define ARGS_IDX3	3
#define ARGS_IDX4	4
#define ARGS_IDX5	5
#define ARGS_IDX6	6
#define ARGS_IDX7	7
#define ARGS_IDX8	8
#define ARGS_IDX9	9
#define ARGS_IDX10	10
#define ARGS_IDX11	11
#define ARGS_MAX_NUM	12
#define LOG_FILE_PATH_MAX_LEN	512
#define HIKP_COLLECT_LOG_DIR_PATH	"/var/log/hikp/"
#define HIKP_ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#define HIKP_NIC_NAME_DIR		"/sys/class/net/"
#define HIKP_NIC_DRV_DIR		"device/driver/module/drivers/"
#define HIKP_NIC_DRV_NAME		"pci:hns3"

typedef int (*collect_cmd_handler_t)(void *);

struct info_collect_cmd {
	char *group;
	char *log_name;
	char *args[ARGS_MAX_NUM];
};

int hikp_compress_log(void);
int hikp_create_save_path(const char *name);
int hikp_get_file_path(char *file_path, unsigned int file_path_len,
		       char *group);
int hikp_collect_exec(void *data);
int hikp_collect_log(char *group, char *log_name,
		collect_cmd_handler_t func, void *data);
int hikp_move_files(struct info_collect_cmd *mv_cmd);
int hikp_save_files(struct info_collect_cmd *save_cmd);
int hikp_collect_cat_glob_exec(void *data);
int hikp_collect_cp_glob_exec(void *data);
void hikp_collect_all_nic_cmd_log(collect_cmd_handler_t hikp_collect_one_nic_log);
#endif /* HIKP_COLLECT_LIB_H */
