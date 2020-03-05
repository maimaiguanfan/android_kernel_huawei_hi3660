/*
 * check_root.c
 *
 * the check_root.c file for creatting check_root proc file
 *
 * Copyright (c) 2001-2021, Huawei Tech. Co., Ltd. All rights reserved.
 *
 * Zhangzhebo <zhangzhebo@huawei.com>
 *
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include<chipset_common/security/check_root.h>

#define ANDROID_THIRD_PART_APK_UID 10000
#define AID_SHELL		   2000

static struct checkroot_ref_cnt checkroot_ref;

uint get_setids_state(void)
{
	unsigned int ids = 0;

	if (checkroot_ref.setuid)
		ids |= CHECKROOT_SETUID_FLAG;
	if (checkroot_ref.setgid)
		ids |= CHECKROOT_SETGID_FLAG;
	if (checkroot_ref.setresuid)
		ids |= CHECKROOT_SETRESUID_FLAG;
	if (checkroot_ref.setresgid)
		ids |= CHECKROOT_SETRESGID_FLAG;

	return ids;
}

/* uncomment this for force stop setXid */
/* #define CONFIG_CHECKROOT_FORCE_STOP */

static int checkroot_risk_id(int curr_id, int flag)
{
	const struct cred *now;

	now = current_cred();

	if (curr_id < ANDROID_THIRD_PART_APK_UID && curr_id != AID_SHELL) {
		return 0;
	}
	pr_emerg("check_root: Uid %d, Gid %d, try to Privilege Escalate\n",
			now->uid.val, now->gid.val);

#ifdef CONFIG_CHECKROOT_FORCE_STOP
	if (curr_id >= ANDROID_THIRD_PART_APK_UID) {
		force_sig(SIGKILL, current);
		return -1;
	}
#endif
	if (flag & CHECKROOT_SETUID_FLAG) {
		checkroot_ref.setuid++;
	}
	if (flag & CHECKROOT_SETGID_FLAG) {
		checkroot_ref.setgid++;
	}
	if (flag & CHECKROOT_SETRESUID_FLAG) {
		checkroot_ref.setresuid++;
	}
	if (flag & CHECKROOT_SETRESGID_FLAG) {
		checkroot_ref.setresgid++;
	}
	return 0;
}

int checkroot_setuid(uid_t uid)
{
	return checkroot_risk_id((int)uid, CHECKROOT_SETUID_FLAG);
}

int checkroot_setgid(gid_t gid)
{
	return checkroot_risk_id((int)gid, CHECKROOT_SETGID_FLAG);
}

int checkroot_setresuid(uid_t uid)
{
	return checkroot_risk_id((int)uid, CHECKROOT_SETRESUID_FLAG);
}

int checkroot_setresgid(gid_t gid)
{
	return checkroot_risk_id((int)gid, CHECKROOT_SETRESGID_FLAG);
}

static int __init proc_checkroot_init(void)
{
	(void)memset(&checkroot_ref, 0, sizeof(checkroot_ref));
	return 0;
}

static void __exit proc_checkroot_exit(void)
{
	return;
}

module_init(proc_checkroot_init);
module_exit(proc_checkroot_exit);
