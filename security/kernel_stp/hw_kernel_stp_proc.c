/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2018. All rights reserved.
 * Description: the hw_kernel_stp_proc.c for proc file create and destroy
 * Author: sunhongqing <sunhongqing@huawei.com>
 * Create: 2018-03-31
 */

#include "hw_kernel_stp_proc.h"
#ifdef CONFIG_HW_SLUB_DF
#include <linux/slub_def.h> /* for harden double-free check */
#endif

static const char *TAG = "kernel_stp_proc";
static struct proc_dir_entry *g_proc_entry;

static const umode_t FILE_CREAT_RO_MODE = 0220;
static const kgid_t SYSTEM_GID = KGIDT_INIT((gid_t)1000);

static inline ssize_t kernel_stp_trigger(unsigned int param)
{
	if (param == KERNEL_STP_TRIGGER_MARK) {
		KSTPLogTrace(TAG, "kernel stp trigger scanner success");
		kernel_stp_scanner();
		return 0;
	}
	KSTPLogError(TAG, "kernel stp trigger scanner fail");
	return -EINVAL;
}

static ssize_t kernel_stp_proc_write(struct file *file, const char __user *buffer,
				size_t count, loff_t *pos)
{
	char str[KERNEL_STP_PROC_MAX_LEN + 1] = { 0 };
	int ret = count;
	STP_PROC_TYPE data = {
		.val = 0
	};

	if ((count <= 0) || (count > KERNEL_STP_PROC_MAX_LEN) ||
		copy_from_user(str, buffer, count)) {
		KSTPLogError(TAG, "copy data from user failed");
		return -EFAULT;
	}

	if (kstrtoull(str, KERNEL_STP_PROC_HEX_BASE, &data.val)) {
		return -EINVAL;
	}
	KSTPLogTrace(TAG, "stp proc feature %u, param %u",
				data.s.feat, data.s.para);

	switch (data.s.feat) {
	case KERNEL_STP_SCAN:
		ret = kernel_stp_trigger(data.s.para);
		break;
	case HARDEN_DBLFREE_CHECK:
#ifdef CONFIG_HW_SLUB_DF
		ret = set_harden_double_free_status(data.s.para);
#endif
		break;
	default:
		ret = -EINVAL;
		break;
	}

	if (ret < 0) {
		KSTPLogError(TAG, "stp proc process error, %d", ret);
		return ret;
	}
	return count;
}

/*
 * the function is called by kerenl function
 * single_open(struct file *, int (*)(struct seq_file *, void *), void *)
 */
static int kernel_stp_proc_show(struct seq_file *m, void *v)
{
	seq_printf(m, "%d", 0);
	return 0;
}

static int kernel_stp_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, kernel_stp_proc_show, NULL);
}

static const struct file_operations kernel_stp_proc_fops = {
	.owner          = THIS_MODULE,
	.open           = kernel_stp_proc_open,
	.read           = seq_read,
	.write          = kernel_stp_proc_write,
	.llseek         = seq_lseek,
};

int kernel_stp_proc_init(void)
{
	g_proc_entry = proc_create("kernel_stp", FILE_CREAT_RO_MODE, NULL,
				&kernel_stp_proc_fops);

	if (g_proc_entry == NULL) {
		KSTPLogError(TAG, "g_proc_entry create is failed");
		return -ENOMEM;
	}

	/* set proc file gid to system gid */
	proc_set_user(g_proc_entry, GLOBAL_ROOT_UID, SYSTEM_GID);

	KSTPLogTrace(TAG, "g_proc_entry init success");
	return 0;
}

void kernel_stp_proc_exit(void)
{
	remove_proc_entry("kernel_stp", NULL);
	KSTPLogTrace(TAG, "g_proc_entry cleanup success");
}
