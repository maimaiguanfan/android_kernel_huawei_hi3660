/*
 * patch_mgr.c - patch manager
 *
 * Copyright (C) 2016 Baidu, Inc. All Rights Reserved.
 *
 * You should have received a copy of license along with this program;
 * if not, ask for it from Baidu, Inc.
 *
 */

#include <linux/vmalloc.h>
#include <linux/mutex.h>
#include <asm/uaccess.h>
#include <linux/mm_types.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/syscalls.h>
#include <linux/kobject.h>

#include "patch_mgr.h"
#include "util.h"
#include "patch_file.h"
#include "patch_info.h"
#include "patch_base.h"
#include "sysfs.h"
#include "plts.h"

#define FUNC_NAME_LEN_MAX 255
#define UEVENT_INFO_LEN_MAX 128

LIST_HEAD(patchinfo_list);
struct kmem_cache *oases_patch_slab;

extern struct kobject *attack_kobj;
extern struct kset *attack_kset;
extern int attack_upload_init;

int oases_check_patch_func(const char *name)
{
	struct oases_patch_info *ctx, *c;
	struct oases_patch_entry *patch, *p;

	/* oases_mutex held */
	list_for_each_entry_safe(ctx, c, &patchinfo_list, list) {
		list_for_each_entry_safe(patch, p, &ctx->patches, list) {
			if (!strncmp(name, kp_vtab(patch)->get_name(patch), FUNC_NAME_LEN_MAX - 1)) {
				oases_error("duplicate name: %s\n", name);
				return -EEXIST;
			}
		}
	}
	return 0;
}

int oases_check_patch(const char *id)
{
	struct oases_patch_info *pos;
	struct oases_patch_info *n;

	/* oases_mutex held */
	list_for_each_entry_safe(pos, n, &patchinfo_list, list) {
		if (strncmp(pos->id, id, PATCH_ID_LEN - 1) == 0) {
			oases_error("patch exist: %s\n", id);
			return -EEXIST;
		}
	}
	return 0;
}

void oases_free_patch(struct oases_patch_info *info)
{
	struct oases_patch_entry *patch, *p;

	if (!info) {
		return;
	}
	list_for_each_entry_safe(patch, p, &info->patches, list) {
		kp_vtab(patch)->destroy(patch);
		list_del(&patch->list);
		kfree(patch->data);
		kfree(patch);
	}
	oases_patch_addr_free(&info->addresses);
	if (info->code_base) {
		vfree(info->code_base);
	}
	if (info->plog)
		kfree(info->plog);
	kmem_cache_free(oases_patch_slab, info);
}

struct notify_work {
	struct work_struct work;
	long uid;
	char id[PATCH_ID_LEN];
};

static void oases_notify_uevent(struct work_struct *work)
{
	enum {
		UEVENT_UID = 0,
		UEVENT_PATCHID = 1,
		UEVENT_NULL = 2
	};

	struct notify_work *notify = container_of(work, struct notify_work, work);
	char *uevent_envp[UEVENT_NULL + 1] = {NULL};
	int index;

	oases_info("notify uevent starting\n");

	if (!notify) {
		oases_error("notify=NULL\n");
		return;
	}
	if ((attack_upload_init != ATKSYS_INIT) || !attack_kobj) {
		oases_error("check attack_upload_init failed\n");
		kfree(notify);
		return;
	}

	index = 0;
	while (index < UEVENT_NULL) {
		uevent_envp[index] = vmalloc(UEVENT_INFO_LEN_MAX);
		if (!uevent_envp[index]) {
			oases_error("vmalloc failed\n");
			break;
		}
		memset(uevent_envp[index], 0, UEVENT_INFO_LEN_MAX);
		++index;
	}

	if (index == UEVENT_NULL) {
		snprintf(uevent_envp[UEVENT_UID], UEVENT_INFO_LEN_MAX,
			"uid=%ld", notify->uid);
		snprintf(uevent_envp[UEVENT_PATCHID], UEVENT_INFO_LEN_MAX,
			"patch_id=%s", notify->id);
		uevent_envp[UEVENT_NULL] = NULL;
		kobject_uevent_env(attack_kobj, KOBJ_CHANGE, uevent_envp);
	}

	while (index > 0) {
		--index;
		vfree(uevent_envp[index]);
		uevent_envp[index] = NULL;
	}
	kfree(notify);
}

static void increase_attack_count(struct oases_patch_info *ctx, long uid)
{
	int i;
	struct oases_attack_log *entry;
	struct notify_work *notify;

	/* node 0 belongs to INVALID_UID */
	if (oases_uid_eq(uid, OASES_INVALID_UID)) {
		entry = ctx->plog;
		entry->count++;
		entry->end_time = get_seconds();
		if (!entry->start_time)
			entry->start_time = entry->end_time;
		return;
	}

	for (i = 1; i < ctx->log_index; i++) {
		entry = ctx->plog + i;
		if (oases_uid_eq(entry->uid, uid)) {
			entry->count++;
			entry->end_time = get_seconds();
			return;
		}
	}

	/* new uid */
	entry = ctx->plog + ctx->log_index;
	entry->uid = uid;
	entry->count = 1;
	entry->start_time = get_seconds();
	entry->end_time = entry->start_time;
	ctx->log_index++;

	if (ctx->id) {
		notify = kmalloc(sizeof(struct notify_work), GFP_ATOMIC);
		if (notify) {
			INIT_WORK(&notify->work, oases_notify_uevent);
			notify->uid = uid;
			memcpy(notify->id, ctx->id, sizeof(notify->id));
			queue_work(system_long_wq, &notify->work);
		} else {
			oases_error("kmalloc failed\n");
		}
	}
}

void oases_attack_logger(struct oases_patch_info *ctx)
{
	long uid = OASES_INVALID_UID;
	unsigned long flags;

	spin_lock_irqsave(&ctx->log_lock, flags);
	if (ctx->log_index >= OASES_LOG_NODE_MAX)
		goto index_zero;
	uid = sys_getuid();

index_zero:
	increase_attack_count(ctx, uid);
	spin_unlock_irqrestore(&ctx->log_lock, flags);
}
