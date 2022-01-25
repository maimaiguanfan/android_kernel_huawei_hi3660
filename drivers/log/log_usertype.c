/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2018. All rights reserved.
 *
 * Description: drivers to indicate user types in kernel
 *      Author: shenchenkai <shenchenkai@huawei.com>
 */

#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

#include <log/log_usertype.h>

static unsigned int log_usertype = 0;

unsigned int get_log_usertype(void)
{
	return log_usertype;
}
EXPORT_SYMBOL(get_log_usertype);

static void set_log_usertype(int value)
{
	log_usertype = value;
}

static int log_usertype_info_show(struct seq_file *m, void *v)
{
	seq_printf(m, "%d\n", log_usertype);
	return 0;
}

static int log_usertype_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, log_usertype_info_show, NULL);
}

static ssize_t log_usertype_proc_write(struct file *file,
		const char __user *buffer, size_t count, loff_t *data)
{
	char tmp;

	/* should be '0' to '9' with '\0' */
	if (count > 2)
		return -EINVAL;

	/* should be assigned only once */
	if (get_log_usertype() > 0)
		return -EINVAL;

	if (copy_from_user(&tmp, buffer, 1))
		return -EFAULT;

	if (tmp >= '1' && tmp <= '9')
		set_log_usertype((unsigned int)(tmp - '0'));

	return count;
}

static const struct file_operations log_usertype_proc_fops = {
	.open		= log_usertype_proc_open,
	.read		= seq_read,
	.write		= log_usertype_proc_write,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static int __init log_usertype_proc_init(void)
{
	proc_create("log-usertype", 0600, NULL, &log_usertype_proc_fops);
	return 0;
}

module_init(log_usertype_proc_init);
