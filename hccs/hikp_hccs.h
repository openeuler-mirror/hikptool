/*
 * Copyright (c) 2024-2025 Hisilicon Technologies Co., Ltd.
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

#ifndef HIKP_HCCS_H
#define HIKP_HCCS_H

#include <stdint.h>

enum hikp_hccs_cmd_type {
	HCCS_GET_CHIP_NUM = 0,
	HCCS_GET_DIE_NUM,
	HCCS_GET_DIE_INFO,
	HCCS_GET_PORT_IDS_ON_DIE,
	HCCS_GET_PORT_FIXED_ATTR,
	HCCS_GET_PORT_DFX_INFO,
};

struct hccs_die_info {
	uint8_t die_id;
	uint16_t port_num;
	uint8_t *port_ids;
};

struct hccs_chip_info {
	uint8_t chip_id;
	uint8_t die_num;
	struct hccs_die_info *die_info;
};

struct hccs_port_fixed_attr {
	uint8_t hccs_type; /* HCCS_V1, HCCS_V2 and etc. */
	uint8_t lane_mode;
	uint16_t speed; /* Unit Mbps. */
	uint8_t enabled; /* Indicate if port is enabled. */
	uint8_t rsv[3];
};

struct hccs_port_dfx_info {
	uint8_t link_fsm;
	uint8_t cur_lane_num;
	uint16_t lane_mask;
	uint32_t crc_err_cnt;
	uint32_t retry_cnt;
	uint32_t phy_reinit_cnt;
	uint32_t tx_credit;
	uint32_t rsv1[54];
};

struct hccs_port_dfx_info_vld {
	struct hccs_port_dfx_info info;
	uint8_t vld_size; /* Indicate the valid bytes firmware reported. */
};

union hccs_feature_info {
	struct hccs_port_fixed_attr attr;
	struct hccs_port_dfx_info_vld dfx_info;
};

struct hccs_chip_num_rsp_data {
	uint8_t chip_num;
};

struct hccs_die_num_req_para {
	uint8_t chip_id;
};

struct hccs_die_num_rsp_data {
	uint8_t die_num;
};

struct hccs_die_info_req_para {
	uint8_t chip_id;
	uint8_t die_idx;
};

struct hccs_die_info_rsp_data {
	uint8_t die_id;
	uint8_t rsv;
	uint16_t port_num;
};

struct hccs_die_ports_req_para {
	uint8_t chip_id;
	uint8_t die_id;
};

struct hccs_port_attr_req_para {
	uint8_t chip_id;
	uint8_t die_id;
	uint8_t port_id;
};

struct hccs_port_dfx_req_para {
	uint8_t chip_id;
	uint8_t die_id;
	uint8_t port_id;
};

struct hikp_hccs_rsp_head {
	uint16_t total_blk_num;
	uint8_t cur_blk_size;  /* real data size, not contain header size. */
	uint8_t rsv;
};

#define HCCS_MAX_RSP_DATA  59
struct hikp_hccs_rsp {
	struct hikp_hccs_rsp_head rsp_head; /* 4 Byte */
	uint32_t rsp_data[HCCS_MAX_RSP_DATA];
};

struct hikp_hccs_req_head {
	uint16_t blk_id;
	uint16_t rsv;
};

#define HCCS_MAX_REQ_DATA   31
struct hikp_hccs_req {
	struct hikp_hccs_req_head head; /* 4 Byte */
	uint32_t req_data[HCCS_MAX_REQ_DATA];
};

#define HCCS_ENABLE_CHIP_ID	HI_BIT(0)
#define HCCS_ENABLE_DIE_ID	HI_BIT(1)
#define HCCS_ENABLE_PORT_ID	HI_BIT(2)
#define HCCS_PORT_INFO_MASK	(HCCS_ENABLE_CHIP_ID | HCCS_ENABLE_DIE_ID | \
				 HCCS_ENABLE_PORT_ID)

struct hccs_param {
	int feature_idx;
	uint8_t chip_id;
	uint8_t die_id;
	uint32_t port_id;
	/* mask for param passed by user, see HCCS_ENABLE_XXX. */
	uint16_t param_mask;
};

struct hikp_plat_hccs_info {
	uint8_t chip_num;
	struct hccs_chip_info *chip_info;
};

#define HIKP_HCCS_FEATURE_NAME_LEN 20
struct hikp_hccs_feature_cmd {
	const char feature_name[HIKP_HCCS_FEATURE_NAME_LEN];
	uint32_t cmd_code;
	int (*query)(struct hccs_param *param, union hccs_feature_info *info);
	void (*show)(union hccs_feature_info *data);
	uint16_t param_needed;
};

#endif /* HIKP_HCCS_H */
