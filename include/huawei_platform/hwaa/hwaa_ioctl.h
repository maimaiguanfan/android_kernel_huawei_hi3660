
#ifndef _HWAA_IOCTL_H
#define _HWAA_IOCTL_H

#ifdef __KERNEL__
#include <linux/types.h>
#include <linux/ioctl.h>
#include "huawei_platform/hwaa/hwaa_error.h"
#include "huawei_platform/hwaa/hwaa_limits.h"
#else
#include <stdbool.h>
#include <sys/ioctl.h>
#include "hwaa_error.h"
#include "hwaa_limits.h"
#endif

/* device name */
#ifndef HWAA_DEVICE_NAME
#define HWAA_DEVICE_NAME "hwaa"
#endif

/* permission codes */
#define HWAA_NO_PERMISSIONS 0
#define HWAA_RW_IMAGE 1
#define HWAA_RW_VIDEO 2
#define RESERVE_LENGTH 22
#define PACKAGE_INFO_SHUID_LENGTH_FWK 64

#define SHA256_HASH_SIZE 32

/* permission response codes */
typedef enum {
	HWAA_GRANT,
	HWAA_DENY,
	HWAA_INVALID_PERM_RESPONSE /* must be last */
} hwaa_perm_resp_code_t;

#pragma pack(push, 1)
struct appid_without_ext_signer_t {
	uid_t uid; /* package uid */
	s64 profile_id;
	u8 reserve[RESERVE_LENGTH];
	/* sha-256(shared uid (android manifest)) */
	s8 shuid[SHA256_HASH_SIZE];
	s8 name[SHA256_HASH_SIZE]; /* sha-256(package name) */
};
#pragma pack(pop)

/* user space shared data structures */
struct hwaa_package_info_t {
	s32 appid; /* in: unique package identifier  */
	s8 *name; /* sha-256(package name) */
	u64 name_len; /* character count of package name */
	s8 *shuid; /* sha-256(shared uid (android manifest)) */
	u64 shuid_len; /* character count of shared uid */
	s8 *app_policy;
	u64 app_policy_len;
	u8 reserve[RESERVE_LENGTH];
};

struct hwaa_init_user_t {
	s64 ausn; /* in: android user serial number */
	u8 *key_desc; /* in: key descriptor */
	u64 key_desc_len; /* in: length of key_desc key */
	hwaa_result_t ret; /* out: return code */
};

struct hwaa_init_tee_t {
	hwaa_result_t ret; /* out: return code */
};

struct hwaa_clear_key_t {
	u64 ausn; /* in: android user serial number */
	/* in: remove key in file or only remove key in cache */
	s32 is_remove_file;
	hwaa_result_t ret; /* out: return code */
};

struct hwaa_sync_installed_packages_t {
	struct hwaa_package_info_t *packages; /* list of installed packages */
	u32 package_count; /* in: number of packages */
	hwaa_result_t ret; /* out: return code */
};

struct hwaa_install_package_t {
	struct hwaa_package_info_t pinfo; /* package to install */
	hwaa_result_t ret; /* out: return code */
};

struct hwaa_uninstall_package_t {
	struct hwaa_package_info_t pinfo; /* package to uninstall */
	hwaa_result_t ret; /* out: return code */
};

/* magic number */
#define HWAA_MAGIC 0x2001

/* ioctl commands */
#define HWAA_INITIALIZE_USER _IOWR(HWAA_MAGIC, 1, struct hwaa_init_user_t *)
#define HWAA_INIT_TEE _IOR(HWAA_MAGIC, 12, struct hwaa_init_tee_t *)
#define HWAA_CLEAR_KEY _IOR(HWAA_MAGIC, 13, struct hwaa_clear_key_t *)
#define HWAA_SYNC_INSTALLED_PACKAGES _IOWR( \
	HWAA_MAGIC, 14, struct hwaa_sync_installed_packages_t *)
#define HWAA_INSTALL_PACKAGE _IOWR(HWAA_MAGIC, 15, \
	struct hwaa_install_package_t *)
#define HWAA_UNINSTALL_PACKAGE _IOWR(HWAA_MAGIC, 16, \
	struct hwaa_uninstall_package_t *)


void init_user(struct hwaa_init_user_t *iusr);

void hwaa_sync_installed_packages(
	struct hwaa_sync_installed_packages_t *sync_installed_packages);

void hwaa_install_package(struct hwaa_install_package_t *install_package);

void hwaa_uninstall_package(struct hwaa_uninstall_package_t *uninstall_package);

#endif
