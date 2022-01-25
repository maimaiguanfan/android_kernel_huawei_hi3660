/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2017-2020. All rights reserved.
 * Description: Access Control Module source code. This module maintains
 *     a white list consists of app package names. Apps in the whitelist
 *     can delete media files freely, while apps not included by the whitelist
 *     can NOT delete media files. When a not-whitelist-app tries to delete a
 *     media file, the file was renamed to origin_name.hwbk by filesystem, and
 *     then moved to the Recycle Bin by Gallery. The medie file type was
 *     maintained in fs/f2fs/namei.c.
 *
 *     This file is released under the GPL v2.
 */
#include "acm.h"
#include <linux/completion.h>

/* Hash table for white list */
static struct acm_htbl acm_hash;
/* List for dir */
static struct acm_list acm_dir_list;

static dev_t acm_devno;
static struct cdev *acm_cdev;
static struct class *acm_class;
static struct device *acm_device;
static struct kset *acm_kset;

/* List for framework */
static struct acm_list acm_fwk_list;
static struct task_struct *acm_fwk_task;
static struct acm_env fwk_env;

#ifdef CONFIG_ACM_DSM
static struct acm_list acm_dmd_list;
static struct task_struct *acm_dmd_task;
static struct acm_cache dmd_cache;
static struct acm_env dsm_env;
DECLARE_COMPLETION(acm_dmd_comp);
#endif

/*
 * The flag of acm state after acm_init.
 * 0 is successful or none-zero errorno failed.
 * It should be initialized as none-zero.
 */
static long acm_init_state = -ENOTTY;

static int valid_len(const char *str, size_t maxlen)
{
	size_t len;

	len = strnlen(str, maxlen);
	if (len == 0 || len > maxlen - 1)
		return -EINVAL;

	return ACM_SUCCESS;
}

static size_t get_valid_strlen(char *p, size_t maxlen)
{
	size_t len;

	len = strnlen(p, maxlen);
	if (len > maxlen - 1) {
		len = maxlen - 1;
		*(p + len) = '\0';
	}

	return len;
}

static void sleep_if_list_empty(struct acm_list *list)
{
	set_current_state(TASK_INTERRUPTIBLE);
	spin_lock(&list->spinlock);
	if (list_empty(&list->head)) {
		spin_unlock(&list->spinlock);
		schedule();
	} else {
		spin_unlock(&list->spinlock);
	}

	set_current_state(TASK_RUNNING);
}

static struct acm_lnode *get_first_entry(struct list_head *head)
{
	struct list_head *pos = NULL;
	struct acm_lnode *node = NULL;

	pos = head->next;
	node = (struct acm_lnode *)list_entry(pos, struct acm_lnode, lnode);
	list_del(pos);

	return node;
}

/* elf_hash function */
static unsigned int elf_hash(const char *str)
{
	unsigned int x = 0;
	unsigned int hash = 0;
	unsigned int ret;

	while (*str) {
		hash = (hash << ACM_HASH_LEFT_SHIFT) + (*str++);
		x = hash & ACM_HASH_MASK;
		if (x != 0) {
			hash ^= (x >> ACM_HASH_RIGHT_SHIFT);
			hash &= ~x;
		}
	}
	ret = (hash & ACM_HASH_RESULT_MASK) % ACM_HASHTBL_SZ;
	return ret;
}

/*
 * Search for a key in hash table. Note that caller is responsible for
 * holding acm_hash.spinlock.
 */
static struct acm_hnode *acm_hsearch(const struct hlist_head *hash, const char *keystring)
{
	const struct hlist_head *phead = NULL;
	struct acm_hnode *pnode = NULL;
	unsigned int idx;

	idx = elf_hash(keystring);
	phead = &hash[idx];
	hlist_for_each_entry(pnode, phead, hnode) {
		if (pnode) {
			if (!strcmp(pnode->pkgname, keystring))
				break;
		}
	}
	return pnode;
}

/*
 * Add a key into @hash_table. Note that caller is responsible for
 * holding locks of @hash_table.
 */
static inline int acm_hash_add(struct acm_htbl *hash_table,
			 struct acm_hnode *hash_node)
{
	struct hlist_head *phead = NULL;
	struct hlist_head *hash = hash_table->head;

	if (unlikely(hash_table->nr_nodes > HASHTBL_MAX_SZ - 1)) {
		pr_err("Failed to add node, acm hash table is full!\n");
		return -ENOSPC;
	}
	phead = &hash[elf_hash(hash_node->pkgname)];
	hlist_add_head(&hash_node->hnode, phead);
	hash_table->nr_nodes++;
	return ACM_SUCCESS;
}

/*
 * Delete a key from @hash_table. Note that caller is responsible for
 * holding locks of @hash_table.
 */
static inline void acm_hash_del(struct acm_htbl *hash_table,
			 struct acm_hnode *hash_node)
{
	WARN_ON(hash_table->nr_nodes < 1);
	hlist_del(&(hash_node->hnode));
	hash_table->nr_nodes--;
	kfree(hash_node);
}

/*
 * Add a node to @head. Note that caller is responsible for
 * holding locks of @head.
 */
static inline int acm_dir_list_add(struct list_head *head, struct acm_dnode *node)
{
	if (unlikely(acm_dir_list.nr_nodes > ACM_DIR_LIST_MAX - 1)) {
		pr_err("Failed to add node, acm dir list is full!\n");
		return -ENOSPC;
	}
	list_add_tail(&node->lnode, head);
	acm_dir_list.nr_nodes++;
	return ACM_SUCCESS;
}

static inline void acm_fwk_add(struct list_head *head, struct acm_lnode *node)
{
	spin_lock(&acm_fwk_list.spinlock);
	list_add_tail(&node->lnode, head);
	spin_unlock(&acm_fwk_list.spinlock);
}

/* ioctl related functions */
static int do_cmd_add(unsigned long args)
{
	int err = 0;
	struct acm_hnode *hnode = NULL;
	struct acm_hnode *result = NULL;

	hnode = kzalloc(sizeof(*hnode), GFP_KERNEL);
	if (!hnode)
		return -ENOMEM;
	INIT_HLIST_NODE(&hnode->hnode);

	if (copy_from_user(hnode->pkgname, (char *)args,
		ACM_PKGNAME_MAX)) {
		err = -EFAULT;
		goto do_cmd_add_ret;
	}
	if (valid_len(hnode->pkgname, ACM_PKGNAME_MAX)) {
		err = -EINVAL;
		goto do_cmd_add_ret;
	}
	hnode->pkgname[ACM_PKGNAME_MAX - 1] = '\0';

	spin_lock(&acm_hash.spinlock);
	result = acm_hsearch(acm_hash.head, hnode->pkgname);
	if (result) {
		spin_unlock(&acm_hash.spinlock);
		pr_err("ACM: Package is already in the white list!\n");
		err = ACM_SUCCESS;
		goto do_cmd_add_ret;

	}

	err = acm_hash_add(&acm_hash, hnode);
	spin_unlock(&acm_hash.spinlock);
	if (err < 0) {
		goto do_cmd_add_ret;
	}
	return err;

do_cmd_add_ret:
	kfree(hnode);
	return err;
}

static int do_cmd_delete(unsigned long args)
{
	int err = 0;
	struct acm_hnode *hnode = NULL;
	struct acm_fwk_pkg *fwk_pkg = NULL;

	fwk_pkg = kzalloc(sizeof(*fwk_pkg), GFP_KERNEL);
	if (!fwk_pkg)
		return -ENOMEM;

	if (copy_from_user(fwk_pkg->pkgname, (char *)args, ACM_PKGNAME_MAX)) {
		err = -EFAULT;
		goto do_cmd_delete_ret;
	}
	if (valid_len(fwk_pkg->pkgname, ACM_PKGNAME_MAX)) {
		err = -EINVAL;
		goto do_cmd_delete_ret;
	}
	fwk_pkg->pkgname[ACM_PKGNAME_MAX - 1] = '\0';

	spin_lock(&acm_hash.spinlock);
	hnode = acm_hsearch(acm_hash.head, fwk_pkg->pkgname);
	if (!hnode) {
		spin_unlock(&acm_hash.spinlock);
		pr_err("Package not found!\n");
		err = -ENODATA;
		goto do_cmd_delete_ret;
	}

	acm_hash_del(&acm_hash, hnode);
	spin_unlock(&acm_hash.spinlock);
	hnode = NULL;

do_cmd_delete_ret:
	kfree(fwk_pkg);
	return err;
}

static int do_cmd_search(unsigned long args)
{
	int err = 0;
	struct acm_hnode *hsearch_ret = NULL;
	struct acm_mp_node *mp_node = NULL;
	struct timespec __maybe_unused start;
	struct timespec __maybe_unused end;

	if (IS_ENABLED(CONFIG_ACM_TIME_COST))
		getrawmonotonic(&start);

	mp_node = kzalloc(sizeof(*mp_node), GFP_KERNEL);
	if (!mp_node)
		return -ENOMEM;
	if (copy_from_user(mp_node, (struct acm_mp_node *)args,
			   sizeof(struct acm_mp_node))) {
		err = -EFAULT;
		goto do_cmd_search_ret;
	}
	if (valid_len(mp_node->pkgname, ACM_PKGNAME_MAX)) {
		err = -EINVAL;
		goto do_cmd_search_ret;
	}
	if (valid_len(mp_node->path, ACM_PATH_MAX)) {
		err = -EINVAL;
		goto do_cmd_search_ret;
	}
	mp_node->pkgname[ACM_PKGNAME_MAX - 1] = '\0';
	mp_node->path[ACM_PATH_MAX - 1] = '\0';

	spin_lock(&acm_hash.spinlock);
	hsearch_ret = acm_hsearch(acm_hash.head, mp_node->pkgname);
	spin_unlock(&acm_hash.spinlock);
	mp_node->flag = hsearch_ret ? 0 : -1;

	if (copy_to_user((struct acm_mp_node *)args, mp_node,
			 sizeof(struct acm_mp_node))) {
		err = -EFAULT;
		goto do_cmd_search_ret;
	}
	pr_info("Search result = %d\n", mp_node->flag);

do_cmd_search_ret:
	kfree(mp_node);
	mp_node = NULL;

	if (IS_ENABLED(CONFIG_ACM_TIME_COST)) {
		getrawmonotonic(&end);
		pr_err("TIME_COST: start.tv_sec = %lu start.tv_nsec = %lu, end.tv_sec = %lu end.tv_nsec = %lu duraion = %lu\n",
		       start.tv_sec, start.tv_nsec, end.tv_sec, end.tv_nsec,
		       end.tv_nsec - start.tv_nsec);
	}
	return err;
}

static int do_cmd_add_dir(unsigned long args)
{
	int err = 0;
	struct acm_dnode *dir_node = NULL;
	struct acm_dnode *pos = NULL;
	struct acm_fwk_dir *fwk_dir = NULL;

	fwk_dir = kzalloc(sizeof(*fwk_dir), GFP_KERNEL);
	if (!fwk_dir)
		return -ENOMEM;

	if (copy_from_user(fwk_dir->dir, (struct acm_fwk_dir *)args,
			   sizeof(struct acm_fwk_dir))) {
		pr_err("Failed to copy dir from user space!\n");
		err = -EFAULT;
		goto add_dir_ret;
	}
	if (valid_len(fwk_dir->dir, ACM_DIR_MAX)) {
		pr_err("Failed to check the length of dir name!\n");
		err = -EINVAL;
		goto add_dir_ret;
	}
	fwk_dir->dir[ACM_DIR_MAX - 1] = '\0';

	dir_node = kzalloc(sizeof(*dir_node), GFP_KERNEL);
	if (!dir_node) {
		err = -ENOMEM;
		goto add_dir_ret;
	}

	memcpy(dir_node->dir, fwk_dir->dir, ACM_DIR_MAX - 1);
	dir_node->dir[ACM_DIR_MAX - 1] = '\0';

	/* Check whether dir is already in the acm_dir_list */
	spin_lock(&acm_dir_list.spinlock);
	list_for_each_entry(pos, &acm_dir_list.head, lnode) {
		if (strncasecmp(fwk_dir->dir, pos->dir,
				ACM_DIR_MAX - 1) == 0) {
			spin_unlock(&acm_dir_list.spinlock);
			pr_err("Dir is already in the dir list!\n");
			kfree(dir_node);
			goto add_dir_ret;
		}
	}

	err = acm_dir_list_add(&acm_dir_list.head, dir_node);
	spin_unlock(&acm_dir_list.spinlock);
	if (err < 0) {
		kfree(dir_node);
		goto add_dir_ret;
	}

	pr_info("Add a dir: %s\n", dir_node->dir);

add_dir_ret:
	kfree(fwk_dir);
	return err;
}

static int do_cmd_del_dir(unsigned long args)
{
	int err = 0;
	struct acm_dnode *n = NULL;
	struct acm_dnode *dir_node = NULL;
	struct acm_fwk_dir *fwk_dir = NULL;

	fwk_dir = kzalloc(sizeof(*fwk_dir), GFP_KERNEL);
	if (!fwk_dir)
		return -ENOMEM;

	if (copy_from_user(fwk_dir, (struct acm_fwk_dir *)args,
			   sizeof(struct acm_fwk_dir))) {
		pr_err("Failed to copy dir from user space!\n");
		err = -EFAULT;
		goto del_dir_ret;
	}
	if (valid_len(fwk_dir->dir, ACM_DIR_MAX)) {
		pr_err("Failed to check the length of dir name!\n");
		err = -EINVAL;
		goto del_dir_ret;
	}
	fwk_dir->dir[ACM_DIR_MAX - 1] = '\0';

	spin_lock(&acm_dir_list.spinlock);
	list_for_each_entry_safe(dir_node, n, &acm_dir_list.head, lnode) {
		if (strncasecmp(dir_node->dir, fwk_dir->dir,
				ACM_DIR_MAX - 1) == 0) {
			WARN_ON(acm_dir_list.nr_nodes < 1);
			list_del(&dir_node->lnode);
			acm_dir_list.nr_nodes--;
			spin_unlock(&acm_dir_list.spinlock);
			kfree(dir_node);
			dir_node = NULL;
			goto del_dir_ret;
		}
	}
	spin_unlock(&acm_dir_list.spinlock);

	pr_info("Dir not found!\n");

del_dir_ret:
	kfree(fwk_dir);
	return err;
}

#ifdef CONFIG_ACM_DSM
static int acm_dmd_add(struct list_head *head, struct acm_lnode *node)
{
	spin_lock(&acm_dmd_list.spinlock);
	if (acm_dmd_list.nr_nodes > ACM_DMD_LIST_MAX_NODES - 1) {
		spin_unlock(&acm_dmd_list.spinlock);
		pr_err("List was too long! Dropped a pkgname!\n");
		return -ENOSPC;
	}
	list_add_tail(&node->lnode, head);
	acm_dmd_list.nr_nodes++;
	spin_unlock(&acm_dmd_list.spinlock);
	return ACM_SUCCESS;
}

static int do_cmd_add_dsm(unsigned long args)
{
	int err = 0;
	struct acm_mp_node *mp_node = NULL;
	struct acm_lnode *new_dmd_node = NULL;

	mp_node = kzalloc(sizeof(*mp_node), GFP_KERNEL);
	if (!mp_node)
		return -ENOMEM;
	if (copy_from_user(mp_node, (struct acm_mp_node *)args,
			   sizeof(struct acm_mp_node))) {
		err = -EFAULT;
		goto do_cmd_add_dsm_ret;
	}

	new_dmd_node = kzalloc(sizeof(*new_dmd_node), GFP_KERNEL);
	if (!new_dmd_node) {
		err = -ENOMEM;
		goto do_cmd_add_dsm_ret;
	}

	memcpy(new_dmd_node->pkgname, mp_node->pkgname,
		ACM_PKGNAME_MAX - 1);
	new_dmd_node->pkgname[ACM_PKGNAME_MAX - 1] = '\0';

	memcpy(new_dmd_node->path, mp_node->path, ACM_PATH_MAX - 1);
	new_dmd_node->path[ACM_PATH_MAX - 1] = '\0';

	new_dmd_node->depth = DEPTH_INIT;
	new_dmd_node->file_type = mp_node->file_type;

	err = acm_dmd_add(&acm_dmd_list.head, new_dmd_node);
	if (err < 0) {
		kfree(new_dmd_node);
		goto do_cmd_add_dsm_ret;
	}
	if (likely(acm_dmd_task))
		complete(&acm_dmd_comp);

do_cmd_add_dsm_ret:
	kfree(mp_node);
	return err;
}
#endif

static long acm_ioctl(struct file *filp, unsigned int cmd, unsigned long args)
{
	int err = 0;

	if (acm_init_state) {
		pr_err("Access Control Module init failed! err = %ld\n",
		       acm_init_state);
		return -ENOTTY;
	}

	if (_IOC_TYPE(cmd) != ACM_MAGIC) {
		pr_err("Failed to check ACM_MAGIC!\n");
		return -EINVAL;
	}

	if (_IOC_NR(cmd) > ACM_CMD_MAXNR) {
		pr_err("Failed to check ACM_CMD_MAXNR!\n");
		return -EINVAL;
	}

	if (_IOC_DIR(cmd) & _IOC_READ)
		err = !access_ok(VERIFY_WRITE, (void *)args, _IOC_SIZE(cmd));
	else if (_IOC_DIR(cmd) & _IOC_WRITE)
		err = !access_ok(VERIFY_READ, (void *)args, _IOC_SIZE(cmd));

	if (err) {
		pr_err("Failed to check access permission!\n");
		return -EINVAL;
	}

	switch (cmd) {
	case ACM_ADD:
		err = do_cmd_add(args);
		break;
	case ACM_DEL:
		err = do_cmd_delete(args);
		break;
	case ACM_SEARCH:
		err = do_cmd_search(args);
		break;
	case ACM_ADD_DIR:
		err = do_cmd_add_dir(args);
		break;
	case ACM_DEL_DIR:
		err = do_cmd_del_dir(args);
		break;
#ifdef CONFIG_ACM_DSM
	case ACM_ADD_DSM:
		err = do_cmd_add_dsm(args);
		break;
#endif
	default:
		pr_err("Unknown command!\n");
		return -EINVAL;
	}

	return err;
}

static int set_path(struct acm_lnode *node, struct dentry *dentry)
{
	char *buffer = NULL;
	char *dentry_path = NULL;
	size_t path_len;

	buffer = kzalloc(ACM_PATH_MAX, GFP_KERNEL);
	if (!buffer)
		return -ENOMEM;
	dentry_path = dentry_path_raw(dentry, buffer, ACM_PATH_MAX);
	if (IS_ERR(dentry_path)) {
		kfree(buffer);
		pr_err("Failed to get path! err = %lu\n", PTR_ERR(dentry_path));
		return -EINVAL;
	}

	path_len = get_valid_strlen(dentry_path, ACM_PATH_MAX);
	memcpy(node->path, dentry_path, path_len);
	node->path[path_len] = '\0';
	kfree(buffer);
	return ACM_SUCCESS;
}

#ifdef CONFIG_ACM_DSM
static int do_get_path_error(struct acm_lnode *node, struct dentry *dentry)
{
	int i;
	int err;
	int depth;
	struct dentry *d[ERR_PATH_MAX_DENTRIES] = { NULL };

	for (i = 0; i < ERR_PATH_MAX_DENTRIES; i++)
		d[i] = dget(dentry);

	/*
	 * Find the root dentry of the current file system.The d[i] saves the
	 * top ERR_PATH_MAX_DENTRIES-1 dentries to the root dentry.
	 */
	depth = 0;
	while (!IS_ROOT(dentry)) {
		dput(d[0]);
		for (i = 0; i < ERR_PATH_MAX_DENTRIES - 1; i++)
			d[i] = d[i + 1];
		dentry = d[ERR_PATH_MAX_DENTRIES - 1] = dget_parent(dentry);
		depth++;
	}
	node->depth = depth;

	dentry = d[ERR_PATH_LAST_DENTRY];

	for (i = 0; i < ERR_PATH_MAX_DENTRIES; i++)
		dput(d[i]);

	dentry = dget(dentry);
	err = set_path(node, dentry);
	if (err) {
		dput(dentry);
		pr_err("Unknown error! err = %d\n", err);
		return -EINVAL;
	}
	dput(dentry);

	return ACM_SUCCESS;
}

static int delete_log_upload_dmd(const char *pkgname, struct dentry *dentry,
				int file_type, int op)
{
	int err;
	struct acm_lnode *new_dmd_node = NULL;

	new_dmd_node = kzalloc(sizeof(*new_dmd_node), GFP_NOFS);
	if (!new_dmd_node)
		return -ENOMEM;

	memcpy(new_dmd_node->pkgname, pkgname, ACM_PKGNAME_MAX - 1);
	new_dmd_node->pkgname[ACM_PKGNAME_MAX - 1] = '\0';

	new_dmd_node->depth = DEPTH_INIT;
	new_dmd_node->file_type = file_type;
	new_dmd_node->op = op;
	err = set_path(new_dmd_node, dentry);
	if (err) {
		pr_err("Failed to get full path! err = %d\n", err);
		err = do_get_path_error(new_dmd_node, dentry);
		if (err)
			pr_err("Failed to get path for dmd! err = %d\n", err);
	}

	/*
	 * Data in new_dmd_list will be uploaded to unrmd by acm_dmd_task, and
	 * then uploaded to dmd server.
	 *
	 * Note that error returned by acm_dmd_add is not fatal, and should not
	 * be regarded as an error that would block the deletion. So don't
	 * return it here.
	 */
	if (acm_dmd_add(&acm_dmd_list.head, new_dmd_node) < 0) {
		kfree(new_dmd_node);
		return err;
	}
	complete(&acm_dmd_comp);

	return err;
}
#endif

static int delete_log_upload_fwk(const char *pkgname, struct dentry *dentry)
{
	int err;
	struct acm_lnode *new_fwk_node = NULL;

	/* Data not found */
	new_fwk_node = kzalloc(sizeof(*new_fwk_node), GFP_NOFS);
	if (!new_fwk_node)
		return -ENOMEM;

	memcpy(new_fwk_node->pkgname, pkgname, ACM_PKGNAME_MAX - 1);
	new_fwk_node->pkgname[ACM_PKGNAME_MAX - 1] = '\0';

	err = set_path(new_fwk_node, dentry);
	if (err) {
		kfree(new_fwk_node);
		new_fwk_node = NULL;
		pr_err("Failed to get path for framework! err = %d\n", err);
		return err;
	}

	/* Data in new_fwk_list will be uploaded
	 * to framework by acm_fwk_task
	 */
	acm_fwk_add(&acm_fwk_list.head, new_fwk_node);
	wake_up_process(acm_fwk_task);

	return ACM_SUCCESS;
}

static int inquiry_delete_policy(char *pkgname, uid_t uid)
{
	struct acm_hnode *hsearch_ret = NULL;

	if (uid < UID_BOUNDARY)
		return DEL_ALLOWED;

	/*
	 * Search the whitelist for @pkgname. Return -ENODATA if @pkgname
	 * is not in the white list.
	 */
	spin_lock(&acm_hash.spinlock);
	hsearch_ret = acm_hsearch(acm_hash.head, pkgname);
	spin_unlock(&acm_hash.spinlock);
	if (!hsearch_ret)
		return -ENODATA;

	return DEL_ALLOWED;
}

/*
 * acm_search() - search the white list for a package name and collect
 *		  delete info to upload to DMD
 * @pkgname:   the package name to search
 * @dentry:    the file dentry to be deleted
 * @uid:       the uid of the task doing the delete operation
 * @file_type: an integer to represent different file types, defined
 *	       in fs/f2fs/namei.c
 *
 * Returns 0 if the package name is in the white list, -ENODATA if
 * the package name is not in the white list, and other values on error.
 */
int acm_search(char *pkgname, struct dentry *dentry, uid_t uid, int file_type,
	       int op)
{
	int ret = 0;
	int err = 0;
	struct timespec __maybe_unused start;
	struct timespec __maybe_unused end;

	if (IS_ENABLED(CONFIG_ACM_TIME_COST))
		getrawmonotonic(&start);
	if (acm_init_state) {
		pr_err("Access Control Module init failed! err = %ld\n",
		       acm_init_state);
		err = -EINVAL;
		goto acm_search_fail;
	}
	/* Parameter validity check */
	if (!pkgname) {
		pr_err("Package name was NULL!\n");
		err = -EINVAL;
		goto acm_search_fail;
	}
	if (valid_len(pkgname, ACM_PKGNAME_MAX)) {
		pr_err("Failed to check the length of package name!\n");
		err = -EINVAL;
		goto acm_search_fail;
	}
	pkgname[ACM_PKGNAME_MAX - 1] = '\0';

	if (op == ACM_OP_DEL) {
		ret = inquiry_delete_policy(pkgname, uid);
		if (ret != DEL_ALLOWED) {
			err = delete_log_upload_fwk(pkgname, dentry);
			/* We just do nothing but print an error message here,
			 * becasuse the return value is delete-not-allowed, so the
			 * file system won't delete the file, and data
			 * will be upload to dmd anyway.
			 */
			if (err)
				pr_err("Failed to upload to fwk! err = %d ret = %d\n", err, ret);
		}
	}

	if (op == ACM_OP_DEL_DMD)
		op = ACM_OP_DEL;
#ifdef CONFIG_ACM_DSM
	/* Data should be upload to dmd whether the file is allowed to
	 * be deleted or not.
	 */
	err = delete_log_upload_dmd(pkgname, dentry, file_type, op);
	if (err) {
		pr_err("Failed to upload to dmd! err = %d op = %d\n", err, op);
		goto acm_search_fail;
	}
#endif

	if (IS_ENABLED(CONFIG_ACM_TIME_COST)) {
		getrawmonotonic(&end);
		pr_err("TIME_COST: start.tv_sec = %lu start.tv_nsec = %lu end.tv_sec = %lu end.tv_nsec = %lu duraion = %lu\n",
		       start.tv_sec, start.tv_nsec, end.tv_sec,
		       end.tv_nsec, end.tv_nsec - start.tv_nsec);
	}

	return ret;

acm_search_fail:
	/* Just forget about calculating the time cost */
	return err;
}

#ifdef CONFIG_ACM_DSM
static char *remain_first_dname(char *path)
{
	unsigned long i;
	size_t len;
	char *pt;

	len = get_valid_strlen(path, ACM_PATH_MAX);

	pt = strstr(path, ".thumbnails");
	if (pt != NULL) {
		memset(pt + strlen(".thumbnails"), 0,
		       len - (size_t)(pt - path) - strlen(".thumbnails"));
		return path;
	}

	for (i = 0; i < len; i++) {
		if (*(path + i) == '/') {
			memset(path + i, 0, len - i);
			break;
		}
	}

	return path;
}

static void asterisk_path(char *path)
{
	unsigned long i;
	size_t len;

	len = get_valid_strlen(path, ACM_DNAME_MAX);

	for (i = 0; i < len; i++)
		*(path + i) = '*';
}

void remove_user_dir(struct acm_lnode *node)
{
	char *path = node->path;
	struct acm_dnode *dir_node = NULL;

	spin_lock(&acm_dir_list.spinlock);
	list_for_each_entry(dir_node, &acm_dir_list.head, lnode) {
		if (strncasecmp(path, dir_node->dir,
				strlen(dir_node->dir)) == 0) {
			spin_unlock(&acm_dir_list.spinlock);
			remain_first_dname(path);
			return;
		}

	}
	spin_unlock(&acm_dir_list.spinlock);

	remain_first_dname(path);

	asterisk_path(path);
}

static void set_dmd_uevent_env(struct acm_lnode *node)
{
	int idx;

	memset(&dsm_env, 0, sizeof(struct acm_env));
	snprintf(dsm_env.pkgname, sizeof(dsm_env.pkgname),
		 "DSM_PKGNAME=%s", node->pkgname);
	snprintf(dsm_env.path, sizeof(dsm_env.path),
		 "DSM_PATH=%s", node->path);
	snprintf(dsm_env.depth, sizeof(dsm_env.depth),
		 "DSM_DEPTH=%d", node->depth);
	snprintf(dsm_env.file_type, sizeof(dsm_env.file_type),
		 "DSM_FTYPE=%d", node->file_type);
	snprintf(dsm_env.nr, sizeof(dsm_env.nr),
		 "DSM_NR=%d", node->nr);
	snprintf(dsm_env.op, sizeof(dsm_env.op),
		 "DSM_OP=%d", node->op);

	idx = 0;
	dsm_env.envp[idx++] = dsm_env.pkgname;
	dsm_env.envp[idx++] = dsm_env.path;
	dsm_env.envp[idx++] = dsm_env.depth;
	dsm_env.envp[idx++] = dsm_env.file_type;
	dsm_env.envp[idx++] = dsm_env.nr;
	dsm_env.envp[idx++] = dsm_env.op;
	dsm_env.envp[idx] = NULL;
}

static void upload_delete_log(void)
{
	int i;
	int err = 0;

	for (i = 0; i < dmd_cache.count; i++) {
		set_dmd_uevent_env(&dmd_cache.cache[i]);
		err = kobject_uevent_env(&(acm_cdev->kobj), KOBJ_CHANGE,
					 dsm_env.envp);
		if (err)
			pr_err("Failed to send uevent!\n");
	}

	memset(&dmd_cache, 0, sizeof(struct acm_cache));

	/*
	 * Compiler optimization may remove the call to memset(),
	 * causing dmd_cache uncleaned, if dmd_cache is not accessed
	 * after memset(). So we access the count member after memset()
	 * to avoid this optimization.
	 */
	dmd_cache.count = 0;
}

/*
 * Return true if in the cache, false if NOT in the cache.
 */
static bool is_a_cache(struct acm_lnode *node, struct acm_lnode *cache_node)
{
	return (node->depth == cache_node->depth) &&
	       (node->op == cache_node->op) &&
	       (node->file_type == cache_node->file_type) &&
	       (strcmp(node->path, cache_node->path) == 0) &&
	       (strcmp(node->pkgname, cache_node->pkgname) == 0);
}

static void add_cache(struct acm_lnode *node)
{

	if (dmd_cache.count > MAX_CACHED_DELETE_LOG - 1)
		return;

	memcpy(&dmd_cache.cache[dmd_cache.count], node,
	       sizeof(struct acm_lnode));
	dmd_cache.cache[dmd_cache.count].nr++;
	dmd_cache.count++;
	pr_info("count = %d, nr = %d\n",
		dmd_cache.count, dmd_cache.cache[dmd_cache.count - 1].nr);
}

/*
 * Return true if in the cache, false if NOT in the cache.
 */
static bool is_cached(struct acm_lnode *node, int *idx)
{
	int i;

	for (i = 0; i < dmd_cache.count; i++) {
		if (is_a_cache(node, &dmd_cache.cache[i])) {
			*idx = i;
			return true;
		}
	}
	return false;
}

static void cache_log(struct acm_lnode *node)
{
	int which = -1;

	if (is_cached(node, &which)) {
		WARN_ON(which < 0 || which > MAX_CACHED_DELETE_LOG - 1);

		dmd_cache.cache[which].nr++;
	} else {
		add_cache(node);
	}
}

static int cal_nr_slashes(char *str)
{
	int i;
	int len;
	int nr_slashes = 0;

	len = get_valid_strlen(str, ACM_PATH_MAX);

	for (i = 0; i < len; i++) {
		if (*(str + i) == '/')
			nr_slashes++;
	}
	return nr_slashes;
}

static void do_remove_prefix(struct acm_lnode *node, int nr_slashes)
{
	unsigned long i;
	size_t len;
	int slashes = 0;
	char *p = node->path;

	len = get_valid_strlen(node->path, ACM_PATH_MAX);

	for (i = 0; i < len; i++) {
		if (*(p + i) == '/') {
			if (++slashes > nr_slashes) {
				i++;
				break;
			}
		}
	}

	if (i > len - 1) {
		pr_err("Invalid path syntax!\n");
		memset(node->path, 0, sizeof(node->path));
		memcpy(node->path, PATH_UNKNOWN, sizeof(PATH_UNKNOWN));
	} else {
		memcpy(node->path, p + i, len - i);
		memset(p + len - i, 0, i);
	}
}

/*
 * Remove the specific prefix of a path.
 *
 * If path doesn't have the prefix PATH_PREFIX_MEDIA or
 * PATH_PREFIX_STORAGE_EMULATED, then set path to PATH_UNKNOWN.
 */
static void remove_path_prefix(struct acm_lnode *node)
{
	int nr_slashes_in_prefix = 0;

	if (!strncmp(node->path, PATH_PREFIX_MEDIA,
		     strlen(PATH_PREFIX_MEDIA))) {
		nr_slashes_in_prefix = cal_nr_slashes(PATH_PREFIX_MEDIA);
	} else if (!strncmp(node->path, PATH_PREFIX_STORAGE_EMULATED,
			    strlen(PATH_PREFIX_STORAGE_EMULATED))) {
		nr_slashes_in_prefix =
			cal_nr_slashes(PATH_PREFIX_STORAGE_EMULATED);
	} else {
		pr_err("Invalid path prefix!\n");
		nr_slashes_in_prefix = ACM_PATH_MAX - 1;
	}

	if (node->depth != DEPTH_INIT) {
		node->depth = node->depth - nr_slashes_in_prefix;
		node->depth--;	  /* For user id */
		node->depth--;	  /* For control dir */
		node->depth--;	  /* For file name */
	}

	do_remove_prefix(node, nr_slashes_in_prefix + 1);

}

static void set_depth(struct acm_lnode *node)
{
	if (node->depth == DEPTH_INIT)
		node->depth = cal_nr_slashes(node->path) - 1;
}

static void process_delete_log(struct acm_lnode *node)
{
	remove_path_prefix(node);
	set_depth(node);
	remove_user_dir(node);
	cache_log(node);
}

static void process_and_upload_delete_log(struct list_head *list)
{
	struct acm_lnode *node = NULL;

	while (1) {
		if (list_empty(list))
			break;
		node = get_first_entry(list);

		process_delete_log(node);

		kfree(node);
		node = NULL;

		if (dmd_cache.count > MAX_CACHED_DELETE_LOG - 1)
			upload_delete_log();
	}
	if (dmd_cache.count > 0)
		upload_delete_log();
}

static int acm_dmd_loop(void *data)
{
	struct list_head list = LIST_HEAD_INIT(list);
	struct timespec __maybe_unused start;
	struct timespec __maybe_unused end;

	while (!kthread_should_stop()) {
		wait_for_completion(&acm_dmd_comp);
		msleep(DELETE_LOG_UPLOAD_INTERVAL_MS);
		spin_lock(&acm_dmd_list.spinlock);
		list_cut_position(&list, &acm_dmd_list.head,
				  acm_dmd_list.head.prev);
		acm_dmd_list.nr_nodes = 0;
		spin_unlock(&acm_dmd_list.spinlock);

		if (IS_ENABLED(CONFIG_ACM_TIME_COST))
			getrawmonotonic(&start);

		process_and_upload_delete_log(&list);

		if (IS_ENABLED(CONFIG_ACM_TIME_COST)) {
			getrawmonotonic(&end);
			pr_err("TIME_COST: start.tv_sec = %lu start.tv_nsec = %lu, end.tv_sec = %lu end.tv_nsec = %lu duraion = %lu\n",
			       start.tv_sec, start.tv_nsec,
			       end.tv_sec, end.tv_nsec,
			       end.tv_nsec - start.tv_nsec);
		}
	}

	return ACM_SUCCESS;
}

static void acm_cache_init(void)
{
	memset(&dmd_cache.cache, 0, sizeof(dmd_cache.cache));

	dmd_cache.count = 0;
}
#endif

static void set_fwk_uevent_env(struct acm_lnode *node)
{
	int idx;

	memset(&fwk_env, 0, sizeof(struct acm_env));
	snprintf(fwk_env.pkgname, sizeof(fwk_env.pkgname),
		 "PKGNAME=%s", node->pkgname);
	snprintf(fwk_env.path, sizeof(fwk_env.path),
		 "PIC_PATH=%s", node->path);

	idx = 0;
	fwk_env.envp[idx++] = fwk_env.pkgname;
	fwk_env.envp[idx++] = fwk_env.path;
	fwk_env.envp[idx] = NULL;
}

static void upload_data_to_fwk(void)
{
	int err = 0;
	struct acm_lnode *node = NULL;
	struct timespec __maybe_unused start;
	struct timespec __maybe_unused end;

	while (1) {

		spin_lock(&acm_fwk_list.spinlock);
		if (list_empty(&acm_fwk_list.head)) {
			spin_unlock(&acm_fwk_list.spinlock);
			break;
		}
		node = get_first_entry(&acm_fwk_list.head);
		spin_unlock(&acm_fwk_list.spinlock);

		if (IS_ENABLED(CONFIG_ACM_TIME_COST))
			getrawmonotonic(&start);
		set_fwk_uevent_env(node);

		err = kobject_uevent_env(&(acm_cdev->kobj), KOBJ_CHANGE,
			fwk_env.envp);
		if (err)
			pr_err("Failed to upload to fwk!\n");

		kfree(node);
		node = NULL;
		if (IS_ENABLED(CONFIG_ACM_TIME_COST)) {
			getrawmonotonic(&end);
			pr_err("TIME_COST: start.tv_sec = %lu start.tv_nsec = %lu, end.tv_sec = %lu end.tv_nsec = %lu duraion = %lu\n",
			       start.tv_sec, start.tv_nsec,
			       end.tv_sec, end.tv_nsec,
			       end.tv_nsec - start.tv_nsec);
		}
	}
}

static int acm_fwk_loop(void *data)
{
	while (!kthread_should_stop()) {

		upload_data_to_fwk();

		sleep_if_list_empty(&acm_fwk_list);
	}
	return ACM_SUCCESS;
}

static int acm_hash_init(void)
{
	int i;
	struct hlist_head *head = NULL;

	head = kzalloc(sizeof(*head) * ACM_HASHTBL_SZ, GFP_KERNEL);
	if (!head)
		return -ENOMEM;
	for (i = 0; i < ACM_HASHTBL_SZ; i++)
		INIT_HLIST_HEAD(&(head[i]));

	acm_hash.head = head;
	acm_hash.nr_nodes = 0;
	spin_lock_init(&acm_hash.spinlock);
	return ACM_SUCCESS;
}

static void acm_list_init(struct acm_list *list)
{
	INIT_LIST_HEAD(&list->head);
	list->nr_nodes = 0;
	spin_lock_init(&list->spinlock);
}

static int acm_task_init(void)
{
	long err = 0;

	/*
	 *Create the acm_fwk_loop task to asynchronously
	 * upload data to framework.
	 */
	acm_fwk_task = kthread_run(acm_fwk_loop, NULL, "acm_fwk_loop");
	if (IS_ERR(acm_fwk_task)) {
		err = PTR_ERR(acm_fwk_task);
		pr_err("Failed to create acm_fwk_task! err = %ld\n", err);
		return err;
	}

#ifdef CONFIG_ACM_DSM
	/*
	 *Create the acm_dmd_loop task to asynchronously
	 * upload data to dmd.
	 */
	acm_dmd_task = kthread_run(acm_dmd_loop, NULL, "acm_dmd_loop");
	if (IS_ERR(acm_dmd_task)) {
		err = PTR_ERR(acm_dmd_task);
		pr_err("Failed to create acm_dmd_task! err = %ld\n", err);
		return err;
	}
#endif

	return err;
}

static const struct file_operations acm_fops = {
	.owner = THIS_MODULE,
	.unlocked_ioctl = acm_ioctl,
};

static int acm_chr_dev_init(void)
{

	long err;

	/* Dynamiclly allocate a device number */
	err = alloc_chrdev_region(&acm_devno,
				  ACM_DEV_BASE_MINOR,
				  ACM_DEV_COUNT,
				  ACM_DEV_NAME);
	if (err) {
		pr_err("Failed to alloc device number! err = %ld\n", err);
		return err;
	}

	/* Initialize and add a cdev data structure to kernel */
	acm_cdev = cdev_alloc();
	if (!acm_cdev) {
		err = -ENOMEM;
		pr_err("Failed to alloc memory for cdev! err = %ld\n", err);
		goto free_devno;
	}
	acm_cdev->owner = THIS_MODULE;
	acm_cdev->ops = &acm_fops;
	err = cdev_add(acm_cdev, acm_devno, ACM_DEV_COUNT);
	if (err) {
		pr_err("Failed to register cdev! err = %ld\n", err);
		goto free_cdev;
	}

	/* Dynamiclly create a device file */
	acm_class = class_create(THIS_MODULE, ACM_DEV_NAME);
	if (IS_ERR(acm_class)) {
		err = PTR_ERR(acm_class);
		pr_err("Failed to create a class! err = %ld\n", err);
		goto free_cdev;
	}
	acm_device = device_create(acm_class, NULL, acm_devno, NULL,
				   ACM_DEV_NAME);
	if (IS_ERR(acm_device)) {
		err = PTR_ERR(acm_device);
		pr_err("Failed to create a class! err = %ld\n", err);
		goto free_class;
	}

	/* Initialize uevent stuff */
	acm_kset = kset_create_and_add(ACM_DEV_NAME, NULL, kernel_kobj);
	if (!acm_kset) {
		err = -ENOMEM;
		pr_err("Failed to create kset! err = %ld\n", err);
		goto free_device;
	}
	acm_cdev->kobj.kset = acm_kset;
	acm_cdev->kobj.uevent_suppress = 0;
	err = kobject_add(&(acm_cdev->kobj),
			  &(acm_kset->kobj),
			  "acm_cdev_kobj");
	if (err) {
		kobject_put(&(acm_cdev->kobj));
		pr_err("Failed to add kobject to kernel! err = %ld\n", err);
		goto free_kset;
	}

	pr_info("Initialize acm character device succeed!\n");
	return err;

free_kset:
	kset_unregister(acm_kset);
free_device:
	device_destroy(acm_class, acm_devno);
free_class:
	class_destroy(acm_class);
free_cdev:
	cdev_del(acm_cdev);
free_devno:
	unregister_chrdev_region(acm_devno, ACM_DEV_COUNT);

	pr_err("Failed to init acm character device! err = %ld\n", err);
	return err;
}

static int __init acm_init(void)
{

	long err = 0;

	/* Initialize hash table */
	err = acm_hash_init();
	if (err) {
		pr_err("Failed to initialize hash table! err = %ld\n", err);
		goto fail_hash;
	}

	acm_list_init(&acm_dir_list);
	acm_list_init(&acm_fwk_list);
#ifdef CONFIG_ACM_DSM
	acm_list_init(&acm_dmd_list);
	acm_cache_init();
#endif

	err = acm_task_init();
	if (err) {
		pr_err("Failed to initialize main task! err = %ld\n", err);
		goto fail_task;
	}

	/* Initialize acm character device */
	err = acm_chr_dev_init();
	if (err) {
		pr_err("Failed to initialize acm chrdev! err = %ld\n", err);
		goto fail_task;
	}

	pr_info("Initialize ACM moudule succeed!\n");

	acm_init_state = err;
	return err;

fail_task:
	kfree(acm_hash.head);
	acm_hash.head = NULL;
fail_hash:
	acm_init_state = err;
	return err;
}

void acm_hash_cleanup(struct hlist_head *hash)
{
	int i;
	struct hlist_head *phead = NULL;
	struct acm_hnode *pnode = NULL;

	spin_lock(&acm_hash.spinlock);
	for (i = 0; i < ACM_HASHTBL_SZ; i++) {
		phead = &hash[i];
		while (!hlist_empty(phead)) {
			pnode = hlist_entry(phead->first, struct acm_hnode,
					    hnode);
			hlist_del(&pnode->hnode);
			kfree(pnode);
			pnode = NULL;
		}
	}
	acm_hash.nr_nodes = 0;
	spin_unlock(&acm_hash.spinlock);
}

void acm_list_cleanup(struct acm_list *list)
{
	struct acm_lnode *node = NULL;
	struct list_head *head = NULL, *pos = NULL;

	spin_lock(&list->spinlock);
	head = &list->head;
	while (!list_empty(head)) {
		pos = head->next;
		node = (struct acm_lnode *)list_entry(pos, struct acm_lnode,
						      lnode);
		list_del(pos);
		kfree(node);
		node = NULL;
	}
	list->nr_nodes = 0;
	spin_unlock(&list->spinlock);
}

static void __exit acm_exit(void)
{
	device_destroy(acm_class, acm_devno);
	class_destroy(acm_class);
	cdev_del(acm_cdev);
	unregister_chrdev_region(acm_devno, ACM_DEV_COUNT);
	kset_unregister(acm_kset);

	acm_hash_cleanup(acm_hash.head);
	acm_list_cleanup(&acm_dir_list);

	pr_info("Exited from ACM module.\n");
}

MODULE_LICENSE("GPL");
module_init(acm_init);
module_exit(acm_exit);

#ifdef CONFIG_ACM_DEBUG
#include "acm_test.c"
#endif
