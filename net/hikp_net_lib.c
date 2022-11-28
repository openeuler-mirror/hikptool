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
#include "hikp_net_lib.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <dirent.h>
#include <linux/ethtool.h>
#include <linux/if.h>
#include <linux/netlink.h>
#include <linux/sockios.h>
#include "tool_lib.h"
#include "hikptdev_plug.h"

static int hikp_read_net_pci_info(const char *file_path, uint32_t len, char *content)
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

int hikp_net_creat_sock(void)
{
	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);

	if (sockfd < MIN_SOCKFD) {
		if (sockfd >= 0)
			close(sockfd);
		sockfd = socket(AF_NETLINK, SOCK_RAW, NETLINK_GENERIC);
	}
	if (sockfd < 0) {
		HIKP_ERROR_PRINT("cannot get control socket.\n");
		return -EIO;
	}
	if (sockfd < MIN_SOCKFD) {
		HIKP_ERROR_PRINT("sockfd invalid\n");
		close(sockfd);
		return -EINVAL;
	}
	return sockfd;
}

static void fill_bdf_id(struct bdf_t *bdf, uint8_t bus, uint8_t dev, uint8_t func)
{
	bdf->bus_id = bus;
	bdf->dev_id = dev;
	bdf->fun_id = func;
}

static bool check_and_parse_domain_bdf_id(const char *bus_info, struct bdf_t *bdf)
{
	char remains[IFNAMSIZ + 1] = {0};
	uint32_t domain = 0;
	uint32_t bus = 0;
	uint32_t dev = 0;
	uint32_t fun = 0;
	int retval;

	if (strlen(bus_info) >= IFNAMSIZ)
		return false;

	/* pci_id: 0000:bd:00.0 ==> doman:bus:device.function add
	 * remains here to solve such input: 0000:bd:00.0abcdef
	 */
	retval = sscanf(bus_info, "%x:%x:%x.%u%s", &domain, &bus, &dev, &fun, remains);
	if ((retval == PCI_ID_FOUR_NUMS) && (bus != 0) && !(domain & ~PCI_DOMAIN_MASK) &&
	    !(dev & ~PCI_DEV_ID_MASK) && !(fun & ~PCI_FUNC_ID_MASK)) {
		bdf->domain = domain;
		fill_bdf_id(bdf, (uint8_t)bus, (uint8_t)dev, (uint8_t)fun);
		return true;
	}

	return false;
}

static bool check_and_parse_simple_bdf_id(const char *bus_info, struct bdf_t *bdf)
{
	char remains[IFNAMSIZ + 1] = {0};
	uint32_t bus = 0;
	uint32_t dev = 0;
	uint32_t fun = 0;
	int retval;

	if (strlen(bus_info) >= IFNAMSIZ)
		return false;

	/* pci_id: bd:00.0 ==> bus:device.function */
	retval = sscanf(bus_info, "%x:%x.%u%s", &bus, &dev, &fun, remains);
	if ((retval == PCI_ID_THREE_NUMS) &&
	    (bus != 0) && !(dev & ~PCI_DEV_ID_MASK) && !(fun & ~PCI_FUNC_ID_MASK)) {
		bdf->domain = 0;
		fill_bdf_id(bdf, (uint8_t)bus, (uint8_t)dev, (uint8_t)fun);
		return true;
	}

	return false;
}

static bool check_dev_name_and_get_bdf(int sockfd, struct tool_target *target)
{
	struct ethtool_drvinfo drvinfo = { 0 };
	struct ifreq ifr = { 0 };

	ifr.ifr_data = (char *)&drvinfo;
	drvinfo.cmd = ETHTOOL_GDRVINFO;

	strncpy(ifr.ifr_name, target->dev_name, IFNAMSIZ - 1);

	if (ioctl(sockfd, SIOCETHTOOL, &ifr) < 0)
		return false;

	if (!check_and_parse_domain_bdf_id(drvinfo.bus_info, &(target->bdf)))
		return false;

	if (strncmp(drvinfo.driver, HNS3_DRIVER_NAME, sizeof(HNS3_DRIVER_NAME)) != 0)
		return false;

	return true;
}

static bool interface_is_bdf_id(const char *dev_name, struct tool_target *target)
{
	bool ret;

	ret = check_and_parse_domain_bdf_id(dev_name, &(target->bdf));
	if (ret)
		return true;

	return check_and_parse_simple_bdf_id(dev_name, &(target->bdf));
}

static int tool_get_bdf_by_dev_name(const char *name, struct tool_target *target)
{
	int sockfd;

	if (strlen(name) >= IFNAMSIZ) {
		HIKP_ERROR_PRINT("parameter of target name is too long.\n");
		return -EINVAL;
	}
	strncpy(target->dev_name, name, sizeof(target->dev_name));
	target->dev_name[sizeof(target->dev_name) - 1] = '\0';

	sockfd = hikp_net_creat_sock();
	if (sockfd < 0)
		return sockfd;

	if (!check_dev_name_and_get_bdf(sockfd, target)) {
		HIKP_ERROR_PRINT("device name error or unsupported.\n");
		close(sockfd);
		return -EINVAL;
	}
	close(sockfd);

	return 0;
}

int tool_check_and_get_valid_bdf_id(const char *name, struct tool_target *target)
{
	if (!name || !target)
		return 0;

	if (interface_is_bdf_id(name, target))
		return 0;

	return tool_get_bdf_by_dev_name(name, target);
}

bool is_dev_valid_and_special(int sockfd, struct tool_target *target)
{
	if (!check_dev_name_and_get_bdf(sockfd, target))
		return false;

	/* if device id is zero, we can assume that it is special. */
	if (target->bdf.dev_id != 0)
		return false;

	return true;
}

int get_revision_id_by_bdf(const struct bdf_t *bdf, char *revision_id)
{
	char revision_dir[MAX_BUS_PCI_DIR_LEN] = { 0 };
	int ret;

	ret = snprintf(revision_dir, sizeof(revision_dir), "%s%04x:%02x:%02x.%u%s",
		       HIKP_BUS_PCI_DEV_DIR, bdf->domain, bdf->bus_id, bdf->dev_id,
		       bdf->fun_id, HIKP_PCI_REVISION_DIR);
	if (ret < 0) {
		HIKP_ERROR_PRINT("get revision dir fail.\n");
		return -EIO;
	}
	ret = hikp_read_net_pci_info((const char *)revision_dir, MAX_PCI_ID_LEN, revision_id);
	if (ret != 0)
		return ret;

	return 0;
}

static int hikp_get_dir_name_of_device(const char *path, uint32_t len, char *dir_name)
{
	struct dirent *ptr;
	DIR *dir = NULL;

	if (len > PCI_MAX_DIR_NAME_LEN)
		return -EINVAL;

	dir = opendir(path);
	if (dir == NULL) {
		HIKP_ERROR_PRINT("read path %s fail.\n", path);
		return -EINVAL;
	}

	while ((ptr = readdir(dir)) != NULL) {
		if (ptr->d_name[0] == '.')
			continue;

		if (strlen(ptr->d_name) >= len) {
			HIKP_ERROR_PRINT("parameter of directory name is too long.\n");
			closedir(dir);
			return -EINVAL;
		}
		strncpy(dir_name, ptr->d_name, len);
		dir_name[len - 1] = '\0';
		break;
	}

	return closedir(dir);
}

int get_dev_name_by_bdf(const struct bdf_t *bdf, char *dev_name)
{
	char dev_name_dir[MAX_BUS_PCI_DIR_LEN] = { 0 };
	int ret;

	if (!dev_name || !bdf)
		return -EINVAL;

	/* if dev_name already has a value, we do not need to obtain it. */
	if (dev_name[0] != 0)
		return 0;

	ret = snprintf(dev_name_dir, sizeof(dev_name_dir), "%s%04x:%02x:%02x.%u%s",
		       HIKP_BUS_PCI_DEV_DIR, bdf->domain, bdf->bus_id, bdf->dev_id,
		       bdf->fun_id, HIKP_NET_PATH);
	if (ret < 0) {
		HIKP_ERROR_PRINT("get dev_name dir fail.\n");
		return -EIO;
	}
	if (!is_dir_exist(dev_name_dir))
		return -ENOENT;

	return hikp_get_dir_name_of_device(dev_name_dir, IFNAMSIZ, dev_name);
}

int get_vf_dev_info_by_pf_dev_name(const char *pf_dev_name,
				   struct tool_target *vf_target, uint8_t vf_id)
{
	char dev_name_dir[MAX_BUS_PCI_DIR_LEN] = { 0 };
	int ret;

	if (!pf_dev_name || !vf_target)
		return -EINVAL;

	ret = snprintf(dev_name_dir, sizeof(dev_name_dir), "%s%s%s%u%s", HIKP_NET_DEV_PATH,
		       pf_dev_name, HIKP_VIRTFN_PATH, vf_id, HIKP_NET_PATH);
	if (ret < 0 || ret >= sizeof(dev_name_dir)) {
		HIKP_ERROR_PRINT("get vf dev_name dir fail.\n");
		return -EIO;
	}
	if (!is_dir_exist(dev_name_dir))
		return -ENOENT;

	ret = hikp_get_dir_name_of_device(dev_name_dir, IFNAMSIZ, vf_target->dev_name);
	if (ret != 0)
		return ret;

	return tool_get_bdf_by_dev_name(vf_target->dev_name, vf_target);
}

int get_pf_dev_info_by_vf_dev_name(const char *vf_dev_name, struct tool_target *pf_target)
{
	char dev_name_dir[MAX_BUS_PCI_DIR_LEN] = { 0 };
	int ret;

	if (!vf_dev_name || !pf_target)
		return -EINVAL;

	ret = snprintf(dev_name_dir, sizeof(dev_name_dir), "%s%s%s", HIKP_NET_DEV_PATH,
		       vf_dev_name, HIKP_PHYSFN_PATH);
	if (ret < 0 || ret >= sizeof(dev_name_dir)) {
		HIKP_ERROR_PRINT("get vf dev_name dir fail.\n");
		return -EIO;
	}
	if (!is_dir_exist(dev_name_dir))
		return -ENOENT;

	ret = hikp_get_dir_name_of_device(dev_name_dir, IFNAMSIZ, pf_target->dev_name);
	if (ret != 0)
		return ret;

	return tool_get_bdf_by_dev_name(pf_target->dev_name, pf_target);
}

int get_numvfs_by_bdf(const struct bdf_t *bdf, uint8_t *numvfs)
{
#define MAX_STR_LEN_OF_NUMVFS 8
	char numvfs_dir[MAX_BUS_PCI_DIR_LEN] = { 0 };
	char numvf[MAX_STR_LEN_OF_NUMVFS] = { 0 };
	int ret;

	if (!bdf || !numvfs)
		return -EINVAL;

	ret = snprintf(numvfs_dir, sizeof(numvfs_dir), "%s%04x:%02x:%02x.%u%s",
		       HIKP_BUS_PCI_DEV_DIR, bdf->domain, bdf->bus_id, bdf->dev_id,
		       bdf->fun_id, HIKP_SRIOV_NUMVFS_PATH);
	if (ret < 0) {
		HIKP_ERROR_PRINT("get numvfs dir fail.\n");
		return -EIO;
	}
	ret = hikp_read_net_pci_info((const char *)numvfs_dir, MAX_PCI_ID_LEN, numvf);
	if (ret != 0)
		return ret;

	ret = (int)strtol(numvf, NULL, 0);
	if ((ret > UCHAR_MAX) || (ret < 0)) {
		HIKP_ERROR_PRINT("get numvfs by bdf fail.\n");
		return -EINVAL;
	}
	*numvfs = (uint8_t)ret;

	return 0;
}

void hikp_ether_format_addr(char *buf, uint16_t size, const uint8_t *mac_addr)
{
	int len;

	if (buf == NULL || mac_addr == NULL) {
		HIKP_WARN_PRINT("buf or mac_addr pointer is NULL.\n");
		return;
	}

	len = snprintf(buf, size, "%02X:%02X:%02X:%02X:%02X:%02X", mac_addr[0], mac_addr[1],
		       mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
	if (len < 0 || len >= size)
		HIKP_WARN_PRINT("fail to get ether format addr.\n");
}
