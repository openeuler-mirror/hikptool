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

#ifndef HIKP_NIC_PORT_H
#define HIKP_NIC_PORT_H

#include "hikp_net_lib.h"

#define MAC_LSPORT_LINK     HI_BIT(0)
#define MAC_LSPORT_MAC      HI_BIT(1)
#define MAC_LSPORT_PHY      HI_BIT(2)
#define MAC_LSPORT_ARB      HI_BIT(3)
#define MAC_HOT_PLUG_CARD      HI_BIT(4)

enum {
	PORT_CFG_NOT_SET = 0,
	PORT_CFG_AN_ON = 1,
	PORT_CFG_AN_OFF = 2,
};

enum {
	PORT_CFG_FEC_NOT_SET = 0,
	PORT_CFG_FEC_RSFEC = 1,
	PORT_CFG_FEC_BASEFEC = 2,
	PORT_CFG_FEC_NOFEC = 3,
	PORT_CFG_FEC_AUTO = 4,
	PORT_CFG_FEC_LLRSFEC = 5
};

enum {
	PORT_SPEED_NOT_SET = 0,
	PORT_SPEED_10MB = 1,
	PORT_SPEED_100MB = 2,
	PORT_SPEED_1GB = 3,
	PORT_SPEED_10GB = 4,
	PORT_SPEED_25GB = 5,
	PORT_SPEED_40GB = 6,
	PORT_SPEED_50GB = 7,
	PORT_SPEED_100GB = 8,
	PORT_SPEED_200GB = 9,
	PORT_SPEED_UNKNOWN = 0xff,
};

enum {
	PORT_CFG_ADAPT_NOT_SET = 0,
	PORT_CFG_ADAPT_ON = 1,
	PORT_CFG_ADAPT_OFF = 2,
};

enum {
	HIKP_MAC_DUPLEX_UNKNOWN = 0,
	HIKP_MAC_DUPLEX_FULL,
	HIKP_MAC_DUPLEX_HALF
};

enum {
	HIKP_MAC_SDS_RATE_UNKNOWN = 0,
	HIKP_MAC_SDS_RATE_1P25G,
	HIKP_MAC_SDS_RATE_10P3125G,
	HIKP_MAC_SDS_RATE_25P78125G,
	HIKP_MAC_SDS_RATE_26P5625G,
	HIKP_MAC_SDS_RATE_53P125G
};

enum {
	HIKP_MAC_LANES_UNKNOWN = 0,
	HIKP_MAC_LANES_X1 = 1,
	HIKP_MAC_LANES_X2 = 2,
	HIKP_MAC_LANES_X4 = 4,
	HIKP_MAC_LANES_X8 = 8,
};

enum {
	HIKP_PHY_DUPLEX_HALF = 0,
	HIKP_PHY_DUPLEX_FULL,
};

#define HIKP_MAC_PHY_ABI_10M_HALF       HI_BIT(0)
#define HIKP_MAC_PHY_ABI_10M_FULL       HI_BIT(1)
#define HIKP_MAC_PHY_ABI_100M_HALF      HI_BIT(2)
#define HIKP_MAC_PHY_ABI_100M_FULL      HI_BIT(3)
#define HIKP_MAC_PHY_ABI_1000M_HALF     HI_BIT(4)
#define HIKP_MAC_PHY_ABI_1000M_FULL     HI_BIT(5)
#define HIKP_MAC_PHY_ABI_AUTONEG        HI_BIT(6)
#define HIKP_MAC_PHY_ABI_PAUSE          HI_BIT(13)
#define HIKP_MAC_PHY_ABI_ASYM_PAUSE     HI_BIT(14)

#define HIKP_PORT_TYPE_PHY		1
#define HIKP_PORT_TYPE_PHY_SDS		3

struct mac_item {
	uint32_t key;
	const char *name;
};

struct mac_cmd_port_hardware {
	uint8_t port_type;
	uint8_t media_type;
};

struct mac_cmd_mac_dfx {
	uint8_t autoneg;
	uint8_t speed;
	uint8_t fec;
	uint8_t duplex;
	uint8_t lanes;
	uint8_t sds_rate;
	uint8_t rf_lf;
	uint8_t mac_tx_en    : 1;
	uint8_t mac_rx_en    : 1;
	uint8_t pma_ctrl     : 2;
	uint8_t pcs_link     : 1;
	uint8_t mac_link     : 1;
	uint8_t reserve      : 2;
	uint32_t pcs_err_cnt;
};

struct mac_port_param {
	uint8_t adapt;
	uint8_t an;
	uint8_t speed;
	uint8_t lanes;
	uint8_t sds_rate;
	uint8_t fec;
	uint8_t duplex;
	uint32_t rsvd;
};

struct mac_cmd_link_dfx {
	struct mac_port_param port_cfg;
	uint8_t port_link        : 1;
	uint8_t port_enable      : 1;
	uint8_t link_debug_en    : 1;
	uint8_t link_report_en   : 1;
	uint8_t reserve          : 4;
	uint8_t loop_type;
	uint8_t reserve_1[2];
	uint32_t cur_link_machine;
	uint32_t his_link_machine;
};

struct mac_cmd_arb_dfx {
	struct mac_port_param default_cfg;
	struct mac_port_param bios_cfg;
	struct mac_port_param user_cfg;
	struct mac_port_param port_cfg;
	struct mac_port_param arb_cfg;
};

#define MAC_PHY_DFX_REG_NUM     12
struct mac_cmd_phy_dfx {
	uint16_t reg_val[MAC_PHY_DFX_REG_NUM];
};

struct mac_cfg_phy_cfg {
	uint32_t speed;
	uint8_t duplex;
	uint8_t autoneg;
	uint8_t mac_tp_mdix;
	uint8_t mac_tp_mdix_ctrl;
	uint8_t port;
	uint8_t transceiver;
	uint8_t phy_addr;
	uint8_t reserved;
	uint32_t supported;
	uint32_t advertising;
	uint32_t lp_advertising;
	/* reserved cfg */
	uint8_t master_slve_cfg;
	uint8_t master_slave_state;
	uint8_t rsvd[2];
};

struct mac_cmd_phy_info {
	struct mac_cfg_phy_cfg phy_cfg;
	struct mac_cmd_phy_dfx phy_dfx;
};

struct mac_cmd_dfx_callback {
	uint32_t mask;
	void (*show_dfx)(struct major_cmd_ctrl *self);
};

struct cmd_port_info {
	struct tool_target target;
	bool port_flag;
};

struct cmd_hot_plug_card_info {
	uint8_t in_pos;
	uint8_t support_type;
	uint8_t cur_type;
};

#endif
