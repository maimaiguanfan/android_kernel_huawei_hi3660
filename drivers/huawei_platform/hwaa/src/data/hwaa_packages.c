
#include "inc/data/hwaa_packages.h"
#include <linux/hashtable.h>
#include <linux/slab.h>
#include "inc/base/hwaa_utils.h"
#include "inc/policy/hwaa_policy.h"

#define HWAA_PKG_DEFAULT_HASHTABLESIZE 12
static DEFINE_HASHTABLE(g_packages_hashtable, HWAA_PKG_DEFAULT_HASHTABLESIZE);
static s32 get_appid(uid_t uid)
{
	return (s32)(uid % HWAA_PER_USER_RANGE);
}

static struct package_hashnode_t *get_package_hashnode(s32 appid)
{
	struct package_hashnode_t *package_hashnode = NULL;
	struct hlist_node *tmp = NULL;

	hash_for_each_possible_safe(g_packages_hashtable, package_hashnode,
		tmp, hash_list, appid) {
		if (package_hashnode->appid == appid)
			return package_hashnode;
	}
	return NULL;
}

bool hwaa_packages_exists(uid_t uid)
{
	return (get_package_hashnode(get_appid(uid))) ? true : false;
}

static void delete_pinfo_listnode(
	struct package_info_listnode_t *pinfo_listnode, bool free_self)
{
	if (!pinfo_listnode)
		return;
	hwaa_utils_free_package_info(pinfo_listnode->pinfo);
	kzfree(pinfo_listnode->pinfo);
	pinfo_listnode->pinfo = NULL;
	hwaa_utils_free_policy_ruleset(pinfo_listnode->ruleset_cache);
	kzfree(pinfo_listnode->ruleset_cache);
	pinfo_listnode->ruleset_cache = NULL;
	// make sure list add at the last time everytime so as not need to
	// list_del pinfo_listnode
	if (free_self)
		kzfree(pinfo_listnode);
}

static void delete_package_hashnode(struct package_hashnode_t *package_hashnode)
{
	struct package_info_listnode_t *package_info_listnode = NULL;
	struct package_info_listnode_t *tmp = NULL;

	if (!package_hashnode)
		return;
	if (!list_empty(&package_hashnode->pinfo_list)) {
		list_for_each_entry_safe(package_info_listnode, tmp,
			&package_hashnode->pinfo_list, list) {
			delete_pinfo_listnode(package_info_listnode, false);
			list_del(&package_info_listnode->list);
			kfree(package_info_listnode);
		}
	}
	hash_del(&package_hashnode->hash_list);
	kfree(package_hashnode);
}

static void delete_package_hashnode_by_name(
	struct package_hashnode_t *package_hashnode, const s8 *name,
	u64 name_len)
{
	struct package_info_listnode_t *package_info_listnode = NULL;
	struct package_info_listnode_t *tmp = NULL;

	if (!package_hashnode)
		return;
	if (!list_empty(&package_hashnode->pinfo_list)) {
		list_for_each_entry_safe(package_info_listnode,
			tmp, &package_hashnode->pinfo_list, list) {
			if ((package_info_listnode->pinfo->name_len == name_len) &&
				memcmp(package_info_listnode->pinfo->name, name,
				name_len) == 0) {
				delete_pinfo_listnode(package_info_listnode,
					false);
				list_del(&package_info_listnode->list);
				kfree(package_info_listnode);
			}
		}
	}
	if (list_empty(&package_hashnode->pinfo_list)) {
		hash_del(&package_hashnode->hash_list);
		kfree(package_hashnode);
	}
}

static struct package_info_listnode_t *get_pinfo_from_package_node(
	struct package_hashnode_t *package_hashnode,
	struct hwaa_package_info_t *pinfo)
{
	struct package_info_listnode_t *pinfo_listnode = NULL;

	if (!package_hashnode)
		return NULL;
	list_for_each_entry(pinfo_listnode, &package_hashnode->pinfo_list,
		list) {
		if ((pinfo->name_len == pinfo_listnode->pinfo->name_len) &&
			(memcmp(pinfo->name, pinfo_listnode->pinfo->name,
			pinfo->name_len) == 0))
			return pinfo_listnode;
	}
	return NULL;
}

static s32 add_pinfo_to_pkg_listnode(
	struct package_hashnode_t *package_hashnode,
	struct hwaa_package_info_t *pinfo)
{
	struct package_info_listnode_t *pinfo_listnode;
	s32 ret;

	pinfo_listnode = get_pinfo_from_package_node(package_hashnode, pinfo);
	if (pinfo_listnode == NULL) {
		pinfo_listnode = (struct package_info_listnode_t *)kzalloc(
			sizeof(struct package_info_listnode_t), GFP_KERNEL);
		if (!pinfo_listnode) {
			hwaa_pr_err("Failed to allocate memory!");
			ret = -ENOMEM;
			goto cleanup;
		}
		pinfo_listnode->pinfo = (struct hwaa_package_info_t *)kzalloc(
			sizeof(struct hwaa_package_info_t), GFP_KERNEL);
		if (!pinfo_listnode->pinfo) {
			hwaa_pr_err("Failed to allocate pinfo memory!");
			ret = -ENOMEM;
			goto cleanup;
		}
		ret = hwaa_utils_copy_package_info(pinfo_listnode->pinfo,
			pinfo);
		if (ret)
			goto cleanup;
		pinfo_listnode->ruleset_cache =
			(struct ruleset_cache_entry_t *)kzalloc(
			sizeof(struct ruleset_cache_entry_t), GFP_KERNEL);
		if (pinfo_listnode->ruleset_cache == NULL) {
			hwaa_pr_err("Failed to allocate memory!");
			ret = -ENOMEM;
			goto cleanup;
		}
		ret = hwaa_analys_policy_ruleset(pinfo_listnode->ruleset_cache,
			pinfo);
		if (ret)
			goto cleanup;
		list_add(&pinfo_listnode->list, &package_hashnode->pinfo_list);
		return ret;
	} else {
		hwaa_pr_info("already exist!");
		return 0;
	}
cleanup:
	delete_pinfo_listnode(pinfo_listnode, true);
	return ret;
}

s32 hwaa_packages_insert(struct hwaa_package_info_t *pinfo)
{
	s32 ret;
	struct package_hashnode_t *package_hashnode;
	package_hashnode = get_package_hashnode(pinfo->appid);
	if (package_hashnode) {
		// if exists, just add pinfo
		return add_pinfo_to_pkg_listnode(package_hashnode, pinfo);
	}
	package_hashnode = (struct package_hashnode_t *)kzalloc(
		sizeof(struct package_hashnode_t), GFP_KERNEL);
	if (package_hashnode == NULL) {
		hwaa_pr_err("Failed to allocate memory!");
		ret = -ENOMEM;
		goto done;
	}
	package_hashnode->appid = pinfo->appid;
	INIT_LIST_HEAD(&package_hashnode->pinfo_list);
	hash_add(g_packages_hashtable, &package_hashnode->hash_list,
		package_hashnode->appid);
	ret = add_pinfo_to_pkg_listnode(package_hashnode, pinfo);
	if (ret)
		goto cleanup;
	goto done;

cleanup:
	if (package_hashnode) {
		// hunter make sure everything cleans up here
		delete_package_hashnode(package_hashnode);
	}
done:
	return ret;
}

struct package_hashnode_t *get_hwaa_package_hashnode(uid_t uid)
{
	struct package_hashnode_t *package_hashnode;
	s32 appid = get_appid(uid);

	package_hashnode = get_package_hashnode(appid);
	if (!package_hashnode) {
		hwaa_pr_err("get_hwaa_package empty");
		return NULL;
	}
	return package_hashnode;
}

s32 hwaa_packages_get_app_data(uid_t uid,
	struct package_info_listnode_t *pinfo_listnode, u8 **app_data,
	u32 *app_data_len)
{
	s32 ret = -EINVAL;

	if (pinfo_listnode && pinfo_listnode->pinfo)
		ret = hwaa_utils_get_app_data(app_data, app_data_len,
			pinfo_listnode->pinfo, uid);
	return ret;
}

void hwaa_packages_delete(struct hwaa_package_info_t *pinfo)
{
	struct package_hashnode_t *package_node = NULL;
	struct hlist_node *tmp = NULL;
	s32 bkt = 0;
	if (!pinfo || !pinfo->name) {
		hwaa_pr_err("unexpected bad param");
		return;
	}

	if (hash_empty(g_packages_hashtable)) {
		hwaa_pr_err("pkg hash table empty");
		return;
	}
	hash_for_each_safe(g_packages_hashtable, bkt, tmp, package_node,
		hash_list) {
		delete_package_hashnode_by_name(package_node, pinfo->name,
			pinfo->name_len);
	}
}

void hwaa_packages_delete_all(void)
{
	s32 bkt = 0;
	struct package_hashnode_t *package_node = NULL;
	struct hlist_node *tmp = NULL;

	hash_for_each_safe(g_packages_hashtable, bkt, tmp,
		package_node, hash_list) {
		delete_package_hashnode(package_node);
	}
}
