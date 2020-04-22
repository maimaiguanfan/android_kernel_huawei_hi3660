
#ifndef _HWAA_POLICY_H
#define _HWAA_POLICY_H

#include "huawei_platform/hwaa/hwaa_ioctl.h"
#include "inc/data/hwaa_packages.h"

#define CERTINFO_HEX_LENGTH 64
#define MASTER_USER_FILE_PATH "/data/"
#define SUB_USER_FILE_PATH "/user/"
#define MASTER_PATH_NODE "/data/user/0/"
#define SUB_PATH_NODE "/data/user/"

bool hwaa_evaluate_policies(struct package_info_listnode_t *pkg_info_node,
	const s8 *fsname, const s8 *filename,
	mode_t mode, u64 *perms);

s32 hwaa_analys_policy_ruleset(struct ruleset_cache_entry_t *ruleset,
	struct hwaa_package_info_t *pinfo);

void hwaa_utils_free_policy_ruleset(struct ruleset_cache_entry_t *ruleset);
#endif

