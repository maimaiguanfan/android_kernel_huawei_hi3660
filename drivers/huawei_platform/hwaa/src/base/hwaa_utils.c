
#include "inc/base/hwaa_utils.h"
#include <linux/err.h>
#include <linux/file.h>
#include <linux/fs.h>
#include <linux/limits.h>
#include <linux/mm.h>
#include <linux/path.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/uaccess.h>
#include <linux/rcupdate.h>
#include <securec.h>
#include "inc/base/macros.h"
#include "huawei_platform/hwaa/hwaa_limits.h"

#define EXE_MAX_ARG_STRLEN 256
#define MAX_JSON_STR_LEN 256
#define HEX_STR_PER_BYTE 2
#define HWAA_PERMISSION_LEN 8
#define PROCESS_INIT_TAG "init="
#define PROCESS_INIT_TAG_LEN 5

/*
 * This function truns byte into hex and the number in function is not
 * magic number, because they are understandable
 */
static void do_turn_byte_into_hex(const u8 *in, s8 *out, s32 size, s32 sizeOut)
{
	/* get data into pure hex */
	s32 i;
	s32 j;

	for (i = 0; i < size; i++) {
		out[HEX_STR_PER_BYTE * i] = (in[i] >> 4);
		out[HEX_STR_PER_BYTE * i + 1] = (in[i] & 0x0F);
	}
	/* turn hex into hex char */
	for (j = 0; j < sizeOut; j++) {
		if (out[j] <= 9)
			out[j] += '0';
		else
			out[j] += ('A' - 0x0A);
	}
}

s8 *turn_byte_into_hex(u8 *in, s32 size)
{
	s32 sizeOut;
	s8 *out;
	// this check is useless for the caller len
	// is fixed and much less than this
	if (!in || (size > HWAA_MAX_BUFFER / HEX_STR_PER_BYTE))
		return NULL;
	sizeOut = size * HEX_STR_PER_BYTE;
	out = (u8 *)kzalloc(sizeOut + 1, GFP_NOFS);
	if (!out)
		return NULL;
	do_turn_byte_into_hex(in, out, size, sizeOut);
	return out;
}

static s8 hex2char(u8 hex)
{
	s8 c;
	u8 lower = hex & 0x0f;

	if (lower >= 0x0a)
		c = ('a' + lower - 0x0a);
	else
		c = '0' + lower;
	return c;
}

void bytes2string(u8 *bytes, u32 len, s8 *str, u32 str_len)
{
	s32 i;

	if (!bytes || !str || ((HEX_STR_PER_BYTE * len + 1) > str_len))
		return;
	for (i = 0; i < len; i++) {
		str[HEX_STR_PER_BYTE * i] = hex2char((bytes[i] & 0xf0) >> 4);
		str[HEX_STR_PER_BYTE * i + 1] = hex2char(bytes[i] & 0x0f);
	}
	str[HEX_STR_PER_BYTE * len] = '\0';
}

s32 valid_init_user(struct hwaa_init_user_t *usr)
{
	s32 ret = 0;

	if (!usr || !usr->key_desc ||
		(usr->key_desc_len < HWAA_USER_KEY_DESC_MIN) ||
		(usr->key_desc_len > HWAA_USER_KEY_DESC_MAX)) {
		hwaa_pr_err("hwaa_init_user_t was invalid!");
		ret = -EINVAL;
	}
	return ret;
}

void print_init_user_input(struct hwaa_init_user_t *usr)
{
	s32 hex_string_size = sizeof(s8) *
		(HEX_STR_PER_BYTE * usr->key_desc_len + 1);
	s8 *key_desc = (s8 *)kmalloc(hex_string_size, GFP_KERNEL);
	if (!key_desc)
		return;
	bytes2string(usr->key_desc, usr->key_desc_len, key_desc,
		hex_string_size);
	kzfree(key_desc);
	key_desc = NULL;
}

static s8 hex_char_to_int(s8 c)
{
	if ((c >= '0') && (c <= '9'))
		return (c - '0');
	if ((c >= 'A') && (c <= 'F'))
		return (c - 'A' + 10);
	if ((c >= 'a') && (c <= 'f'))
		return (c - 'a' + 10);
	hwaa_pr_err("invalid input detected in hexCharToInt [%d]\n", c);
	return 0;
}

static s8 *hwaa_hexstring_to_bytes_without_check(s8 *hex, s32 len)
{
	s32 i;
	s8 *ret = (s8 *)kmalloc(len / HEX_STR_PER_BYTE, GFP_KERNEL);
	if (ret == NULL)
		return NULL;
	for (i = 0 ; i < len ; i += HEX_STR_PER_BYTE) {
		ret[i / HEX_STR_PER_BYTE] = (s8)((hex_char_to_int(hex[i]) <<
			4) | hex_char_to_int(hex[i + 1]));
	}
	return ret;
}

s8 *hwaa_hexstring_to_bytes(s8 *hex, s32 len)
{
	if ((len <= 0) || (len > HWAA_MAX_BUFFER) || (hex == NULL))
		return NULL;

	return hwaa_hexstring_to_bytes_without_check(hex, len);
}

static u32 get_app_data_len(struct hwaa_package_info_t *pinfo)
{
	u32 shuid_bytes_len;
	u32 name_bytes_len;

	if (!pinfo)
		return 0;
	name_bytes_len = pinfo->name_len / HEX_STR_PER_BYTE;
	if (!pinfo->shuid) {
		shuid_bytes_len = PACKAGE_INFO_SHUID_LENGTH_FWK /
				  HEX_STR_PER_BYTE;
	} else {
		shuid_bytes_len = pinfo->shuid_len / HEX_STR_PER_BYTE;
	}
	return sizeof(uid_t) + sizeof(s64) + RESERVE_LENGTH +
		shuid_bytes_len + name_bytes_len;
}

static bool build_app_data(u8 *app_data, s32 app_data_length, uid_t uid,
	s64 ausn, const struct hwaa_package_info_t *pinfo,
	const u8 *shuid_bytes, u32 shuid_bytes_len, const u8 *name_bytes,
	u32 name_bytes_len)
{
	s32 length = app_data_length;
	if (memcpy_s(app_data, length, &uid, sizeof(uid_t)) != EOK)
		return false;
	app_data += sizeof(uid_t);
	length -= sizeof(uid_t);
	if (memcpy_s(app_data, length, &ausn, sizeof(s64)) != EOK)
		return false;
	app_data += sizeof(s64);
	length -= sizeof(s64);
	if (memcpy_s(app_data, length, pinfo->reserve, RESERVE_LENGTH)
		!= EOK)
		return false;
	app_data += RESERVE_LENGTH;
	length -= RESERVE_LENGTH;
	if (memcpy_s(app_data, length, shuid_bytes, shuid_bytes_len)
		!= EOK)
		return false;
	app_data += shuid_bytes_len;
	length -= shuid_bytes_len;
	if (memcpy_s(app_data, length, name_bytes, name_bytes_len)
		!= EOK)
		return false;

	return true;
}

static s32 get_app_data(u8 *app_data, s32 app_data_length,
	struct hwaa_package_info_t *pinfo, uid_t uid, s64 ausn)
{
	s32 ret = 0;
	u8 *shuid_bytes = NULL;
	u8 *name_bytes;
	u32 shuid_bytes_len = 0;
	u32 name_bytes_len = pinfo->name_len / HEX_STR_PER_BYTE;

	if (!app_data)
		return -EINVAL;
	name_bytes = hwaa_hexstring_to_bytes(pinfo->name, pinfo->name_len);
	if (!name_bytes) {
		hwaa_pr_err("Failed to convert pinfo name into bytes!");
		ret = -EINVAL;
		goto cleanup;
	}
	if (!pinfo->shuid) {
		shuid_bytes = kzalloc(SHA256_HASH_SIZE, GFP_KERNEL);
		if (!shuid_bytes) {
			hwaa_pr_err("Failed to allocate memory!");
			ret = -ENOMEM;
			goto cleanup;
		}
		shuid_bytes_len = PACKAGE_INFO_SHUID_LENGTH_FWK /
			HEX_STR_PER_BYTE;
	} else {
		shuid_bytes = hwaa_hexstring_to_bytes(pinfo->shuid,
			pinfo->shuid_len);
		if (!shuid_bytes) {
			hwaa_pr_err("Failed to convert pinfo shuid to bytes!");
			ret = -EINVAL;
			goto cleanup;
		}
		shuid_bytes_len = pinfo->shuid_len / HEX_STR_PER_BYTE;
	}
	if (!build_app_data(app_data, app_data_length, uid, ausn, pinfo,
		shuid_bytes, shuid_bytes_len, name_bytes, name_bytes_len))
		ret = -EINVAL;
cleanup:
	SAFE_FREE(name_bytes);
	SAFE_FREE(shuid_bytes);
	return ret;
}

static s32 hwaa_utils_get_app_data_without_check(u8 **app_data,
	u32 *app_data_len, struct hwaa_package_info_t *pinfo, uid_t uid)
{
	s32 ret;

	*app_data_len = get_app_data_len(pinfo);
	if (*app_data_len <= 0)
		return -EINVAL;
	*app_data = (u8 *)kzalloc(*app_data_len + HWAA_PERMISSION_LEN,
		GFP_KERNEL);
	if (*app_data == NULL) {
		hwaa_pr_err("Failed to allocate memory!");
		return -EINVAL;
	}
	ret = get_app_data(*app_data, *app_data_len, pinfo, uid,
		hwaa_utils_get_ausn(uid));
	if (ret) {
		kfree(*app_data);
		*app_data = NULL;
	}
	return ret;
}

s32 hwaa_utils_get_app_data(u8 **app_data, u32 *app_data_len,
	struct hwaa_package_info_t *pinfo, uid_t uid)
{
	if (!app_data || *app_data || !app_data_len)
		return -EINVAL;
	return hwaa_utils_get_app_data_without_check(app_data, app_data_len,
		pinfo, uid);
}

void hwaa_utils_free_package_info(struct hwaa_package_info_t *pinfo)
{
	if (!pinfo)
		return;
	kzfree(pinfo->name);
	pinfo->name = NULL;
	kzfree(pinfo->shuid);
	pinfo->shuid = NULL;
	kzfree(pinfo->app_policy);
	pinfo->app_policy = NULL;
}


static bool hwaa_utils_exe_check_without_check_para(const s8 *exe_path,
	s32 exe_path_len, pid_t pid)
{
	bool is_matched = false;
	s8 *buf;
	s8 *path;
	struct file *exe_file;
	struct mm_struct *mm;
	struct task_struct *task;
	rcu_read_lock();
	task = find_task_by_vpid(pid);
	if (!task) {
		rcu_read_unlock();
		hwaa_pr_err("Failed to lookup running task! (pid:%d)", pid);
		return is_matched;
	}
	get_task_struct(task);
	rcu_read_unlock();
	mm = get_task_mm(task);
	if (mm) {
		buf = (s8 *)__get_free_page(GFP_KERNEL);
		if (buf == NULL)
			goto done_mmput;
		exe_file = get_mm_exe_file(mm);
		if (exe_file == NULL)
			goto done_free;
		path = file_path(exe_file, buf, PAGE_SIZE);
		if (path == NULL || IS_ERR(path))
			goto done_put;
		if (strncmp(path, exe_path, exe_path_len) == 0)
			is_matched = true;
		else
			hwaa_pr_err("arg dismatch %s", path);
	} else {
		put_task_struct(task);
		return is_matched;
	}
done_put:
	fput(exe_file);
done_free:
	free_page((unsigned long)buf);
done_mmput:
	mmput(mm); // only when mm is not null would this branch take affect
	put_task_struct(task);
	return is_matched;
}

s8 *get_cmdline_value(const s8 *cmdline)
{
	s8 *start;
	s8 *end;
	s8 *result;
	u32 len;
	if (!cmdline)
		return NULL;
	start = strstr(cmdline, PROCESS_INIT_TAG);
	if (!start)
		return NULL;
	start += PROCESS_INIT_TAG_LEN;
	end = strchr(start, ' ');
	if (!end)
		return NULL;
	len = end - start;
	result = (s8 *)kzalloc(len + 1, GFP_KERNEL);
	if (!result)
		return NULL;
	if (memcpy_s(result, len + 1, start, len) != EOK) {
		kfree(result);
		result = NULL;
	}
	return result;
}

bool hwaa_utils_exe_check(pid_t pid, const s8 *exe_path, s32 exe_path_len)
{
	if ((exe_path == NULL) || (exe_path_len > EXE_MAX_ARG_STRLEN) ||
		(exe_path_len < 0))
		return false;

	return hwaa_utils_exe_check_without_check_para(exe_path,
		exe_path_len, pid);
}

s32 hwaa_utils_copy_package_info(struct hwaa_package_info_t *dst,
	struct hwaa_package_info_t *src)
{
	// copy name
	dst->name_len = src->name_len;
	dst->name = (s8 *)kmalloc(dst->name_len + 1, GFP_KERNEL);
	if (!dst->name)
		goto cleanup;
	if (memcpy_s(dst->name, dst->name_len, src->name, src->name_len) != EOK)
		goto cleanup;
	dst->name[dst->name_len] = '\0';
	dst->app_policy_len = src->app_policy_len;
	dst->app_policy = (s8 *)kmalloc(dst->app_policy_len + 1, GFP_KERNEL);
	if (!dst->app_policy)
		goto cleanup;
	if (memcpy_s(dst->app_policy, dst->app_policy_len, src->app_policy,
		src->app_policy_len) != EOK)
		goto cleanup;
	dst->app_policy[dst->app_policy_len] = '\0';
	// copy shared uid (optional)
	dst->shuid_len = src->shuid_len;
	if (dst->shuid_len > 0) {
		dst->shuid = (s8 *)kmalloc(dst->shuid_len + 1, GFP_KERNEL);
		if (!dst->shuid)
			goto cleanup;
		if (memcpy_s(dst->shuid, dst->shuid_len, src->shuid,
			src->shuid_len) != EOK)
			goto cleanup;
		dst->shuid[dst->shuid_len] = '\0';
	}
	return 0;

cleanup:
	hwaa_pr_err("Failed to allocate memory!");
	hwaa_utils_free_package_info(dst);
	return -ENOMEM;
}

static s32 hwaa_utils_copy_package_name_from_user(
	struct hwaa_package_info_t *kpinfo,
	struct hwaa_package_info_t *upinfo)
{
	if ((!upinfo ->name) || (kpinfo->name_len < HWAA_PACKAGE_NAME_MIN) ||
		(kpinfo->name_len > HWAA_PACKAGE_NAME_MAX))
		return -EINVAL;

	kpinfo->name = (s8 *)kzalloc(kpinfo->name_len + 1, GFP_KERNEL);
	if (!kpinfo->name) {
		hwaa_pr_err("Failed while allocating memory in kernel space!");
		return -ENOMEM;
	}
	if (copy_from_user(kpinfo->name, upinfo->name, kpinfo->name_len)) {
		hwaa_pr_err("Failed while copying from user space!");
		return -EFAULT;
	}
	kpinfo->name[kpinfo->name_len] = '\0';
	return 0;
}

static s32 hwaa_utils_copy_shuid_from_user(struct hwaa_package_info_t *kpinfo,
	struct hwaa_package_info_t *upinfo, bool copy_extinfo)
{
	if (!copy_extinfo)
		return 0;
	// kpinfo->shuid_len certainly equals upinfo->shuid_len
	if ((kpinfo->shuid_len > 0) && upinfo->shuid) {
		if ((kpinfo->shuid_len < HWAA_PACKAGE_SHUID_MIN) ||
			(kpinfo->shuid_len > HWAA_PACKAGE_SHUID_MAX)) {
			return -EINVAL;
		}
		kpinfo->shuid = (s8 *)kzalloc(kpinfo->shuid_len + 1,
			GFP_KERNEL);
		if (!kpinfo->shuid) {
			hwaa_pr_err("Failed when alloc mem in kernel space!");
			return -ENOMEM;
		}
		if (copy_from_user(kpinfo->shuid, upinfo->shuid,
			kpinfo->shuid_len)) {
			hwaa_pr_err("Failed while copying from user space!");
			return -EFAULT;
		}
		kpinfo->shuid[kpinfo->shuid_len] = '\0';
	}
	return 0;
}

static s32 hwaa_utils_copy_policy_from_user(struct hwaa_package_info_t *kpinfo,
	struct hwaa_package_info_t *upinfo, bool copy_extinfo)
{
	if (!copy_extinfo)
		return 0;
	if (!upinfo->app_policy ||
		(kpinfo->app_policy_len < HWAA_POLICY_RULESET_MIN) ||
		(kpinfo->app_policy_len > HWAA_POLICY_RULESET_MAX))
		return -EINVAL;
	kpinfo->app_policy = (s8 *)kzalloc(kpinfo->app_policy_len + 1,
		GFP_KERNEL);
	if (!kpinfo->app_policy) {
		hwaa_pr_err("Failed while allocating memory in kernel space!");
		return -ENOMEM;
	}
	if (copy_from_user(kpinfo->app_policy, upinfo->app_policy,
		kpinfo->app_policy_len)) {
		hwaa_pr_err("Failed while copying from user space!");
		return -EFAULT;
	}
	kpinfo->app_policy[kpinfo->app_policy_len] = '\0';
	return 0;
}

s32 hwaa_utils_copy_package_info_from_user(struct hwaa_package_info_t *kpinfo,
	struct hwaa_package_info_t *upinfo, bool copy_extinfo)
{
	s32 ret;

	if (!kpinfo || !upinfo)
		return -EINVAL;
	// pre set NULL is to avoid freeing user memory under err cases
	kpinfo->app_policy = NULL;
	kpinfo->name = NULL;
	kpinfo->shuid = NULL;
	// copy package name from user space
	ret = hwaa_utils_copy_package_name_from_user(kpinfo, upinfo);
	if (ret != 0)
		goto cleanup;
	// copy shared uid from user space (optional)
	ret = hwaa_utils_copy_shuid_from_user(kpinfo, upinfo, copy_extinfo);
	if (ret != 0)
		goto cleanup;
	// copy policy from user space
	ret = hwaa_utils_copy_policy_from_user(kpinfo, upinfo, copy_extinfo);
	if (ret != 0)
		goto cleanup;
	return 0;

cleanup:
	hwaa_utils_free_package_info(kpinfo);
	return ret;
}

s32 hwaa_utils_copy_packages_from_user(struct hwaa_package_info_t *kpackages,
	struct hwaa_package_info_t *upackages, u32 package_count)
{
	s32 ret;
	s32 i;

	if (!kpackages || !upackages)
		return -EFAULT;
	if ((package_count <= 0) || (package_count > SYNC_PKG_CNT_MAX))
		return -EINVAL;
	for (i = 0; i < package_count; i++) {
		if (memcpy_s(&kpackages[i], sizeof(struct hwaa_package_info_t),
			&upackages[i], sizeof(struct hwaa_package_info_t))
			!= EOK)
			return -EINVAL;
		ret = hwaa_utils_copy_package_info_from_user(&kpackages[i],
			&upackages[i], true);
		if (ret)
			goto done;
		if (kpackages[i].name) {
			hwaa_pr_debug("%dth pkg info name %s", i,
				kpackages[i].name);
		}
		if (kpackages[i].shuid) {
			hwaa_pr_debug("%dth pkg info name %s", i,
				kpackages[i].shuid);
		}
		if (kpackages[i].app_policy) {
			hwaa_pr_debug("%dth pkg info policy %s", i,
				kpackages[i].app_policy);
		}
	}

done:
	return ret;
}

s64 hwaa_utils_get_ausn(uid_t uid)
{
	return (s64)(uid / HWAA_PER_USER_RANGE);
}

static s8 *hwaa_utils_get_json_start_str(const s8 *str, const s8 *key)
{
	s8 *pc_start_buf;
	// no need to check key
	if (!str || (strlen(str) > MAX_JSON_STR_LEN) ||
		(strlen(str) == 0))
		return NULL;
	pc_start_buf = strstr((s8 *)str, key);
	if (!pc_start_buf) {
		hwaa_pr_err("Faild get label \"%s\" content from json\n", str);
		return NULL;
	}
	return pc_start_buf;
}

s8 *hwaa_utils_get_json_str(const s8 *str, const s8 *key, u32 *out_len)
{
	s8 *pc_end_buf;
	s8 *pc_key_str_end = (s8 *)"\"";
	u32 ui_conten_len;
	u32 ui_offset;
	s8 ch;
	s8 *pc_content;
	s32 len;

	s8 *pc_start_buf = hwaa_utils_get_json_start_str(str, key);
	if (!pc_start_buf)
		return NULL;
	ui_offset = 0;
	len = strlen(key);
	ch = *(pc_start_buf + len + 1);// one indicates size of end "
	// continue until no space and :, multy : is also valid
	while ((ch == ' ') || (ch == '\n') || (ch == ':')) {
		ui_offset++;
		ch = *(pc_start_buf + len + ui_offset);
	}
	if (ch != '\"')
		goto error; // json formart err if " doesn't after :
	ui_offset++; // skip starting " of content
	pc_start_buf += len + ui_offset; // set start point at content
	pc_end_buf = strstr(pc_start_buf, pc_key_str_end);
	if (!pc_end_buf) {
		hwaa_pr_err("Faild to get label \"%s\" from json.\n", key);
		goto error;
	}
	/* get content */
	ui_conten_len = pc_end_buf - pc_start_buf;
	if ((ui_conten_len > strlen(pc_start_buf)) || (ui_conten_len == 0)) {
		hwaa_pr_err("json data len error %d", ui_conten_len);
		goto error;
	}
	pc_content = (s8 *)kzalloc(ui_conten_len + 1, GFP_KERNEL);
	if (!pc_content)
		goto error;
	if (memcpy_s(pc_content, ui_conten_len, pc_start_buf, ui_conten_len)
		!= EOK) {
		kzfree(pc_content);
		goto error;
	}
	*out_len = ui_conten_len;
	return pc_content;
error:
	hwaa_pr_err("str err as %s end", str);
	return NULL;
}
