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

#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "hikp_unic_ppp.h"

static struct hikp_unic_ppp_hw_resources g_unic_ppp_hw_res = { 0 };
static struct unic_ppp_param g_unic_ppp_param = { 0 };

static void hikp_unic_ppp_show_ip_tbl(const void *data);
static void hikp_unic_ppp_show_guid_tbl(const void *data);

static int hikp_unic_query_ppp_ip_tbl(struct hikp_cmd_header *req_header,
				      const struct bdf_t *bdf, void *data);
static int hikp_unic_query_ppp_guid_tbl(struct hikp_cmd_header *req_header,
					const struct bdf_t *bdf, void *data);

static const struct unic_ppp_feature_cmd g_unic_ppp_feature_cmd[] = {
	{ UNIC_PPP_IP_TBL_NAME, UNIC_IP_TBL_DUMP, true,
	 hikp_unic_query_ppp_ip_tbl, hikp_unic_ppp_show_ip_tbl },
	{ UNIC_PPP_GUID_TBL_NAME, UNIC_GUID_TBL_DUMP, true,
	 hikp_unic_query_ppp_guid_tbl, hikp_unic_ppp_show_guid_tbl },
};

static int hikp_unic_ppp_cmd_help(struct major_cmd_ctrl *self, const char *argv)
{
	printf("\n  Usage: %s %s\n", self->cmd_ptr->name, "-i <interface>");
	printf("\n         %s\n", self->cmd_ptr->help_info);
	printf("\n  Options:\n\n");
	printf("    %s, %-25s %s\n", "-h", "--help", "display this help and exit");
	printf("    %s, %-25s %s\n", "-i", "--interface=<interface>",
	       "device target or bdf id, e.g. ubn0 or 0000:35:00.0");
	printf("    %s, %-24s %s\n", "-du", "--dump", "dump ip or guid table info.");

	return 0;
}

static int hikp_unic_cmd_get_ppp_target(struct major_cmd_ctrl *self, const char *argv)
{
	self->err_no = tool_check_and_get_valid_bdf_id(argv, &(g_unic_ppp_param.target));
	if (self->err_no != 0) {
		snprintf(self->err_str, sizeof(self->err_str), "unknown device!");
		return self->err_no;
	}

	return 0;
}

static int hikp_unic_cmd_ppp_feature_select(struct major_cmd_ctrl *self, const char *argv)
{
	size_t feat_size = HIKP_ARRAY_SIZE(g_unic_ppp_feature_cmd);
	size_t i;

	for (i = 0; i < feat_size; i++) {
		if (strncmp(argv, g_unic_ppp_feature_cmd[i].feature_name,
			    HIKP_UNIC_PPP_MAX_FEATURE_NAME_LEN) == 0) {
			g_unic_ppp_param.feature_idx = i;
			return 0;
		}
	}

	hikp_unic_ppp_cmd_help(self, NULL);
	snprintf(self->err_str, sizeof(self->err_str), "please input valid dump type.");
	self->err_no = -EINVAL;

	return self->err_no;
}

static int hikp_unic_ppp_get_blk(struct hikp_cmd_header *req_header,
				 const struct unic_ppp_req_para *req_data, void *buf,
				 size_t buf_len, struct unic_ppp_rsp_head *rsp_head)
{
	struct hikp_cmd_ret *cmd_ret;
	struct unic_ppp_rsp *rsp;
	int ret = 0;

	cmd_ret = hikp_cmd_alloc(req_header, req_data, sizeof(*req_data));
	if (cmd_ret == NULL || cmd_ret->status != 0) {
		ret = -EIO;
		goto out;
	}

	rsp = (struct unic_ppp_rsp *)cmd_ret->rsp_data;
	if (rsp->rsp_head.cur_blk_size > buf_len) {
		HIKP_ERROR_PRINT("unic_ppp block context copy size error, "
				 "buffer size=%llu, data size=%u.\n",
				 buf_len, rsp->rsp_head.cur_blk_size);
		ret = -EINVAL;
		goto out;
	}
	memcpy(buf, rsp->rsp_data, rsp->rsp_head.cur_blk_size);
	memcpy(rsp_head, &rsp->rsp_head, sizeof(struct unic_ppp_rsp_head));

out:
	free(cmd_ret);
	return ret;
}

static int hikp_unic_query_ppp_by_blkid(struct hikp_cmd_header *req_header, const struct bdf_t *bdf,
					void *data, size_t len)
{
	struct unic_ppp_rsp_head rsp_head = { 0 };
	struct unic_ppp_req_para req_data = { 0 };
	uint32_t total_blk_size;
	uint8_t blk_id = 0;
	int ret = 0;

	req_data.bdf = *bdf;
	req_data.block_id = blk_id;
	ret = hikp_unic_ppp_get_blk(req_header, &req_data, data, len, &rsp_head);
	if (ret != 0) {
		HIKP_ERROR_PRINT("Fail to get block-%u context.\n", blk_id);
		return ret;
	}
	total_blk_size = rsp_head.cur_blk_size;

	for (blk_id = 1; blk_id < rsp_head.total_blk_num; blk_id++) {
		req_data.block_id = blk_id;
		ret = hikp_unic_ppp_get_blk(req_header, &req_data, (uint8_t *)data + total_blk_size,
					    len - total_blk_size, &rsp_head);
		if (ret != 0) {
			HIKP_ERROR_PRINT("Fail to get block-%u context.\n", blk_id);
			return ret;
		}
		total_blk_size += rsp_head.cur_blk_size;
	}

	return ret;
}

static int hikp_unic_get_ppp_entry_hw_res(const struct bdf_t *bdf,
					  struct hikp_unic_ppp_hw_resources *hw_res)
{
	struct hikp_cmd_header req_header = { 0 };

	hikp_cmd_init(&req_header, UB_MOD, GET_UNIC_PPP_CMD, UNIC_PPP_ENTRY_HW_SPEC_GET);
	return hikp_unic_query_ppp_by_blkid(&req_header, bdf, hw_res,
					    sizeof(struct hikp_unic_ppp_hw_resources));
}

static int hikp_unic_ppp_alloc_ip_tbl_entry(const struct hikp_unic_ppp_hw_resources *hw_res,
					    struct unic_ip_tbl *ip_tbl)
{
	uint32_t max_ip_entry_size;

	max_ip_entry_size = hw_res->ip_max_mem_size + hw_res->ip_overflow_size;
	if (max_ip_entry_size == 0) {
		HIKP_ERROR_PRINT("ip tbl query is not supported\n");
		return -EIO;
	}
	ip_tbl->entry = (struct unic_ip_entry *)calloc(max_ip_entry_size,
						       sizeof(struct unic_ip_entry));
	if (ip_tbl->entry == NULL) {
		HIKP_ERROR_PRINT("fail to alloc ip_table_entry memory.\n");
		return -ENOMEM;
	}

	return 0;
}

static int hikp_unic_ppp_alloc_guid_tbl_entry(const struct hikp_unic_ppp_hw_resources *hw_res,
					      struct unic_guid_tbl *guid_tbl)
{
	if (hw_res->uc_guid_tbl_size == 0 && hw_res->mc_guid_tbl_size == 0) {
		HIKP_ERROR_PRINT("guid tbl query is not supported\n");
		return -EIO;
	}
	guid_tbl->uc_tbl.entry = (struct unic_guid_uc_entry *)calloc(hw_res->uc_guid_tbl_size,
				 sizeof(struct unic_guid_uc_entry));
	if (guid_tbl->uc_tbl.entry == NULL) {
		HIKP_ERROR_PRINT("fail to alloc uc_guid_entry_table memory.\n");
		return -ENOMEM;
	}

	guid_tbl->mc_tbl.entry = (struct unic_guid_mc_entry *)calloc(hw_res->mc_guid_tbl_size,
				 sizeof(struct unic_guid_mc_entry));
	if (guid_tbl->mc_tbl.entry == NULL) {
		HIKP_ERROR_PRINT("fail to alloc mc_guid_entry_table memory.\n");
		free(guid_tbl->uc_tbl.entry);
		return -ENOMEM;
	}

	return 0;
}

static union unic_ppp_feature_info*
hikp_unic_ppp_data_alloc(const struct unic_ppp_feature_cmd *unic_ppp_cmd,
			 const struct hikp_unic_ppp_hw_resources *hw_res)
{
	union unic_ppp_feature_info *unic_ppp_data;
	int ret = -1;

	unic_ppp_data = (union unic_ppp_feature_info *)calloc(1,
							      sizeof(union unic_ppp_feature_info));
	if (unic_ppp_data == NULL) {
		HIKP_ERROR_PRINT("Fail to allocate unic_ppp_feature_info memory.\n");
		return NULL;
	}

	if (g_unic_ppp_param.feature_idx == UNIC_PPP_IP_FEATURE_IDX) {
		ret = hikp_unic_ppp_alloc_ip_tbl_entry(hw_res, &unic_ppp_data->ip_tbl);
	} else if (g_unic_ppp_param.feature_idx == UNIC_PPP_GUID_FEATURE_IDX) {
		ret = hikp_unic_ppp_alloc_guid_tbl_entry(hw_res, &unic_ppp_data->guid_tbl);
	}

	if (ret != 0) {
		goto out;
	}

	return unic_ppp_data;
out:
	free(unic_ppp_data);
	return NULL;
}

static int hikp_unic_query_ppp_ip_tbl(struct hikp_cmd_header *req_header, const struct bdf_t *bdf,
				      void *data)
{
	struct unic_ip_tbl *ip_tbl = (struct unic_ip_tbl *)data;
	struct unic_ppp_rsp_head unic_rsp_head = { 0 };
	struct unic_ppp_req_para req_data = { 0 };
	uint32_t max_ip_entry_size;
	uint32_t entry_size = 0;
	size_t left_buf_len = 0;
	uint32_t index = 0;
	int ret = -1;

	max_ip_entry_size = g_unic_ppp_hw_res.ip_max_mem_size + g_unic_ppp_hw_res.ip_overflow_size;
	req_data.bdf = *bdf;
	while (index < max_ip_entry_size) {
		req_data.cur_entry_idx = index;
		left_buf_len = sizeof(struct unic_ip_entry) * (max_ip_entry_size - entry_size);
		ret = hikp_unic_ppp_get_blk(req_header, &req_data, ip_tbl->entry + entry_size,
					    left_buf_len, &unic_rsp_head);
		if (ret != 0) {
			HIKP_ERROR_PRINT("Fail to get the ip entry after index=%u, ret=%d.\n",
					 index, ret);
			return ret;
		}
		entry_size += unic_rsp_head.cur_blk_entry_cnt;
		index = unic_rsp_head.next_entry_idx;
	}
	ip_tbl->entry_size = entry_size;

	return ret;
}

static int hikp_unic_query_ppp_guid_tbl(struct hikp_cmd_header *req_header,
					const struct bdf_t *bdf, void *data)
{
	struct unic_guid_tbl *guid_tbl = (struct unic_guid_tbl*)data;
	struct unic_ppp_rsp_head unic_rsp_head = { 0 };
	struct unic_ppp_req_para req_data = { 0 };
	uint32_t entry_size = 0;
	size_t left_buf_len = 0;
	uint32_t index = 0;
	int ret = -1;

	req_data.bdf = *bdf;
	req_data.is_unicast = 1;
	while (index < g_unic_ppp_hw_res.uc_guid_tbl_size) {
		req_data.cur_entry_idx = index;
		left_buf_len = sizeof(struct unic_guid_uc_entry) *
				     (g_unic_ppp_hw_res.uc_guid_tbl_size - entry_size);
		ret = hikp_unic_ppp_get_blk(req_header, &req_data,
					    guid_tbl->uc_tbl.entry + entry_size,
					    left_buf_len, &unic_rsp_head);
		if (ret != 0) {
			HIKP_ERROR_PRINT("Fail to get the uc_guid entry after index=%u, ret=%d.\n",
					 index, ret);
			return ret;
		}
		entry_size += unic_rsp_head.cur_blk_entry_cnt;
		index = unic_rsp_head.next_entry_idx;
	}
	guid_tbl->uc_tbl.entry_size = entry_size;

	left_buf_len = 0;
	entry_size = 0;
	index = 0;
	req_data.is_unicast = 0;

	while (index < g_unic_ppp_hw_res.mc_guid_tbl_size) {
		req_data.cur_entry_idx = index;
		left_buf_len = sizeof(struct unic_guid_mc_entry) *
				     (g_unic_ppp_hw_res.mc_guid_tbl_size - entry_size);
		ret = hikp_unic_ppp_get_blk(req_header, &req_data,
					    guid_tbl->mc_tbl.entry + entry_size,
					    left_buf_len, &unic_rsp_head);
		if (ret != 0) {
			HIKP_ERROR_PRINT("Fail to get the mc_guid entry after index=%u, ret=%d.\n",
					 index, ret);
			return ret;
		}
		entry_size += unic_rsp_head.cur_blk_entry_cnt;
		index = unic_rsp_head.next_entry_idx;
	}
	guid_tbl->mc_tbl.entry_size = entry_size;

	return ret;
}

static void hikp_unic_ppp_show_ip_tbl(const void *data)
{
	struct unic_ip_tbl *ip_tbl = (struct unic_ip_tbl *)data;
	struct unic_ip_entry *entry;
	uint16_t *ip_addr_tbl_str;
	int i, j;

	printf("ip_table_size = %u\n", ip_tbl->entry_size);
	printf("index\t| func_id\t| ip_addr\n");
	for (i = 0; i < ip_tbl->entry_size; i++) {
		entry = &ip_tbl->entry[i];
		ip_addr_tbl_str = (uint16_t *)entry->ip_addr;
		printf("%-4u\t| %-3u\t\t| ", entry->index, entry->function_id);
		for (j = 0; j < IP_ADDR_TBL_LEN - 1; j++)
			printf("%04x:" , ntohs(ip_addr_tbl_str[j]));
		printf("%04x\n",  ntohs(ip_addr_tbl_str[IP_ADDR_TBL_LEN - 1]));
	}
}

static void hikp_unic_ppp_show_guid_tbl(const void *data)
{
	struct unic_guid_tbl *guid_tbl = (struct unic_guid_tbl *)data;
	uint32_t cnt;
	int i;

	printf("unicast guid num : %u\n", guid_tbl->uc_tbl.entry_size);
	if (guid_tbl->uc_tbl.entry_size > 0) {
		printf("| num\t| func id | GUID \n");
		for (cnt = 0; cnt < guid_tbl->uc_tbl.entry_size; cnt++) {
			printf("| %3u\t| %7u | ", cnt, guid_tbl->uc_tbl.entry[cnt].function_id);
			for (i = 0; i < HIKP_UNIC_GUID_ADDR_LEN - 1; i++) {
				printf("%02x:", guid_tbl->uc_tbl.entry[cnt].guid_addr[i]);
			}
			printf("%02x\n",
			       guid_tbl->uc_tbl.entry[cnt].guid_addr[HIKP_UNIC_GUID_ADDR_LEN - 1]);
		}
	}

	printf("multicast guid num : %u\n", guid_tbl->mc_tbl.entry_size);
	if (guid_tbl->mc_tbl.entry_size > 0) {
		printf("| num\t|  idx\t| %-48s\t| bitmap\n", "GUID");
		for (cnt = 0; cnt < guid_tbl->mc_tbl.entry_size; cnt++) {
			printf("| %3u\t| %4u\t| ", cnt, guid_tbl->mc_tbl.entry[cnt].idx);
			for (i = 0; i < HIKP_UNIC_GUID_ADDR_LEN - 1; i++) {
				printf("%02x:", guid_tbl->mc_tbl.entry[cnt].guid_addr[i]);
			}
			printf("%02x\t| ",
			       guid_tbl->mc_tbl.entry[cnt].guid_addr[HIKP_UNIC_GUID_ADDR_LEN - 1]);
			for (i = HIKP_UNIC_GUID_BITMAP_LEN - 1; i > 0; i--) {
				printf("%08x:", guid_tbl->mc_tbl.entry[cnt].function_bitmap[i]);
			}
			printf("%08x\n", guid_tbl->mc_tbl.entry[cnt].function_bitmap[0]);
		}
	}
}

static int hikp_unic_ppp_check_input_param(struct major_cmd_ctrl *self,
					   const struct unic_ppp_param *ppp_param)
{
	const struct bdf_t *bdf = &ppp_param->target.bdf;

	if (bdf->dev_id != 0) {
		snprintf(self->err_str, sizeof(self->err_str), "VF does not support query!");
		self->err_no = -EINVAL;
		return self->err_no;
	}

	if (ppp_param->feature_idx == UNIC_PPP_INIT_FEATURE_IDX) {
		hikp_unic_ppp_cmd_help(self, NULL);
		snprintf(self->err_str, sizeof(self->err_str), "-du/--dump parameter error!");
		self->err_no = -EINVAL;
		return self->err_no;
	}

	return 0;
}

static void hikp_unic_ppp_data_free(union unic_ppp_feature_info *unic_ppp_data)
{
	struct unic_guid_tbl *guid_tbl;
	struct unic_ip_tbl *ip_tbl;

	if (g_unic_ppp_param.feature_idx == UNIC_PPP_IP_FEATURE_IDX) {
		ip_tbl = &unic_ppp_data->ip_tbl;
		free(ip_tbl->entry);
	} else if (g_unic_ppp_param.feature_idx == UNIC_PPP_GUID_FEATURE_IDX) {
		guid_tbl = &unic_ppp_data->guid_tbl;
		free(guid_tbl->uc_tbl.entry);
		free(guid_tbl->mc_tbl.entry);
	}

	free(unic_ppp_data);
}

static void hikp_unic_ppp_cmd_execute(struct major_cmd_ctrl *self)
{
	const struct unic_ppp_feature_cmd *unic_ppp_cmd;
	union unic_ppp_feature_info *unic_ppp_data;
	struct hikp_cmd_header req_header = {0};
	int ret;

	ret = hikp_unic_ppp_check_input_param(self, &g_unic_ppp_param);
	if (ret != 0)
		return;

	ret = hikp_unic_get_ppp_entry_hw_res(&g_unic_ppp_param.target.bdf, &g_unic_ppp_hw_res);
	if (ret != 0) {
		snprintf(self->err_str, sizeof(self->err_str),
			 "fail to obtain unic_ppp hardware resources.");
		self->err_no = ret;
		return;
	}

	unic_ppp_cmd = &g_unic_ppp_feature_cmd[g_unic_ppp_param.feature_idx];
	unic_ppp_data = hikp_unic_ppp_data_alloc(unic_ppp_cmd, &g_unic_ppp_hw_res);
	if (unic_ppp_data == NULL) {
		snprintf(self->err_str, sizeof(self->err_str),
			 "failed to allocate unic_ppp_data memory!");
		self->err_no = -ENOMEM;
		return;
	}

	hikp_cmd_init(&req_header, UB_MOD, GET_UNIC_PPP_CMD, unic_ppp_cmd->sub_cmd_code);
	ret = unic_ppp_cmd->query(&req_header, &g_unic_ppp_param.target.bdf, unic_ppp_data);
	if (ret != 0) {
		snprintf(self->err_str, sizeof(self->err_str),
			 "failed to query %s info, ret = %d.", unic_ppp_cmd->feature_name, ret);
		self->err_no = ret;
		goto out;
	}

	printf("############## UNIC_PPP: %s info ############\n", unic_ppp_cmd->feature_name);
	unic_ppp_cmd->show(unic_ppp_data);
	printf("#################### END #######################\n");

out:
	hikp_unic_ppp_data_free(unic_ppp_data);
}

static void cmd_unic_get_ppp_init(void)
{
	struct major_cmd_ctrl *major_cmd = get_major_cmd();

	g_unic_ppp_param.feature_idx = UNIC_PPP_INIT_FEATURE_IDX;
	major_cmd->execute = hikp_unic_ppp_cmd_execute;

	cmd_option_register("-h", "--help", false, hikp_unic_ppp_cmd_help);
	cmd_option_register("-i", "--interface", true, hikp_unic_cmd_get_ppp_target);
	cmd_option_register("-du", "--dump", true, hikp_unic_cmd_ppp_feature_select);
}

HIKP_CMD_DECLARE("unic_ppp", "dump ppp info of unic!", cmd_unic_get_ppp_init);