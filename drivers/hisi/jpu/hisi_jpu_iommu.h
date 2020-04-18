/* Copyright (c) 2018-2019, Hisilicon Tech. Co., Ltd. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#ifndef HISI_JPU_IOMMU_H
#define HISI_JPU_IOMMU_H
#include <linux/iommu.h>
#include <linux/dma-mapping.h>
#include <linux/dma-buf.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/version.h>
#include <linux/hisi-iommu.h>

struct jpu_iommu_page_info {
	struct page *page;
	unsigned int order;
	struct list_head list;
};
#endif
