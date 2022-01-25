/*
 * vbat_hkadc.h
 *
 * vbat hkadc driver
 *
 * Copyright (c) 2012-2018 Huawei Technologies Co., Ltd.
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

#ifndef _VBAT_HKADC_H_
#define _VBAT_HKADC_H_

#define VBAT_HKADC_RETRY_TIMES      (3)
#define VBAT_HKADC_COEF_MULTIPLE    (1000)

#define V_OFFSET_A_DEFAULT          (1000000)
#define V_OFFSET_A_MIN              (800000)
#define V_OFFSET_A_MAX              (1200000)
#define V_OFFSET_B_DEFAULT          (0)
#define V_OFFSET_B_MIN              (-2000000)  /* uV */
#define V_OFFSET_B_MAX              (2000000)   /* uV */

enum vbat_hkadc_sysfs_type {
	VBAT_HKADC_SYSFS_V_OFFSET_A,
	VBAT_HKADC_SYSFS_V_OFFSET_B,
	VBAT_HKADC_SYSFS_BAT1_CALI_VOL,
};

struct vbat_hkadc_info {
	struct platform_device *pdev;
	struct device *dev;
	int adc_channel;
	int coef;
	int r_pcb;
};

#endif /* _VBAT_HKADC_H_ */
