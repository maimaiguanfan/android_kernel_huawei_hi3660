#ifndef _OASES_PATCH_MGR_H
#define _OASES_PATCH_MGR_H

#include "patch_info.h"

#include <linux/slab.h>
#include <linux/mm.h>

enum {
	OASES_PATCH_SUCCESS = 0,
	OASES_PATCH_FAILURE
};

#define oases_uid_eq(a, b) ((a) == (b))
#define OASES_INVALID_UID (long)(-1)

extern struct list_head patchinfo_list;
extern struct kmem_cache *oases_patch_slab;

static inline struct oases_patch_info *patch_info_from_id(
	const char *id)
{
	struct oases_patch_info *tmp;
	struct oases_patch_info *ret = NULL;
	struct kmem_cache *cachep;
	struct page *page;

	list_for_each_entry(tmp, &patchinfo_list, list) {
		if (!strcmp(id, tmp->id)) {
			ret = tmp;
			break;
		}
	}
	if (ret) {
		page = virt_to_head_page(ret);
		cachep = page->slab_cache;
		if (cachep != oases_patch_slab)
			ret = NULL;
	}
	return ret;
}

struct oases_unpatch {
	char id[PATCH_ID_LEN];
};

int oases_check_patch(const char *name);

int oases_check_patch_func(const char *name);

void oases_free_patch(struct oases_patch_info *ctx);

void oases_attack_logger(struct oases_patch_info *ctx);

#endif/* _OASES_PATCH_MGR_H */
