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
#include "hisi_dss_iommu.h"
#include "hisi_fb.h"

struct dss_iova_info {
	struct list_head list_node;
	struct dma_buf *dmabuf;
	iova_info_t iova_info;
};

struct platform_device *g_hdss_platform_device = NULL;

int hisi_dss_alloc_cma_buffer(size_t size, dma_addr_t *dma_handle, void **cpu_addr)
{
	*cpu_addr = dma_alloc_coherent(__hdss_get_dev(), size, dma_handle, GFP_KERNEL);
	if (!*cpu_addr) {
		HISI_FB_ERR("dma alloc coherent failed!\n");
		return -ENOMEM;
	}
	return 0;
}

void hisi_dss_free_cma_buffer(size_t size, dma_addr_t dma_handle, void *cpu_addr)
{
	if ((size > 0) && (cpu_addr != NULL)) {
		dma_free_coherent(__hdss_get_dev(), size, cpu_addr, dma_handle);
	}
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,14,0)
static struct dss_mm_info g_mm_list;
static void hisi_dss_buffer_iova_dump(void)
{
	struct dss_iova_info *node, *_node_;
	struct dss_mm_info *mm_list = &g_mm_list;

	spin_lock(&mm_list->map_lock);
	list_for_each_entry_safe(node, _node_, &mm_list->mm_list, list_node) {
		HISI_FB_INFO("Display dump: daf(%p) iva: 0x%llx size: 0x%llx calling_pid: %d .\n",
			node->dmabuf, node->iova_info.iova, node->iova_info.size,
			node->iova_info.calling_pid);
	}
	spin_unlock(&mm_list->map_lock);
}

struct dma_buf *hisi_dss_get_dmabuf(int sharefd)
{
	struct dma_buf *buf = NULL;

	/* dim layer share fd -1 */
	if (sharefd < 0) {
		HISI_FB_ERR("Invalid file sharefd(%d).\n", sharefd);
		return NULL;
	}

	buf = dma_buf_get(sharefd);
	if (IS_ERR(buf)) {
		HISI_FB_ERR("Invalid file buf(%p).\n", buf);
		return NULL;
	}

	/* show_stack(current, NULL); */
	HISI_FB_DEBUG("get dma buf(%p).\n", buf);

	return buf;
}

void hisi_dss_put_dmabuf(struct dma_buf *buf)
{
	if (IS_ERR(buf)) {
		HISI_FB_ERR("Invalid dmabuf(%p).\n", buf);
		return;
	}

	dma_buf_put(buf);
	HISI_FB_DEBUG("put dma buf(%p).\n", buf);
}

struct dma_buf *hisi_dss_get_buffer_by_sharefd(uint64_t *iova, int fd, uint32_t size)
{
	unsigned long buf_size = 0;
	struct dma_buf *buf = NULL;

	buf = hisi_dss_get_dmabuf(fd);
	if (IS_ERR(buf)) {
		HISI_FB_ERR("Invalid file shared_fd(%d).\n", fd);
		return NULL;
	}

	*iova = hisi_iommu_map_dmabuf(__hdss_get_dev(), buf, 0, &buf_size);
	if ((*iova == 0) || (buf_size < size)) {
		HISI_FB_ERR("get iova_size(0x%llx) smaller then size(0x%lx). \n",
			buf_size, size);
		if (*iova) {
			hisi_iommu_unmap_dmabuf(__hdss_get_dev(), buf, *iova);
			*iova = 0;
		}
		return NULL;
	}

	return buf;
}

void hisi_dss_put_buffer_by_dmabuf(uint64_t iova, struct dma_buf *dmabuf)
{
	if (IS_ERR(dmabuf)) {
		HISI_FB_ERR("Invalid dmabuf(%p).\n", dmabuf);
		return;
	}
	hisi_iommu_unmap_dmabuf(__hdss_get_dev(), dmabuf, iova);

	hisi_dss_put_dmabuf(dmabuf);
}

bool hisi_dss_check_addr_validate(dss_img_t *img)
{
	uint64_t iova = 0;
	struct dma_buf *buf = NULL;

	if (!img) {
		HISI_FB_ERR("img is null.\n");
		return false;
	}

	buf = hisi_dss_get_buffer_by_sharefd(&iova, img->shared_fd, img->buf_size);
	if ((buf == NULL) || (iova == 0)) {
		HISI_FB_ERR("buf or iova is error.\n");
		return false;
	}

	if (img->vir_addr != iova) {
		img->vir_addr = iova;
		img->afbc_header_addr = iova + img->afbc_header_offset;
		img->afbc_payload_addr = iova + img->afbc_payload_offset;
		img->hfbc_header_addr0 = iova + img->hfbc_header_offset0;
		img->hfbc_payload_addr0 = iova + img->hfbc_payload_offset0;
		img->hfbc_header_addr1 = iova + img->hfbc_header_offset1;
		img->hfbc_payload_addr1 = iova + img->hfbc_payload_offset1;
	}

	/* don't unmap iova, dangerous!!!
	 * iova would be unmapped by dmabuf put.
	 */
	hisi_dss_put_dmabuf(buf);

	return true;
}

int hisi_dss_iommu_enable(struct platform_device *pdev)
{
	phys_addr_t ttbr = 0;
	struct dss_mm_info *mm_list = &g_mm_list;
	if (!pdev) {
		HISI_FB_ERR("pdev is NULL.\n");
		return -EINVAL;
	}
	ttbr = hisi_domain_get_ttbr(&pdev->dev);

	dma_set_mask_and_coherent(&pdev->dev, DMA_BIT_MASK(64));

	g_hdss_platform_device = pdev;

	spin_lock_init(&mm_list->map_lock);
	INIT_LIST_HEAD(&mm_list->mm_list);

	return 0;
}

phys_addr_t hisi_dss_domain_get_ttbr(void)
{
	return hisi_domain_get_ttbr(__hdss_get_dev());
}

int hisi_dss_alloc_cmdlist_buffer(struct hisi_fb_data_type *hisifd)
{
	void *cpu_addr = NULL;
	size_t buf_len = 0;
	dma_addr_t dma_handle = 0;

	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL.\n");
		return -EINVAL;
	}

	buf_len = hisifd->sum_cmdlist_pool_size;
	cpu_addr = dma_alloc_coherent(__hdss_get_dev(), buf_len, &dma_handle, GFP_KERNEL);
	if (!cpu_addr) {
		HISI_FB_ERR("fb%d dma alloc 0x%zxB coherent failed!\n", hisifd->index, buf_len);
		return -ENOMEM;
	}
	hisifd->cmdlist_pool_vir_addr = cpu_addr;
	hisifd->cmdlist_pool_phy_addr = dma_handle;

	memset(hisifd->cmdlist_pool_vir_addr, 0, buf_len);

	return 0;
}

void hisi_dss_free_cmdlist_buffer(struct hisi_fb_data_type *hisifd)
{
	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL.\n");
		return;
	}

	if (hisifd->cmdlist_pool_vir_addr != 0) {
		dma_free_coherent(__hdss_get_dev(), hisifd->sum_cmdlist_pool_size,
			hisifd->cmdlist_pool_vir_addr, hisifd->cmdlist_pool_phy_addr);
		hisifd->cmdlist_pool_vir_addr = 0;
	}
}

int hisi_dss_buffer_map_iova(struct fb_info *info, void __user *arg)
{
	iova_info_t map_data;
	struct dss_iova_info *node = NULL;
	struct dss_mm_info *mm_list = &g_mm_list;

	node = kzalloc(sizeof(struct dss_iova_info), GFP_KERNEL);
	if (node == NULL) {
		HISI_FB_ERR("alloc display meminfo failed.\n");
		goto error;
	}

	if (copy_from_user(&map_data, (void __user *)arg, sizeof(map_data))) {
		HISI_FB_ERR("copy_from_user failed.\n");
		goto error;
	}

	node->iova_info.share_fd = map_data.share_fd;
	node->iova_info.calling_pid = map_data.calling_pid;
	node->iova_info.size = map_data.size;
	node->dmabuf = hisi_dss_get_buffer_by_sharefd(&map_data.iova,
		map_data.share_fd, map_data.size);
	if (!node->dmabuf) {
		HISI_FB_ERR("dma buf map share_fd(%d) failed.\n", map_data.share_fd);
		goto error;
	}
	node->iova_info.iova = map_data.iova;

	if (copy_to_user((void __user *)arg, &map_data, sizeof(map_data))) {
		HISI_FB_ERR("copy_to_user failed.\n");
		goto error;
	}

	/* save map list */
	spin_lock(&mm_list->map_lock);
	list_add_tail(&node->list_node, &mm_list->mm_list);
	spin_unlock(&mm_list->map_lock);

	DDTF(g_debug_dump_iova, hisi_dss_buffer_iova_dump);

	return 0;

error:
	if (node) {
		if (node->dmabuf) {
			hisi_dss_put_buffer_by_dmabuf(node->iova_info.iova, node->dmabuf);
		}
		kfree(node);
	}
	return -EFAULT;
}

int hisi_dss_buffer_unmap_iova(struct fb_info *info, void __user *arg)
{
	iova_info_t umap_data;
	struct dma_buf *dmabuf = NULL;
	struct dss_iova_info *node, *_node_;
	struct dss_mm_info *mm_list = &g_mm_list;

	if (copy_from_user(&umap_data, (void __user *)arg, sizeof(umap_data))) {
		HISI_FB_ERR("copy_from_user failed.\n");
		return -EFAULT;
	}
	dmabuf = hisi_dss_get_dmabuf(umap_data.share_fd);

	spin_lock(&mm_list->map_lock);
	list_for_each_entry_safe(node, _node_, &mm_list->mm_list, list_node) {
		if (node->dmabuf == dmabuf) {
			list_del(&node->list_node);
			/* already map, need put twice.*/
			hisi_dss_put_dmabuf(node->dmabuf);
			/* iova would be unmapped by dmabuf put. */
			kfree(node);
		}
	}
	spin_unlock(&mm_list->map_lock);

	hisi_dss_put_dmabuf(dmabuf);

	DDTF(g_debug_dump_iova, hisi_dss_buffer_iova_dump);

	return 0;
}

#else

struct iommu_domain* g_hdss_domain = NULL;
phys_addr_t hisi_dss_domain_get_ttbr(void)
{
	struct iommu_domain_data *domain_data = NULL;

	if (!g_hdss_domain) {
		HISI_FB_ERR("g_hdss_domain is null.\n");
		return 0;
	}

	domain_data = (struct iommu_domain_data *)(g_hdss_domain->priv);
	if (!domain_data) {
		HISI_FB_ERR("domain_data is null.\n");
		return 0;
	}

	return domain_data->phy_pgd_base;
}

int hisi_dss_iommu_enable(struct platform_device *pdev)
{
	struct iommu_domain *hisi_domain = NULL;
	struct iommu_domain_data *domain_data = NULL;
	if (NULL == pdev) {
		HISI_FB_ERR("pdev is NULL.\n");
		return -EINVAL;
	}

	/* create iommu domain */
	hisi_domain = hisi_ion_enable_iommu(pdev);
	if (!hisi_domain) {
		HISI_FB_ERR("iommu_domain_alloc failed!\n");
		return -EINVAL;
	}
	domain_data = (struct iommu_domain_data *)(hisi_domain->priv);
	if (!domain_data) {
		HISI_FB_ERR("domain_data is null!\n");
		return -EINVAL;
	}

	/* save domain and platform dev */
	g_hdss_domain = hisi_domain;
	g_hdss_platform_device = pdev;

	return 0;
}

int hisi_dss_alloc_cmdlist_buffer(struct hisi_fb_data_type *hisifd)
{
	int ret = 0;
	size_t tmp = 0;
	void *cpu_addr = NULL;
	size_t buf_len = 0;
	dma_addr_t dma_handle = 0;

	hisifd->cmdlist_pool_vir_addr = 0;
	hisifd->cmdlist_pool_phy_addr = 0;

	buf_len = hisifd->sum_cmdlist_pool_size;
	/* alloc cmdlist pool buffer */
	hisifd->cmdlist_pool_ion_handle =
		ion_alloc(hisifd->buffer_client, buf_len, 0, ION_HEAP(ION_GRALLOC_HEAP_ID), 0);
	if (IS_ERR(hisifd->cmdlist_pool_ion_handle)) {
		HISI_FB_ERR("failed to ion alloc cmdlist_ion_handle.\n");
		return -ENOMEM;
	}

	cpu_addr = ion_map_kernel(hisifd->buffer_client, hisifd->cmdlist_pool_ion_handle);
	if (!cpu_addr) {
		HISI_FB_ERR("failed to ion_map_kernel cmdlist_pool_vir_addr!\n");
		return -ENOMEM;
	}
	hisifd->cmdlist_pool_vir_addr = cpu_addr;

	ret = hisifb_ion_phys(hisifd->buffer_client, hisifd->cmdlist_pool_ion_handle,
			&(hisifd->pdev->dev), (unsigned long *)&dma_handle, &tmp);
	if (ret < 0) {
		HISI_FB_ERR("failed to ion_phys node->header_phys!\n");
		return -ENOMEM;
	}
	hisifd->cmdlist_pool_phy_addr = dma_handle;

	memset(hisifd->cmdlist_pool_vir_addr, 0, buf_len);

	return 0;
}

void hisi_dss_free_cmdlist_buffer(struct hisi_fb_data_type *hisifd)
{
	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL.\n");
		return;
	}

	if (!hisifd->buffer_client) {
		HISI_FB_ERR("buffer_client is NULL.\n");
		return;
	}

	if (hisifd->cmdlist_pool_vir_addr && hisifd->cmdlist_pool_ion_handle) {
		ion_unmap_kernel(hisifd->buffer_client, hisifd->cmdlist_pool_ion_handle);
		hisifd->cmdlist_pool_vir_addr = NULL;
	}

	if (hisifd->cmdlist_pool_ion_handle) {
		ion_free(hisifd->buffer_client, hisifd->cmdlist_pool_ion_handle);
	}

	hisifd->cmdlist_pool_ion_handle = NULL;
}

#endif

