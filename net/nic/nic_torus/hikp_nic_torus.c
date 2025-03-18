/*
 * Copyright (c) 2023 Hisilicon Technologies Co., Ltd.
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

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "hikp_nic_torus.h"

static struct tool_target g_torus_target;

static int hikp_nic_torus_query(const struct bdf_t *bdf,
				struct nic_torus_info *info)
{
	struct nic_torus_req_para req = { 0 };
	struct hikp_cmd_header header = { 0 };
	struct hikp_cmd_ret *cmd_ret;
	struct nic_torus_rsp *rsp;
	int ret = 0;

	req.bdf = *bdf;
	hikp_cmd_init(&header, NIC_MOD, GET_TORUS_INFO_CMD, NIC_TORUS_INFO_DUMP);
	cmd_ret = hikp_cmd_alloc(&header, &req, sizeof(req));
	if (cmd_ret == NULL || cmd_ret->status != 0) {
		ret = cmd_ret ? (int)(-cmd_ret->status) : -EIO;
		HIKP_ERROR_PRINT("fail to get torus info, retcode: %d\n", ret);
		hikp_cmd_free(&cmd_ret);
		return ret;
	}

	rsp = (struct nic_torus_rsp *)cmd_ret->rsp_data;
	*info = *(struct nic_torus_info *)rsp->data;
	hikp_cmd_free(&cmd_ret);

	return 0;
}

static void hikp_nic_torus_lan_prt_pair_show(const struct nic_torus_info *info)
{
	uint32_t mac_id = hikp_get_field(info->lan_prt_pair, NIC_TORUS_MAC_ID_M,
					 NIC_TORUS_MAC_ID_S);
	uint32_t uc_lan_pair_en = hikp_get_bit(info->lan_prt_pair,
					       NIC_TORUS_UC_LAN_PAIR_EN);
	uint32_t mc_bc_lan_pair_en = hikp_get_bit(info->lan_prt_pair,
						  NIC_TORUS_MC_BC_LAN_PAIR_EN);
	uint32_t lldp_lan_pair_en = hikp_get_bit(info->lan_prt_pair,
						 NIC_TORUS_LLDP_LAN_PAIR_EN);
	uint32_t tc2vlanpri_mapping_en = hikp_get_bit(info->lan_prt_pair,
						      NIC_TORUS_TC2VLANPRI_MAPPING_EN);
	uint32_t torus_lpbk_drop_en = hikp_get_bit(info->lan_prt_pair,
						   NIC_TORUS_LPBK_DROP_EN);

	printf("dst mac id: %u\n", mac_id);
	printf("uc_lan_pair_en: %u\n", uc_lan_pair_en);
	printf("mc_bc_lan_pair_en: %u\n", mc_bc_lan_pair_en);
	printf("lldp_lan_pair_en: %u\n", lldp_lan_pair_en);
	printf("tc2vlanpri_mapping_en: %u\n", tc2vlanpri_mapping_en);
	printf("torus_lpbk_drop_en: %u\n", torus_lpbk_drop_en);
}

static void hikp_nic_torus_lan_fwd_tc_cfg_show(const struct nic_torus_info *info)
{
	uint32_t tc0_map_tc = hikp_get_field(info->lan_fwd_tc_cfg,
					     NIC_TORUS_TC0_MAP_TC_M,
					     NIC_TORUS_TC0_MAP_TC_S);
	uint32_t tc1_map_tc = hikp_get_field(info->lan_fwd_tc_cfg,
					     NIC_TORUS_TC1_MAP_TC_M,
					     NIC_TORUS_TC1_MAP_TC_S);
	uint32_t tc2_map_tc = hikp_get_field(info->lan_fwd_tc_cfg,
					     NIC_TORUS_TC2_MAP_TC_M,
					     NIC_TORUS_TC2_MAP_TC_S);
	uint32_t tc3_map_tc = hikp_get_field(info->lan_fwd_tc_cfg,
					     NIC_TORUS_TC3_MAP_TC_M,
					     NIC_TORUS_TC3_MAP_TC_S);

	printf("tc0_map_tc : %u\n", tc0_map_tc);
	printf("tc1_map_tc : %u\n", tc1_map_tc);
	printf("tc2_map_tc : %u\n", tc2_map_tc);
	printf("tc3_map_tc : %u\n", tc3_map_tc);
}

static void hikp_nic_torus_switch_param_show(const struct nic_torus_info *info)
{
	printf("nic_mac_anti_spoof_en: %s\n", info->nic_switch_param &
		NIC_TORUS_MAC_ANTI_SPOOF_EN_MASK ? "enable" : "disable");
	printf("nic_alw_lpbk: %s\n", info->nic_switch_param &
		NIC_TORUS_ALW_LPBK_MASK ? "enable" : "disable");
	printf("nic_alw_lcl_lpbk: %s\n", info->nic_switch_param &
		NIC_TORUS_ALW_LCL_LPBK_MASK ? "enable" : "disable");
	printf("nic_alw_dst_ovrd: %s\n", info->nic_switch_param &
		NIC_TORUS_ALW_DST_OVRD_MASK ? "enable" : "disable");
	printf("roce_mac_anti_spoof_en: %s\n", info->roce_switch_param &
		NIC_TORUS_MAC_ANTI_SPOOF_EN_MASK ? "enable" : "disable");
	printf("roce_alw_lpbk: %s\n", info->roce_switch_param &
		NIC_TORUS_ALW_LPBK_MASK ? "enable" : "disable");
	printf("roce_alw_lcl_lpbk: %s\n", info->roce_switch_param &
		NIC_TORUS_ALW_LCL_LPBK_MASK ? "enable" : "disable");
	printf("roce_alw_dst_ovrd: %s\n", info->roce_switch_param &
		NIC_TORUS_ALW_DST_OVRD_MASK ? "enable" : "disable");
}

static void hikp_nic_torus_show(const struct nic_torus_info *info)
{
	printf("################ NIC TORUS info ##################\n");
	printf("enable: %s\n", info->enable ? "true" : "false");

	hikp_nic_torus_lan_prt_pair_show(info);

	hikp_nic_torus_lan_fwd_tc_cfg_show(info);

	printf("ssu_pause_time_out: %uus\n", info->pause_time_out);
	printf("ssu_pause_time_out_en: %u\n", info->pause_time_out_en);
	printf("vlan_fe: 0x%x (for port vlan)\n", info->vlan_fe);
	printf("ets_tcg0_mapping: 0x%x\n", info->ets_tcg0_mapping);
	printf("ets_tcg0_mapping is showed as 0xff if ncl_config forward bit is setted to 1\n");

	hikp_nic_torus_switch_param_show(info);

	printf("#################### END ##########################\n");
}

void hikp_nic_torus_cmd_execute(struct major_cmd_ctrl *self)
{
	struct bdf_t *bdf = &g_torus_target.bdf;
	struct nic_torus_info info = { 0 };
	int ret;

	ret = hikp_nic_torus_query(bdf, &info);
	if (ret != 0) {
		snprintf(self->err_str, sizeof(self->err_str), "fail to obtain torus info.");
		self->err_no = ret;
		return;
	}

	hikp_nic_torus_show(&info);
}

static int hikp_nic_torus_cmd_help(struct major_cmd_ctrl *self, const char *argv)
{
	HIKP_SET_USED(argv);

	printf("\n  Usage: %s %s\n", self->cmd_ptr->name, "-i <device>");
	printf("\n         %s\n", self->cmd_ptr->help_info);
	printf("\n  Options:\n\n");
	printf("    %s, %-25s %s\n", "-h", "--help", "display this help and exit");
	printf("    %s, %-25s %s\n", "-i", "--interface=<interface>",
	       "device target or bdf id, e.g. eth0~7 or 0000:35:00.0");

	return 0;
}

int hikp_nic_torus_get_target(struct major_cmd_ctrl *self, const char *argv)
{
	self->err_no = tool_check_and_get_valid_bdf_id(argv, &g_torus_target);
	if (self->err_no != 0) {
		snprintf(self->err_str, sizeof(self->err_str), "unknown device!");
		return self->err_no;
	}

	if (g_torus_target.bdf.dev_id != 0) {
		snprintf(self->err_str, sizeof(self->err_str), "VF does not support query!");
		self->err_no = -EOPNOTSUPP;
		return self->err_no;
	}

	return 0;
}

static void cmd_nic_torus_init(void)
{
	struct major_cmd_ctrl *major_cmd = get_major_cmd();

	major_cmd->option_count = 0;
	major_cmd->execute = hikp_nic_torus_cmd_execute;

	cmd_option_register("-h", "--help", false, hikp_nic_torus_cmd_help);
	cmd_option_register("-i", "--interface", true, hikp_nic_torus_get_target);
}

HIKP_CMD_DECLARE("nic_torus", "dump torus info of nic!", cmd_nic_torus_init);
