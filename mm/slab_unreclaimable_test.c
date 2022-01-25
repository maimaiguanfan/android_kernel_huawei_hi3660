
#include <linux/debugfs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/seq_file.h>

#include "slab_unreclaimable_test.h"

#define BUFFER_SIZE 16
#define MAX_THRESHOLD_LENGTH 12
#define MAX_THRESHOLD_VALUE 0x400000000

unsigned long g_slab_unreclaimable_threshold = 0x60000000;

static int slab_unreclaimable_test_read(struct seq_file *s, void *unused)
{
	seq_printf(s, "Current slab unreclaimable shreshold is set %lu bytes.\n",
			g_slab_unreclaimable_threshold);
	return 0;
}

static int slab_unreclaimable_test_open(struct inode *inode, struct file *fp)
{
	return single_open(fp, slab_unreclaimable_test_read, NULL);
}

static ssize_t slab_unreclaimable_test_write(struct file *fp,
					     const char __user *user_buf,
					     size_t count, loff_t *ppos)
{
	char kernel_buf[BUFFER_SIZE] = {0};

	pr_info("%s: user_buf: %p, count: %d, ppos: %lld\n",
		__func__, user_buf, count, *ppos);
	int ret = simple_write_to_buffer(kernel_buf, BUFFER_SIZE,
					 ppos, user_buf, count);

	if (count <= 1 || ret != count || count > MAX_THRESHOLD_LENGTH) {
		pr_err("%s: invalid input length %d.\n", __func__, ret);
		return ret;
	}

	kernel_buf[count - 1] = '\0';
	size_t pos = 0;

	while (kernel_buf[pos] != '\0') {
		if (kernel_buf[pos] < '0' || kernel_buf[pos] > '9') {
			pr_err("%s: invalid input type.\n", __func__);
			return ret;
		}
		pos++;
	}

	unsigned long threshold = 0;

	sscanf(kernel_buf, "%lu", &threshold);
	if (threshold > MAX_THRESHOLD_VALUE) {
		pr_err("%s: invalid input value: too large.\n", __func__);
		return ret;
	}
	g_slab_unreclaimable_threshold = threshold;
	pr_info("%s: slab unreclaimable threshold is changed to %lu bytes.\n",
		__func__, g_slab_unreclaimable_threshold);
	return ret;
}

static const struct file_operations slab_unreclaimable_test_fops = {
	.open = slab_unreclaimable_test_open,
	.read = seq_read,
	.write = slab_unreclaimable_test_write,
	.release = single_release,
};

static int __init slab_unreclaimable_test_init(void)
{
	struct dentry *file = debugfs_create_file("slab_unreclaimable_test",
						0644, NULL, NULL,
						&slab_unreclaimable_test_fops);
	if (file == NULL) {
		pr_err("%s: fail to create slab_unreclaimable_test entry\n",
			__func__);
		debugfs_remove(file);
	} else
		pr_info("%s: succeed to create slab_unreclaimable_test entry\n",
			__func__);
	return 0;
}

module_init(slab_unreclaimable_test_init);
MODULE_LICENSE("GPL");

