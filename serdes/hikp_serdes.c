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

#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <string.h>
#include "hikptdev_plug.h"
#include "tool_lib.h"
#include "tool_cmd.h"
#include "hikp_serdes.h"

static struct cmd_serdes_param g_serdes_param = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

#define SERDES_OUTPUT_MAX_SIZE 2560
static char g_serdes_data_out_buf[SERDES_OUTPUT_MAX_SIZE] = {0};
static struct hilink_cmd_out g_out_put = {0};

static void hikp_serdes_info_print(struct cmd_serdes_param *cmd);
static void hikp_serdes_dump_print(struct cmd_serdes_param *cmd);

static int cmd_serdes_maininfo_help(struct major_cmd_ctrl *self, const char *argv)
{
	printf("\n  Usage: %s %s\n", self->cmd_ptr->name,
	       "-i <chipid> -s <start_lane_id> -n <lane_num> [-k]");
	printf("\n         %s, e.g. hikptool serdes_info -i 0 -s m3d0 -n 4 -k\n",
	       self->cmd_ptr->help_info);
	printf("\n  Options:\n\n"
	       "  -h    --help           display the help info\n"
	       "  -i    --chipid         chipid, usage: -i [chip_id], e.g. -i 0\n"
	       "  -s    --start_lane_id  the start of lane id, "
	       "usage: -s m[macro_id]d[lane_id], e.g. -s m3d0\n"
	       "  -n    --lane_num       lane num, usage: -n [lane_num], e.g. -n 4\n"
	       "  -k    --key_info       show detail info with -k, brief info without -k\n");
	return 0;
}

static int cmd_serdes_chipid(struct major_cmd_ctrl *self, const char *argv)
{
	int err;
	uint8_t chip_id;
	size_t len = strlen("chip");
	char *p_index = (char *)argv;

	if (strncmp(p_index, "chip", len) == 0)
		p_index = p_index + len;

	err = string_toub(p_index, &chip_id);
	if (err) {
		snprintf(self->err_str, sizeof(self->err_str), "Invalid chipid.");
		self->err_no = err;
		return err;
	}

	g_serdes_param.chip_id = chip_id;
	return 0;
}

static int cmd_serdes_start_lane_id(struct major_cmd_ctrl *self, const char *argv)
{
	int64_t macro_id;
	int64_t ds_id;
	char *endptr = NULL;
	const char *ptr = argv;

	if ((*ptr != 'm') && (*ptr != 'M'))
		goto _START_LANE_ID_ERR_PRO_;

	ptr++;
	macro_id = strtol(ptr, &endptr, 10); /* 10:decimal */
	if ((endptr <= ptr) || (macro_id < 0) || (macro_id > UCHAR_MAX))
		goto _START_LANE_ID_ERR_PRO_;

	ptr = endptr;
	if ((*ptr != 'd') && (*ptr != 'D'))
		goto _START_LANE_ID_ERR_PRO_;

	ptr++;
	ds_id = strtol(ptr, &endptr, 10); /* 10:decimal */
	if ((endptr <= ptr) || (*endptr != 0))
		goto _START_LANE_ID_ERR_PRO_;

	if ((ds_id < 0) || (ds_id > UCHAR_MAX))
		goto _START_LANE_ID_ERR_PRO_;

	g_serdes_param.macro_id = (uint8_t)macro_id;
	g_serdes_param.start_sds_id = (uint8_t)ds_id;
	return 0;

_START_LANE_ID_ERR_PRO_:
	snprintf(self->err_str, sizeof(self->err_str), "Invalid start_lane_id.e.g. m4d1");
	self->err_no = -EINVAL;
	return -EINVAL;
}

static int cmd_serdes_lane_num(struct major_cmd_ctrl *self, const char *argv)
{
	uint8_t lane_num;

	self->err_no = string_toub(argv, &lane_num);
	if (self->err_no || lane_num == 0) {
		snprintf(self->err_str, sizeof(self->err_str), "Invalid lane_num.");
		return self->err_no;
	}

	g_serdes_param.sds_num = lane_num;
	return 0;
}

static int cmd_serdes_key_info_pro(struct major_cmd_ctrl *self, const char *argv)
{
	g_serdes_param.sub_cmd = 1;
	return 0;
}

#define USEMODE_SSC_STR_MAXLEN 20
static void hikp_serdes_brief_info_print(struct cmd_serdes_param *cmd,
	const struct hilink_brief_info *data, uint32_t data_size)
{
	uint8_t ds_id;
	uint8_t start_sds_id = cmd->start_sds_id;
	uint8_t sds_num = cmd->sds_num;
	char usemode_ssc_str[USEMODE_SSC_STR_MAXLEN] = {0};
	const char *usemode_array[HILINK_USE_MODE_END] = {
		"default", "pcie", "sata", "sas", "hccs",
		"eth", "fc", "cxl", "roh", "eth_roh", "roh_eth", "ubn"
	};
	const char *ssc_type_array[HILINK_SSC_TYPE_END] = {
		"nossc", "ssc", "mssc_in", "mssc_s", "mssc_n", "mssc_w", "mssc_e"
	};

	if (data_size != sds_num) {
		printf("serdes brief info data size is wrong.\n");
		return;
	}

	for (ds_id = 0; ds_id < sds_num; ds_id++) {
		if (data[ds_id].usemode >= HILINK_USE_MODE_END) {
			printf("usemode[%u] is out of range.\n", data[ds_id].usemode);
			return;
		}
		strncpy(usemode_ssc_str, usemode_array[data[ds_id].usemode],
			sizeof(usemode_ssc_str) - 1);

		if (data[ds_id].ssc_type >= HILINK_SSC_TYPE_END) {
			printf("ssc_type[%u] is out of range.\n", data[ds_id].ssc_type);
			return;
		}
		if (data[ds_id].usemode < HILINK_USE_MODE_HCCS) {
			strncat(usemode_ssc_str, ",", HIKP_STR_BUF_LEFT_LEN(usemode_ssc_str));
			strncat(usemode_ssc_str, ssc_type_array[data[ds_id].ssc_type],
				HIKP_STR_BUF_LEFT_LEN(usemode_ssc_str));
		}
		printf("chip%u (M%u,ds%d) pll(%u, %u) pn(%u, %u) power(%u, %u)"
		       "(refclk_sel:%u) rate(%u, %u Mhz) usemode(%s)\n",
		       cmd->chip_id, cmd->macro_id,
		       ds_id + start_sds_id, data[ds_id].tx_cs_sel, data[ds_id].rx_cs_sel,
		       data[ds_id].tx_pn, data[ds_id].rx_pn,
		       data[ds_id].tx_power, data[ds_id].rx_power,
		       data[ds_id].refclk_sel, data[ds_id].tx_data_rate_mhz,
		       data[ds_id].rx_data_rate_mhz, usemode_ssc_str);
	}
}

#define KEY_INFO_TITLE								\
	("Brief Data format\n"							\
	 "FFE: [pre2 pre1 main post1 post2]\n"					\
	 "CTLE: [ALOS,LPBK,GN1,GN2,GN3,BST1,BST2,BST3,ZA1,ZA2,ZA3,SQH1,SQH2,SQH3,"	\
	 "CMBAND1,CMBAND2,CMBAND3,RMBAND1,RMBAND2,RMBAND3]\n"			\
	 "DFE:[Tap1,Tap2,Tap3,Tap4,Tap5,Tap6,Tap7,Tap8,Tap9,Tap10,Tap11,Tap12,Tap13,Tap14,Tap15," \
	 "Tap16,Tap17,Tap18,Tap19,Tap20]\n"					\
	 "FWFourEye: It only takes effect when the firmware is running and "	\
	 "continuous adaptation is turned on\n"					\
	 "Snr: [SNR_METRIC,SNR_METRIC_HIS_MIN,SNR_CYCLES,HEH,SNR_METRIC_SW]\n" \
	 "-----------------------------------------------------"		\
	 "------------------------------------------------"			\
	 "-----------------------------------------------------"		\
	 "------------------------------------------------"			\
	 "--------------------------------\n"						\
	 "               [       FFE         ]"					\
	 "[                         CTLE                            ]"		\
	 "[                                               "			\
	 "DFE                                              ]"			\
	 "[     FWFourEye     ][       Snr         ]\n"					\
	 "-----------------------------------------------------"		\
	 "-------------------------------------------------"			\
	 "-----------------------------------------------------"		\
	 "-------------------------------------------------"			\
	 "--------------------------------\n")

static void hikp_serdes_detail_info_print(struct cmd_serdes_param *cmd,
	const struct hilink_detail_info *data, uint32_t data_size)
{
	uint32_t i;
	uint8_t ds_id;
	uint8_t start_sds_id = cmd->start_sds_id;
	uint8_t sds_num = cmd->sds_num;

	if (data_size != sds_num) {
		printf("serdes detail info data size is wrong.\n");
		return;
	}
	printf(KEY_INFO_TITLE);
	for (ds_id = 0; ds_id < sds_num; ds_id++) {
		printf("chip%u (M%u,ds%d) [%3d,%3d,%3u,%3d,%3d]",
		       cmd->chip_id, cmd->macro_id, ds_id + start_sds_id,
		       data[ds_id].tx_cfg.fir_pre2, data[ds_id].tx_cfg.fir_pre1,
		       data[ds_id].tx_cfg.fir_main, data[ds_id].tx_cfg.fir_post1,
		       data[ds_id].tx_cfg.fir_post2);
		printf("[%u,%u", data[ds_id].alos_status, data[ds_id].loopback_type);
		for (i = 0; i < HILINK_SERDES_RX_PARA_COUNT; i++)
			printf(",%2u", data[ds_id].rx_ctle_cfg.data[i]);

		printf("][");
		for (i = 0; i < HILINK_SERDES_RX_TAP_COUNT; i++)
			printf("%3d,", data[ds_id].rx_tap_cfg.tap_value[i]);

		printf("][%4d,%4d,%4d,%4d]",
		       data[ds_id].eye_diagram.top, data[ds_id].eye_diagram.bottom,
		       data[ds_id].eye_diagram.left, data[ds_id].eye_diagram.right);
		/* 0: SNR_METRIC, 1:SNR_METRIC_HIS_MIN */
		printf("[%3d,%3d,%3d,%3d,%3d]\n", data[ds_id].snr_para[0], data[ds_id].snr_para[1],
		       /* 2:SNR_CYCLES, 3:HEH, 4:SNR_METRIC_SW */
		       data[ds_id].snr_para[2], data[ds_id].snr_para[3], data[ds_id].snr_para[4]);
	}
}

static void hikp_serdes_logout_init(struct hilink_cmd_out *logout, char *buffer,
				    uint32_t size, uint32_t type)
{
	memset(buffer, 0, size);

	logout->str_len = size;
	logout->result_offset = 0;
	logout->out_str = buffer;
	logout->type = type;
	logout->ret_val = 0;
}

static int hikp_serdes_info_para_check(struct major_cmd_ctrl *self)
{
	if (g_serdes_param.chip_id == 0xff) {
		self->err_no = -EINVAL;
		snprintf(self->err_str, sizeof(self->err_str), "Need chip id.");
		return self->err_no;
	}

	if (g_serdes_param.macro_id == 0xff) {
		self->err_no = -EINVAL;
		snprintf(self->err_str, sizeof(self->err_str), "Need macro id.");
		return self->err_no;
	}

	if (g_serdes_param.start_sds_id == 0xff) {
		self->err_no = -EINVAL;
		snprintf(self->err_str, sizeof(self->err_str), "Need lane id.");
		return self->err_no;
	}

	if (g_serdes_param.sds_num == 0xff) {
		self->err_no = -EINVAL;
		snprintf(self->err_str, sizeof(self->err_str), "Need lane num.");
		return self->err_no;
	}

	if (g_serdes_param.sub_cmd == 0xff)
		g_serdes_param.sub_cmd = 0;

	return 0;
}

static void hikp_serdes_print(struct cmd_serdes_param *cmd)
{
	if (cmd->cmd_type == SERDES_KEY_INFO)
		hikp_serdes_info_print(cmd);
	else if (cmd->cmd_type == SERDES_DUMP_REG)
		hikp_serdes_dump_print(cmd);
}

int hikp_serdes_get_reponse(struct cmd_serdes_param *cmd)
{
	struct hikp_cmd_header req_header = {0};
	struct hikp_cmd_ret *cmd_ret;
	struct hilink_cmd_in hilink_cmd = {0};
	size_t out_out_header_size;

	hilink_cmd.cmd_type              = cmd->cmd_type;
	hilink_cmd.sub_cmd               = cmd->sub_cmd;
	hilink_cmd.cmd_para.chip_id      = cmd->chip_id;
	hilink_cmd.cmd_para.macro_id     = cmd->macro_id;
	hilink_cmd.cmd_para.start_sds_id = cmd->start_sds_id;
	hilink_cmd.cmd_para.sds_num      = cmd->sds_num;

	hikp_serdes_logout_init(&g_out_put, g_serdes_data_out_buf, SERDES_OUTPUT_MAX_SIZE, 0);

	hikp_cmd_init(&req_header, SERDES_MOD, cmd->cmd_type, cmd->sub_cmd);
	cmd_ret = hikp_cmd_alloc(&req_header, &hilink_cmd, sizeof(hilink_cmd));
	if (cmd_ret == NULL || cmd_ret->status != 0) {
		printf("hikp_cmd_alloc err.\n");
		hikp_cmd_free(&cmd_ret);
		return -EINVAL;
	}
	out_out_header_size = sizeof(g_out_put.str_len) + sizeof(g_out_put.result_offset) +
			      sizeof(g_out_put.type) + sizeof(g_out_put.ret_val);
	memcpy(&g_out_put, cmd_ret->rsp_data, out_out_header_size);

	if ((cmd_ret->rsp_data_num * sizeof(uint32_t) - out_out_header_size) > SERDES_OUTPUT_MAX_SIZE) {
		printf("serdes_info rsp_data data copy size error, data size:0x%zx max size:0x%x.",
			(cmd_ret->rsp_data_num * sizeof(uint32_t) - out_out_header_size),
			SERDES_OUTPUT_MAX_SIZE);
		hikp_cmd_free(&cmd_ret);
		return -EINVAL;
	}
	memcpy(g_out_put.out_str, cmd_ret->rsp_data + out_out_header_size / sizeof(uint32_t),
		cmd_ret->rsp_data_num * sizeof(uint32_t) - out_out_header_size);
	hikp_cmd_free(&cmd_ret);

	hikp_serdes_print(cmd);

	return 0;
}

static void hikp_serdes_info_print(struct cmd_serdes_param *cmd)
{
	struct hilink_brief_info *brief_info_data = NULL;
	struct hilink_detail_info *detail_info_data = NULL;

	if (cmd->sub_cmd > 0) {
		detail_info_data = (struct hilink_detail_info *)g_out_put.out_str;
		hikp_serdes_detail_info_print(cmd, detail_info_data,
			g_out_put.result_offset / sizeof(struct hilink_detail_info));
	} else {
		brief_info_data = (struct hilink_brief_info *)g_out_put.out_str;
		hikp_serdes_brief_info_print(cmd, brief_info_data,
			g_out_put.result_offset / sizeof(struct hilink_brief_info));
	}
}

static void hikp_serdes_info_cmd_execute(struct major_cmd_ctrl *self)
{
	int ret;

	ret = hikp_serdes_info_para_check(self);
	if (ret != 0)
		return;

	g_serdes_param.cmd_type = SERDES_KEY_INFO;
	ret = hikp_serdes_get_reponse(&g_serdes_param);
	if (ret != 0) {
		self->err_no = ret;
		snprintf(self->err_str, sizeof(self->err_str), "serdes_info hikp_serdes_get_reponse err\n");
		return;
	}
}

static void cmd_serdes_maininfo_init(void)
{
	struct major_cmd_ctrl *major_cmd = get_major_cmd();

	major_cmd->option_count = 0;
	major_cmd->execute = hikp_serdes_info_cmd_execute;

	cmd_option_register("-h", "--help",          false, cmd_serdes_maininfo_help);
	cmd_option_register("-i", "--chipid",        true,  cmd_serdes_chipid);
	cmd_option_register("-s", "--start_lane_id", true,  cmd_serdes_start_lane_id);
	cmd_option_register("-n", "--lane_num",      true,  cmd_serdes_lane_num);
	cmd_option_register("-k", "--key_info",      false, cmd_serdes_key_info_pro);
}

static int cmd_serdes_dump_help(struct major_cmd_ctrl *self, const char *argv)
{
	printf("\n  Usage: %s %s\n", self->cmd_ptr->name,
	       "-c <subcmd> -i <chipid> -s <start_lane_id>");
	printf("\n         %s, e.g. hikptool serdes_dump -c cs -i 0 -s m0d0\n",
	       self->cmd_ptr->help_info);
	printf("\n  Options:\n\n"
	       "  -h    --help           display the help info\n"
	       "  -i    --chipid         chipid, usage: -i [chip_id], e.g. -i 0\n"
	       "  -s    --start_lane_id  the start of lane id, "
	       "usage: -s m[macro_id]d[lane_id], e.g. -s m3d0\n"
	       "  -c    --subcmd         subcmd, usage: -c [subcmd], e.g. -c cs/ds/subctrl\n");
	return 0;
}

static int cmd_serdes_dump_subcmds(struct major_cmd_ctrl *self, const char *argv)
{
	const char *set_cmds[HILINK_DUMP_TYPE_END] = { "cs", "ds", "csds", "ram", "subctrl"};
	uint8_t i;

	for (i = 0; i < HILINK_DUMP_TYPE_END; i++) {
		if (strcmp(argv, set_cmds[i]) == 0)
			goto _SERDES_DUMP_SUBCMD_PRO_;
	}

	snprintf(self->err_str, sizeof(self->err_str), "Invalid dump subcmd.");
	self->err_no = -EINVAL;
	return -EINVAL;

_SERDES_DUMP_SUBCMD_PRO_:
	g_serdes_param.sub_cmd = (enum hilink_dump_type_e)i;
	return 0;
}

static void hikp_serdes_dump_print(struct cmd_serdes_param *cmd)
{
	uint32_t *dump_data = (uint32_t *)g_out_put.out_str;
	uint32_t data_size = g_out_put.result_offset / sizeof(uint32_t);
	uint32_t i;

	if (g_out_put.type == 1) { /* 0:data; 1:string */
		printf("serdes dump data type is string, buffer is not enough.\n");
		return;
	}

	/* 2: Check whether addresses and values are paired */
	if (data_size == 0 || data_size % 2 != 0) {
		printf("serdes dump data size is wrong.\n");
		return;
	}

	printf("\n[-------Macro%uCS/DS%u-------]\nAddr   Value",
		cmd->macro_id, cmd->start_sds_id);
	for (i = 0; i < data_size; i += 2) { /* 2: Addresses and values are paired */
		printf("\n0x%04x 0x%08x", dump_data[i], dump_data[i + 1]);
	}
	printf("\n");
}

static int hikp_serdes_dump_para_check(struct major_cmd_ctrl *self)
{
	if (g_serdes_param.sub_cmd == 0xff) {
		self->err_no = -EINVAL;
		snprintf(self->err_str, sizeof(self->err_str), "Need subcmd.");
		return self->err_no;
	}

	if (g_serdes_param.chip_id == 0xff) {
		self->err_no = -EINVAL;
		snprintf(self->err_str, sizeof(self->err_str), "Need chip id.");
		return self->err_no;
	}

	if (g_serdes_param.macro_id == 0xff) {
		self->err_no = -EINVAL;
		snprintf(self->err_str, sizeof(self->err_str), "Need macro id.");
		return self->err_no;
	}

	if (g_serdes_param.start_sds_id == 0xff) {
		self->err_no = -EINVAL;
		snprintf(self->err_str, sizeof(self->err_str), "Need lane id.");
		return self->err_no;
	}

	return 0;
}

static void hikp_serdes_dump_cmd_execute(struct major_cmd_ctrl *self)
{
	int ret;

	ret = hikp_serdes_dump_para_check(self);
	if (ret != 0)
		return;

	g_serdes_param.cmd_type = SERDES_DUMP_REG;
	ret = hikp_serdes_get_reponse(&g_serdes_param);
	if (ret != 0) {
		self->err_no = ret;
		snprintf(self->err_str, sizeof(self->err_str), "serdes_dump hikp_serdes_get_reponse err\n");
		return;
	}
}

static void cmd_serdes_dump_init(void)
{
	struct major_cmd_ctrl *major_cmd = get_major_cmd();

	major_cmd->option_count = 0;
	major_cmd->execute = hikp_serdes_dump_cmd_execute;

	cmd_option_register("-h", "--help",          false, cmd_serdes_dump_help);
	cmd_option_register("-c", "--subcmd",        true,  cmd_serdes_dump_subcmds);
	cmd_option_register("-i", "--chipid",        true,  cmd_serdes_chipid);
	cmd_option_register("-s", "--start_lane_id", true,  cmd_serdes_start_lane_id);
}

HIKP_CMD_DECLARE("serdes_dump", "serdes_dump cmd", cmd_serdes_dump_init);
HIKP_CMD_DECLARE("serdes_info", "serdes_info cmd", cmd_serdes_maininfo_init);
