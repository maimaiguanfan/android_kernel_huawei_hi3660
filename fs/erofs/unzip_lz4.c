// SPDX-License-Identifier: GPL-2.0
/*
 * linux/drivers/staging/erofs/unzip_lz4.c
 *
 * Copyright (C) 2018 HUAWEI, Inc.
 *             http://www.huawei.com/
 * Created by Gao Xiang <gaoxiang25@huawei.com>
 */
#include "generic/lz4.h"
#include "lz4armv8/lz4accel.h"

#define LZ4_FAST_MARGIN                (128)

static ssize_t __maybe_unused __lz4_decompress_safe_partial_trusted(
	void            *dest,
	size_t          outputSize,
	const void      *source,
	size_t          inputSize,
	bool		accel)
{
	uint8_t         *dstPtr = dest;
	const uint8_t   *srcPtr = source;
	ssize_t         ret;

#ifdef __ARCH_HAS_LZ4_ACCELERATOR
	/* Go fast if we can, keeping away from the end of buffers */
	if (outputSize > LZ4_FAST_MARGIN && inputSize > LZ4_FAST_MARGIN &&
	    accel && lz4_decompress_accel_enable()) {
	        ret = lz4_decompress_asm(&dstPtr, dest,
					 dest + outputSize - LZ4_FAST_MARGIN,
					 &srcPtr,
					 source + inputSize - LZ4_FAST_MARGIN);
		if (ret)
			return -1;
	}
#endif
	/* Finish in safe */
	return __lz4_decompress_safe_partial(dstPtr, srcPtr, dest, outputSize,
					     source, inputSize, true);
}

static ssize_t __maybe_unused __lz4_decompress_safe_partial_untrusted(
	void            *dest,
	size_t          outputSize,
	const void      *source,
	size_t          inputSize,
	bool		accel)
{
	uint8_t         *dstPtr = dest;
	const uint8_t   *srcPtr = source;
	ssize_t         ret;

#ifdef __ARCH_HAS_LZ4_ACCELERATOR
	/* Go fast if we can, keeping away from the end of buffers */
	if (outputSize > LZ4_FAST_MARGIN && inputSize > LZ4_FAST_MARGIN &&
	    accel && lz4_decompress_accel_enable()) {
	        ret = lz4_decompress_asm(&dstPtr, dest,
					 dest + outputSize - LZ4_FAST_MARGIN,
					 &srcPtr,
					 source + inputSize - LZ4_FAST_MARGIN);
		if (ret)
			return -1;
	}
#endif
	/* Finish in safe */
	return __lz4_decompress_safe_partial(dstPtr, srcPtr, dest, outputSize,
					     source, inputSize, false);
}

int z_erofs_unzip_lz4(void *in, void *out, size_t inlen,
		      size_t outlen, bool accel)
{
	ssize_t ret;

#ifdef CONFIG_EROFS_FS_DEBUG
	ret = __lz4_decompress_safe_partial_untrusted(out, outlen, in, inlen, accel);
#else
	ret = __lz4_decompress_safe_partial_trusted(out, outlen, in, inlen, accel);
#endif

	if (ret >= 0)
		return (int)ret;

	/*
	 * LZ4_decompress_safe will return an error code
	 * (< 0) if decompression failed
	 */
	errln("%s, failed to decompress, in[%p, %zu] outlen[%p, %zu]",
	      __func__, in, inlen, out, outlen);
	WARN_ON(1);
	print_hex_dump(KERN_DEBUG, "raw data [in]: ", DUMP_PREFIX_OFFSET,
		16, 1, in, inlen, true);
	print_hex_dump(KERN_DEBUG, "raw data [out]: ", DUMP_PREFIX_OFFSET,
		16, 1, out, outlen, true);
	return -EIO;
}

