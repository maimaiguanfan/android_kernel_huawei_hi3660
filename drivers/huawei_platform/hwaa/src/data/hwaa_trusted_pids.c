
#include "inc/data/hwaa_trusted_pids.h"
#include <linux/list.h>
#include <linux/hashtable.h>
#include <linux/slab.h>
#include "inc/base/hwaa_utils.h"

#define HWAA_APPS_HTABLE_BITS 12

static struct zygote_pid_t g_zygote_pids;

static DEFINE_HASHTABLE(g_app_pids, HWAA_APPS_HTABLE_BITS);

static struct zygote_pid_t *get_zygote_pid(pid_t pid)
{
	struct zygote_pid_t *zygote_pid = 0;
	struct zygote_pid_t *tmp = 0;

	list_for_each_entry_safe(zygote_pid, tmp, &g_zygote_pids.list, list) {
		if (zygote_pid->pid == pid)
			return zygote_pid;
	}
	return NULL;
}

static struct app_pid_t *get_app_pid(pid_t pid)
{
	struct app_pid_t *app_pid = NULL;

	hash_for_each_possible(g_app_pids, app_pid, hash_list, pid) {
		if (app_pid->pid == pid)
			return app_pid;
	}
	return NULL;
}

static bool has_zygote_pid(pid_t pid)
{
	return get_zygote_pid(pid) ? true : false;
}

static bool has_app_pid(pid_t pid)
{
	return get_app_pid(pid) ? true : false;
}

static s32 insert_app_pid(pid_t pid)
{
	s32 ret = 0;
	struct app_pid_t *app_pid = get_app_pid(pid);
#ifdef HWAA_DEBUG
	hwaa_pr_info("Adding process pid:%d to list of trusted apps", pid);
#endif
	if (app_pid) {
#ifdef HWAA_DEBUG
		hwaa_pr_warn("Process pid:%d is already in list trusted apps",
			pid);
#endif
		goto done;
	}
	app_pid = kzalloc(sizeof(struct app_pid_t), GFP_KERNEL);
	if (!app_pid) {
		ret = -ENOMEM;
		goto done;
	}

	app_pid->pid = pid;
	hash_add(g_app_pids, &app_pid->hash_list, app_pid->pid);
done:
	return ret;
}

static s32 insert_zygote_pid(pid_t pid)
{
	s32 ret = 0;
	struct zygote_pid_t *zygote_pid = get_zygote_pid(pid);

	if (zygote_pid) {
		hwaa_pr_warn("pid:%d is already in trusted zygotes list", pid);
		goto done;
	}
	zygote_pid = kzalloc(sizeof(struct zygote_pid_t), GFP_KERNEL);
	if (!zygote_pid) {
		ret = -ENOMEM;
		goto done;
	}
	zygote_pid->pid = pid;
	list_add(&zygote_pid->list, &g_zygote_pids.list);
done:
	return ret;
}

static void do_delete_zygote_pid(struct zygote_pid_t *zygote_pid)
{
#ifdef HWAA_DEBUG
	hwaa_pr_info("Removing  pid:%d from list of trusted zygotes",
		zygote_pid->pid);
#endif
	list_del(&zygote_pid->list);
	kfree(zygote_pid);
}

static void delete_zygote_pid(pid_t pid)
{
	struct zygote_pid_t *zygote_pid = 0;
	struct zygote_pid_t *tmp = 0;

	list_for_each_entry_safe(zygote_pid, tmp, &g_zygote_pids.list, list) {
		if (zygote_pid->pid == pid) {
			do_delete_zygote_pid(zygote_pid);
			return;
		}
	}
#ifdef HWAA_DEBUG
	hwaa_pr_warn("pid:%d was not in trusted zygotes list", zygote_pid->pid);
#endif
}

static void do_delete_app_pid(struct app_pid_t *app_pid)
{
#ifdef HWAA_DEBUG
	hwaa_pr_info("Removing pid:%d from trusted apps list", app_pid->pid);
#endif
	hash_del(&app_pid->hash_list);
	kfree(app_pid);
}

static void delete_app_pid(pid_t pid)
{
	struct app_pid_t *app_pid = NULL;
	struct hlist_node *tmp = NULL;

	hash_for_each_possible_safe(g_app_pids, app_pid, tmp, hash_list, pid) {
		if (app_pid->pid == pid) {
			do_delete_app_pid(app_pid);
			return;
		}
	}
#ifdef HWAA_DEBUG
	hwaa_pr_warn("pid:%d was not in trusted apps list", pid);
#endif
}

static void delete_app_pids(void)
{
	s32 bkt = 0;
	struct app_pid_t *app_pid = NULL;
	struct hlist_node *tmp = NULL;

	hash_for_each_safe(g_app_pids, bkt, tmp, app_pid, hash_list) {
		do_delete_app_pid(app_pid);
	}
}

static void delete_zygote_pids(void)
{
	struct zygote_pid_t *zygote_pid = 0;
	struct zygote_pid_t *tmp = 0;

	list_for_each_entry_safe(zygote_pid, tmp, &g_zygote_pids.list, list) {
		do_delete_zygote_pid(zygote_pid);
	}
}

bool hwaa_trusted_pids_exists(hwaa_trusted_pid_t trusted_pid_type, pid_t pid)
{
	switch (trusted_pid_type) {
	case HWAA_TRUSTED_APP:
		return has_app_pid(pid);
	case HWAA_TRUSTED_ZYGOTE:
		return has_zygote_pid(pid);
	default:
		return false;
	}
}

s32 hwaa_trusted_pids_insert(hwaa_trusted_pid_t trusted_pid_type, pid_t pid)
{
	switch (trusted_pid_type) {
	case HWAA_TRUSTED_APP:
		return insert_app_pid(pid);
	case HWAA_TRUSTED_ZYGOTE:
		return insert_zygote_pid(pid);
	default:
		return -EINVAL;
	}
}

void hwaa_trusted_pids_delete(hwaa_trusted_pid_t trusted_pid_type, pid_t pid)
{
	switch (trusted_pid_type) {
	case HWAA_TRUSTED_APP:
		delete_app_pid(pid);
		break;
	case HWAA_TRUSTED_ZYGOTE:
		delete_zygote_pid(pid);
		break;
	default:
		return;
	}
}

void hwaa_trusted_pids_delete_all(hwaa_trusted_pid_t trusted_pid_type)
{
	switch (trusted_pid_type) {
	case HWAA_TRUSTED_APP:
		delete_app_pids();
		break;
	case HWAA_TRUSTED_ZYGOTE:
		delete_zygote_pids();
		break;
	default:
		return;
	}
}

void hwaa_trusted_pids_init(void)
{
	INIT_LIST_HEAD(&g_zygote_pids.list);
}

void hwaa_trusted_pids_deinit(void)
{
	delete_app_pids();
	delete_zygote_pids();
	INIT_LIST_HEAD(&g_zygote_pids.list);
}
