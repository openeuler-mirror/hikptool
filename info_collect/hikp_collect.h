/*
 * Copyright (c) 2024 Hisilicon Technologies Co., Ltd.
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

#ifndef HIKP_COLLECT_H
#define HIKP_COLLECT_H

#define GROUP_SAS "sas"
#define GROUP_SATA "sata"
#define GROUP_ACC "acc"
#define GROUP_SOCIP "socip"
#define GROUP_NIC "nic"
#define GROUP_ROCE "roce"
#define GROUP_PCIE "pcie"
#define GROUP_IMP "imp"
#define GROUP_COMMON "common"
#define GROUP_SERDES "serdes"

enum info_collect_type {
	COLLECT_ACC,
	COLLECT_IMP,
	COLLECT_NIC,
	COLLECT_PCIE,
	COLLECT_ROCE,
	COLLECT_SAS,
	COLLECT_SATA,
	COLLECT_SERDES,
	COLLECT_SOCIP,
	COLLECT_ALL,
	COLLECT_UNKNOWN_TYPE,
};

void collect_sas_log(void);
void collect_sata_log(void);
void collect_acc_log(void);
void collect_socip_log(void);
void collect_common_log(void);
void collect_nic_log(void);
void collect_roce_log(void);
void collect_pcie_info(void);
void collect_imp_log(void);
void collect_serdes_log(void);

#endif /* HIKP_COLLECT_H */
