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
#include <sys/types.h>
#include <unistd.h>

#include "hikp_nic_ppp.h"

static struct hikp_nic_ppp_hw_resources g_ppp_hw_res = { 0 };
static struct nic_ppp_param g_ppp_param = { 0 };

static void hikp_nic_ppp_show_mac_tbl(const void *data);
static void hikp_nic_ppp_show_vlan_tbl(const void *data);
static void hikp_nic_ppp_show_promisc_tbl(const void *data);
static void hikp_nic_ppp_show_manager_tbl(const void *data);
static void hikp_nic_ppp_show_vlan_offload(const void *data);

static int hikp_nic_query_ppp_by_blkid(struct hikp_cmd_header *req_header, const struct bdf_t *bdf,
				       void *data, size_t len);
static int hikp_nic_query_ppp_by_entryid(struct hikp_cmd_header *req_header,
					 const struct bdf_t *bdf,
					 void *data, size_t len);

#define HIKP_NIC_FUNC_NAME_LEN    8
#define HIKP_NIC_PPP_FUNC_BITMAP_SIZE       32

#define HIKP_NIC_PPP_VLAN_ID_NUM_PER_LEN    6

#define NIC_PPP_MAC_TBL_NAME    "mac"
#define NIC_PPP_VLAN_TBL_NAME   "vlan"
#define NIC_PPP_MNG_TBL_NAME    "mng"

#define HIKP_PPP_MAX_MAC_ID_NUM	8

static const struct ppp_feature_cmd g_ppp_feature_cmd[] = {
	{NIC_PPP_MAC_TBL_NAME, NIC_MAC_TBL_DUMP,  true,
	 hikp_nic_query_ppp_by_entryid, hikp_nic_ppp_show_mac_tbl},
	{NIC_PPP_VLAN_TBL_NAME, NIC_VLAN_TBL_DUMP, true,
	 hikp_nic_query_ppp_by_entryid, hikp_nic_ppp_show_vlan_tbl},
	{NIC_PPP_MNG_TBL_NAME, NIC_MNG_TBL_DUMP,   true,
	 hikp_nic_query_ppp_by_entryid, hikp_nic_ppp_show_manager_tbl},
	{"promisc",      NIC_PROMISCUOUS_TBL_DUMP, false,
	 hikp_nic_query_ppp_by_blkid,   hikp_nic_ppp_show_promisc_tbl},
	{"vlan_offload", NIC_VLAN_OFFLOAD_DUMP,    false,
	 hikp_nic_query_ppp_by_blkid,   hikp_nic_ppp_show_vlan_offload},
};

static int hikp_nic_ppp_cmd_help(struct major_cmd_ctrl *self, const char *argv)
{
	printf("\n  Usage: %s %s\n", self->cmd_ptr->name, "-i <device>");
	printf("\n         %s\n", self->cmd_ptr->help_info);
	printf("\n  Options:\n\n");
	printf("    %s, %-25s %s\n", "-h", "--help", "display this help and exit");
	printf("	%s, %-25s %s\n", "-i", "--interface=<interface>",
	       "device target or bdf id, e.g. eth0~7 or 0000:35:00.0");
	printf("%s\n",
	       "      [-du/--dump mac -func/--func_id <func_id> -uc/--unicast <1/0>]\n"
	       "              dump MAC table info.\n"
	       "              dump unicast/multicast MAC address for a function.\n"
	       "      [-du/--dump vlan -func/--function <func_no>]\n"
	       "              dump VLAN table info.\n"
	       "              dump VLAN of a function.\n"
	       "      [-du/--dump mng]\n"
	       "              dump manager table info.\n"
	       "      [-du/--dump promisc]\n"
	       "              dump promiscuous info.\n"
	       "      [-du/--dump vlan_offload]\n"
	       "              dump VLAN offload info.\n");

	return 0;
}

static uint16_t hikp_nic_ppp_get_abs_func_id(const struct bdf_t *bdf, int relative_func_id)
{
	uint16_t abs_func_id;

	if (relative_func_id == 0)
		abs_func_id = bdf->fun_id;
	else
		abs_func_id = g_ppp_hw_res.abs_func_id_base + relative_func_id - 1;

	return abs_func_id;
}

static void hikp_nic_ppp_get_func_name(char *buf, uint8_t len, uint8_t id)
{
	if (id > 0)
		snprintf(buf, len, "vf%u", id - 1);
	else
		snprintf(buf, len, "pf");
}

static void hikp_nic_ppp_get_overflow_mac(struct nic_mac_tbl *of_tbl, struct nic_mac_tbl *tbl)
{
	struct mac_vlan_uc_tbl *of_uc_tbl = &of_tbl->uc_tbl;
	struct mac_vlan_mc_tbl *of_mc_tbl = &of_tbl->mc_tbl;
	struct mac_vlan_uc_tbl *uc_tbl = &tbl->uc_tbl;
	struct mac_vlan_mc_tbl *mc_tbl = &tbl->mc_tbl;
	uint32_t idx = 0;
	uint32_t i;

	for (i = 0; i < uc_tbl->entry_size; i++) {
		struct mac_vlan_uc_entry *entry = &uc_tbl->entry[i];

		if (entry->idx >= g_ppp_hw_res.max_key_mem_size)
			of_uc_tbl->entry[idx++] = *entry;
	}
	of_uc_tbl->entry_size = idx;

	idx = 0;
	for (i = 0; i < mc_tbl->entry_size; i++) {
		struct mac_vlan_mc_entry *entry = &mc_tbl->entry[i];

		if (entry->idx >= g_ppp_hw_res.max_key_mem_size)
			of_mc_tbl->entry[idx++] = *entry;
	}
	of_mc_tbl->entry_size = idx;
}

static void hikp_nic_ppp_show_key_mem(struct nic_mac_tbl *tbl, bool is_key_mem)
{
	char mac_str[HIKP_NIC_ETH_ADDR_FMT_SIZE] = {0};
	struct mac_vlan_uc_tbl *uc_tbl = &tbl->uc_tbl;
	struct mac_vlan_mc_tbl *mc_tbl = &tbl->mc_tbl;
	struct mac_vlan_uc_entry *uc_entry;
	struct mac_vlan_mc_entry *mc_entry;
	uint32_t idx;

	printf("%s[total_entry_size=%u]:\n", is_key_mem ? "Key mem" : "Overflow cam",
		   is_key_mem ? g_ppp_hw_res.max_key_mem_size : g_ppp_hw_res.overflow_cam_size);
	printf("Unicast MAC table[entry number=%u]:\n", uc_tbl->entry_size);
	printf("index | valid | mac_addr          | "
	       "vlan_id | VMDq1 | mac_en | in_port(mac_id) | E_vPort_type | E_vPort\n");
	for (idx = 0; idx < uc_tbl->entry_size; idx++) {
		uc_entry = &uc_tbl->entry[idx];
		if (uc_entry->idx < g_ppp_hw_res.max_key_mem_size || !is_key_mem) {
			hikp_ether_format_addr(mac_str,
					       HIKP_NIC_ETH_ADDR_FMT_SIZE, uc_entry->mac_addr,
					       HIKP_NIC_ETH_MAC_ADDR_LEN);
			printf("%04u  | %01u     | %s | ", uc_entry->idx, uc_entry->valid, mac_str);
			printf("%04u    | %u     | %01u      | ",
			       uc_entry->vlan_id, uc_entry->vmdq1, uc_entry->mac_en);
			printf("%01u               | %01u            | %06x",
			       uc_entry->ingress_port, uc_entry->e_vport_type, uc_entry->e_vport);
			printf("\n");
		}
	}

	printf("Multicast MAC table[entry number=%u]:\n", mc_tbl->entry_size);
	printf("index | mac_addr          | func bitMap[255  <--  0]\n");
	for (idx = 0; idx < mc_tbl->entry_size; idx++) {
		mc_entry = &mc_tbl->entry[idx];
		if (mc_entry->idx < g_ppp_hw_res.max_key_mem_size || !is_key_mem) {
			hikp_ether_format_addr(mac_str,
					       HIKP_NIC_ETH_ADDR_FMT_SIZE, mc_entry->mac_addr,
					       HIKP_NIC_ETH_MAC_ADDR_LEN);
			printf("%04u  | %s | ", mc_entry->idx, mac_str);
			printf("%08x:%08x:%08x:%08x:%08x:%08x:%08x:%08x",
			       mc_entry->function_bitmap[7], mc_entry->function_bitmap[6],
			       mc_entry->function_bitmap[5], mc_entry->function_bitmap[4],
			       mc_entry->function_bitmap[3], mc_entry->function_bitmap[2],
			       mc_entry->function_bitmap[1], mc_entry->function_bitmap[0]);
			printf("\n");
		}
	}
}

static void hikp_nic_ppp_show_func_uc_mac_addr(struct mac_vlan_uc_tbl *uc_tbl,
					       const struct bdf_t *bdf, uint16_t func_id)
{
	char mac_str[HIKP_NIC_ETH_ADDR_FMT_SIZE] = {0};
	struct mac_vlan_uc_entry *uc_entry;
	uint8_t pf_id;
	uint8_t vf_id;
	uint8_t i;

	for (i = 0; i < uc_tbl->entry_size; i++) {
		uc_entry = &uc_tbl->entry[i];
		if (uc_entry->idx >= g_ppp_hw_res.max_key_mem_size)
			break;

		pf_id = hikp_get_field(uc_entry->e_vport, HIKP_NIC_PF_ID_MASK, HIKP_NIC_PF_ID_S);
		vf_id = hikp_get_field(uc_entry->e_vport, HIKP_NIC_VF_ID_MASK, HIKP_NIC_VF_ID_S);
		if (bdf->fun_id == pf_id && vf_id == func_id) {
			hikp_ether_format_addr(mac_str, HIKP_NIC_ETH_ADDR_FMT_SIZE,
					       uc_entry->mac_addr, HIKP_NIC_ETH_MAC_ADDR_LEN);
			printf("\t%s\n", mac_str);
		}
	}
}

static void hikp_nic_ppp_show_func_mc_mac_addr(struct mac_vlan_mc_tbl *mc_tbl,
					       const struct bdf_t *bdf, uint16_t func_id)
{
	char mac_str[HIKP_NIC_ETH_ADDR_FMT_SIZE] = {0};
	struct mac_vlan_mc_entry *mc_entry;
	uint16_t abs_func_id;
	uint8_t offset;
	uint8_t idx;
	uint32_t i;

	abs_func_id = hikp_nic_ppp_get_abs_func_id(bdf, func_id);
	idx = abs_func_id / HIKP_NIC_PPP_FUNC_BITMAP_SIZE;
	offset = abs_func_id % HIKP_NIC_PPP_FUNC_BITMAP_SIZE;

	for (i = 0; i < mc_tbl->entry_size; i++) {
		mc_entry = &mc_tbl->entry[i];
		if (mc_entry->idx >= g_ppp_hw_res.max_key_mem_size)
			break;

		if (hikp_get_bit(mc_entry->function_bitmap[idx], offset) != 0) {
			hikp_ether_format_addr(mac_str, HIKP_NIC_ETH_ADDR_FMT_SIZE,
					       mc_entry->mac_addr, HIKP_NIC_ETH_MAC_ADDR_LEN);
			printf("\t%s\n", mac_str);
		}
	}
}

static void hikp_nic_ppp_show_func_mac(struct nic_mac_tbl *tbl,
				       const struct nic_ppp_param *ppp_param)
{
	const struct bdf_t *bdf = &ppp_param->target.bdf;
	char func_name[HIKP_NIC_FUNC_NAME_LEN];
	uint16_t abs_func_id;

	abs_func_id = hikp_nic_ppp_get_abs_func_id(bdf, ppp_param->func_id);
	hikp_nic_ppp_get_func_name(func_name, sizeof(func_name), ppp_param->func_id);
	printf("%s_abs_func_id=%u\n", func_name, abs_func_id);
	printf("%s %s MAC addrs:\n", func_name, ppp_param->is_uc == 1 ? "unicast" : "multicast");
	if (ppp_param->is_uc == 1) {
		hikp_nic_ppp_show_func_uc_mac_addr(&tbl->uc_tbl, bdf, ppp_param->func_id);
		return;
	}

	hikp_nic_ppp_show_func_mc_mac_addr(&tbl->mc_tbl, bdf, ppp_param->func_id);
}

static void hikp_nic_ppp_show_mac_tbl(const void *data)
{
	struct nic_mac_tbl overflow_tbl = {0};
	struct mac_vlan_uc_entry *of_uc_entry;
	struct mac_vlan_mc_entry *of_mc_entry;

	/* Display unicast or multicast MAC address in the specified function. */
	if (g_ppp_param.func_id != -1) {
		hikp_nic_ppp_show_func_mac((struct nic_mac_tbl *)data, &g_ppp_param);
		return;
	}

	hikp_nic_ppp_show_key_mem((struct nic_mac_tbl *)data, true);

	if (g_ppp_hw_res.overflow_cam_size == 0)
		return;

	of_uc_entry = (struct mac_vlan_uc_entry *)calloc(g_ppp_hw_res.overflow_cam_size,
		      sizeof(struct mac_vlan_uc_entry));
	if (of_uc_entry == NULL) {
		HIKP_ERROR_PRINT("Failed to alloc memory for overflow uc table.\n");
		return;
	}

	of_mc_entry = (struct mac_vlan_mc_entry *)calloc(g_ppp_hw_res.overflow_cam_size,
		      sizeof(struct mac_vlan_mc_entry));
	if (of_mc_entry == NULL) {
		HIKP_ERROR_PRINT("Failed to alloc memory for overflow mc table.\n");
		free(of_uc_entry);
		return;
	}

	overflow_tbl.uc_tbl.entry = of_uc_entry;
	overflow_tbl.mc_tbl.entry = of_mc_entry;
	hikp_nic_ppp_get_overflow_mac(&overflow_tbl, (struct nic_mac_tbl *)data);
	hikp_nic_ppp_show_key_mem(&overflow_tbl, false);

	free(of_uc_entry);
	free(of_mc_entry);
}

static void hikp_nic_ppp_show_port_vlan_info(const struct port_vlan_tbl *port_tbl,
					     const struct hikp_nic_ppp_hw_resources *hw_res)
{
	struct port_vlan_tbl_entry *port_entry;
	uint32_t mac_id = hw_res->mac_id;
	uint8_t vlan_cnt = 0;
	uint32_t i;

	printf("mac_id=%u\n", hw_res->mac_id);
	printf("total_func_num=%u\n", hw_res->total_func_num);
	printf("abs_func_id_base=%u\n", hw_res->abs_func_id_base);
	printf("port VLAN id:\n\t");
	for (i = 0; i < port_tbl->entry_size; i++) {
		port_entry = &port_tbl->entry[i];
		if (hikp_get_bit(port_entry->port_bitmap, mac_id) != 0) {
			printf("%4u ", port_entry->vlan_id);
			vlan_cnt++;
			if (vlan_cnt == HIKP_NIC_PPP_VLAN_ID_NUM_PER_LEN) {
				printf("\n\t");
				vlan_cnt = 0;
			}
		}
	}
	printf("\n");
}

static void hikp_nic_ppp_show_vf_vlan_info(const struct vf_vlan_tbl *vf_tbl, uint16_t func_id,
					   const struct bdf_t *bdf,
					   const struct hikp_nic_ppp_hw_resources *hw_res)
{
	char func_name[HIKP_NIC_FUNC_NAME_LEN];
	struct vf_vlan_tbl_entry *vf_entry;
	uint16_t abs_func_id;
	uint8_t vlan_cnt = 0;
	uint8_t offset;
	uint8_t idx;
	uint32_t i;

	abs_func_id = hikp_nic_ppp_get_abs_func_id(bdf, func_id);
	idx = abs_func_id / HIKP_NIC_PPP_FUNC_BITMAP_SIZE;
	offset = abs_func_id % HIKP_NIC_PPP_FUNC_BITMAP_SIZE;
	hikp_nic_ppp_get_func_name(func_name, sizeof(func_name), func_id);
	printf("%s_abs_func_id: %u\n", func_name,
	       (uint32_t)(hw_res->abs_func_id_base + func_id - 1));
	printf("%s VLAN id:\n\t", func_name);

	for (i = 0; i < vf_tbl->entry_size; i++) {
		vf_entry = &vf_tbl->entry[i];
		if (hikp_get_bit(vf_entry->func_bitmap[idx], offset) != 0) {
			printf("%4u ", vf_entry->vlan_id);
			vlan_cnt++;
			if (vlan_cnt == HIKP_NIC_PPP_VLAN_ID_NUM_PER_LEN) {
				printf("\n\t");
				vlan_cnt = 0;
			}
		}
	}
	printf("\n");
}

static void hikp_nic_ppp_show_func_vlan(const struct nic_vlan_tbl *vlan_tbl,
					const struct nic_ppp_param *ppp_param)
{
	const struct port_vlan_tbl *port_tbl = &vlan_tbl->port_vlan_tbl;
	const struct vf_vlan_tbl *vf_tbl = &vlan_tbl->vf_vlan_tbl;
	const struct bdf_t *bdf = &ppp_param->target.bdf;

	/* VF VLAN filter table isn't configured by DPDK driver,
	 * so we have to get VLAN id by port VLAN table.
	 */
	if (ppp_param->func_id == 0) {
		hikp_nic_ppp_show_port_vlan_info(port_tbl, &g_ppp_hw_res);
		return;
	}

	hikp_nic_ppp_show_vf_vlan_info(vf_tbl, ppp_param->func_id, bdf, &g_ppp_hw_res);
}

static void hikp_nic_ppp_show_vlan_tbl(const void *data)
{
	struct nic_vlan_tbl *tbl = (struct nic_vlan_tbl *)data;
	struct port_vlan_tbl *port_tbl = &tbl->port_vlan_tbl;
	struct vf_vlan_tbl *vf_tbl = &tbl->vf_vlan_tbl;
	uint32_t i;

	/* Display vlan information in the specified function. */
	if (g_ppp_param.func_id != -1) {
		hikp_nic_ppp_show_func_vlan(tbl, &g_ppp_param);
		return;
	}

	printf("port_vlan_table_size=%u\n", g_ppp_hw_res.port_vlan_tbl_size);
	printf("vf_vlan_table_size=%u\n", g_ppp_hw_res.vf_vlan_tbl_size);
	printf("vlan_id | vf filter bitmap[func_255 <--- func_0]\n");
	for (i = 0; i < vf_tbl->entry_size; i++) {
		printf(" %04u  | %08x:%08x:%08x:%08x:%08x:%08x:%08x:%08x\n",
		       vf_tbl->entry[i].vlan_id,
		       vf_tbl->entry[i].func_bitmap[7], vf_tbl->entry[i].func_bitmap[6],
		       vf_tbl->entry[i].func_bitmap[5], vf_tbl->entry[i].func_bitmap[4],
		       vf_tbl->entry[i].func_bitmap[3], vf_tbl->entry[i].func_bitmap[2],
		       vf_tbl->entry[i].func_bitmap[1], vf_tbl->entry[i].func_bitmap[0]);
	}

	printf("vlan_id | port filter bitmap(based on mac_id)\n");
	for (i = 0; i < port_tbl->entry_size; i++)
		printf(" %04u   | %02x\n",
		       port_tbl->entry[i].vlan_id, port_tbl->entry[i].port_bitmap);
}

static void hikp_nic_ppp_show_manager_tbl(const void *data)
{
	struct nic_mng_tbl *tbl = (struct nic_mng_tbl *)data;
	char mac_str[HIKP_NIC_ETH_ADDR_FMT_SIZE] = {0};
	struct manager_entry *entry;
	uint32_t i;

	printf("manager_table_size=%u\n", g_ppp_hw_res.mng_tbl_size);
	printf("entry | mac               | mask | ether | mask | vlan | mask "
	       "| i_map | i_dir | e_type | pf_id | vf_id | q_id | drop\n");
	for (i = 0; i < tbl->entry_size; i++) {
		entry = &tbl->entry[i];
		hikp_ether_format_addr(mac_str, HIKP_NIC_ETH_ADDR_FMT_SIZE, entry->mac_addr,
				       HIKP_NIC_ETH_MAC_ADDR_LEN);
		printf(" %02u   | %s | %u    ", entry->entry_no, mac_str, entry->mac_mask);
		printf("| %04x  | %u    | %04u | %u    ",
		       entry->ether_type, entry->ether_mask, entry->vlan_id, entry->vlan_mask);
		printf("| %02x    | %02x    | %01u      | %02u    | %03u   | %04u | %u",
		       entry->i_port_bitmap, entry->i_port_dir, entry->e_port_type,
		       entry->pf_id, entry->vf_id, entry->q_id, entry->drop);
	}
	printf("\n");
}

static void hikp_nic_ppp_show_promisc_tbl(const void *data)
{
	struct nic_promisc_tbl *tbl = (struct nic_promisc_tbl *)data;
	char func_name[HIKP_NIC_FUNC_NAME_LEN];
	struct func_promisc_cfg *func;
	uint16_t i;

	printf("func_id\t uc_en\t mc_en\t bc_en\n");
	for (i = 0; i < tbl->func_num; i++) {
		func = &tbl->func[i];
		hikp_nic_ppp_get_func_name(func_name, HIKP_NIC_FUNC_NAME_LEN, func->func_id);
		printf("%s\t %u\t %u\t %u\n", func_name, func->uc_en, func->mc_en, func->bc_en);
	}
}

static void hikp_nic_ppp_show_vlan_offload(const void *data)
{
	struct nic_vlan_offload_cfg *tbl = (struct nic_vlan_offload_cfg *)data;
	const char * const state_str[] = { "off", "on" };
	char func_name[HIKP_NIC_FUNC_NAME_LEN];
	struct func_vlan_offload_cfg *func;
	uint8_t ingress, egress;
	uint16_t i;

	ingress = !!(tbl->port_vlan_fe & HIKP_FILTER_FE_NIC_INGRESS_B);
	egress = !!(tbl->port_vlan_fe & HIKP_FILTER_FE_NIC_EGRESS_B);
	printf("port VLAN filter configuration:\n");
	printf("ingress_port_vlan_filter: %s\n", state_str[ingress]);
	printf("egress_port_vlan_filter: %s\n", state_str[egress]);

	printf("func VLAN filter configuration:\n");
	printf("func_id\t ingress_vlan_filter\t egress_vlan_filter\t port_vlan_filter_bypass\n");
	for (i = 0; i < tbl->func_num; i++) {
		func = &tbl->func[i];
		ingress = !!(func->vlan_fe & HIKP_FILTER_FE_NIC_INGRESS_B);
		egress = !!(func->vlan_fe & HIKP_FILTER_FE_NIC_EGRESS_B);
		hikp_nic_ppp_get_func_name(func_name, HIKP_NIC_FUNC_NAME_LEN, i);
		printf("%s\t %s\t\t\t %s\t\t\t %s\n", func_name, state_str[ingress],
		       state_str[egress],
		       func->port_vlan_bypass > 1 ? "NA" : state_str[func->port_vlan_bypass]);
	}

	printf("func VLAN offload configuration:\n");
	printf("func_id\tpvid\taccept_tag1\taccept_tag2\taccept_untag1\taccept_untag2\t"
	       "insert_tag1\tinsert_tag2\tshift_tag\tstrip_tag1\tstrip_tag2\tdrop_tag1\t"
	       "drop_tag2\tpri_only_tag1\tpri_only_tag2\n");
	for (i = 0; i < tbl->func_num; i++) {
		func = &tbl->func[i];
		hikp_nic_ppp_get_func_name(func_name, HIKP_NIC_FUNC_NAME_LEN, i);
		printf("%s\t%u\t%s\t\t%s\t\t%s\t\t%s\t\t%s\t\t%s\t\t"
		       "%s\t\t%s\t\t%s\t\t%s\t\t%s\t\t%s\t\t%s\n", func_name,
		       func->pvid, state_str[!!func->accept_tag1], state_str[!!func->accept_tag2],
		       state_str[!!func->accept_untag1], state_str[!!func->accept_untag2],
		       state_str[!!func->insert_tag1], state_str[!!func->insert_tag2],
		       state_str[!!func->shift_tag], state_str[!!func->strip_tag1],
		       state_str[!!func->strip_tag2], state_str[!!func->drop_tag1],
		       state_str[!!func->drop_tag2], state_str[!!func->pri_only_tag1],
		       state_str[!!func->pri_only_tag2]);
	}
}

static int hikp_nic_ppp_get_blk(struct hikp_cmd_header *req_header,
				const struct nic_ppp_req_para *req_data,
				void *buf, size_t buf_len, struct nic_ppp_rsp_head *rsp_head)
{
	struct hikp_cmd_ret *cmd_ret;
	struct nic_ppp_rsp *rsp;
	int ret = 0;

	cmd_ret = hikp_cmd_alloc(req_header, req_data, sizeof(*req_data));
	ret = hikp_rsp_normal_check(cmd_ret);
	if (ret)
		goto out;

	rsp = (struct nic_ppp_rsp *)cmd_ret->rsp_data;
	if (rsp->rsp_head.cur_blk_size > buf_len ||
	    rsp->rsp_head.cur_blk_size > sizeof(rsp->rsp_data)) {
		HIKP_ERROR_PRINT("nic_ppp block context copy size error, "
				 "dst buffer size=%zu, src buffer size=%zu, data size=%u.\n",
				 buf_len, sizeof(rsp->rsp_data), rsp->rsp_head.cur_blk_size);
		ret = -EINVAL;
		goto out;
	}
	memcpy(buf, rsp->rsp_data, rsp->rsp_head.cur_blk_size);

	rsp_head->total_blk_num = rsp->rsp_head.total_blk_num;
	rsp_head->cur_blk_size = rsp->rsp_head.cur_blk_size;
	rsp_head->next_entry_idx = rsp->rsp_head.next_entry_idx;
	rsp_head->cur_blk_entry_cnt = rsp->rsp_head.cur_blk_entry_cnt;

out:
	free(cmd_ret);
	return ret;
}

static int hikp_nic_ppp_query_uc_mac_addr(struct hikp_cmd_header *req_header,
					  struct nic_ppp_req_para *req_data,
					  struct mac_vlan_uc_tbl *uc_tbl,
					  uint16_t max_hw_entry_size)
{
	struct mac_vlan_uc_entry *uc_entry = uc_tbl->entry;
	struct nic_ppp_rsp_head rsp_head = {0};
	uint32_t entry_size = 0;
	size_t left_buf_len;
	uint16_t idx = 0;
	int ret = 0;

	while (idx < max_hw_entry_size) {
		req_data->cur_entry_idx = idx;
		req_data->is_unicast = 1;
		left_buf_len = sizeof(struct mac_vlan_uc_entry) * (max_hw_entry_size - entry_size);
		ret = hikp_nic_ppp_get_blk(req_header, req_data,
					   uc_entry + entry_size, left_buf_len, &rsp_head);
		if (ret != 0) {
			HIKP_ERROR_PRINT("Fail to get the uc entry after idx=%u, ret=%d.\n",
					 idx, ret);
			return ret;
		}
		if (rsp_head.next_entry_idx <= idx) {
			HIKP_ERROR_PRINT("Next entry index (%u) should be greater than current (%u).\n",
					 rsp_head.next_entry_idx, idx);
			return -EINVAL;
		}
		if (entry_size + rsp_head.cur_blk_entry_cnt > max_hw_entry_size) {
			HIKP_ERROR_PRINT("The sum of entry number (%lu) after block-%u "
					 "is over the maximum entry nubmer (%u) of unicast MAC table.\n",
					 entry_size + rsp_head.cur_blk_entry_cnt, idx, max_hw_entry_size);
			return -EINVAL;
		}
		entry_size += rsp_head.cur_blk_entry_cnt;
		idx = rsp_head.next_entry_idx;
	}
	uc_tbl->entry_size = entry_size;

	return ret;
}

static int hikp_nic_ppp_query_mc_mac_addr(struct hikp_cmd_header *req_header,
					  struct nic_ppp_req_para *req_data,
					  struct mac_vlan_mc_tbl *mc_tbl,
					  uint16_t max_hw_entry_size)
{
	struct mac_vlan_mc_entry *mc_entry = mc_tbl->entry;
	struct nic_ppp_rsp_head rsp_head = {0};
	uint32_t entry_size = 0;
	size_t left_buf_len;
	uint16_t idx = 0;
	int ret = 0;

	while (idx < max_hw_entry_size) {
		req_data->cur_entry_idx = idx;
		req_data->is_unicast = 0;
		left_buf_len = sizeof(struct mac_vlan_mc_entry) * (max_hw_entry_size - entry_size);
		ret = hikp_nic_ppp_get_blk(req_header, req_data,
					   mc_entry + entry_size, left_buf_len, &rsp_head);
		if (ret != 0) {
			HIKP_ERROR_PRINT("Fail to get the mc entry after idx=%u, ret=%d.\n",
					 idx, ret);
			return ret;
		}
		if (rsp_head.next_entry_idx <= idx) {
			HIKP_ERROR_PRINT("Next entry index (%u) should be greater than current (%u).\n",
					 rsp_head.next_entry_idx, idx);
			return -EINVAL;
		}
		if (entry_size + rsp_head.cur_blk_entry_cnt > max_hw_entry_size) {
			HIKP_ERROR_PRINT("The sum of entry number (%lu) after block-%u "
					 "is over the maximum entry nubmer (%u) of multicast MAC table.\n",
					 entry_size + rsp_head.cur_blk_entry_cnt, idx, max_hw_entry_size);
			return -EINVAL;
		}
		entry_size += rsp_head.cur_blk_entry_cnt;
		idx = rsp_head.next_entry_idx;
	}
	mc_tbl->entry_size = entry_size;

	return ret;
}

static int hikp_nic_query_mac_tbl(struct hikp_cmd_header *req_header,
				  struct nic_ppp_req_para *req_data,
				  struct nic_mac_tbl *mac_tbl)
{
	struct mac_vlan_uc_tbl *uc_tbl = &mac_tbl->uc_tbl;
	struct mac_vlan_mc_tbl *mc_tbl = &mac_tbl->mc_tbl;
	uint32_t max_hw_entry_size;
	int ret = 0;
	bool query;

	max_hw_entry_size = g_ppp_hw_res.max_key_mem_size + g_ppp_hw_res.overflow_cam_size;
	query = g_ppp_param.is_uc != -1 ? (g_ppp_param.is_uc == 1) : true;
	if (query) {
		ret = hikp_nic_ppp_query_uc_mac_addr(req_header, req_data,
						     uc_tbl, max_hw_entry_size);
		if (ret != 0)
			return ret;
	}

	query = g_ppp_param.is_uc != -1 ? (g_ppp_param.is_uc == 0) : true;
	if (query) {
		ret = hikp_nic_ppp_query_mc_mac_addr(req_header, req_data,
						     mc_tbl, max_hw_entry_size);
		if (ret != 0)
			return ret;
	}

	return ret;
}

static int hikp_nic_ppp_query_vf_vlan_tbl(struct hikp_cmd_header *req_header,
					  struct nic_ppp_req_para *req_data,
					  struct vf_vlan_tbl *vlan_tbl, uint16_t hw_entry_size)
{
	struct vf_vlan_tbl_entry *vf_entry = vlan_tbl->entry;
	struct nic_ppp_rsp_head rsp_head = {0};
	uint32_t entry_size = 0;
	size_t left_buf_len;
	uint32_t idx = 0;
	int ret = 0;

	while (idx < hw_entry_size) {
		req_data->cur_entry_idx = idx;
		req_data->is_port_vlan = 0;
		left_buf_len = sizeof(struct vf_vlan_tbl_entry) * (hw_entry_size - entry_size);
		ret = hikp_nic_ppp_get_blk(req_header, req_data,
					   vf_entry + entry_size, left_buf_len, &rsp_head);
		if (ret != 0) {
			HIKP_ERROR_PRINT("Fail to get the vf vlan entry after idx=%u, ret=%d.\n",
					 idx, ret);
			return ret;
		}
		if (rsp_head.next_entry_idx <= idx) {
			HIKP_ERROR_PRINT("Next entry index (%u) should be greater than current (%u).\n",
					 rsp_head.next_entry_idx, idx);
			return -EINVAL;
		}
		if (entry_size + rsp_head.cur_blk_entry_cnt > hw_entry_size) {
			HIKP_ERROR_PRINT("The sum of entry number (%lu) after block-%u "
					 "is over the maximum entry nubmer (%u) of VF VLAN table.\n",
					 entry_size + rsp_head.cur_blk_entry_cnt, idx, hw_entry_size);
			return -EINVAL;
		}
		entry_size += rsp_head.cur_blk_entry_cnt;
		idx = rsp_head.next_entry_idx;
	}
	vlan_tbl->entry_size = entry_size;

	return ret;
}

static int hikp_nic_ppp_query_port_vlan_tbl(struct hikp_cmd_header *req_header,
					    struct nic_ppp_req_para *req_data,
					    struct port_vlan_tbl *vlan_tbl, uint16_t hw_entry_size)
{
	struct port_vlan_tbl_entry *port_entry = vlan_tbl->entry;
	struct nic_ppp_rsp_head rsp_head = {0};
	uint32_t entry_size = 0;
	size_t left_buf_len;
	uint32_t idx = 0;
	int ret = 0;

	while (idx < hw_entry_size) {
		req_data->cur_entry_idx = idx;
		req_data->is_port_vlan = 1;
		left_buf_len = sizeof(struct port_vlan_tbl_entry) * (hw_entry_size - entry_size);
		ret = hikp_nic_ppp_get_blk(req_header, req_data,
					   port_entry + entry_size, left_buf_len, &rsp_head);
		if (ret != 0) {
			HIKP_ERROR_PRINT("Fail to get the port vlan entry after idx=%u, ret=%d.\n",
					 idx, ret);
			return ret;
		}
		if (rsp_head.next_entry_idx <= idx) {
			HIKP_ERROR_PRINT("Next entry index (%u) should be greater than current (%u).\n",
					 rsp_head.next_entry_idx, idx);
			return -EINVAL;
		}
		if (entry_size + rsp_head.cur_blk_entry_cnt > hw_entry_size) {
			HIKP_ERROR_PRINT("The sum of entry number (%lu) after block-%u "
					 "is over the maximum entry nubmer (%u) of port VLAN table.\n",
					 entry_size + rsp_head.cur_blk_entry_cnt, idx, hw_entry_size);
			return -EINVAL;
		}
		entry_size += rsp_head.cur_blk_entry_cnt;
		idx = rsp_head.next_entry_idx;
	}
	vlan_tbl->entry_size = entry_size;

	return ret;
}

static int hikp_nic_query_vlan_tbl(struct hikp_cmd_header *req_header,
				   struct nic_ppp_req_para *req_data,
				   struct nic_vlan_tbl *vlan_tbl)
{
	struct port_vlan_tbl *port_vlan_tbl = &vlan_tbl->port_vlan_tbl;
	struct vf_vlan_tbl *vf_vlan_tbl = &vlan_tbl->vf_vlan_tbl;
	int ret;

	ret = hikp_nic_ppp_query_vf_vlan_tbl(req_header, req_data, vf_vlan_tbl,
					     g_ppp_hw_res.vf_vlan_tbl_size);
	if (ret != 0)
		return ret;

	ret = hikp_nic_ppp_query_port_vlan_tbl(req_header, req_data, port_vlan_tbl,
					       g_ppp_hw_res.port_vlan_tbl_size);
	if (ret != 0)
		return ret;

	return ret;
}

static int hikp_nic_query_mng_tbl(struct hikp_cmd_header *req_header,
				  struct nic_ppp_req_para *req_data,
				  struct nic_mng_tbl *mng_tbl)
{
	struct manager_entry *entry = mng_tbl->entry;
	struct nic_ppp_rsp_head rsp_head = {0};
	uint32_t entry_size = 0;
	size_t left_buf_len;
	uint32_t idx = 0;
	int ret = 0;

	while (idx < g_ppp_hw_res.mng_tbl_size) {
		req_data->cur_entry_idx = idx;
		left_buf_len = sizeof(struct manager_entry) *
			       (g_ppp_hw_res.mng_tbl_size - entry_size);
		ret = hikp_nic_ppp_get_blk(req_header, req_data,
					   entry + entry_size, left_buf_len, &rsp_head);
		if (ret != 0) {
			HIKP_ERROR_PRINT("Fail to get the mng entry after idx=%u, ret=%d.\n",
					 idx, ret);
			return ret;
		}
		if (rsp_head.next_entry_idx <= idx) {
			HIKP_ERROR_PRINT("Next entry index (%u) should be greater than current (%u).\n",
					 rsp_head.next_entry_idx, idx);
			return -EINVAL;
		}
		if (entry_size + rsp_head.cur_blk_entry_cnt > g_ppp_hw_res.mng_tbl_size) {
			HIKP_ERROR_PRINT("The sum of entry number (%lu) after block-%u "
					 "is over the maximum entry nubmer (%u) of manager table.\n",
					 entry_size + rsp_head.cur_blk_entry_cnt, idx, g_ppp_hw_res.mng_tbl_size);
			return -EINVAL;
		}
		entry_size += rsp_head.cur_blk_entry_cnt;
		idx = rsp_head.next_entry_idx;
	}
	mng_tbl->entry_size = entry_size;

	return ret;
}

static int hikp_nic_query_ppp_by_entryid(struct hikp_cmd_header *req_header,
					 const struct bdf_t *bdf,
					 void *data, size_t len)
{
	const struct ppp_feature_cmd *ppp_cmd;
	struct nic_ppp_req_para req_data = {0};

	req_data.bdf = *bdf;
	ppp_cmd = &g_ppp_feature_cmd[g_ppp_param.feature_idx];
	if (strcmp(ppp_cmd->feature_name, NIC_PPP_MAC_TBL_NAME) == 0)
		return hikp_nic_query_mac_tbl(req_header, &req_data, (struct nic_mac_tbl *)data);
	else if (strcmp(ppp_cmd->feature_name, NIC_PPP_VLAN_TBL_NAME) == 0)
		return hikp_nic_query_vlan_tbl(req_header, &req_data, (struct nic_vlan_tbl *)data);
	else
		return hikp_nic_query_mng_tbl(req_header, &req_data, (struct nic_mng_tbl *)data);
}

static int hikp_nic_ppp_check_func_num(void *data)
{
	const struct ppp_feature_cmd *ppp_cmd;
	uint16_t func_num = 0;

	ppp_cmd = &g_ppp_feature_cmd[g_ppp_param.feature_idx];
	if (ppp_cmd->sub_cmd_code == NIC_PROMISCUOUS_TBL_DUMP)
		func_num = ((struct nic_promisc_tbl *)data)->func_num;
	else if (ppp_cmd->sub_cmd_code == NIC_VLAN_OFFLOAD_DUMP)
		func_num = ((struct nic_vlan_offload_cfg *)data)->func_num;

	if (func_num > HIKP_NIC_MAX_FUNC_NUM) {
		HIKP_ERROR_PRINT("Illegal function num(%u) from firmware.\n", func_num);
		return -EINVAL;
	}
	return 0;
}

static int hikp_nic_query_ppp_by_blkid(struct hikp_cmd_header *req_header, const struct bdf_t *bdf,
				       void *data, size_t len)
{
	struct nic_ppp_rsp_head rsp_head = {0};
	struct nic_ppp_req_para req_data = {0};
	uint32_t total_blk_size;
	uint8_t total_blk_num;
	uint8_t blk_id = 0;
	int ret = 0;

	req_data.bdf = *bdf;
	req_data.block_id = blk_id;
	ret = hikp_nic_ppp_get_blk(req_header, &req_data, data, len, &rsp_head);
	if (ret != 0) {
		HIKP_ERROR_PRINT("Fail to get block-%u context.\n", blk_id);
		return ret;
	}
	total_blk_num = rsp_head.total_blk_num;
	total_blk_size = rsp_head.cur_blk_size;

	/* Copy the remaining block content if total block number is greater than 1. */
	for (blk_id = 1; blk_id < total_blk_num; blk_id++) {
		req_data.block_id = blk_id;
		if (len <= total_blk_size) {
			HIKP_ERROR_PRINT("No enough buffer to get block-%u context.\n", blk_id);
			return -ENOMEM;
		}
		ret = hikp_nic_ppp_get_blk(req_header, &req_data, (uint8_t *)data + total_blk_size,
					   len - total_blk_size, &rsp_head);
		if (ret != 0) {
			HIKP_ERROR_PRINT("Fail to get block-%u context.\n", blk_id);
			return ret;
		}
		total_blk_size += rsp_head.cur_blk_size;
	}

	return hikp_nic_ppp_check_func_num(data);
}

static int hikp_nic_ppp_get_hw_resources(const struct bdf_t *bdf,
					 struct hikp_nic_ppp_hw_resources *hw_res)
{
	struct hikp_cmd_header req_header = {0};

	if (!g_ppp_feature_cmd[g_ppp_param.feature_idx].need_query_hw_res)
		return 0;

	hikp_cmd_init(&req_header, NIC_MOD, GET_PPP_INFO_CMD, NIC_PPP_HW_RES_DUMP);
	return hikp_nic_query_ppp_by_blkid(&req_header, bdf, hw_res, sizeof(*hw_res));
}

static int hikp_nic_ppp_alloc_mac_tbl_entry(struct nic_mac_tbl *mac_tbl,
					    const struct hikp_nic_ppp_hw_resources *hw_res)
{
	uint32_t max_entry_size;

	max_entry_size = hw_res->max_key_mem_size + hw_res->overflow_cam_size;
	mac_tbl->uc_tbl.entry = (struct mac_vlan_uc_entry *)calloc(max_entry_size,
				sizeof(struct mac_vlan_uc_entry));
	if (mac_tbl->uc_tbl.entry == NULL) {
		HIKP_ERROR_PRINT("Fail to alloc uc_entry memory.\n");
		return -ENOMEM;
	}

	mac_tbl->mc_tbl.entry = (struct mac_vlan_mc_entry *)calloc(max_entry_size,
				sizeof(struct mac_vlan_mc_entry));
	if (mac_tbl->mc_tbl.entry == NULL) {
		HIKP_ERROR_PRINT("Fail to alloc mc_entry memory.\n");
		free(mac_tbl->uc_tbl.entry);
		mac_tbl->uc_tbl.entry = NULL;
		return -ENOMEM;
	}

	return 0;
}

static int hikp_nic_ppp_alloc_vlan_tbl_entry(struct nic_vlan_tbl *vlan_tbl,
					     const struct hikp_nic_ppp_hw_resources *hw_res)
{
	vlan_tbl->port_vlan_tbl.entry =
		(struct port_vlan_tbl_entry *)calloc(hw_res->port_vlan_tbl_size,
		sizeof(struct port_vlan_tbl_entry));
	if (vlan_tbl->port_vlan_tbl.entry == NULL) {
		HIKP_ERROR_PRINT("Fail to alloc port_vlan_tbl_entry memory.\n");
		return -ENOMEM;
	}

	vlan_tbl->vf_vlan_tbl.entry = (struct vf_vlan_tbl_entry *)calloc(hw_res->vf_vlan_tbl_size,
				      sizeof(struct vf_vlan_tbl_entry));
	if (vlan_tbl->vf_vlan_tbl.entry == NULL) {
		HIKP_ERROR_PRINT("Fail to alloc vf_vlan_tbl_entry memory.\n");
		free(vlan_tbl->port_vlan_tbl.entry);
		vlan_tbl->port_vlan_tbl.entry = NULL;
		return -ENOMEM;
	}

	return 0;
}

static int hikp_nic_ppp_alloc_mng_tbl_entry(struct nic_mng_tbl *mng_tbl,
					    const struct hikp_nic_ppp_hw_resources *hw_res)
{
	mng_tbl->entry = (struct manager_entry *)calloc(hw_res->mng_tbl_size,
			 sizeof(struct manager_entry));
	if (mng_tbl->entry == NULL) {
		HIKP_ERROR_PRINT("fail to alloc manager_entry memory.\n");
		return -ENOMEM;
	}

	return 0;
}

static union nic_ppp_feature_info *hikp_nic_ppp_data_alloc(const struct ppp_feature_cmd *ppp_cmd,
	const struct hikp_nic_ppp_hw_resources *hw_res)
{
	union nic_ppp_feature_info *ppp_data;
	int ret = 0;

	ppp_data = (union nic_ppp_feature_info *)calloc(1, sizeof(union nic_ppp_feature_info));
	if (ppp_data == NULL) {
		HIKP_ERROR_PRINT("Fail to allocate nic_ppp_feature_info memory.\n");
		return NULL;
	}

	if (strcmp(ppp_cmd->feature_name, NIC_PPP_MAC_TBL_NAME) == 0)
		ret = hikp_nic_ppp_alloc_mac_tbl_entry(&ppp_data->mac_tbl, hw_res);
	else if (strcmp(ppp_cmd->feature_name, NIC_PPP_VLAN_TBL_NAME) == 0)
		ret = hikp_nic_ppp_alloc_vlan_tbl_entry(&ppp_data->vlan_tbl, hw_res);
	else if (strcmp(ppp_cmd->feature_name, NIC_PPP_MNG_TBL_NAME) == 0)
		ret = hikp_nic_ppp_alloc_mng_tbl_entry(&ppp_data->mng_tbl, hw_res);
	if (ret != 0)
		goto out;

	return ppp_data;

out:
	free(ppp_data);

	return NULL;
}

static void hikp_nic_ppp_data_free(union nic_ppp_feature_info *ppp_data)
{
	const struct ppp_feature_cmd *ppp_cmd;
	struct nic_vlan_tbl *vlan_tbl;
	struct nic_mac_tbl *mac_tbl;
	struct nic_mng_tbl *mng_tbl;

	ppp_cmd = &g_ppp_feature_cmd[g_ppp_param.feature_idx];
	if (strcmp(ppp_cmd->feature_name, NIC_PPP_MAC_TBL_NAME) == 0) {
		mac_tbl = &ppp_data->mac_tbl;
		free(mac_tbl->uc_tbl.entry);
		mac_tbl->uc_tbl.entry = NULL;
		free(mac_tbl->mc_tbl.entry);
		mac_tbl->mc_tbl.entry = NULL;
	} else if (strcmp(ppp_cmd->feature_name, NIC_PPP_VLAN_TBL_NAME) == 0) {
		vlan_tbl = &ppp_data->vlan_tbl;
		free(vlan_tbl->vf_vlan_tbl.entry);
		vlan_tbl->vf_vlan_tbl.entry = NULL;
		free(vlan_tbl->port_vlan_tbl.entry);
		vlan_tbl->port_vlan_tbl.entry = NULL;
	} else if (strcmp(ppp_cmd->feature_name, NIC_PPP_MNG_TBL_NAME) == 0) {
		mng_tbl = &ppp_data->mng_tbl;
		free(mng_tbl->entry);
		mng_tbl->entry = NULL;
	}
	free(ppp_data);
	ppp_data = NULL;
}

static int hikp_nic_ppp_check_optional_param(struct major_cmd_ctrl *self,
					     const struct nic_ppp_param *ppp_param,
					     const struct ppp_feature_cmd *ppp_cmd)
{
	switch (ppp_cmd->sub_cmd_code) {
	case NIC_MAC_TBL_DUMP:
		if ((ppp_param->func_id != -1 && ppp_param->is_uc == -1) ||
			(ppp_param->func_id == -1 && ppp_param->is_uc != -1)) {
			snprintf(self->err_str, sizeof(self->err_str),
				 "please input func_id and unicast value at the same time.");
			self->err_no = -EINVAL;
			return self->err_no;
		}
		break;
	case NIC_VLAN_TBL_DUMP:
		if (ppp_param->is_uc != -1) {
			snprintf(self->err_str, sizeof(self->err_str),
				 "%s cmd no need '-uc/--unicast' parameter",
				 ppp_cmd->feature_name);
			self->err_no = -EINVAL;
			return self->err_no;
		}
		break;
	case NIC_MNG_TBL_DUMP:
	case NIC_PROMISCUOUS_TBL_DUMP:
	case NIC_VLAN_OFFLOAD_DUMP:
		if (ppp_param->func_id != -1 || ppp_param->is_uc != -1) {
			snprintf(self->err_str, sizeof(self->err_str),
				 "%s cmd no need '-func/--func_id' and '-uc/--unicast' parameter",
				 ppp_cmd->feature_name);
			self->err_no = -EINVAL;
			return self->err_no;
		}
		break;
	default:
		break;
	}

	return 0;
}

static int hikp_nic_ppp_check_input_param(struct major_cmd_ctrl *self,
					  const struct nic_ppp_param *ppp_param)
{
	const struct bdf_t *bdf = &ppp_param->target.bdf;
	const struct ppp_feature_cmd *ppp_cmd;

	if (bdf->dev_id != 0) {
		snprintf(self->err_str, sizeof(self->err_str), "VF does not support query!");
		self->err_no = -EINVAL;
		return self->err_no;
	}

	if (ppp_param->feature_idx == -1) {
		hikp_nic_ppp_cmd_help(self, NULL);
		snprintf(self->err_str, sizeof(self->err_str), "-du/--dump parameter error!");
		self->err_no = -EINVAL;
		return self->err_no;
	}

	ppp_cmd = &g_ppp_feature_cmd[g_ppp_param.feature_idx];
	return hikp_nic_ppp_check_optional_param(self, ppp_param, ppp_cmd);
}

static int hikp_nic_check_func_id_valid(struct major_cmd_ctrl *self,
					const struct ppp_feature_cmd *ppp_cmd,
					const struct nic_ppp_param *ppp_param,
					const struct hikp_nic_ppp_hw_resources *hw_res)
{
	if (ppp_param->func_id >= hw_res->total_func_num) {
		snprintf(self->err_str, sizeof(self->err_str),
			 "-func/--func_id parameter is invalid, func_id should be less than %u.",
			 hw_res->total_func_num);
		self->err_no = -EINVAL;
		return self->err_no;
	}

	return 0;
}

static int hikp_nic_check_hw_res(struct hikp_nic_ppp_hw_resources *hw_res)
{
	if (!g_ppp_feature_cmd[g_ppp_param.feature_idx].need_query_hw_res)
		return 0;

	if (hw_res->max_key_mem_size == 0) {
		HIKP_ERROR_PRINT("Max key memory size should not be zero!\n");
		return -EINVAL;
	}
	if (hw_res->port_vlan_tbl_size == 0) {
		HIKP_ERROR_PRINT("PORT VLAN Table size should not be zero!\n");
		return -EINVAL;
	}
	if (hw_res->vf_vlan_tbl_size == 0) {
		HIKP_ERROR_PRINT("VF VLAN Table size should not be zero!\n");
		return -EINVAL;
	}
	if (hw_res->mng_tbl_size == 0) {
		HIKP_ERROR_PRINT("Manager Table size should not be zero!\n");
		return -EINVAL;
	}
	if (hw_res->mac_id >= HIKP_PPP_MAX_MAC_ID_NUM) {
		HIKP_ERROR_PRINT("MAC ID (%u) should be less than %u.\n",
				 hw_res->mac_id, HIKP_PPP_MAX_MAC_ID_NUM);
		return -EINVAL;
	}
	if (hw_res->total_func_num == 0 || hw_res->total_func_num > HIKP_NIC_MAX_FUNC_NUM) {
		HIKP_ERROR_PRINT("Total_func_num (%u）should be in [1, %u].\n",
				 hw_res->total_func_num, HIKP_NIC_MAX_FUNC_NUM);
		return -EINVAL;
	}
	if (hw_res->abs_func_id_base >= HIKP_NIC_MAX_FUNC_NUM) {
		HIKP_ERROR_PRINT("Function ID base (%u) should be less than %u.\n",
				 hw_res->abs_func_id_base, HIKP_NIC_MAX_FUNC_NUM);
		return -EINVAL;
	}

	return 0;
}

static void hikp_nic_ppp_cmd_execute(struct major_cmd_ctrl *self)
{
	struct bdf_t *bdf = &g_ppp_param.target.bdf;
	const struct ppp_feature_cmd *ppp_cmd;
	union nic_ppp_feature_info *ppp_data;
	struct hikp_cmd_header req_header = {0};
	int ret;

	ret = hikp_nic_ppp_check_input_param(self, &g_ppp_param);
	if (ret != 0)
		return;

	ret = hikp_nic_ppp_get_hw_resources(bdf, &g_ppp_hw_res);
	if (ret != 0) {
		snprintf(self->err_str, sizeof(self->err_str),
			 "fail to obtain ppp hardware resources.");
		self->err_no = ret;
		return;
	}

	ret = hikp_nic_check_hw_res(&g_ppp_hw_res);
	if (ret != 0) {
		snprintf(self->err_str, sizeof(self->err_str),
			 "ppp hardware resources obtained is invalid.");
		self->err_no = ret;
		return;
	}

	ppp_cmd = &g_ppp_feature_cmd[g_ppp_param.feature_idx];
	ret = hikp_nic_check_func_id_valid(self, ppp_cmd, &g_ppp_param, &g_ppp_hw_res);
	if (ret != 0)
		return;

	ppp_data = hikp_nic_ppp_data_alloc(ppp_cmd, &g_ppp_hw_res);
	if (ppp_data == NULL) {
		snprintf(self->err_str, sizeof(self->err_str),
			 "failed to allocate ppp_data memory!");
		self->err_no = -ENOMEM;
		return;
	}

	hikp_cmd_init(&req_header, NIC_MOD, GET_PPP_INFO_CMD, ppp_cmd->sub_cmd_code);
	ret = ppp_cmd->query(&req_header, &g_ppp_param.target.bdf, ppp_data,
			     sizeof(union nic_ppp_feature_info));
	if (ret != 0) {
		snprintf(self->err_str, sizeof(self->err_str),
			 "failed to query %s info, ret = %d.", ppp_cmd->feature_name, ret);
		self->err_no = ret;
		goto out;
	}

	printf("############## NIC PPP: %s info ############\n", ppp_cmd->feature_name);
	ppp_cmd->show(ppp_data);
	printf("#################### END #######################\n");

out:
	hikp_nic_ppp_data_free(ppp_data);
}

static int hikp_nic_cmd_get_ppp_target(struct major_cmd_ctrl *self, const char *argv)
{
	self->err_no = tool_check_and_get_valid_bdf_id(argv, &(g_ppp_param.target));
	if (self->err_no != 0) {
		snprintf(self->err_str, sizeof(self->err_str), "unknown device!");
		return self->err_no;
	}

	return 0;
}

static int hikp_nic_cmd_ppp_feature_select(struct major_cmd_ctrl *self, const char *argv)
{
	size_t feat_size = HIKP_ARRAY_SIZE(g_ppp_feature_cmd);
	size_t i;

	for (i = 0; i < feat_size; i++) {
		if (strcmp(argv, g_ppp_feature_cmd[i].feature_name) == 0) {
			g_ppp_param.feature_idx = i;
			return 0;
		}
	}

	hikp_nic_ppp_cmd_help(self, NULL);
	snprintf(self->err_str, sizeof(self->err_str), "please input valid subfunction.");
	self->err_no = -EINVAL;

	return self->err_no;
}

static int hikp_nic_cmd_ppp_parse_func_id(struct major_cmd_ctrl *self, const char *argv)
{
	uint32_t func_id;

	self->err_no = string_toui(argv, &func_id);
	if (self->err_no) {
		snprintf(self->err_str, sizeof(self->err_str),
			 "parse -func/--function parameter failed.");
		return self->err_no;
	}

	if (func_id >= HIKP_NIC_MAX_FUNC_NUM) {
		snprintf(self->err_str, sizeof(self->err_str),
			 "maximum func_id should be less than %u.", HIKP_NIC_MAX_FUNC_NUM);
		self->err_no = -EINVAL;
		return self->err_no;
	}

	g_ppp_param.func_id = (uint16_t)func_id;

	return 0;
}

static int hikp_nic_cmd_ppp_parse_unicast(struct major_cmd_ctrl *self, const char *argv)
{
	uint32_t val;

	self->err_no = string_toui(argv, &val);
	if (self->err_no) {
		snprintf(self->err_str, sizeof(self->err_str),
			 "parse -un/--unicast parameter failed.");
		return self->err_no;
	}

	if (val != 0 && val != 1) {
		snprintf(self->err_str, sizeof(self->err_str),
			 "please input 0 or 1 for -un/--unicast parameter.");
		self->err_no = -EINVAL;
		return self->err_no;
	}

	g_ppp_param.is_uc = val;

	return 0;
}

static void cmd_nic_get_ppp_init(void)
{
	struct major_cmd_ctrl *major_cmd = get_major_cmd();

	g_ppp_param.func_id = -1;
	g_ppp_param.is_uc = -1;
	g_ppp_param.feature_idx = -1;
	major_cmd->option_count = 0;
	major_cmd->execute = hikp_nic_ppp_cmd_execute;

	cmd_option_register("-h", "--help", false, hikp_nic_ppp_cmd_help);
	cmd_option_register("-i", "--interface", true, hikp_nic_cmd_get_ppp_target);
	cmd_option_register("-du", "--dump", true, hikp_nic_cmd_ppp_feature_select);
	cmd_option_register("-func", "--func_id", true, hikp_nic_cmd_ppp_parse_func_id);
	cmd_option_register("-uc", "--unicast", true, hikp_nic_cmd_ppp_parse_unicast);
}

HIKP_CMD_DECLARE("nic_ppp", "dump ppp info of nic!", cmd_nic_get_ppp_init);
