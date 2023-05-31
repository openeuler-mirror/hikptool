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

#include "hikp_roh_mac.h"
#include "hikp_roh_cmd.h"

static struct roh_mac_param g_roh_mac_param = { 0 };

static int hikp_roh_mac_help(struct major_cmd_ctrl *self, const char *argv)
{
	printf("\n  Usage: %s %s\n", self->cmd_ptr->name,
	       "-i <interface> -s <mac_type|cam|credit>\n");
	printf("\n         %s\n", self->cmd_ptr->help_info);
	printf("  Options:\n\n");
	printf("    %s, %-25s %s\n", "-h", "--help", "display this help and exit");
	printf("    %s, %-25s %s\n", "-i", "--interface=<interface>", "device target, e.g. eth0");
	printf("    %s, %-25s %s\n", "-s", "--show=<cam|mac_type|credit>",
		   "show cam table mapping or mac_type or credit_cnt");
	printf("\n");
	return 0;
}

static int hikp_roh_mac_target(struct major_cmd_ctrl *self, const char *argv)
{
	self->err_no = tool_check_and_get_valid_bdf_id(argv, &(g_roh_mac_param.target));
	if (self->err_no != 0)
		snprintf(self->err_str, sizeof(self->err_str), "Unknown device %s.", argv);

	return self->err_no;
}

static int cmd_show_mac_type_parse(void)
{
	g_roh_mac_param.flag |= CMD_SHOW_MAC_TYPE_FLAG;
	return 0;
}

static int cmd_show_cam_parse(void)
{
	g_roh_mac_param.flag |= CMD_SHOW_CAM_FLAG;
	return 0;
}

static int cmd_show_credit_parse(void)
{
	g_roh_mac_param.flag |= CMD_SHOW_CREDIT_CNT;
	return 0;
}

static int hikp_roh_mac_show_parse(struct major_cmd_ctrl *self, const char *argv)
{
	int ret;

	if (strncmp(argv, "cam", sizeof("cam")) == 0) {
		ret = cmd_show_cam_parse();
	} else if (strncmp(argv, "mac_type", sizeof("mac_type")) == 0) {
		ret = cmd_show_mac_type_parse();
	} else if (strncmp(argv, "credit", sizeof("credit")) == 0) {
		ret = cmd_show_credit_parse();
	} else {
		hikp_roh_mac_help(self, NULL);
		snprintf(self->err_str, sizeof(self->err_str),
			 "-s/--show param should be cam/mac_type/credit");
		self->err_no = -EINVAL;
		return -EINVAL;
	}
	return ret;
}

int hikp_roh_get_mac_type(struct major_cmd_ctrl *self, struct bdf_t bdf)
{
	struct roh_mac_req_para req_data = { 0 };
	struct roh_mac_get_type *mac_rsp = NULL;
	struct hikp_cmd_header req_header = { 0 };
	struct hikp_cmd_ret *cmd_ret = NULL;
	uint8_t is_roh;

	hikp_cmd_init(&req_header, ROH_MOD, HIKP_ROH_MAC, CMD_SHOW_MAC_TYPE);
	req_data.bdf = bdf;
	cmd_ret = hikp_cmd_alloc(&req_header, &req_data, sizeof(req_data));
	if (cmd_ret == NULL || cmd_ret->status != 0) {
		HIKP_ERROR_PRINT("failed to get roh info, retcode: %u\n",
				 cmd_ret ? cmd_ret->status : EIO);
		self->err_no = -EIO;
		free(cmd_ret);
		return -EIO;
	}
	mac_rsp = (struct roh_mac_get_type *)(cmd_ret->rsp_data);
	is_roh = mac_rsp->mac_type;
	free(cmd_ret);
	cmd_ret = NULL;
	return is_roh;
}

static void hikp_roh_show_mac_type(struct major_cmd_ctrl *self, int mac_type)
{
	if (mac_type)
		printf("MAC_TYPE: ROH\n");
	else
		printf("MAC_TYPE: ETH\n");
}

static void hikp_roh_print_cam(unsigned int cam_convert_enable, unsigned int convert_enable,
			       const struct cam_table_entry_t *cam_table)
{
	printf("**************CAM TABLE INFO*************\n");
	if (cam_convert_enable & DMAC_CONVERT_ENABLE_MASK)
		printf("CAM TABLE DMAC convert : Enable\n");
	else
		printf("CAM TABLE DMAC convert : Disable\n");

	if (cam_convert_enable & SMAC_CONVERT_ENABLE_MASK)
		printf("CAM TABLE SMAC convert : Enable\n");
	else
		printf("CAM TABLE SMAC convert : Disable\n");

	printf("%3s%8s%16s%8s\n", "ID", "EID", "MAC", "Status");
	for (int camid = 0; camid < MAX_CAM_SIZE; camid++) {
		printf("%3d  %06x    %012lx", camid, cam_table[camid].eid, cam_table[camid].mac);
		if (convert_enable & (1UL << camid))
			printf("%8s\n", "Enable");
		else
			printf("%8s\n", "Disable");
	}
	printf("*****************************************\n");
}

static int hikp_roh_get_cam_reg_num(struct major_cmd_ctrl *self)
{
	struct roh_mac_cam_reg_num *mac_rsp = NULL;
	struct roh_mac_req_para req_data = { 0 };
	struct hikp_cmd_header req_header = { 0 };
	struct hikp_cmd_ret *cmd_ret = NULL;
	uint32_t cam_reg_num;

	req_data.bdf = g_roh_mac_param.target.bdf;
	hikp_cmd_init(&req_header, ROH_MOD, HIKP_ROH_MAC, CMD_GET_CAM_REG_NUM);
	cmd_ret = hikp_cmd_alloc(&req_header, &req_data, sizeof(req_data));
	if (cmd_ret == NULL || cmd_ret->status != 0) {
		HIKP_ERROR_PRINT("fail to get cam reg num, retcode: %u\n",
				 cmd_ret ? cmd_ret->status : EIO);
		self->err_no = -EIO;
		free(cmd_ret);
		return -EIO;
	}
	mac_rsp = (struct roh_mac_cam_reg_num *)(cmd_ret->rsp_data);
	cam_reg_num = mac_rsp->cam_reg_num;
	free(cmd_ret);
	cmd_ret = NULL;
	return cam_reg_num;
}

static int hikp_roh_build_cam(struct major_cmd_ctrl *self, struct cam_table_entry_t *cam_table)
{
	struct roh_mac_req_para req_data = { 0 };
	struct roh_mac_cam_table *mac_rsp = NULL;
	struct hikp_cmd_header req_header = { 0 };
	struct hikp_cmd_ret *cmd_ret = NULL;
	int block_num;
	int reg_num;
	int addtion;
	int index;

	reg_num = hikp_roh_get_cam_reg_num(self);
	if (reg_num < 0)
		return -EIO;

	addtion = reg_num % RESPONSE_DATA_NUMBER_MAX ? 1 : 0;
	block_num = reg_num / RESPONSE_DATA_NUMBER_MAX + addtion;

	for (int i = 0; i < block_num; i++) {
		req_data.bdf = g_roh_mac_param.target.bdf;
		req_data.cam_block_index = i;
		hikp_cmd_init(&req_header, ROH_MOD, HIKP_ROH_MAC, CMD_BUILD_CAM_TABLE);
		cmd_ret = hikp_cmd_alloc(&req_header, &req_data, sizeof(req_data));
		if (cmd_ret == NULL || cmd_ret->status != 0) {
			HIKP_ERROR_PRINT("fail to get cam table info, retcode: %u\n",
					 cmd_ret ? cmd_ret->status : EIO);
			self->err_no = -EIO;
			free(cmd_ret);
			return -EIO;
		}
		mac_rsp = (struct roh_mac_cam_table *)(cmd_ret->rsp_data);
		for (int j = 0; j < BLOCK_SIZE; j++) {
			index = i * BLOCK_SIZE + j;
			if (index >= MAX_CAM_SIZE)
				break;

			cam_table[index].eid = (mac_rsp->cam_eid[j]) & 0xffffff;
			cam_table[index].mac = 0xffffffffUL &
					       (unsigned long)(mac_rsp->cam_mac_low32[j]);
			cam_table[index].mac |= ((0xffffUL &
						(unsigned long)(mac_rsp->cam_mac_high16[j])) <<
						ROH_MAC_CAM_OFFSET);
		}
		free(cmd_ret);
		cmd_ret = NULL;
	}
	return 0;
}

static void hikp_roh_show_cam(struct major_cmd_ctrl *self)
{
	struct cam_table_entry_t cam_table[MAX_CAM_SIZE] = { 0 };
	struct roh_mac_req_para req_data = { 0 };
	struct roh_mac_cam_caps *mac_rsp = NULL;
	unsigned int cam_convert_enable;
	struct hikp_cmd_header req_header = { 0 };
	struct hikp_cmd_ret *cmd_ret = NULL;
	unsigned int convert_enable;
	int ret;

	hikp_cmd_init(&req_header, ROH_MOD, HIKP_ROH_MAC, CMD_SHOW_CAM);
	req_data.bdf = g_roh_mac_param.target.bdf;
	cmd_ret = hikp_cmd_alloc(&req_header, &req_data, sizeof(req_data));
	if (cmd_ret == NULL || cmd_ret->status != 0) {
		HIKP_ERROR_PRINT("fail to get cam info, retcode: %u\n",
				 cmd_ret ? cmd_ret->status : EIO);
		self->err_no = -EIO;
		free(cmd_ret);
		return;
	}
	mac_rsp = (struct roh_mac_cam_caps *)cmd_ret->rsp_data;
	convert_enable = mac_rsp->convert_enable;
	cam_convert_enable = mac_rsp->cam_convert_enable;
	free(cmd_ret);
	cmd_ret = NULL;

	ret = hikp_roh_build_cam(self, cam_table);
	if (ret != 0)
		return;

	hikp_roh_print_cam(cam_convert_enable, convert_enable, cam_table);
}

static int hikp_roh_query_crd(uint8_t crd_type, uint32_t num_rows, char const *crds[][2])
{
	struct roh_mac_credit_data *mac_rsp = NULL;
	struct roh_mac_req_para req_data = { 0 };
	struct hikp_cmd_header req_header = { 0 };
	struct hikp_cmd_ret *cmd_ret = NULL;
	int ret;

	hikp_cmd_init(&req_header, ROH_MOD, HIKP_ROH_MAC, CMD_SHOW_CREDIT);
	req_data.bdf = g_roh_mac_param.target.bdf;
	req_data.crd_type = crd_type;
	cmd_ret = hikp_cmd_alloc(&req_header, &req_data, sizeof(req_data));
	ret = hikp_rsp_normal_check(cmd_ret);
	if (ret != 0) {
		free(cmd_ret);
		cmd_ret = NULL;
		return ret;
	}
	mac_rsp = (struct roh_mac_credit_data *)(cmd_ret->rsp_data);
	for (int i = 0; i < num_rows; i++) {
		union cut_reg reg;

		reg.value = (mac_rsp->cut_reg_value)[i];
		if ((strcmp(crds[i][0], "NULL") != 0) && (reg.cut[0] != 0))
			printf("%-28s : %#x\n", crds[i][0], (reg.cut)[0]);

		if ((strcmp(crds[i][1], "NULL") != 0) && (reg.cut)[1] != 0)
			printf("%-28s : %#x\n", crds[i][1], (reg.cut)[1]);
	}
	free(cmd_ret);
	cmd_ret = NULL;
	return 0;
}

static int hikp_roh_print_holding_crd(void)
{
	int ret;

	char const *holding_crds[][2] = {
		{"MAC0_TX_ICRD_VNA", "NULL"}, {"MAC0_TX_ICRD_VN_TC0", "MAC0_TX_ICRD_VN_TC1"},
		{"MAC0_TX_ICRD_VN_TC2", "MAC0_TX_ICRD_VN_TC3"},
		{"MAC0_TX_ICRD_VN_TC4", "MAC0_TX_ICRD_VN_TC5"},
		{"MAC0_TX_ICRD_VN_TC6", "MAC0_TX_ICRD_VN_TC7"}, {"MAC2_TX_ICRD_VNA", "NULL"},
		{"MAC2_TX_ICRD_VN_TC0", "MAC2_TX_ICRD_VN_TC1"},
		{"MAC2_TX_ICRD_VN_TC2", "MAC2_TX_ICRD_VN_TC3"},
		{"MAC2_TX_ICRD_VN_TC4", "MAC2_TX_ICRD_VN_TC5"},
		{"MAC2_TX_ICRD_VN_TC6", "MAC2_TX_ICRD_VN_TC7"} };

	ret = hikp_roh_query_crd(HOLDING_CRD, NUM_ROWS_HOLDING_CRDS, holding_crds);
	return ret;
}

static int hikp_roh_print_init_crd(void)
{
	int ret;

	char const *init_crds[][2] = {
		{"MAC0_RX_CFG_REMOTE_ICRD", "NULL"},
		{"MAC0_RX_CFG_REMOTE_CRD_VL6", "MAC0_RX_CFG_REMOTE_CRD_VL7"},
		{"MAC0_RX_CFG_REMOTE_CRD_VL4", "MAC0_RX_CFG_REMOTE_CRD_VL5"},
		{"MAC0_RX_CFG_REMOTE_CRD_VL2", "MAC0_RX_CFG_REMOTE_CRD_VL3"},
		{"MAC0_RX_CFG_REMOTE_CRD_VL0", "MAC0_RX_CFG_REMOTE_CRD_VL1"},
		{"MAC0_RX_CFG_REMOTE_CRD_VL8", "NULL"}, {"MAC2_RX_CFG_REMOTE_ICRD", "NULL"},
		{"MAC2_RX_CFG_REMOTE_CRD_VL6", "MAC2_RX_CFG_REMOTE_CRD_VL7"},
		{"MAC2_RX_CFG_REMOTE_CRD_VL4", "MAC2_RX_CFG_REMOTE_CRD_VL5"},
		{"MAC2_RX_CFG_REMOTE_CRD_VL2", "MAC2_RX_CFG_REMOTE_CRD_VL3"},
		{"MAC2_RX_CFG_REMOTE_CRD_VL0", "MAC2_RX_CFG_REMOTE_CRD_VL1"},
		{"MAC2_RX_CFG_REMOTE_CRD_VL8", "NULL"} };

	ret = hikp_roh_query_crd(INIT_CRD, NUM_ROWS_INIT_CRDS, init_crds);
	return ret;
}

static int hikp_roh_print_temp_crd(void)
{
	int ret;

	char const *temp_crds[][2] = {
		{"MAC0_TX_LCRD_VNA_EXIST_NUM", "NULL"}, {"MAC0_TX_ICRD_VNA_EXIST_NUM", "NULL"},
		{"MAC0_TX_CRD_VN0_EXIST_NUM", "NULL"}, {"MAC0_TX_CRD_VN1_EXIST_NUM", "NULL"},
		{"MAC0_TX_CRD_VN2_EXIST_NUM", "NULL"}, {"MAC0_TX_CRD_VN3_EXIST_NUM", "NULL"},
		{"MAC0_TX_CRD_VN4_EXIST_NUM", "NULL"}, {"MAC0_TX_CRD_VN5_EXIST_NUM", "NULL"},
		{"MAC0_TX_CRD_VN6_EXIST_NUM", "NULL"}, {"MAC0_TX_CRD_VN7_EXIST_NUM", "NULL"},
		{"MAC2_TX_LCRD_VNA_EXIST_NUM", "NULL"}, {"MAC2_TX_ICRD_VNA_EXIST_NUM", "NULL"},
		{"MAC2_TX_CRD_VN0_EXIST_NUM", "NULL"}, {"MAC2_TX_CRD_VN1_EXIST_NUM", "NULL"},
		{"MAC2_TX_CRD_VN2_EXIST_NUM", "NULL"}, {"MAC2_TX_CRD_VN3_EXIST_NUM", "NULL"},
		{"MAC2_TX_CRD_VN4_EXIST_NUM", "NULL"}, {"MAC2_TX_CRD_VN5_EXIST_NUM", "NULL"},
		{"MAC2_TX_CRD_VN6_EXIST_NUM", "NULL"}, {"MAC2_TX_CRD_VN7_EXIST_NUM", "NULL"} };

	ret = hikp_roh_query_crd(TEMP_CRD, NUM_ROWS_TEMP_CRDS, temp_crds);
	return ret;
}

static void hikp_roh_mac_show_credit(struct major_cmd_ctrl *self)
{
	int ret;

	printf("******************** CREDIT CNT START ********************\n");
	printf("-------------------  HOLDING CREDIT START-----------------\n");
	ret = hikp_roh_print_holding_crd();
	if (ret != 0) {
		HIKP_ERROR_PRINT("failed to get holding crd, retcode: %d\n", ret);
		self->err_no = ret;
		return;
	}
	printf("-------------------  HOLDING CREDIT END  -----------------\n");
	printf("-------------------  INIT CREDIT START   -----------------\n");
	ret = hikp_roh_print_init_crd();
	if (ret != 0) {
		HIKP_ERROR_PRINT("failed to get init crd, retcode: %d\n", ret);
		self->err_no = ret;
		return;
	}
	printf("-------------------  INIT CREDIT END  --------------------\n");
	printf("-------------------  TEMP CREDIT START  ------------------\n");
	ret = hikp_roh_print_temp_crd();
	if (ret != 0) {
		HIKP_ERROR_PRINT("failed to get temp crd, retcode: %d\n", ret);
		self->err_no = ret;
		return;
	}
	printf("-------------------  TEMP CREDIT END  --------------------\n");
	printf("********************* CREDIT CNT END *********************\n");
}

static int hikp_roh_is_roh(struct major_cmd_ctrl *self)
{
	int mac_type = hikp_roh_get_mac_type(self, g_roh_mac_param.target.bdf);

	if (mac_type == -EIO) {
		HIKP_ERROR_PRINT("Failed get current mac type\n");
	} else if (mac_type == 0) {
		HIKP_ERROR_PRINT("Current mac is not roh type, don't support\n");
		self->err_no = -EINVAL;
	}
	return mac_type;
}

static void hikp_roh_mac_execute_entry(struct major_cmd_ctrl *self)
{
	int mac_type;

	switch (g_roh_mac_param.flag) {
	case (CMD_SHOW_MAC_TYPE_FLAG):
		mac_type = hikp_roh_get_mac_type(self, g_roh_mac_param.target.bdf);
		if (mac_type == -EIO) {
			HIKP_ERROR_PRINT("Failed get current mac type\n");
			return;
		}
		hikp_roh_show_mac_type(self, mac_type);
		break;
	case (CMD_SHOW_CAM_FLAG):
		if (hikp_roh_is_roh(self) > 0)
			hikp_roh_show_cam(self);
		break;
	case (CMD_SHOW_CREDIT_CNT):
		if (hikp_roh_is_roh(self) > 0)
			hikp_roh_mac_show_credit(self);
		break;
	default:
		printf("roh_mac param format error\n");
		self->err_no = -EINVAL;
	}
}

static void cmd_roh_mac_init(void)
{
	struct major_cmd_ctrl *major_cmd = get_major_cmd();

	major_cmd->option_count = 0;
	major_cmd->execute = hikp_roh_mac_execute_entry;

	cmd_option_register("-h", "--help", false, hikp_roh_mac_help);
	cmd_option_register("-i", "--interface", true, hikp_roh_mac_target);
	cmd_option_register("-s", "--show", true, hikp_roh_mac_show_parse);
}

HIKP_CMD_DECLARE("roh_mac", "get roh mac information", cmd_roh_mac_init);
