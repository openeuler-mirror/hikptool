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
#include "tool_lib.h"
#include <unistd.h>
#include <limits.h>
#include <errno.h>
#include <time.h>

uint32_t get_chip_type(void)
{
	char part_num_str[MIDR_BUFFER_SIZE] = {0};
	char midr_buffer[MIDR_BUFFER_SIZE] = {0};
	uint32_t chip_type = CHIP_UNKNOW;
	uint64_t midr_el1;
	uint32_t part_num;
	char *end = NULL;
	FILE *file;

	file = fopen(MIDR_EL1_PATH, "r");
	if (file == NULL) {
		HIKP_ERROR_PRINT("Open file: %s failed\n", MIDR_EL1_PATH);
		return chip_type;
	}

	if (fgets(midr_buffer, MIDR_BUFFER_SIZE, file) == NULL) {
		HIKP_ERROR_PRINT("Read file: %s failed\n", MIDR_EL1_PATH);
		fclose(file);
		return chip_type;
	}

	fclose(file);
	midr_el1 = strtoul(midr_buffer, &end, MIDR_HEX_TYPE);
	if ((end <= midr_buffer) || (midr_el1 == ULONG_MAX)) {
		HIKP_ERROR_PRINT("Get chip type failed: %d\n", errno);
		return chip_type;
	}

	part_num = (midr_el1 & 0xffff) >> PART_NUM_OFFSET;
	(void)snprintf(part_num_str, MIDR_BUFFER_SIZE, "%x", part_num);

	if (strcmp(part_num_str, "d02") == 0)
		chip_type = CHIP_HIP09;
	else if (strcmp(part_num_str, "d03") == 0)
		chip_type = CHIP_HIP10;
	else if (strcmp(part_num_str, "d45") == 0)
		chip_type = CHIP_HIP10C;
	else if (strcmp(part_num_str, "d22") == 0)
		chip_type = CHIP_HIP11;
	else if (strcmp(part_num_str, "d06") == 0)
		chip_type = CHIP_HIP12;
	else
		chip_type = CHIP_UNKNOW;

	return chip_type;
}

int string_toui(const char *nptr, uint32_t *value)
{
	char *endptr = NULL;
	int64_t tmp_value;

	if (nptr == NULL || value == NULL)
		return -EINVAL;

	tmp_value = strtol(nptr, &endptr, 0);
	if ((*endptr != 0) || (tmp_value > UINT_MAX) || (tmp_value < 0))
		return -EINVAL;

	*value = (uint32_t)tmp_value;
	return 0;
}

int string_toub(const char *nptr, uint8_t *value)
{
	char *endptr = NULL;
	int64_t tmp_value;

	if (nptr == NULL || value == NULL)
		return -EINVAL;

	tmp_value = strtol(nptr, &endptr, 0);
	if ((*endptr != 0) || (tmp_value > UCHAR_MAX) || (tmp_value < 0))
		return -EINVAL;

	*value = (uint8_t)tmp_value;
	return 0;
}

static void dir_break_char(char *absolute_path, uint32_t absolute_len)
{
	uint32_t i;

	i = (uint32_t)strlen(absolute_path);
	if (i + 1 >= absolute_len) {
		HIKP_ERROR_PRINT("absolute path invalid.\n");
		return;
	}
	if (i != 1) {
		if (absolute_path[i] != DIR_BREAK_CHAR)
			absolute_path[i++] = DIR_BREAK_CHAR;
		absolute_path[i] = '\0';
	}
}

static int check_current_patch_dir(const char *file, char *absolute_path, uint32_t absolute_len)
{
	char tmp_path[TOOL_REAL_PATH_MAX_LEN] = {0};
	int ret;

	strncat(tmp_path, PWD_STR, HIKP_STR_BUF_LEFT_LEN(tmp_path));
	ret = uda_realpath(tmp_path, absolute_path);
	if (ret)
		return ret;

	dir_break_char(absolute_path, absolute_len);

	strncat(absolute_path, file, absolute_len - strlen(absolute_path) - 1);

	return 0;
}

static int check_absolute_patch_dir(char *file, char *absolute_path, uint32_t absolute_len)
{
	char file_name[TOOL_REAL_PATH_MAX_LEN] = {0};
	char *pchar = strrchr(file, DIR_BREAK_CHAR);
	int ret;
	int i;

	if (pchar == NULL)
		return -EINVAL;

	pchar++;
	i = 0;
	while (*pchar != '\0') {
		file_name[i] = *pchar;
		*pchar = '\0';
		pchar++;
		i++;
	}
	file_name[i] = '\0';

	ret = uda_realpath(file, absolute_path);
	if (ret)
		return ret;

	dir_break_char(absolute_path, absolute_len);

	strncat(absolute_path, file_name, absolute_len - strlen(absolute_path) - 1);

	return 0;
}

static int check_file_path_dir(const char *file_path, char *absolute_path, uint32_t absolute_len)
{
	char file[TOOL_REAL_PATH_MAX_LEN] = {0};

	if ((file_path == NULL) || (absolute_path == NULL) || (absolute_len == 0))
		return -EFAULT;

	if (strlen(file_path) >= sizeof(file))
		return -EINVAL;

	strncpy(file, file_path, sizeof(file) - 1);

	memset(absolute_path, 0, absolute_len);

	return (strchr(file, DIR_BREAK_CHAR)) == NULL ?
	       check_current_patch_dir(file, absolute_path, absolute_len) :
	       check_absolute_patch_dir(file, absolute_path, absolute_len);
}

int check_file_access(const char *file_dir)
{
	if (file_dir == NULL)
		return -ENXIO;

	return uda_access(file_dir);
}

static uint32_t get_file_size(const char *file_dir)
{
	char format_dir[PATH_MAX + 1] = {0};
	struct stat file_stat = { 0 };
	int ret;

	if (file_dir == NULL)
		return 0;

	ret = check_file_path_dir(file_dir, format_dir, (PATH_MAX + 1));
	if (ret) {
		HIKP_ERROR_PRINT("This file path[%s] is not exist.\n", file_dir);
		return 0;
	}

	ret = stat(format_dir, &file_stat);
	if (ret) {
		HIKP_ERROR_PRINT("Can not get size of file %s.\n", format_dir);
		return 0;
	}
	return (uint32_t)file_stat.st_size;
}

bool is_file_exist(const char *file)
{
	if (file == NULL)
		return false;

	/* access return 0, means file exist, else file no exist. */
	return (uda_access(file) == 0);
}

bool is_dir_exist(const char *path)
{
	return is_file_exist(path);
}

int tool_mk_dir(const char *path)
{
	if (path == NULL)
		return -EFAULT;

	return (int)mkdir(path, 0700);
}

int tool_flock(const char *name, uint32_t operation, int *fd, const char *log_dir)
{
	char lock_file[TOOL_LOCK_PATH_MAX_LEN] = {0};
	size_t file_len;
	int ret;

	if (!name || !fd || !log_dir)
		return -EFAULT;

	file_len = strlen(log_dir) + strlen(TOOL_LOCK_FLODER_NAME) + strlen(name);
	if (file_len > TOOL_LOCK_PATH_MAX_LEN - 1)
		return -EINVAL;

	if (!is_dir_exist(log_dir)) {
		if (tool_mk_dir(log_dir)) {
			HIKP_ERROR_PRINT("manage folder [%s] can not be created.\n", log_dir);
			return -ENOENT;
		}
	}

	ret = snprintf(lock_file, sizeof(lock_file), "%s", log_dir);
	if (ret < 0 || (size_t)ret >= sizeof(lock_file)) {
		HIKP_ERROR_PRINT("generate flock [%s] folder name failed, errno is %d\n",
				 log_dir, errno);
		return -errno;
	}

	strncat(lock_file, TOOL_LOCK_FLODER_NAME, HIKP_STR_BUF_LEFT_LEN(lock_file));
	if (!is_dir_exist(lock_file)) {
		if (tool_mk_dir(lock_file)) {
			HIKP_ERROR_PRINT("lock file folder[%s] can not be created.\n", lock_file);
			return -ENOENT;
		}
	}

	strncat(lock_file, name, HIKP_STR_BUF_LEFT_LEN(lock_file));

	return uda_fcntl(lock_file, operation, fd);
}

void tool_unlock(int *fd, uint32_t operation)
{
	if ((fd != NULL) && (*fd != 0))
		(void)uda_unfcntl(fd, operation);
}

int file_rollback(const char *cur_file, const char *backup_file, uint32_t file_max_size)
{
	bool check = (cur_file == NULL) || (backup_file == NULL) || (file_max_size == 0);
	bool rename_flag = false;
	FILE *fd = NULL;
	int ret;
	uint32_t file_size;

	if (check)
		return -EFAULT;

	ret = check_file_access(cur_file);
	if (ret == 0) {
		file_size = get_file_size(cur_file);
		if (file_size > file_max_size) {
			ret = rename(cur_file, backup_file);
			if (ret != 0) {
				HIKP_ERROR_PRINT("rename file(%s) fail, errno is %d\n",
						 cur_file, errno);
				return -errno;
			}
			rename_flag = true;
		} else {
			return FILE_LEN_OK;
		}
	}

	fd = fopen(cur_file, "a+");
	if (fd == NULL) {
		HIKP_ERROR_PRINT("open %s fail, errno is %d\n", cur_file, errno);
		return -errno;
	}
	(void)chmod(cur_file, 0640);
	(void)fclose(fd);

	return rename_flag ? 0 : FILE_LEN_OK;
}

static int get_rand_str(char *str, int length)
{
#define TYPE_NUMBER 0
#define TYPE_UPPERCASE 1
#define TYPE_LOWERCASE 2
#define RANDOM_CHAR_TYPE_NUM 3
#define RANDOM_NUM 2
	struct type_trans type_arr[RANDOM_CHAR_TYPE_NUM] = {
		[TYPE_NUMBER] = {'0', 10},
		[TYPE_UPPERCASE] = {'A', 26},
		[TYPE_LOWERCASE] = {'a', 26},
	};
	uint32_t r[RANDOM_NUM];
	uint32_t type;
	int fd, size;
	int i, j;

	fd = open("/dev/urandom", O_RDONLY);
	if (fd < 0) {
		HIKP_ERROR_PRINT("open urandom fail, errno is %d\n", errno);
		return -errno;
	}
	for (i = 0; i < (length - 1); i++) {
		for (j = 0; j < RANDOM_NUM; j++) {
			size = read(fd, &r[j], sizeof(uint32_t));
			if (size < 0) {
				HIKP_ERROR_PRINT("read fd fail, errno is %d\n", errno);
				close(fd);
				return -errno;
			}
		}
		type = r[0] % RANDOM_CHAR_TYPE_NUM;
		str[i] = type_arr[type].type_base + r[1] % type_arr[type].type_size;
	}
	close(fd);

	return 0;
}

int generate_file_name(unsigned char *file_name,
		       uint32_t file_name_len, const unsigned char *prefix)
{
#define RANDOM_STR_LENGTH 7
	char str_r[RANDOM_STR_LENGTH] = {0};
	time_t time_seconds = time(0);
	struct tm timeinfo = {0};
	int ret;

	ret = get_rand_str(str_r, RANDOM_STR_LENGTH);
	if (ret) {
		HIKP_ERROR_PRINT("get randrom string failed.\n");
		return ret;
	}
	(void)localtime_r(&time_seconds, &timeinfo);
	ret = snprintf((char *)file_name, file_name_len, "%s_%d_%d_%d_%d_%d_%d_%s.log", prefix,
		       timeinfo.tm_year + START_YEAR, timeinfo.tm_mon + 1, timeinfo.tm_mday,
		       timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec, str_r);
	if (ret < 0 || (uint32_t)ret >= file_name_len) {
		HIKP_ERROR_PRINT("generate file name failed, errno is %d\n", errno);
		return -errno;
	}

	return 0;
}

bool tool_can_print(uint32_t interval, uint32_t burst, uint32_t *print_num, uint64_t *last_time)
{
	uint64_t cur_time;

	if (!print_num || !last_time)
		return false;

	cur_time = (uint64_t)time(NULL);
	if ((*last_time + interval) == cur_time)
		*print_num = 0;

	if (*print_num < (burst)) {
		(*print_num)++;
		*last_time = cur_time;
		return true;
	}

	return false;
}
