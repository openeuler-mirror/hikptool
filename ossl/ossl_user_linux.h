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

#ifndef OSSL_USER_LINUX_H
#define OSSL_USER_LINUX_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <assert.h>
#include <limits.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <stdint.h>

#undef NULL
#if defined(__cplusplus)
#define NULL 0
#else
#define NULL ((void *)0)
#endif

#ifndef F_OK
#define F_OK 0
#endif

#define TOOL_REAL_PATH_MAX_LEN 512

#define HIKP_LOG_DIR_PATH "/var/log/hikp/operation_logs/"

#define PWD_STR "./"
#define DIR_BREAK_CHAR '/'
#define DIR_BREAK_STRING "/"

#ifndef true
#define true 1
#endif
#ifndef false
#define false 0
#endif
#ifndef IFNAMSIZ
#define IFNAMSIZ 16
#endif

enum {
	UDA_FLOCK_NOBLOCK = 0,
	UDA_FLOCK_BLOCK = 1
};

extern int uda_access(const char *file_dir);
extern int uda_realpath(const char *file_dir, char *format_dir);
extern int uda_fcntl(const char *lock_file, uint32_t operation, int *fd);
int uda_unfcntl(const int *fd, const uint32_t operation);

extern void record_syslog(const char *ident, const int priority, const char *logs);

#endif /* OSSL_USER_LINUX_H */
