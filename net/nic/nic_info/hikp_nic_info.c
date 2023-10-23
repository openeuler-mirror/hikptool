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

#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <unistd.h>
#include "tool_cmd.h"
#include "hikp_net_lib.h"
#include "hikp_nic_info.h"

static struct nic_info_param g_info_param = { 0 };

static int hikp_nic_cmd_get_info_help(struct major_cmd_ctrl *self, const char *argv)
{
	printf("\n  Usage: %s %s\n", self->cmd_ptr->name, "-i <device>");
	printf("\n         %s\n", self->cmd_ptr->help_info);
	printf("\n  Options:\n\n");
	printf("    %s, %-25s %s\n", "-h", "--help", "display this help and exit");
	printf("    %s, %-25s %s\n", "-i", "--interface=<interface>",
	       "device target or bdf id, e.g. eth0~7 or 0000:35:00.0");

	printf("\n");

	return 0;
}

static int hikp_nic_cmd_get_info_target(struct major_cmd_ctrl *self, const char *argv)
{
	self->err_no = tool_check_and_get_valid_bdf_id(argv, &(g_info_param.target));
	if (self->err_no != 0) {
		snprintf(self->err_str, sizeof(self->err_str), "Unknown device %s.", argv);
		return self->err_no;
	}
	g_info_param.have_interface = true;

	return 0;
}

static int hikp_nic_get_hw_info(struct hikp_cmd_ret **cmd_ret)
{
	struct nic_info_req_para req_data = { 0 };
	struct hikp_cmd_header req_header = { 0 };

	req_data.bdf = g_info_param.target.bdf;
	hikp_cmd_init(&req_header, NIC_MOD, GET_CHIP_INFO_CMD, CHIP_INFO_DUMP);
	*cmd_ret = hikp_cmd_alloc(&req_header, &req_data, sizeof(req_data));

	return hikp_rsp_normal_check(*cmd_ret);
}

static int hikp_nic_get_curr_die_info(void)
{
	struct nic_info_rsp_t *info_rsp = NULL;
	struct hikp_cmd_ret *cmd_ret = NULL;
	int ret;

	ret = hikp_nic_get_hw_info(&cmd_ret);
	if (ret != 0) {
		HIKP_ERROR_PRINT("Get chip info fail.\n");
		free(cmd_ret);
		cmd_ret = NULL;
		return ret;
	}
	info_rsp = (struct nic_info_rsp_t *)(cmd_ret->rsp_data);
	g_info_param.info = *info_rsp;
	free(cmd_ret);
	cmd_ret = NULL;

	ret = get_revision_id_by_bdf(&g_info_param.target.bdf, g_info_param.revision_id);
	if (ret != 0) {
		HIKP_ERROR_PRINT("Getting revision fail.\n");
		return ret;
	}
	ret = get_numvfs_by_bdf(&g_info_param.target.bdf, &g_info_param.numvfs);
	if (ret != 0)
		HIKP_ERROR_PRINT("Getting numvfs fail, ret = %d.\n", ret);
	ret = get_dev_name_by_bdf(&g_info_param.target.bdf, g_info_param.target.dev_name);
	if ((ret != 0) && (ret != -ENOENT)) {
		HIKP_ERROR_PRINT("Getting dev name fail.\n");
		return ret;
	}

	return 0;
}

static void hikp_nic_info_print_cur_pf(const struct bdf_t *bdf)
{
	struct tool_target *pf_target = &g_info_param.target;
	struct tool_target target = { 0 };
	uint8_t pf_id = bdf->fun_id;
	uint8_t i;
	int ret;

	printf("Current function: pf%u\n", pf_id);
	printf("\t%-16s %s\n", "pf mode:",
	       g_info_param.info.pf_info[pf_id].pf_mode ? "X86" : "ARM");
	printf("\t%-16s %04x:%02x:%02x.%u\n", "bdf id:",
	       bdf->domain, bdf->bus_id, bdf->dev_id, bdf->fun_id);
	printf("\t%-16s %u\n", "mac id:", g_info_param.info.pf_info[pf_id].mac_id);
	printf("\t%-16s %s\n", "mac type:",
	       g_info_param.info.pf_info[pf_id].mac_type ? "ROH" : "ETH");
	printf("\t%-16s %u\n", "func_num:", g_info_param.info.pf_info[pf_id].func_num);
	printf("\t%-16s %u\n", "tqp_num:", g_info_param.info.pf_info[pf_id].tqp_num);
	printf("\t%-16s 0x%x\n", "pf_cap_flag:", g_info_param.info.pf_info[pf_id].pf_cap_flag);
	if (pf_target->dev_name[0] != 0) {
		printf("\t%-16s %s\n", "dev name:", pf_target->dev_name);
		for (i = 0; i < g_info_param.numvfs; i++) {
			ret = get_vf_dev_info_by_pf_dev_name((const char *)pf_target->dev_name,
							     &target, i);
			if (ret == -ENOENT)
				continue;

			if (ret != 0) {
				HIKP_ERROR_PRINT("Getting vf's dev name fail.\n");
				return;
			}
			printf("\t    pf%u-vf%u: %s <-> %04x:%02x:%02x.%u\n", pf_id, i,
			       target.dev_name, target.bdf.domain, target.bdf.bus_id,
			       target.bdf.dev_id, target.bdf.fun_id);
		}
	}
	printf("\n");
}

static void hikp_nic_info_print_cur_die(void)
{
	uint8_t i;

	printf("Current die(chip%u-die%u) info:\n",
	       g_info_param.info.chip_id, g_info_param.info.die_id);
	printf("revision id: %s", g_info_param.revision_id);
	printf("mac mode: %u\n", g_info_param.info.mac_mode);
	printf("pf number: %u\n", g_info_param.info.pf_num);
	printf("pf's capability flag: 0x%x\n", g_info_param.info.cap_flag);
	printf("pf's attributes and capabilities:\n");
	printf("%-16s", "pf id:");
	for (i = 0; i < g_info_param.info.pf_num; i++)
		printf("pf%u\t", i);

	printf("\n%-16s", "pf mode:");
	for (i = 0; i < g_info_param.info.pf_num; i++)
		printf("%s\t", g_info_param.info.pf_info[i].pf_mode ? "X86" : "ARM");

	printf("\n%-16s", "mac id:");
	for (i = 0; i < g_info_param.info.pf_num; i++)
		printf("mac%u\t", g_info_param.info.pf_info[i].mac_id);

	printf("\n%-16s", "mac type:");
	for (i = 0; i < g_info_param.info.pf_num; i++)
		printf("%s\t", g_info_param.info.pf_info[i].mac_type ? "ROH" : "ETH");

	printf("\n%-16s", "func num:");
	for (i = 0; i < g_info_param.info.pf_num; i++)
		printf("%u\t", g_info_param.info.pf_info[i].func_num);

	printf("\n%-16s", "tqp num:");
	for (i = 0; i < g_info_param.info.pf_num; i++)
		printf("%u\t", g_info_param.info.pf_info[i].tqp_num);

	printf("\n\n");
}

static bool is_bus_id_accessed(void)
{
	uint8_t i;

	for (i = 0; i < g_info_param.accessed_die_num; i++) {
		if (g_info_param.accessed_bus_id[i] == g_info_param.target.bdf.bus_id)
			return true;
	}
	g_info_param.accessed_bus_id[i] = g_info_param.target.bdf.bus_id;

	return false;
}

static int hikp_nic_get_and_print_curr_die(int sockfd, struct tool_target *target,
					   char *revision_id, struct ifaddrs *ifa_node)
{
	int ret;

	memset(target, 0, sizeof(struct tool_target));
	memset(revision_id, 0, MAX_PCI_ID_LEN + 1);

	if (strlen(ifa_node->ifa_name) >= IFNAMSIZ) {
		HIKP_ERROR_PRINT("parameter of device name is too long.\n");
		return -EINVAL;
	}
	strncpy(target->dev_name, ifa_node->ifa_name, sizeof(target->dev_name));
	target->dev_name[sizeof(target->dev_name) - 1] = '\0';
	if (!is_dev_valid_and_special(sockfd, target) ||
		is_bus_id_accessed()) {
		return 0;
	}
	ret = hikp_nic_get_curr_die_info();
	if (ret != 0)
		return ret;
	hikp_nic_info_print_cur_die();
	g_info_param.accessed_die_num++;

	return 0;
}

static int hikp_nic_traverse_all_hns3_dev_and_get_info(void)
{
	struct tool_target *target = &g_info_param.target;
	char *revision_id = g_info_param.revision_id;
	struct ifaddrs *ifa_node;
	struct ifaddrs *ifa_lst;
	int sockfd;
	int ret;

	sockfd = hikp_net_creat_sock();
	if (sockfd < MIN_SOCKFD) {
		HIKP_ERROR_PRINT("creat sockfd failed, sockfd is %d.\n", sockfd);
		return -EIO;
	}
	ret = getifaddrs(&ifa_lst);
	if (ret < 0) {
		HIKP_ERROR_PRINT("getifaddrs failed.\n");
		goto sock_out;
	}
	for (ifa_node = ifa_lst; ifa_node != NULL; ifa_node = ifa_node->ifa_next) {
		if (ifa_node->ifa_addr == NULL)
			continue;

		if (ifa_node->ifa_addr->sa_family != AF_PACKET)
			continue;

		ret = hikp_nic_get_and_print_curr_die(sockfd, target, revision_id, ifa_node);
		if (ret != 0)
			break;
	}
	freeifaddrs(ifa_lst);
sock_out:
	close(sockfd);

	return ret;
}

static void hikp_nic_info_print_cur_vf(const struct bdf_t *bdf)
{
	struct tool_target target = { 0 };
	int ret;

	printf("Current function is vf:\n");
	printf("\t%-16s %04x:%02x:%02x.%u\n", "vf bdf id:",
	       bdf->domain, bdf->bus_id, bdf->dev_id, bdf->fun_id);
	ret = get_dev_name_by_bdf(&g_info_param.target.bdf, g_info_param.target.dev_name);
	if ((ret != 0) && (ret != -ENOENT)) {
		HIKP_ERROR_PRINT("Getting dev name fail.\n");
		return;
	}
	if (g_info_param.target.dev_name[0] != 0)
		printf("\t%-16s %s\n", "vf dev name:", g_info_param.target.dev_name);

	if (ret == 0) {
		ret = get_pf_dev_info_by_vf_dev_name((const char *)g_info_param.target.dev_name,
						     &target);
		if ((ret != 0) && (ret != -ENOENT)) {
			HIKP_ERROR_PRINT("Getting pf dev name fail.\n");
			return;
		}
		printf("Belong to:\n");
		printf("\t%-16s pf%u\n", "pf id:", target.bdf.fun_id);
		printf("\t%-16s %04x:%02x:%02x.%u\n", "pf bdf id:", target.bdf.domain,
		       target.bdf.bus_id, target.bdf.dev_id, target.bdf.fun_id);
		printf("\t%-16s %s\n", "pf dev name:", target.dev_name);
	}
}

static void hikp_nic_info_cmd_execute(struct major_cmd_ctrl *self)
{
	struct bdf_t *bdf = &g_info_param.target.bdf;

	if (!g_info_param.have_interface) {
		self->err_no = hikp_nic_traverse_all_hns3_dev_and_get_info();
		if (self->err_no != 0)
			snprintf(self->err_str, sizeof(self->err_str),
				 "traverse all hns3 dev fail.");
		return;
	}
	if (bdf->dev_id != 0) {
		hikp_nic_info_print_cur_vf(&g_info_param.target.bdf);
		return;
	}
	self->err_no = hikp_nic_get_curr_die_info();
	if (self->err_no != 0) {
		snprintf(self->err_str, sizeof(self->err_str), "get current die info fail.");
		return;
	}
	hikp_nic_info_print_cur_pf(&g_info_param.target.bdf);
	hikp_nic_info_print_cur_die();
}

static void cmd_nic_get_info_init(void)
{
	struct major_cmd_ctrl *major_cmd = get_major_cmd();

	major_cmd->option_count = 0;
	major_cmd->execute = hikp_nic_info_cmd_execute;

	cmd_option_register("-h", "--help", false, hikp_nic_cmd_get_info_help);
	cmd_option_register("-i", "--interface", true, hikp_nic_cmd_get_info_target);
}

HIKP_CMD_DECLARE("nic_info", "show basic info of network!", cmd_nic_get_info_init);
