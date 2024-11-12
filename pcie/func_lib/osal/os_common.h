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

#define LOG_PCIE "[PCIE]"

#define Info(x, args...) printf(x, ##args)

#define Err(x, args...) Info("[ ERROE ] " x, ##args)

#define Warn(x, args...) Info("[ WARN ] " x, ##args)

#define Debug(x, args...) Info("[ DEBUG ] " x, ##args)

#define Tips(x, args...) Info("[ TIPS ] " x, ##args)

#endif
