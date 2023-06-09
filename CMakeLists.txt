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

project(hikptool C)

set(CMAKE_C_FLAGS
	"${CMAKE_C_FLAGS} -O2 -fPIC -fPIE -Wall -Wextra -fno-common -std=gnu11 -Wfloat-equal")
set(CMAKE_SKIP_RPATH TRUE)

macro(get_header_dir_recurse HEADER_DIR_LIST)
    file(GLOB_RECURSE HEADER_LIST *.h)
    set(DIR_LIST "")
    foreach(HEADER ${HEADER_LIST})
        get_filename_component(DIR_PATH ${HEADER} PATH)
        set(DIR_LIST ${DIR_LIST} ${DIR_PATH})
    endforeach()
    list(REMOVE_DUPLICATES DIR_LIST)
    set(${HEADER_DIR_LIST} ${DIR_LIST})
endmacro()

file(GLOB_RECURSE HIKPTOOL_SRC
	${CMAKE_CURRENT_SOURCE_DIR}/cxl/*.c
	${CMAKE_CURRENT_SOURCE_DIR}/net/*.c
	${CMAKE_CURRENT_SOURCE_DIR}/ossl/*.c
	${CMAKE_CURRENT_SOURCE_DIR}/pcie/*.c
	${CMAKE_CURRENT_SOURCE_DIR}/sas/*.c
	${CMAKE_CURRENT_SOURCE_DIR}/sata/*.c
	${CMAKE_CURRENT_SOURCE_DIR}/serdes/*.c
	${CMAKE_CURRENT_SOURCE_DIR}/socip/*.c
	${CMAKE_CURRENT_SOURCE_DIR}/tool_lib/*.c
	${CMAKE_CURRENT_SOURCE_DIR}/hikp_init_main.c
	)
add_subdirectory(libhikptdev)
add_executable(hikptool ${HIKPTOOL_SRC})
get_header_dir_recurse(HIKPTOOL_HEADER_DIR)
target_include_directories(hikptool PRIVATE ${HIKPTOOL_HEADER_DIR})
target_link_directories(hikptool PRIVATE ${CMAKE_INSTALL_PREFIX}/lib)
target_link_libraries(hikptool PRIVATE KPTDEV_SO)
target_link_options(hikptool PRIVATE
	-Wl,-z,relro,-z,now -Wl,-z,noexecstack -fPIE -pie
	-g -lpthread -ldl -lm -lrt -T ${CMAKE_CURRENT_SOURCE_DIR}/hikp_register.ld)
install(TARGETS hikptool RUNTIME DESTINATION bin OPTIONAL)
