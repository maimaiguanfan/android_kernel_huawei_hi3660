
#include "inc/data/hwaa_running_apps.h"
#include <linux/hashtable.h>
#include <linux/slab.h>
#include "inc/base/hwaa_utils.h"

static DEFINE_HASHTABLE(g_running_apps, 12);

static struct running_app_t *get_running_app(uid_t uid)
{
	struct running_app_t *running_app = NULL;

	hash_for_each_possible(g_running_apps, running_app, hash_list, uid) {
		if (running_app->uid == uid)
			return running_app;
	}
	return NULL;
}

static struct running_app_pid_t *get_running_app_pid(
	struct running_app_t *running_app, pid_t pid)
{
	struct running_app_pid_t *running_app_pid = NULL;

	list_for_each_entry(running_app_pid, &running_app->pid_list, list) {
		if (running_app_pid->pid == pid)
			return running_app_pid;
	}
	return NULL;
}

static s32 add_pid(struct running_app_t *running_app, pid_t pid)
{
	s32 ret = 0;

	struct running_app_pid_t *running_app_pid =
		get_running_app_pid(running_app, pid);

	if (running_app_pid == NULL) {
#ifdef HWAA_DEBUG
		hwaa_pr_info("Adding process pid:%d to running app uid:%d",
		pid, running_app->uid);
#endif
		running_app_pid = kmalloc(sizeof(struct running_app_pid_t),
			GFP_KERNEL);
		if (running_app_pid) {
			running_app_pid->pid = pid;
			list_add(&running_app_pid->list,
				&running_app->pid_list);
		} else {
			hwaa_pr_err("Failed to allocate memory!");
			ret = -ENOMEM;
		}
	}

	return ret;
}

static void delete(struct running_app_t *running_app)
{
	struct running_app_pid_t *running_app_pid = NULL;
	struct running_app_pid_t *tmp = NULL;

	list_for_each_entry_safe(running_app_pid, tmp,
		&running_app->pid_list, list) {
#ifdef HWAA_DEBUG
		hwaa_pr_info("Removing process pid:%d from running app uid:%d",
			running_app_pid->pid, running_app->uid);
#endif
		list_del(&running_app_pid->list);
		kfree(running_app_pid);
	}
#ifdef HWAA_DEBUG
	hwaa_pr_info("Deleting running app uid:%d", running_app->uid);
#endif
	hash_del(&running_app->hash_list);
	kfree(running_app);
}

s32 hwaa_running_apps_add_pid(uid_t uid, pid_t pid)
{
	s32 ret = 0;

	struct running_app_t *running_app = get_running_app(uid);
	if (running_app) {
		ret = add_pid(running_app, pid);
		running_app = NULL;
	} else {
		ret = -EINVAL;
	}

	return ret;
}

s32 hwaa_running_apps_remove_pid(uid_t uid, pid_t pid)
{
	s32 was_last_pid = 0;
	struct running_app_pid_t *running_app_pid = NULL;
	struct running_app_pid_t *tmp = NULL;
	struct running_app_t *running_app = get_running_app(uid);

	if (running_app) {
		list_for_each_entry_safe(running_app_pid, tmp,
			&running_app->pid_list, list) {
			if (running_app_pid->pid == pid) {
				list_del(&running_app_pid->list);
				kfree(running_app_pid);
				running_app_pid = NULL;
				break;
			}
		}
		if (list_empty(&running_app->pid_list))
			was_last_pid = 1;
	}
	running_app = NULL;
	running_app_pid = NULL;

	return was_last_pid;
}

s32 hwaa_running_apps_insert(uid_t uid, pid_t pid)
{
	s32 ret = -EINVAL;
	struct running_app_t *running_app = get_running_app(uid);

	if (running_app) {
		ret = add_pid(running_app, pid); // if exists, just add pid
		if (ret)
			goto cleanup;
	} else {
#ifdef HWAA_DEBUG
		hwaa_pr_info("Inserting running app (uid:%d)", uid);
#endif
		running_app = kzalloc(sizeof(struct running_app_t), GFP_KERNEL);
		if (running_app == NULL) {
			ret = -ENOMEM;
			goto done;
		}
		running_app->uid = uid;
		INIT_LIST_HEAD(&running_app->pid_list);
		hash_add(g_running_apps, &running_app->hash_list,
			running_app->uid);
		ret = add_pid(running_app, pid);
		if (ret)
			goto cleanup;
		ret = 0;
		goto done;
	}
cleanup:
	if (running_app)
		delete(running_app);
done:

	return ret;
}

bool hwaa_running_apps_exists(uid_t uid)
{
	return (get_running_app(uid) != NULL) ? true : false;
}

s32 hwaa_running_apps_delete(uid_t uid)
{
	s32 ret = -EINVAL;
	struct running_app_t *running_app = NULL;
	struct hlist_node *tmp = NULL;

	hash_for_each_possible_safe(g_running_apps, running_app, tmp,
		hash_list, uid) {
		if (running_app->uid == uid) {
			delete(running_app);
			ret = 0;
			break;
		}
	}

	return ret;
}

void hwaa_running_apps_delete_all(void)
{
	s32 bkt = 0;
	struct running_app_t *running_app = NULL;
	struct hlist_node *tmp = NULL;

	hash_for_each_safe(g_running_apps, bkt, tmp, running_app, hash_list) {
		delete(running_app);
	}
}
