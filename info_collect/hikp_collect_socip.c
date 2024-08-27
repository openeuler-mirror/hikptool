/*
 * Copyright (c) 2024 Hisilicon Technologies Co., Ltd.
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

#include <glob.h>
#include "hikp_collect_lib.h"
#include "hikp_collect.h"
#include "tool_lib.h"
#include "hikptdev_plug.h"
#include "hikp_socip.h"
#include "hikp_collect_socip.h"

#define SOCIP_MODULENUM	7
#define MAX_CHIPNUM 30

enum socip_collect_type {
	GPIO,
	SPI,
	I2C,
	SFC,
	USB,
	UART,
	BT,
};

const char *g_socip_modulename[SOCIP_MODULENUM] = {
	"gpio",
	"spi",
	"i2c",
	"sfc",
	"usb",
	"uart",
	"bt",
};

struct info_collect_cmd gpio_cmd_arr[] = {
	{
		.group = GROUP_SOCIP,
		.log_name = "gpio",
		.args = {"cat", "/sys/kernel/debug/gpio", NULL},
	},
};

struct info_collect_cmd spi_cmd_arr[] = {
	{
		.group = GROUP_SOCIP,
		.log_name = "spi",
		.args = {"cat", "/sys/kernel/debug/dw_spi*/registers", NULL},
	},

	{
		.group = GROUP_SOCIP,
		.log_name = "spi",
		.args = {"cat", "/sys/kernel/debug/hisi_spi*/registers", NULL},
	},

	{
		.group = GROUP_SOCIP,
		.log_name = "spi",
		.args = {"cat", "/sys/class/spi_master/spi*/statistics/bytes", NULL},
	},

	{
		.group = GROUP_SOCIP,
		.log_name = "spi",
		.args = {"cat", "/sys/class/spi_master/spi*/statistics/bytes_rx", NULL},
	},

	{
		.group = GROUP_SOCIP,
		.log_name = "spi",
		.args = {"cat", "/sys/class/spi_master/spi*/statistics/bytes_tx", NULL},
	},
};

struct info_collect_cmd i2c_cmd_arr[] = {
	{
		.group = GROUP_SOCIP,
		.log_name = "i2c",
		.args = {"ls", "/sys/class/i2c-adapter", NULL},
	},

	{
		.group = GROUP_SOCIP,
		.log_name = "i2c",
		.args = {"ls", "/sys/class/i2c-dev", NULL},
	},
};

struct info_collect_cmd sfc_cmd_arr[] = {
	{
		.group = GROUP_SOCIP,
		.log_name = "sfc",
		.args = {"mtd_debug", "info", "/dev/mtd0", NULL},
	},
};

struct info_collect_cmd usb_cmd_arr[] = {
	{
		.group = GROUP_SOCIP,
		.log_name = "usb",
		.args = {"cat", "/sys/kernel/debug/usb/devices", NULL},
	},
};

struct info_collect_cmd uart_cmd_arr[] = {
	{
		.group = GROUP_SOCIP,
		.log_name = "uart",
		.args = {"stty", "-F", "/dev/ttyAMA0", "-a", NULL},
	},

	{
		.group = GROUP_SOCIP,
		.log_name = "uart",
		.args = {"cat", "/proc/tty/driver/ttyAMA", NULL},
	},
};

struct info_collect_cmd bt_cmd_arr[] = {
	{
		.group = GROUP_SOCIP,
		.log_name = "bt",
		.args = {"ls", "/dev/ipmi0", NULL},
	},
};

static int socip_get_dumpregparam(struct socip_collect_dumpreg_req req_struct,
				  struct socip_dump_reg_req_data_t *req_data_ptr, uint8_t chip_id, int controller_num)
{
	req_data_ptr->controller_id = req_struct.controller_id[controller_num];
	if (req_data_ptr->controller_id == CONTROLLER_MAX_NUM)
		return -EINVAL;

	req_data_ptr->chip_id = chip_id;
	req_data_ptr->die_id = req_struct.die_id;
	return 0;
}

static int collect_socip_dumpreglog(void *version)
{
	size_t i, msize;
	uint8_t chip_id;
	uint32_t cpu_version = *(uint32_t *)version;
	int controller_num, ret;
	struct hikp_cmd_ret *cmd_ret;
	struct socip_collect_dumpreg_req *req_struct;
	struct socip_dump_reg_req_data_t req_data = {0};
	struct hikp_cmd_header req_header = {0};

	switch (cpu_version) {
	case CHIP_HIP09:
	case CHIP_HIP10:
	case CHIP_HIP10C:
		req_struct = socip_hip09_hip10x_reg_arr;
		msize = HIKP_ARRAY_SIZE(socip_hip09_hip10x_reg_arr);
		break;
	case CHIP_HIP11:
		req_struct = socip_hip11_reg_arr;
		msize = HIKP_ARRAY_SIZE(socip_hip11_reg_arr);
		break;
	default:
		HIKP_ERROR_PRINT("Cpu version not support.\n");
		return 0;
	}

	for (chip_id = 0; chip_id < MAX_CHIPNUM; chip_id++) {
		for (i = 0; i < msize; i++) {
			controller_num = 0;
			while(controller_num < CONTROLLER_MAX_NUM) {
				struct socip_dump_reg_req_data_t *req_data_ptr = &req_data;
				ret = socip_get_dumpregparam(req_struct[i], req_data_ptr, chip_id, controller_num);
				if (ret)
					break;

				hikp_cmd_init(&req_header, SOCIP_MOD, HIKP_SOCIP_CMD_DUMPREG,
						req_struct[i].module);
				cmd_ret = hikp_cmd_alloc(&req_header, &req_data, sizeof(req_data));
				if (!cmd_ret || cmd_ret->status != 0) {
					HIKP_ERROR_PRINT("hikp_cmd_alloc failed\n");
					hikp_cmd_free(&cmd_ret);
					return 0;
				}
				printf("%s\n", req_struct[i].reg_info);
				printf("hikptool socip_dumpreg -c %u -d %u -m %u -i %u\n",req_data.chip_id,
					req_data.die_id, req_struct[i].module, req_data.controller_id);
				dump_reg_info(&cmd_ret->rsp_data[0], cmd_ret->rsp_data_num);
				hikp_cmd_free(&cmd_ret);
				controller_num++;
			}
		}
	}

	return 0;
}

static int collect_socip_modulelog(void *module)
{
	size_t i, msize;
	int ret;

	switch (*(int *)module) {
	case GPIO:
		msize = HIKP_ARRAY_SIZE(gpio_cmd_arr);
		for (i = 0; i < msize; i++)
			ret = hikp_collect_exec((void *)&gpio_cmd_arr[i]);
		break;
	case SPI:
		msize = HIKP_ARRAY_SIZE(spi_cmd_arr);
		for (i = 0; i < msize; i++)
			ret = hikp_collect_cat_glob_exec((void *)&spi_cmd_arr[i]);
		break;
	case I2C:
		msize = HIKP_ARRAY_SIZE(i2c_cmd_arr);
		for (i = 0; i < msize; i++)
			ret = hikp_collect_exec((void *)&i2c_cmd_arr[i]);
		break;
	case SFC:
		msize = HIKP_ARRAY_SIZE(sfc_cmd_arr);
		for (i = 0; i < msize; i++)
			ret = hikp_collect_exec((void *)&sfc_cmd_arr[i]);
		break;
	case USB:
		msize = HIKP_ARRAY_SIZE(usb_cmd_arr);
		for (i = 0; i < msize; i++)
			ret = hikp_collect_exec((void *)&usb_cmd_arr[i]);
		break;
	case UART:
		msize = HIKP_ARRAY_SIZE(uart_cmd_arr);
		for (i = 0; i < msize; i++)
			ret = hikp_collect_exec((void *)&uart_cmd_arr[i]);
		break;
	case BT:
		msize = HIKP_ARRAY_SIZE(bt_cmd_arr);
		for (i = 0; i < msize; i++)
			ret = hikp_collect_exec((void *)&bt_cmd_arr[i]);
		break;
	default:
		ret = 0;
		break;
	}

	return ret;
}

void collect_socip_log(void)
{
	int i, ret;
	uint32_t cpu_version = get_chip_type();

	for (i = 0; i < SOCIP_MODULENUM; i++) {
		ret = hikp_collect_log(GROUP_SOCIP, (char *)g_socip_modulename[i],
					collect_socip_modulelog, (void *)&i);
		if (ret) {
			HIKP_ERROR_PRINT("collect_socip_log %s arr failed: %d\n",
					 g_socip_modulename[i], ret);
		}
	}

	ret = hikp_collect_log(GROUP_SOCIP, "dumpreg", collect_socip_dumpreglog, (void *)&cpu_version);
	if (ret)
		HIKP_ERROR_PRINT("collect_socip_log dumpreg failed: %d\n", ret);
}
