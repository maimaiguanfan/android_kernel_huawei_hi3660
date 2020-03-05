/*
 * hw_rscan_proc.c
 *
 * the hw_rscan_proc.c for proc file create and destroy
 *
 * likun <quentin.lee@huawei.com>
 *
 * Copyright (c) 2001-2021, Huawei Tech. Co., Ltd. All rights reserved.
 */

#include "./include/hw_rscan_proc.h"

static const char *TAG = "hw_rscan_proc";
static struct proc_dir_entry *proc_entry;

static const umode_t FILE_CREAT_RO_MODE = 0440;
static const kuid_t ROOT_UID =  KUIDT_INIT((uid_t)0);
static const kgid_t SYSTEM_GID = KGIDT_INIT((gid_t)1000);

#ifdef CONFIG_HUAWEI_EIMA
static int has_trigger_flag = 0;
extern int eima_baselining_trigger(void);
#endif

static int rscan_proc_show(struct seq_file *m, void *v)
{
	int ret = 0;

#ifdef CONFIG_HUAWEI_EIMA
	/* EIMA baselineling trigger */
	if( 0 == has_trigger_flag) {
		ret = eima_baselining_trigger();
		if(0 == ret ) {
			has_trigger_flag = 1;
		}
	}
#endif

	seq_printf(m, "%d", ret);
	return 0;
}

static int rscan_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, rscan_proc_show, NULL);
}

static const struct file_operations rscan_proc_fops = {
	.open		= rscan_proc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

int rscan_proc_init(void)
{
	int ret = 0;

	proc_entry = proc_create("root_scan", FILE_CREAT_RO_MODE, NULL,
							&rscan_proc_fops);

	if (proc_entry == NULL) {
		RSLogError(TAG, "proc_entry create is failed");
		return -ENOMEM;
	}

	/* set proc file gid to system gid */
	proc_set_user(proc_entry, ROOT_UID, SYSTEM_GID);

	RSLogTrace(TAG, "proc_entry init success");
	return ret;
}

void rscan_proc_deinit(void)
{
	remove_proc_entry("root_scan", NULL);
	RSLogTrace(TAG, "proc_entry cleanup success");
}
