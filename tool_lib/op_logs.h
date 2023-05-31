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

#ifndef OP_LOGS_H
#define OP_LOGS_H

#define OP_LOG_SEC_AND_MICROSEC_TRANS 1000000.0
#define OP_LOG_DIR_NAME "operation_logs"
#define OP_LOG_FILE_NAME "operations.log"
#define OP_LOG_FILE_BACKUP "operations.log.old"
#define OP_LOG_TIME_TEMP "[%Y-%m-%d %H:%M:%S]"
#define OP_LOG_RESULT_TIME_TEMP "[%H:%M:%S]"
#define OP_LOG_SEC_TIME_TEMP " [%.6f]"
#define OP_LOG_ITEM_END "\r\n"
#define OP_LOG_FILE_PATH_MAXLEN 256
#define OP_LOG_FILE_W_MAXSIZE 1024
#define OP_LOG_PARAM_MAX_STRING 512
#define OP_LOG_LOCK_NAME "op_log"
#define OP_LOG_FILE_MAX_SIZE 0x400000 // 4M

#define LOG_FLAG_DATE_TIME 0x1
#define LOG_FLAG_ONLY_TIME 0x2

#define LOG_TIME_LENGTH 60

#define MINUTES_PER_HOUR	60
#define SECONDS_PER_MINUTE	60
#define SECONDS_PER_HOUR	(SECONDS_PER_MINUTE * MINUTES_PER_HOUR)
#define SECONDS_PER_DAY		(24 * SECONDS_PER_HOUR)
#define LOG_TIME_DECIMAL	10

#define MAX_FORMAT_STRING 10
struct op_log_print_t {
	const char format[MAX_FORMAT_STRING];
	char *str;
};

void sig_init(void);

void op_log_on(void);
void op_log_off(void);
int op_log_initialise(const char *log_dir);
void op_log_record_input(const int argc, const char **argv);
void op_log_record_result(int ret, const char *tool_name, const char *log_dir);

#endif /* OP_LOGS_H */
