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
#include <time.h>
#include <syslog.h>
#include <unistd.h>
#include <sys/stat.h>
#include <signal.h>
#include "tool_lib.h"
#include "hikptdev_plug.h"
#include "op_logs.h"

static char g_op_log[OP_LOG_FILE_PATH_MAXLEN] = {0};
static char g_cmd_exec_time[LOG_TIME_LENGTH] = {0};
static bool g_record = true;
static bool g_log_info;
static char g_input_buf[OP_LOG_FILE_W_MAXSIZE + 1] = {0};

static void op_log_write(const char *log_data)
{
	size_t w_size;
	FILE *fd;
	int ret;

	if (strlen(g_op_log) == 0)
		return;

	if (!is_file_exist(g_op_log))
		return;

	fd = fopen(g_op_log, "a");
	if (fd == NULL) {
		HIKP_ERROR_PRINT("Can not open operation log file[%s], errno is %d\n",
				 g_op_log, errno);
		return;
	}
	(void)chmod(g_op_log, 0640);
	w_size = fwrite((void *)log_data, 1U, strlen(log_data), fd);
	if (strlen(log_data) > ((uint32_t)w_size))
		HIKP_ERROR_PRINT("Error data size write to file, errno is %d\n", errno);

	(void)fclose(fd);
}

static int op_log_write_buffer(const char *log_data, const char *log_dir)
{
	sigset_t sigset;
	int op_lock_fd;
	int ret;

	sigfillset(&sigset);
	sigprocmask(SIG_BLOCK, &sigset, NULL);
	ret = tool_flock(OP_LOG_LOCK_NAME, UDA_FLOCK_BLOCK, &op_lock_fd, log_dir);
	if (ret == -ENOENT) {
		HIKP_ERROR_PRINT("Folder or file required by the operation is not exist.\n");
		return ret;
	}

	if (ret) {
		HIKP_ERROR_PRINT("Multi-user operate in the meantime will causes fault(%d).\n",
				 ret);
		return ret;
	}

	op_log_write(log_data);
	tool_unlock(&op_lock_fd, UDA_FLOCK_BLOCK);
	g_log_info = true;
	sigprocmask(SIG_UNBLOCK, &sigset, NULL);
	return 0;
}

void op_log_on(void)
{
	g_record = true;
}

void op_log_off(void)
{
	g_record = false;
}

static bool op_log_is_on(void)
{
	return g_record;
}

static double op_log_diff_timeval(const struct timeval *now, const struct timeval *last)
{
	double time_val;

	time_val = now->tv_sec - last->tv_sec;
	time_val += (now->tv_usec - last->tv_usec) / OP_LOG_SEC_AND_MICROSEC_TRANS;
	return time_val;
}

static int op_log_add_time_to_log(char *log_base, int *offset, uint32_t flag)
{
	static struct timeval g_tv;
	struct timeval tv;
	struct tm ptm;
	int len = 0;
	long usec;
	long sec;
	int ret;

	(void)gettimeofday(&tv, NULL);
	localtime_r(&tv.tv_sec, &ptm);
	if (flag == LOG_FLAG_DATE_TIME) {
		g_tv = tv;
		len = (int)strftime(log_base + *offset, (OP_LOG_FILE_W_MAXSIZE + 1 - *offset),
				    OP_LOG_TIME_TEMP, &ptm);
		ret = snprintf(log_base + *offset + len,
			       (OP_LOG_FILE_W_MAXSIZE + 1 - *offset - len), OP_LOG_SEC_TIME_TEMP,
			       tv.tv_sec + tv.tv_usec / OP_LOG_SEC_AND_MICROSEC_TRANS);
	} else if (flag == LOG_FLAG_ONLY_TIME) {
		len = (int)strftime(log_base + *offset, (OP_LOG_FILE_W_MAXSIZE + 1 - *offset),
				    OP_LOG_RESULT_TIME_TEMP, &ptm);
		ret = snprintf(log_base + *offset + len,
			       (OP_LOG_FILE_W_MAXSIZE + 1 - *offset - len),
			       OP_LOG_SEC_TIME_TEMP,
			       op_log_diff_timeval((const struct timeval *)&tv,
			       (const struct timeval *)&g_tv));
	}
	len += ret;
	if (ret < 0 || len >= (OP_LOG_FILE_W_MAXSIZE + 1 - *offset))
		return -EINVAL;

	*offset += len;

	return 0;
}

static int op_log_add_info_to_log(char *log_base, int *offset, const char *str)
{
	int len;

	len = snprintf(log_base + *offset, (OP_LOG_FILE_W_MAXSIZE + 1 - *offset), "%s", str);
	if (len < 0 || len >= (OP_LOG_FILE_W_MAXSIZE + 1 - *offset))
		return -EINVAL;

	*offset += len;

	return 0;
}

static int op_log_file_rollback(const char *op_log_backup, const char *log_dir)
{
	char rollback_log[OP_LOG_FILE_W_MAXSIZE + 1] = {0};
	int offset = 0;
	int ret;
	int len;

	ret = file_rollback(g_op_log, op_log_backup, OP_LOG_FILE_MAX_SIZE);
	if (ret) {
		if (ret == FILE_LEN_OK)
			return 0;

		HIKP_ERROR_PRINT("Log file rollback failed.\n");
		return ret;
	}

	ret = op_log_add_time_to_log(rollback_log, &offset, LOG_FLAG_DATE_TIME);
	if (ret)
		return ret;

	ret = op_log_add_info_to_log(rollback_log, &offset,
				     "The old operation log has been backed up to ");
	if (ret)
		return ret;

	ret = op_log_add_info_to_log(rollback_log, &offset, op_log_backup);
	if (ret)
		return ret;

	snprintf(rollback_log + offset,
		 (uint32_t)(OP_LOG_FILE_W_MAXSIZE + 1 - offset), OP_LOG_ITEM_END);

	op_log_write_buffer(rollback_log, log_dir);

	return ret;
}

static int op_log_dir_mk(const char *log_path)
{
	if (!is_dir_exist(log_path))
		return tool_mk_dir(log_path);

	return 0;
}

static int op_log_dir_create(char *log_path, int log_path_len)
{
	int ret = 0;
	int i;

	for (i = 1; i < log_path_len; i++) {
		if (log_path[i] == '/') {
			log_path[i] = '\0';
			ret = op_log_dir_mk((const char *)log_path);
			if (ret)
				return ret;

			log_path[i] = '/';
		}
	}

	return ret;
}

static void op_log_record_time(void)
{
	int offset = 0;

	(void)op_log_add_time_to_log(g_cmd_exec_time, &offset, LOG_FLAG_DATE_TIME);
}

int op_log_initialise(const char *log_dir)
{
	char op_log_backup[OP_LOG_FILE_PATH_MAXLEN] = {0};
	char log_path[OP_LOG_FILE_PATH_MAXLEN];
	int ret;

	if ((log_dir == NULL) || (strlen(g_op_log) != 0))
		return -EINVAL;

	memset(log_path, '\0', OP_LOG_FILE_PATH_MAXLEN);
	memset(g_op_log, '\0', OP_LOG_FILE_PATH_MAXLEN);
	ret = snprintf(log_path, sizeof(log_path), "%s", log_dir);
	if (ret < 0 || ret >= sizeof(log_path))
		return -EINVAL;

	if (!is_dir_exist(log_path)) {
		ret = op_log_dir_create(log_path, strlen(log_path));
		if (ret != 0) {
			HIKP_ERROR_PRINT("Operation Log file folder[%s] can not be created.\n",
					 log_path);
			return ret;
		}
	}

	snprintf(g_op_log, OP_LOG_FILE_PATH_MAXLEN, "%s" DIR_BREAK_STRING "%s",
		 log_path, OP_LOG_FILE_NAME);
	snprintf(op_log_backup, OP_LOG_FILE_PATH_MAXLEN, "%s" DIR_BREAK_STRING "%s",
		 log_path, OP_LOG_FILE_BACKUP);

	ret = op_log_file_rollback((const char *)op_log_backup, log_dir);
	if (ret)
		return ret;

	op_log_record_time();

	return ret;
}

void op_log_record_input(const int argc, const char **argv)
{
	char input_str[OP_LOG_FILE_W_MAXSIZE + 1] = {0};
	struct op_log_print_t log_info[] = {
		{"%s", g_cmd_exec_time},
		{"[%s]", input_str},
	};
	size_t i, arr_size;
	int offset = 0;
	char *arg;
	int ret;

	memset(g_input_buf, 0, sizeof(g_input_buf));

	if (argv == NULL || argc == 0)
		return;

	arg = input_str;
	for (i = 0; i < argc; i++) {
		snprintf(arg, (sizeof(input_str) - (arg - input_str)), "%s ", argv[i]);
		arg = arg + strlen(argv[i]) + 1;
	}
	input_str[strlen(input_str) - 1] = 0;
	arr_size = HIKP_ARRAY_SIZE(log_info);
	for (i = 0; i < arr_size; i++) {
		ret = snprintf(g_input_buf + offset, (OP_LOG_FILE_W_MAXSIZE + 1 - offset),
			log_info[i].format, log_info[i].str);
		if (ret < 0 || ret >= (OP_LOG_FILE_W_MAXSIZE + 1 - offset))
			return;

		offset += ret;
	}
}

void op_log_record_result(int ret, const char *tool_name, const char *log_dir)
{
	char result_str[OP_LOG_FILE_W_MAXSIZE + 1] = {0};
	int offset = 0;
	int len;

	/* must to open */
	if (op_log_is_on() == false && (ret == 0))
		return;

	len = snprintf(result_str + offset, (OP_LOG_FILE_W_MAXSIZE + 1 - offset), "%s",
		       g_input_buf);
	if (len < 0 || len >= (OP_LOG_FILE_W_MAXSIZE + 1 - offset))
		return;

	offset += len;

	if (op_log_add_time_to_log(result_str, &offset, LOG_FLAG_ONLY_TIME))
		return;

	len = snprintf(result_str + offset, (OP_LOG_FILE_W_MAXSIZE + 1 - offset),
		       "[%s<%d>].", (ret == 0) ? "SUCCEED" : "FAILED", ret);
	if (len < 0 || len >= (OP_LOG_FILE_W_MAXSIZE + 1 - offset))
		return;

	offset += len;

	record_syslog(tool_name, LOG_INFO, result_str);

	snprintf(result_str + offset, (sizeof(result_str) - offset), OP_LOG_ITEM_END);
	(void)op_log_write_buffer(result_str, log_dir);
}

static bool log_info_is_ok(void)
{
	return g_log_info;
}

static void signal_format_end_log_str(char *log_str, int signal_code)
{
	time_t seconds = time(NULL);
	int sec_of_last_day = seconds % SECONDS_PER_DAY;
	int hour = sec_of_last_day / SECONDS_PER_HOUR;
	int min = (sec_of_last_day % SECONDS_PER_HOUR) / SECONDS_PER_MINUTE;
	int sec = (sec_of_last_day % SECONDS_PER_MINUTE);

	/* end of log string format: [hh:mm:ss] [KILLED<xx>]. */
	log_str[1] += hour / LOG_TIME_DECIMAL; /* 1: format time: tens of hour */
	log_str[2] += hour % LOG_TIME_DECIMAL; /* 2: format time: units of hour */
	log_str[4] += min / LOG_TIME_DECIMAL; /* 4: format time: tens of min */
	log_str[5] += min % LOG_TIME_DECIMAL; /* 5: format time: units of min */
	log_str[7] += sec / LOG_TIME_DECIMAL; /* 7: format time: tens of sec */
	log_str[8] += sec % LOG_TIME_DECIMAL; /* 8: format time: units of sec */
	log_str[19] += signal_code / LOG_TIME_DECIMAL; /* 19: tens of signal_code */
	log_str[20] += signal_code % LOG_TIME_DECIMAL; /* 20: units of signal_code */
}

static int signal_fcntl(const char *name, uint32_t operation, int *fd)
{
	char lock_file[TOOL_LOCK_PATH_MAX_LEN] = HIKP_LOG_DIR_PATH TOOL_LOCK_FLODER_NAME;
	int lock_file_len = 0;
	int tmp = 0;
	int ret;

	while (lock_file[lock_file_len] != '\0')
		lock_file_len++;

	if (!is_dir_exist(lock_file)) {
		ret = op_log_dir_create(lock_file, lock_file_len);
		if (ret)
			return ret;
	}

	while (name[tmp] != '\0') {
		lock_file[lock_file_len] = name[tmp];
		lock_file_len++;
		tmp++;
	}
	lock_file[lock_file_len] = '\0';

	return uda_fcntl(lock_file, operation, fd);
}

void signal_op_log_write(int signal_code)
{
	char log_str[] = "[00:00:00] [KILLED<00>].\r\n";
	int op_log_fd;
	int start_len;
	int len;
	int fd;

	if (log_info_is_ok())
		return;

	if (signal_fcntl(OP_LOG_LOCK_NAME, UDA_FLOCK_BLOCK, &op_log_fd))
		return;

	fd = open(g_op_log, O_WRONLY | O_APPEND);
	if (fd == -1) {
		(void)uda_unfcntl(&op_log_fd, UDA_FLOCK_BLOCK);
		return;
	}
	(void)chmod(g_op_log, 0640);
	start_len = 0;
	while (g_input_buf[start_len] != '\0')
		start_len += 1;

	len = write(fd, g_input_buf, start_len);
	if (len == -1)
		goto SIGNAL_LOG_OUT;

	signal_format_end_log_str(log_str, signal_code);
	len = write(fd, log_str, sizeof(log_str));
	if (len == -1)
		goto SIGNAL_LOG_OUT;

SIGNAL_LOG_OUT:
	(void)close(fd);
	(void)uda_unfcntl(&op_log_fd, UDA_FLOCK_BLOCK);
}

static void signal_handle(int arg)
{
	signal_op_log_write(arg);
	hikp_unlock();
	_exit(1);
}

void sig_init(void)
{
	(void)signal(SIGINT,  signal_handle); /* Quit process */
	(void)signal(SIGTERM, signal_handle);
	(void)signal(SIGQUIT, signal_handle);
	(void)signal(SIGHUP,  signal_handle);
	(void)signal(SIGSEGV, signal_handle);
	(void)signal(SIGBUS,  signal_handle);
    (void)signal(SIGFPE,  signal_handle);
    (void)signal(SIGABRT, signal_handle);
    (void)signal(SIGTSTP, signal_handle); /* Stop process */
}
