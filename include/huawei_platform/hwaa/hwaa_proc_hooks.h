
#ifndef _HWAA_PROC_HOOKS_H
#define _HWAA_PROC_HOOKS_H

#include <linux/sched.h>
#include <linux/fs.h>

/* process management callback functions */
struct hwaa_proc_callbacks_t {
	s32 (*pre_execve) (const s8 *exe_path);
	void (*post_execve) (pid_t pid);
	void (*on_task_forked) (struct task_struct *tsk);
	void (*on_task_exit) (struct task_struct *tsk);
	void (*on_caps_setuid) (struct task_struct *tsk, struct cred *new_cred);
};

/*
 * Function: hwaa_proc_pre_execve
 * Description:
 * Input: const s8 __user * __user *argv: an array of
 *        arguments passed to the new program
 * Output: None
 * Return: 0 to signal hwaa_proc_post_execve should be called,
 *         non-zero otherwise
 */
s32 hwaa_proc_pre_execve(const s8 *exe_path);

/*
 * Function: hwaa_proc_post_execve
 * Description:
 * Input: pid_t pid: process id of the recently exec'd task
 * Output: None
 * Return: None
 */
void hwaa_proc_post_execve(pid_t pid);

/*
 * Function: hwaa_proc_on_task_forked
 * Description:
 * Input: struct task_struct *tsk: the newly forked task/process
 * Output: None
 * Return: None
 */
void hwaa_proc_on_task_forked(struct task_struct *tsk);

/*
 * Function: hwaa_proc_on_task_exit
 * Description:
 * Input: struct task_struct *tsk: the task/process exiting
 * Output: None
 * Return: None
 */
void hwaa_proc_on_task_exit(struct task_struct *tsk);

/*
 * Function: hwaa_proc_on_caps_setuid
 * Description:
 * Input: struct task_struct *tsk: the task/process receiving new credentials
 *        struct cred *new_cred: the new credentials
 * Output: None
 * Return: None
 */
void hwaa_proc_on_caps_setuid(struct task_struct *tsk, struct cred *new_cred);

/*
 * Function: hwaa_register_proc_callbacks
 * Description: Loads HWAA process management callback functions.
 * Input: struct hwaa_proc_callbacks_t *callbacks: callback functions
 *        to be registered
 * Output: None
 * Return: None
 */
void hwaa_register_proc_callbacks(struct hwaa_proc_callbacks_t *callbacks);

/*
 * Function: hwaa_unregister_proc_callbacks
 * Description: Unloads HWAA process management callback functions.
 * Input: None
 * Output: None
 * Return: None
 */
void hwaa_unregister_proc_callbacks(void);

#endif
