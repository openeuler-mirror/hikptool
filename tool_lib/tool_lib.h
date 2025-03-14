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

#ifndef TOOL_LIB_H
#define TOOL_LIB_H

#include "ossl_user_linux.h"

#define TOOL_NAME "hikptool"

#define TOOL_VER "1.1.4"

#define HI_GET_BITFIELD(value, start, mask) (((value) >> (start)) & (mask))
#define HI_SET_FIELD(origin, shift, val)	((origin) |= (val) << (shift))

#define HI_BIT(shift) (1UL << (shift))
#define HI_SETBIT(x, y) ((x) |= (HI_BIT(y)))
#define HI_CLRBIT(x, y) ((x) &= (~(HI_BIT(y))))
#define HI_GETBIT(x, y) ((x) & (HI_BIT(y)))

#define HIKP_ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

#define HIKP_DIV_ROUND_UP(n, d) (((n) + (d) - 1) / (d))

#define HIKP_MIN(a, b)          ((a) < (b) ? (a) : (b))

#define BITS_PER_LONG	(sizeof(long) * 8)
#define GENMASK(h, l) (((~0UL) << (l)) & (~0UL >> (BITS_PER_LONG - 1 - (h))))

#define hikp_get_field(origin, mask, shift) (((origin) & (mask)) >> (shift))
#define hikp_get_bit(origin, shift) hikp_get_field((origin), (0x1UL << (shift)), (shift))

#define HIKP_STR_BUF_LEFT_LEN(str)	(sizeof(str) - strlen(str) - 1)

#define HIKP_BITS_PER_BYTE  8

#define MAX_CMD_LEN 30
#define MAX_HELP_INFO_LEN 100
struct hikp_cmd_type {
	char name[MAX_CMD_LEN];
	char help_info[MAX_HELP_INFO_LEN];
	void (*cmd_init)(void);
};

#define _cmd_data_ __attribute__((section(".cmd_data")))

#define HIKP_CMD_DECLARE(cmd_name, help_info, init_func)                           \
	static volatile struct hikp_cmd_type _cmd_data_ g_hikp_##init_func##_arr[] = { \
		{cmd_name, help_info, init_func},                                        \
	}

struct type_trans {
	char type_base;
	uint32_t type_size;
};

#define FILE_LEN_OK 27
#define TOOL_LOCK_PATH_MAX_LEN 512
#define TOOL_LOCK_FLODER_NAME "locks/"

#define MAX_LOG_NAME_LEN 128
#define START_YEAR 1900

#define HIKP_DEF_RATELIMIT_INTERVAL 5U
#define HIKP_DEF_RATELIMIT_BURST 10U

#define HIKP_LOG_PRINT(x, args...)                      \
	do {                                                \
		static uint32_t print_num;                  \
		static uint64_t last_time;                  \
		bool can_print = tool_can_print(HIKP_DEF_RATELIMIT_INTERVAL, \
			HIKP_DEF_RATELIMIT_BURST, &print_num, &last_time);                    \
		if (can_print) {                                \
			printf(x, ##args);                          \
		}                                               \
	} while (0)

#define HIKP_CRITICAL_PRINT(x, args...) HIKP_LOG_PRINT("[ CRITICAL ] " x, ##args)
#define HIKP_ERROR_PRINT(x, args...) HIKP_LOG_PRINT("[ ERROR ] " x, ##args)
#define HIKP_WARN_PRINT(x, args...) HIKP_LOG_PRINT("[ WARN ] " x, ##args)
#define HIKP_INFO_PRINT(x, args...) HIKP_LOG_PRINT("[ INFO ] " x, ##args)
#define HIKP_DBG_PRINT(x, args...) HIKP_LOG_PRINT("[ DBG ] " x, ##args)

/* definition to mark a variable or function parameter as used so
 * as to avoid a compiler warning
 */
#define HIKP_SET_USED(x) (void)(x)

#define MIDR_EL1_PATH "/sys/devices/system/cpu/cpu0/regs/identification/midr_el1"
#define MIDR_BUFFER_SIZE 20
#define PART_NUM_OFFSET 4
#define MIDR_HEX_TYPE 16

enum chip_type {
	CHIP_HIP09,
	CHIP_HIP10,
	CHIP_HIP10C,
	CHIP_HIP11,
	CHIP_UNKNOW,
};

uint32_t get_chip_type(void);

int string_toui(const char *nptr, uint32_t *value);
int string_toub(const char *nptr, uint8_t *value);

int check_file_access(const char *file_dir);
int tool_flock(const char *name, uint32_t operation, int *fd, const char *log_dir);
void tool_unlock(int *fd, uint32_t operation);
int file_rollback(const char *cur_file, const char *backup_file, uint32_t file_max_size);
bool is_dir_exist(const char *path);
int tool_mk_dir(const char *path);
bool is_file_exist(const char *file);
int generate_file_name(unsigned char *file_name, uint32_t file_name_len,
		       const unsigned char *prefix);
bool tool_can_print(uint32_t interval, uint32_t burst, uint32_t *print_num, uint64_t *last_time);

#endif /* TOOL_LIB_H */
