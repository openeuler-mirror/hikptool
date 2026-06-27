/*
 * Copyright (c) 2026 Hisilicon Technologies Co., Ltd.
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
#include "hikp_optical_dom.h"
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include "tool_lib.h"
#include "tool_cmd.h"
#include "hikptdev_plug.h"

static struct optical_dom_param g_optical_dom_param;

static const struct dump_annotation g_dump_annotations[] = {
	{ DUMP_ANNOTATION_PAGE_LOWER, 0, "[Identifier/Rev/MemoryModel]" },
	{ DUMP_ANNOTATION_PAGE_LOWER, 14, "[Temperature/Voltage]" },
	{ DUMP_ANNOTATION_PAGE_LOWER, 85, "[Module Type/Media]" },
	{ OPTICAL_DOM_PAGE_00H, 128, "[Vendor Name(0x81-0x90)]" },
	{ OPTICAL_DOM_PAGE_00H, 144, "[Vendor OUI(0x91-0x93)]" },
	{ OPTICAL_DOM_PAGE_00H, 160, "[Vendor PN end(0xA0-0xA3)/Rev(0xA4-0xA5)]" },
	{ OPTICAL_DOM_PAGE_00H, 176, "[Vendor SN end(0xB0-0xB5)/Date Code(0xB6-0xBC)]" },
	{ OPTICAL_DOM_PAGE_00H, 192, "[CLEI(0xBE-0xC7)/Power Class(0xC8)/Max Power(0xC9)]" },
	{ OPTICAL_DOM_PAGE_00H, 208, "[Media Intf Tech(0xD4)]" },
	{ OPTICAL_DOM_PAGE_01H, 144, "[Diag Type(0x97)/Diag Flags(0x9D-0x9E)]" },
	{ OPTICAL_DOM_PAGE_01H, 160, "[Diag Chan Adver/bias_mul(0xA0)]" },
	{ OPTICAL_DOM_PAGE_02H, 128, "[Temp thresh: HALRM/LALRM/HWARN/LWARN(0x80-0x87)]" },
	{ OPTICAL_DOM_PAGE_02H, 136, "[Volt thresh: HALRM/LALRM/HWARN/LWARN(0x88-0x8F)]" },
	{ OPTICAL_DOM_PAGE_02H, 176, "[TX PWR thresh(0xB0-0xB7)/TX Bias thresh(0xB8-0xBF)]" },
	{ OPTICAL_DOM_PAGE_02H, 192, "[RX PWR thresh(0xC0-0xC7)]" },
	{ OPTICAL_DOM_PAGE_11H, 128, "[TX Fail(0x87)/TX LOS(0x88)/TX LOL(0x89)]" },
	{ OPTICAL_DOM_PAGE_11H, 144, "[RX LOS(0x93)/RX LOL(0x94)]" },
	{ OPTICAL_DOM_PAGE_11H, 160, "[TX Bias monitors(0xAA-0xB9)]" },
	{ OPTICAL_DOM_PAGE_11H, 176, "[RX Power monitors(0xBA-0xC9)]" },
};

static const char *page_header_name(uint8_t page, uint8_t bank)
{
	static __thread char buf[64]; /* enough for "Page XXh (Bank YY) Slot Z (0x80-0xFF)" */

	switch (page) {
	case DUMP_ANNOTATION_PAGE_LOWER:
		return "Lower Page (0x00-0x7F)";
	case OPTICAL_DOM_PAGE_00H:
		snprintf(buf, sizeof(buf),
			 "Page 00h (Bank %u) Slot 0 (0x80-0xFF)", bank);
		return buf;
	case OPTICAL_DOM_PAGE_01H:
		snprintf(buf, sizeof(buf),
			 "Page 01h (Bank %u) Slot 1 (0x80-0xFF)", bank);
		return buf;
	case OPTICAL_DOM_PAGE_02H:
		snprintf(buf, sizeof(buf),
			 "Page 02h (Bank %u) Slot 2 (0x80-0xFF)", bank);
		return buf;
	case OPTICAL_DOM_PAGE_11H:
		snprintf(buf, sizeof(buf),
			 "Page 11h (Bank %u) Slot 3 (0x80-0xFF)", bank);
		return buf;
	case OPTICAL_DOM_PAGE_14H:
		snprintf(buf, sizeof(buf),
			 "Page 14h (Bank %u) Slot 4 (0x80-0xFF)", bank);
		return buf;
	default:
		snprintf(buf, sizeof(buf),
			 "Page 0x%02X (Bank %u) (0x80-0xFF)", page, bank);
		return buf;
	}
}

static const char *find_annotation(uint8_t page, uint8_t page_row_base)
{
	uint32_t i;

	for (i = 0; i < (uint32_t)HIKP_ARRAY_SIZE(g_dump_annotations); i++) {
		if (g_dump_annotations[i].page == page &&
		    g_dump_annotations[i].page_row_base == page_row_base)
			return g_dump_annotations[i].label;
	}
	return NULL;
}

static void dump_hex_row(const uint8_t *data, uint8_t page_row_base, int row_off,
			 uint8_t page)
{
	uint8_t row_cmis_addr = page_row_base + row_off;
	const char *anno;
	int j;

	printf("0x%02X    :", row_cmis_addr);
	for (j = 0; j < DUMP_BYTES_PER_ROW && row_off + j < OPTICAL_DOM_PAGE_DATA_SIZE; j++)
		printf(" %02X", data[row_off + j]);

	anno = find_annotation(page, row_cmis_addr);
	if (anno)
		printf("   %s", anno);
	printf("\n");
}

static void optical_dom_dump_raw(const struct opti_dynamic_container *raw)
{
	uint32_t i;
	const uint8_t *page_data;

	printf("\n========== Optical Module DOM Raw Data Dump ==========\n");

	printf("\n--- %s ---\n", page_header_name(DUMP_ANNOTATION_PAGE_LOWER, 0));
	printf("Offset    ");
	for (i = 0; i < DUMP_BYTES_PER_ROW; i++)
		printf("%02X ", i);
	printf("\n");
	for (i = 0; i < OPTICAL_DOM_PAGE_DATA_SIZE; i += DUMP_BYTES_PER_ROW)
		dump_hex_row(raw->lower_page, 0, i, DUMP_ANNOTATION_PAGE_LOWER);

	for (i = 0; i < raw->total_slots_count; i++) {
		uint8_t page = raw->slots[i].page_num;
		uint8_t bank = raw->slots[i].bank_num;

		page_data = raw->slots[i].data;

		printf("\n--- %s ---\n",
		       page_header_name(page, bank));
		printf("Offset    ");
		for (uint32_t j = 0; j < DUMP_BYTES_PER_ROW; j++)
			printf("%02X ", j);
		printf("\n");
		for (uint32_t k = 0; k < OPTICAL_DOM_PAGE_DATA_SIZE; k += DUMP_BYTES_PER_ROW)
			/* 128 = upper page row base address */
			dump_hex_row(page_data, 128, k, page);
	}

	printf("\n====================================================\n");
}

static const uint8_t *optical_dom_get_page_data(const struct opti_dynamic_container *container,
						uint8_t page, uint8_t bank)
{
	uint32_t i;

	for (i = 0; i < container->total_slots_count; i++) {
		if (container->slots[i].page_num == page && container->slots[i].bank_num == bank)
			return container->slots[i].data;
	}

	printf("page %02Xh bank %u is not have.\n", page, bank);

	return NULL;
}

/* Page 01h Byte 0xA0 bits[4:3] → bias_mul shift amount (0=1x, 1=2x, 2=4x) */
static uint8_t optical_dom_get_bias_mul(const uint8_t *page01_data)
{
	uint8_t adver;
	uint8_t raw;

	if (!page01_data)
		return 0;

	adver = page01_data[DOM_CIS_SLOT_IDX(DOM_CIS_DIAG_CHAN_ADVER_OFFSET)];
	raw = adver & DOM_CIS_TX_BIAS_MUL_MSK;
	if (raw == DOM_CIS_TX_BIAS_MUL_2X)
		return 1; /* shift=1: TX Bias raw >> 1, effective multiplier 2x */

	if (raw == DOM_CIS_TX_BIAS_MUL_4X)
		return 2; /* shift=2: TX Bias raw >> 2, effective multiplier 4x */

	return 0; /* shift=0: no shift, effective multiplier 1x */
}

/* Helpers: read s16/u16 big-endian from page slot data (use DOM_CIS_SLOT_IDX) */
static int16_t read_s16_be(const uint8_t *data, uint8_t cmis_byte)
{
	/* 128 or 254：upper page range check */
	if (cmis_byte < 128 || cmis_byte > 254)
		return 0;

	return (int16_t)DOM_U16_BE_AT(data, DOM_CIS_SLOT_IDX(cmis_byte));
}

static uint16_t read_u16_be(const uint8_t *data, uint8_t cmis_byte)
{
	/* 128 or 254：upper page range check */
	if (cmis_byte < 128 || cmis_byte > 254)
		return 0;

	return DOM_U16_BE_AT(data, DOM_CIS_SLOT_IDX(cmis_byte));
}

/* Helpers: read u16 little-endian from page slot data (use DOM_CIS_SLOT_IDX) */
static uint16_t read_u16_le(const uint8_t *data, uint8_t cmis_byte)
{
	/* 128 or 254：upper page range check */
	if (cmis_byte < 128 || cmis_byte > 254)
		return 0;

	return DOM_U16_LE_AT(data, DOM_CIS_SLOT_IDX(cmis_byte));
}

static uint8_t optical_dom_get_lane_cnt(uint8_t identifier)
{
	switch (identifier) {
	case ID_SFP:
	case ID_SFP_PLUS_CMIS:
		return 0x1;
	case ID_SFP_DD:
	case ID_SFP_DD_CMIS:
		return 0x2;
	case ID_QSFP:
	case ID_QSFP_PLUS:
	case ID_QSFP28:
	case ID_QSFP_PLUS_CMIS:
		return 0x4;
	case ID_QSFP_DD:
	case ID_OSFP:
		return 0x8;
	default:
		/* Current default x4 mode. */
		return 0x4;
	}
}

static void optical_dom_parse_lower_page(const uint8_t *lp,
					 struct optical_dom_parse_data *out)
{
	/* 256.0 = signed 1/256th degC */
	out->temperature = (double)(int16_t)DOM_U16_BE_AT(lp, DOM_CIS_CUR_TEMP_OFFSET) / 256.0;
	/* 10000.0 = 0.1uW units to Volt */
	out->voltage = (double)DOM_U16_BE_AT(lp, DOM_CIS_CUR_VCC_OFFSET) / 10000.0;
	out->identifier = lp[DOM_CIS_IDENT_OFFSET];

	if (lp[DOM_CIS_MODULE_TYPE_OFFSET] < MEDIA_TYPE_RSVD)
		out->media_type = lp[DOM_CIS_MODULE_TYPE_OFFSET];
	else
		out->media_type = MEDIA_TYPE_UNDEFINED;

	out->host_lane_count = optical_dom_get_lane_cnt(lp[DOM_CIS_IDENT_OFFSET]);
	out->media_lane_count = optical_dom_get_lane_cnt(lp[DOM_CIS_IDENT_OFFSET]);
}

static void optical_dom_parse_page00(const uint8_t *p00,
				     struct optical_dom_parse_data *out)
{
	memcpy(out->vendor_name, &p00[DOM_CIS_SLOT_IDX(DOM_CIS_VENDOR_NAME_START)],
	       VENDOR_NAME_LEN);
	out->vendor_name[VENDOR_NAME_LEN] = '\0';

	memcpy(out->vendor_pn, &p00[DOM_CIS_SLOT_IDX(DOM_CIS_VENDOR_PN_START)],
	       VENDOR_PN_LEN);
	out->vendor_pn[VENDOR_PN_LEN] = '\0';

	memcpy(out->vendor_rev, &p00[DOM_CIS_SLOT_IDX(DOM_CIS_VENDOR_REV_START)],
	       VENDOR_REV_LEN);
	out->vendor_rev[VENDOR_REV_LEN] = '\0';

	memcpy(out->vendor_sn, &p00[DOM_CIS_SLOT_IDX(DOM_CIS_VENDOR_SN_START)],
	       VENDOR_SN_LEN);
	out->vendor_sn[VENDOR_SN_LEN] = '\0';

	memcpy(out->date_code, &p00[DOM_CIS_SLOT_IDX(DOM_CIS_DATE_YEAR_OFFSET)],
	       DATE_CODE_LEN);
	out->date_code[DATE_CODE_LEN] = '\0';

	/* bits[7:5]+1 = 1-based class */
	out->power_class = ((p00[DOM_CIS_SLOT_IDX(DOM_CIS_PWR_CLASS_OFFSET)] >> 5) & 7) + 1;
	/* 0.25 = 0.25W units */
	out->max_power = (double)p00[DOM_CIS_SLOT_IDX(DOM_CIS_PWR_MAX_OFFSET)] * 0.25;

	memcpy(out->vendor_oui,
	       &p00[DOM_CIS_SLOT_IDX(DOM_CIS_VENDOR_OUI_OFFSET)], DOM_CIS_VENDOR_OUI_LEN);
}

static void optical_dom_parse_page01(const uint8_t *p01,
				     struct optical_dom_parse_data *out)
{
	uint8_t raw;
	uint8_t mul_bits;
	uint16_t wraw;

	out->bias_mul = optical_dom_get_bias_mul(p01);

	raw = p01[DOM_CIS_SLOT_IDX(DOM_CIS_SMF_LEN_OFFSET)];
	/* extract bits[7:6] for unit */
	mul_bits = (raw & DOM_CIS_SMF_LEN_MUL_MSK) >> 6;
	if (mul_bits == 0)
		/* 0.1 = 0.1 km units */
		out->smf_length_km = (double)(raw & DOM_CIS_SMF_LEN_VAL_MSK) * 0.1;
	else
		/* 1.0 = km units */
		out->smf_length_km = (double)(raw & DOM_CIS_SMF_LEN_VAL_MSK) * 1.0;

	/* 2.0 = 2m units */
	out->om5_length_m = (double)p01[DOM_CIS_SLOT_IDX(DOM_CIS_OM5_LEN_OFFSET)] * 2.0;
	/* 2.0 = 2m units */
	out->om4_length_m = (double)p01[DOM_CIS_SLOT_IDX(DOM_CIS_OM4_LEN_OFFSET)] * 2.0;
	/* 2.0 = 2m units */
	out->om3_length_m = (double)p01[DOM_CIS_SLOT_IDX(DOM_CIS_OM3_LEN_OFFSET)] * 2.0;
	/* 1.0 = 1m units */
	out->om2_length_m = (double)p01[DOM_CIS_SLOT_IDX(DOM_CIS_OM2_LEN_OFFSET)] * 1.0;

	wraw = DOM_U16_BE_AT(p01, DOM_CIS_SLOT_IDX(DOM_CIS_NOM_WAVE_MSB));
	/* 0.05 = 50 GHz step in nm */
	out->nom_wavelength_nm = (double)wraw * 0.05;

	wraw = DOM_U16_BE_AT(p01, DOM_CIS_SLOT_IDX(DOM_CIS_WAVE_TOL_MSB));
	/* 0.005 = 0.005 nm resolution */
	out->wavelength_tol_nm = (double)wraw * 0.005;
}

static double power_raw_to_dbm_safe(uint16_t raw)
{
	if (raw == 0)
		return -40.0; /* zero-power sentinel: -40.0 dBm */

	/* 10.0 * log10(mW): raw is in 0.1uW, / 10000.0->mW */
	return 10.0 * log10((double)raw / 10000.0);
}

static void optical_dom_parse_page02(const uint8_t *p02,
				     struct optical_dom_parse_data *out)
{
	uint16_t uraw;
	uint8_t mul = out->bias_mul;

	/* divided by 256.0 = degC */
	out->temp_high_alarm = (double)read_s16_be(p02, DOM_CIS_TEMP_HALRM_OFFSET) / 256.0;
	/* divided by 256.0 = degC */
	out->temp_low_alarm  = (double)read_s16_be(p02, DOM_CIS_TEMP_LALRM_OFFSET) / 256.0;
	/* divided by 256.0 = degC */
	out->temp_high_warn  = (double)read_s16_be(p02, DOM_CIS_TEMP_HWARN_OFFSET) / 256.0;
	/* divided by 256.0 = degC */
	out->temp_low_warn   = (double)read_s16_be(p02, DOM_CIS_TEMP_LWARN_OFFSET) / 256.0;

	/* divided by 10000.0 = Volt */
	out->volt_high_alarm = (double)read_u16_be(p02, DOM_CIS_VCC_HALRM_OFFSET) / 10000.0;
	/* divided by 10000.0 = Volt */
	out->volt_low_alarm  = (double)read_u16_be(p02, DOM_CIS_VCC_LALRM_OFFSET) / 10000.0;
	/* divided by 10000.0 = Volt */
	out->volt_high_warn  = (double)read_u16_be(p02, DOM_CIS_VCC_HWARN_OFFSET) / 10000.0;
	/* divided by 10000.0 = Volt */
	out->volt_low_warn   = (double)read_u16_be(p02, DOM_CIS_VCC_LWARN_OFFSET) / 10000.0;

	uraw = read_u16_be(p02, DOM_CIS_TX_BIAS_HALRM_OFFSET);
	/* divided by 500.0 = mA */
	out->tx_bias_high_alarm = (double)(uraw >> mul) / 500.0;
	uraw = read_u16_be(p02, DOM_CIS_TX_BIAS_LALRM_OFFSET);
	/* divided by 500.0 = mA */
	out->tx_bias_low_alarm  = (double)(uraw >> mul) / 500.0;
	uraw = read_u16_be(p02, DOM_CIS_TX_BIAS_HWARN_OFFSET);
	/* divided by 500.0 = mA */
	out->tx_bias_high_warn  = (double)(uraw >> mul) / 500.0;
	uraw = read_u16_be(p02, DOM_CIS_TX_BIAS_LWARN_OFFSET);
	/* divided by 500.0 = mA */
	out->tx_bias_low_warn   = (double)(uraw >> mul) / 500.0;

	out->tx_power_high_alarm = power_raw_to_dbm_safe(
		read_u16_be(p02, DOM_CIS_TX_PWR_HALRM_OFFSET));
	out->tx_power_low_alarm  = power_raw_to_dbm_safe(
		read_u16_be(p02, DOM_CIS_TX_PWR_LALRM_OFFSET));
	out->tx_power_high_warn  = power_raw_to_dbm_safe(
		read_u16_be(p02, DOM_CIS_TX_PWR_HWARN_OFFSET));
	out->tx_power_low_warn   = power_raw_to_dbm_safe(
		read_u16_be(p02, DOM_CIS_TX_PWR_LWARN_OFFSET));

	out->rx_power_high_alarm = power_raw_to_dbm_safe(
		read_u16_be(p02, DOM_CIS_RX_PWR_HALRM_OFFSET));
	out->rx_power_low_alarm  = power_raw_to_dbm_safe(
		read_u16_be(p02, DOM_CIS_RX_PWR_LALRM_OFFSET));
	out->rx_power_high_warn  = power_raw_to_dbm_safe(
		read_u16_be(p02, DOM_CIS_RX_PWR_HWARN_OFFSET));
	out->rx_power_low_warn   = power_raw_to_dbm_safe(
		read_u16_be(p02, DOM_CIS_RX_PWR_LWARN_OFFSET));
}

static void optical_dom_parse_page11(const uint8_t *p11,
				     struct optical_dom_parse_data *out)
{
	uint8_t mul = out->bias_mul;
	int i;

	out->tx_los_mask  = p11[DOM_CIS_SLOT_IDX(DOM_CIS_TX_LOS_OFFSET)];
	out->tx_lol_mask  = p11[DOM_CIS_SLOT_IDX(DOM_CIS_TX_LOL_OFFSET)];
	out->rx_los_mask  = p11[DOM_CIS_SLOT_IDX(DOM_CIS_RX_LOS_OFFSET)];
	out->rx_lol_mask  = p11[DOM_CIS_SLOT_IDX(DOM_CIS_RX_LOL_OFFSET)];

	for (i = 0; i < OPTICAL_DOM_MAX_LANES; i++) {
		/* +2 per lane */
		uint16_t tx_pwr  = read_u16_be(p11, DOM_CIS_TX_PWR_OFFSET + 2 * i);
		/* +2 per lane */
		uint16_t tx_bias = read_u16_be(p11, DOM_CIS_TX_BIAS_OFFSET + 2 * i);
		/* +2 per lane */
		uint16_t rx_pwr  = read_u16_be(p11, DOM_CIS_RX_PWR_OFFSET + 2 * i);

		if (tx_pwr == 0)
			/* zero-power sentinel, default -40.0dbm */
			out->tx_power_dbm[i] = -40.0;
		else
			/* dBm: 10.0 * log10(mW), divided by 10000.0 is uints */
			out->tx_power_dbm[i] = 10.0 * log10((double)tx_pwr / 10000.0);

		if (tx_bias == LANE_DATA_INVALID_FFFF)
			out->tx_bias_ma[i] = 0.0;
		else
			/* divided by 500.0 = mA */
			out->tx_bias_ma[i] = (double)(tx_bias >> mul) / 500.0;

		if (rx_pwr == 0)
			/* zero-power sentinel, default -40.0dbm */
			out->rx_power_dbm[i] = -40.0;
		else
			/* dBm: 10.0 * log10(mW), divided by 10000.0 is uints */
			out->rx_power_dbm[i] = 10.0 * log10((double)rx_pwr / 10000.0);
	}
}

static void optical_dom_parse_page14(const uint8_t *p14,
				     struct optical_dom_parse_data *out)
{
	for (uint32_t i = 0; i < OPTICAL_DOM_MAX_LANES; i++) {
		/* 208: host snr, +2 per lane, divided by 256.0 = 1/256 dB units */
		out->host_snr[i] = (double)read_u16_le(p14, 208 + 2 * i) / 256.0;

		/* 240: media snr, +2 per lane, divided by 256.0 = 1/256 dB units */
		out->media_snr[i] = (double)read_u16_le(p14, 240 + 2 * i) / 256.0;
	}
}

static int optical_dom_parse(const struct opti_dynamic_container *raw,
			     struct optical_dom_parse_data *out)
{
	const uint8_t *page_data;

	out->page_present |= OPTICAL_DOM_PG_LOWER;
	optical_dom_parse_lower_page(raw->lower_page, out);

	page_data = optical_dom_get_page_data(raw, OPTICAL_DOM_PAGE_00H, OPTICAL_DOM_BANK_0);
	if (page_data) {
		out->page_present |= OPTICAL_DOM_PG_00H;
		optical_dom_parse_page00(page_data, out);
	}

	page_data = optical_dom_get_page_data(raw, OPTICAL_DOM_PAGE_01H, OPTICAL_DOM_BANK_0);
	if (page_data) {
		out->page_present |= OPTICAL_DOM_PG_01H;
		optical_dom_parse_page01(page_data, out);
	}

	page_data = optical_dom_get_page_data(raw, OPTICAL_DOM_PAGE_02H, OPTICAL_DOM_BANK_0);
	if (page_data) {
		out->page_present |= OPTICAL_DOM_PG_02H;
		optical_dom_parse_page02(page_data, out);
	}

	page_data = optical_dom_get_page_data(raw, OPTICAL_DOM_PAGE_11H, OPTICAL_DOM_BANK_0);
	if (page_data) {
		out->page_present |= OPTICAL_DOM_PG_11H;
		optical_dom_parse_page11(page_data, out);
	}

	page_data = optical_dom_get_page_data(raw, OPTICAL_DOM_PAGE_14H, OPTICAL_DOM_BANK_0);
	if (page_data) {
		out->page_present |= OPTICAL_DOM_PG_14H;
		optical_dom_parse_page14(page_data, out);
	}

	return 0;
}

static const char *optical_dom_id_name(uint8_t id)
{
	switch (id) {
	case ID_SFP:
		return "SFP/SFP+/SFP28";
	case ID_QSFP_PLUS:
		return "QSFP+";
	case ID_QSFP28:
		return "QSFP28";
	case ID_QSFP_DD:
		return "QSFP-DD";
	case ID_OSFP:
		return "OSFP";
	case ID_SFP_DD:
		return "SFP-DD";
	case ID_QSFP_PLUS_CMIS:
		return "QSFP+(CMIS)";
	default:
		return "Unknown";
	}
}

static const char *optical_dom_media_name(uint8_t media_type)
{
	switch (media_type) {
	case MEDIA_TYPE_UNDEFINED:
		return "Undefined";
	case MEDIA_TYPE_MMF:
		return "MMF Optical";
	case MEDIA_TYPE_SMF:
		return "SMF Optical";
	case MEDIA_TYPE_PASSIVE_COPPER:
		return "Passive Copper";
	case MEDIA_TYPE_ACTIVE_CABLE:
		return "Active Cable";
	case MEDIA_TYPE_BASE_T:
		return "Base-T";
	default:
		return "Unknown";
	}
}

static void optical_dom_show_header(const struct optical_dom_parse_data *d)
{
	printf("\n===================== Optical Module DOM Diagnostics =======================\n");
	printf("%-20s: %s (0x%02x)\n", "Module Type",
	       optical_dom_id_name(d->identifier), d->identifier);
	printf("%-20s: %s\n", "Media Type",
	       optical_dom_media_name(d->media_type));
	printf("%-20s: x%u\n", "Host Lane Count", d->host_lane_count);
	printf("%-20s: x%u\n", "Media Lane Count", d->media_lane_count);
}

static void optical_dom_show_vendor(const struct optical_dom_parse_data *d)
{
	printf("%-20s: %s\n", "Vendor Name", d->vendor_name);
	printf("%-20s: %s\n", "Vendor PN", d->vendor_pn);
	printf("%-20s: %s\n", "Vendor Rev", d->vendor_rev);
	printf("%-20s: %s\n", "Vendor SN", d->vendor_sn);
	printf("%-20s: %02X:%02X:%02X\n", "Vendor OUI",
	       d->vendor_oui[0x0], d->vendor_oui[0x1], d->vendor_oui[0x2]);
	printf("%-20s: %s\n", "Date Code", d->date_code);
	printf("%-20s: %u\n", "Power Class", d->power_class);
	printf("%-20s: %.2f W\n", "Max Power", d->max_power);
}

static void optical_dom_show_fiber_wl(const struct optical_dom_parse_data *d)
{
	/* shift amount: 1x/2x/4x */
	printf("%-20s: %u\n", "Bias Multiplier", 1 << d->bias_mul);
	printf("%-20s: %.2f nm\n", "Nominal Wavelength", d->nom_wavelength_nm);
	printf("%-20s: %.3f nm\n", "Wavelength Tolerance", d->wavelength_tol_nm);
	printf("%-20s: %.1f km\n", "Length(SMF)", d->smf_length_km);
	printf("%-20s: %.0f m\n", "Length(OM5)", d->om5_length_m);
	printf("%-20s: %.0f m\n", "Length(OM4)", d->om4_length_m);
	printf("%-20s: %.0f m\n", "Length(OM3)", d->om3_length_m);
	printf("%-20s: %.0f m\n", "Length(OM2)", d->om2_length_m);
}

static void optical_dom_show_global_mon(const struct optical_dom_parse_data *d)
{
	printf("\n------------------------------ Global Monitor ------------------------------\n");
	printf("%-20s: %+.2f C\n", "Temperature", d->temperature);
	printf("%-20s: %.3f V\n", "Voltage", d->voltage);
}

static void optical_dom_show_thresholds(const struct optical_dom_parse_data *d)
{
	printf("\n------------------------- Alarm/Warning Thresholds -------------------------\n");
	printf("%-20s  %12s  %12s  %12s  %12s\n",
	       "", "High Alarm", "Low Alarm", "High Warn", "Low Warn");
	printf("%-20s: %12.2f  %12.2f  %12.2f  %12.2f\n",
	       "Temperature(C)",
	       d->temp_high_alarm, d->temp_low_alarm,
	       d->temp_high_warn, d->temp_low_warn);
	printf("%-20s: %12.3f  %12.3f  %12.3f  %12.3f\n",
	       "Voltage(V)",
	       d->volt_high_alarm, d->volt_low_alarm,
	       d->volt_high_warn, d->volt_low_warn);
	printf("%-20s: %12.2f  %12.2f  %12.2f  %12.2f\n",
	       "TX Bias(mA)",
	       d->tx_bias_high_alarm, d->tx_bias_low_alarm,
	       d->tx_bias_high_warn, d->tx_bias_low_warn);
	printf("%-20s: %+12.2f  %+12.2f  %+12.2f  %+12.2f\n",
	       "TX Power(dBm)",
	       d->tx_power_high_alarm, d->tx_power_low_alarm,
	       d->tx_power_high_warn, d->tx_power_low_warn);
	printf("%-20s: %+12.2f  %+12.2f  %+12.2f  %+12.2f\n",
	       "RX Power(dBm)",
	       d->rx_power_high_alarm, d->rx_power_low_alarm,
	       d->rx_power_high_warn, d->rx_power_low_warn);
}

static void optical_dom_show_channel_diag(const struct optical_dom_parse_data *d,
					  bool has_11h, bool has_14h)
{
	uint32_t i;

	printf("\n--------------------------- Channel Diagnostics ----------------------------\n");
	printf("%-5s:", "Lane");
	if (has_11h)
		printf(" %13s %14s %12s", "TX Power(dBm)", "RX Power(dBm)", "TX Bias(mA)");

	if (has_14h)
		printf(" %13s %14s", "Host SNR(dB)", "Media SNR(dB)");

	printf("\n");
	for (i = 0; i < d->host_lane_count; i++) {
		printf("%-5u:", i);
		if (has_11h)
			printf(" %12.2f %14.2f %12.2f",
			       d->tx_power_dbm[i], d->rx_power_dbm[i], d->tx_bias_ma[i]);
		if (has_14h)
			printf(" %13.1f %14.1f", d->host_snr[i], d->media_snr[i]);
		printf("\n");
	}
}

static void optical_dom_show_status_flags(const struct optical_dom_parse_data *d)
{
	printf("\n------------------------------- Status Flags -------------------------------\n");
	printf("RX LOS: 0x%02x       TX LOS: 0x%02x       RX LOL: 0x%02x       TX LOL: 0x%02x\n",
	       d->rx_los_mask, d->tx_los_mask, d->rx_lol_mask, d->tx_lol_mask);
}

static void optical_dom_display(const struct optical_dom_parse_data *d)
{
	bool has_11h = d->page_present & OPTICAL_DOM_PG_11H;
	bool has_14h = d->page_present & OPTICAL_DOM_PG_14H;

	optical_dom_show_header(d);
	if (d->page_present & OPTICAL_DOM_PG_00H)
		optical_dom_show_vendor(d);
	if (d->page_present & OPTICAL_DOM_PG_01H)
		optical_dom_show_fiber_wl(d);
	optical_dom_show_global_mon(d);
	if (has_11h || has_14h)
		optical_dom_show_channel_diag(d, has_11h, has_14h);
	if (d->page_present & OPTICAL_DOM_PG_02H)
		optical_dom_show_thresholds(d);
	if (has_11h)
		optical_dom_show_status_flags(d);
	printf("============================================================================\n");
}

static int cmd_optical_dom_help(struct major_cmd_ctrl *self, const char *argv)
{
	HIKP_SET_USED(argv);

	printf("\n  Usage: %s %s\n", self->cmd_ptr->name,
	       "-c <chip_id> -d <die_id> -p <port_id> [-r]");
	printf("\n         %s\n", self->cmd_ptr->help_info);
	printf("\n  Options:\n\n");
	printf("    -h, %-15s %s\n", "--help", "display this help and exit");
	printf("    -c, %-15s %s\n", "--chip", "chip id");
	printf("    -d, %-15s %s\n", "--die", "die id");
	printf("    -p, %-15s %s\n", "--port", "port id");
	printf("    -r, %-15s %s\n", "--raw", "dump raw binary data with protocol offsets");
	printf("\n");

	return 0;
}

static int cmd_optical_dom_chip(struct major_cmd_ctrl *self, const char *argv)
{
	uint8_t val;

	if (string_toub(argv, &val) != 0) {
		snprintf(self->err_str, sizeof(self->err_str), "invalid chip_id=%s", argv);
		self->err_no = -EINVAL;
		return -EINVAL;
	}

	g_optical_dom_param.chip_id = val;
	g_optical_dom_param.param_mask |= OPTICAL_DOM_PARAM_CHIP;
	return 0;
}

static int cmd_optical_dom_port(struct major_cmd_ctrl *self, const char *argv)
{
	uint8_t val;

	if (string_toub(argv, &val) != 0) {
		snprintf(self->err_str, sizeof(self->err_str), "invalid port_id=%s", argv);
		self->err_no = -EINVAL;
		return -EINVAL;
	}

	g_optical_dom_param.port_id = val;
	g_optical_dom_param.param_mask |= OPTICAL_DOM_PARAM_PORT;
	return 0;
}

static int cmd_optical_dom_die(struct major_cmd_ctrl *self, const char *argv)
{
	uint8_t val;

	if (string_toub(argv, &val) != 0) {
		snprintf(self->err_str, sizeof(self->err_str), "invalid die_id=%s", argv);
		self->err_no = -EINVAL;
		return -EINVAL;
	}

	g_optical_dom_param.die_id = val;
	g_optical_dom_param.param_mask |= OPTICAL_DOM_PARAM_DIE;
	return 0;
}

static int cmd_optical_dom_raw(struct major_cmd_ctrl *self, const char *argv)
{
	HIKP_SET_USED(self);
	HIKP_SET_USED(argv);

	g_optical_dom_param.param_mask |= OPTICAL_DOM_PARAM_RAW;
	return 0;
}

static bool optical_dom_check_params(struct major_cmd_ctrl *self)
{
	uint32_t missing = OPTICAL_DOM_PARAM_MANDATORY & (~g_optical_dom_param.param_mask);

	if ((missing & OPTICAL_DOM_PARAM_CHIP) != 0) {
		snprintf(self->err_str, sizeof(self->err_str), "chip_id is not set");
		self->err_no = -EINVAL;
		return false;
	}
	if ((missing & OPTICAL_DOM_PARAM_PORT) != 0) {
		snprintf(self->err_str, sizeof(self->err_str), "port_id is not set");
		self->err_no = -EINVAL;
		return false;
	}
	if ((missing & OPTICAL_DOM_PARAM_DIE) != 0) {
		snprintf(self->err_str, sizeof(self->err_str), "die_id is not set");
		self->err_no = -EINVAL;
		return false;
	}
	return true;
}

static int optical_dom_get_blk_data(struct hikp_cmd_ret **cmd_ret, uint32_t blk_id)
{
	struct optical_dom_req_data req_data = {0};
	struct hikp_cmd_header req_header = {0};

	req_data.chip_id = g_optical_dom_param.chip_id;
	req_data.port_id = g_optical_dom_param.port_id;
	req_data.die_id  = g_optical_dom_param.die_id;
	req_data.block_id = blk_id;
	hikp_cmd_init(&req_header, OPTICAL_DOM_MOD,
		      OPTICAL_DOM_CMD_DIAG, OPTICAL_DOM_SUBCMD_GET_DATA);
	*cmd_ret = hikp_cmd_alloc(&req_header, &req_data, sizeof(req_data));
	return hikp_rsp_normal_check(*cmd_ret);
}

static int optical_dom_check_ret_code(struct major_cmd_ctrl *self, uint8_t ret_code)
{
	switch (ret_code) {
	case MODULE_STATUS_OK:
		return 0;
	case MODULE_STATUS_CHIP_DIE_ERR:
		snprintf(self->err_str, sizeof(self->err_str),
			 "chip %u die %u not supported",
			 g_optical_dom_param.chip_id, g_optical_dom_param.die_id);
		return -EINVAL;
	case MODULE_STATUS_BLK_ID_ERR:
		snprintf(self->err_str, sizeof(self->err_str),
			 "block id error, chip %u die %u",
			 g_optical_dom_param.chip_id, g_optical_dom_param.die_id);
		return -EINVAL;
	case MODULE_STATUS_ENODEV:
		printf("no optical module present on chip %u die %u port %u\n",
		       g_optical_dom_param.chip_id, g_optical_dom_param.die_id,
		       g_optical_dom_param.port_id);
		return MODULE_STATUS_ENODEV;
	case MODULE_STATUS_EOPNOTSUPP:
		printf("optical module query not supported on chip %u die %u port %u\n",
		       g_optical_dom_param.chip_id, g_optical_dom_param.die_id,
		       g_optical_dom_param.port_id);
		return MODULE_STATUS_EOPNOTSUPP;
	case MODULE_STATUS_READ_FAILED:
		snprintf(self->err_str, sizeof(self->err_str),
			 "failed to read optical module info on chip %u die %u port %u",
			 g_optical_dom_param.chip_id, g_optical_dom_param.die_id,
			 g_optical_dom_param.port_id);
		return -EIO;
	default:
		snprintf(self->err_str, sizeof(self->err_str),
			 "operation failed, ret_code %u from firmware", ret_code);
		return -EIO;
	}
}

static int optical_dom_get_first_blk(struct major_cmd_ctrl *self, uint32_t *total_blk,
				     uint32_t *cur_size, uint8_t **buffer, uint32_t *buf_size)
{
	struct optical_dom_blk_rsp *blk_rsp = NULL;
	struct hikp_cmd_ret *cmd_ret = NULL;
	int ret;

	ret = optical_dom_get_blk_data(&cmd_ret, 0);
	if (ret) {
		snprintf(self->err_str, sizeof(self->err_str),
			 "get first block failed, ret=%d", ret);
		hikp_cmd_free(&cmd_ret);
		self->err_no = ret;
		return ret;
	}

	blk_rsp = (struct optical_dom_blk_rsp *)(cmd_ret->rsp_data);
	ret = optical_dom_check_ret_code(self, blk_rsp->ret_code);
	if (ret != 0) {
		hikp_cmd_free(&cmd_ret);
		/* When the port does not support the operation
		 * or the optical module is not detected, a success message is returned.
		 */
		self->err_no = (ret < 0) ? ret : 0x0;
		return ret;
	}

	*buf_size = blk_rsp->total_blk_num * OPTICAL_DOM_DATA_BLK_SIZE;

	if ((blk_rsp->cur_blk_size == 0) || (blk_rsp->cur_blk_size > *buf_size) ||
	    (blk_rsp->cur_blk_size > sizeof(blk_rsp->data))) {
		snprintf(self->err_str, sizeof(self->err_str),
			 "invalid block header: total=%u, cur_size=%u",
			 blk_rsp->total_blk_num, blk_rsp->cur_blk_size);
		hikp_cmd_free(&cmd_ret);
		self->err_no = -EINVAL;
		return -EINVAL;
	}

	*buffer = (uint8_t *)calloc(1, *buf_size);
	if (*buffer == NULL) {
		snprintf(self->err_str, sizeof(self->err_str),
			 "failed to allocate %u bytes", *buf_size);
		hikp_cmd_free(&cmd_ret);
		self->err_no = -ENOMEM;
		return -ENOMEM;
	}

	*cur_size = blk_rsp->cur_blk_size;
	*total_blk = blk_rsp->total_blk_num;
	memcpy(*buffer, blk_rsp->data, blk_rsp->cur_blk_size);

	hikp_cmd_free(&cmd_ret);
	return 0;
}

static int optical_dom_get_remain_blk(struct major_cmd_ctrl *self,
				      struct optical_dom_blk_ctrl *blk_ctrl,
				      uint8_t *buf, uint32_t remain_size, uint32_t offset)
{
	struct optical_dom_blk_rsp *blk_rsp = NULL;
	struct hikp_cmd_ret *cmd_ret = NULL;
	int ret;

	ret = optical_dom_get_blk_data(&cmd_ret, blk_ctrl->blk_id);
	if (ret) {
		snprintf(self->err_str, sizeof(self->err_str), "get block %u failed, ret=%d",
			 blk_ctrl->blk_id, ret);
		self->err_no = ret;
		hikp_cmd_free(&cmd_ret);
		return ret;
	}

	blk_rsp = (struct optical_dom_blk_rsp *)(cmd_ret->rsp_data);
	ret = optical_dom_check_ret_code(self, blk_rsp->ret_code);
	if (ret != 0) {
		hikp_cmd_free(&cmd_ret);
		/* When the port does not support the operation
		 * or the optical module is not detected, a success message is returned.
		 */
		self->err_no = (ret < 0) ? ret : 0x0;
		return ret;
	}

	blk_ctrl->resp_blk_size = blk_rsp->cur_blk_size;
	blk_ctrl->total_blk_num = blk_rsp->total_blk_num;
	if (blk_rsp->cur_blk_size > remain_size ||
	    blk_rsp->cur_blk_size > sizeof(blk_rsp->data)) {
		snprintf(self->err_str, sizeof(self->err_str), "block %u data size %u > remain %u",
			 blk_ctrl->blk_id, blk_rsp->cur_blk_size, remain_size);
		hikp_cmd_free(&cmd_ret);
		self->err_no = -EINVAL;
		return -EINVAL;
	}

	memcpy(buf + offset, blk_rsp->data, blk_rsp->cur_blk_size);
	hikp_cmd_free(&cmd_ret);
	return 0;
}

static int optical_dom_get_diag_data(struct major_cmd_ctrl *self,
				     struct opti_dynamic_container **out_container)
{
	struct optical_dom_blk_ctrl blk_ctrl = {0};
	struct opti_dynamic_container *opti_data;
	uint8_t *buffer = NULL;
	uint32_t remain_size;
	uint32_t offset = 0;
	uint32_t total_blk;
	uint32_t max_slots;
	uint32_t buf_size;
	int ret;

	ret = optical_dom_get_first_blk(self, &total_blk, &offset, &buffer, &buf_size);
	if (ret != 0)
		return ret;

	remain_size = buf_size - offset;

	for (uint32_t i = 1; i < total_blk; i++) {
		blk_ctrl.blk_id = i;
		ret = optical_dom_get_remain_blk(self, &blk_ctrl, buffer, remain_size, offset);
		if (ret != 0) {
			free(buffer);
			buffer = NULL;
			return ret;
		}
		remain_size -= blk_ctrl.resp_blk_size;
		offset += blk_ctrl.resp_blk_size;
		if (blk_ctrl.total_blk_num == 0 || blk_ctrl.resp_blk_size == 0)
			break;
	}

	max_slots = (buf_size - OPTICAL_DOM_HEADER_SIZE) / OPTICAL_DOM_SLOT_SIZE;
	opti_data = (struct opti_dynamic_container *)buffer;
	if (opti_data->total_slots_count > max_slots) {
		snprintf(self->err_str, sizeof(self->err_str),
			 "invalid total_slots_count %u > max %u",
			 opti_data->total_slots_count, max_slots);
		free(buffer);
		buffer = NULL;
		self->err_no = -EINVAL;
		return -EINVAL;
	}

	*out_container = opti_data;
	return 0;
}

static void hikp_optical_dom_execute(struct major_cmd_ctrl *self)
{
	struct opti_dynamic_container *raw_container = NULL;
	struct optical_dom_parse_data parse_data = {0};
	int ret;

	if (!optical_dom_check_params(self)) {
		cmd_optical_dom_help(self, NULL);
		return;
	}

	ret = optical_dom_get_diag_data(self, &raw_container);
	if (ret != 0)
		return;

	if (g_optical_dom_param.param_mask & OPTICAL_DOM_PARAM_RAW) {
		optical_dom_dump_raw(raw_container);
	} else {
		ret = optical_dom_parse(raw_container, &parse_data);
		if (ret != 0) {
			snprintf(self->err_str, sizeof(self->err_str),
				 "failed to parse DOM data, ret=%d", ret);
			self->err_no = ret;
			goto out_free;
		}
		optical_dom_display(&parse_data);
	}

out_free:
	free(raw_container);
	raw_container = NULL;
}

static void cmd_optical_dom_init(void)
{
	struct major_cmd_ctrl *major_cmd = get_major_cmd();

	major_cmd->option_count = 0;
	major_cmd->execute = hikp_optical_dom_execute;

	cmd_option_register("-h", "--help", false, cmd_optical_dom_help);
	cmd_option_register("-c", "--chip", true,  cmd_optical_dom_chip);
	cmd_option_register("-d", "--die",  true,  cmd_optical_dom_die);
	cmd_option_register("-p", "--port", true,  cmd_optical_dom_port);
	cmd_option_register("-r", "--raw", false, cmd_optical_dom_raw);
}

HIKP_CMD_DECLARE("optical_dom", "query optical module DOM diagnostics", cmd_optical_dom_init);
