/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 * Description: This file contains the function required for init_user and
 *              package management
 * Author: xuejikan 370596
 * Create: 2018-12-10
 */
#ifndef _HWAA_IOCTL_IMPL_H
#define _HWAA_IOCTL_IMPL_H
#include "huawei_platform/hwaa/hwaa_ioctl.h"

void hwaa_sync_installed_packages(
	struct hwaa_sync_installed_packages_t *sync_installed_packages);

/*
 * This function insert package information when install app
 * @param uninstall_package      [in] package information
 */
void hwaa_install_package(struct hwaa_install_package_t *install_package);

/*
 * This function delete package information when uninstall app
 * @param uninstall_package      [in] package information
 */
void hwaa_uninstall_package(struct hwaa_uninstall_package_t *uninstall_package);

void init_user(struct hwaa_init_user_t *iusr);
#endif
