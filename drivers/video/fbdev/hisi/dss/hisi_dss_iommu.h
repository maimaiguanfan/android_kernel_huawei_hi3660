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
#ifndef HISI_DSS_IOMMU_H
#define HISI_DSS_IOMMU_H
#include <linux/iommu.h>
#include <linux/fb.h>
#include <linux/dma-mapping.h>
#include <linux/dma-buf.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/version.h>
#include <linux/hisi-iommu.h>
#include "hisi_dss.h"

struct dss_mm_info {
	struct list_head mm_list;
	spinlock_t map_lock;
};

extern struct platform_device *g_hdss_platform_device;

static inline struct device *__hdss_get_dev(void)
{
	if (!g_hdss_platform_device) {
		pr_err("g_hdss_platform_device is null.\n");
		return NULL;
	}

	return &(g_hdss_platform_device->dev);
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,14,0)
struct dma_buf *hisi_dss_get_dmabuf(int sharefd);
void hisi_dss_put_dmabuf(struct dma_buf *buf);
bool hisi_dss_check_addr_validate(dss_img_t *img);
struct dma_buf *hisi_dss_get_buffer_by_sharefd(uint64_t *iova, int fd, uint32_t size);
void hisi_dss_put_buffer_by_dmabuf(uint64_t iova, struct dma_buf *dmabuf);
int hisi_dss_buffer_map_iova(struct fb_info *info, void __user *arg);
int hisi_dss_buffer_unmap_iova(struct fb_info *info, void __user *arg);
#endif

int hisi_dss_iommu_enable(struct platform_device *pdev);
phys_addr_t hisi_dss_domain_get_ttbr(void);
int hisi_dss_alloc_cma_buffer(size_t size, dma_addr_t *dma_handle, void **cpu_addr);
void hisi_dss_free_cma_buffer(size_t size, dma_addr_t dma_handle, void *cpu_addr);

#endif
