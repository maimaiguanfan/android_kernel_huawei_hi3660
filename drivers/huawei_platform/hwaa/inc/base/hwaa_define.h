/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: This file contains the function definations.
 * Create: 2020-10-10
 */

#ifndef _HWAA_DEFINE_H
#define _HWAA_DEFINE_H

// constant
#define USER_KEY_LENGTH 64
// Length of one AES block
#define AES_BLOCK_LENGTH 16
#define PHASE_1_KEY_LENGTH 32

// error code
enum {
	ERR_MSG_SUCCESS = 0,
	ERR_MSG_OUT_OF_MEMORY = 1,
	// A pointer (that should not be null) is null
	ERR_MSG_NULL_PTR = 2,
	ERR_MSG_BAD_PARAM = 3,
	ERR_MSG_KERNEL_PHASE1_KEY_NULL = 4,
	ERR_MSG_KERNEL_PHASE1_KEY_NOTMATCH = 5,
	ERR_MSG_LIST_NODE_ALREADY_IN = 6,
	ERR_MSG_LIST_NODE_NOT_EXIST  = 7,
	ERR_MSG_GENERATE_FAIL = 8,
	ERR_MSG_LIST_EMPTY = 9,
};

#endif
