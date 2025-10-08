/*
 * Copyright (c) 2025 Hisilicon Technologies Co., Ltd.
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

#ifndef HIKP_NIC_LED_H
#define HIKP_NIC_LED_H

#include "hikp_net_lib.h"

enum nic_led_sub_cmd_type {
	NIC_LED_CFG_DUMP = 1,
};

#define LED_PORT_TARGET_BIT		HI_BIT(0)

struct speed_status {
	uint8_t status;
	const char *speed;
};

#define SPEED_1G_BLINK		0x0
#define SPEED_10G_BLINK		0x1
#define SPEED_25G_BLINK		0x8
#define SPEED_40G_BLINK		0x2
#define SPEED_50G_BLINK		0x6
#define SPEED_100G_BLINK	0xa
#define SPEED_200G_BLINK	0x16
#define SPEED_NO_LINK_BLINK	0x10
#define SPEED_TEST_BLINK	0xe
#define SPEED_ERROR_BLINK	0x9

struct nic_led_req_para {
	struct bdf_t bdf;
	uint32_t rsvd[2];
};

struct nic_led_resp {
	uint8_t led_en;
	uint8_t hw_err_mode;
	uint8_t hw_locate_mode;
	uint8_t hw_active_mode;
	uint8_t sw_err_mode;
	uint8_t sw_locate_mode;
	uint8_t sw_active_mode;
	uint8_t speed_led_status;

	uint32_t rsvd[58];
};

struct nic_led_cmd_info {
	struct tool_target port_dev;
	uint32_t cmd_flag;
};

#endif /* HIKP_NIC_LED_H */
