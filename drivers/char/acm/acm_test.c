/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2017-2020. All rights reserved.
 * Description:
 *
 *     This module is just for debug use, and DOES NOT COMPILE in commercial
 *     versions.
 *
 *     Debugfs interfaces ofAccess Control Module. This module will
 *     create some files under /sys/kernel/debug for white-box testing of
 *     Access Control Module.
 *
 *     This file is released under the GPL v2.
 */

#include <linux/debugfs.h>

static struct dentry *acm_debugfs_root;

bool acm_hash_empty(struct hlist_head *hash)
{
	struct hlist_head *phead = NULL;
	bool ret = true;
	int i;

	for (i = 0; i < ACM_HASHTBL_SZ; i++) {
		phead = &hash[i];
		if (!hlist_empty(phead)) {
			ret = false;
			break;
		}
	}

	return ret;
}

static int acm_hash_table_open(struct inode *inode, struct file *filp)
{
	int i;
	int err = 0;
	struct hlist_head *phead = NULL;
	struct acm_hnode *pnode = NULL;
	struct hlist_head *hash = acm_hash.head;

	filp->private_data = inode->i_private;

	for (i = 0; i < ACM_HASHTBL_SZ; i++) {
		pr_info("===acm_hash[%d]===\n", i);
		phead = &(hash[i]);
		hlist_for_each_entry(pnode, phead, hnode) {
			if (pnode)
				pr_info("======PKGNAME=%s\n",
				       pnode->pkgname);
		}
	}
	return err;
}

static int acm_fwk_list_open(struct inode *inode, struct file *filp)
{
	int err = 0;
	struct acm_lnode *pnode = NULL;

	filp->private_data = inode->i_private;

	pr_info("===acm_fwk_list===\n");
	list_for_each_entry(pnode, &acm_fwk_list.head, lnode) {
		if (pnode)
			pr_info("======pkgname=%s \t path=%s\n",
			       pnode->pkgname, pnode->path);
	}
	pr_info("===acm_fwk_list.nrnodes:%lu===\n", acm_fwk_list.nr_nodes);

	return err;
}

static int acm_dmd_list_open(struct inode *inode, struct file *filp)
{
	int err = 0;
	struct acm_lnode *pnode = NULL;

	filp->private_data = inode->i_private;

	pr_info("===acm_dmd_list===\n");
	list_for_each_entry(pnode, &acm_dmd_list.head, lnode) {
		if (pnode)
			pr_info("======pkgname=%s \t path=%s \t\n",
			       pnode->pkgname, pnode->path);
	}
	pr_info("===acm_dmd_list.nrnodes:%lu===\n", acm_dmd_list.nr_nodes);

	return err;
}

static int acm_test_acm_fwk_upload(struct inode *inode, struct file *filp)
{
	struct acm_lnode *list_node = NULL;
	struct acm_lnode *tail_node = NULL;

	filp->private_data = inode->i_private;

	acm_list_cleanup(&acm_fwk_list);
	if (!list_empty(&acm_fwk_list.head) || acm_fwk_list.nr_nodes != 0) {
		pr_err("Faile to execute test!\n");
		return -EINVAL;
	}

	pr_info("===Testing acm_fwk_upload()===\n");
	/* Add a node to fwk list, then do upload, List should be empty */
	list_node = kzalloc(sizeof(struct acm_lnode), GFP_KERNEL);
	if (!list_node)
		return -ENOMEM;
	strcpy(list_node->pkgname, "hello");
	strcpy(list_node->path, "h");
	acm_fwk_add(&acm_fwk_list.head, list_node);

	tail_node = (struct acm_lnode *)list_entry(acm_fwk_list.head.prev,
						   struct acm_lnode, lnode);
	if (strcmp(tail_node->pkgname, list_node->pkgname) ||
	    strcmp(tail_node->path, list_node->path)) {
		pr_err("Error!");
		return -EINVAL;
	}

	upload_data_to_fwk();
	if (!list_empty(&acm_fwk_list.head)) {
		pr_err("Error!");
		return -EINVAL;
	}

	/* Add 2 nodes to fwk list, then do upload.List should be empty */
	list_node = kzalloc(sizeof(struct acm_lnode), GFP_KERNEL);
	if (!list_node)
		return -ENOMEM;
	strcpy(list_node->pkgname, "hello");
	strcpy(list_node->path, "h");
	acm_fwk_add(&acm_fwk_list.head, list_node);
	tail_node = (struct acm_lnode *)list_entry(acm_fwk_list.head.prev,
						   struct acm_lnode, lnode);
	if (strcmp(tail_node->pkgname, list_node->pkgname) ||
	    strcmp(tail_node->path, list_node->path)) {
		pr_err("Error!");
		return -EINVAL;
	}

	list_node = kzalloc(sizeof(struct acm_lnode), GFP_KERNEL);
	if (!list_node)
		return -ENOMEM;
	strcpy(list_node->pkgname, "hello");
	strcpy(list_node->path, "h2");
	acm_fwk_add(&acm_fwk_list.head, list_node);

	tail_node = (struct acm_lnode *)list_entry(acm_fwk_list.head.prev,
						   struct acm_lnode, lnode);
	if (strcmp(tail_node->pkgname, list_node->pkgname) ||
	    strcmp(tail_node->path, list_node->path)) {
		pr_err("Error!");
		return -EINVAL;
	}

	upload_data_to_fwk();
	if (!list_empty(&acm_fwk_list.head)) {
		pr_err("Error!");
		return -EINVAL;
	}

	pr_info("===Testing acm_fwk_upload() PASSED!===\n");
	return ACM_SUCCESS;
}

static void add_hash(struct acm_hnode *node, char *pkg)
{
	int idx;
	struct hlist_head *phead = NULL;
	struct hlist_head *hash = acm_hash.head;

	INIT_HLIST_NODE(&node->hnode);
	strcpy(node->pkgname, pkg);
	node->pkgname[ACM_PKGNAME_MAX - 1] = '\0';
	idx = elf_hash(node->pkgname);
	spin_lock(&acm_hash.spinlock);
	phead = &hash[idx];
	hlist_add_head(&node->hnode, phead);
	acm_hash.nr_nodes++;
	spin_unlock(&acm_hash.spinlock);
}

static int acm_test_acm_hsearch(struct inode *inode, struct file *filp)
{
	struct acm_hnode *temp_hash_node = NULL;
	char teststr1[ACM_PKGNAME_MAX] = {"com.acm.testacm50"};
	char teststr2[ACM_PKGNAME_MAX] = {"com.acm.testacm51"};
	char teststr3[ACM_PKGNAME_MAX] = {"com.acm.testacm52"};

	filp->private_data = inode->i_private;

	/* Clean hash table, then add 3nodes */
	acm_hash_cleanup(acm_hash.head);
	if (!acm_hash_empty(acm_hash.head)) {
		pr_err("Error!");
		return -ENODATA;
	}
	temp_hash_node = kzalloc(sizeof(struct acm_hnode), GFP_KERNEL);
	if (!temp_hash_node)
		return -ENOMEM;
	add_hash(temp_hash_node, teststr1);

	temp_hash_node = kzalloc(sizeof(struct acm_hnode), GFP_KERNEL);
	if (!temp_hash_node)
		return -ENOMEM;
	add_hash(temp_hash_node, teststr2);

	temp_hash_node = kzalloc(sizeof(struct acm_hnode), GFP_KERNEL);
	if (!temp_hash_node)
		return -ENOMEM;
	add_hash(temp_hash_node, teststr3);

	pr_info("===Testing acm_hsearch()===\n");

	/* Search for teststr1 */
	temp_hash_node = acm_hsearch(acm_hash.head, teststr1);
	if (!temp_hash_node) {
		pr_err("Error!");
		return -EINVAL;
	}
	if (strcmp(temp_hash_node->pkgname, teststr1)) {
		pr_err("Error!");
		return -EINVAL;
	}
	/* Search for teststr2 */
	temp_hash_node = acm_hsearch(acm_hash.head, teststr2);
	if (!temp_hash_node) {
		pr_err("Error!");
		return -EINVAL;
	}
	if (strcmp(temp_hash_node->pkgname, teststr2)) {
		pr_err("Error!");
		return -EINVAL;
	}
	/* Search for teststr3 */
	temp_hash_node = acm_hsearch(acm_hash.head, teststr3);
	if (!temp_hash_node) {
		pr_err("Error!");
		return -EINVAL;
	}
	if (strcmp(temp_hash_node->pkgname, teststr3)) {
		pr_err("Error!");
		return -EINVAL;
	}

	pr_info("===Testing acm_hsearch() PASSED!===\n");
	return ACM_SUCCESS;
}

static int acm_test_ioctl(struct inode *inode, struct file *filp)
{
	struct acm_hnode *temp_hash_node = NULL;
	struct hlist_head *phead = NULL;
	struct hlist_head *hash = acm_hash.head;
	char teststr[ACM_PKGNAME_MAX] = {"com.acm.testacm50"};

	filp->private_data = inode->i_private;

	pr_info("===Testing acm_ioctl()===\n");
	/* Test ACM_ADD */
	acm_hash_cleanup(acm_hash.head);
	pr_info("VAL acm_hash_empty=%d\n", acm_hash_empty(acm_hash.head));
	if (!acm_hash_empty(acm_hash.head)) {
		pr_err("Error!");
		return -ENODATA;
	}
	temp_hash_node = kzalloc(sizeof(struct acm_hnode), GFP_KERNEL);
	if (!temp_hash_node)
		return -ENOMEM;
	add_hash(temp_hash_node, teststr);
	spin_lock(&acm_hash.spinlock);
	phead = &hash[0];
	temp_hash_node = hlist_entry(phead->first, struct acm_hnode, hnode);
	spin_unlock(&acm_hash.spinlock);
	if (strcmp(temp_hash_node->pkgname, teststr)) {
		pr_err("Error!");
		return -EINVAL;
	}

	/* Test ACM_SEARCH */
	temp_hash_node = acm_hsearch(acm_hash.head, teststr);
	if (!temp_hash_node) {
		pr_err("Error!");
		return -EINVAL;
	}
	if (strcmp(temp_hash_node->pkgname, teststr)) {
		pr_err("Error!");
		return -EINVAL;
	}

	/* Test ACM_DEL */
	temp_hash_node = acm_hsearch(acm_hash.head, teststr);
	if (!temp_hash_node) {
		pr_err("Error!");
		return -ENODATA;
	}
	spin_lock(&acm_hash.spinlock);
	hlist_del(&temp_hash_node->hnode);
	acm_hash.nr_nodes--;
	spin_unlock(&acm_hash.spinlock);
	kfree(temp_hash_node);
	pr_info("VAL acm_hash_empty=%d\n", acm_hash_empty(acm_hash.head));
	if (!acm_hash_empty(acm_hash.head)) {
		pr_err("Error!");
		return -ENODATA;
	}

	pr_info("===Testing acm_ioctl() PASSED!===\n");
	return ACM_SUCCESS;
}

int acm_enable;
EXPORT_SYMBOL(acm_enable);
static int acm_enable_open(struct inode *inode, struct file *filp)
{

	filp->private_data = inode->i_private;

	return 0;
}

#define ENABLE_NUM_LEN 4
static ssize_t acm_enable_read(struct file *filp, char __user *buff,
			       size_t count, loff_t *offp)
{
	char kbuf[ENABLE_NUM_LEN] = {'\0'};

	snprintf(kbuf, sizeof(kbuf), "%d", acm_enable);
	pr_info("kbuf = %s acm_enable = %d\n", kbuf, acm_enable);
	if (copy_to_user(buff, kbuf, ENABLE_NUM_LEN))
		return -EFAULT;

	return 0;
}

static ssize_t acm_enable_write(struct file *filp, const char __user *buff,
				size_t count, loff_t *offp)
{
	char kbuf[ENABLE_NUM_LEN] = {'\0'};

	if (copy_from_user(kbuf, buff, ENABLE_NUM_LEN))
		return -EFAULT;
	if (!strncmp(kbuf, "1", 1))
		acm_enable = 1;
	else
		acm_enable = 0;
	pr_info("acm_enable = %d\n", acm_enable);
	return count;
}

ssize_t acm_test_read(struct file *filp, char __user *buff,
		      size_t count, loff_t *offp)
{
	return 0;
}

ssize_t acm_test_write(struct file *filp, const char __user *buff,
		       size_t count, loff_t *offp)
{
	return 0;
}

static const struct file_operations acm_hash_table_file_ops = {
	.owner = THIS_MODULE,
	.open = acm_hash_table_open,
	.read = acm_test_read,
	.write = acm_test_write,
};

static const struct file_operations acm_fwk_list_file_ops = {
	.owner = THIS_MODULE,
	.open = acm_fwk_list_open,
	.read = acm_test_read,
	.write = acm_test_write,
};

static const struct file_operations acm_dmd_list_file_ops = {
	.owner = THIS_MODULE,
	.open = acm_dmd_list_open,
	.read = acm_test_read,
	.write = acm_test_write,
};

static const struct file_operations acm_fwk_upload_file_ops = {
	.owner = THIS_MODULE,
	.open = acm_test_acm_fwk_upload,
	.read = acm_test_read,
	.write = acm_test_write,
};

static const struct file_operations acm_hsearch_file_ops = {
	.owner = THIS_MODULE,
	.open = acm_test_acm_hsearch,
	.read = acm_test_read,
	.write = acm_test_write,
};

static const struct file_operations acm_test_ioctl_file_ops = {
	.owner = THIS_MODULE,
	.open = acm_test_ioctl,
	.read = acm_test_read,
	.write = acm_test_write,
};

static const struct file_operations acm_enable_file_ops = {
	.owner = THIS_MODULE,
	.open = acm_enable_open,
	.read = acm_enable_read,
	.write = acm_enable_write,
};

static int __init acm_debugfs_init(void)
{
	static struct dentry *acm_hash_table = NULL;
	static struct dentry *acm_fwk_list = NULL;
	static struct dentry *acm_dmd_list = NULL;
	static struct dentry *acm_fwk_upload = NULL;
	static struct dentry *acm_hsearch = NULL;
	static struct dentry *acm_ioctl = NULL;
	static struct dentry *enable = NULL;

	pr_info("Initializing debugfs.\n");
	acm_debugfs_root = debugfs_create_dir("acm_debug", NULL);
	if (!acm_debugfs_root) {
		pr_err("Failed to create acm_debug directory!\n");
		return -EINVAL;
	}

	acm_hash_table = debugfs_create_file("acm_hash_table",
		0644, acm_debugfs_root, NULL, &acm_hash_table_file_ops);
	if (!acm_hash_table) {
		pr_err("Failed to create file acm_hash_table!\n");
		goto free_debugfs;
	}

	acm_fwk_list = debugfs_create_file("acm_fwk_list", 0644,
		acm_debugfs_root, NULL, &acm_fwk_list_file_ops);
	if (!acm_fwk_list) {
		pr_err("Failed to create file acm_fwk_list!\n");
		goto free_debugfs;
	}

	acm_dmd_list = debugfs_create_file("acm_dmd_list", 0644,
		acm_debugfs_root, NULL, &acm_dmd_list_file_ops);
	if (!acm_dmd_list) {
		pr_err("Failed to create file acm_dmd_list!\n");
		goto free_debugfs;
	}

	acm_fwk_upload = debugfs_create_file("acm_fwk_upload", 0644,
		acm_debugfs_root, NULL, &acm_fwk_upload_file_ops);
	if (!acm_fwk_upload) {
		pr_err("Failed to create file acm_fwk_upload!\n");
		goto free_debugfs;
	}

	acm_hsearch = debugfs_create_file("acm_hsearch", 0644,
		acm_debugfs_root, NULL, &acm_hsearch_file_ops);
	if (!acm_hsearch) {
		pr_err("Failed to create file acm_hsearch!\n");
		goto free_debugfs;
	}

	acm_ioctl = debugfs_create_file("acm_ioctl", 0644, acm_debugfs_root,
		NULL, &acm_test_ioctl_file_ops);
	if (!acm_ioctl) {
		pr_err("Failed to create file acm_test_ioctl!\n");
		goto free_debugfs;
	}

	enable = debugfs_create_file("enable",
		0644, acm_debugfs_root, NULL, &acm_enable_file_ops);
	if (!enable) {
		pr_err("Failed to create file acm_enable!\n");
		goto free_debugfs;
	}

	return ACM_SUCCESS;

free_debugfs:
	debugfs_remove_recursive(acm_debugfs_root);
	return -EINVAL;
}

static void __exit acm_debugfs_exit(void)
{
	debugfs_remove_recursive(acm_debugfs_root);
}

MODULE_LICENSE("GPL");
module_init(acm_debugfs_init);
module_exit(acm_debugfs_exit);
