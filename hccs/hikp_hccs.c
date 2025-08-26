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

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <inttypes.h>
#include "tool_cmd.h"
#include "hikpt_rciep.h"
#include "hikp_hccs.h"

enum hccs_link_fsm {
	HCCS_PORT_RESET = 1,
	HCCS_PORT_SETUP,
	HCCS_PORT_CONFIG,
	HCCS_PORT_READY
};

const struct {
	enum hccs_link_fsm link_fsm;
	const char *str;
} link_fsm_map[] = {
	{HCCS_PORT_RESET,		"reset"},
	{HCCS_PORT_SETUP,		"setup"},
	{HCCS_PORT_CONFIG,		"config"},
	{HCCS_PORT_READY,		"link-up"},
};

static struct hccs_param g_hccs_param = { 0 };
static struct hikp_plat_hccs_info g_hccs_info = { 0 };

static int hikp_hccs_get_plat_topo(struct hccs_param *param,
				   union hccs_feature_info *info);
static void hikp_hccs_show_topo(union hccs_feature_info *data);
static int hikp_hccs_get_port_attr(struct hccs_param *param,
				   union hccs_feature_info *info);
static void hikp_hccs_show_port_attr(union hccs_feature_info *feature_info);
static int hikp_hccs_get_port_dfx_info(struct hccs_param *param,
				       union hccs_feature_info *info);
static void hikp_hccs_show_port_dfx_info(union hccs_feature_info *feature_info);
static int hikp_plat_hccs_hw_info(struct hikp_plat_hccs_info *hccs_info);
static int hikp_hccs_get_die_num(uint8_t chip_id, struct hikp_plat_hccs_info *hccs_info);

static const struct hikp_hccs_feature_cmd g_hccs_feature_cmd[] = {
	{"topo", HCCS_GET_PORT_IDS_ON_DIE, hikp_hccs_get_plat_topo,
		hikp_hccs_show_topo, 0},
	{"fixed_attr", HCCS_GET_PORT_FIXED_ATTR, hikp_hccs_get_port_attr,
		hikp_hccs_show_port_attr, HCCS_PORT_INFO_MASK},
	{"dfx_info", HCCS_GET_PORT_DFX_INFO, hikp_hccs_get_port_dfx_info,
		hikp_hccs_show_port_dfx_info, HCCS_PORT_INFO_MASK},
};

static int hikp_hccs_cmd_help(struct major_cmd_ctrl *self, const char *argv)
{
	HIKP_SET_USED(argv);

	printf("\n  Usage: %s\n", self->cmd_ptr->name);
	printf("\n         %s\n", self->cmd_ptr->help_info);
	printf("\n  Options:\n\n");
	printf("    %s, %-25s %s\n", "-h", "--help", "display this help and exit");
	printf("    %s, %-25s %s\n", "-c", "--chip_id=<chip_id>", "target chip");
	printf("    %s, %-25s %s\n", "-d", "--die_id=<die_id>", "target die");
	printf("    %s, %-25s %s\n", "-p", "--port_id=<port_id>", "target port");
	printf("      %s\n",
	       "[-g/--get <options>]\n"
	       "          topo : get hccs_typo info, no target specified.\n"
	       "          fixed_attr : get fixed attributes for port specified by -c X -d X -p X.\n"
	       "          dfx_info : get dfx info for port specified by -c X -d X -p X.\n");
	return 0;
}

static int hikp_hccs_cmd_send(struct hikp_cmd_header *req_header,
			      struct hikp_hccs_req *req_data,
			      void *buff, size_t buff_len,
			      struct hikp_hccs_rsp_head *rsp_head)
{
	struct hikp_cmd_ret *cmd_ret;
	struct hikp_hccs_rsp *rsp;
	uint64_t cur_blk_size;
	int ret;

	cmd_ret = hikp_cmd_alloc(req_header, req_data, sizeof(struct hikp_hccs_req));
	ret = hikp_rsp_normal_check(cmd_ret);
	if (ret != 0) {
		HIKP_ERROR_PRINT("Failed to query from firmware! ret = %d.\n", ret);
		goto out;
	}

	rsp = (struct hikp_hccs_rsp *)cmd_ret->rsp_data;
	cur_blk_size = rsp->rsp_head.cur_blk_size;
	if (cur_blk_size == 0) {
		HIKP_ERROR_PRINT("Firmware reported zero data size!\n");
		ret = -EINVAL;
		goto out;
	}
	if (cur_blk_size > buff_len ||
		cur_blk_size > sizeof(rsp->rsp_data)) {
		HIKP_ERROR_PRINT("The firmware data size (%" PRIu64 ") greater than "
				 "sw buffer(%zu) or hikp_hccs_rsp size(%zu).\n",
				 cur_blk_size, buff_len, sizeof(struct hikp_hccs_rsp));
		ret = -EINVAL;
		goto out;
	}

	memcpy(buff, rsp->rsp_data, rsp->rsp_head.cur_blk_size);
	rsp_head->total_blk_num = rsp->rsp_head.total_blk_num;
	rsp_head->cur_blk_size = rsp->rsp_head.cur_blk_size;

out:
	hikp_cmd_free(&cmd_ret);
	return ret;
}

static int hikp_hccs_query(struct hikp_cmd_header *req_header,
			   struct hikp_hccs_req *req_data,
			   void *buff, size_t buff_len)
{
	struct hikp_hccs_rsp_head rsp_head = {0};
	uint16_t total_blk_size = 0;
	uint16_t total_blk_num;
	uint16_t blk_id = 0;
	int ret;

	ret = hikp_hccs_cmd_send(req_header, req_data, buff, buff_len, &rsp_head);
	if (ret != 0)
		return ret;
	total_blk_num = rsp_head.total_blk_num;
	total_blk_size += rsp_head.cur_blk_size;
	for (blk_id = 1; blk_id < total_blk_num; blk_id++) {
		if (buff_len <= total_blk_size) {
			HIKP_ERROR_PRINT("No enough buffer to get block-(%u) context.\n",
					 blk_id);
			return -EINVAL;
		}

		req_data->head.blk_id = blk_id;
		ret = hikp_hccs_cmd_send(req_header, req_data,
					 (uint8_t *)buff + total_blk_size,
					 buff_len - total_blk_size, &rsp_head);
		if (ret != 0) {
			HIKP_ERROR_PRINT("Failed to get context for block-(%u)!\n",
					 blk_id);
			return -EINVAL;
		}
		total_blk_size += rsp_head.cur_blk_size;
	}

	return 0;
}

static int hikp_hccs_get_chip_num(struct hikp_plat_hccs_info *hccs_info)
{
	struct hccs_chip_num_rsp_data rsp_data = {0};
	struct hikp_cmd_header req_header = {0};
	struct hikp_hccs_req req = {0};
	int ret;

	hikp_cmd_init(&req_header, HCCS_MOD, HCCS_GET_CHIP_NUM, 0);
	ret = hikp_hccs_query(&req_header, &req,
			      &rsp_data, sizeof(struct hccs_chip_num_rsp_data));
	if (ret != 0) {
		HIKP_ERROR_PRINT("Failed to get chip number from firmware, ret = %d!\n", ret);
		return ret;
	}

	if (rsp_data.chip_num == 0) {
		HIKP_ERROR_PRINT("The chip number obtained from firmware is zero!\n");
		return  -EINVAL;
	}
	hccs_info->chip_num = rsp_data.chip_num;

	return 0;
}

static int hikp_hccs_get_die_num(uint8_t chip_id, struct hikp_plat_hccs_info *hccs_info)
{
	struct hccs_die_num_rsp_data rsp_data = {0};
	struct hikp_cmd_header req_header = {0};
	struct hccs_die_num_req_para *req_param;
	struct hikp_hccs_req req = {0};
	int ret;

	hikp_cmd_init(&req_header, HCCS_MOD, HCCS_GET_DIE_NUM, 0);
	req_param = (struct hccs_die_num_req_para *)&req.req_data;
	req_param->chip_id = chip_id;
	ret = hikp_hccs_query(&req_header, &req,
			      &rsp_data, sizeof(struct hccs_die_num_rsp_data));
	if (ret != 0) {
		HIKP_ERROR_PRINT("Failed to get die number from firmware, ret = %d!\n", ret);
		return ret;
	}

	hccs_info->chip_info[chip_id].die_num = rsp_data.die_num;
	return 0;
}

static int hikp_hccs_get_die_info(struct hccs_die_info *die_info,
			      uint8_t chip_id, uint8_t die_idx)
{
	struct hccs_die_info_rsp_data rsp_data = {0};
	struct hccs_die_info_req_para *req_param;
	struct hikp_cmd_header req_header = {0};
	struct hikp_hccs_req req = {0};
	int ret;

	hikp_cmd_init(&req_header, HCCS_MOD, HCCS_GET_DIE_INFO, 0);
	req_param = (struct hccs_die_info_req_para *)&req.req_data;
	req_param->chip_id = chip_id;
	req_param->die_idx = die_idx;
	ret = hikp_hccs_query(&req_header, &req,
			      &rsp_data, sizeof(struct hccs_die_info_rsp_data));
	if (ret != 0) {
		HIKP_ERROR_PRINT("Fail to get die information from firmware, ret = %d!\n", ret);
		return ret;
	}

	die_info->die_id = rsp_data.die_id;
	die_info->port_num = rsp_data.port_num;
	return 0;
}

static int hikp_hccs_get_all_die_info(struct hikp_plat_hccs_info *hccs_info)
{
	uint8_t chip_id, die_idx, die_num;
	struct hccs_chip_info *chip_info;
	struct hccs_die_info *die_info;
	bool has_die = false;
	int ret;

	for (chip_id = 0; chip_id < hccs_info->chip_num; chip_id++) {
		chip_info = &hccs_info->chip_info[chip_id];
		die_num = chip_info->die_num;
		if (die_num == 0)
			continue;

		has_die = true;
		die_info = (struct hccs_die_info *)calloc(die_num, sizeof(struct hccs_die_info));
		if (die_info == NULL) {
			HIKP_ERROR_PRINT("Failed to allocate memory for die information!\n");
			return -ENOMEM;
		}
		chip_info->die_info = die_info;
		for (die_idx = 0; die_idx < die_num; die_idx++, die_info++) {
			ret = hikp_hccs_get_die_info(die_info, chip_id, die_idx);
			if (ret < 0) {
				HIKP_ERROR_PRINT("Failed to get die information for die idx %u!\n",
						 die_idx);
				return ret;
			}
		}
	}

	return has_die ? 0 : -EINVAL;
}

static int hikp_hccs_get_ports_on_die(uint8_t *port_ids, uint16_t port_num,
				      uint8_t chip_id, uint8_t die_id)
{
	struct hccs_die_ports_req_para *req_param;
	struct hikp_cmd_header req_header = {0};
	struct hikp_hccs_req req = {0};
	int ret;

	hikp_cmd_init(&req_header, HCCS_MOD, HCCS_GET_PORT_IDS_ON_DIE, 0);
	req_param = (struct hccs_die_ports_req_para *)&req.req_data;
	req_param->chip_id = chip_id;
	req_param->die_id = die_id;
	ret = hikp_hccs_query(&req_header, &req,
			      port_ids, sizeof(uint8_t) * port_num);
	if (ret != 0) {
		HIKP_ERROR_PRINT("Fail to get port ids from firmware, ret = %d!\n", ret);
		return ret;
	}

	return 0;
}

static int hikp_hccs_get_all_port_info(struct hikp_plat_hccs_info *hccs_info)
{
	struct hccs_chip_info *chip_info;
	struct hccs_die_info *die_info;
	struct hccs_die_info *dies;
	uint8_t chip_id, die_idx;
	bool has_port = false;
	int ret;

	for (chip_id = 0; chip_id < hccs_info->chip_num; chip_id++) {
		chip_info = &hccs_info->chip_info[chip_id];
		dies = chip_info->die_info;
		has_port = false;
		for (die_idx = 0; die_idx < chip_info->die_num; die_idx++) {
			die_info = &dies[die_idx];
			if (die_info->port_num == 0)
				continue;

			has_port = true;
			die_info->port_ids = (uint8_t *)calloc(die_info->port_num,
							       sizeof(uint8_t));
			if (die_info->port_ids == NULL)
				return -ENOMEM;

			ret = hikp_hccs_get_ports_on_die(die_info->port_ids, die_info->port_num,
							 chip_id, die_info->die_id);
			if (ret < 0) {
				HIKP_ERROR_PRINT("Failed to get port ids for "
						 "chip (%u) die idx (%u)!\n",
						 chip_id, die_idx);
				return ret;
			}
		}
	}

	return has_port ? 0 : -EINVAL;
}

static int hikp_hccs_get_chip_info(struct hikp_plat_hccs_info *hccs_info)
{
	uint8_t chip_id;
	int ret;

	ret = hikp_hccs_get_chip_num(hccs_info);
	if (ret < 0) {
		HIKP_ERROR_PRINT("Failed to get chip num!\n");
		return ret;
	}

	hccs_info->chip_info = (struct hccs_chip_info *)calloc(hccs_info->chip_num,
							       sizeof(struct hccs_chip_info));
	if (hccs_info->chip_info == NULL) {
		HIKP_ERROR_PRINT("Failed to allocate memory for chip info!\n");
		return -ENOMEM;
	}

	for (chip_id = 0; chip_id < hccs_info->chip_num; chip_id++) {
		ret = hikp_hccs_get_die_num(chip_id, hccs_info);
		if (ret < 0) {
			HIKP_ERROR_PRINT("Failed to get die num!\n");
			return ret;
		}
	}

	return 0;
}

static int hikp_plat_hccs_hw_info(struct hikp_plat_hccs_info *hccs_info)
{
	int ret;

	ret = hikp_hccs_get_chip_info(hccs_info);
	if (ret != 0) {
		HIKP_ERROR_PRINT("Failed to get chip info, ret = %d!\n", ret);
		return ret;
	}

	ret = hikp_hccs_get_all_die_info(hccs_info);
	if (ret != 0) {
		HIKP_ERROR_PRINT("Failed to get all die info, ret = %d!\n", ret);
		return ret;
	}

	ret = hikp_hccs_get_all_port_info(hccs_info);
	if (ret != 0) {
		HIKP_ERROR_PRINT("Failed to get all port info, ret = %d!\n", ret);
		return ret;
	}
	return 0;
}

static int hikp_hccs_get_plat_topo(struct hccs_param *param,
				   union hccs_feature_info *info)
{
	int ret;

	HIKP_SET_USED(param);
	HIKP_SET_USED(info);

	ret = hikp_plat_hccs_hw_info(&g_hccs_info);
	if (ret < 0) {
		HIKP_ERROR_PRINT("Failed to get HCCS hardware info, ret = %d!\n", ret);
		return ret;
	}
	return 0;
}

static void hikp_plat_hccs_free(struct hikp_plat_hccs_info *hccs_info)
{
	struct hccs_chip_info *chip_info;
	struct hccs_die_info *die_info;
	uint8_t chip_id, die_idx;

	if (hccs_info->chip_info == NULL)
		return;

	for (chip_id = 0; chip_id < hccs_info->chip_num; chip_id++) {
		chip_info = &hccs_info->chip_info[chip_id];
		die_info = chip_info->die_info;
		if (die_info == NULL)
			continue;

		for (die_idx = 0; die_idx < chip_info->die_num; die_idx++) {
			if (die_info[die_idx].port_ids == NULL)
				continue;
			free(die_info[die_idx].port_ids);
			die_info[die_idx].port_ids = NULL;
		}
		free(die_info);
		chip_info->die_info = NULL;
	}
	free(hccs_info->chip_info);
	hccs_info->chip_info = NULL;
}

static bool hikp_hccs_die_id_valid(struct hikp_plat_hccs_info *hccs_info,
				   struct hccs_param *param, uint8_t *die_idx)
{
	struct hccs_chip_info *chip_info;
	struct hccs_die_info *die_info;
	uint8_t idx;

	chip_info = &hccs_info->chip_info[param->chip_id];
	die_info = chip_info->die_info;
	for (idx = 0; idx < chip_info->die_num; idx++) {
		if (param->die_id != die_info[idx].die_id)
			continue;
		*die_idx = idx;
		return true;
	}
	return false;
}

static bool hikp_hccs_port_id_valid(struct hikp_plat_hccs_info *hccs_info,
				    struct hccs_param *param, uint8_t die_idx)
{
	struct hccs_chip_info *chip_info;
	struct hccs_die_info *die_info;
	uint8_t port_id, *port_ids;

	chip_info = &hccs_info->chip_info[param->chip_id];
	die_info = &chip_info->die_info[die_idx];
	port_ids = die_info->port_ids;

	for (port_id = 0; port_id < die_info->port_num; port_id++) {
		if (param->port_id == port_ids[port_id])
			return true;
	}

	return false;
}

static bool hikp_hccs_req_param_check(struct hikp_plat_hccs_info *hccs_info,
				      struct hccs_param *param)
{
	uint8_t die_idx;

	if (param->chip_id >= hccs_info->chip_num) {
		HIKP_ERROR_PRINT("param error: chip id %u exceed chip number %u!\n",
				 param->chip_id, hccs_info->chip_num);
		return false;
	}

	if (!hikp_hccs_die_id_valid(hccs_info, param, &die_idx)) {
		HIKP_ERROR_PRINT("Param error: die%u not found on chip%u!\n",
				 param->die_id, param->chip_id);
		return false;
	}

	if (!hikp_hccs_port_id_valid(hccs_info, param, die_idx)) {
		HIKP_ERROR_PRINT("Param error: port id %u not found on chip%u die%u!\n",
				 param->port_id, param->chip_id, param->die_id);
		return false;
	}

	return true;
}

static int hikp_hccs_get_port_attr(struct hccs_param *param,
				   union hccs_feature_info *info)
{
	struct hccs_port_attr_req_para *req_para;
	struct hikp_cmd_header req_header = {0};
	struct hikp_hccs_req req = {0};
	int ret;

	ret = hikp_plat_hccs_hw_info(&g_hccs_info);
	if (ret != 0) {
		HIKP_ERROR_PRINT("Failed to get HCCS hardware info for "
				 "port attributes, ret = %d.\n", ret);
		return ret;
	}

	if (!hikp_hccs_req_param_check(&g_hccs_info, param))
		return -EINVAL;

	hikp_cmd_init(&req_header, HCCS_MOD, HCCS_GET_PORT_FIXED_ATTR, 0);
	req_para = (struct hccs_port_attr_req_para *)&req.req_data;
	req_para->chip_id = param->chip_id;
	req_para->die_id = param->die_id;
	req_para->port_id = param->port_id;
	ret = hikp_hccs_query(&req_header, &req,
			      info, sizeof(union hccs_feature_info));
	if (ret != 0)
		return ret;
	return 0;
}

static int hikp_hccs_get_port_dfx_info(struct hccs_param *param,
				       union hccs_feature_info *info)
{
	struct hikp_hccs_rsp_head rsp_head = {0};
	struct hikp_cmd_header req_header = {0};
	struct hccs_port_dfx_info_vld *dfx_info;
	struct hccs_port_dfx_req_para *dfx_req;
	struct hikp_hccs_req req = {0};
	int ret;

	ret = hikp_plat_hccs_hw_info(&g_hccs_info);
	if (ret != 0) {
		HIKP_ERROR_PRINT("Failed to get HCCS hardware info for dfx info, ret = %d\n",
				 ret);
		return ret;
	}

	if (!hikp_hccs_req_param_check(&g_hccs_info, param))
		return -EINVAL;

	dfx_info = &info->dfx_info;
	dfx_req = (struct hccs_port_dfx_req_para *)&req.req_data;
	dfx_req->chip_id = param->chip_id;
	dfx_req->port_id = param->port_id;
	dfx_req->die_id = param->die_id;
	hikp_cmd_init(&req_header, HCCS_MOD, HCCS_GET_PORT_DFX_INFO, 0);
	ret = hikp_hccs_cmd_send(&req_header, &req,
				 &dfx_info->info,
				 sizeof(struct hccs_port_dfx_info),
				 &rsp_head);
	if (ret != 0)
		return ret;

	dfx_info->vld_size = rsp_head.cur_blk_size;

	return 0;
}

static void hikp_hccs_show_topo(union hccs_feature_info *data)
{
	uint8_t chip_id, die_idx, die_num, port_idx, *port_ids;
	struct hccs_die_info *die_info;
	struct hccs_die_info *dies;

	HIKP_SET_USED(data);

	for (chip_id = 0; chip_id < g_hccs_info.chip_num; chip_id++) {
		die_num = g_hccs_info.chip_info[chip_id].die_num;
		dies = g_hccs_info.chip_info[chip_id].die_info;
		printf("--chip%u\n", chip_id);
		if (die_num == 0)
			continue;
		for (die_idx = 0; die_idx < die_num; die_idx++) {
			die_info = &dies[die_idx];
			printf("\t--die%u\n", die_info->die_id);
			port_ids = die_info->port_ids;
			if (die_info->port_num == 0)
				continue;

			for (port_idx = 0; port_idx < die_info->port_num; port_idx++)
				printf("\t\t--hccs%u\n", port_ids[port_idx]);
		}
	}
}

static void hikp_hccs_show_port_attr(union hccs_feature_info *feature_info)
{
	struct hccs_port_fixed_attr *info = &feature_info->attr;

	printf("%-16s\tHCCS-V%u\n"
	       "%-16s\tx%u\n"
	       "%-16s\t%uMbps\n"
	       "%-16s\t%u\n",
	       "hccs_type", info->hccs_type,
	       "lane_mode", info->lane_mode,
	       "speed", info->speed,
	       "enabled", info->enabled);
}

static const char *hikp_hccs_link_fsm_to_str(uint8_t link_fsm)
{
	size_t i;

	for (i = 0; i < sizeof(link_fsm_map) / sizeof(link_fsm_map[0]); i++) {
		if (link_fsm_map[i].link_fsm == link_fsm)
			return link_fsm_map[i].str;
	}

	return "unknown";
}

static void hikp_hccs_show_port_dfx_info(union hccs_feature_info *feature_info)
{
	struct hccs_port_dfx_info_vld *info_vld = &feature_info->dfx_info;
	struct hccs_port_dfx_info *info = &info_vld->info;
	size_t vld_size;

	vld_size = (size_t)info_vld->vld_size;
	if (vld_size > offsetof(struct hccs_port_dfx_info, link_fsm)) {
		printf("%-16s\t%s\n", "link_fsm", hikp_hccs_link_fsm_to_str(info->link_fsm));
	}

	if (vld_size > offsetof(struct hccs_port_dfx_info, cur_lane_num)) {
		printf("%-16s\t%u\n", "cur_lane_num", info->cur_lane_num);
	}

	if (vld_size > offsetof(struct hccs_port_dfx_info, lane_mask)) {
		printf("%-16s\t0x%x\n", "lane_mask", info->lane_mask);
	}

	if (vld_size > offsetof(struct hccs_port_dfx_info, crc_err_cnt)) {
		printf("%-16s\t%u\n", "crc_err_cnt", info->crc_err_cnt);
	}

	if (vld_size > offsetof(struct hccs_port_dfx_info, retry_cnt)) {
		printf("%-16s\t%u\n", "retry_cnt", info->retry_cnt);
	}

	if (vld_size > offsetof(struct hccs_port_dfx_info, phy_reinit_cnt)) {
		printf("%-16s\t%u\n", "phy_reinit_cnt", info->phy_reinit_cnt);
	}

	if (vld_size > offsetof(struct hccs_port_dfx_info, tx_credit)) {
		printf("%-16s\t%u\n", "tx_credit", info->tx_credit);
	}
}

static void hikp_hccs_cmd_execute(struct major_cmd_ctrl *self)
{
	const struct hikp_hccs_feature_cmd *hccs_cmd;
	union hccs_feature_info info = {0};
	int ret;

	if (g_hccs_param.feature_idx == -1) {
		hikp_hccs_cmd_help(self, NULL);
		snprintf(self->err_str, sizeof(self->err_str), "-g/--get param error!");
		self->err_no = -EINVAL;
		return;
	}

	hccs_cmd = &g_hccs_feature_cmd[g_hccs_param.feature_idx];
	if (g_hccs_param.param_mask != hccs_cmd->param_needed) {
		hikp_hccs_cmd_help(self, NULL);
		snprintf(self->err_str, sizeof(self->err_str), "Parameter mismatched!");
		self->err_no = -EINVAL;
		return;
	}

	ret = hikp_hccs_get_chip_num(&g_hccs_info);
	if (ret < 0) {
		self->err_no = ret;
		return;
	}

	if (g_hccs_info.chip_num == 1) {
		snprintf(self->err_str, sizeof(self->err_str),
			 "The command is just supported on multi-sockets!\n");
		self->err_no = -EINVAL;
		return;
	}

	ret = hccs_cmd->query(&g_hccs_param, &info);
	if (ret != 0) {
		snprintf(self->err_str, sizeof(self->err_str), "Failed to query %s, ret = %d.",
			 hccs_cmd->feature_name, ret);
		self->err_no = ret;
		hikp_plat_hccs_free(&g_hccs_info);
		return;
	}

	printf("############## HCCS: %s info ############\n", hccs_cmd->feature_name);
	hccs_cmd->show(&info);
	printf("#################### END #######################\n");

	hikp_plat_hccs_free(&g_hccs_info);
}

static int hikp_hccs_cmd_feature_select(struct major_cmd_ctrl *self, const char *argv)
{
	size_t feat_size = HIKP_ARRAY_SIZE(g_hccs_feature_cmd);
	size_t i;

	for (i = 0; i < feat_size; i++) {
		if (strcmp(argv, g_hccs_feature_cmd[i].feature_name) == 0) {
			g_hccs_param.feature_idx = i;
			return 0;
		}
	}

	hikp_hccs_cmd_help(self, NULL);
	snprintf(self->err_str, sizeof(self->err_str), "-g/--get param error!!!");
	self->err_no = -EINVAL;

	return self->err_no;
}

static int hikp_hccs_cmd_parse_chip(struct major_cmd_ctrl *self, const char *argv)
{
	uint32_t chip_id;

	self->err_no = string_toui(argv, &chip_id);
	if (self->err_no) {
		snprintf(self->err_str, sizeof(self->err_str),
			 "Failed to parse -c/--chip_id parameter.");
		return self->err_no;
	}
	if (chip_id > UINT8_MAX) {
		snprintf(self->err_str, sizeof(self->err_str),
			 "chip id should not be greater than %u.", UINT8_MAX);
		self->err_no = -EINVAL;
		return self->err_no;
	}

	g_hccs_param.param_mask |= HCCS_ENABLE_CHIP_ID;
	g_hccs_param.chip_id = (uint8_t)chip_id;

	return 0;
}

static int hikp_hccs_cmd_parse_die(struct major_cmd_ctrl *self, const char *argv)
{
	uint32_t die_id;

	self->err_no = string_toui(argv, &die_id);
	if (self->err_no) {
		snprintf(self->err_str, sizeof(self->err_str),
			 "Failed to parse -d/--die_id parameter.");
		return self->err_no;
	}

	if (die_id > UINT8_MAX) {
		snprintf(self->err_str, sizeof(self->err_str),
			 "die id should not be greater than %u.", UINT8_MAX);
		self->err_no = -EINVAL;
		return self->err_no;
	}

	g_hccs_param.param_mask |= HCCS_ENABLE_DIE_ID;
	g_hccs_param.die_id = (uint8_t)die_id;

	return 0;
}

static int hikp_hccs_cmd_parse_port(struct major_cmd_ctrl *self, const char *argv)
{
	uint32_t port_id;

	self->err_no = string_toui(argv, &port_id);
	if (self->err_no) {
		snprintf(self->err_str, sizeof(self->err_str),
			 "Failed to parse -p/--port_id parameter.");
		return self->err_no;
	}

	if (port_id > UINT8_MAX) {
		snprintf(self->err_str, sizeof(self->err_str),
			 "port id should not be greater than %u.", UINT8_MAX);
		self->err_no = -EINVAL;
		return self->err_no;
	}

	g_hccs_param.param_mask |= HCCS_ENABLE_PORT_ID;
	g_hccs_param.port_id = (uint8_t)port_id;
	return 0;
}

static void hikp_hccs_cmd_init(void)
{
	struct major_cmd_ctrl *major_cmd = get_major_cmd();

	g_hccs_param.feature_idx = -1;

	major_cmd->option_count = 0;
	major_cmd->execute = hikp_hccs_cmd_execute;

	cmd_option_register("-h", "--help", false, hikp_hccs_cmd_help);
	cmd_option_register("-g", "--get", true, hikp_hccs_cmd_feature_select);
	cmd_option_register("-c", "--chip_id", true, hikp_hccs_cmd_parse_chip);
	cmd_option_register("-d", "--die_id", true, hikp_hccs_cmd_parse_die);
	cmd_option_register("-p", "--port_id", true, hikp_hccs_cmd_parse_port);
}

HIKP_CMD_DECLARE("hccs", "dump HCCS information.", hikp_hccs_cmd_init);
