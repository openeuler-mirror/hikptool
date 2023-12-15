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

#include "hikp_roh_show_mib.h"
#include "hikp_roh_cmd.h"

static struct cmd_roh_show_mib_param roh_show_mib_param = { 0 };

static char g_roh_mac_mib_name[MIB_EVENT_COUNT][ROH_NAME_MAX] = {
	"TX_FROM_SSU_UDM",
	"TX_FROM_SSU_MIN_64",
	"TX_FROM_SSU_65_127",
	"TX_FROM_SSU_128_255",
	"TX_FROM_SSU_256_511",
	"TX_FROM_SSU_512_1023",
	"TX_FROM_SSU_1024_1518",
	"TX_FROM_SSU_1519_2047",
	"TX_FROM_SSU_2048_4095",
	"TX_FROM_SSU_4096_MAX",
	"TX_FROM_SSU_LARGER_THAN_MAX",
	"TX_FROM_SSU_RDMA_PKT",
	"TX_FROM_SSU_RDMA_INC_PKT",
	"TX_FROM_SSU_IP_PKT",
	"TX_FROM_SSU_HNMP_EID_PKT",
	"TX_FROM_SSU_HNMP_DR_PKT",
	"TX_FROM_SSU_ERR_PKT",
	"TX_FROM_SSU_TC0",
	"TX_FROM_SSU_TC1",
	"TX_FROM_SSU_TC2",
	"TX_FROM_SSU_TC3",
	"TX_FROM_SSU_TC4",
	"TX_FROM_SSU_TC5",
	"TX_FROM_SSU_TC6",
	"TX_FROM_SSU_TC7",
	"TX_SENDBY_ROH_MAC_BLOCK",
	"TX_SENDBY_ROH_MAC_PKT",
	"RX_TO_SSU_UDM",
	"RX_TO_SSU_MIN_64",
	"RX_TO_SSU_65_127",
	"RX_TO_SSU_128_255",
	"RX_TO_SSU_256_511",
	"RX_TO_SSU_512_1023",
	"RX_TO_SSU_1024_1518",
	"RX_TO_SSU_1519_2047",
	"RX_TO_SSU_2048_4095",
	"RX_TO_SSU_4096_MAX",
	"RX_TO_SSU_LARGER_THAN_MAX",
	"RX_TO_SSU_RDMA_PKT",
	"RX_TO_SSU_RDMA_INC_PKT",
	"RX_TO_SSU_IP_PKT",
	"RX_TO_SSU_HNMP_EID_PKT",
	"RX_TO_SSU_HNMP_DR_PKT",
	"RX_TO_SSU_ERR_PKT",
	"RX_TO_SSU_TC0",
	"RX_TO_SSU_TC1",
	"RX_TO_SSU_TC2",
	"RX_TO_SSU_TC3",
	"RX_TO_SSU_TC4",
	"RX_TO_SSU_TC5",
	"RX_TO_SSU_TC6",
	"RX_TO_SSU_TC7",
	"RX_RECVBY_ROH_MAC_BLOCK",
	"RX_RECVBY_ROH_MAC_PKT",

	"TX_FROM_SSU_HNMP_UAAP_PKT",
	"RX_FROM_SSU_HNMP_UAAP_PKT",

	// ========== 56 ================
	"NA", "NA", "NA", "NA", "NA", "NA",
	// ========== 62 ================
	"TX_FROM_SSU_HNMP_UAAP_BYTE",
	"RX_FROM_SSU_HNMP_UAAP_BYTE",
	"TX_SENDBY_ROH_MAC_FLIT",
	"RX_RECVBY_ROH_MAC_FLIT",
	// ========== 66 ================
	"TX_FROM_SSU_RDMA_BYTE",
	"TX_FROM_SSU_RDMA_INC_BYTE",
	"TX_FROM_SSU_IP_BYTE",
	"TX_FROM_SSU_HNMP_EID_BYTE",
	"TX_FROM_SSU_HNMP_DR_BYTE",
	"TX_FROM_SSU_TC0_BYTE",
	"TX_FROM_SSU_TC1_BYTE",
	"TX_FROM_SSU_TC2_BYTE",
	"TX_FROM_SSU_TC3_BYTE",
	"TX_FROM_SSU_TC4_BYTE",
	"TX_FROM_SSU_TC5_BYTE",
	"TX_FROM_SSU_TC6_BYTE",
	"TX_FROM_SSU_TC7_BYTE",
	"RX_TO_SSU_RDMA_BYTE",
	"RX_TO_SSU_RDMA_INC_BYTE",
	"RX_TO_SSU_IP_BYTE",
	"RX_TO_SSU_HNMP_EID_BYTE",
	"RX_TO_SSU_HNMP_DR_BYTE",
	"RX_TO_SSU_TC0_BYTE",
	"RX_TO_SSU_TC1_BYTE",
	"RX_TO_SSU_TC2_BYTE",
	"RX_TO_SSU_TC3_BYTE",
	"RX_TO_SSU_TC4_BYTE",
	"RX_TO_SSU_TC5_BYTE",
	"RX_TO_SSU_TC6_BYTE",
	"RX_TO_SSU_TC7_BYTE",
	// ========== 92 ================
	"TX_ICRD_VNA_USE",
	"TX_ICRD_VN_USE_TC0",
	"TX_ICRD_VN_USE_TC1",
	"TX_ICRD_VN_USE_TC2",
	"TX_ICRD_VN_USE_TC3",
	"TX_ICRD_VN_USE_TC4",
	"TX_ICRD_VN_USE_TC5",
	"TX_ICRD_VN_USE_TC6",
	"TX_ICRD_VN_USE_TC7",
	"TX_ICRD_VNA_RELEASE",
	"TX_ICRD_VN_RELEASE_TC0",
	"TX_ICRD_VN_RELEASE_TC1",
	"TX_ICRD_VN_RELEASE_TC2",
	"TX_ICRD_VN_RELEASE_TC3",
	"TX_ICRD_VN_RELEASE_TC4",
	"TX_ICRD_VN_RELEASE_TC5",
	"TX_ICRD_VN_RELEASE_TC6",
	"TX_ICRD_VN_RELEASE_TC7",

	"RX_ICRD_VNA_RECV",
	"RX_ICRD_VN_RECV_TC0",
	"RX_ICRD_VN_RECV_TC1",
	"RX_ICRD_VN_RECV_TC2",
	"RX_ICRD_VN_RECV_TC3",
	"RX_ICRD_VN_RECV_TC4",
	"RX_ICRD_VN_RECV_TC5",
	"RX_ICRD_VN_RECV_TC6",
	"RX_ICRD_VN_RECV_TC7",
	"RX_ICRD_VNA_RELEASE_TO_SSU",
	"RX_ICRD_VN_RELEASE_TO_SSU_TC0",
	"RX_ICRD_VN_RELEASE_TO_SSU_TC1",
	"RX_ICRD_VN_RELEASE_TO_SSU_TC2",
	"RX_ICRD_VN_RELEASE_TO_SSU_TC3",
	"RX_ICRD_VN_RELEASE_TO_SSU_TC4",
	"RX_ICRD_VN_RELEASE_TO_SSU_TC5",
	"RX_ICRD_VN_RELEASE_TO_SSU_TC6",
	"RX_ICRD_VN_RELEASE_TO_SSU_TC7"
};

static int hikp_roh_show_mib_help(struct major_cmd_ctrl *self, const char *argv)
{
	printf("\n  Usage: %s %s\n", self->cmd_ptr->name, "-i <interface> -s\n");
	printf("\n         %s\n", self->cmd_ptr->help_info);
	printf("  Options:\n\n");
	printf("    %s, %-25s %s\n", "-h", "--help", "display this help and exit");
	printf("    %s, %-25s %s\n", "-i", "--interface=<interface>", "device target, e.g. eth1");
	printf("    %s, %-25s %s\n", "-s", "--show", "show non-zero mib");
	printf("\n");
	return 0;
}

static int hikp_roh_show_mib_target(struct major_cmd_ctrl *self, const char *argv)
{
	self->err_no = tool_check_and_get_valid_bdf_id(argv, &(roh_show_mib_param.target));
	if (self->err_no) {
		snprintf(self->err_str, sizeof(self->err_str), "Unknown device %s.", argv);
		return self->err_no;
	}
	return 0;
}

static int hikp_roh_fill_pmu_cnt(int round)
{
	struct roh_show_mib_req_paras req_data = { 0 };
	struct roh_show_mib_rsp_t *mib_rsp = NULL;
	struct hikp_cmd_header req_header = { 0 };
	struct hikp_cmd_ret *cmd_ret = NULL;
	uint64_t mac0_pmu_cnt;
	uint64_t mac2_pmu_cnt;
	int index;
	int ret;

	hikp_cmd_init(&req_header, ROH_MOD, HIKP_ROH_SHOW_MIB, CMD_SHOW_MIB_FILL_CNT);
	req_data.bdf = roh_show_mib_param.target.bdf;
	req_data.round = round;

	cmd_ret = hikp_cmd_alloc(&req_header, &req_data, sizeof(req_data));
	ret = hikp_rsp_normal_check(cmd_ret);
	if (ret != 0) {
		free(cmd_ret);
		cmd_ret = NULL;
		return ret;
	}
	mib_rsp = (struct roh_show_mib_rsp_t *)(cmd_ret->rsp_data);
	for (int i = 0; i < BLOCK_SIZE; i++) {
		index = round * BLOCK_SIZE + i;
		if (index >= MIB_EVENT_COUNT)
			break;

		mac0_pmu_cnt = mib_rsp->reg_data[i][0];
		mac2_pmu_cnt = mib_rsp->reg_data[i][1];
		if (mac0_pmu_cnt)
			printf("MAC0_%-28s : 0x%lx\n", g_roh_mac_mib_name[index], mac0_pmu_cnt);

		if (mac2_pmu_cnt)
			printf("MAC2_%-28s : 0x%lx\n", g_roh_mac_mib_name[index], mac2_pmu_cnt);
	}
	free(cmd_ret);
	cmd_ret = NULL;
	return ret;
}

static void hikp_roh_show_mib_in_multi_rounds(struct major_cmd_ctrl *self)
{
	int mac_type = hikp_roh_get_mac_type(self, roh_show_mib_param.target.bdf);
	int total_round;
	int addition;
	int ret;

	addition = MIB_EVENT_COUNT % RESPONSE_MIB_NUMBER_MAX ? 1 : 0;
	total_round = MIB_EVENT_COUNT / RESPONSE_MIB_NUMBER_MAX + addition;

	if (mac_type == -EIO) {
		self->err_no = -EIO;
		return;
	}
	if (mac_type == 0) {
		HIKP_ERROR_PRINT("Current mac is not roh type, don't support\n");
		self->err_no = -EINVAL;
		return;
	}

	printf("**************ROH MAC MIB INFO*************\n");
	for (int i = 0; i < total_round; i++) {
		ret = hikp_roh_fill_pmu_cnt(i);
		if (ret != 0) {
			printf("Failed to get mib info in %dth round!\n", i);
			return;
		}
	}
	printf("*****************************************\n");
}

static void hikp_roh_show_mib_execute(struct major_cmd_ctrl *self)
{
	if (roh_show_mib_param.flag & ROH_CMD_SHOW_MIB) {
		hikp_roh_show_mib_in_multi_rounds(self);
	} else {
		self->err_no = -EINVAL;
		snprintf(self->err_str, sizeof(self->err_str), "Invalid operation\n");
	}
}

static int hikp_roh_show_mib_parse(struct major_cmd_ctrl *self, const char *argv)
{
	roh_show_mib_param.flag |= ROH_CMD_SHOW_MIB;
	return 0;
}

static void cmd_roh_show_mib_init(void)
{
	struct major_cmd_ctrl *major_cmd = get_major_cmd();

	major_cmd->option_count = 0;
	major_cmd->execute = hikp_roh_show_mib_execute;

	cmd_option_register("-h", "--help", false, hikp_roh_show_mib_help);
	cmd_option_register("-i", "--interface", true, hikp_roh_show_mib_target);
	cmd_option_register("-s", "--show", false, hikp_roh_show_mib_parse);
}

HIKP_CMD_DECLARE("roh_show_mib", "get roh mac mib information", cmd_roh_show_mib_init);
