/*
 * stwlc33_fw_nvm.h
 *
 * nvm data array
 *
 * Copyright (c) 2012-2019 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#ifndef _STWLC33_FW_NVM_H_
#define _STWLC33_FW_NVM_H_

#include <stwlc33_fw_nvm_1100.h>

struct st_fw_nvm_info {
	const int sec_no;
	const unsigned char *sec_data;
	const unsigned int sec_size;
	int same_flag;
};

static struct st_fw_nvm_info st_fw_nvm_data[] = {
	{
		.sec_no = 0, /* sector No.0 */
		.sec_data = st_nvm_data00,
		.sec_size = ARRAY_SIZE(st_nvm_data00),
		.same_flag = 0,
	},
	{
		.sec_no = 1, /* sector No.1 */
		.sec_data = st_nvm_data01,
		.sec_size = ARRAY_SIZE(st_nvm_data01),
		.same_flag = 0,
	},
	{
		.sec_no = 2, /* sector No.2 */
		.sec_data = st_nvm_data02,
		.sec_size = ARRAY_SIZE(st_nvm_data02),
		.same_flag = 0,
	},
	{
		.sec_no = 3, /* sector No.3 */
		.sec_data = st_nvm_data03,
		.sec_size = ARRAY_SIZE(st_nvm_data03),
		.same_flag = 0,
	},
	{
		.sec_no = 4, /* sector No.4 */
		.sec_data = st_nvm_data04,
		.sec_size = ARRAY_SIZE(st_nvm_data04),
		.same_flag = 0,
	},
	{
		.sec_no = 5, /* sector No.5 */
		.sec_data = st_nvm_data05,
		.sec_size = ARRAY_SIZE(st_nvm_data05),
		.same_flag = 0,
	},
	{
		.sec_no = 6, /* sector No.6 */
		.sec_data = st_nvm_data06,
		.sec_size = ARRAY_SIZE(st_nvm_data06),
		.same_flag = 0,
	},
	{
		.sec_no = 7, /* sector No.7 */
		.sec_data = st_nvm_data07,
		.sec_size = ARRAY_SIZE(st_nvm_data07),
		.same_flag = 0,
	},
	{
		.sec_no = 8, /* sector No.8 */
		.sec_data = st_nvm_data08,
		.sec_size = ARRAY_SIZE(st_nvm_data08),
		.same_flag = 0,
	},
	{
		.sec_no = 10, /* sector No.10 */
		.sec_data = st_nvm_data10,
		.sec_size = ARRAY_SIZE(st_nvm_data10),
		.same_flag = 0,
	},
	{
		.sec_no = 12, /* sector No.12 */
		.sec_data = st_nvm_data12,
		.sec_size = ARRAY_SIZE(st_nvm_data12),
		.same_flag = 0,
	},
	{
		.sec_no = 13, /* sector No.13 */
		.sec_data = st_nvm_data13,
		.sec_size = ARRAY_SIZE(st_nvm_data13),
		.same_flag = 0,
	},
};

#endif /* _STWLC33_FW_NVM_H_ */
