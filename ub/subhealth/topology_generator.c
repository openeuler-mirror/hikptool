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

#include "sub_health.h"
#include "cJSON.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>

/* ========================================================================
 * 常量
 * ======================================================================== */

#define MAX_EID_COUNT        1024   /* static-urma-eid 条目上限 */
#define MAX_EID_INFO_NUM     1024   /* 每条目的 urma-eid-info 上限 */
#define MAX_LINE_LEN         4096   /* XML 单行最大长度 */
#define XML_BUF_INIT_SIZE    (1 * 1024 * 1024)   /* XML 缓冲区初始大小（1MB） */
#define MAX_XML_BUF_SIZE     (16 * 1024 * 1024)  /* XML 缓冲区上限（16MB） */

/* RESTCONF 查询命令（通过 Unix socket 调用） */
#define CURL_CMD \
	"curl -X GET " \
	"--unix-socket /run/ubm/socket/ubm_nuds/restconf.sock " \
	"\"http://localhost/restconf/data/huawei-vbussw-service:vbussw-service/" \
	"static-urma-eids\" " \
	"-H \"Accept: application/yang-data+xml\" " \
	"-H \"Content-Type: application/yang-data+xml\" 2>/dev/null"

/* ========================================================================
 * 数据结构
 * ======================================================================== */

/* 单个设备的 EID 条目（对应 XML 中 <static-urma-eid>） */
struct static_urma_eid {
	int slot_id;
	int ubpu_id;              /* RESTCONF 返回 1-based */
	int iou_id;
	int entity_id;
	char label[MAX_EID_LEN];
	int eid_info_num;
	struct {
		char eid[MAX_EID_LEN];
		int port_group_id;
	} eid_infos[MAX_EID_INFO_NUM];
};

/* 按 IP 分组后的条目 */
struct ip_entry {
	char ip[64];
	int ubpu_num;
	int ubpu_ids[MAX_UBPU_NUM];          /* 0-based UBPU key */
	char eids[MAX_UBPU_NUM][MAX_EID_LEN];
};

static struct static_urma_eid g_eids[MAX_EID_COUNT];
static int g_eid_count = 0;

static struct ip_entry g_ip_entries[MAX_NODE_NUM];
static int g_ip_entry_count = 0;

/* ========================================================================
 * Step ① - 通过 curl 获取 XML
 * ======================================================================== */

static int fetch_xml_from_curl(char **out_xml)
{
	FILE *fp;
	char *buf;
	size_t cap = XML_BUF_INIT_SIZE;
	size_t len = 0;
	int exit_code;
	int nread;

	fp = popen(CURL_CMD, "r");
	if (!fp)
		return -EIO;

	buf = (char *)malloc(cap);
	if (!buf) {
		pclose(fp);
		return -ENOMEM;
	}

	while (1) {
		if (len + 4096 >= cap) {
			char *new_buf;

			if (cap >= MAX_XML_BUF_SIZE) {
				free(buf);
				pclose(fp);
				return -ENOSPC;
			}
			new_buf = (char *)realloc(buf, cap * 2);
			if (!new_buf) {
				free(buf);
				pclose(fp);
				return -ENOMEM;
			}
			buf = new_buf;
			cap *= 2;
		}

		nread = (int)fread(buf + len, 1, 4096, fp);
		if (nread <= 0)
			break;
		len += (size_t)nread;
	}

	exit_code = pclose(fp);
	if (exit_code != 0) {
		free(buf);
		return -EIO;
	}

	if (len == 0) {
		free(buf);
		return -EIO;
	}

	buf[len] = '\0';
	*out_xml = buf;
	return 0;
}

/* ========================================================================
 * XML 标签值提取
 * ======================================================================== */

static int extract_tag_value(const char *line, const char *tag,
			     char *value, size_t value_size)
{
	char open_tag[128];
	char close_tag[128];
	const char *start;
	const char *end;
	size_t len;

	snprintf(open_tag, sizeof(open_tag), "<%s>", tag);
	snprintf(close_tag, sizeof(close_tag), "</%s>", tag);

	start = strstr(line, open_tag);
	if (!start)
		return -ENOENT;
	start += strlen(open_tag);

	end = strstr(start, close_tag);
	if (!end)
		return -EINVAL;

	len = (size_t)(end - start);
	if (len >= value_size)
		return -ENOSPC;

	memcpy(value, start, len);
	value[len] = '\0';
	return 0;
}

static int parse_int_from_tag(const char *line, const char *tag, int *out)
{
	char value[64];
	int ret;

	ret = extract_tag_value(line, tag, value, sizeof(value));
	if (ret != 0)
		return ret;

	*out = atoi(value);
	return 0;
}

/* ========================================================================
 * Step ② - 逐行解析 XML 缓冲区
 * ======================================================================== */

static void reset_parser_state(void)
{
	g_eid_count = 0;
	g_ip_entry_count = 0;
	memset(g_eids, 0, sizeof(g_eids));
	memset(g_ip_entries, 0, sizeof(g_ip_entries));
}

/* 解析单行 XML，状态由三个标志位控制 */
static int parse_xml_line(const char *line,
			  int *in_static_urma_eid,
			  int *in_urma_eid_infos,
			  int *in_urma_eid_info,
			  int *cur_eid_idx)
{
	struct static_urma_eid *eid;

	if (strstr(line, "<static-urma-eid>")) {
		if (g_eid_count >= MAX_EID_COUNT)
			return -EINVAL;

		*in_static_urma_eid = 1;
		*cur_eid_idx = g_eid_count;

		eid = &g_eids[*cur_eid_idx];
		memset(eid, 0, sizeof(*eid));

		eid->slot_id = -1;
		eid->ubpu_id = -1;
		eid->iou_id = -1;
		eid->entity_id = -1;

		g_eid_count++;
		return 0;
	}

	if (strstr(line, "</static-urma-eid>")) {
		if (!*in_static_urma_eid)
			return -EINVAL;

		eid = &g_eids[*cur_eid_idx];
		if (eid->slot_id < 0 ||
			eid->ubpu_id < 0 ||
			eid->iou_id < 0 ||
			eid->entity_id < 0) {
			g_eid_count--;
			memset(eid, 0, sizeof(*eid));
			}

		*in_static_urma_eid = 0;
		*in_urma_eid_infos = 0;
		*in_urma_eid_info = 0;
		*cur_eid_idx = -1;
		return 0;
	}

	if (!*in_static_urma_eid)
		return 0;

	/* 原来的 urma-eid-infos 状态处理保持 */

	eid = &g_eids[*cur_eid_idx];

	if (strstr(line, "<slot-id>") &&
		parse_int_from_tag(line, "slot-id", &eid->slot_id) != 0)
		return -EINVAL;

	if (strstr(line, "<ubpu-id>") &&
		parse_int_from_tag(line, "ubpu-id", &eid->ubpu_id) != 0)
		return -EINVAL;

	if (strstr(line, "<iou-id>") &&
		parse_int_from_tag(line, "iou-id", &eid->iou_id) != 0)
		return -EINVAL;

	if (strstr(line, "<entity-id>") &&
		parse_int_from_tag(line, "entity-id", &eid->entity_id) != 0)
		return -EINVAL;

	if (strstr(line, "<label>") &&
		extract_tag_value(line, "label",
				  eid->label, sizeof(eid->label)) != 0)
		return -EINVAL;

	return 0;
}

static int parse_xml_from_buffer(const char *buf)
{
	char line_buf[MAX_LINE_LEN];
	const char *p;
	const char *line_start;
	int in_static_urma_eid = 0;
	int in_urma_eid_infos = 0;
	int in_urma_eid_info = 0;
	int cur_eid_idx = -1;
	size_t len;
	int ret;

	if (buf == NULL)
		return -EINVAL;

	reset_parser_state();

	p = buf;
	line_start = p;

	while (*p != '\0') {
		if (*p == '\n' || *p == '\r') {
			len = (size_t)(p - line_start);

			if (len >= MAX_LINE_LEN)
				return -EINVAL;

			if (len > 0) {
				memcpy(line_buf, line_start, len);
				line_buf[len] = '\0';

				ret = parse_xml_line(line_buf,
							 &in_static_urma_eid,
							 &in_urma_eid_infos,
							 &in_urma_eid_info,
							 &cur_eid_idx);
				if (ret != 0)
					return ret;
			}

			/* Treat "\r\n" as one line separator. */
			if (*p == '\r' && *(p + 1) == '\n')
				p++;

			line_start = p + 1;
		}

		p++;
	}

	/*
	 * Process the last line if the XML buffer does not end with
	 * '\n' or '\r'.
	 */
	if (line_start < p) {
		len = (size_t)(p - line_start);

		if (len >= MAX_LINE_LEN)
			return -EINVAL;

		memcpy(line_buf, line_start, len);
		line_buf[len] = '\0';

		ret = parse_xml_line(line_buf,
					 &in_static_urma_eid,
					 &in_urma_eid_infos,
					 &in_urma_eid_info,
					 &cur_eid_idx);
		if (ret != 0)
			return ret;
	}

	/* Unclosed tag is considered a parsing error. */
	if (in_static_urma_eid)
		return -EINVAL;

	return (g_eid_count > 0) ? 0 : -EINVAL;
}

/* ========================================================================
 * Step ③ - 按 IP 分组
 * ======================================================================== */

static void build_ip(int slot_id, char *ip_buf, size_t size)
{
	snprintf(ip_buf, size, "%d.%d.%d.%d", slot_id, slot_id, slot_id, slot_id);
}

static void group_by_ip(void)
{
	int i, j;

	g_ip_entry_count = 0;

	for (i = 0; i < g_eid_count; i++) {
		struct static_urma_eid *eid = &g_eids[i];
		struct ip_entry *entry = NULL;
		char ip[64];
		int ubpu_key;
		int k;

		/* 只选取 port_group_id == 1 的主端口 EID */
		for (k = 0; k < eid->eid_info_num; k++) {
			if (eid->eid_infos[k].port_group_id == 1)
				break;
		}
		if (k >= eid->eid_info_num)
			continue;

		/* 校验 slot_id(0-255)、ubpu_id(>0)、eid(非空) */
		if (eid->slot_id < 0 || eid->slot_id > 255)
			continue;
		if (eid->ubpu_id <= 0)
			continue;
		if (strlen(eid->eid_infos[k].eid) == 0)
			continue;

		build_ip(eid->slot_id, ip, sizeof(ip));

		/* 按 IP 去重分组 */
		for (j = 0; j < g_ip_entry_count; j++) {
			if (strcmp(g_ip_entries[j].ip, ip) == 0) {
				entry = &g_ip_entries[j];
				break;
			}
		}
		if (!entry && g_ip_entry_count < MAX_NODE_NUM) {
			entry = &g_ip_entries[g_ip_entry_count];
			snprintf(entry->ip, sizeof(entry->ip), "%s", ip);
			g_ip_entry_count++;
		}
		if (!entry)
			continue;

		/* ubpu_id 从 1-based 转为 0-based，作为 UBPU key */
		ubpu_key = eid->ubpu_id - 1;
		if (ubpu_key >= MAX_UBPU_NUM)
			continue;

		/* 同一 IP 下 UBPU key 去重，后到者覆盖 */
		for (j = 0; j < entry->ubpu_num; j++) {
			if (entry->ubpu_ids[j] == ubpu_key) {
				snprintf(entry->eids[j], MAX_EID_LEN, "%s",
					 eid->eid_infos[k].eid);
				break;
			}
		}
		if (j >= entry->ubpu_num && entry->ubpu_num < MAX_UBPU_NUM) {
			entry->ubpu_ids[entry->ubpu_num] = ubpu_key;
			snprintf(entry->eids[entry->ubpu_num], MAX_EID_LEN, "%s",
				 eid->eid_infos[k].eid);
			entry->ubpu_num++;
		}
	}
}

/* ========================================================================
 * Step ④ - 生成拓扑 JSON
 * 格式：{ "l2_switches": [], "l1_switches": { "1D-FULLMESSH": {...} } }
 * ======================================================================== */

static cJSON *build_topology_json(void)
{
	cJSON *root = NULL;
	cJSON *l2_switches = NULL;
	cJSON *l1_switches = NULL;
	cJSON *l1_obj = NULL;
	int i;

	root = cJSON_CreateObject();
	if (root == NULL)
		return NULL;

	l2_switches = cJSON_CreateArray();
	if (l2_switches == NULL)
		goto err;

	if (!cJSON_AddItemToObjectChecked(root, "l2_switches",
					  l2_switches)) {
		cJSON_Delete(l2_switches);
		goto err;
					  }

	/* Ownership of l2_switches has been transferred to root. */
	l2_switches = NULL;

	l1_switches = cJSON_CreateObject();
	if (l1_switches == NULL)
		goto err;

	if (!cJSON_AddItemToObjectChecked(root, "l1_switches",
					  l1_switches)) {
		cJSON_Delete(l1_switches);
		goto err;
					  }

	/*
	 * Ownership of l1_switches has been transferred to root.
	 * The pointer remains valid and is used to add child objects.
	 */
	l1_obj = cJSON_CreateObject();
	if (l1_obj == NULL)
		goto err;

	if (!cJSON_AddItemToObjectChecked(l1_switches,
					  "1D-FULLMESSH",
					  l1_obj)) {
		cJSON_Delete(l1_obj);
		goto err;
					  }

	/*
	 * Ownership of l1_obj has been transferred to l1_switches.
	 * The pointer remains valid and is used to add node objects.
	 */
	for (i = 0; i < g_ip_entry_count; i++) {
		struct ip_entry *entry = &g_ip_entries[i];
		cJSON *ip_obj;
		int j;

		ip_obj = cJSON_CreateObject();
		if (ip_obj == NULL)
			goto err;

		for (j = 0; j < entry->ubpu_num; j++) {
			char ubpu_key[16];
			int written;

			written = snprintf(ubpu_key, sizeof(ubpu_key), "%d",
					   entry->ubpu_ids[j]);
			if (written < 0 ||
				(size_t)written >= sizeof(ubpu_key)) {
				cJSON_Delete(ip_obj);
				goto err;
				}

			if (!cJSON_AddStringToObjectChecked(
					ip_obj, ubpu_key, entry->eids[j])) {
				cJSON_Delete(ip_obj);
				goto err;
					}
		}

		if (!cJSON_AddItemToObjectChecked(l1_obj, entry->ip,
						  ip_obj)) {
			cJSON_Delete(ip_obj);
			goto err;
						  }

		/* Ownership of ip_obj has been transferred to l1_obj. */
	}

	return root;

	err:
		cJSON_Delete(root);
	return NULL;
}

/* ========================================================================
 * 公开接口
 * ======================================================================== */

int generate_topology_from_restconf(const char *output_file)
{
	char *xml_buf = NULL;
	char *json_str = NULL;
	cJSON *root = NULL;
	FILE *fp = NULL;
	size_t json_len;
	int saved_errno;
	int ret;

	if (output_file == NULL || output_file[0] == '\0')
		return -EINVAL;

	/*
	 * Step 1: Fetch static URMA EID information through RESTCONF.
	 */
	ret = fetch_xml_from_curl(&xml_buf);
	if (ret != 0)
		return ret;

	/*
	 * Step 2: Parse the XML response.
	 */
	ret = parse_xml_from_buffer(xml_buf);
	free(xml_buf);
	xml_buf = NULL;
	if (ret != 0)
		return ret;

	/*
	 * Step 3: Group EIDs by generated node IP.
	 */
	group_by_ip();
	if (g_ip_entry_count == 0)
		return -ENOENT;

	/*
	 * Step 4: Build topology JSON.
	 */
	root = build_topology_json();
	if (root == NULL)
		return -ENOMEM;

	json_str = cJSON_Print(root);
	cJSON_Delete(root);
	root = NULL;
	if (json_str == NULL)
		return -ENOMEM;

	/*
	 * Step 5: Write topology JSON to the output file.
	 */
	fp = fopen(output_file, "wb");
	if (fp == NULL) {
		saved_errno = errno;
		free(json_str);
		return -saved_errno;
	}

	json_len = strlen(json_str);

	if (fwrite(json_str, 1, json_len, fp) != json_len ||
		fputc('\n', fp) == EOF) {
		saved_errno = errno ? errno : EIO;

		fclose(fp);
		free(json_str);
		unlink(output_file);
		return -saved_errno;
		}

	if (fclose(fp) != 0) {
		saved_errno = errno ? errno : EIO;

		free(json_str);
		unlink(output_file);
		return -saved_errno;
	}

	free(json_str);
	return 0;
}