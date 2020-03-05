/*
 * kcode.c
 *
 * the kcode.c for kernel code integrity checking
 *
 * Yongzheng Wu <Wu.Yongzheng@huawei.com>
 * likun <quentin.lee@huawei.com>
 * likan <likan82@huawei.com>
 *
 * Copyright (c) 2001-2021, Huawei Tech. Co., Ltd. All rights reserved.
 */

#include "./include/kcode.h"

#ifdef CONFIG_HW_ROOT_SCAN_ENG_DEBUG
#define MAX_CODE_SIZE (50000000)
#else
#define MAX_CODE_SIZE (30000000)
#endif

static const char *TAG = "kcode";

static const struct memrange {
	char *start;
	char *end;
} ranges[] = {
	{_stext, _etext},
	{NULL, NULL}
};

static const int memrange_num = ARRAY_SIZE(ranges) - 1;
static size_t memrange_size;

/* Do sanity check on _stext, __v7_setup_stack, _etext.
 * The size of android-msm-hammerhead-3.4-lollipop-release is 14583504.
 * We think it's good as long as the size is less than 30M.
 */
static int kcode_verify_ranges(void)
{
	const struct memrange *range;
	char *ptr = NULL;

	memrange_size = 0;
	for (range = ranges; range->start != NULL; range++) {
		if (range->end <= range->start) {
			RSLogError(TAG, "range error 1, start=%pK, end=%pK",
						range->start, range->end);
			return 1;
		}

		if ((unsigned)((range->end - range->start) - 1) > MAX_CODE_SIZE ||
		    (uintptr_t)range->start % 4 ||
		    (uintptr_t)range->end % 4) {
			RSLogError(TAG, "range error 2, start=%pK, end=%pK",
						range->start, range->end);
			return 1;
		}

		if ((NULL != ptr) && range->start <= ptr) {
			RSLogError(TAG, "range error 3, prev=%pK, start=%pK",
						ptr, range->start);
			return 1;
		}
		ptr = range->end;
		memrange_size += (unsigned)(range->end - range->start);
	}

	if (memrange_size > MAX_CODE_SIZE) {
		RSLogError(TAG, "range error 4, memrange_size=%zu",
							memrange_size);
		return 1;
	}
	return 0;
}

int kcode_scan(uint8_t *hash)
{
	int i;
	int err;
	struct crypto_shash *tfm = crypto_alloc_shash("sha256", 0, 0);
	SHASH_DESC_ON_STACK(shash, tfm);

	if (IS_ERR(tfm)) {
		RSLogError(TAG, "crypto_alloc_hash(sha256) error %ld",
							PTR_ERR(tfm));
		return -ENOMEM;
	}

	if (memrange_size == 0){
			if (kcode_verify_ranges()){
					crypto_free_shash(tfm);
					return -ENOMEM;
			}
	}

	shash->tfm = tfm;
	shash->flags = 0;

	err = crypto_shash_init(shash);
	if (err < 0) {
		RSLogError(TAG, "crypto_shash_init() error %d", err);
		crypto_free_shash(tfm);
		return err;
	}

	for (i = 0; NULL != ranges[i].start; i++)
	{
		crypto_shash_update(shash, (char *)ranges[i].start, (unsigned int)(ranges[i].end - ranges[i].start));
	}

	err = crypto_shash_final(shash, (u8 *)hash);
	RSLogDebug(TAG, "kscan result %d", err);
	crypto_free_shash(tfm);
	return err;
}

size_t kcode_get_size(void)
{
	return (size_t)(_etext - _stext);
}

void kcode_copy(char *buffer)
{
	memcpy(buffer, _stext, (ulong)(_etext - _stext));
}

int kcode_syscall_scan(uint8_t *hash)
{
	size_t size;
	void *ptr = (void *)sys_call_table;
	int err;
	struct crypto_shash *tfm = crypto_alloc_shash("sha256", 0, 0);

	SHASH_DESC_ON_STACK(shash, tfm);

	if (IS_ERR(tfm)) {
		RSLogError(TAG, "crypto_alloc_hash(sha256) error %ld",
							PTR_ERR(tfm));
		return -ENOMEM;
	}

	shash->tfm = tfm;
	shash->flags = 0;

	err = crypto_shash_init(shash);
	if (err < 0) {
		RSLogError(TAG, "crypto_shash_init error %d", err);
		crypto_free_shash(tfm);
		return err;
	}

	/* define NR_syscalls as 326 */
	size = NR_syscalls * sizeof(void *);

	crypto_shash_update(shash, (char *)ptr, (unsigned int)size);
	err = crypto_shash_final(shash, (u8 *)hash);
	RSLogDebug(TAG, "syscallscan result %d", err);

	crypto_free_shash(tfm);
	return err;
}
