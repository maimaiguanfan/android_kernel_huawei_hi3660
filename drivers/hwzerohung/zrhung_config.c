/**********************************************************
 * Filename:    zrhung_config.c
 *
 * Discription: kernel configuration implementaion of zerohung
 *
 * Copyright: (C) 2017 huawei.
 *
 * Author: zhaochenxiao(00344580) zhangliang(00175161)
 *
**********************************************************/
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/security.h>
#include <asm/current.h>
#include <linux/fs.h>
#include <linux/gfp.h>
#include <linux/syscalls.h>
#include <linux/mman.h>
#include <linux/vmalloc.h>
#include <linux/mm.h>
#include <asm/cacheflush.h>
#include <linux/moduleloader.h>

#include "chipset_common/hwzrhung/zrhung.h"

#define SCONTEXT "u:r:logserver:s0"
#define HCFG_VAL_SIZE_MAX ((ZRHUNG_CFG_VAL_LEN_MAX+1)*ZRHUNG_CFG_ENTRY_NUM)
static const uint32_t HCFG_FEATURE_VERSION = 1;
#define NOT_SUPPORT	-2
#define NO_CONFIG	-1
#define NOT_READY	1

struct hcfg_entry {
	uint32_t offset;
	uint32_t valid:1;
};

struct hcfg_table_version {
	uint64_t len;
	struct hcfg_entry entry[ZRHUNG_CFG_ENTRY_NUM];
	char data[0];
};

struct hcfg_table {
	uint64_t len;
	struct hcfg_entry *entry;
	char *data;
};

struct hcfg_val {
	uint64_t wp;
	char data[ZRHUNG_CFG_VAL_LEN_MAX];
};

struct hcfg_ctx {
	struct hcfg_table table;
	unsigned long mem_size;
	int flag;
	void *user_table;
	unsigned long cfg_feature;
	unsigned long entry_num;
};

static DEFINE_SPINLOCK(lock);
static struct hcfg_ctx ctx;

int zrhung_is_id_valid(short wp_id)
{
    if (wp_id <= ZRHUNG_WP_NONE ||
        (wp_id >= ZRHUNG_WP_MAX && wp_id <= APPEYE_NONE) ||
        (wp_id >= APPEYE_MAX && wp_id <= ZRHUNG_EVENT_NONE) ||
        wp_id == XCOLLIE_FWK_SERVICE ||
        wp_id >= ZRHUNG_XCOLLIE_MAX) {
        return -EINVAL;
    }
    return 0;
}

int hcfgk_set_cfg(struct file *file, void __user*arg)
{
	int ret;
	uint64_t len;
	uint64_t mem_size;
	void* user_table = NULL;
	void* tmp = NULL;
	uint64_t table_len = 0;
	uint64_t entry_num = 0;
	struct hcfg_table_version *t = NULL;

	if(!arg)
		return -EINVAL;

	ret = copy_from_user(&len, arg, sizeof(len));
	if(ret) {
		printk(KERN_ERR "copy hung config table from user failed.\n");
		return ret;
	}
	if(len > HCFG_VAL_SIZE_MAX || len <= 0)
		return -EINVAL;

	spin_lock(&lock);

	if (ctx.cfg_feature != HCFG_FEATURE_VERSION) {
		spin_unlock(&lock);
		pr_err("cfg_feature is invalid\n");
		return -EINVAL;
	}
	table_len = sizeof(struct hcfg_table_version);
	entry_num = ZRHUNG_CFG_ENTRY_NUM;

	spin_unlock(&lock);

	mem_size = PAGE_ALIGN(table_len+len);

	user_table = vmalloc(mem_size);
	if(!user_table) {
		printk(KERN_ERR "Alloc hung config table failed.\n");
		return -ENOMEM;
	}
	memset(user_table, 0, mem_size);

	ret = copy_from_user(user_table, arg, table_len+len);
	if(ret) {
		printk(KERN_ERR "copy hung config table from user failed.\n");
		vfree(user_table);
		return ret;
	}

	spin_lock(&lock);

	tmp = ctx.user_table;
	ctx.user_table = user_table;
	user_table = tmp;
	ctx.mem_size = mem_size;
	ctx.entry_num = entry_num;

	/*
	 * init table entry
	 */
	if (ctx.cfg_feature != HCFG_FEATURE_VERSION) {
		spin_unlock(&lock);
		pr_err("cfg_feature is invalid\n");
		vfree(user_table);
		return -EINVAL;
	}

	t = ctx.user_table;
	ctx.table.entry = t->entry;
	ctx.table.data = t->data;

	/*
	 * make sure last byte in data is 0 terminated
	 */
	ctx.table.len = len;
	ctx.table.data[len-1] = '\0';

	spin_unlock(&lock);

	if(user_table != NULL) {
		vfree(user_table);
	}

	return ret;
}

int zrhung_get_config(zrhung_wp_id wp, char *data, uint32_t maxlen)
{
	int ret = NOT_READY;
	long long len = 0;
	short entry_id = 0;

	entry_id = ZRHUNG_WP_TO_ENTRY(wp);

	spin_lock(&lock);

	if(!ctx.table.entry || !ctx.table.data || (zrhung_is_id_valid(wp) == 0 && ctx.flag == 0))
		goto out;

	if(!data || entry_id >= ctx.entry_num || maxlen == 0) {
                ret = -EINVAL;
                goto out;
        }

	if(!ctx.table.entry[entry_id].valid) {
		ret = NO_CONFIG;
		goto out;
	}

	if(ctx.table.entry[entry_id].offset >= ctx.table.len) {
		ret = -EINVAL;
		goto out;
	}

	len = ctx.table.len - ctx.table.entry[entry_id].offset;
	if(len <= 0) {
		ret = -EINVAL;
		goto out;
	}

	strncpy(data, ctx.table.data+ctx.table.entry[entry_id].offset, len>maxlen-1?maxlen-1:len);
	data[maxlen-1] = '\0';
	ret = 0;

out:
	spin_unlock(&lock);
	return ret;
}
EXPORT_SYMBOL(zrhung_get_config);

int hcfgk_ioctl_get_cfg(struct file *file, void __user*arg)
{
	struct hcfg_val val;
	int ret;

	if(!arg)
		return -EINVAL;

	memset(&val, 0, sizeof(val));
	if(copy_from_user(&val.wp, arg, sizeof(val.wp))) {
		printk(KERN_ERR "Get WP id from user failed.\n");
		return NOT_SUPPORT;
	}

	ret = zrhung_get_config(val.wp, val.data, sizeof(val.data));
	if(!ret && copy_to_user(arg, &val, sizeof(val))) {
		printk(KERN_ERR "Failed to copy hung config val to user.\n");
		ret = -EFAULT;
	}

	return ret;
}

int hcfgk_set_cfg_flag(struct file *file, void __user*arg)
{
	uint32_t flag;

	if(!arg)
		return -EINVAL;

	if(copy_from_user(&flag, arg, sizeof(flag))) {
		printk(KERN_ERR "Copy Hung config flag from user failed.\n");
		return -EFAULT;
	}

	printk(KERN_DEBUG "set hcfg flag: %d\n", ctx.flag);

	spin_lock(&lock);

	if(flag > 0)
		ctx.flag = flag;

	spin_unlock(&lock);

	return 0;
}

int hcfgk_get_cfg_flag(struct file *file, void __user*arg)
{
	if(!arg)
		return -EINVAL;

	printk(KERN_DEBUG "get hcfg flag: %d\n", ctx.flag);

	spin_lock(&lock);

	if(copy_to_user(arg, &ctx.flag, sizeof(ctx.flag))) {
		spin_unlock(&lock);
		printk(KERN_ERR "Failed to copy hung config flag to user.\n");
		return -EFAULT;
	}

	spin_unlock(&lock);

	return 0;
}

int hcfgk_set_feature(struct file *file, void __user *arg)
{
	uint32_t feature;

	if(copy_from_user(&feature, arg, sizeof(feature))) {
		printk(KERN_ERR "Copy Hung config flag from user failed.\n");
		return -EFAULT;
	}

	spin_lock(&lock);

	ctx.cfg_feature = feature;

	spin_unlock(&lock);

	return 0;
}
