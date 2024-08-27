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
#ifndef HIKP_COLLECT_SOCIP_H
#define HIKP_COLLECT_SOCIP_H

#define MIN_DIE_ID  0
#define NIMBUS_A_ID 0
#define TOTEM_A_ID  1
#define NIMBUS_B_ID 2
#define TOTEM_C_ID  2
#define TOTEM_B_ID  3
#define INVALID_DIE_ID 0xFF
#define MAX_DIE_ID  3
#define CONTROLLER_MAX_NUM	12

#define DIE_MIN_INDEX     0
#define CPUDIE_MIN_INDEX  0
#define CPUDIE_A_INDEX    0
#define CPUDIE_B_INDEX    1
#define CPUDIE_MAX_INDEX  1
#define IODIE_MIN_INDEX   2
#define IODIE_A0_INDEX    2
#define IODIE_A1_INDEX    3
#define IODIE_B0_INDEX    4
#define IODIE_B1_INDEX    5
#define IODIE_MAX_INDEX   5
#define DIE_MAX_INDEX     5

struct socip_collect_dumpreg_req {
	char *reg_info;
	uint8_t module;
	uint8_t die_id;
	uint8_t controller_id[CONTROLLER_MAX_NUM];
};

struct socip_collect_dumpreg_req socip_hip09_hip10x_reg_arr[] = {
	{
		.reg_info = "gpio_NA",
		.module = HIKP_SOCIP_SUBCMD_DUMPREG_GPIO,
		.die_id = NIMBUS_A_ID,
		.controller_id = {0, 1, CONTROLLER_MAX_NUM},
	},

	{
		.reg_info = "gpio_NB",
		.module = HIKP_SOCIP_SUBCMD_DUMPREG_GPIO,
		.die_id = NIMBUS_B_ID,
		.controller_id = {0, 1, CONTROLLER_MAX_NUM},
	},

	{
		.reg_info = "gpio_TA",
		.module = HIKP_SOCIP_SUBCMD_DUMPREG_GPIO,
		.die_id = TOTEM_A_ID,
		.controller_id = {0, 1, CONTROLLER_MAX_NUM},
	},

	{
		.reg_info = "gpio_TB",
		.module = HIKP_SOCIP_SUBCMD_DUMPREG_GPIO,
		.die_id = TOTEM_B_ID,
		.controller_id = {0, 1, CONTROLLER_MAX_NUM},
	},

	{
		.reg_info = "i2c_NA",
		.module = HIKP_SOCIP_SUBCMD_DUMPREG_I2C,
		.die_id = NIMBUS_A_ID,
		.controller_id = {0, 1, 5, CONTROLLER_MAX_NUM},
	},

	{
		.reg_info = "i2c_NB",
		.module = HIKP_SOCIP_SUBCMD_DUMPREG_I2C,
		.die_id = NIMBUS_A_ID,
		.controller_id = {0, 1, 5, CONTROLLER_MAX_NUM},
	},

	{
		.reg_info = "spi_NA",
		.module = HIKP_SOCIP_SUBCMD_DUMPREG_SPI,
		.die_id = NIMBUS_A_ID,
		.controller_id = {0, CONTROLLER_MAX_NUM},
	},

	{
		.reg_info = "spi_NB",
		.module = HIKP_SOCIP_SUBCMD_DUMPREG_SPI,
		.die_id = NIMBUS_B_ID,
		.controller_id = {0, CONTROLLER_MAX_NUM},
	},

	{
		.reg_info = "sfc_NA",
		.module = HIKP_SOCIP_SUBCMD_DUMPREG_SFC,
		.die_id = NIMBUS_A_ID,
		.controller_id = {0, 1, CONTROLLER_MAX_NUM},
	},

	{
		.reg_info = "sfc_NB",
		.module = HIKP_SOCIP_SUBCMD_DUMPREG_SFC,
		.die_id = NIMBUS_B_ID,
		.controller_id = {0, 1, CONTROLLER_MAX_NUM},
	},

	{
		.reg_info = "btc_NA",
		.module = HIKP_SOCIP_SUBCMD_DUMPREG_SFC,
		.die_id = NIMBUS_A_ID,
		.controller_id = {0, CONTROLLER_MAX_NUM},
	},

	{
		.reg_info = "btc_NB",
		.module = HIKP_SOCIP_SUBCMD_DUMPREG_SFC,
		.die_id = NIMBUS_B_ID,
		.controller_id = {0, CONTROLLER_MAX_NUM},
	},
};

struct socip_collect_dumpreg_req socip_hip11_reg_arr[] = {
	{
		.reg_info = "gpio_IOA0",
		.module = HIKP_SOCIP_SUBCMD_DUMPREG_GPIO,
		.die_id = IODIE_A0_INDEX,
		.controller_id = {0, CONTROLLER_MAX_NUM},
	},

	{
		.reg_info = "gpio_IOA1",
		.module = HIKP_SOCIP_SUBCMD_DUMPREG_GPIO,
		.die_id = IODIE_A1_INDEX,
		.controller_id = {0, CONTROLLER_MAX_NUM},
	},

	{
		.reg_info = "gpio_IOB0",
		.module = HIKP_SOCIP_SUBCMD_DUMPREG_GPIO,
		.die_id = IODIE_B0_INDEX,
		.controller_id = {0, CONTROLLER_MAX_NUM},
	},

	{
		.reg_info = "gpio_IOB1",
		.module = HIKP_SOCIP_SUBCMD_DUMPREG_GPIO,
		.die_id = IODIE_B1_INDEX,
		.controller_id = {0, CONTROLLER_MAX_NUM},
	},

	{
		.reg_info = "gpio_CA",
		.module = HIKP_SOCIP_SUBCMD_DUMPREG_GPIO,
		.die_id = CPUDIE_A_INDEX,
		.controller_id = {0, 1, CONTROLLER_MAX_NUM},
	},

	{
		.reg_info = "gpio_CB",
		.module = HIKP_SOCIP_SUBCMD_DUMPREG_GPIO,
		.die_id = CPUDIE_B_INDEX,
		.controller_id = {0, 1, CONTROLLER_MAX_NUM},
	},

	{
		.reg_info = "i2c_IOA0",
		.module = HIKP_SOCIP_SUBCMD_DUMPREG_I2C,
		.die_id = IODIE_A0_INDEX,
		.controller_id = {0, 1, CONTROLLER_MAX_NUM},
	},

	{
		.reg_info = "i2c_IOA1",
		.module = HIKP_SOCIP_SUBCMD_DUMPREG_I2C,
		.die_id = IODIE_A1_INDEX,
		.controller_id = {0, 1, CONTROLLER_MAX_NUM},
	},

	{
		.reg_info = "i2c_IOB0",
		.module = HIKP_SOCIP_SUBCMD_DUMPREG_I2C,
		.die_id = IODIE_B0_INDEX,
		.controller_id = {0, 1, CONTROLLER_MAX_NUM},
	},

	{
		.reg_info = "i2c_IOB1",
		.module = HIKP_SOCIP_SUBCMD_DUMPREG_I2C,
		.die_id = IODIE_B1_INDEX,
		.controller_id = {0, 1, CONTROLLER_MAX_NUM},
	},

	{
		.reg_info = "i2c_CA",
		.module = HIKP_SOCIP_SUBCMD_DUMPREG_I2C,
		.die_id = CPUDIE_A_INDEX,
		.controller_id = {0, CONTROLLER_MAX_NUM},
	},

	{
		.reg_info = "i2c_CB",
		.module = HIKP_SOCIP_SUBCMD_DUMPREG_I2C,
		.die_id = CPUDIE_B_INDEX,
		.controller_id = {0, CONTROLLER_MAX_NUM},
	},

	{
		.reg_info = "spi_CB",
		.module = HIKP_SOCIP_SUBCMD_DUMPREG_SPI,
		.die_id = CPUDIE_B_INDEX,
		.controller_id = {0, CONTROLLER_MAX_NUM},
	},

	{
		.reg_info = "sfc_CA",
		.module = HIKP_SOCIP_SUBCMD_DUMPREG_SFC,
		.die_id = CPUDIE_A_INDEX,
		.controller_id = {0, CONTROLLER_MAX_NUM},
	},

	{
		.reg_info = "BTC_IOB0",
		.module = HIKP_SOCIP_SUBCMD_DUMPREG_SFC,
		.die_id = IODIE_B0_INDEX,
		.controller_id = {0, CONTROLLER_MAX_NUM},
	},
};
#endif /* HIKP_COLLECT_SOCIP_H */
