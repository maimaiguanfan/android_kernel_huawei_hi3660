/*
 * check_root proc and stp trace log save
 *
 * Copyright (c) 2001-2017, Huawei Tech. Co., Ltd., Zhebo Zhang <zhangzhebo at huawei.com>
 * Copyright (C) 2018 Huawei Tech. Co., Ltd., Ningyu Wang <wangningyu at huawei.com>
 *
 */

#include <linux/fs.h>
#include <linux/syscalls.h>
#include <linux/time.h>
#include <linux/uaccess.h>
#include <linux/rtc.h>
#include <linux/statfs.h>
#include <linux/vmalloc.h>
#include <linux/stacktrace.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include<chipset_common/security/check_root.h>

#define ANDROID_THIRD_PART_APK_UID 10000
#define AID_SHELL		   2000
#define STP_LOG_DIR  "/log/stp"
#define STP_LOG_PATH_LEN  64
#define STP_LOG_DIR_MODE  0775
#define STP_LOG_FILE_MODE  0664
#define STP_LOG_SIZE  1024
#define STP_STACK_DEPTH  10

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

static char* get_current_asctime(void)
{
	struct timeval tv = {0};
	unsigned long long rtc_time;
	struct rtc_time tm;
	static char asctime[32];

	/*1.get rtc timer*/
	do_gettimeofday(&tv);
	rtc_time = (unsigned long long)tv.tv_sec;

	/*2.convert rtc to asctime*/
	memset((void *)asctime, 0, sizeof(asctime));
	memset((void *)&tm, 0, sizeof(struct rtc_time));
	rtc_time_to_tm(rtc_time, &tm);
	snprintf(asctime, sizeof(asctime) - 1, "%04d%02d%02d%02d%02d%02d",
			tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
			tm.tm_hour, tm.tm_min, tm.tm_sec);

	return asctime;
}

static void stp_print_trace_info(char *buf, size_t len, const char *name)
{
	int ret;
	unsigned long entries[STP_STACK_DEPTH];
	struct stack_trace trace = {
		.nr_entries = 0,
		.entries = entries,
		.max_entries = STP_STACK_DEPTH,
		.skip = 0
	};

	if (!buf || !len) {
		return;
	}

	ret = snprintf(buf, len,
			"{\"name\":\"%s\",\"detail\":\"time:%s,pid:%d,pcomm:%.20s,tgid:%d,tgcomm:%.20s,stack:",
			name, get_current_asctime(), current->pid, current->comm,
			current->tgid, current->group_leader->comm);
	if (ret >= len) {
		goto out;
	}

	save_stack_trace(&trace);
	ret += snprint_stack_trace(buf + ret, len - ret, &trace, 0);
	if (ret >= len) {
		goto out;
	}
	snprintf(buf + ret, len - ret, "\"}\n");

out:
	buf[len - 1] = '\0';
	return;
}

void stp_save_trace_log(const char *name)
{
	mm_segment_t old_fs;
	int fd = -1;
	char *buf = NULL;
	char path[STP_LOG_PATH_LEN] = {0};
	size_t len, written;

	/* judge irq is comment out for all calls of stp_save_trace_log should be
	followed by BUG or VENDOR_EXCEPTION */
	/* if (in_irq() || in_softirq() || irqs_disabled())
	{
		return;
	} */

	old_fs = get_fs();
	set_fs(KERNEL_DS);

	if (sys_access(STP_LOG_DIR, 0) != 0) {
		if (sys_mkdir(STP_LOG_DIR, STP_LOG_DIR_MODE) < 0) {
			pr_err("create stp log dir failed!\n");
			goto out;
		}
	}

	(void)snprintf(path, sizeof(path) - 1, "%s/%s.log", STP_LOG_DIR, name);
	fd = sys_open(path, O_CREAT | O_WRONLY | O_APPEND, STP_LOG_FILE_MODE);
	if (fd < 0) {
		pr_err("open file %s failed! fd = %d\n", path, fd);
		goto out;
	}

	buf = (char *)vmalloc(STP_LOG_SIZE);
	if (!buf) {
		pr_err("vmalloc stp log buffer failed\n");
		goto nomem;
	}

	stp_print_trace_info(buf, STP_LOG_SIZE, name);
	pr_err("stp trace log: %s", buf);
	len = strlen((char *)buf);
	written = sys_write(fd, buf, len);
	if (written != len) {
		pr_err("write %s log failed! %ld/%ld\n", name, written, len);
	}

	vfree(buf);
	sys_sync();

nomem:
	sys_close(fd);

out:
	set_fs(old_fs);
}

module_init(proc_checkroot_init);
module_exit(proc_checkroot_exit);
