
#include "inc/policy/hwaa_policy.h"
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/hashtable.h>
#include <securec.h>
#include "huawei_platform/hwaa/hwaa_file.h"
#include "inc/fek/hwaa_file_perm.h"
#include "inc/base/hwaa_utils.h"
#include "inc/data/hwaa_data.h"

#define MAX_POLICY_BUF_LEN 256

static bool parse_ruleset_jduge_json(const s8 *ruleset_json_buffer,
	struct ruleset_cache_entry_t *parsed_rule)
{
	parsed_rule->include_subdir_node = hwaa_utils_get_json_str(
		ruleset_json_buffer, "subdirs",
		&parsed_rule->include_subdir_node_len);
	if (!parsed_rule->include_subdir_node ||
		(parsed_rule->include_subdir_node_len == 0))
		return false;
	parsed_rule->ext_node = hwaa_utils_get_json_str(ruleset_json_buffer,
		"ext", &parsed_rule->ext_node_len);
	if (!parsed_rule->ext_node || (parsed_rule->ext_node_len == 0))
		return false;
	return true;
}

static bool parse_ruleset_jduge(const s8 *ruleset_json_buffer,
	struct ruleset_cache_entry_t *parsed_rule)
{
	if (!parsed_rule->path_node || (parsed_rule->path_node_len == 0))
		return false;
	if ((parsed_rule->path_node[0] != '/') ||
		(strstr(parsed_rule->path_node, "..") != NULL))
		return false;
	if (parsed_rule->path_node[parsed_rule->path_node_len-1] != '/')
		return false;
	if (!parse_ruleset_jduge_json(ruleset_json_buffer, parsed_rule))
		return false;
	return true;
}

/*
 * This function parse rule to jduge path node is valid
 * @return true for success
 */
static bool parse_ruleset(const s8 *ruleset_json_buffer,
	struct ruleset_cache_entry_t *parsed_rule)
{
	if (memset_s(parsed_rule, sizeof(struct ruleset_cache_entry_t), 0,
		sizeof(struct ruleset_cache_entry_t)) != EOK)
		return false;
	parsed_rule->path_node = hwaa_utils_get_json_str(ruleset_json_buffer,
		"path", &parsed_rule->path_node_len);
	if (parse_ruleset_jduge(ruleset_json_buffer, parsed_rule))
		return true;

	kzfree(parsed_rule->path_node);
	kzfree(parsed_rule->ext_node);
	kzfree(parsed_rule->include_subdir_node);
	if (memset_s(parsed_rule, sizeof(struct ruleset_cache_entry_t), 0,
		sizeof(struct ruleset_cache_entry_t)) != EOK)
		hwaa_pr_err("memset_s fail, should never happen!");
	return false;
}

s32 hwaa_analys_policy_ruleset(struct ruleset_cache_entry_t *ruleset,
	struct hwaa_package_info_t *pinfo)
{
	return (parse_ruleset(pinfo->app_policy, ruleset)) ? 0 : -EINVAL;
}

static bool do_path_node_matches_rule(const s8 *filename, u32 filename_len,
	const s8 *path_node, u32 path_node_len)
{
	// maybe path_node is data/user/0/XXX,
	// filename is /user/10/XXX
	const s8 *p;
	s32 count_num;
	s32 sub_user_file_path_len = strlen(SUB_USER_FILE_PATH);
	s32 master_path_node_len = strlen(MASTER_PATH_NODE);

	p = filename + sub_user_file_path_len;
	count_num = 0;
	while (*p != '\0' && (*p <= '9' && *p >= '0')) {
		count_num++;
		p++;
	}
	if (*p != '/')
		return false;
	if ((path_node_len - master_path_node_len) >
		(filename_len - (sub_user_file_path_len + count_num + 1)))
		return false;
	if (strncmp(path_node + master_path_node_len, filename +
		sub_user_file_path_len + count_num + 1,
		path_node_len - master_path_node_len) != 0) {
		hwaa_pr_err("filename (%s) not match pathnode (%s)",
		filename, path_node);
		return false;
	}
	return true;
}

static bool path_node_matches_rule(const s8 *filename, u32 filename_len,
	const s8 *path_node, u32 path_node_len)
{
	s32 master_user_file_path_len = strlen(MASTER_USER_FILE_PATH);
	s32 sub_user_file_path_len = strlen(SUB_USER_FILE_PATH);
	s32 master_path_node_len = strlen(MASTER_PATH_NODE);
	s32 sub_user_path_node_len = strlen(SUB_PATH_NODE);
	// sub-user filename is /user/10/XXX, path_node is /data/user/10/XXX;
	// master-user filename is /data/XXX,path_node is /data/user/0/XXX
	if ((filename_len > sub_user_file_path_len) &&
		(strncmp(SUB_USER_FILE_PATH, filename,
		sub_user_file_path_len) == 0) &&
		(path_node_len > sub_user_path_node_len) &&
		(strncmp(SUB_PATH_NODE, path_node,
		sub_user_path_node_len) == 0)) {
		if (strncmp(path_node + sub_user_path_node_len, filename +
			sub_user_file_path_len,
			path_node_len - sub_user_file_path_len) == 0) {
			return true;
		}
		return do_path_node_matches_rule(filename, filename_len,
			path_node, path_node_len);
	} else if ((filename_len > master_user_file_path_len) &&
		(strncmp(MASTER_USER_FILE_PATH,
		filename, master_user_file_path_len) == 0) &&
		(path_node_len > master_path_node_len) &&
		(strncmp(MASTER_PATH_NODE, path_node,
		master_path_node_len) == 0)) {
		if (strncmp(path_node + master_path_node_len, filename +
		    master_user_file_path_len,
		    path_node_len - master_path_node_len) != 0) {
		    hwaa_pr_err("master user not match filename path");
			return false;
		}
		return true;
	}
	return false;
}

static bool ext_name_matches(const s8 *filename, const s8 *policy_ext_node)
{
	s8 *substr = NULL;
	s32 len;
	s8 policy_buf[MAX_POLICY_BUF_LEN] = {0};
	s8 *policy_ext = &policy_buf[0];
	s8 *file_ext_name;

	if ((filename == NULL) || (policy_ext_node == NULL))
		return false;
	file_ext_name = strrchr(filename, '.');
	if (file_ext_name == NULL)
		return false;
	len = strlen(policy_ext_node);
	if ((len >= sizeof(policy_buf)) || (len < 0))
		return false;
	if (memcpy_s(policy_buf, MAX_POLICY_BUF_LEN, policy_ext_node, len)
		!= EOK)
		return false;

	while (true) {
		substr = strsep((char **)&policy_ext, "|");
		if (substr != NULL) {
			if (strcasecmp(file_ext_name, substr) == 0)
				return true;
		} else {
			return false;
		}
	}
}

static bool jduge_filename_valid(const s8 *fsname, const s8 *filename)
{
	if (strcmp("f2fs", fsname) != 0) {
		hwaa_pr_err("fsname err [%s]", fsname);
		return false;
	}
	if (strstr(filename, "../") != NULL)
		return false; // make sure we are not under filepath attact
	return true;
}

static bool file_matches_rule_path_node(
	const struct ruleset_cache_entry_t *rule,
	const s8 *filename_part, size_t filename_len, const s8 *filename)
{
	// If the input filename is too short, then fail
	if (filename_len <= rule->path_node_len) {
		hwaa_pr_err(" bad filename_len <= rule->path_node_len");
		return false;
	}
	// Check that the filename has a '/' in it
	if (filename_part != NULL) {
		// Check that the initial path matches the filename path
		if (path_node_matches_rule(filename, filename_len,
			rule->path_node, rule->path_node_len) != true) {
			hwaa_pr_debug("failed filename %s rule->path_node %s ",
				filename, rule->path_node);
			return false;
		}
		// if subdir_node is not on, then check to ensure that the file
		// is contained in the parent dir
		if (!rule->include_subdir_node) {
			hwaa_pr_err("rule's subdir node empty");
			return false;
		}
		if (strcmp(rule->include_subdir_node, "true") != 0) {
			if ((filename_part - filename) !=
				(rule->path_node_len - 1)) {
				hwaa_pr_err("matches_rule subdir failed [%s]",
					      rule->include_subdir_node);
				return false;
			}
		}
	} else {
		hwaa_pr_err("file_matches_rule filename_part empty");
		return false;
	}
	return true;
}

/*
 * JSON rule tags
 *
 * "fs" - exact match on the filesystem identification
 * "path" - match on the pathname
 * "subdirs" - the pathname will match on subdirectories too,
 *             otherwise only match on the path
 * "file" - exact match on the filename
 * "ext" - case insensitive match on the filename extension
 */
static bool file_matches_rule(const struct ruleset_cache_entry_t *rule,
	const s8 *fsname, const s8 *filename)
{
	size_t filename_len;
	const s8 *filename_part;

	if ((rule == NULL) || (fsname == NULL) || (filename == NULL)) {
		hwaa_pr_err("file_matches_rule bad arg");
		return false;
	}
	if (!jduge_filename_valid(fsname, filename))
		return false;
	filename_part = strrchr(filename, '/');
	filename_len = strlen(filename);
	if (rule->path_node) {
		if (!file_matches_rule_path_node(rule, filename_part,
		     filename_len, filename))
		return false;
	}
	if (rule->ext_node) {
		if (!ext_name_matches(filename, rule->ext_node)) {
			hwaa_pr_debug("extname not match [%s] [%s]",
				filename, rule->ext_node);
			return false;
		}
	}
	return true;
}

static void apply_default_perms(mode_t mode, u64 *perms)
{
	*perms = 0;
	if (S_IRUSR & mode)
		HWAA_FILE_PERM_SET_USER_READ(*perms);
	if (S_IWUSR & mode)
		HWAA_FILE_PERM_SET_USER_WRITE(*perms);
	if (S_IROTH & mode)
		HWAA_FILE_PERM_SET_OTHER_READ(*perms);
	if (S_IWOTH & mode)
		HWAA_FILE_PERM_SET_OTHER_WRITE(*perms);
}

void hwaa_utils_free_policy_ruleset(struct ruleset_cache_entry_t *ruleset)
{
	if (!ruleset)
		return;
	kzfree(ruleset->ext_node);
	ruleset->ext_node = NULL;
	kzfree(ruleset->include_subdir_node);
	ruleset->include_subdir_node = NULL;
	kzfree(ruleset->path_node);
	ruleset->path_node = NULL;
}

bool hwaa_evaluate_policies(struct package_info_listnode_t *pkg_info_node,
	const s8 *fsname, const s8 *filename, mode_t mode,
	u64 *perms)
{
	u64 local_perms = 0;
	bool matched = false;
	if (pkg_info_node == NULL)
		return NULL;
	apply_default_perms(mode, &local_perms);
	if (file_matches_rule(pkg_info_node->ruleset_cache, fsname, filename)) {
		HWAA_FILE_PERM_SET_OTHER(local_perms, HWAA_FILE_PERM_NULL);
		*perms = local_perms;
		matched = true;
	}
	if (matched) {
		hwaa_pr_debug("evaluate_policies called %s filename matched %d",
			filename, matched);
	}
	return matched;
}
