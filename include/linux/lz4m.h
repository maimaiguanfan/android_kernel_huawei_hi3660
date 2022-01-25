/*
 * Copyright (C) 2018 HUAWEI, Inc.
 *             http://www.huawei.com/
 *
 * Original code taken from 'lz4.h'
 */
/* LZ4 Kernel Interface
 *
 * Copyright (C) 2013, LG Electronics, Kyungsik Lee <kyungsik.lee@lge.com>
 * Copyright (C) 2016, Sven Schmidt <4sschmid@informatik.uni-hamburg.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This file is based on the original header file
 * for LZ4 - Fast LZ compression algorithm.
 *
 * LZ4 - Fast LZ compression algorithm
 * Copyright (C) 2011-2016, Yann Collet.
 * BSD 2-Clause License (http://www.opensource.org/licenses/bsd-license.php)
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *	* Redistributions of source code must retain the above copyright
 *	  notice, this list of conditions and the following disclaimer.
 *	* Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * You can contact the author at :
 *	- LZ4 homepage : http://www.lz4.org
 *	- LZ4 source repository : https://github.com/lz4/lz4
 */

#ifndef __LZ4M_H
#define __LZ4M_H
#include <stddef.h>
#include <linux/types.h>

#define memcpy __builtin_memcpy

#define LZ4M_HASH_BITS 10
#define LZ4M_HASH_SIZE (1 << LZ4M_HASH_BITS)
#define LZ4M_LBIT 3
#define LZ4M_MBIT 3
#define LZ4M_D_BITS (16 - LZ4M_LBIT - LZ4M_MBIT)
#define LZ4M_LSHIFT (16 - LZ4M_LBIT)
#define LZ4M_MSHIFT (16 - LZ4M_LBIT - LZ4M_MBIT)
#define LZ4M_LMASK ((1 << LZ4M_LBIT) - 1)
#define LZ4M_MMASK ((1 << LZ4M_MBIT) - 1)
#define LZ4M_DMASK ((1 << LZ4M_D_BITS) - 1)
#define LZ4M_TOKEN_LEN 2
#define LZ4M_MAX_PART2_LEN 10

#define LZ4M_RUN_MASK ((1U << LZ4M_MBIT) - 1)
#define LZ4M_NEW_MASK (((1U << LZ4M_MBIT) - 1)<<2)

#define LZ4M_PAGE_SIZE 4096
#define LZ4M_DST_BUF_MIN_SIZE (LZ4M_PAGE_SIZE + 256)
#define LZ4M_MFLIMIT 3 //size is (12Byte/sizeof(uint32_t))
#define LZ4M_MEM_COMPRESS (16384)

size_t lz4m_compress(const unsigned char *src_buffer, size_t src_size,
		unsigned char *dst_buffer, size_t *dst_size,
		uint16_t hash_table[LZ4M_HASH_SIZE]);
size_t lz4m_decompress(const uint8_t *src_buffer, size_t src_size,
		uint8_t *dst_buffer, size_t *dst_size);
#endif
