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

#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/dma-mapping.h>

#include <hicam_buf.h>
#include <cam_log.h>
#include <hicam_buf_priv.h>

struct hicam_buf_device {
	struct miscdevice dev;
	struct platform_device *pdev;
	atomic_t ref_count;
	void *private;
};

static struct hicam_buf_device *hicam_buf_dev = NULL;

void* hicam_buf_map_kernel(int fd)
{
	struct device *dev;
	if (IS_ERR_OR_NULL(hicam_buf_dev)) {
		cam_err("%s: hicam_buf_dev is null.", __func__);
		return NULL;
	}

	if (fd < 0) {
		cam_err("%s: fd invalid.", __func__);
		return NULL;
	}

	dev = &hicam_buf_dev->pdev->dev;
	return hicam_internal_map_kernel(dev, fd);
}

void hicam_buf_unmap_kernel(int fd)
{
	struct device *dev;
	if (IS_ERR_OR_NULL(hicam_buf_dev)) {
		cam_err("%s: hicam_buf_dev is null.", __func__);
		return ;
	}

	if (fd < 0) {
		cam_err("%s: fd invalid.", __func__);
		return ;
	}

	dev = &hicam_buf_dev->pdev->dev;
	hicam_internal_unmap_kernel(dev, fd);
}

int hicam_buf_map_iommu(int fd, struct iommu_format *fmt)
{
	struct device *dev;
	if (IS_ERR_OR_NULL(hicam_buf_dev)) {
		cam_err("%s: hicam_buf_dev is null.", __func__);
		return -ENODEV;
	}

	if (fd < 0 || !fmt) {
		cam_err("%s: fd or fmt invalid.", __func__);
		return -EINVAL;
	}

	dev = &hicam_buf_dev->pdev->dev;
	return hicam_internal_map_iommu(dev, fd, fmt);
}

void hicam_buf_unmap_iommu(int fd, struct iommu_format *fmt)
{
	struct device *dev;
	if (IS_ERR_OR_NULL(hicam_buf_dev)) {
		cam_err("%s: hicam_buf_dev is null.", __func__);
		return ;
	}

	if (fd < 0 || !fmt) {
		cam_err("%s: fd or fmt invalid.", __func__);
		return ;
	}

	dev = &hicam_buf_dev->pdev->dev;
	hicam_internal_unmap_iommu(dev, fd, fmt);
}

int hicam_buf_get_phys(int fd, struct phys_format *fmt)
{
	struct device *dev;
	if (IS_ERR_OR_NULL(hicam_buf_dev)) {
		cam_err("%s: hicam_buf_dev is null.", __func__);
		return -ENODEV;
	}

	if (fd < 0 || !fmt) {
		cam_err("%s: fd or fmt invalid.", __func__);
		return -EINVAL;
	}

	dev = &hicam_buf_dev->pdev->dev;
	return hicam_internal_get_phys(dev, fd, fmt);
}

// return 0 if pgd_base get failed, caller check please
phys_addr_t hicam_buf_get_pgd_base(void)
{
	struct device *dev;
	if (IS_ERR_OR_NULL(hicam_buf_dev)) {
		cam_err("%s: hicam_buf_dev is null.", __func__);
		return 0;
	}

	dev = &hicam_buf_dev->pdev->dev;
	return hicam_internal_get_pgd_base(dev);
}

int hicam_buf_sync(int fd, struct sync_format *fmt)
{
	if (fd < 0 || !fmt) {
		cam_err("%s: fd or fmt invalid.", __func__);
		return -EINVAL;
	}

	return hicam_internal_sync(fd, fmt);
}

int hicam_buf_local_sync(int fd, struct local_sync_format *fmt)
{
	if (fd < 0 || !fmt) {
		cam_err("%s: fd or fmt invalid.", __func__);
		return -EINVAL;
	}

	return hicam_internal_local_sync(fd, fmt);
}

// CARE: get sg_table will hold the related buffer,
// please save ret ptr, and call put with it.
struct sg_table* hicam_buf_get_sgtable(int fd)
{
	struct device *dev;
	if (IS_ERR_OR_NULL(hicam_buf_dev)) {
		cam_err("%s: hicam_buf_dev is null.", __func__);
		return ERR_PTR(-ENODEV);
	}

	if (fd < 0) {
		cam_err("%s: fd invalid.", __func__);
		return ERR_PTR(-EINVAL);
	}
	dev = &hicam_buf_dev->pdev->dev;
	return hicam_internal_get_sgtable(dev, fd);
}

void hicam_buf_put_sgtable(struct sg_table *sgt)
{
	struct device *dev;
	if (IS_ERR_OR_NULL(hicam_buf_dev)) {
		cam_err("%s: hicam_buf_dev is invalid.", __func__);
		return ;
	}

	if (IS_ERR_OR_NULL(sgt)) {
		cam_err("%s: sgt is invalid.", __func__);
		return ;
	}
	dev = &hicam_buf_dev->pdev->dev;
	hicam_internal_put_sgtable(dev, sgt);
}

static long hicam_config(struct hicam_buf_cfg *cfg)
{
	long ret = 0;
	switch (cfg->type) {
	case HICAM_BUF_MAP_IOMMU:
		ret = hicam_buf_map_iommu(cfg->fd, &cfg->iommu_format);
		break;
	case HICAM_BUF_UNMAP_IOMMU:
		hicam_buf_unmap_iommu(cfg->fd, &cfg->iommu_format);
		break;
	case HICAM_BUF_SYNC:
		ret = hicam_buf_sync(cfg->fd, &cfg->sync_format);
		break;
	case HICAM_BUF_LOCAL_SYNC:
		ret = hicam_buf_local_sync(cfg->fd, &cfg->local_sync_format);
		break;
	case HICAM_BUF_GET_PHYS:
		ret = hicam_buf_get_phys(cfg->fd, &cfg->phys_format);
		break;
	}

	return ret;
}

static long hicam_buf_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	long ret = 0;
	struct hicam_buf_cfg data;

	if (_IOC_SIZE(cmd) > sizeof(data)) {
		cam_err("%s: cmd size too large!\n", __func__);
		return -EINVAL;
	}

	if (copy_from_user(&data, (void __user *)arg, _IOC_SIZE(cmd))){
		cam_err("%s: copy in arg failed!\n", __func__);
		return -EFAULT;
	}

	switch (cmd) {
	case HICAM_BUF_IOC_CFG:
		ret = hicam_config(&data);
		break;
	default:
		cam_info("%s: invalid command %d.", __func__, cmd);
		return -EINVAL;
	}

	if (copy_to_user((void __user *)arg, &data, _IOC_SIZE(cmd))){
		cam_err("%s: copy back arg failed!\n", __func__);
		return -EFAULT;
	}

	return ret;
}

static int hicam_buf_release(struct inode *inode, struct file *file)
{
	int ref_count;
	struct hicam_buf_device *idev = file->private_data;
	struct miscdevice *mdev = &idev->dev;

	ref_count = atomic_dec_return(&idev->ref_count);
	cam_info("%s: %s device closed, ref:%d.",
			__func__, mdev->name, ref_count);
	return 0;
}

static int hicam_buf_open(struct inode *inode, struct file *file)
{
	int ref_count;
	struct miscdevice *mdev = file->private_data;
	struct hicam_buf_device *idev = container_of(mdev, struct hicam_buf_device, dev);
	file->private_data = idev;

	ref_count = atomic_inc_return(&idev->ref_count);
	cam_info("%s: %s device opened, ref:%d.",
			__func__, mdev->name, ref_count);
	return 0;
}

static const struct file_operations hicam_buf_fops = {
	.owner = THIS_MODULE,
	.open = hicam_buf_open,
	.release = hicam_buf_release,
	.unlocked_ioctl = hicam_buf_ioctl,
#if CONFIG_COMPAT
	.compat_ioctl = hicam_buf_ioctl,
#endif
};

static int hicam_buf_parse_of_node(struct hicam_buf_device *idev)
{
	struct device_node *np = idev->pdev->dev.of_node;
	if (!np) {
		return -ENODEV;
	}

	return 0;
}

struct hicam_buf_device *hicam_buf_device_create(struct platform_device *pdev)
{
	int ret;
	struct hicam_buf_device *idev;

	idev = kzalloc(sizeof(*idev), GFP_KERNEL);
	if (!idev)
		return ERR_PTR(-ENOMEM);

	atomic_set(&idev->ref_count, 0);
	idev->pdev = pdev;

	ret = hicam_buf_parse_of_node(idev);
	if (ret) {
		cam_err("%s: failed to parse device of_node.", __func__);
		goto err_out;
	}

	idev->dev.minor = MISC_DYNAMIC_MINOR;
	idev->dev.name = "hicam_buf"; /* dev name under /dev */
	idev->dev.fops = &hicam_buf_fops;
	idev->dev.parent = NULL;
	ret = misc_register(&idev->dev);
	if (ret) {
		cam_err("%s: failed to register misc device.", __func__);
		goto err_out;
	}

	return idev;
err_out:
	kfree(idev);
	return ERR_PTR(ret);
}

void hicam_buf_device_destroy(struct hicam_buf_device *idev)
{
	misc_deregister(&idev->dev);
	kfree(idev);
}

#ifdef CONFIG_HISI_DEBUG_FS
ssize_t hicam_buf_info_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	hicam_internal_dump_debug_info(dev);
	return snprintf(buf, PAGE_SIZE, "info dumpped to kmsg...");
}
DEVICE_ATTR_RO(hicam_buf_info);
#endif /* CONFIG_HISI_DEBUG_FS */

static int32_t hicam_buf_probe(struct platform_device* pdev)
{
	int rc = 0;

	if (!pdev) {
		cam_err("%s: null pdev.", __func__);
		return -ENODEV;
	}

	dma_set_mask_and_coherent(&pdev->dev, DMA_BIT_MASK(64)); /*lint !e598 !e648 */
	hicam_buf_dev = hicam_buf_device_create(pdev);
	if (IS_ERR(hicam_buf_dev)) {
		cam_err("%s: fail to create hicam ion device.", __func__);
		rc = PTR_ERR(hicam_buf_dev);
		goto err_create_device;
	}

	rc = hicam_internal_init(&hicam_buf_dev->pdev->dev);
	if (rc) {
		cam_err("%s: fail to init internal data.", __func__);
		goto err_init_internal;
	}

#ifdef CONFIG_HISI_DEBUG_FS
	rc = device_create_file(&pdev->dev, &dev_attr_hicam_buf_info);
	if (rc < 0) {
		// just log it, it's not fatal.
		cam_err("%s: fail to create hicam_buf_info file.", __func__);
	}
#endif /* CONFIG_HISI_DEBUG_FS */
	return 0;
err_init_internal:
	hicam_buf_device_destroy(hicam_buf_dev);
err_create_device:
	hicam_buf_dev = NULL;
	return rc;
}

static int32_t hicam_buf_remove(struct platform_device* pdev)
{
	if (!hicam_buf_dev) {
		cam_err("%s: hicam_buf_dev is not inited.", __func__);
		return -EINVAL;
	}
	hicam_internal_deinit(&hicam_buf_dev->pdev->dev);
	hicam_buf_device_destroy(hicam_buf_dev);
#ifdef CONFIG_HISI_DEBUG_FS
	device_remove_file(&pdev->dev, &dev_attr_hicam_buf_info);
#endif /* CONFIG_HISI_DEBUG_FS */
	return 0;
}

static const struct of_device_id hicam_buf_dt_match[] = {
	{ .compatible = "huawei,hicam_buf", },
	{ },
};
MODULE_DEVICE_TABLE(of, hicam_buf_dt_match);

static struct platform_driver hicam_buf_platform_driver = {
	.driver = {
		.name = "huawei,hicam_buf",
		.owner = THIS_MODULE,
		.of_match_table = hicam_buf_dt_match,
	},

	.probe = hicam_buf_probe,
	.remove = hicam_buf_remove,
};

static int __init hicam_buf_init_module(void)
{
	cam_info("enter %s", __func__);
	return platform_driver_register(&hicam_buf_platform_driver);
}

static void __exit hicam_buf_exit_module(void)
{
	platform_driver_unregister(&hicam_buf_platform_driver);
}

subsys_initcall_sync(hicam_buf_init_module);
module_exit(hicam_buf_exit_module);

MODULE_DESCRIPTION("hicam_buf");
MODULE_LICENSE("GPL v2");
