/*
 * storage_rochk.h
 *
 * Storage Read Only Check header file
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

#ifndef STORAGE_ROCHK_INTERNAL_H
#define STORAGE_ROCHK_INTERNAL_H

#define EMMC_MANUFACTURER_ID_TOSHIBA    0x11
#define EMMC_MANUFACTURER_ID_HYNIX      0x90
#define EMMC_MANUFACTURER_ID_SAMSUNG    0x15
#define EMMC_MANUFACTURER_ID_MICRON     0x13
#define EMMC_MANUFACTURER_ID_SANDISK    0x45

#define UFS_MANUFACTURER_ID_TOSHIBA     0x0198
#define UFS_MANUFACTURER_ID_HYNIX       0x01AD
#define UFS_MANUFACTURER_ID_SAMSUNG     0x01CE
#define UFS_MANUFACTURER_ID_MICRON      0x012C
#define UFS_MANUFACTURER_ID_SANDISK     0x0145
#define UFS_MANUFACTURER_ID_HI1861      0x08B6

#define STORAGE_ROINFO_UNKNOWN_OR_UNCARE            0x00
#define STORAGE_ROINFO_LDR_WRITE_PROT               0x01
#define STORAGE_ROINFO_LDR_WRMON_DRIVER             0x02
#define STORAGE_ROINFO_LDR_WRMON_TMOUT              0x03
#define STORAGE_ROINFO_KNL_WRITE_PROT               0x04
#define STORAGE_ROINFO_KNL_WRMON_DRIVER             0x05
#define STORAGE_ROINFO_KNL_WRMON_SUSPICIOUS         0x06
#define STORAGE_ROINFO_KNL_WRTRY_BLOCK              0x07
#define STORAGE_ROINFO_KNL_WRTRY_STUCK              0x08
#define STORAGE_ROINFO_KNL_WRTRY_MISC               0x09

#define STORAGE_ROINFO_METHOD_MASK              0x1F

#define STORAGE_ROCHK_ACTION_NONE    0
#define STORAGE_ROCHK_ACTION_REBOOT  1

#define STORAGE_ROCHK_ROUND_MAX                0x03

#endif
