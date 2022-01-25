/*
 * Hisilicon Kirin camera driver source file
 *
 * Copyright (C) Huawei Technology Co., Ltd.
 *
 * Author:	wenjianyue
 * Email:	wenjianyue@huawei.com
 * Date:	2018-11-28
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <hicam_buf.h>
#include <cam_log.h>

#include <linux/hisi-iommu.h>
#include <linux/device.h>
#include <linux/rbtree.h>
#include <linux/ion.h>
#include <linux/dma-buf.h>
#include <linux/kernel.h>
#include <linux/hisi/hisi_ion.h>
#include <linux/list.h>

struct sgtable_node {
	struct kref ref;
	struct list_head list;
	struct priv_ion_t *ion;
	struct sg_table *sgt;
	struct dma_buf *buf;
	struct dma_buf_attachment *attachment;
};

struct priv_ion_t {
	struct device *dev;
	struct ion_client *ion_client;

	struct mutex sg_mutex;
	struct list_head sg_nodes;
};

int hicam_internal_sync(int fd, struct sync_format *fmt)
{
	cam_err("%s: not supported, please use /dev/ion ioctl.", __func__);
	return -EFAULT;
}

int hicam_internal_local_sync(int fd, struct local_sync_format *fmt)
{
	cam_err("%s: not supported, please use /dev/ion ioctl.", __func__);
	return -EFAULT;
}

void hicam_internal_dump_debug_info(struct device *dev)
{
	struct sgtable_node *node;
	struct priv_ion_t *ion = dev_get_drvdata(dev);
	cam_info("%s: dumping.....%pK", __func__, ion);
	list_for_each_entry(node, &ion->sg_nodes, list) {
		cam_info("%s: pending sg_table:%pK for dmabuf:%pK",
				__func__, node->sgt, node->buf);
	}
	cam_info("%s: end", __func__);
}

void* hicam_internal_map_kernel(struct device *dev, int fd)
{
	void *kaddr = NULL;
	struct ion_handle *handle;
	struct priv_ion_t *ion = dev_get_drvdata(dev);

	handle = ion_import_dma_buf_fd(ion->ion_client, fd);
	if (unlikely(IS_ERR(handle))) {
		cam_err("%s: fail to import ion buffer.", __func__);
		return NULL;
	}

	kaddr = ion_map_kernel(ion->ion_client, handle);
	if (IS_ERR_OR_NULL(kaddr)) {
		cam_err("%s: fail to map iommu.", __func__);
	}

	ion_free(ion->ion_client, handle);
	return kaddr;
}

void hicam_internal_unmap_kernel(struct device *dev, int fd)
{
	struct ion_handle *handle;
	struct priv_ion_t *ion = dev_get_drvdata(dev);

	handle = ion_import_dma_buf_fd(ion->ion_client, fd);
	if (unlikely(IS_ERR(handle))) {
		cam_err("%s: fail to import ion buffer.", __func__);
		return ;
	}

	ion_unmap_kernel(ion->ion_client, handle);
	ion_free(ion->ion_client, handle);
}

int hicam_internal_map_iommu(struct device *dev,
		int fd, struct iommu_format *fmt)
{
	int rc = 0;
	struct ion_handle *handle;
	struct priv_ion_t *ion = dev_get_drvdata(dev);
	struct iommu_map_format map_format = {
		.prot = fmt->prot,
	};

	handle = ion_import_dma_buf_fd(ion->ion_client, fd);
	if (unlikely(IS_ERR(handle))) {
		cam_err("%s: fail to import ion buffer.", __func__);
		return -ENOENT;
	}

	if (ion_map_iommu(ion->ion_client, handle, &map_format)) {
		cam_err("%s: fail to map iommu.", __func__);
		rc = -ENOMEM;
		goto err_map_iommu;
	}
	cam_debug("%s: fd:%d, iova:%#lx, size:%#lx.", __func__, fd,
			map_format.iova_start, map_format.iova_size);
	fmt->iova = map_format.iova_start;
	fmt->size = map_format.iova_size;

err_map_iommu:
	ion_free(ion->ion_client, handle);
	return rc;
}

void hicam_internal_unmap_iommu(struct device *dev,
		int fd, struct iommu_format *fmt)
{
	struct ion_handle *handle;
	struct priv_ion_t *ion = dev_get_drvdata(dev);

	handle = ion_import_dma_buf_fd(ion->ion_client, fd);
	if (unlikely(IS_ERR(handle))) {
		cam_err("%s: fail to import ion buffer.", __func__);
		return ;
	}

	cam_debug("%s: fd:%d.", __func__, fd);
	ion_unmap_iommu(ion->ion_client, handle);
	ion_free(ion->ion_client, handle);
}

static struct sgtable_node* find_sgtable_node_by_fd(struct priv_ion_t *ion, int fd)
{
	struct dma_buf *dmabuf;
	struct sgtable_node *node;
	struct sgtable_node *ret_node = ERR_PTR(-ENOENT);

	dmabuf = dma_buf_get(fd);
	if (IS_ERR(dmabuf)) {
		cam_err("%s: fail to get dma buf.", __func__);
		return ret_node;
	}

	list_for_each_entry(node, &ion->sg_nodes, list) {
		if (node->buf == dmabuf) {
			ret_node = node;
			break;
		}
	}

	dma_buf_put(dmabuf);
	return ret_node;
}

static struct sgtable_node* find_sgtable_node_by_sg(struct priv_ion_t *ion,
		struct sg_table *sgt)
{
	struct sgtable_node *node;

	list_for_each_entry(node, &ion->sg_nodes, list) {
		if (node->sgt == sgt) {
			return node;
		}
	}

	return ERR_PTR(-ENOENT);
}

static struct sgtable_node* create_sgtable_node(struct priv_ion_t *ion, int fd)
{
	struct sgtable_node *node;

	node = kzalloc(sizeof(*node), GFP_KERNEL);
	if (!node) {
		cam_err("%s: fail to alloc sgtable_node.", __func__);
		return ERR_PTR(-ENOMEM);
	}

	node->buf = dma_buf_get(fd);
	if (IS_ERR(node->buf)) {
		cam_err("%s: fail to get dma buf.", __func__);
		goto err_get_buf;
	}

	node->attachment = dma_buf_attach(node->buf, ion->dev);
	if (IS_ERR(node->attachment)) {
		cam_err("%s: fail to attach dma buf.", __func__);
		goto err_attach_buf;
	}

	node->sgt = dma_buf_map_attachment(node->attachment, DMA_BIDIRECTIONAL);
	if (IS_ERR(node->sgt)) {
		cam_err("%s: fail to map attachment.", __func__);
		goto err_map_buf;
	}

	kref_init(&node->ref);
	return node;

err_map_buf:
	dma_buf_detach(node->buf, node->attachment);
err_attach_buf:
	dma_buf_put(node->buf);
err_get_buf:
	kfree(node);
	return ERR_PTR(-ENOENT);
}

struct sg_table* hicam_internal_get_sgtable(struct device *dev, int fd)
{
	struct sgtable_node *node;
	struct priv_ion_t *ion = dev_get_drvdata(dev);

	mutex_lock(&ion->sg_mutex);
	node = find_sgtable_node_by_fd(ion, fd);
	if (!IS_ERR(node)) {
		kref_get(&node->ref);
		mutex_unlock(&ion->sg_mutex);
		return node->sgt;
	}

	node = create_sgtable_node(ion, fd);
	if (!IS_ERR(node)) {
		list_add(&node->list, &ion->sg_nodes);
		mutex_unlock(&ion->sg_mutex);
		return node->sgt;
	}
	mutex_unlock(&ion->sg_mutex);
	return ERR_PTR(-ENODEV);
}

static void hicam_sgtable_deletor(struct kref *ref)
{
	struct sgtable_node *node = container_of(ref, struct sgtable_node, ref);

	/* release sgtable things we saved. */
	dma_buf_unmap_attachment(node->attachment, node->sgt, DMA_BIDIRECTIONAL);
	dma_buf_detach(node->buf, node->attachment);
	dma_buf_put(node->buf);

	list_del(&node->list);
	kfree(node);
}

void hicam_internal_put_sgtable(struct device *dev, struct sg_table *sgt)
{
	struct sgtable_node *node;
	struct priv_ion_t *ion = dev_get_drvdata(dev);
	mutex_lock(&ion->sg_mutex);
	node = find_sgtable_node_by_sg(ion, sgt);
	if (IS_ERR(node)) {
		cam_err("%s: putting non-exist sg_table:%pK.", __func__, sgt);
		goto err_out;
	}
	kref_put(&node->ref, hicam_sgtable_deletor);
err_out:
	mutex_unlock(&ion->sg_mutex);
}

int hicam_internal_get_phys(struct device *dev, int fd, struct phys_format *fmt)
{
	struct sg_table *sgt = hicam_internal_get_sgtable(dev, fd);
	if (IS_ERR(sgt)) {
		return PTR_ERR(sgt);
	}

	fmt->phys = sg_phys(sgt->sgl);
	hicam_internal_put_sgtable(dev, sgt);
	return 0;
}

phys_addr_t hicam_internal_get_pgd_base(struct device *dev)
{
	struct iommu_domain *domain;
	struct iommu_domain_data *data;

	(void)dev;
	domain = hisi_ion_enable_iommu(NULL);
	if (IS_ERR_OR_NULL(domain)) {
		cam_err("%s: fail to get iommu domain.", __func__);
		return 0;
	}

	data = domain->priv;
	if (IS_ERR_OR_NULL(data)) {
		cam_err("%s: iommu domain data is null.", __func__);
		return 0;
	}

	return data->phy_pgd_base;
}
//lint -save -e429
int hicam_internal_init(struct device *dev)
{
	const char *devname;
	struct ion_client *ion_client;
	struct priv_ion_t *ion = devm_kzalloc(dev, /* ion saved in drvdata. */
			sizeof(struct priv_ion_t), GFP_KERNEL);
	if (!ion) {
		cam_err("%s: failed to allocate internal data.", __func__);
		return -ENOMEM;
	}

	devname = dev_name(dev) ? dev_name(dev) : __FILE__;
	ion_client = hisi_ion_client_create(devname);
	if (IS_ERR(ion_client)) {
		cam_err("%s: failed to create ion_client.", __func__);
		devm_kfree(dev, ion);
		return PTR_ERR(ion_client);
	}

	cam_info("%s: ion:%pK, ion_client:%pK", __func__, ion, ion_client);

	ion->dev = dev;
	ion->ion_client = ion_client;
	mutex_init(&ion->sg_mutex);
	INIT_LIST_HEAD(&ion->sg_nodes);
	dev_set_drvdata(dev, ion);
	return 0;
}
//lint -restore

int hicam_internal_deinit(struct device *dev)
{
	struct priv_ion_t *ion = dev_get_drvdata(dev);
	if (!ion) {
		cam_err("%s: deinit before init.", __func__);
		return -EINVAL;
	}

	if (ion->ion_client) {
		ion_client_destroy(ion->ion_client);
		ion->ion_client = NULL;
	}

	hicam_internal_dump_debug_info(ion->dev);
	devm_kfree(dev, ion);
	return 0;
}
