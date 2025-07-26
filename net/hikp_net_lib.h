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
#ifndef HIKP_NET_LIB_H
#define HIKP_NET_LIB_H

#include "ossl_user_linux.h"
#include "tool_cmd.h"
#include "hikptdev_plug.h"

struct bdf_t {
	uint32_t domain;
	/* bdf_id ===> fun_id: bit0~bit7, device_id: bit8~bit15, bus_id: bit16~bit23 */
	union {
		uint32_t bdf_id;
		struct {
			uint8_t fun_id;
			uint8_t dev_id;
			uint8_t bus_id;
			uint8_t rsv;
		};
	};
};

struct tool_target {
	char dev_name[IFNAMSIZ]; /* *net device name* */
	struct bdf_t bdf;
};

#define NET_MAX_REQ_DATA_NUM 30

#define HNS3_DRIVER_NAME "hns3"

#define HIKP_NET_DEV_PATH "/sys/class/net/"
#define HIKP_NET_PATH "/net/"
#define HIKP_SRIOV_NUMVFS_PATH "/sriov_numvfs"
#define HIKP_VIRTFN_PATH "/device/virtfn"
#define HIKP_PHYSFN_PATH "/device/physfn/net/"

#define MAX_BUS_PCI_DIR_LEN 300
#define MAX_PCI_ID_LEN 6
#define HIKP_BUS_PCI_DEV_DIR "/sys/bus/pci/devices/"
#define HIKP_PCI_REVISION_DIR "/revision"

#define PCI_MAX_DIR_NAME_LEN 30

#define PCI_ID_FOUR_NUMS 4
#define PCI_ID_THREE_NUMS 3

#define PCI_BUS_ID_OFF 16
#define PCI_DEV_ID_OFF 8
#define PCI_FUNC_ID_OFF 0

#define PCI_DOMAIN_MASK 0xFFFF
#define PCI_BUS_ID_MASK 0xFF
#define PCI_DEV_ID_MASK 0x1F
#define PCI_FUNC_ID_MASK 0x7

#define MIN_SOCKFD 3

enum nic_cmd_type {
	GET_CHIP_INFO_CMD = 0x1,
	GET_FW_LOG_INFO_CMD,
	GET_DFX_INFO_CMD,
	GET_RSS_INFO_CMD,
	GET_QOS_INFO_CMD,
	GET_QUEUE_INFO_CMD,
	GET_PPP_INFO_CMD,
	GET_FD_INFO_CMD,
	GET_FEC_INFO_CMD,
	GET_GRO_INFO_CMD,
	GET_NCSI_INFO_CMD,
	GET_NOTIFY_PKT_CMD,
	GET_TORUS_INFO_CMD = 0xD,
	GET_PORT_FAULT_STATUS = 0xE,
};

enum roh_cmd_type {
	HIKP_ROH_MAC = 0x1,
	HIKP_ROH_SHOW_MIB = 0x2,
	HIKP_ROH_SHOW_BP = 0x3,
};

enum roce_cmd_type {
	GET_ROCEE_MDB_CMD = 0x1,
	GET_ROCEE_GMV_CMD,
	GET_ROCEE_CAEP_CMD,
	GET_ROCEE_PKT_CMD,
	GET_ROCEE_SCC_CMD,
	GET_ROCEE_QMM_CMD,
	GET_ROCEE_TIMER_CMD,
	GET_ROCEE_TRP_CMD,
	GET_ROCEE_TSP_CMD,
	GET_ROCEE_RST_CMD,
	GET_ROCEE_GLOBAL_CFG_CMD,
	GET_ROCEE_BOND_CMD,
	GET_ROCEE_DFX_STA_CMD,
};

enum ub_cmd_type {
	GET_UNIC_PPP_CMD = 0x1,
	GET_UB_DFX_INFO_CMD,
	GET_UB_LINK_INFO_CMD,
	GET_UB_BP_INFO_CMD,
	GET_UB_CRD_INFO_CMD,
	GET_UB_BASIC_INFO_CMD,
};

enum nic_get_ncsi_sub_cmd {
	NIC_NCSI_GET_DFX_INFO,
};

#define HIKP_MAX_PF_NUM 8
#define HIKP_NIC_MAX_FUNC_NUM   256

#define HIKP_NIC_MAX_TC_NUM 8
#define HIKP_NIC_MAX_USER_PRIO_NUM 8

#define HIKP_NIC_ETH_MAC_ADDR_LEN   6
#define HIKP_NIC_ETH_ADDR_FMT_SIZE  18

int hikp_net_creat_sock(void);
int tool_check_and_get_valid_bdf_id(const char *name, struct tool_target *target);
bool is_dev_valid_and_special(int sockfd, struct tool_target *target);
int get_revision_id_by_bdf(const struct bdf_t *bdf, char *revision_id, size_t id_len);
int get_dev_name_by_bdf(const struct bdf_t *bdf, char *dev_name, size_t name_len);
int get_numvfs_by_bdf(const struct bdf_t *bdf, uint8_t *numvfs);
int get_vf_dev_info_by_pf_dev_name(const char *pf_dev_name,
				   struct tool_target *vf_target, uint8_t vf_id);
int get_pf_dev_info_by_vf_dev_name(const char *vf_dev_name, struct tool_target *pf_target);
void hikp_ether_format_addr(char *buf, uint16_t size, const uint8_t *mac_addr, uint8_t mac_len);

#endif /* HIKP_NET_LIB_H */
