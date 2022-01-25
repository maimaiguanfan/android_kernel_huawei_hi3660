/*
 * Copyright (C) 2018 HUAWEI, Inc.
 *             http://www.huawei.com/
 *
 * Original code taken from 'lz4_compress.c'
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

#include <linux/lz4m.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/kernel.h>

#define FORCE_INLINE __always_inline

static FORCE_INLINE uint32_t lz4m_hash(uint32_t x)
{
	return ((x * 2654435761U) >> (32 - LZ4M_HASH_BITS));
}

/*
 * The length of latteral and match are measured in 4 bytes.
 * The length of dst is measured in 1 bytes.
 */
static FORCE_INLINE uint8_t *encode_sequence(
	uint16_t l_len, uint16_t m_len,
	uint16_t offset, const uint32_t *literal,
	uint8_t *dst, uint32_t dst_len)
{
	uint32_t len;

	//If dst buffer not enough, avoiding overflows, return NULL
	if (LZ4M_TOKEN_LEN + LZ4M_MAX_PART2_LEN + (l_len << 2) > dst_len)
		return NULL;

	//token
	*(uint16_t *)dst =
		(((l_len >= LZ4M_LMASK) ? LZ4M_LMASK : l_len) << LZ4M_LSHIFT) |
		(((m_len >= LZ4M_MMASK) ? LZ4M_MMASK : m_len) << LZ4M_MSHIFT) |
		(offset & LZ4M_DMASK);
	dst += LZ4M_TOKEN_LEN;

	//latteral length part2
	if (l_len >= LZ4M_LMASK) {
		for (len = l_len - LZ4M_LMASK; len >= 255; len -= 255)
			*dst++ = 255;
		*dst++ = (uint8_t)len;
	}

	//letterals
	if (l_len > 0) {
		uint64_t *d = (uint64_t *)dst;
		uint64_t *s = (uint64_t *)literal;

		for (len = 1; len < l_len; len += 2)
			*d++ = *s++;
		if (len == l_len)
			*(uint32_t *)d = *(uint32_t *)s;
		dst += (l_len << 2);
	}

	//match length part2
	if (m_len >= LZ4M_MMASK) {
		for (len = m_len - LZ4M_MMASK; len >= 255; len -= 255)
			*dst++ = 255;
		*dst++ = (uint8_t)len;
	}

	return dst;
}

/*
 * The @src_size no larger than 4K, and 4 byte aligned
 */
size_t lz4m_compress(const unsigned char *src_buffer, size_t src_size,
		unsigned char *dst_buffer, size_t *dst_size,
		uint16_t hash_table[LZ4M_HASH_SIZE])
{
	const uint32_t *src_base = (uint32_t *)src_buffer;
	const uint32_t *src_end = (uint32_t *)(src_buffer + src_size);
	const uint32_t *src = (uint32_t *)src_buffer;
	uint8_t *dst = (uint8_t *)dst_buffer;
	uint8_t *dst_end = dst_buffer + *dst_size;
	const uint32_t *src_anchor = NULL; //last time encoded source pos
	const uint32_t *match = NULL; //matched pos
	uint16_t offset;
	uint32_t hash;


	if ((src_buffer == NULL) || (dst_buffer == NULL) || (dst_size == NULL))
		return -1;

	if ((src_size & 0x3) || (src_size > LZ4M_PAGE_SIZE) ||
		(*dst_size < LZ4M_DST_BUF_MIN_SIZE))
		return -1;

	{//same as memset(hash_table, 0, sizeof(uint16_t) * LZ4M_HASH_SIZE);
		uint64_t *h_t = (uint64_t *)hash_table;
		int i;

		for (i = 0; i < (LZ4M_HASH_SIZE >> 2);) {
			h_t[i++] = 0;
			h_t[i++] = 0;
			h_t[i++] = 0;
			h_t[i++] = 0;
			h_t[i++] = 0;
			h_t[i++] = 0;
			h_t[i++] = 0;
			h_t[i++] = 0;
		}
	}

	hash = lz4m_hash(*src);
	hash_table[hash] = (uint16_t)(src - src_base);
	src_anchor = src++;
	src_end -= LZ4M_MFLIMIT;

	while (dst < dst_end) {
		const uint32_t *src_exp = NULL;

		if (src >= src_end) {
			//the last sequence
			dst = encode_sequence(
				(uint32_t)(src_end + LZ4M_MFLIMIT - src_anchor),
				0, 0, src_anchor, dst,
				(uint32_t)(dst_end - dst));
			if (dst == NULL)
				return -1;
			break;
		}

		//Get a match
		do {
			hash = lz4m_hash(*src);
			match = src_base + hash_table[hash];
			hash_table[hash] = (uint16_t)(src - src_base);
			if (*src == *match)
				break;
			src++;
		} while (src < src_end);

		//Did not match, go to encode the last sequence
		if (src >= src_end)
			continue;

		offset = src - match;//Get offset

		//Expand the match string and get the match length
		src_exp = src + 1;
		match++;
		while ((src_exp < src_end) && (*src_exp == *match)) {
			src_exp++;
			match++;
		}
		match = src - offset;
		while ((src > src_anchor) && (match > src_base) &&
			(*(src - 1) == *(match - 1))) {
			src--;
			match--;
		}

		//Encode sequence
		dst = encode_sequence((uint16_t)(src - src_anchor),
			(uint16_t)(src_exp - src), offset, src_anchor,
			dst, (uint32_t)(dst_end - dst));
		if (dst == NULL)
			return -1;

		src = src_exp;
		src_anchor = src;
	}

	*dst_size = (size_t)(dst - dst_buffer);
	return 0;
}
EXPORT_SYMBOL(lz4m_compress);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("LZ4M compressor");
