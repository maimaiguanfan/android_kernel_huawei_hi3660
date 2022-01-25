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
#ifndef HISI_DSS_ION_H
#define HISI_DSS_ION_H
#include <linux/dma-mapping.h>
#include <linux/dma-buf.h>
#include <linux/device.h>
#include <linux/of_reserved_mem.h>
#include <linux/ion.h>
#include <linux/fb.h>
#include <linux/version.h>
#include <linux/hisi-iommu.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,14,0)
#define CONFIG_ION_ALLOC_BUFFER (0)
#else
#define CONFIG_ION_ALLOC_BUFFER (1)
#endif

#if !defined(CONFIG_SWITCH) || !defined(CONFIG_DP_AUX_SWITCH) || !defined(CONFIG_HW_DP_SOURCE)
#define CONFIG_DP_ENABLE (0)
#else
#define CONFIG_DP_ENABLE (1)
#endif

struct iommu_page_info {
	struct page *page;
	unsigned int order;
	struct list_head list;
};

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,14,0)
void *hisifb_iommu_map_kernel(struct sg_table *sg_table, size_t size);
void hisifb_iommu_unmap_kernel(void *vaddr);
#else
int hisifb_get_ion_phys(struct fb_info *info, void __user *arg);
int hisifb_ion_phys(struct ion_client *client, struct ion_handle *handle,
	struct device *dev, unsigned long *addr, size_t *len);
#endif

int hisi_fb_mmap(struct fb_info *info, struct vm_area_struct * vma);

#endif
