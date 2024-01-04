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

#ifndef HIKP_NIC_XSFP_H
#define HIKP_NIC_XSFP_H
#include "hikp_net_lib.h"

#define SFF_XSFP_DATA_LEN   640

#define XSFP_PRINT_BLK      128
#define XSFP_PRINT_COL      16

#define SFF_TRANCEIV_LEN            8
#define SFF_WAVE_LEN_UNIT           256

#define SFF_CONNECTOR_COPPER        0x21

#define VEND_NAME_LEN               16
#define VEND_OUI_LEN                3
#define VEND_PN_LEN                 16
#define VEND_SN_LEN                 16
#define VEND_DATE_CODE_LEN          8
#define VEND_SPEC_EEPROM_LEN        32
#define VEND_SPEC_LEN               7
#define VEND_USER_EEPROM_LEN        120
#define VEND_CTRL_LEN               8

#define SFP_VEND_REV_LEN            4

#define QSFP_CHAN_NUM               4
#define QSFP_VEND_REV_LEN           2
#define QSFP_WAVE_LEN_DIV           20
#define QSFP_TOL_WAVE_LEN_DIV       200

#define QSFP_OM3_LEN_UNIT           2

/* reg3: 10G Ethernet Compliance Codes mask */
#define SFP_10GBASE_SR_MASK        HI_BIT(4)
#define SFP_10GBASE_LR_MASK        HI_BIT(5)
#define SFP_10GBASE_LRM_MASK       HI_BIT(6)
#define SFP_10GBASE_ER_MASK        HI_BIT(7)

/* reg6: Ethernet Compliance Codes mask */
#define SFP_1000BASE_SX_MASK       HI_BIT(0)
#define SFP_1000BASE_LX_MASK       HI_BIT(1)
#define SFP_1000BASE_CX_MASK       HI_BIT(2)
#define SFP_1000BASE_T_MASK        HI_BIT(3)
#define SFP_100BASE_LX_LX10_MASK   HI_BIT(4)
#define SFP_100BASE_FX_MASK        HI_BIT(5)
#define SFP_BASE_BX10_MASK         HI_BIT(6)
#define SFP_BASE_PX_MASK           HI_BIT(7)

/* reg134: Gigabit Ethernet Compliance Codes mask */
#define QSFP_1000BASE_SX_MASK       HI_BIT(0)
#define QSFP_1000BASE_LX_MASK       HI_BIT(1)
#define QSFP_1000BASE_CX_MASK       HI_BIT(2)
#define QSFP_1000BASE_T_MASK        HI_BIT(3)

/* reg131: 10/40G/100G Ethernet Compliance Codes mask */
#define QSFP_40G_ACT_CABLE_MASK     HI_BIT(0)
#define QSFP_40GBASE_LR4_MASK       HI_BIT(1)
#define QSFP_40GBASE_SR4_MASK       HI_BIT(2)
#define QSFP_40GBASE_CR4_MASK       HI_BIT(3)
#define QSFP_10GBASE_SR_MASK        HI_BIT(4)
#define QSFP_10GBASE_LR_MASK        HI_BIT(5)
#define QSFP_10GBASE_LRM_MASK       HI_BIT(6)

#define CMIS_WAVE_LEN_DIV           20
#define CMIS_TOL_WAVE_LEN_DIV       200

#define CMIS_VEND_REV_LEN           2

enum print_type {
	PRINT_ASCII = 0,
	PRINT_HEX,
};

#define SFF_ID_OFFSET           0
enum sff_id_val {
	ID_UNDEF = 0x0,
	ID_SFP = 0x3,
	ID_QSFP = 0x0C,
	ID_QSFP_PLUS = 0x0D,
	ID_QSFP28 = 0x11,
	ID_QSFP_DD = 0x18,
	ID_SFP_DD = 0x1A,
	ID_QSFP_P_CMIS = 0x1E,
	ID_SFP_DD_CMIS = 0x1F,
	ID_SFP_P_CMIS = 0x20,
};

struct sff_comp_info {
	uint8_t mask;
	const char *description;
};

struct sff_ext_comp {
	uint8_t val;
	const char *module_cap;
};

struct sff_host_media_id {
	uint8_t id;
	const char *int_spec;
	uint8_t lane_cnt;
	const char *modulation;
};

struct sff_media_cable_id {
	uint8_t id;
	const char *app_name;
};

struct sfp_a0_page {
	uint8_t identifier;                     /* reg 0: Identifier */
	uint8_t ext_identifier;                 /* reg 1: Ext. Identifier */
	uint8_t connector;                      /* reg 2: Connector */
	uint8_t transceiver[SFF_TRANCEIV_LEN];  /* reg 3-10: Transceiver */
	uint8_t encoding;                       /* reg 11: Encoding */
	uint8_t br_nominal;             /* reg 12: Nominal signalling rate, units of 100MBd. */
	uint8_t rate_identifier;        /* reg 13: Type of rate select functionality */
	/* reg 14: Link length supported for single mode fiber, units of km */
	uint8_t len_smf_km;
	/* reg 15: Link length supported for single mode fiber, units of 100 m */
	uint8_t len_smf;
	/* reg 16: Link length supported for 50 um OM2 fiber, units of 10 m */
	uint8_t len_smf_om2;
	/* reg 17: Link length supported for 62.5 um OM1 fiber, units of 10 m */
	uint8_t len_smf_om1;
	/* reg 18: Link length supported for copper or direct attach cable, units of m */
	uint8_t len_cable;
	/* reg 19: Link length supported for 50 um OM3 fiber, units of 10 m */
	uint8_t len_om3;
	uint8_t vendor_name[VEND_NAME_LEN];     /* reg 20-35: Vendor name */
	uint8_t transceiver_code;       /* reg 36: Code for electronic or optical compatibility */
	uint8_t vendor_oui[VEND_OUI_LEN];       /* reg 37-39: SFP vendor IEEE company ID */
	/* reg 40-55: Part number provided by SFP vendor (ASCII) */
	uint8_t vendor_pn[VEND_PN_LEN];
	/* reg 56-59: Revision level for part number provided by vendor (ASCII) */
	uint8_t vendor_rev[SFP_VEND_REV_LEN];
	/* reg 60-61: Laser wavelength (Passive/Active Cable Specification Compliance) */
	uint8_t wave_leng[2];
	uint8_t un_allocated;       /* reg 62 */
	uint8_t ccbase;             /* reg 63: Check code for Base ID Fields (addresses 0 to 62) */
	struct {                    /* reg 64-65: Option Values */
		uint8_t option_val;
		uint8_t rsvd0 : 1;
		uint8_t rx_los_imp : 1;
		uint8_t sign_detect_imp : 1;
		uint8_t tx_fault_imp : 1;
		uint8_t tx_disable_imp : 1;
		uint8_t rate_sel_imp : 1;
		uint8_t rsvd1 : 2;
	};
	uint8_t br_max;                         /* reg 66: Signaling Rate, max */
	uint8_t br_min;                         /* reg 67: Signaling Rate, min */
	uint8_t vendor_sn[VEND_SN_LEN];         /* reg 68-83: Vendor SN */
	uint8_t date_code[VEND_DATE_CODE_LEN];  /* reg 84-91: Date code */
	struct {                                /* reg 92: Diagnostic check Type */
		uint8_t rsvd2 : 2;
		uint8_t addr_chge : 1;
		uint8_t rx_power_measure_type : 1;
		uint8_t ext_cal : 1;
		uint8_t inter_cal : 1;
		uint8_t ddm_imp : 1;
		uint8_t rsvd_dm : 1;
	};
	uint8_t enhanced_options;               /* reg 93: Enhanced Options */
	uint8_t Sff_8472_compliance;            /* reg 94: SFF-8472 Compliance */
	uint8_t cc_ext; /* reg 95: Check code for the Extended ID Fields (addresses 64 to 94) */

	/* 96~255 */
	uint8_t vendor_spec[VEND_SPEC_EEPROM_LEN];  /* reg 96-127: Vendor Specific */
	uint8_t rsvd[128];                          /* reg 128-255: Reserved */
};

struct sfp_a2_page {
	uint8_t temp_alarm_high[2];     /* reg 0-1: Temp High Alarm */
	uint8_t temp_alarm_low[2];      /* reg 2-3: Temp Low Alarm */
	uint8_t temp_warning_high[2];   /* reg 4-5: Temp High Warning */
	uint8_t temp_warning_low[2];    /* reg 6-7: Temp Low Warning */
	uint8_t vcc_alarm_high[2];      /* reg 8-9: Voltage High Alarm */
	uint8_t vcc_alarm_low[2];       /* reg 10-11: Voltage Low Alarm */
	uint8_t vcc_warning_high[2];    /* reg 12-13: Voltage High Warning */
	uint8_t vcc_warning_low[2];     /* reg 14-15: Voltage Low Warning */
	uint8_t bias_alarm_high[2];     /* reg 16-17: Bias High Alarm */
	uint8_t bias_alarm_low[2];      /* reg 18-19: Bias Low Alarm */
	uint8_t bias_warning_high[2];   /* reg 20-21: Bias High Warning */
	uint8_t bias_warning_low[2];    /* reg 22-23: Bias Low Warning */
	uint8_t tx_alarm_high[2];       /* reg 24-25: TX Power High Alarm */
	uint8_t tx_alarm_low[2];        /* reg 26-27: TX Power Low Alarm */
	uint8_t tx_warning_high[2];     /* reg 28-29: TX Power High Warning */
	uint8_t tx_warning_low[2];      /* reg 30-31: TX Power Low Warning */
	uint8_t rx_alarm_high[2];       /* reg 32-33: RX Power High Alarm */
	uint8_t rx_alarm_low[2];        /* reg 34-35: RX Power Low Alarm */
	uint8_t rx_warning_high[2];     /* reg 36-37: RX Power High Warning */
	uint8_t rx_warning_low[2];      /* reg 38-39: RX Power Low Warning */
	uint8_t opt_thres[16];          /* reg 40-55: Optional A/W Thresholds */
	/* reg 56-91: Ext Cal Constants or Additional Enhanced Features */
	uint8_t ext_cal_constants[36];
	uint8_t un_allocated1[3];       /* reg 92-94: Reserved */
	uint8_t cc_dmi;                 /* reg 95: CC_DMI */
	uint8_t temperature[2];         /* reg 96-97: Temperature MSB/LSB */
	uint8_t vcc[2];                 /* reg 98-99: Vcc MSB/LSB */
	uint8_t tx_bias[2];             /* reg 100-101: TX Bias MSB/LSB */
	uint8_t tx_power[2];            /* reg 102-103: TX Power MSB/LSB */
	uint8_t rx_power[2];            /* reg 104-105: RX Power MSB/LSB */
	/* reg 106-109: Optional Laser Temp/Wavelength and Optional TEC current */
	uint8_t optional_val[4];
	struct {                        /* reg 110: Optional Status/Control */
		uint8_t data_rdy_bar_state : 1;
		uint8_t rx_los_state : 1;
		uint8_t tx_fault_state : 1;
		uint8_t soft_rate_sel_state : 1;
		uint8_t rate_sel_state : 1;
		uint8_t rs_state : 1;
		uint8_t soft_tx_dis_select : 1;
		uint8_t tx_dis_state : 1;
	};
	uint8_t rsvd;                           /* reg 111: Reserved */
	uint8_t alarm_warn_flag[6];             /* reg 112-117: Optional Alarm and Warning Flag */
	uint8_t ext_status_ctrl[2];             /* reg 118-119: Extended Module Control/Status */
	uint8_t vendor_spec[VEND_SPEC_LEN];     /* reg: 120-126: Vendor Specific */
	uint8_t opt_page_sel;                       /* reg: 127: Optional Page Select */
	uint8_t user_eeprom[VEND_USER_EEPROM_LEN];  /* reg: 128-247: User EEPROM */
	/* reg: 248-255: Vendor specific control functions */
	uint8_t vendor_ctrl[VEND_CTRL_LEN];
};

/* SFF-8472 Rev 10.4 SFP */
struct sfp_page_info {
	struct sfp_a0_page page_a0;
	struct sfp_a2_page page_a2;
};

#define QSFP_TRANS_TECH_BIT     4
#define QSFP_TRANS_OPTICAL_MAX  0x9

struct qsfp_page0_lower {
	uint8_t Identifier;             /* reg 0: Identifier (1 Byte) */
	uint8_t revision_comp;          /* reg 1: Revision Compliance */
	uint8_t status_indicator;       /* reg 2: Status Indicators */
	uint8_t tx_rx_los;              /* reg 3: Latched Tx/Rx LOS indicator */
	/* reg 4: Latched Tx input Adaptive EQ/Transmitter/Laser fault indicator */
	uint8_t tx_fault;
	uint8_t tx_rx_lol;              /* reg 5: Latched Tx/Rx CDR LOL indicator */
	uint8_t mon_intr_flags[3];      /* reg 6-8: Free Side check Interrupt Flags */
	uint8_t l_rx_pw_alarm[2];       /* reg 9-10: Latched Rx high/low power alarm/warning */
	uint8_t l_tx_bias_alarm[2];     /* reg 11-12: Latched Tx high/low bias alarm/warning */
	uint8_t l_tx_pw_alarm[2];       /* reg 13-14: Latched Tx high/low power alarm/warning */
	uint8_t rsvd0[2];               /* reg 15-16: Reserved channel check flags, set 4 */
	uint8_t rsvd1[2];               /* reg 17-18: Reserved channel check flags, set 5 */
	uint8_t rsvd_spec[3];           /* reg 19-21: Vendor Specific */
	uint8_t temperature_msb;        /* reg 22: Temperature MSB */
	uint8_t temperature_lsb;        /* reg 23: Temperature LSB */
	uint8_t rsvd2[2];               /* reg 24-25: Reserved */
	uint8_t supply_vol[2];          /* reg 26-27: Supply Voltage MSB/LSB */
	uint8_t rsvd3[6];               /* reg 28-33: Reserved */
	uint8_t rx_power[8];            /* reg 34-41: Rx Power */
	uint8_t tx_bias[8];             /* reg 42-49: Tx Bias */
	uint8_t tx_power[8];            /* reg 50-57: Tx Power */
	uint8_t rsvd4[8];               /* reg 58-65: Reserved channel check set 4 */
	uint8_t rsvd5[8];               /* reg 66-73: Reserved channel check set 5 */
	uint8_t rsvd6[8];               /* reg 74-81: Vendor Specific */
	uint8_t rsvd7[4];               /* reg 82-85: Reserved (4 Bytes) */
	uint8_t tx_disable;             /* reg 86: Read/Write bit for software disable of Tx */
	uint8_t rx_rate_sel;            /* reg 87: Rx Software rate select */
	uint8_t tx_rate_sel;            /* reg 88: Tx Software rate select */
	uint8_t rsvd8[4];               /* reg 89-92: Reserved */
	struct {                        /* reg 93: Power control */
		uint8_t pw_override : 1;
		uint8_t pw_set : 1;
		uint8_t high_pw_en : 2;
		uint8_t rsvd9 : 3;
		uint8_t sw_reset : 1;
	};
	uint8_t rsvd10[4];              /* reg 94-97: Reserved */
	uint8_t tx_rx_cdr_ctrl;         /* reg 98: Tx/Rx_CDR_control */
	uint8_t signal_ctrl;    /* reg 99: IntL/LOSL output/LPMode/TxDis input signal control */
	uint8_t tx_rx_los_mask;         /* reg 100: Masking bit for Tx/Rx LOS */
	uint8_t tx_fault_mask;          /* reg 101: Masking bit for Tx Fault */
	uint8_t tx_rx_cdr_lol_mask;     /* reg 102: Masking bit for Tx/Rx CDR Loss of Lock */
	uint8_t temp_alarm;             /* reg 103: Masking bit for high/low-temperature alarm */
	uint8_t vcc_alarm;              /* reg 104: Masking bit for Vcc high/low alarm */
	uint8_t rsvd11[2];              /* reg 105-106: Vendor Specific */
	uint8_t fs_dev_properties[12];  /* reg 107-118: Free Side Device Properties */
	uint8_t pwd_entry_chge[8];      /* reg 119-126: Password Entry and Change */
	uint8_t page_select;            /* reg 127: Page Select */
};

struct qsfp_page0_upper {
	uint8_t identifier;             /* reg 128: Identifier Type of serial transceiver */
	struct {                        /* reg 129: Extended identifier of serial transceiver */
		uint8_t pw_class_5_7 : 2;
		uint8_t rx_cdr_present : 1;
		uint8_t tx_cdr_present : 1;
		uint8_t clei_code_present : 1;
		uint8_t pw_class_8_imp : 1;
		uint8_t pw_class_1_4 : 2;
	};
	uint8_t connector;              /* reg 130: Code for connector type */
	/* reg 131-138: Code for electronic
	 * compatibility or optical compatibility
	 */
	uint8_t spec_compliance[SFF_TRANCEIV_LEN];
	uint8_t encoding;               /* reg 139: Code for serial encoding algorithm */
	uint8_t br_nominal;             /* reg 140: Nominal bit rate, units of 100 MBits/s. */
	uint8_t ext_rate_sel;           /* reg 141: Tags for Extended RateSelect compliance */
	uint8_t smf_len_km;             /* reg 142: Link length supported for SMF fiber in km */
	/* reg 143: Link length supported for EBW 50/125 fiber, units of 2 m */
	uint8_t om3_len;
	/* reg 144: Link length supported for 50/125 fiber, units of 1 m */
	uint8_t om2_len;
	/* reg 145: Link length supported for 62.5/125 fiber, units of 1 m */
	uint8_t om1_len;
	/* reg 146: Link length supported for copper, units of 1m */
	uint8_t om4_cable_copper_len;
	uint8_t device_technology;              /* reg 147: Device technology */
	uint8_t vendor_name[VEND_NAME_LEN];     /* reg 148-163: QSFP vendor name (ASCII) */
	uint8_t extended_module;        /* reg 164: Extended Transceiver Codes for InfiniBand */
	uint8_t vendor_oui[VEND_OUI_LEN];       /* reg 165-167: QSFP vendor IEEE company ID */
	/* reg 168-183: Part number provided by QSFP vendor (ASCII) */
	uint8_t vendor_pn[VEND_PN_LEN];
	/* reg 184-185: Revision level for part number provided by vendor (ASCII) */
	uint8_t vendor_rev[QSFP_VEND_REV_LEN];
	/* reg 186-187: Nominal laser wavelength (Wavelength = value / 20 in nm) */
	uint8_t wavelength[2];
	/* reg 188-189: Guaranteed range of laser wavelength (+/- value) from
	 * Nominal wavelength.(Wavelength Tol. = value/200 in nm)
	 */
	uint8_t wavelength_tolerance[2];
	uint8_t max_case_temp;  /* reg 190: Maximum Case Temperature in Degrees C. */
	uint8_t cc_base;        /* reg 191: Check code for Base ID Fields (addresses 128-190) */
	uint8_t link_codes;     /* reg 192: Extended Specification Compliance Codes */
	/* reg 193-194: Optional features implemented */
	uint8_t option[2];
	/* reg 195: Rate Select, TX Disable, TX Fault, LOS */
	struct {
		uint8_t page_20_21h_imp : 1;
		uint8_t tx_loss_sign_imp : 1;
		uint8_t tx_squelch_imp : 1;
		uint8_t tx_fault_imp : 1;
		uint8_t tx_dis_imp : 1;
		uint8_t rate_sel_imp : 1;
		uint8_t page_01h_imp : 1;
		uint8_t page_02h_imp : 1;
	};
	/* reg 196-211: Serial number provided by vendor (ASCII) */
	uint8_t vendor_sn[VEND_SN_LEN];
	/* reg 212-219: Vendor's manufacturing date code */
	uint8_t date_code[VEND_DATE_CODE_LEN];
	/* reg 220: Indicates which type of diagnostic check is implemented
	 * (if any) in the transceiver. Bit 1, 0 Reserved
	 */
	uint8_t ddm_type;
	/* reg 221: Indicates which optional enhanced features are
	 * implemented in the transceiver.
	 */
	uint8_t enhanced_option;
	/* reg 222: Reserved */
	uint8_t br_nominal_ext;
	/* reg 223: Check code for the Extended ID Fields (addresses 192-222) */
	uint8_t cc_ext;
	/* reg 224-255 */
	uint8_t reserved_a0_up1[32];
};

struct qsfp_page0_info {
	struct qsfp_page0_lower page_lower;
	struct qsfp_page0_upper page_upper;
};

enum cmis_media_type {
	UNDEFINED = 0,
	OPT_MMF,
	OPT_SMF,
	PASSIVE_COPPER,
	ACTIVE_CABLE,
	BASE_T,
	MEDIA_TYPE_RSVD,
};

#define CMIS_LOW_MEM_APP_DESC_NUM	8
struct cmis_app_desc {
	uint8_t host_id;                 /* host electrical interface id */
	uint8_t media_id;                /* module media electrical interface id */
	uint8_t media_lane_cnt : 4,
		host_lane_cnt : 4;       /* host and media lane counts */
	uint8_t host_assign;             /*  Host Lane Assignment Options */
};

struct cmis_page0_lower {
	uint8_t identifier;              /* reg 0: Identifier */
	uint8_t rev_compliance;          /* reg 1: CMIS revision */
	/* reg 2: Module Management Characteristics */
	uint8_t rsv0 : 2,
		mci_max_speed : 2,
		rsv1 : 2,
		step_cfg_only : 1,
		mem_model : 1;
	/* reg 3: Global Status Information */
	uint8_t intr_deasserted : 1,
		module_state : 3,
		rsv2 : 4;
	uint8_t flags_sum[4];            /* reg 4-7: Lane-Level Flags Summary */
	uint8_t module_flags[6];         /* reg 8-13: Module-Level Flags */
	uint8_t module_temp[2];          /* reg 14-15: TempMonValue */
	uint8_t module_vcc[2];           /* reg 16-17: VccMonVoltage */
	uint8_t module_mon_val[8];       /* reg 18-25: Module-Level Mon Value */
	/* reg 26: Module Global Controls */
	uint8_t rsv3 : 3,
		sw_reset : 1,
		lowpwr_req_sw : 1,
		squ_method_sel : 1,
		lowpwr_allow_req_hw : 1,
		bank_bc_enable : 1;
	uint8_t rsv4[14];                /* reg 27-40:  */
	uint8_t module_fault;            /* reg 41: Module Fault Information */
	uint8_t rsv5[22];                /* reg 42-63: Reserved */
	uint8_t custom[21];              /* reg 64-84: Custom */
	uint8_t media_type;              /* reg 85: Media Type Encodings */
	/* reg 86-117: Application Descriptor */
	struct cmis_app_desc apps[CMIS_LOW_MEM_APP_DESC_NUM];
	uint8_t pwd_area[8];             /* reg 118-125: Password Facilities */
	uint8_t bank_sel;                /* reg 126: Bank Index of Page mapped to Upper Memory */
	uint8_t page_sel;                /* reg 127: Page Index of Page mapped to Upper Memory */
};

struct cmis_page0_upper {
	uint8_t identifier_cp;           /* reg 128: The Same Byte 00h:0 */
	uint8_t vend_name[16];           /* reg 129-144: Vendor name (ASCII) */
	uint8_t vend_oui[3];             /* reg 145-147: Vendor IEEE company ID */
	uint8_t vend_pn[16];             /* reg 148-163: Part number provided by vendor (ASCII) */
	/* reg 164-165: Revision level for part number provided by vendor (ASCII) */
	uint8_t vend_rev[2];
	uint8_t vend_sn[16];             /* reg 166-181: Vendor Serial Number (ASCII) */
	uint8_t date_code[8];            /* reg 182-189: Manufacturing Date Code (ASCII) */
	/* reg 190-199: Common Language Equipment Identification Code (ASCII) */
	uint8_t clei_code[10];
	uint8_t module_pwr_class;        /* reg 200: Module Power Class */
	/* reg 201: Maximum power consumption in multiples of 0.25 W
	 * rounded up to the next whole multiple of 0.25 W
	 */
	uint8_t max_power;
	/* reg 202: Cable Assembly Link Length */
	uint8_t cab_base_len : 6,
		len_multiplier : 2;
	uint8_t connector_type;          /* reg 203: Media Connector Type */
	uint8_t copp_attenuation[6];     /* reg 204-209: Copper Cable Attenuation */
	uint8_t media_lanes;             /* reg 210: Media Lane Information */
	uint8_t cable_assembly_lane;     /* reg 211: Cable Assembly Lane Information */
	uint8_t media_int_tech;          /* reg 212: Media Interface Technology */
	uint8_t rsv0[8];                 /* reg 213-220: Reserved */
	uint8_t rsv1;                    /* reg 221: Custom1 */
	uint8_t page_check_sum;          /* reg 222: Page Checksum over bytes 128-221 */
	uint8_t rsv[33];                 /* reg 223-255: Custom Info (non-volatile) */
};

struct cmis_page1_info {
	uint8_t inac_fw_hw_ver[4];       /* reg 128-131: Inactive FW revision and HW revision */
	uint8_t smf_len : 6,             /* reg 132: Base link length for SMF fiber in km */
		smf_len_multip : 2;      /* Link length multiplier for SMF fiber */
	uint8_t om5_len;                 /* reg 133: Link length supported for OM5 fiber */
	uint8_t om4_len;                 /* reg 134: Link length supported for OM4 fiber */
	uint8_t om3_len;         /* reg 135: Link length supported for EBW 50/125 µm fiber (OM3) */
	uint8_t om2_len;         /* reg 136: Link length supported for 50/125 µm fiber (OM2) */
	uint8_t rsv0;                    /* reg 137: Reserved */
	uint8_t nominal_wave_len[2];     /* reg 138-139: NominalWavelength */
	uint8_t wave_len_tolerance[2];   /* reg 140-141: WavelengthTolerance */
	uint8_t pages_support;           /* reg 142: Supported Pages Advertising */
	uint8_t duration_adv[2];         /* reg 143-144: Durations Advertising */
	uint8_t module_char[10];         /* reg 145-154: Module Characteristics Advertising */
	uint8_t contrl_support[2];       /* reg 155-156: Supported Controls Advertisement */
	uint8_t flags_support[2];        /* reg 157-158: Supported Flags Advertisement */
	/* reg 159: Supported Mon Advertisement */
	uint8_t temp_mon_supp : 1,
		vcc_mon_supp : 1,
		aux1_mon_supp : 1,
		aux2_mon_supp : 1,
		aux3_mon_supp : 1,
		custom_mon_supp : 1,
		rsv1 : 2;
	/* reg 160: Supported Power Mon Advertisement */
	uint8_t txbias_mon_supp : 1,
		tx_pwr_mon_supp : 1,
		rx_pwr_mon_supp : 1,
		txbias_curr_scal : 2,
		rsv2 : 3;
	/* reg 161-162: Supported Configuration and Signal Integrity Controls Advertisement */
	uint8_t sig_intr_support[2];
	uint8_t cdb_func_support[4];     /* reg 163-166: CDB Messaging Support Advertisement */
	uint8_t add_dura_adv[3];         /* reg 167-169: Additional Durations Advertising */
	uint8_t rsv3[7];                 /* reg 170-175: Reserved */
	uint8_t media_lane_adv[15];      /* reg 176-190: Media Lane Assignment Advertising */
	uint8_t custom[32];              /* reg 191-222: Custom */
	uint8_t add_app_desc[28];    /* reg 223-250: Additional Application Descriptor Registers */
	uint8_t rsv4[4];                 /* reg 251-254: Reserved */
	uint8_t page_check_sum;          /* reg 255: Page Checksum */
};

/* Current support max 640 bytes data */
struct cmis_page_info {
	struct cmis_page0_lower page0_lower;
	struct cmis_page0_upper page0_upper;
	struct cmis_page1_info page1;
	uint8_t page2_data[128];
	uint8_t page3_data[128];
};

#define XSFP_TARGET_BIT     HI_BIT(0)
#define XSFP_RAW_DATA_BIT   HI_BIT(1)

enum hikp_port_media_type {
	MEDIA_TYPE_UNKNOWN = 0,
	MEDIA_TYPE_FIBER,
	MEDIA_TYPE_PHY,
	MEDIA_TYPE_BACKPLANE,
};

enum hikp_xsfp_present {
	MODULE_ABSENT = 0,
	MODULE_PRESENT,
};

struct hikp_xsfp_basic {
	uint32_t media_type;        /* only fiber type port support get eeprom data */
	uint32_t present_status;    /* optical module is or not insert */
	uint32_t data_size;         /* eeprom data size, unit Byte */
	uint32_t total_blk_num;
};

struct hikp_xsfp_req {
	struct bdf_t bdf;
	uint32_t blk_id;
};

struct hikp_xsfp_ctrl {
	struct tool_target target;
	uint32_t dump_param;
};

#endif
