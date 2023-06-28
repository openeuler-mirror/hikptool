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

#ifndef __PCIE_COMMON_H_
#define __PCIE_COMMON_H_

/* PCIe command code */
enum pcie_cmd_type {
	PCIE_TRACE = 0,
	PCIE_INFO = 1,
	PCIE_DUMP = 2,
	PCIE_REGRD = 3,
};

enum pcie_trace_cmd_type {
	TRACE_HELP = 0,
	TRACE_SHOW = 1,
	TRACE_CLEAR = 2,
	TRACE_INFO = 3,
	TRACE_MODE = 4,
	TRACE_PM = 5,
};

enum pcie_info_cmd_type {
	INFO_HELP = 0,
	INFO_DISTRIBUTION = 1,
	INFO_ERR_STATE_SHOW = 2,
	INFO_ERR_STATE_CLEAR = 3,
};

enum pcie_dump_cmd_type {
	DUMPREG_HELP = 0,
	DUMPREG_DUMP = 1,
};

enum pcie_reg_read_cmd_type {
	REGRD_HELP = 0,
	REGRD_READ = 1,
};

#endif
