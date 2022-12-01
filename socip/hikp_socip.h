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
#ifndef __HIKP_SOCIP_H__
#define __HIKP_SOCIP_H__

#include <stdint.h>

/***************************************************
			 hikptool socip module
****************************************************/

/* hikptool socip command code */
enum {
	// socip.dumpreg
	HIKP_SOCIP_CMD_DUMPREG = 1,
};

/* hikptool socip sub command code */
enum {
	// socip.dumpreg.i2c
	HIKP_SOCIP_SUBCMD_DUMPREG_I2C = 1,
	// socip.dumpreg.gpio
	HIKP_SOCIP_SUBCMD_DUMPREG_GPIO = 2,
	// socip.dumpreg.spi
	HIKP_SOCIP_SUBCMD_DUMPREG_SPI = 3,
	// socip.dumpreg.sfc
	HIKP_SOCIP_SUBCMD_DUMPREG_SFC = 4,
	// socip.dumpreg.btc
	HIKP_SOCIP_SUBCMD_DUMPREG_BTC = 5,
};

// dumpreg.request
struct socip_dump_reg_req_data_t {
	uint8_t chip_id;
	uint8_t die_id;
	uint8_t controller_id;
};

#endif /* __HIKP_SOCIP_H__ */
