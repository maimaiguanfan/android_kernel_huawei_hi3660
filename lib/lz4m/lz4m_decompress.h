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

#ifndef __LZ4M_DECOMPRESS_H
#define __LZ4M_DECOMPRESS_H

//#include <string.h>	 /* memset, memcpy */
#include <linux/lz4m.h>
#define FORCE_INLINE __always_inline


/*-************************************
 *	Basic Types
 **************************************/
#include <linux/types.h>

typedef	uint8_t BYTE;
typedef uint16_t U16;
typedef uint32_t U32;
typedef	int32_t S32;
typedef uint64_t U64;

/*-************************************
 *	Architecture specifics
 **************************************/
#if defined(CONFIG_64BIT)
#define LZ4M_ARCH64 1
#else
#define LZ4M_ARCH64 0
#endif

#if defined(__LITTLE_ENDIAN)
#define LZ4M_LITTLE_ENDIAN 1
#else
#define LZ4M_LITTLE_ENDIAN 0
#endif

/*-************************************
 *	Constants
 **************************************/
#define LZ4M_DEBUG  0
#define MINMATCH 4

#define WILDCOPYLENGTH 8
#define LASTLITERALS 5
#define MFLIMIT (WILDCOPYLENGTH + MINMATCH)

/*
 * ensure it's possible to write 2 x wildcopyLength
 * without overflowing output buffer
 */
#define MATCH_SAFEGUARD_DISTANCE  ((2 * WILDCOPYLENGTH) - MINMATCH)

#define HASH_UNIT sizeof(size_t)

#define KB (1 << 10)
#define MB (1 << 20)
#define GB (1U << 30)

/*-************************************
 *	Reading and writing into memory
 **************************************/
static FORCE_INLINE U16 LZ4M_readLE16(const void *memPtr)
{
	return *(const U16 *) memPtr;
}

static FORCE_INLINE void LZ4M_copy8(void *dst, const void *src)
{
	U64 a = *((const U64 *)src);
	*(U64 *)dst = a;
}

/*
 * customized variant of memcpy,
 * which can overwrite up to 7 bytes beyond dstEnd
 */
static FORCE_INLINE void LZ4M_wildCopy(void *dstPtr,
		const void *srcPtr, void *dstEnd)
{
	BYTE *d = (BYTE *)dstPtr;
	const BYTE *s = (const BYTE *)srcPtr;
	BYTE *const e = (BYTE *)dstEnd;

	do {
		LZ4M_copy8(d, s);
		d += 8;
		s += 8;
	} while (d < e);
}

#define DEBUGLOG(l, ...) {}	/* disabled */
#define LZ4M_STATIC_ASSERT(c)    BUILD_BUG_ON(!(c))

/*
 * no public solution to solve our requirement yet.
 * see: <required buffer size for LZ4M_decompress_safe_partial>
 *      https://groups.google.com/forum/#!topic/lz4c/_3kkz5N6n00
 */
static FORCE_INLINE int __lz4m_decompress_safe_partial(
		uint8_t         **dst_ptr,
		const uint8_t   **src_ptr,
		BYTE            *dst,
		int             outputSize,
		const void      *src,
		int             inputSize,
		int trusted)
{
	/* Local Variables */
	const BYTE *ip = *(const BYTE **) src_ptr;
	const BYTE *const iend = (const BYTE *) src + inputSize;

	BYTE *op = *(BYTE **) dst_ptr;
	BYTE *const oend = dst + outputSize;
	BYTE *cpy = NULL;

	static const unsigned int inc32table[] = { 0, 1, 2, 1, 0, 4, 4, 4 };
	static const int dec64table[] = { 0, 0, 0, -1, -4, 1, 2, 3 };

	/* Set up the "end" pointers for the shortcut. */
	const BYTE *const shortiend = iend - 24 /*maxLL*/ - 2 /*offset*/;
	const BYTE *const shortoend = oend - 24 /*maxLL*/ - 24 /*maxML*/;

	DEBUGLOG(5, "%s (srcSize:%i, dstSize:%i)", __func__,
			inputSize, outputSize);

	/* Empty output buffer */
	if ((!outputSize))
		return ((inputSize == 1) && (*ip == 0)) ? 0 : -2; //EINVAL;

	if ((!inputSize))
		return -2; //EINVAL;

	/* Main Loop : decode sequences */
	while (1) {
		size_t length;
		const BYTE *match = NULL;
		size_t offset;
		/* get literal length */
		unsigned int token = LZ4M_readLE16(ip);

		offset = (token & LZ4M_DMASK) << 2;

		token = token >> LZ4M_D_BITS;
		length = (token >> LZ4M_MBIT) << 2;
		ip += 2;
		/*
		 * A two-stage shortcut for the most common case:
		 * 1) If the literal length is 0..14, and there is enough
		 * space, enter the shortcut and copy 16 bytes on behalf
		 * of the literals (in the fast mode, only 8 bytes can be
		 * safely copied this way).
		 * 2) Further if the match length is 4..18, copy 18 bytes
		 * in a similar manner; but we ensure that there's enough
		 * space in the output for those 18 bytes earlier, upon
		 * entering the shortcut (in other words, there is a
		 * combined check for both stages).
		 */
		if (length < LZ4M_NEW_MASK &&
				/*
				 * strictly "less than" on input, to re-enter
				 * the loop with at least one byte
				 */
				((ip < shortiend) && (op <= shortoend))) {

			/* Copy the literals */
			LZ4M_wildCopy(op, ip, op + length);
			op += length;
			ip += length;

			/*
			 * The second stage:
			 * prepare for match copying, decode full info.
			 * If it doesn't work out, the info won't be wasted.
			 */
			length = token & LZ4M_MMASK; /* match length */
			match = op - offset;
			length <<= 2;

			/* Do not deal with overlapping matches. */
			if ((length < LZ4M_NEW_MASK) &&
					(offset >= 8) && (match >= dst)) {
				/* Copy the match. */
				LZ4M_copy8(op + 0, match + 0);
				if (length > 16) {
					LZ4M_copy8(op + 8, match + 8);
					LZ4M_copy8(op + 16, match + 16);
				} else if (length > 8) {
					LZ4M_copy8(op + 8, match + 8);
				}
				op += length;
				/* Both stages worked, load the next token. */
				continue;
			}
			/*
			 * The second stage didn't work out, but the info
			 * is ready. Propel it right to the point of match
			 * copying.
			 */
			goto _copy_match;
		}

		/* decode literal lengths */
		if (length == LZ4M_NEW_MASK) {
			unsigned int s;

			if ((!trusted && ip >= iend - LZ4M_RUN_MASK)) {
				/* overflow detection */
				goto _output_error;
			}

			do {
				s = *ip++;
				length += (s << 2);
			} while ((ip < iend - LZ4M_RUN_MASK) && (s == 255));

			if (!trusted) {
				if (((uintptr_t)(op) + length < (uintptr_t)op))
					/* overflow detection */
					goto _output_error;
				if (((uintptr_t)(ip) + length < (uintptr_t)ip))
					/* overflow detection */
					goto _output_error;
			}
		}

		/* copy literals */
		cpy = op + length;

		if ((cpy > oend - MFLIMIT)
			|| (ip + length > iend - (2 + 1 + LASTLITERALS))) {
			if (cpy > oend) {
				/*
				 * Partial decoding :
				 * stop in the middle of literal segment
				 */
				cpy = oend;
				length = oend - op;
			}

			if (!trusted && ip + length > iend) {
				/*
				 * Error :
				 * read attempt beyond
				 * end of input buffer
				 */
				goto _output_error;
			}

			memcpy(op, ip, length);
			ip += length;
			op += length;

			/* Necessarily EOF, due to parsing restrictions */
			if (cpy == oend)
				break;
		} else {
			/* may overwrite up to WILDCOPYLENGTH beyond cpy */
			LZ4M_wildCopy(op, ip, cpy);
			ip += length;
			op = cpy;
		}

		/*src end*/
		if (ip > iend-2)
			break;
		match = op - offset;

		/* get matchlength */
		length = (token & LZ4M_MMASK) << 2;

_copy_match:
		if (length == LZ4M_NEW_MASK) {
			unsigned int s;

			do {
				s = *ip++;

				if (!trusted && ip > iend - LASTLITERALS)
					goto _output_error;

				length += (s << 2);
			} while (s == 255);

			if ((!trusted &&
				(uintptr_t)(op) + length < (uintptr_t)op)) {
				/* overflow detection */
				goto _output_error;
			}
		}
		/* copy match within block */
		cpy = op + length;

		if (cpy > oend - MATCH_SAFEGUARD_DISTANCE) {
			size_t const mlen = length > (size_t)(oend - op) ?
					(size_t)(oend - op) : length;
			const BYTE * const matchEnd = match + mlen;
			BYTE * const copyEnd = op + mlen;

			if (matchEnd > op) {
				while (op < copyEnd)
					*op++ = *match++;
			} else {
				memcpy(op, match, mlen);
			}

			op = copyEnd;
			if (op == oend)
				break;

			continue;
		}

		if ((offset < 8)) {
			op[0] = match[0];
			op[1] = match[1];
			op[2] = match[2];
			op[3] = match[3];
			match += inc32table[offset];
			memcpy(op + 4, match, 4);
			match -= dec64table[offset];
		} else {
			LZ4M_copy8(op, match);
			match += 8;
		}

		op += 8;
		if ((cpy > oend - MATCH_SAFEGUARD_DISTANCE)) {
			BYTE * const oCopyLimit = oend - (WILDCOPYLENGTH - 1);

			if (!trusted && cpy > oend - LASTLITERALS) {
				/*
				 * Error : last LASTLITERALS bytes
				 * must be literals (uncompressed)
				 */
				goto _output_error;
			}

			if (op < oCopyLimit) {
				LZ4M_wildCopy(op, match, oCopyLimit);
				match += oCopyLimit - op;
				op = oCopyLimit;
			}
			while (op < cpy)
				*op++ = *match++;
		} else {
			LZ4M_copy8(op, match);
			if (length > 16)
				LZ4M_wildCopy(op + 8, match + 8, cpy);
		}
		op = cpy; /* wildcopy correction */
	}

	/* end of decoding */
	/* Nb of output bytes decoded */
	*src_ptr = ip;
	*dst_ptr = op;
	return 0;

	/* Overflow error detected */
_output_error:
	return -1;
}

#endif

