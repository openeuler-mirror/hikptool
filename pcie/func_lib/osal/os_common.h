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

#ifndef _OS_COMMON_H_
#define _OS_COMMON_H_

enum {
	HW_LOG_TIPS = 0,
	HW_LOG_DBG,
	HW_LOG_INFO,
	HW_LOG_WARN,
	HW_LOG_ERR
};

#define LOG_PCIE "[PCIE]"

void hilog(int pri, const char *module, const char *fun, int line, const char *fmt, ...);

#define Log(pri, module, fmt, args...) hilog(pri, module, __func__, __LINE__, fmt, ##args)

#define Err(module, fmt, args...) hilog(HW_LOG_ERR, module, __func__, __LINE__, fmt, ##args)

#define Warn(module, fmt, args...) hilog(HW_LOG_WARN, module, __func__, __LINE__, fmt, ##args)

#define Info(module, fmt, args...) hilog(HW_LOG_INFO, module, __func__, __LINE__, fmt, ##args)

#define Debug(module, fmt, args...) hilog(HW_LOG_DBG, module, __func__, __LINE__, fmt, ##args)

#define Tips(module, fmt, args...) hilog(HW_LOG_TIPS, module, __func__, __LINE__, fmt, ##args)

#endif
