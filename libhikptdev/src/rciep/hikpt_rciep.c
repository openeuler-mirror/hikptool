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
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/file.h>
#include <fcntl.h>
#include <dirent.h>
#include <stdint.h>
#include <errno.h>
#include "hikpt_rciep.h"

#define MAX_BUS_PCI_DIR_LEN 300
#define MAX_PCI_ID_LEN 6
#define HIKP_BUS_PCI_DEV_DIR "/sys/bus/pci/devices/"
#define HIKP_PCI_REVISION_DIR "/revision"

static int g_iep_fd;
static uint8_t g_unmap_flag;
static volatile union hikp_space_req *g_hikp_req;
static volatile union hikp_space_rsp *g_hikp_rsp;

static int hikp_memcpy_io(void *dst, size_t dst_size, void const *src, size_t src_size)
{
	uint32_t i;

	if (dst_size < src_size)
		return -EINVAL;

	for (i = 0; i < src_size / sizeof(uint32_t); i++)
		((uint32_t *)dst)[i] = ((uint32_t *)src)[i];

	return 0;
}

static void hikp_memclr_io(void)
{
	uint32_t i;

	for (i = 0; i < HIKP_REQ_DATA_MAX; i++)
		g_hikp_req->field.data[i] = 0;
}

static int hikp_try_lock(void)
{
	int count;
	pid_t pid;

	pid = getpid();
	count = MAX_LOCK_COUNT;
	/* Try to lock */
	do {
		if (flock(g_iep_fd, LOCK_EX | LOCK_NB) == 0) {
			g_hikp_req->field.sw_db.db_flag = 1;
			g_hikp_req->field.pid_record = (uint32_t)pid;
			g_hikp_req->field.sw_db.db_mask = 0;
			return 0;
		}
		count--;
		usleep(LOCK_CHECK_GAP_US);
	} while (count);
	printf("dev lock by other process:%u.\n", g_hikp_req->field.pid_record);

	return -EBUSY;
}

void hikp_unlock(void)
{
	if (g_unmap_flag == 0 && g_hikp_req && g_hikp_req->field.sw_db.db_flag == 1) {
		g_hikp_req->field.sw_db.db_flag = 0;
		g_hikp_req->field.sw_db.db_mask = 1;
	}
}

static void hikp_init_cpl_status(void)
{
	g_hikp_req->field.cpl_status = 0;
}

static uint32_t hikp_wait_for_cpl_status(void)
{
	int count = WAIT_CPL_MAX_MS;
	uint32_t status;

	do {
		status = g_hikp_rsp->field.cpl_status;
		if (status != HIKP_INIT_STAT)
			return status;
		count--;
		usleep(CPL_CHECK_GAP_US);
	} while (count);

	return HIKP_APP_WAIT_TIMEOUT;
}

static void req_issue(void)
{
	hikp_init_cpl_status();
	g_hikp_req->field.sw_db.db_trig = 1;
}

static void hikp_cmd_header_set(struct hikp_cmd_header *req_header)
{
	if (req_header == NULL)
		return;

	g_hikp_req->field.req_header.version = req_header->version;
	g_hikp_req->field.req_header.mod_code = req_header->mod_code;
	g_hikp_req->field.req_header.cmd_code = req_header->cmd_code;
	g_hikp_req->field.req_header.sub_cmd_code = req_header->sub_cmd_code;
}

void hikp_cmd_init(struct hikp_cmd_header *req_header, uint32_t mod_code,
		   uint32_t cmd_code, uint32_t sub_cmd_code)
{
	if (req_header == NULL) {
		printf("Command init header NULL.\n");
		return;
	}

	memset(req_header, 0, sizeof(struct hikp_cmd_header));

	req_header->mod_code = mod_code;
	req_header->cmd_code = cmd_code;
	req_header->sub_cmd_code = sub_cmd_code;
}

static int hikp_rep_init(void const *req_data, uint32_t req_size,
			 uint32_t **align_req_data, uint32_t *align_data_num)
{
	size_t data_num;

	if (req_data == NULL) {
		printf("The request data is NULL.\n");
		return -EINVAL;
	}
	data_num = (req_size + (sizeof(uint32_t) - 1)) / sizeof(uint32_t);
	if (data_num > HIKP_REQ_DATA_MAX) {
		printf("request data num(%u) exceeds max size(%u).\n", data_num, HIKP_REQ_DATA_MAX);
		return -EINVAL;
	}
	if (data_num != 0) {
		*align_req_data = (uint32_t *)calloc(data_num, sizeof(uint32_t));
		if (*align_req_data == NULL) {
			printf("request memory malloc failed.\n");
			return -ENOMEM;
		}
		memcpy(*align_req_data, req_data, req_size);
	}
	g_hikp_req->field.req_para_num = data_num;
	*align_data_num = data_num;

	return 0;
}

static int hikp_req_first_round(uint32_t *req_data, uint32_t rep_num, uint32_t *cpl_status)
{
	size_t src_size, dst_size;
	int ret;

	if (req_data == NULL)
		return 0;

	src_size = rep_num * sizeof(uint32_t);
	dst_size = sizeof(g_hikp_req->field.data);
	hikp_memclr_io();
	ret = hikp_memcpy_io((uint32_t *)(g_hikp_req->field.data), dst_size, req_data, src_size);
	if (ret != 0) {
		printf("size error, dst_size:%u, src_size:%u.\n", dst_size, src_size);
		return ret;
	}
	g_hikp_req->field.exe_round = 0;
	req_issue(); /* On the first round, an interrupt is triggered. */
	*cpl_status = hikp_wait_for_cpl_status();
	if (*cpl_status != HIKP_CPL_BY_TF && *cpl_status != HIKP_CPL_BY_IMU) {
		printf("First round failed. Error code:%u.\n", *cpl_status);
		return RCIEP_FAIL;
	}

	return 0;
}

static int hikp_multi_round_interact(struct hikp_cmd_ret **cmd_ret, uint32_t status)
{
	struct hikp_cmd_ret *p_cmd_ret;
	uint32_t src_size, dst_size;
	uint32_t cycle, i;
	uint32_t rsp_num;
	uint32_t cpl_status;

	rsp_num = g_hikp_rsp->field.rsp_para_num;
	if (rsp_num > HIKP_RSP_ALL_DATA_MAX) {
		printf("Response data num[%u] out of range[%d].\n", rsp_num, HIKP_RSP_ALL_DATA_MAX);
		return -EINVAL;
	}

	p_cmd_ret = (struct hikp_cmd_ret *)malloc(sizeof(struct hikp_cmd_ret) +
		    rsp_num * sizeof(uint32_t));
	if (p_cmd_ret == NULL) {
		printf("response memory malloc fail.\n");
		return -ENOMEM;
	}
	p_cmd_ret->version = g_hikp_rsp->field.version;
	p_cmd_ret->rsp_data_num = rsp_num;

	cycle = (rsp_num + (HIKP_RSP_DATA_MAX - 1)) / HIKP_RSP_DATA_MAX;
	for (i = 0; i < cycle; i++) {
		if (i != 0) {
			g_hikp_req->field.exe_round = i;
			if (status == HIKP_CPL_BY_TF)
				req_issue(); /* For next rounds, interrupt is triggered again. */
			else
				hikp_init_cpl_status();

			cpl_status = hikp_wait_for_cpl_status();
			if (cpl_status != HIKP_CPL_BY_TF && cpl_status != HIKP_CPL_BY_IMU) {
				printf("multi round failed, Error code:%u.\n", cpl_status);
				p_cmd_ret->status = RCIEP_FAIL;
				*cmd_ret = p_cmd_ret;
				return RCIEP_FAIL;
			}
		}
		src_size = (i == cycle - 1) ?
			(rsp_num - (i * HIKP_RSP_DATA_MAX)) * sizeof(uint32_t) :
			HIKP_RSP_DATA_MAX * sizeof(uint32_t);
		dst_size = src_size;
		(void)hikp_memcpy_io(&(p_cmd_ret->rsp_data)[i * HIKP_RSP_DATA_MAX],
			dst_size, (uint32_t *)(g_hikp_rsp->field.data), src_size);
	}
	p_cmd_ret->status = 0;
	*cmd_ret = p_cmd_ret;

	return 0;
}

/*
 * Before using the returned struct hikp_cmd_ret structure, check the following:
 * 1. Whether NULL is returned
 * 2. Check whether the status field of struct hikp_cmd_ret meets the expectation
 * 3. Use rsp_data according to rsp_data_num
 */
struct hikp_cmd_ret *hikp_cmd_alloc(struct hikp_cmd_header *req_header,
				    const void *req_data, uint32_t req_size)
{
	struct hikp_cmd_ret *cmd_ret = NULL;
	uint32_t *p_req_data = NULL;
	uint32_t rep_num, rsp_num;
	uint32_t cpl_status = HIKP_INIT_STAT;
	int ret;

	ret = hikp_rep_init(req_data, req_size, &p_req_data, &rep_num);
	if (ret)
		return NULL;

	hikp_cmd_header_set(req_header);

	ret = hikp_req_first_round(p_req_data, rep_num, &cpl_status);
	if (ret)
		goto free_req_data;

	ret = hikp_multi_round_interact(&cmd_ret, cpl_status);
	if (ret)
		goto free_req_data;

free_req_data:
	free(p_req_data);

	return cmd_ret;
}

int hikp_rsp_normal_check(const struct hikp_cmd_ret *cmd_ret)
{
	if (cmd_ret == NULL)
		return -ENOSPC;

	if (cmd_ret->status != 0)
		return -EINVAL;

	return 0;
}

int hikp_rsp_normal_check_with_version(const struct hikp_cmd_ret *cmd_ret, uint32_t version)
{
	int ret;

	ret = hikp_rsp_normal_check(cmd_ret);
	if (ret)
		return ret;

	if (cmd_ret->version != version)
		return -EINVAL;

	return 0;
}

static int hikp_read_pci_info(const char *file_path, uint32_t len, char *content)
{
	char path[PATH_MAX + 1] = { 0 };
	int ret;
	int fd;

	if (file_path == NULL || content == NULL)
		return -EINVAL;

	if (len > MAX_PCI_ID_LEN)
		return -EINVAL;

	if (strlen(file_path) > PATH_MAX || realpath(file_path, path) == NULL)
		return -ENOENT;

	fd = open(path, O_RDONLY);
	if (fd < 0)
		return -EPERM;

	ret = pread(fd, content, len, 0);
	if (ret < 0) {
		close(fd);
		return -EIO;
	}
	content[len] = '\0'; // The invoker ensures that the bounds are not crossed.
	close(fd);

	return 0;
}

static int hikp_iep_check_item(const char *src_str, uint32_t len, const char *target_str)
{
	char tmp_str[MAX_PCI_ID_LEN + 1] = { 0 };
	int ret;

	ret = hikp_read_pci_info(src_str, len, tmp_str);
	if (ret != 0)
		return ret;

	if (strcmp(tmp_str, target_str) != 0)
		return -ENXIO;

	return 0;
}

static int hikp_iep_check(const char *tmp_vendor_dir,
			  const char *tmp_device_dir, const char *revision_dir)
{
	int ret;

	ret = hikp_iep_check_item(tmp_device_dir, MAX_PCI_ID_LEN, HIKP_IEP_DEV_ID);
	if (ret != 0)
		return ret;

	ret = hikp_iep_check_item(tmp_vendor_dir, MAX_PCI_ID_LEN, HIKP_IEP_VENDOR_ID);
	if (ret != 0)
		return ret;

	ret = hikp_iep_check_item(revision_dir, MAX_PCI_REVISION_LEN, HIKP_IEP_REVISION);
	if (ret != 0)
		printf("Revision id not match %s.\n", HIKP_IEP_REVISION);

	return 0;
}

static void hikp_snprintf_rst_record(int err_ret, int *flag)
{
	if (err_ret <= 0 || *flag != 0)
		*flag = RCIEP_FAIL;
}

static char *hikp_dir_alloc(uint32_t sub_dir, const struct dirent *entry, int *flag)
{
	char *ret_dir = NULL;
	int err_ret = 0;

	ret_dir = (char *)calloc(MAX_BUS_PCI_DIR_LEN, sizeof(char));
	if (ret_dir == NULL) {
		printf("malloc dir memory 0x%x failed.\n", MAX_BUS_PCI_DIR_LEN);
		return NULL;
	}
	if (sub_dir == HIKP_RESOURCE_DIR) {
		err_ret = snprintf(ret_dir, MAX_BUS_PCI_DIR_LEN, "%s%s%s",
				   HIKP_BUS_PCI_DEV_DIR, entry->d_name, HIKP_PCI_RESOURCE);
		hikp_snprintf_rst_record(err_ret, flag);
	} else {
		err_ret = snprintf(ret_dir, MAX_BUS_PCI_DIR_LEN, "%s%s%s",
				   HIKP_BUS_PCI_DEV_DIR, entry->d_name, HIKP_PCI_CONFIG);
		hikp_snprintf_rst_record(err_ret, flag);
	}
	if (*flag != 0) {
		free(ret_dir);
		return NULL;
	}

	return ret_dir;
}

static char *hikp_get_iep_dir(uint32_t sub_dir)
{
	DIR *dir;
	struct dirent *entry;
	char vendor_dir[MAX_BUS_PCI_DIR_LEN] = { 0 };
	char device_dir[MAX_BUS_PCI_DIR_LEN] = { 0 };
	char revision_dir[MAX_BUS_PCI_DIR_LEN] = { 0 };
	char *ret_dir = NULL;
	int flag = 0;
	int err_ret = 0;
	int ret;

	dir = opendir(HIKP_BUS_PCI_DEV_DIR);
	if (!dir)
		return NULL;

	while ((entry = readdir(dir))) {
		if (entry->d_name[0] == '.')
			continue;

		err_ret = snprintf(vendor_dir, sizeof(vendor_dir), "%s%s%s",
				   HIKP_BUS_PCI_DEV_DIR, entry->d_name, HIKP_PCI_VENDOR_DIR);
		hikp_snprintf_rst_record(err_ret, &flag);
		err_ret = snprintf(device_dir, sizeof(device_dir), "%s%s%s",
				   HIKP_BUS_PCI_DEV_DIR, entry->d_name, HIKP_PCI_DEV_DIR);
		hikp_snprintf_rst_record(err_ret, &flag);
		err_ret = snprintf(revision_dir, sizeof(revision_dir), "%s%s%s",
				   HIKP_BUS_PCI_DEV_DIR, entry->d_name, HIKP_PCI_REVISION_DIR);
		hikp_snprintf_rst_record(err_ret, &flag);
		ret = hikp_iep_check((const char *)vendor_dir, (const char *)device_dir,
				     (const char *)revision_dir);
		if (ret != 0)
			continue;

		ret_dir = hikp_dir_alloc(sub_dir, entry, &flag);
		goto out_close_dir;
	}
	printf("Cannot find Device %s:%s.\n", HIKP_IEP_VENDOR_ID, HIKP_IEP_DEV_ID);
out_close_dir:
	closedir(dir);
	if (flag != 0) {
		printf("Get iep dir snprintf failed, flag = %d.\n", flag);
		free(ret_dir);
		return NULL;
	}

	return ret_dir;
}

static int mem_space_enable(int enable)
{
	int fd;
	int ret;
	unsigned char val;
	char *iep;
	char path[PATH_MAX + 1] = { 0 };

	iep = hikp_get_iep_dir(HIKP_CONFIG_DIR);
	if (iep == NULL)
		return -ENOENT;

	if (strlen(iep) > PATH_MAX || realpath(iep, path) == NULL) {
		ret = -ENOENT;
		goto out_free_iep;
	}
	fd = open(path, O_RDWR);
	if (fd < 0) {
		printf("Cannot open %s.\n", iep);
		ret = -errno;
		goto out_free_iep;
	}

	ret = pread(fd, &val, 1, PCI_COMMAND_REG);
	if (ret != 1) {
		printf("MSE enable pread failed.\n");
		ret = -errno;
		goto out_close_fd;
	}

	if (enable)
		(val) |= (1u << 1); /* set up bit1 for memory space enable */
	else
		(val) &= (~(1u << 1)); /* clear bit1 for memory space disable */

	ret = pwrite(fd, &val, 1, PCI_COMMAND_REG);
	if (ret != 1) {
		printf("MSE enable pwrite failed.\n");
		ret = -errno;
		goto out_close_fd;
	}
	ret = 0;

out_close_fd:
	close(fd);
	fd = -1;
out_free_iep:
	free(iep);

	return ret;
}

static void hikp_munmap(void)
{
	g_unmap_flag = 1;
	munmap((void *)g_hikp_req, sizeof(union hikp_space_req));
	g_hikp_req = NULL;
	g_hikp_rsp = NULL;
}

int hikp_dev_init(void)
{
	size_t i, len;
	int ret = 0;
	char *iep;
	char path[PATH_MAX + 1] = { 0 };

	iep = hikp_get_iep_dir(HIKP_RESOURCE_DIR);
	if (iep == NULL)
		return -ENOENT;

	if (strlen(iep) > PATH_MAX || realpath(iep, path) == NULL) {
		ret = -ENOENT;
		goto out_free_iep;
	}
	g_iep_fd = open(path, O_RDWR | O_SYNC);
	if (g_iep_fd < 0) {
		printf("failed to open %s.\n", iep);
		ret = -errno;
		goto out_free_iep;
	}

	g_hikp_req = (union hikp_space_req *)mmap(0, sizeof(union hikp_space_req),
		     PROT_READ | PROT_WRITE, MAP_SHARED, g_iep_fd, 0);
	if (!g_hikp_req) {
		printf("failed to mmap %s.\n", iep);
		ret = -errno;
		goto out_close_fd;
	}
	g_hikp_rsp = (union hikp_space_rsp *)(void *)g_hikp_req;

	ret = mem_space_enable(1); /* 1: enable mem space */
	if (ret) {
		printf("failed to enable mem space.\n");
		goto out_unmap;
	}

	ret = hikp_try_lock();
	if (ret) {
		printf("timed out waitting for lock.\n");
		goto out_unmap;
	}

	len = (sizeof(union hikp_space_req) - sizeof(struct iep_doorbell)) / sizeof(uint32_t);
	for (i = 0; i < len; i++)
		g_hikp_req->dw[i] = 0;

	close(g_iep_fd);
	g_iep_fd = -1;
	free(iep);
	return ret;

out_unmap:
	hikp_munmap();
out_close_fd:
	close(g_iep_fd);
	g_iep_fd = -1;
out_free_iep:
	free(iep);
	return ret;
}

void hikp_dev_uninit(void)
{
	hikp_unlock();
	hikp_munmap();
}
