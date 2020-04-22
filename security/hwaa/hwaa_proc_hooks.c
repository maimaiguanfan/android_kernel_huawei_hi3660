
#include "huawei_platform/hwaa/hwaa_proc_hooks.h"
#include <linux/spinlock.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/rwsem.h>


static DECLARE_RWSEM(g_proc_callbacks_lock);

static s32 default_pre_execve(const s8 *exe_path)
{
	return -EINVAL;
}

static void default_post_execve(pid_t pid)
{

}

static void default_on_task_forked(struct task_struct *tsk)
{

}

static void default_on_task_exit(struct task_struct *tsk)
{

}

static void default_on_caps_setuid(struct task_struct *tsk,
	struct cred *new_cred)
{

}

static struct hwaa_proc_callbacks_t g_proc_callbacks = {
	.pre_execve = default_pre_execve,
	.post_execve = default_post_execve,
	.on_task_forked = default_on_task_forked,
	.on_task_exit = default_on_task_exit,
	.on_caps_setuid = default_on_caps_setuid
};

s32 hwaa_proc_pre_execve(const s8 *exe_path)
{
	s32 ret;
	down_read(&g_proc_callbacks_lock);
	ret = g_proc_callbacks.pre_execve(exe_path);
	up_read(&g_proc_callbacks_lock);
	return ret;
}

void hwaa_proc_post_execve(pid_t pid)
{
	down_read(&g_proc_callbacks_lock);
	g_proc_callbacks.post_execve(pid);
	up_read(&g_proc_callbacks_lock);
}

void hwaa_proc_on_task_forked(struct task_struct *tsk)
{
	down_read(&g_proc_callbacks_lock);
	g_proc_callbacks.on_task_forked(tsk);
	up_read(&g_proc_callbacks_lock);
}

void hwaa_proc_on_task_exit(struct task_struct *tsk)
{
	down_read(&g_proc_callbacks_lock);
	g_proc_callbacks.on_task_exit(tsk);
	up_read(&g_proc_callbacks_lock);
}

void hwaa_proc_on_caps_setuid(struct task_struct *tsk, struct cred *new_cred)
{
	down_read(&g_proc_callbacks_lock);
	g_proc_callbacks.on_caps_setuid(tsk, new_cred);
	up_read(&g_proc_callbacks_lock);
}

void hwaa_register_proc_callbacks(struct hwaa_proc_callbacks_t *callbacks)
{
	down_write(&g_proc_callbacks_lock);
	if (callbacks) {
		if (callbacks->pre_execve)
			g_proc_callbacks.pre_execve = callbacks->pre_execve;
		if (callbacks->post_execve)
			g_proc_callbacks.post_execve = callbacks->post_execve;
		if (callbacks->on_task_forked) {
			g_proc_callbacks.on_task_forked =
				callbacks->on_task_forked;
		}
		if (callbacks->on_task_exit)
			g_proc_callbacks.on_task_exit = callbacks->on_task_exit;
		if (callbacks->on_caps_setuid) {
			g_proc_callbacks.on_caps_setuid =
				callbacks->on_caps_setuid;
		}
	}
	up_write(&g_proc_callbacks_lock);
}
EXPORT_SYMBOL(hwaa_register_proc_callbacks);

void hwaa_unregister_proc_callbacks(void)
{
	down_write(&g_proc_callbacks_lock);
	g_proc_callbacks.pre_execve = default_pre_execve;
	g_proc_callbacks.post_execve = default_post_execve;
	g_proc_callbacks.on_task_forked = default_on_task_forked;
	g_proc_callbacks.on_task_exit = default_on_task_exit;
	g_proc_callbacks.on_caps_setuid = default_on_caps_setuid;
	up_write(&g_proc_callbacks_lock);
}
EXPORT_SYMBOL(hwaa_unregister_proc_callbacks);
