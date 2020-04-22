
#include "inc/proc_mgmt/hwaa_proc_mgmt.h"
#include <linux/string.h>
#include <linux/sched.h>
#include <linux/cred.h>
#include <linux/slab.h>
#include "inc/base/hwaa_utils.h"
#include "inc/data/hwaa_data.h"
#include "inc/data/hwaa_trusted_pids.h"
#include "inc/data/hwaa_running_apps.h"
#include "inc/policy/hwaa_policy.h"
#include "inc/proc_mgmt/hwaa_proc_mgmt.h"
#include "inc/proc_mgmt/hwaa_trusted_zygote.h"

static void hwaa_proc_mgmt_pids_insert(uid_t uid, pid_t pid)
{
	if (hwaa_trusted_pids_insert(HWAA_TRUSTED_APP, pid)) { // err case
		hwaa_pr_err("Failed to insert trusted app! (uid:%d, pid:%d)",
			uid, pid);
		if (hwaa_running_apps_remove_pid(uid, pid))
			hwaa_running_apps_delete(uid);
	}
}

void hwaa_proc_mgmt_on_caps_setuid(struct task_struct *tsk,
	struct cred *new_cred)
{
	uid_t uid;
	pid_t pid;
	pid_t ppid;

	if (!tsk)
		return;
	uid = new_cred->uid.val;
	pid = tsk->tgid;
	ppid = tsk->real_parent->tgid;
	hwaa_data_write_lock();
	if (hwaa_trusted_pids_exists(HWAA_TRUSTED_ZYGOTE, ppid) &&
		(hwaa_packages_exists(uid)) &&
		hwaa_trusted_zygote_has_exe_path(pid)) {
		if (hwaa_running_apps_insert(uid, pid)) {
			hwaa_pr_err("fail insert running app! uid:%d, pid:%d",
				uid, pid);
			goto unlock;
		}
		hwaa_proc_mgmt_pids_insert(uid, pid);
	}
unlock:
	hwaa_data_write_unlock();
}

/*
 * This function check and insert pid ppid uid into trusted map when fork
 */
void hwaa_proc_mgmt_on_task_forked(struct task_struct *tsk)
{
	uid_t uid;
	pid_t pid;
	pid_t ppid;

	if (!tsk)
		return;
	uid = tsk->real_cred->uid.val;
	pid = tsk->tgid;
	ppid = tsk->real_parent->tgid;
	hwaa_data_write_lock();
	// replace hwaa_packages_get_pinfo by hwaa_packages_exists
	if (hwaa_trusted_pids_exists(HWAA_TRUSTED_APP, ppid) &&
		hwaa_packages_exists(uid)) {
		if (hwaa_running_apps_add_pid(uid, pid)) {
			hwaa_pr_err("Fail insert running app! uid:%d, pid:%d",
				uid, pid);
			goto unlock;
		}
		hwaa_proc_mgmt_pids_insert(uid, pid);
	}
unlock:
	hwaa_data_write_unlock();
}

void hwaa_proc_mgmt_on_task_exit(struct task_struct *tsk)
{
	bool app_pid = false;
	uid_t uid;
	pid_t pid;

	if (!tsk)
		return;
	uid = tsk->real_cred->uid.val;
	pid = tsk->pid;
	hwaa_data_write_lock();
	if (hwaa_trusted_pids_exists(HWAA_TRUSTED_APP, pid)) {
		hwaa_trusted_pids_delete(HWAA_TRUSTED_APP, pid);
		app_pid = true;
	} else if (hwaa_trusted_pids_exists(HWAA_TRUSTED_ZYGOTE, pid)) {
		hwaa_trusted_pids_delete(HWAA_TRUSTED_ZYGOTE, pid);
	}

	if (app_pid) {
		if (hwaa_running_apps_remove_pid(uid, pid))
			hwaa_running_apps_delete(uid);
	}
	hwaa_data_write_unlock();
}

/*
 * This function check and insert pid ppid uid into trusted map when pre exec
 */
s32 hwaa_proc_mgmt_pre_execve(const s8 *exe_path)
{
	s32 exe_index = hwaa_trusted_zygote_lookup(exe_path);
	return (exe_index < 0) ? -EINVAL : 0;
}

void hwaa_proc_mgmt_post_execve(pid_t pid)
{
	hwaa_data_write_lock();
	if (hwaa_trusted_pids_exists(HWAA_TRUSTED_ZYGOTE, pid)) {
		hwaa_pr_warn("Zygote pid:%d is already in trusted zygotes!",
			pid);
		hwaa_trusted_pids_delete(HWAA_TRUSTED_ZYGOTE, pid);
	} else {
		hwaa_trusted_pids_insert(HWAA_TRUSTED_ZYGOTE, pid);
	}
	hwaa_data_write_unlock();
}
