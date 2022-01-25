/*
 * Copyright (C) 2018-2019 HUAWEI, Inc.
 * http://www.huawei.com/
 *
 * Original code taken from 'lz4_decompress.c'
 */
/*
 * LZ4 - Fast LZ compression algorithm
 * Copyright (C) 2011 - 2016, Yann Collet.
 * BSD 2 - Clause License (http://www.opensource.org/licenses/bsd - license.php)
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
 *
 *	Changed for kernel usage by:
 *	Sven Schmidt <4sschmid@informatik.uni-hamburg.de>
 */

#include <stddef.h>
#include <linux/types.h>
#include <asm/simd.h>
#include "lz4m_decompress.h"
#define LZ4M_FAST_MARGIN		(128)

#if defined(CONFIG_ARM64) && defined(CONFIG_KERNEL_MODE_NEON)
#include "lz4mneon.h"
#endif


extern int _lz4m_decode_asm(uint8_t **dst_ptr, uint8_t *dst_begin,
		uint8_t *dst_end, const uint8_t **src_ptr,
		const uint8_t *src_end);

size_t lz4m_decompress(const uint8_t *src_buffer, size_t src_size,
		uint8_t *dst_buffer, size_t *dst_size)
{
	const uint8_t *src = src_buffer;
	uint8_t *dst = (uint8_t *)dst_buffer;
	int ret;

	/* Go fast if we can, keeping away from the end of buffers */
	if (*dst_size > LZ4M_FAST_MARGIN && src_size > LZ4M_FAST_MARGIN &&
	    may_use_simd() && cpu_has_neon()) {
		kernel_neon_begin();
		ret = _lz4m_decode_asm(&dst, (uint8_t *)dst_buffer,
			(uint8_t *)dst_buffer + *dst_size - LZ4M_FAST_MARGIN,
			&src,
			src_buffer + src_size - LZ4M_FAST_MARGIN);
		kernel_neon_end();
		if (ret)
			return -1;
	}
	ret = __lz4m_decompress_safe_partial((uint8_t **)&dst, &src,
			dst_buffer, *dst_size, src_buffer, src_size, 1);

	if (ret)
		return -2;

	*dst_size = (size_t)(dst - dst_buffer);
	return 0;
}

