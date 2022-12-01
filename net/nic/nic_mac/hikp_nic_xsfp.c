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
#include <arpa/inet.h>
#include "tool_cmd.h"
#include "hikptdev_plug.h"
#include "hikp_mac_cmd.h"
#include "hikp_nic_xsfp.h"

static struct hikp_xsfp_ctrl g_xsfp_dump = {0};
static struct sff_ext_comp g_sff_ext_spec_comp[] = {
	{0x00, "Unspecified"},
	{0x01, "100G AOC (Active Optical Cable) or 25GAUI C2M AOC"},
	{0x02, "100GBASE-SR4 or 25GBASE-SR"},
	{0x03, "100GBASE-LR4 or 25GBASE-LR"},
	{0x04, "100GBASE-ER4 or 25GBASE-ER"},
	{0x05, "100GBASE-SR10"},
	{0x06, "100G CWDM4"},
	{0x07, "100G PSM4 Parallel SMF"},
	{0x08, "100G ACC (Active Copper Cable) or 25GAUI C2M ACC"},
	{0x0B, "100GBASE-CR4, 25GBASE-CR CA-25G-L or 50GBASE-CR2 with RS (Clause91) FEC"},
	{0x0C, "25GBASE-CR CA-25G-S or 50GBASE-CR2 with BASE-R (Clause 74 Fire code) FEC"},
	{0x0D, "25GBASE-CR CA-25G-N or 50GBASE-CR2 with no FEC"},
	{0x10, "40GBASE-ER4"},
	{0x11, "4x10GBASE-SR"},
	{0x12, "40G PSM4 Parallel SMF"},
	{0x16, "10GBASE-T with SFI electrical interface"},
	{0x17, "100G CLR4"},
	{0x18, "100G AOC or 25GAUI C2M AOC"},
	{0x19, "100G ACC or 25GAUI C2M ACC"},
	{0x1A, "100GE-DWDM2"},
	{0x1C, "10GBASE-T Short Reach (30 meters)"},
	{0x21, "100G PAM4 BiDi"},
	{0x25, "100GBASE-DR"},
	{0x26, "100G-FR or 100GBASE-FR1"},
	{0x27, "100G-LR or 100GBASE-LR1"},
	{0x40, "50GBASE-CR, 100GBASE-CR2, or 200GBASE-CR4"},
	{0x41, "50GBASE-SR, 100GBASE-SR2, or 200GBASE-SR4"},
	{0x42, "50GBASE-FR or 200GBASE-DR4"},
	{0x43, "200GBASE-FR4"},
	{0x44, "200G 1550 nm PSM4"},
	{0x45, "50GBASE-LR"},
	{0x46, "200GBASE-LR4"},
	{0x4A, "50GBASE-ER"},
};

static int hikp_xsfp_get_cmd_data(struct hikp_cmd_ret **cmd_resp, uint32_t sub_cmd, uint32_t blk_id)
{
	struct hikp_xsfp_req req = {0};
	struct hikp_cmd_header req_header = {0};

	req.bdf = g_xsfp_dump.target.bdf;
	req.blk_id = blk_id;
	hikp_cmd_init(&req_header, MAC_MOD, MAC_CMD_DUMP_XSFP, sub_cmd);
	*cmd_resp = hikp_cmd_alloc(&req_header, &req, sizeof(req));
	if (*cmd_resp == NULL)
		return -ENOSPC;

	if ((*cmd_resp)->status != 0)
		return -EAGAIN;

	return 0;
}

static void hikp_xsfp_dump_hex(const uint8_t *data, uint32_t size)
{
	uint32_t i;

	printf("-----------------show eeprom raw data------------------\n");
	printf("        0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f\n");
	for (i = 0; i < size; i++) {
		if ((i % XSFP_PRINT_COL) == 0)
			printf("0x%03x: ", i);

		printf("%02x ", data[i]);
		if ((i % XSFP_PRINT_COL) == (XSFP_PRINT_COL - 1))
			printf("\n");

		if ((i % XSFP_PRINT_BLK) == (XSFP_PRINT_BLK - 1))
			printf("-------------------------------------------------------\n");
	}
}

static void sff_print_ext_comp_code(const uint8_t ext_comp)
{
	uint32_t i;
	size_t size = HIKP_ARRAY_SIZE(g_sff_ext_spec_comp);

	for (i = 0; i < size; i++) {
		if (ext_comp == g_sff_ext_spec_comp[i].val)
			printf("%-24s: %s\n", "ext_spec_compliance",
			       g_sff_ext_spec_comp[i].module_cap);
	}
}

static void xsfp_print_data(const char *str, uint32_t len, const uint8_t *data, uint32_t type)
{
	uint32_t i;

	printf("%-24s: ", str);
	for (i = 0; i < len; i++) {
		if (type == PRINT_ASCII)
			printf("%c", data[i]);
		else
			printf("0x%02x ", data[i]);
	}
	printf("\n");
}

static void xsfp_show_mask(const uint8_t val, const struct sff_comp_info *table, size_t size)
{
	bool flag = false;
	uint32_t i;

	for (i = 0; i < size; i++) {
		if ((val & table[i].mask) != 0) {
			if (flag) {
				printf("%s ", table[i].description);
			} else {
				printf("%-24s: %s ", "transceiver_compliance",
				       table[i].description);
				flag = true;
			}
		}
	}
	if (flag)
		printf("\n");
}

static void sfp_print_trans_comp_code(const struct sfp_page_info *info)
{
	const struct sff_comp_info sff_10g_comp[] = {
		{SFP_10GBASE_SR_MASK, "10GBASE-SR"},    {SFP_10GBASE_LR_MASK, "10GBASE-LR"},
		{SFP_10GBASE_LRM_MASK, "10GBASE-LRM"},  {SFP_10GBASE_ER_MASK, "10GBASE-ER"},
	};
	const struct sff_comp_info sff_comp[] = {
		{SFP_1000BASE_SX_MASK, "1000BASE-SX"}, {SFP_1000BASE_LX_MASK, "1000BASE-LX"},
		{SFP_1000BASE_CX_MASK, "1000BASE-CX"}, {SFP_1000BASE_T_MASK, "1000BASE-T"},
		{SFP_100BASE_LX_LX10_MASK, "100BASE-LX/LX10"}, {SFP_100BASE_FX_MASK, "100BASE-FX"},
		{SFP_BASE_BX10_MASK, "BASE-BX10"}, {SFP_BASE_PX_MASK, "BASE-PX"},
	};
	uint32_t i;

	printf("%-24s: ", "transceiver_code");
	for (i = 0; i < SFF_TRANCEIV_LEN; i++)
		printf("0x%02x ", info->page_a0.transceiver[i]);

	printf("| ext: 0x%02x\n", info->page_a0.transceiver_code);

	xsfp_show_mask(info->page_a0.transceiver[0], sff_10g_comp, HIKP_ARRAY_SIZE(sff_10g_comp));
	xsfp_show_mask(info->page_a0.transceiver[3U], sff_comp, HIKP_ARRAY_SIZE(sff_comp));

	sff_print_ext_comp_code(info->page_a0.transceiver_code);
}

static void sfp_print_base_id_info(const struct sfp_page_info *info)
{
	printf("%-24s: 0x%02x\n", "identifier", info->page_a0.identifier);
	printf("%-24s: 0x%02x\n", "ext_identifier", info->page_a0.ext_identifier);
	printf("%-24s: 0x%02x\n", "connector", info->page_a0.connector);
	printf("%-24s: 0x%02x\n", "encoding", info->page_a0.encoding);
	printf("%-24s: %u (100MBd)\n", "br_nominal", info->page_a0.br_nominal);
	printf("%-24s: 0x%02x\n", "rate_identifier", info->page_a0.rate_identifier);
	printf("%-24s: %u (km)\n", "smf_len_km", info->page_a0.len_smf_km);
	printf("%-24s: %u (100m)\n", "smf_len", info->page_a0.len_smf);
	printf("%-24s: %u (10m)\n", "smf_om2_len", info->page_a0.len_smf_om2);
	printf("%-24s: %u (10m)\n", "smf_om1_len", info->page_a0.len_smf_om1);
	printf("%-24s: %u (m)\n", "cable_len", info->page_a0.len_cable);
	printf("%-24s: %u (10m)\n", "om3_len", info->page_a0.len_om3);
	printf("%-24s: %d (nm)\n", "wavelength",
	       ((uint16_t)info->page_a0.wave_leng[0] << 8U) | (uint16_t)info->page_a0.wave_leng[1]);

	xsfp_print_data("vendor_name", VEND_NAME_LEN, info->page_a0.vendor_name, PRINT_ASCII);
	xsfp_print_data("vendor_oui", VEND_OUI_LEN, info->page_a0.vendor_oui, PRINT_HEX);
	xsfp_print_data("vendor_pn", VEND_PN_LEN, info->page_a0.vendor_pn, PRINT_ASCII);
	xsfp_print_data("vendor_rev", SFP_VEND_REV_LEN, info->page_a0.vendor_rev, PRINT_ASCII);
	xsfp_print_data("vendor_sn", VEND_SN_LEN, info->page_a0.vendor_sn, PRINT_ASCII);
	xsfp_print_data("vendor_date_code", VEND_DATE_CODE_LEN,
			info->page_a0.date_code, PRINT_ASCII);

	printf("%-24s: 0x%02x(%s) support: 0x%02x(%s)\n", "tx_disable",
	       info->page_a2.soft_tx_dis_select,
	       (info->page_a2.soft_tx_dis_select == 0) ? "No" : "Yes",
	       info->page_a0.tx_disable_imp, (info->page_a0.tx_disable_imp == 0) ? "No" : "Yes");

	sfp_print_trans_comp_code(info);
}

static void sfp_print_dom_info(const struct sfp_page_info *info)
{
	/* if not implement diagnostic check or copper not support read power and temperature */
	if (info->page_a0.ddm_imp == 0 || info->page_a0.connector == SFF_CONNECTOR_COPPER)
		return;

	printf("%-24s: %d uW\n", "tx_power",
		((uint16_t)info->page_a2.tx_power[0] << 8U) | (uint16_t)info->page_a2.tx_power[1]);
	printf("%-24s: %d uW\n", "rx_power",
		((uint16_t)info->page_a2.rx_power[0] << 8U) | (uint16_t)info->page_a2.rx_power[1]);
	printf("%-24s: %d.%02u\n", "temperature",
	       (int8_t)info->page_a2.temperature[0], info->page_a2.temperature[1]);
}

static void hikp_show_sfp_info(const uint8_t *data, uint32_t size)
{
	struct sfp_page_info *sfp_data = (struct sfp_page_info *)data;

	printf("------------------------show sfp info-------------------------\n");
	sfp_print_base_id_info(sfp_data);
	sfp_print_dom_info(sfp_data);
	printf("--------------------------------------------------------------\n");
}

static void qsfp_print_trans_comp_code(const struct qsfp_page0_info *info)
{
	const struct sff_comp_info sff_40g_comp[] = {
		{QSFP_40G_ACT_CABLE_MASK, "40G Active Cable (XLPPI)"},
		{QSFP_40GBASE_LR4_MASK, "40GBASE-LR4"},
		{QSFP_40GBASE_SR4_MASK, "40GBASE-SR4"}, {QSFP_40GBASE_CR4_MASK, "40GBASE-CR4"},
		{QSFP_10GBASE_SR_MASK, "10GBASE-SR"}, {QSFP_10GBASE_LR_MASK, "10GBASE-LR"},
		{QSFP_10GBASE_LRM_MASK, "10GBASE-LRM"},
	};
	const struct sff_comp_info sff_comp[] = {
		{QSFP_1000BASE_SX_MASK, "1000BASE-SX"}, {QSFP_1000BASE_LX_MASK, "1000BASE-LX"},
		{QSFP_1000BASE_CX_MASK, "1000BASE-CX"}, {QSFP_1000BASE_T_MASK, "1000BASE-T"},
	};
	uint32_t i;

	printf("%-24s: ", "transceiver_code");
	for (i = 0; i < SFF_TRANCEIV_LEN; i++)
		printf("0x%02x ", info->page_upper.spec_compliance[i]);

	printf("| ext: 0x%02x\n", info->page_upper.link_codes);

	xsfp_show_mask(info->page_upper.spec_compliance[0],
		       sff_40g_comp, HIKP_ARRAY_SIZE(sff_40g_comp));
	xsfp_show_mask(info->page_upper.spec_compliance[3U],
		       sff_comp, HIKP_ARRAY_SIZE(sff_comp));

	sff_print_ext_comp_code(info->page_upper.link_codes);
}

static void qsfp_print_base_id_info(const struct qsfp_page0_info *info)
{
	printf("%-24s: 0x%02x\n", "identifier", info->page_upper.identifier);
	printf("%-24s: 0x%02x\n", "connector", info->page_upper.connector);
	printf("%-24s: 0x%02x\n", "encoding", info->page_upper.encoding);
	printf("%-24s: %u (100MBd)\n", "br_nominal", info->page_upper.br_nominal);
	printf("%-24s: 0x%02x\n", "rate_identifier", info->page_upper.ext_rate_sel);
	printf("%-24s: %u (km)\n", "smf_len_km", info->page_upper.smf_len_km);
	printf("%-24s: %u (m)\n", "om3_len",
	       (uint32_t)(info->page_upper.om3_len * QSFP_OM3_LEN_UNIT));
	printf("%-24s: %u (m)\n", "om2_len", info->page_upper.om2_len);
	printf("%-24s: %u (m)\n", "om1_len", info->page_upper.om1_len);
	printf("%-24s: %u (m)\n", "om4_cable_copper_len", info->page_upper.om4_cable_copper_len);
	printf("%-24s: %d (nm)\n", "wavelength",
	       (((uint16_t)info->page_upper.wavelength[0] << 8U) |
	       (uint16_t)info->page_upper.wavelength[1]) / QSFP_WAVE_LEN_DIV);
	printf("%-24s: %d (nm)\n", "wavelength_tolerance",
		(((uint16_t)info->page_upper.wavelength_tolerance[0] << 8U) |
		(uint16_t)info->page_upper.wavelength_tolerance[1]) / QSFP_TOL_WAVE_LEN_DIV);
	printf("%-24s: 0x%02x\n", "device_technology", info->page_upper.device_technology);

	xsfp_print_data("vendor_name", VEND_NAME_LEN, info->page_upper.vendor_name, PRINT_ASCII);
	xsfp_print_data("vendor_oui", VEND_OUI_LEN, info->page_upper.vendor_oui, PRINT_HEX);
	xsfp_print_data("vendor_pn", VEND_PN_LEN, info->page_upper.vendor_pn, PRINT_ASCII);
	xsfp_print_data("vendor_rev", QSFP_VEND_REV_LEN, info->page_upper.vendor_rev, PRINT_ASCII);
	xsfp_print_data("vendor_sn", VEND_SN_LEN, info->page_upper.vendor_sn, PRINT_ASCII);
	xsfp_print_data("vendor_date_code", VEND_DATE_CODE_LEN,
			info->page_upper.date_code, PRINT_ASCII);

	printf("%-24s: 0x%02x(%s) support: 0x%02x(%s)\n", "high_power",
	       info->page_lower.high_pw_en, (info->page_lower.high_pw_en == 0) ? "No" : "Yes",
	       info->page_upper.pw_class_5_7, (info->page_upper.pw_class_5_7 == 0) ? "No" : "Yes");
	printf("%-24s: 0x%02x(%s) support: 0x%02x(%s)\n", "tx_disable",
	       info->page_lower.tx_disable, (info->page_lower.tx_disable == 0) ? "No" : "Yes",
	       info->page_upper.tx_dis_imp, (info->page_upper.tx_dis_imp == 0) ? "No" : "Yes");

	qsfp_print_trans_comp_code(info);
}

static void qsfp_print_channel_power(const char *str, const uint8_t *power_data)
{
	uint32_t i;

	for (i = 0; i < QSFP_CHAN_NUM; i++)
		printf("%s%-16u: %u uW\n", str, i,
		       ntohs(*((unsigned short *)&(power_data[i * 2U]))) / 10U);
}

static void qsfp_print_dom_info(const struct qsfp_page0_info *info)
{
	/* from 0xa to 0xf is copper, not support read power and temperature */
	if ((info->page_upper.device_technology >> QSFP_TRANS_TECH_BIT) > QSFP_TRANS_OPTICAL_MAX)
		return;

	qsfp_print_channel_power("tx_power", info->page_lower.tx_power);
	qsfp_print_channel_power("rx_power", info->page_lower.rx_power);
	printf("%-24s: %d.%02u\n", "temperature",
	       (int8_t)info->page_lower.temperature_msb, info->page_lower.temperature_lsb);
}

static void hikp_show_qsfp_info(const uint8_t *data, uint32_t size)
{
	struct qsfp_page0_info *qsfp_data = (struct qsfp_page0_info *)(data);

	printf("------------------------show qsfp info------------------------\n");
	qsfp_print_base_id_info(qsfp_data);
	qsfp_print_dom_info(qsfp_data);
	printf("--------------------------------------------------------------\n");
}

static void hikp_xsfp_parse_info(const uint8_t *data, uint32_t size)
{
	if (data[SFF_ID_OFFSET] == ID_SFP) {
		hikp_show_sfp_info(data, size);
	} else if (data[SFF_ID_OFFSET] == ID_QSFP ||
		   data[SFF_ID_OFFSET] == ID_QSFP_PLUS ||
		   data[SFF_ID_OFFSET] == ID_QSFP28) {
		hikp_show_qsfp_info(data, size);
	} else {
		/* unknown type just dump hex data */
		hikp_xsfp_dump_hex(data, size);
	}
}

static int hikp_xsfp_get_raw_data(uint8_t *buf, uint32_t size, uint32_t blk_num)
{
	struct hikp_cmd_ret *cmd_resp = NULL;
	uint32_t left_size = size;
	uint32_t offset = 0;
	uint32_t len = 0;
	int ret;
	uint32_t i;

	for (i = 0; i < blk_num && left_size > 0; i++) {
		ret = hikp_xsfp_get_cmd_data(&cmd_resp, NIC_XSFP_GET_EEPROM_DATA, i);
		if (ret != 0) {
			HIKP_ERROR_PRINT("get optical module eeprom data failed\n");
			free(cmd_resp);
			cmd_resp = NULL;
			return ret;
		}

		len = HIKP_MIN(left_size, (cmd_resp->rsp_data_num * sizeof(uint32_t)));
		memcpy(buf + offset, (uint8_t *)(cmd_resp->rsp_data), len);
		left_size -= len;
		offset += len;

		/* current cmd interaction is complete, so free cmd_buf */
		free(cmd_resp);
		cmd_resp = NULL;
	}

	return 0;
}

static void hikp_xsfp_get_eeprom_data(struct major_cmd_ctrl *self, uint32_t size, uint32_t blk_num)
{
	uint8_t *raw_data = (uint8_t *)calloc(1, size * sizeof(uint8_t));
	int ret;

	if (!raw_data) {
		self->err_no = -ENOSPC;
		HIKP_ERROR_PRINT("no space left for dump eeprom data\n");
		return;
	}

	ret = hikp_xsfp_get_raw_data(raw_data, size, blk_num);
	if (ret != 0)
		goto ERR_OUT;

	if ((g_xsfp_dump.dump_param & XSFP_RAW_DATA_BIT) != 0)
		hikp_xsfp_dump_hex(raw_data, size);
	else
		hikp_xsfp_parse_info(raw_data, size);

ERR_OUT:
	free(raw_data);
	raw_data = NULL;
}

static int hikp_xsfp_dump_pre_check(const struct hikp_xsfp_basic *info)
{
	if (info->media_type != MEDIA_TYPE_FIBER) {
		printf("port media type %u not support get optical module info\n",
		       info->media_type);
		return -EOPNOTSUPP;
	}

	if (info->present_status == MODULE_ABSENT) {
		printf("port optical module not present, cannot get module info\n");
		return -ENXIO;
	}

	if (info->data_size < SFF_XSFP_DATA_LEN) {
		printf("get data size %u less than xsfp min size %u\n",
		       info->data_size, SFF_XSFP_DATA_LEN);
		return -EPERM;
	}

	return 0;
}

static void hikp_xsfp_get_info(struct major_cmd_ctrl *self)
{
	struct hikp_xsfp_basic *info = NULL;
	struct hikp_cmd_ret *cmd_resp = NULL;
	int ret;

	if ((g_xsfp_dump.dump_param & XSFP_TARGET_BIT) == 0) {
		self->err_no = -EINVAL;
		snprintf(self->err_str, sizeof(self->err_str), "need port id");
		return;
	}

	/* first get port basic info */
	ret = hikp_xsfp_get_cmd_data(&cmd_resp, NIC_XSFP_GET_BASIC_INFO, 0);
	if (ret != 0) {
		HIKP_ERROR_PRINT("get port basic info failed\n");
		self->err_no = ret;
		goto ERR_OUT;
	}

	info = (struct hikp_xsfp_basic *)(cmd_resp->rsp_data);
	ret = hikp_xsfp_dump_pre_check(info);
	if (ret != 0) {
		self->err_no = ret;
		goto ERR_OUT;
	}

	hikp_xsfp_get_eeprom_data(self, info->data_size, info->total_blk_num);

ERR_OUT:
	free(cmd_resp);
	cmd_resp = NULL;
}

static int hikp_xsfp_show_help(struct major_cmd_ctrl *self, const char *argv)
{
	printf("\n  Usage: %s %s\n", self->cmd_ptr->name, "-i <interface> [-d]");
	printf("\n         %s\n", self->cmd_ptr->help_info);
	printf("\n  Options:\n\n");
	printf("    %s, %-25s %s\n", "-h", "--help", "display this help and exit");
	printf("    %s, %-25s %s\n", "-i", "--interface=<interface>",
	       "device target or bdf id, e.g. eth0~3 or 0000:35:00.0");
	printf("    %s, %-25s %s\n", "-d", "--dump=<dump>", "dump optical module eeprom raw data");
	printf("\n");

	return 0;
}

static int hikp_xsfp_get_target(struct major_cmd_ctrl *self, const char *argv)
{
	self->err_no = tool_check_and_get_valid_bdf_id(argv, &g_xsfp_dump.target);
	if (self->err_no) {
		snprintf(self->err_str, sizeof(self->err_str), "Unknown device %s.", argv);
		return self->err_no;
	}

	g_xsfp_dump.dump_param |= XSFP_TARGET_BIT;

	return 0;
}

static int hikp_xsfp_dump_raw_data(struct major_cmd_ctrl *self, const char *argv)
{
	g_xsfp_dump.dump_param |= XSFP_RAW_DATA_BIT;

	return 0;
}

static void cmd_get_xsfp_info(void)
{
	struct major_cmd_ctrl *major_cmd = get_major_cmd();

	major_cmd->option_count = 0;
	major_cmd->execute = hikp_xsfp_get_info;

	cmd_option_register("-h", "--help",         false,  hikp_xsfp_show_help);
	cmd_option_register("-i", "--interface",    true,   hikp_xsfp_get_target);
	cmd_option_register("-d", "--dump",         false,  hikp_xsfp_dump_raw_data);
}

HIKP_CMD_DECLARE("nic_xsfp", "query port optical module information", cmd_get_xsfp_info);
