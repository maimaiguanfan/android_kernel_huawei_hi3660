/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2017-2020. All rights reserved.
 * Description: The purpose of this interface is to search whether a given
 *     package is in the apps whitelist. The whitelist was maintained by
 *     Access Control Module.
 *
 *     This file is released under the GPL v2.
 */


#ifndef __ACM_F2FS_H__
#define __ACM_F2FS_H__

#define ACM_PKGNAME_MAX 100
#define UID_BOUNDARY 10000

#define ACM_OP_DEL	0
#define ACM_OP_CRT	1
#define ACM_OP_MKD	2
#define ACM_OP_RNM	3
#define ACM_OP_DEL_DMD	4
int acm_search(char *pkgname, struct dentry *dentry, uid_t uid, int file_type,
	       int op);

#endif /* __ACM_H__ */
