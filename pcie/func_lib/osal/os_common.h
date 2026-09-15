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

#ifndef OS_COMMON_H
#define OS_COMMON_H

#define LOG_PCIE "[PCIE]"

#define INFO(x, args...) printf(x, ##args)

#define ERR(x, args...) INFO("[ ERROR ] " x, ##args)

#define WARN(x, args...) INFO("[ WARN ] " x, ##args)

#define DEBUG(x, args...) INFO("[ DEBUG ] " x, ##args)

#define TIPS(x, args...) INFO("[ TIPS ] " x, ##args)

#endif /* OS_COMMON_H */
