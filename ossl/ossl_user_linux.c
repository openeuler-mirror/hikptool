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

#include <sys/syslog.h>
#include <unistd.h>
#include "tool_lib.h"
#include "ossl_user_linux.h"

static struct flock g_fcntl_lock = {
	.l_whence = SEEK_SET,
	.l_start = 0,
	.l_len = 0,
};

int uda_access(const char *file_dir)
{
	char path[PATH_MAX + 1] = { 0 };

	if (strlen(file_dir) > PATH_MAX || realpath(file_dir, path) == NULL)
		return -ENOENT;

	return faccessat(AT_FDCWD, path, F_OK, AT_EACCESS) ? (-ENOENT) : 0;
}

int uda_realpath(const char *file_dir, char *format_dir)
{
	if (file_dir == NULL || format_dir == NULL)
		return -EFAULT;

	if (realpath(file_dir, format_dir) == NULL) {
		perror("realpath failed");
		return -errno;
	}

	return 0;
}

int uda_fcntl(const char *lock_file, uint32_t operation, int *fd)
{
	int fd_t = 0;
	int ret;

	if ((fd == NULL) || (lock_file == NULL))
		return -EFAULT;

	fd_t = open(lock_file, O_WRONLY | O_CREAT, 0700);
	if (fd_t < 0)
		return -errno;

	*fd = fd_t;

	g_fcntl_lock.l_type = F_WRLCK;
	if (operation == UDA_FLOCK_NOBLOCK)
		ret = fcntl(fd_t, F_SETLK, &g_fcntl_lock);
	else
		ret = fcntl(fd_t, F_SETLKW, &g_fcntl_lock);
	if (ret != 0)
		close(fd_t);

	return ret;
}

int uda_unfcntl(const int *fd, const uint32_t operation)
{
	int fd_t;

	if (fd == NULL)
		return -EFAULT;

	if (*fd < 0)
		return -EBADFD;

	fd_t = *fd;
	g_fcntl_lock.l_type = F_UNLCK;
	if (operation == UDA_FLOCK_NOBLOCK)
		(void)fcntl(fd_t, F_SETLK, &g_fcntl_lock);
	else
		(void)fcntl(fd_t, F_SETLKW, &g_fcntl_lock);

	return close(fd_t);
}

void record_syslog(const char *ident, const int priority, const char *logs)
{
	if (!logs || !ident) {
		printf("Invalid parameter [%s].\n", __func__);
		return;
	}

	openlog(ident, LOG_CONS | LOG_PID, LOG_USER);
	syslog(priority, "%s", logs);
	closelog();
}
