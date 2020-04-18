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
#include "hisi_jpu.h"
#include "hisi_jpu_iommu.h"
#include "hisi_dss_ion.h"
#include "securec.h"

#define MAX_INPUT_DATA_LEN (8192*8192*4)

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,14,0)
int hisijpu_create_buffer_client(struct hisi_jpu_data_type *hisijd)
{
	return 0;
}

void hisijpu_destroy_buffer_client(struct hisi_jpu_data_type *hisijd)
{
	return;
}

int hisijpu_enable_iommu(struct hisi_jpu_data_type *hisijd)
{
	if (!hisijd) {
		HISI_JPU_ERR("hisijd is NULL!\n");
		return -EINVAL;
	}

	if (!hisijd->pdev) {
		HISI_JPU_ERR("pdev is NULL");
		return -EINVAL;
	}

	phys_addr_t ttbr = hisi_domain_get_ttbr(&(hisijd->pdev->dev));

	HISI_JPU_INFO("jpu get iommu (ttbr %pa) success.\n", &ttbr);

	dma_set_mask_and_coherent(&(hisijd->pdev->dev), DMA_BIT_MASK(64));

	return 0;
}

phys_addr_t hisi_jpu_domain_get_ttbr(struct hisi_jpu_data_type *hisijd)
{
	if (!hisijd) {
		HISI_JPU_ERR("hisijd is NULL!\n");
		return 0;
	}

	if (!hisijd->pdev) {
		HISI_JPU_ERR("pdev is NULL");
		return 0;
	}

	return hisi_domain_get_ttbr(&(hisijd->pdev->dev));
}

/*
 * this function allocate physical memory,
 * and make them to scatter lista.
 * table is global .
 */
static struct jpu_iommu_page_info *__hisi_jpu_dma_create_node(void)
{
	struct jpu_iommu_page_info *info = NULL;
	struct page *page = NULL ;
	info = kzalloc(sizeof(struct jpu_iommu_page_info), GFP_KERNEL);
	if (!info) {
		HISI_JPU_INFO("kzalloc info failed! \n");
		return NULL;
	}
	/* alloc 8kb each time */
	page = alloc_pages(GFP_KERNEL, 1);
	if (!page) {
		HISI_JPU_INFO("alloc page error. \n");
		kfree(info);
		return NULL;
	}
	info->page = page;
	info->order = 0;
	INIT_LIST_HEAD(&info->list);
	return info;
}

static struct sg_table *__hisi_jpu_dma_alloc_memory(unsigned int size)
{
	int map_size = 0;
	unsigned int sum = 0;
	struct list_head pages;
	struct jpu_iommu_page_info *info, *tmp_info;
	unsigned int i = 0, ret = 0;
	struct sg_table *table = NULL;
	struct scatterlist *sg = NULL;

	INIT_LIST_HEAD(&pages);

	map_size  = size;
	if (map_size < 0) {
		return NULL;
	}

	HISI_JPU_INFO("map_size = 0x%x \n", map_size);
	do {
		info = __hisi_jpu_dma_create_node();
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

	HISI_JPU_INFO("get alloc sg_table success.\n");
	return table;

error:
	list_for_each_entry_safe(info, tmp_info, &pages, list) {
		__free_pages(info->page, info->order);
		list_del(&info->list);
		kfree(info);
	}
	return NULL;
}

static int __hisi_jpu_dma_free_memory(struct sg_table *table)
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
	HISI_JPU_INFO("free total memory 0x%x.\n", mem_size);
	table = NULL;
	return 0;
}

int hisi_jpu_lb_alloc(struct hisi_jpu_data_type *hisijd)
{
	size_t lb_size = 0;
	unsigned long buf_addr = 0;
	unsigned long buf_size = 0;
	struct sg_table *sg = NULL;
	struct fb_info *fbi = NULL;

	if (NULL == hisijd) {
		HISI_JPU_ERR("hisijd is NULL");
		return -EINVAL;
	}
	hisijd->lb_sg_table = NULL;

	lb_size = JPU_LB_SIZE; // align to PAGE_SIZE
	sg = __hisi_jpu_dma_alloc_memory(lb_size);
	if (!sg) {
		HISI_JPU_ERR("__hisi_jpu_dma_alloc_memory failed!\n");
		return -ENOMEM;
	}

	buf_addr = hisi_iommu_map_sg(&(hisijd->pdev->dev), sg->sgl, 0, &buf_size);
	if (!buf_addr) {
		HISI_JPU_ERR("hisi_iommu_map_sg failed!\n");
		__hisi_jpu_dma_free_memory(sg);
		return -ENOMEM;
	}
	HISI_JPU_INFO("jpu%d alloc lb map sg 0x%zxB succuss.\n",
		 hisijd->index, buf_size);

	hisijd->lb_addr = (uint32_t)(buf_addr >> 4);

	/* start address for line buffer, unit is 16 byte, must align to 128 byte */
	if (hisijd->lb_addr & (JPU_LB_ADDR_ALIGN - 1)) {
		HISI_JPU_ERR("lb_addr(0x%x) is not %d bytes aligned!\n",
			hisijd->lb_addr, JPU_LB_ADDR_ALIGN - 1);
		hisi_iommu_unmap_sg(&(hisijd->pdev->dev), sg->sgl, buf_addr);
		__hisi_jpu_dma_free_memory(sg);
		return -EINVAL;
	}
	hisijd->lb_sg_table = sg;

	HISI_JPU_INFO("lb_size = %zu, hisijd->lb_addr 0x%x\n", lb_size, hisijd->lb_addr);

	return 0;
}

void hisi_jpu_lb_free(struct hisi_jpu_data_type *hisijd)
{
	if (!hisijd) {
		HISI_JPU_ERR("hisijd is NULL!\n");
		return;
	}

	if (hisijd->lb_addr && hisijd->lb_sg_table) {
		hisi_iommu_unmap_sg(&(hisijd->pdev->dev), hisijd->lb_sg_table->sgl, hisijd->lb_addr);
		__hisi_jpu_dma_free_memory(hisijd->lb_sg_table);

		hisijd->lb_addr = 0;
		hisijd->lb_sg_table = NULL;
	}
}

static bool hisi_jpu_check_buffer_validate(struct hisi_jpu_data_type *hisijd, int fd)
{
	struct sg_table *table = NULL;
	struct dma_buf *buf = NULL;
	struct dma_buf_attachment *attach = NULL;

	// dim layer share fd -1
	if (fd < 0) {
		return false;
	}

	buf = dma_buf_get(fd);
	if (IS_ERR(buf)) {
		HISI_JPU_ERR("Invalid file fd(%d)", fd);
		return false;
	}
	dma_buf_put(buf);

	return true;
}

int hisi_jpu_check_inbuff_addr(struct hisi_jpu_data_type *hisijd, jpu_data_t *jpu_req)
{
	unsigned long buf_size = 0;
	uint64_t inbuffer_addr = 0;
	struct dma_buf *buf = NULL;

	if (!hisijd || !(hisijd->pdev)) {
		HISI_JPU_ERR("hisijd is NULL!\n");
		return -EINVAL;
	}

	if (!jpu_req) {
		HISI_JPU_ERR("jpu_req is NULL!\n");
		return -EINVAL;
	}

	if (!hisi_jpu_check_buffer_validate(hisijd, jpu_req->in_sharefd)) {
		HISI_JPU_ERR("Invalid file fd(%d)\n", jpu_req->in_sharefd);
		return -EINVAL;
	}

	buf = dma_buf_get(jpu_req->in_sharefd);
	if (IS_ERR(buf)) {
		HISI_JPU_ERR("Invalid file shared_fd(%d)", jpu_req->in_sharefd);
		return -EINVAL;
	}

	inbuffer_addr = hisi_iommu_map_dmabuf(&(hisijd->pdev->dev), buf, 0, &buf_size);
	if (!inbuffer_addr) {
		HISI_JPU_ERR("get iova_size(0x%x) failed. \n", buf_size);
		return -EFAULT;
	}
	HISI_JPU_DEBUG("get iova success iova(0x%x), iova_size(0x%x).\n",
		inbuffer_addr, buf_size);

	jpu_req->start_addr = jpu_req->start_addr + inbuffer_addr;
	jpu_req->end_addr = jpu_req->end_addr + inbuffer_addr;

	if (jpu_req->end_addr <= jpu_req->start_addr) {
		HISI_JPU_ERR("end_addr invalid!\n");
		hisi_iommu_unmap_dmabuf(&(hisijd->pdev->dev), buf, inbuffer_addr);
		dma_buf_put(buf);
		return -EINVAL;
	}

	if ((jpu_req->addr_offset > jpu_req->start_addr) ||
			(jpu_req->addr_offset > MAX_INPUT_DATA_LEN)) {
		HISI_JPU_ERR("addr offset invalid!\n");
		hisi_iommu_unmap_dmabuf(&(hisijd->pdev->dev), buf, inbuffer_addr);
		dma_buf_put(buf);
		return -EINVAL;
	}

	dma_buf_put(buf);
	return 0;
}

int hisi_jpu_check_outbuff_addr(struct hisi_jpu_data_type *hisijd, jpu_data_t *jpu_req)
{
	unsigned long buf_size = 0;
	uint64_t outbuffer_addr = 0;
	struct dma_buf *buf = NULL;

	if (!hisijd || !hisijd->pdev) {
		HISI_JPU_ERR("hisijd is NULL!\n");
		return -EINVAL;
	}

	if (!jpu_req) {
		HISI_JPU_ERR("jpu_req is NULL!\n");
		return -EINVAL;
	}

	if (!hisi_jpu_check_buffer_validate(hisijd, jpu_req->out_sharefd)) {
		HISI_JPU_ERR("Invalid file fd(%d)\n", jpu_req->out_sharefd);
		return -EINVAL;
	}

	buf = dma_buf_get(jpu_req->out_sharefd);
	if (IS_ERR(buf)) {
		HISI_JPU_ERR("Invalid file shared_fd(%d)", jpu_req->out_sharefd);
		return -EINVAL;
	}

	outbuffer_addr = hisi_iommu_map_dmabuf(&(hisijd->pdev->dev), buf, 0, &buf_size);
	if (!outbuffer_addr) {
		HISI_JPU_ERR("get iova_size(0x%x) failed. \n", buf_size);
		return -EFAULT;
	}
	HISI_JPU_INFO("get iova success iova(0x%x), iova_size(0x%x).\n",
		outbuffer_addr, buf_size);

	jpu_req->start_addr_y = outbuffer_addr/16;
	if ((jpu_req->out_color_format >= HISI_JPEG_DECODE_OUT_RGBA4444)
		&& (HISI_JPEG_DECODE_OUT_BGRA8888 >= jpu_req->out_color_format)) {
		/*
		  jpu_req->stride_y = (uint32_t)out_info->byte_stride/16;
		  jpu_req->last_page_y = (jpu_req->start_addr_y + (uint32_t)out_info->size/16)/2048;
		 */
		jpu_req->last_page_y = jpu_req->last_page_y + jpu_req->start_addr_y/2048;
	} else {
		if (HISI_JPEG_DECODE_OUT_YUV400 == jpu_req->out_color_format) {
			/*
			  jpu_req->stride_y = (uint32_t)out_info->byte_stride/16;
			  jpu_req->last_page_y = (jpu_req->start_addr_y + (uint32_t)out_info->size/16) /2048;
			  jpu_req->stride_c = 0;
			  jpu_req->start_addr_c = 0;
			  jpu_req->last_page_c = 0;
			 */
			jpu_req->last_page_y = jpu_req->last_page_y + jpu_req->start_addr_y/2048;
		}

		if ((HISI_JPEG_DECODE_OUT_YUV420 == jpu_req->out_color_format) ||
				(HISI_JPEG_DECODE_OUT_YUV422_H2V1 == jpu_req->out_color_format) ||
				(HISI_JPEG_DECODE_OUT_YUV444 == jpu_req->out_color_format)) {
			/*
			  jpu_req->stride_y = (uint32_t)out_info->byte_stride/32;
			  jpu_req->stride_c = jpu_req->stride_y;
			  jpu_req->last_page_y = (jpu_req->start_addr_y + (uint32_t)out_info->size/32)/2048;
			  jpu_req->start_addr_c = jpu_req->start_addr_y + (uint32_t)out_info->size/32;
			  jpu_req->last_page_c = (jpu_req->start_addr_c + (uint32_t)out_info->size/32)/2048;
			 */
			jpu_req->last_page_y = jpu_req->last_page_y + jpu_req->start_addr_y/2048;
			jpu_req->start_addr_c = jpu_req->start_addr_c + jpu_req->start_addr_y;
			jpu_req->last_page_c = jpu_req->last_page_c + jpu_req->start_addr_y/2048;

			HISI_JPU_INFO("outbuffer: stride_y 0x%x, stride_c 0x%x \n",
				jpu_req->stride_y, jpu_req->stride_c);
		}

		if (HISI_JPEG_DECODE_OUT_YUV422_H1V2 == jpu_req->out_color_format) {
			/*
			  jpu_req->stride_y = (uint32_t)out_info->byte_stride/32;
			  jpu_req->stride_c = (uint32_t)out_info->byte_stride/16;
			  jpu_req->last_page_y = (jpu_req->start_addr_y + (uint32_t)out_info->size/32)/2048;
			  jpu_req->start_addr_c = jpu_req->start_addr_y + (uint32_t)out_info->size/32;
			  jpu_req->last_page_c = (jpu_req->start_addr_c + (uint32_t)out_info->size/32)/2048;
			 */
			jpu_req->last_page_y = jpu_req->last_page_y + jpu_req->start_addr_y/2048;
			jpu_req->start_addr_c = jpu_req->start_addr_c + jpu_req->start_addr_y;
			jpu_req->last_page_c = jpu_req->last_page_c + jpu_req->start_addr_y/2048;

			HISI_JPU_INFO("outbuffer: stride_y 0x%x, stride_c 0x%x \n",
				jpu_req->stride_y, jpu_req->stride_c);
		}
	}

	//start_addr unit is 16 byte, page unit is 32KB, so start_addr need to devide 2048
	if ((jpu_req->last_page_y < (jpu_req->start_addr_y / 2048)) ||
			(jpu_req->last_page_c < (jpu_req->start_addr_c / 2048))) {
		HISI_JPU_ERR("last_page_y invalid!\n");
		hisi_iommu_unmap_dmabuf(&(hisijd->pdev->dev), buf, outbuffer_addr);
		dma_buf_put(buf);
		return -EINVAL;
	}

	dma_buf_put(buf);
	return 0;
}

#else

#define HISI_JPU_ION_CLIENT_NAME	"hisi_jpu_ion"
int hisijpu_create_buffer_client(struct hisi_jpu_data_type *hisijd)
{
	hisijd->lb_ion_handle = NULL;
	hisijd->ion_client = hisi_ion_client_create(HISI_JPU_ION_CLIENT_NAME);

	if (IS_ERR_OR_NULL(hisijd->ion_client)) {
		dev_err(&hisijd->pdev->dev, "failed to create ion client!\n");
		return -ENOMEM;
	}
	return 0;
}

void hisijpu_destroy_buffer_client(struct hisi_jpu_data_type *hisijd)
{
	if (hisijd->ion_client) {
		ion_client_destroy(hisijd->ion_client);
		hisijd->ion_client = NULL;
	}
}

int hisijpu_enable_iommu(struct hisi_jpu_data_type *hisijd)
{
	struct iommu_domain *domain = NULL;
	struct iommu_domain_data *domain_data = NULL;

	if (!hisijd) {
		HISI_JPU_ERR("hisijd is NULL!\n");
		return -EINVAL;
	}

	/* create iommu domain */
	domain = hisi_ion_enable_iommu(NULL);
	if (!domain) {
		HISI_JPU_ERR("failed to hisi_ion_enable_iommu!\n");
		return -EINVAL;
	}
	domain_data = (struct iommu_domain_data *)(domain->priv);
	HISI_JPU_INFO("jpu get iommu (ttbr %pa) success.\n", &domain_data->phy_pgd_base);

	hisijd->jpu_domain = domain;

	return 0;
}

phys_addr_t hisi_jpu_domain_get_ttbr(struct hisi_jpu_data_type *hisijd)
{
	struct iommu_domain_data *domain_data = NULL;

	if (!hisijd) {
		HISI_JPU_ERR("hisijd is NULL!\n");
		return 0;
	}

	if (!hisijd->jpu_domain) {
		HISI_JPU_ERR("jpu_domain is null.\n");
		return 0;
	}

	domain_data = (struct iommu_domain_data *)(hisijd->jpu_domain->priv);
	if (!domain_data) {
		HISI_JPU_ERR("domain_data is null.\n");
		return 0;
	}

	return domain_data->phy_pgd_base;
}

int hisi_jpu_lb_alloc(struct hisi_jpu_data_type *hisijd)
{
	int ret = 0;
	size_t lb_size = 0;

	if (!hisijd || !hisijd->pdev) {
		HISI_JPU_ERR("hisijd is NULL!\n");
		return -EINVAL;
	}

	if (!hisijd->ion_client) {
		HISI_JPU_ERR("ion_client is NULL!\n");
		return -EINVAL;
	}

	lb_size = JPU_LB_SIZE;

	/*alloc jpu dec lb buffer, start end shoulf 32KB align*/
	hisijd->lb_ion_handle = ion_alloc(hisijd->ion_client, lb_size,
		0, ION_HEAP(ION_SYSTEM_HEAP_ID), 0);
	if (IS_ERR_OR_NULL(hisijd->lb_ion_handle)) {
		HISI_JPU_ERR("failed to ion alloc lb_ion_handle!");
		ret = -ENOMEM;
		goto err_ion_handle;
	}

	if (!ion_map_kernel(hisijd->ion_client, hisijd->lb_ion_handle)) {
		HISI_JPU_ERR("failed to ion_map_kernel!");
		ret = -ENOMEM;
		goto err_map_kernel;
	}

	if (ion_map_iommu(hisijd->ion_client, hisijd->lb_ion_handle, &(hisijd->iommu_format))) {
		ret = -ENOMEM;
		HISI_JPU_ERR("failed to ion_map_iommu!");
		goto err_map_iommu;
	}
	hisijd->lb_addr = (uint32_t)(hisijd->iommu_format.iova_start >> 4);

	/* start address for line buffer, unit is 16 byte, must align to 128 byte */
	if (hisijd->lb_addr & (JPU_LB_ADDR_ALIGN - 1)) {
		HISI_JPU_ERR("lb_addr(0x%x) is not %d bytes aligned!\n",
			hisijd->lb_addr, JPU_LB_ADDR_ALIGN - 1);
		ret = -EINVAL;
		goto err_addr;
	}

	HISI_JPU_INFO("lb_size=%zu, hisijd->lb_addr 0x%x\n", lb_size, hisijd->lb_addr);
	return 0;

err_addr:
	ion_unmap_iommu(hisijd->ion_client, hisijd->lb_ion_handle);

err_map_iommu:
	if (hisijd->lb_ion_handle) {
		ion_unmap_kernel(hisijd->ion_client, hisijd->lb_ion_handle);
	}

err_map_kernel:
	if (hisijd->lb_ion_handle) {
		ion_free(hisijd->ion_client, hisijd->lb_ion_handle);
		hisijd->lb_ion_handle = NULL;
	}

err_ion_handle:
	return ret;
}

void hisi_jpu_lb_free(struct hisi_jpu_data_type *hisijd)
{
	if (!hisijd) {
		HISI_JPU_ERR("hisijd is NULL!\n");
		return;
	}

	if (hisijd->ion_client && hisijd->lb_ion_handle) {
		ion_unmap_iommu(hisijd->ion_client, hisijd->lb_ion_handle);
		ion_unmap_kernel(hisijd->ion_client, hisijd->lb_ion_handle);
		ion_free(hisijd->ion_client, hisijd->lb_ion_handle);
		hisijd->lb_ion_handle = NULL;
	}
}

int hisi_jpu_check_inbuff_addr(struct hisi_jpu_data_type *hisijd, jpu_data_t *jpu_req)
{
	struct ion_handle *inhnd = NULL;
	struct iommu_map_format iommu_in_format;
	size_t buf_len = 0;
	unsigned long buf_addr = 0;
	bool succ = true;

	if (!hisijd || !hisijd->pdev) {
		HISI_JPU_ERR("hisijd is NULL!\n");
		return -EINVAL;
	}

	if (!hisijd->ion_client) {
		HISI_JPU_ERR("ion_client is NULL!\n");
		return -EINVAL;
	}

	if (!jpu_req) {
		HISI_JPU_ERR("jpu_req is NULL!\n");
		return -EINVAL;
	}

	if (jpu_req->in_sharefd < 0) {
		HISI_JPU_ERR("in_sharefd=%d invalid!\n", jpu_req->in_sharefd);
		return -EINVAL;
	}

	if (jpu_req->end_addr <= jpu_req->start_addr) {
		HISI_JPU_ERR("end_addr invalid!\n");
		return -EINVAL;
	}

	if (jpu_req->addr_offset > jpu_req->start_addr || jpu_req->addr_offset > MAX_INPUT_DATA_LEN) {
		HISI_JPU_ERR("addr offset invalid!\n");
		return -EINVAL;
	}

	(void)memset_s(&iommu_in_format, sizeof(struct iommu_map_format), 0, sizeof(struct iommu_map_format));

	//check input buffer addr
	inhnd = ion_import_dma_buf_fd(hisijd->ion_client, jpu_req->in_sharefd);
	if (IS_ERR(inhnd)) {
		HISI_JPU_ERR("inhnd ion_import_dma_buf fail ! \n");
		inhnd = NULL;
		return -EINVAL;
	}

	if (jpu_req->smmu_enable == 1) {
		if (ion_map_iommu(hisijd->ion_client, inhnd, &iommu_in_format)) {
			HISI_JPU_ERR("inhnd ion_map_iommu fail ! \n");
			succ = false;
		} else {
			if ((jpu_req->start_addr - jpu_req->addr_offset) != iommu_in_format.iova_start) {
				HISI_JPU_ERR("inbuffer ion_map_iommu check fail ! \n");
				succ = false;
			}
			ion_unmap_iommu(hisijd->ion_client, inhnd);
		}
	} else {
		if (hisifb_ion_phys(hisijd->ion_client, inhnd, &(hisijd->pdev->dev), &buf_addr, &buf_len)) {
			HISI_JPU_ERR("inhnd ion_phys fail ! \n");
			succ = false;
		} else {
			if (buf_addr != (jpu_req->start_addr - jpu_req->addr_offset)) {
				HISI_JPU_ERR("inbuffer ion_phys check fail ! \n");
				succ = false;
			}
		}
	}
	ion_free(hisijd->ion_client, inhnd);

	return succ ? 0 : -EINVAL;
}

int hisi_jpu_check_outbuff_addr(struct hisi_jpu_data_type *hisijd, jpu_data_t *jpu_req)
{
	struct ion_handle *outhnd = NULL;
	struct iommu_map_format iommu_out_format;
	size_t buf_len = 0;
	unsigned long buf_addr = 0;
	bool succ = true;

	if (!hisijd || !hisijd->pdev) {
		HISI_JPU_ERR("hisijd is NULL!\n");
		return -EINVAL;
	}

	if (!hisijd->ion_client) {
		HISI_JPU_ERR("ion_client is NULL!\n");
		return -EINVAL;
	}

	if (!jpu_req) {
		HISI_JPU_ERR("jpu_req is NULL!\n");
		return -EINVAL;
	}

	if (jpu_req->out_sharefd < 0) {
		HISI_JPU_ERR("out_sharefd=%d invalid!\n", jpu_req->out_sharefd);
		return -EINVAL;
	}

	//start_addr unit is 16 byte, page unit is 32KB, so start_addr need to devide 2048
	if ((jpu_req->last_page_y < (jpu_req->start_addr_y / 2048)) ||
			(jpu_req->last_page_c < (jpu_req->start_addr_c / 2048))) {
		HISI_JPU_ERR("last_page_y invalid!\n");
		return -EINVAL;
	}

	/*uint32_t restart_interval;*/
	(void)memset_s(&iommu_out_format, sizeof(struct iommu_map_format), 0, sizeof(struct iommu_map_format));

	//check output buffer addr
	outhnd = ion_import_dma_buf_fd(hisijd->ion_client, jpu_req->out_sharefd);
	if (IS_ERR(outhnd)) {
		HISI_JPU_ERR("outhnd ion_import_dma_buf fail ! \n");
		outhnd = NULL;
		return -EINVAL;
	}

	if (jpu_req->smmu_enable == 1) {
		if (ion_map_iommu(hisijd->ion_client, outhnd, &iommu_out_format)) {
			HISI_JPU_ERR("outhnd ion_map_iommu fail ! \n");
			succ = false;
		} else {
			if (jpu_req->start_addr_y != (iommu_out_format.iova_start / 16)) {
				HISI_JPU_ERR("outbuffer ion_map_iommu check fail !\n");
				succ = false;
			}
			ion_unmap_iommu(hisijd->ion_client, outhnd);
		}
	} else {
		if (hisifb_ion_phys(hisijd->ion_client, outhnd, &(hisijd->pdev->dev), &buf_addr, &buf_len)) {
			HISI_JPU_ERR("outhnd ion_phys fail ! \n");
			succ = false;
		} else {
			if ((buf_addr / 16) != jpu_req->start_addr_y) {
				HISI_JPU_ERR("outbuffer ion_phys check fail ! \n");
				succ = false;
			}
		}
	}
	ion_free(hisijd->ion_client, outhnd);

	return succ ? 0 : -EINVAL;
}

#endif
