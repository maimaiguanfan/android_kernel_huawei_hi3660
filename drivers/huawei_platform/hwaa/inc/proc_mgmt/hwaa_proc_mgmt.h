
#ifndef _HWAA_PROC_MGMT_H
#define _HWAA_PROC_MGMT_H

#include <linux/fs.h>
#include <linux/sched.h>

s32 hwaa_proc_mgmt_pre_execve(const s8 *exe_path);

/*
 * This function check and insert pid ppid uid into
 * trusted map when post exec
 */
void hwaa_proc_mgmt_post_execve(pid_t pid);

void hwaa_proc_mgmt_on_caps_setuid(struct task_struct *tsk,
	struct cred *new_cred);

void hwaa_proc_mgmt_on_task_forked(struct task_struct *tsk);

void hwaa_proc_mgmt_on_task_exit(struct task_struct *tsk);
#endif
