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
#include "hisi_dss_ion.h"
#include "hisi_fb.h"

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,14,0)

/*
 * this function allocate physical memory,
 * and make them to scatter lista.
 * table is global .
 */
 /*lint -e574 -e737*/
static struct iommu_page_info *__hisifb_dma_create_node(void)
{
	struct iommu_page_info *info = NULL;
	struct page *page = NULL ;
	info = kzalloc(sizeof(struct iommu_page_info), GFP_KERNEL);
	if (!info) {
		HISI_FB_INFO("kzalloc info failed! \n");
		return NULL;
	}
	/* alloc 8kb each time */
	page = alloc_pages(GFP_KERNEL, 1);
	if (!page) {
		HISI_FB_INFO("alloc page error. \n");
		kfree(info);
		return NULL;
	}
	info->page = page;
	info->order = 0;
	INIT_LIST_HEAD(&info->list);
	return info;
}

static struct sg_table *__hisifb_dma_alloc_memory(unsigned int size)
{
	int map_size = 0;
	unsigned int sum = 0;
	struct list_head pages;
	struct iommu_page_info *info, *tmp_info;
	unsigned int i = 0, ret = 0;
	struct sg_table *table = NULL;
	struct scatterlist *sg = NULL;

	INIT_LIST_HEAD(&pages);

	map_size  = size;
	if (map_size < 0) {
		return NULL;
	}

	HISI_FB_INFO("map_size = 0x%x \n", map_size);
	do {
		info = __hisifb_dma_create_node();
		if (!info)
			goto error;
		list_add_tail(&info->list, &pages);
		sum += (1 << info->order) * PAGE_SIZE;
		i++;
	} while (sum < map_size);

	table = kzalloc(sizeof(struct sg_table), GFP_KERNEL);
	if (!table) {
		goto error;
	}

	ret = sg_alloc_table(table, i, GFP_KERNEL);
	if (ret) {
		kfree(table);
		goto error;
	}

	sg = table->sgl;
	list_for_each_entry_safe(info, tmp_info, &pages, list) {
		struct page *page = info->page;
		sg_set_page(sg, page, (1 << info->order) * PAGE_SIZE, 0);
		sg = sg_next(sg);
		list_del(&info->list);
		kfree(info);
	}

	HISI_FB_INFO("get alloc sg_table success.\n");
	return table;

error:
	list_for_each_entry_safe(info, tmp_info, &pages, list) {
		__free_pages(info->page, info->order);
		list_del(&info->list);
		kfree(info);
	}
	return NULL;
}

static int __hisifb_dma_free_memory(struct sg_table *table)
{
	int i;
	struct scatterlist *sg = NULL;
	unsigned int mem_size = 0;
	if (table) {
		for_each_sg(table->sgl, sg, table->nents, i) {
			__free_pages(sg_page(sg), get_order(sg->length));
			mem_size += sg->length;
		}
		sg_free_table(table);
		kfree(table);
	}
	HISI_FB_INFO("free total memory 0x%x.\n", mem_size);
	table = NULL;
	return 0;
}

unsigned long hisifb_alloc_fb_buffer(struct hisi_fb_data_type *hisifd)
{
	size_t buf_len = 0;
	unsigned long buf_addr = 0;
	unsigned long buf_size = 0;
	struct sg_table *sg = NULL;
	struct fb_info *fbi = NULL;


	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL.\n");
		return -EINVAL;
	}

	fbi = hisifd->fbi;
	if (NULL == fbi) {
		HISI_FB_ERR("fbi is NULL.\n");
		return -EINVAL;
	}

	buf_len = fbi->fix.smem_len; // align to PAGE_SIZE
	sg = __hisifb_dma_alloc_memory(buf_len);
	if (!sg) {
		HISI_FB_ERR("__hdss_dma_alloc_memory failed!\n");
		return -ENOMEM;
	}

	buf_addr = hisi_iommu_map_sg(__hdss_get_dev(), sg->sgl, 0, &buf_size);
	if (!buf_addr) {
		HISI_FB_ERR("hisi_iommu_map_sg failed!\n");
		__hisifb_dma_free_memory(sg);
		return -ENOMEM;
	}
	HISI_FB_INFO("fb%d alloc framebuffer map sg 0x%zxB succuss.\n",
		 hisifd->index, buf_size);

	fbi->screen_base = hisifb_iommu_map_kernel(sg, buf_len);
	if (!fbi->screen_base) {
		HISI_FB_ERR("hisifb_iommu_map_kernel failed!\n");
		hisi_iommu_unmap_sg(__hdss_get_dev(), sg->sgl, buf_addr);
		__hisifb_dma_free_memory(sg);
		return -ENOMEM;
	}

	fbi->fix.smem_start = buf_addr;
	fbi->screen_size = buf_len;
	hisifd->fb_sg_table = sg;

	HISI_FB_INFO("fb%d alloc framebuffer 0x%zxB @ (0x%pK virt) (%pa iova).\n",
		 hisifd->index, fbi->screen_size, fbi->screen_base, &fbi->fix.smem_start);

	return buf_addr;
}

void hisifb_free_fb_buffer(struct hisi_fb_data_type *hisifd)
{
	struct fb_info *fbi = NULL;

	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL.\n");
		return;
	}
	fbi = hisifd->fbi;
	if (NULL == fbi) {
		HISI_FB_ERR("fbi is NULL.\n");
		return;
	}

	if ((hisifd->fb_sg_table) && (fbi->screen_base != 0)) {
		HISI_FB_INFO("fb%d free frame buffer 0x%zxB @ (0x%pK virt) (%pa dmaAddr).\n",
			hisifd->index, fbi->screen_size, fbi->screen_base, &fbi->fix.smem_start);
		hisifb_iommu_unmap_kernel(fbi->screen_base);
		hisi_iommu_unmap_sg(__hdss_get_dev(), hisifd->fb_sg_table->sgl, fbi->fix.smem_start);
		__hisifb_dma_free_memory(hisifd->fb_sg_table);

		hisifd->fb_sg_table = NULL;
		fbi->screen_base = 0;
		fbi->fix.smem_start = 0;
	}
}

void *hisifb_iommu_map_kernel(struct sg_table *sg_table, size_t size)
{
	int i, j;
	void *vaddr;
	pgprot_t pgprot;
	struct scatterlist *sg;
	struct sg_table *table = sg_table;
	int npages = PAGE_ALIGN(size) / PAGE_SIZE;
	struct page **pages = vmalloc(sizeof(struct page *) * npages);
	struct page **tmp = pages;

	if (!pages) {
		return ERR_PTR(-ENOMEM);
	}
	pgprot = pgprot_writecombine(PAGE_KERNEL);

	for_each_sg(table->sgl, sg, table->nents, i) {
		int npages_this_entry = PAGE_ALIGN(sg->length) / PAGE_SIZE;
		struct page *page = sg_page(sg);

		BUG_ON(i >= npages);
		for (j = 0; j < npages_this_entry; j++) {
			*(tmp++) = page++;
		}
	}

	vaddr = vmap(pages, npages, VM_MAP, pgprot);
	vfree(pages);

	if (!vaddr) {
		return ERR_PTR(-ENOMEM);
	}

	return vaddr;
}

void hisifb_iommu_unmap_kernel(void *vaddr)
{
	vunmap(vaddr);
}

#else

static struct sg_table *hisifb_get_fb_sg_table(struct device *dev,
	struct ion_client *client, struct ion_handle *handle)
{
	struct sg_table *table = NULL;
	struct dma_buf *buf = NULL;
	struct dma_buf_attachment *attach = NULL;
	int shared_fd = 0;
	shared_fd = ion_share_dma_buf_fd(client, handle);
	if (shared_fd < 0) {
		HISI_FB_ERR("Failed to share ion buffer(0x%pK)!", handle);
		return NULL;
	}

	buf = dma_buf_get(shared_fd);
	if (IS_ERR(buf)) {
		HISI_FB_ERR("Invalid file handle(%d).\n", shared_fd);
		sys_close(shared_fd);
		return NULL;
	}
	attach = dma_buf_attach(buf, dev);
	if (IS_ERR(attach)) {
		dma_buf_put(buf);
		sys_close(shared_fd);
		return NULL;
	}
	table = dma_buf_map_attachment(attach, DMA_BIDIRECTIONAL);
	if (IS_ERR(table)) {
		dma_buf_detach(buf, attach);
		dma_buf_put(buf);
		sys_close(shared_fd);
		return NULL;
    }
	dma_buf_unmap_attachment(attach, table, DMA_BIDIRECTIONAL);
	dma_buf_detach(buf, attach);
	dma_buf_put(buf);
	sys_close(shared_fd);
	return table;
}

unsigned long hisifb_alloc_fb_buffer(struct hisi_fb_data_type *hisifd)
{
	struct fb_info *fbi = NULL;
	struct ion_client *client = NULL;
	struct ion_handle *handle = NULL;
	size_t buf_len = 0;
	unsigned long buf_addr = 0;
	struct iommu_map_format iommu_format;

	if (NULL == hisifd || !(hisifd->pdev)) {
		HISI_FB_ERR("hisifd is NULL.\n");
		goto err_return;
	}

	fbi = hisifd->fbi;
	if (NULL == fbi) {
		HISI_FB_ERR("fbi is NULL.\n");
		goto err_return;
	}

	if (hisifd->buffer_handle != NULL) {
		return fbi->fix.smem_start;
	}

	client = hisifd->buffer_client;
	if (NULL == client) {
		HISI_FB_ERR("failed to create ion client!\n");
		goto err_return;
	}

	buf_len = fbi->fix.smem_len;

	handle = ion_alloc(client, buf_len, PAGE_SIZE, ION_HEAP(ION_SYSTEM_HEAP_ID), 0);
	if (IS_ERR_OR_NULL(handle)) {
		HISI_FB_ERR("failed to ion_alloc!\n");
		goto err_return;
	}

	fbi->screen_base = ion_map_kernel(client, handle);
	if (!fbi->screen_base) {
		HISI_FB_ERR("failed to ion_map_kernel!\n");
		goto err_ion_map;
	}

	memset(&iommu_format, 0, sizeof(struct iommu_map_format));
	if (ion_map_iommu(client, handle, &iommu_format)) {
		HISI_FB_ERR("failed to ion_map_iommu!\n");
		goto err_ion_get_addr;
	}

	buf_addr = iommu_format.iova_start;
	fbi->fix.smem_start = buf_addr;
	fbi->screen_size = fbi->fix.smem_len;
	//memset(fbi->screen_base, 0xFF, fbi->screen_size);

	hisifd->buffer_handle = handle;
	hisifd->fb_sg_table = hisifb_get_fb_sg_table(&hisifd->pdev->dev, client, handle);

	return buf_addr;

err_ion_get_addr:
	ion_unmap_kernel(hisifd->buffer_client, handle);
err_ion_map:
	ion_free(hisifd->buffer_client, handle);
err_return:
	return 0;
}

void hisifb_free_fb_buffer(struct hisi_fb_data_type *hisifd)
{
	struct fb_info *fbi = NULL;

	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL.\n");
		return;
	}
	fbi = hisifd->fbi;
	if (NULL == fbi) {
		HISI_FB_ERR("fbi is NULL.\n");
		return;
	}

	if (hisifd->buffer_client != NULL &&
		hisifd->buffer_handle != NULL) {
		ion_unmap_iommu(hisifd->buffer_client, hisifd->buffer_handle);
		ion_unmap_kernel(hisifd->buffer_client, hisifd->buffer_handle);
		ion_free(hisifd->buffer_client, hisifd->buffer_handle);
		hisifd->buffer_handle = NULL;

		fbi->screen_base = 0;
		fbi->fix.smem_start = 0;
	}
}

int hisifb_get_ion_phys(struct fb_info *info, void __user *arg)
{
	struct ion_phys_data data;
	phys_addr_t phys_addr = 0;
	size_t size = 0;
	int ret = 0;
	struct hisi_fb_data_type *hisifd = NULL;
	struct ion_handle *handle;

	hisifd = (struct hisi_fb_data_type *)info->par;
	if (NULL == hisifd || !(hisifd->pdev)) {
		HISI_FB_ERR("hisifd NULL Pointer.\n");
		return -EFAULT;
	}

	if (copy_from_user(&data, (void __user *)arg,sizeof(data))) {
		return -EFAULT;
	}

	if (NULL == hisifd->buffer_client) {
		HISI_FB_ERR("Failed to get the buffer_client.\n");
		return -EFAULT;
	}

	handle = ion_import_dma_buf_fd(hisifd->buffer_client, data.fd_buffer);
	if (IS_ERR(handle))
		return -EFAULT;

	ret = hisifb_ion_phys(hisifd->buffer_client, handle, &(hisifd->pdev->dev), (unsigned long *)&phys_addr, &size);
	if (ret) {
		ion_free(hisifd->buffer_client, handle);
		HISI_FB_ERR("hisifb_ion_phys:failed to get phys addr.\n");
		return -EFAULT;
	}

	data.size = size & 0xffffffff;
	data.phys_l = phys_addr & 0xffffffff;
	data.phys_h = (phys_addr >> 32) & 0xffffffff;

	if (copy_to_user((void __user *)arg, &data, sizeof(data))) {
		ion_free(hisifd->buffer_client, handle);
		return -EFAULT;
	}
	ion_free(hisifd->buffer_client, handle);
	return 0;
}

int hisifb_ion_phys(struct ion_client *client, struct ion_handle *handle,
	struct device *dev, unsigned long *addr, size_t *len)
{
	int shared_fd = -1;
	struct sg_table *table = NULL;
	struct dma_buf *buf = NULL;
	struct dma_buf_attachment *attach = NULL;

	if (NULL == client || NULL == handle) {
		HISI_FB_ERR("hisifb_ion_phys NULL Pointer.\n");
		return -EFAULT;
	}

	shared_fd = ion_share_dma_buf_fd(client, handle);
	if (shared_fd < 0) {
		HISI_FB_ERR("Failed to share ion buffer(0x%pK)!", handle);
		return -EFAULT;
	}

	buf = dma_buf_get(shared_fd);
	if (IS_ERR(buf)) {
		HISI_FB_ERR("Invalid file handle(%d).\n", shared_fd);
		sys_close(shared_fd);
		return -EFAULT;
	}

	attach = dma_buf_attach(buf, dev);
	if (IS_ERR(attach)) {
		dma_buf_put(buf);
		sys_close(shared_fd);
		return -EFAULT;
	}

	table = dma_buf_map_attachment(attach, DMA_BIDIRECTIONAL);
	if (IS_ERR(table)) {
		dma_buf_detach(buf, attach);
		dma_buf_put(buf);
		sys_close(shared_fd);
		return -EFAULT;
    }

	*addr = sg_phys(table->sgl); /*[false alarm]*/

	dma_buf_unmap_attachment(attach, table, DMA_BIDIRECTIONAL);
	dma_buf_detach(buf, attach);
	dma_buf_put(buf);
	sys_close(shared_fd);
	return 0;
}
#endif

#define HISI_FB_ION_CLIENT_NAME   "hisi_fb_ion"
int hisifb_create_buffer_client(struct hisi_fb_data_type *hisifd)
{
#if CONFIG_ION_ALLOC_BUFFER
	hisifd->buffer_handle = NULL;
	hisifd->buffer_client = hisi_ion_client_create(HISI_FB_ION_CLIENT_NAME);
	if (IS_ERR_OR_NULL(hisifd->buffer_client)) {
		HISI_FB_ERR("failed to create ion client!\n");
		return -ENOMEM;
	}
#endif

	return 0;
}

void hisifb_destroy_buffer_client(struct hisi_fb_data_type *hisifd)
{
#if CONFIG_ION_ALLOC_BUFFER
	if (hisifd->buffer_client) {
		ion_client_destroy(hisifd->buffer_client);
		hisifd->buffer_client = NULL;
	}
#endif
}

int hisi_fb_mmap(struct fb_info *info, struct vm_area_struct * vma)
{
	struct hisi_fb_data_type *hisifd = NULL;
	struct sg_table *table = NULL;
	struct scatterlist *sg = NULL;
	struct page *page = NULL;
	unsigned long remainder = 0;
	unsigned long len = 0;
	unsigned long addr = 0;
	unsigned long offset = 0;
	unsigned long size = 0;
	int i = 0;
	int ret = 0;

	if (NULL == info) {
		HISI_FB_ERR("NULL Pointer.\n");
		return -EINVAL;
	}

	hisifd = (struct hisi_fb_data_type *)info->par;
	if (NULL == hisifd || !(hisifd->pdev)) {
		HISI_FB_ERR("NULL Pointer.\n");
		return -EINVAL;
	}

	if (hisifd->index == PRIMARY_PANEL_IDX) {
		if (hisifd->fb_mem_free_flag) {
			if (!hisifb_alloc_fb_buffer(hisifd)) {
				HISI_FB_ERR("fb%d, hisifb_alloc_buffer failed!\n", hisifd->index);
				return -ENOMEM;
			}
		}
	} else {
		HISI_FB_ERR("fb%d, no fb buffer!\n", hisifd->index);
		return -EFAULT;
	}

	table = hisifd->fb_sg_table;
	if ((table == NULL) || (vma == NULL)) {
		HISI_FB_ERR("fb%d, table or vma is NULL!\n", hisifd->index);
		return -EFAULT;
	}

	vma->vm_page_prot = pgprot_writecombine(vma->vm_page_prot);

	addr = vma->vm_start;
	offset = vma->vm_pgoff * PAGE_SIZE;
	size = vma->vm_end - vma->vm_start;

	if (size > info->fix.smem_len) {
		HISI_FB_ERR("fb%d, size=%lu is out of range(%u)!\n",
			hisifd->index, size, info->fix.smem_len);
		return -EFAULT;
	}

	for_each_sg(table->sgl, sg, table->nents, i) {
		page = sg_page(sg);
		remainder = vma->vm_end - addr;
		len = sg->length;

		if (offset >= sg->length) {
			offset -= sg->length;
			continue;
		} else if (offset) {
			page += offset / PAGE_SIZE;
			len = sg->length - offset;
			offset = 0;
		}
		len = min(len, remainder);
		ret = remap_pfn_range(vma, addr, page_to_pfn(page), len,
			vma->vm_page_prot);
		if (ret != 0) {
			HISI_FB_ERR("fb%d, failed to remap_pfn_range! ret=%d\n",
				hisifd->index, ret);
		}

		addr += len;
		if (addr >= vma->vm_end) {
			return 0;
        }
	}
	return 0;
}

void hisifb_free_logo_buffer(struct hisi_fb_data_type *hisifd)
{
	int i;
	struct fb_info *fbi = NULL;
	uint32_t logo_buffer_base_temp = 0;

	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL.\n");
		return;
	}
	fbi = hisifd->fbi;//lint !e838
	if (NULL == fbi) {
		HISI_FB_ERR("fbi is NULL.\n");
		return;
	}

	logo_buffer_base_temp = g_logo_buffer_base;
	for (i = 0; i < (g_logo_buffer_size / PAGE_SIZE); i++) {
		free_reserved_page(phys_to_page(logo_buffer_base_temp));
		logo_buffer_base_temp += PAGE_SIZE;
	}
	memblock_free(g_logo_buffer_base, g_logo_buffer_size);

	g_logo_buffer_size = 0;
	g_logo_buffer_base = 0;
}
/*lint +e574 +e737*/
