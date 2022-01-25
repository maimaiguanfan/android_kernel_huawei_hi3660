/*
 * Copyright (C) 2018 HUAWEI, Inc.
 *             http://www.huawei.com/
 *
 * Original code taken from 'lz4.c'
 */
/*
 * Cryptographic API.
 *
 * Copyright (c) 2013 Chanho Min <chanho.min@lge.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 51
 * Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 *
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/crypto.h>
#include <linux/vmalloc.h>
#include <linux/lz4m.h>

struct lz4m_ctx {
	void *lz4m_comp_mem;
};

static int lz4m_init(struct crypto_tfm *tfm)
{
	struct lz4m_ctx *ctx = crypto_tfm_ctx(tfm);

	ctx->lz4m_comp_mem = vmalloc(LZ4M_MEM_COMPRESS);
	if (!ctx->lz4m_comp_mem)
		return -ENOMEM;

	return 0;
}

static void lz4m_exit(struct crypto_tfm *tfm)
{
	struct lz4m_ctx *ctx = crypto_tfm_ctx(tfm);

	vfree(ctx->lz4m_comp_mem);
}

static int lz4m_compress_crypto(struct crypto_tfm *tfm, const u8 *src,
		unsigned int slen, u8 *dst, unsigned int *dlen)
{
	struct lz4m_ctx *ctx = crypto_tfm_ctx(tfm);
	size_t tmp_len = *dlen;
	int err;

	err = lz4m_compress(src, slen, dst, &tmp_len, ctx->lz4m_comp_mem);

	if (err < 0)
		return -EINVAL;

	*dlen = tmp_len;
	return 0;
}

static int lz4m_decompress_crypto(struct crypto_tfm *tfm, const u8 *src,
		unsigned int slen, u8 *dst, unsigned int *dlen)
{
	int err;
	size_t tmp_len = *dlen;
	size_t __slen = slen;

	err = lz4m_decompress(src, __slen, dst, &tmp_len);
	if (err < 0)
		return -EINVAL;

	*dlen = tmp_len;
	return err;
}

static struct crypto_alg alg_lz4m = {
	.cra_name		= "lz4m",
	.cra_flags		= CRYPTO_ALG_TYPE_COMPRESS,
	.cra_ctxsize		= sizeof(struct lz4m_ctx),
	.cra_module		= THIS_MODULE,
	.cra_list		= LIST_HEAD_INIT(alg_lz4m.cra_list),
	.cra_init		= lz4m_init,
	.cra_exit		= lz4m_exit,
	.cra_u			= { .compress = {
		.coa_compress		= lz4m_compress_crypto,
		.coa_decompress		= lz4m_decompress_crypto } }
};

static int __init lz4m_mod_init(void)
{
	return crypto_register_alg(&alg_lz4m);
}

static void __exit lz4m_mod_fini(void)
{
	crypto_unregister_alg(&alg_lz4m);
}

module_init(lz4m_mod_init);
module_exit(lz4m_mod_fini);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("LZ4M Compression Algorithm");
MODULE_ALIAS_CRYPTO("lz4m");
