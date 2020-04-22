
#ifndef _HWAA_PACKAGES_H
#define _HWAA_PACKAGES_H

#include "huawei_platform/hwaa/hwaa_ioctl.h"

struct ruleset_cache_entry_t {
	const s8 *ext_node;
	u32 ext_node_len;
	const s8 *path_node;
	u32 path_node_len;
	const s8 *include_subdir_node;
	u32 include_subdir_node_len;
};

struct package_t {
	s32 appid;
	struct hwaa_package_info_t *pinfo;
	struct ruleset_cache_entry_t *ruleset_cache;
	struct hlist_node hash_list;
};

struct package_info_listnode_t {
	struct hwaa_package_info_t *pinfo;
	struct ruleset_cache_entry_t *ruleset_cache;
	struct list_head list;
};

struct package_hashnode_t {
	s32 appid; // this must be assigned
	struct list_head pinfo_list;
	struct hlist_node hash_list;
};

bool hwaa_packages_exists(uid_t uid);

s32 hwaa_packages_insert(struct hwaa_package_info_t *pinfo);

void hwaa_packages_delete(struct hwaa_package_info_t* pinfo);

/* This function delete all packages information from list */
void hwaa_packages_delete_all(void);

s32 hwaa_packages_get_pinfo(uid_t uid, struct hwaa_package_info_t **pinfo);

struct package_hashnode_t *get_hwaa_package_hashnode(uid_t uid);

s32 hwaa_packages_get_app_data(uid_t uid,
	struct package_info_listnode_t *pinfo_listnode, u8 **app_data,
	u32 *app_data_len);

#endif
