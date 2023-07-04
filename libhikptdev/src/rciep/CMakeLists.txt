# Copyright (c) 2022 Hisilicon Technologies Co., Ltd.
# Hikptool is licensed under Mulan PSL v2.
# You can use this software according to the terms and conditions of the Mulan PSL v2.
# You may obtain a copy of Mulan PSL v2 at:
#          http://license.coscl.org.cn/MulanPSL2
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
# EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
# MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
#
# See the Mulan PSL v2 for more details.

cmake_minimum_required(VERSION 3.0.0)

set(KPTDEV_SO_NAME hikptdev)

add_library(KPTDEV_SO SHARED hikpt_rciep.c)

target_include_directories(KPTDEV_SO PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../include)

target_link_options(KPTDEV_SO PRIVATE -Wl,-z,relro,-z,now -Wl,-z,noexecstack -fPIE -pie -s)

set_target_properties(KPTDEV_SO PROPERTIES OUTPUT_NAME ${KPTDEV_SO_NAME} SOVERSION 1 VERSION 1.0.0)
install(TARGETS KPTDEV_SO LIBRARY DESTINATION lib OPTIONAL)
