/*
 * storage_rofa.c
 *
 * storage Read Only First Aid
 *
 * Copyright (c) 2012-2019 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include <linux/kernel.h>
#include <linux/string.h>

#include <chipset_common/storage_rofa/storage_rofa.h>

#define STORAGE_ROFA_STR_CHECK      "check"
#define STORAGE_ROFA_STR_BYPASS     "bypass"
#define ANDROID_BOOT_MODE_NORMAL    "normal"

#define BOPD_MODE_STORAGE_READONLY 0x03
#define BOPD_MODE_STORAGE_READONLY_WITH_CORE_DEGRADE 0x07

static unsigned int g_storage_rochk;
static unsigned int g_storage_row;
static unsigned int g_androidboot_normalmode;
static unsigned int g_bopd_enable_row;

static int __init early_parse_storage_rofa_cmdline(char *cmdline_opt)
{
	if (cmdline_opt == NULL)
		return 0;

	pr_info("early_param storage_rofa option is [%s]\n", cmdline_opt);

	if (strncmp(cmdline_opt, STORAGE_ROFA_STR_CHECK,
	    strlen(STORAGE_ROFA_STR_CHECK)) == 0)
		g_storage_rochk = 0x01;
	else if (strncmp(cmdline_opt, STORAGE_ROFA_STR_BYPASS,
		 strlen(STORAGE_ROFA_STR_BYPASS)) == 0)
		g_storage_row = 0x01;

	return 0;
}
early_param("storage_rofa", early_parse_storage_rofa_cmdline);

static int __init early_parse_androidboot_mode_cmdline(char *cmdline_opt)
{
	if (cmdline_opt == NULL)
		return 0;

	pr_info("early_param androidboot.mode option is [%s]\n", cmdline_opt);

	if (strncmp(cmdline_opt, ANDROID_BOOT_MODE_NORMAL,
	    strlen(ANDROID_BOOT_MODE_NORMAL)) == 0)
		g_androidboot_normalmode = 0x01;

	return 0;
}
early_param("androidboot.mode", early_parse_androidboot_mode_cmdline);

static int __init early_parse_bopd_mode_cmdline(char *bopd_mode_cmdline)
{
	unsigned long long value = 0;

	pr_info("early_param bopd.mode option is [%s]\n", bopd_mode_cmdline);

	if (kstrtoull(bopd_mode_cmdline, 16, &value) == 0 &&
	    (value == BOPD_MODE_STORAGE_READONLY ||
	    value == BOPD_MODE_STORAGE_READONLY_WITH_CORE_DEGRADE))
		g_bopd_enable_row = 0x01;

	return 0;
}
early_param("bopd.mode", early_parse_bopd_mode_cmdline);

unsigned int get_storage_rofa_bootopt(void)
{
	if (g_androidboot_normalmode == 0)
		return STORAGE_ROFA_BOOTOPT_NOP;
	else if (g_bopd_enable_row)
		return STORAGE_ROFA_BOOTOPT_BYPASS;
	else if (g_storage_rochk)
		return STORAGE_ROFA_BOOTOPT_CHECK;
	else
		return STORAGE_ROFA_BOOTOPT_NOP;
}
