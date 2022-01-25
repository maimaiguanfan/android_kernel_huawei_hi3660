/*
 * dm-row.c
 *
 * storage Redirection On Write device-mapper
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

#include <linux/blkdev.h>
#include <linux/device-mapper.h>
#include <linux/fs.h>
#include <linux/kdev_t.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/vmalloc.h>
#include <linux/ctype.h>
#include <linux/blk_types.h>

#define DM_MSG_PREFIX "row"

struct dm_row {
	struct rw_semaphore lock;

	struct dm_dev *origin;
	struct dm_dev *cow;

	struct dm_target *ti;

	/* List of rows per Origin */
	struct list_head list;

	/* You can't use a row if this is 0 (e.g. if full). */
	int valid;

	/* Origin writes don't trigger exceptions until this is set */
	int active;

	/* Size of data blocks saved - must be a power of 2 */
	unsigned int chunk_size; /* keep same type as bi_size */
	sector_t chunk_mask; /* keep same type as bi_sector */
	unsigned int chunk_shift;

	/* Hook flag for origin block device's mrfn */
	unsigned int mrfn_hook;

	/* make_request_fn of origin block device */
	make_request_fn *mrfn;

	/* Fast lookup bitmap for written blocks mapping */
	unsigned long *wr_bitmap;

	/* pointer to the DM device itself */
	struct block_device *bd_self;
};

/*
 * One of these per registered origin, held in the _origins hash
 */
struct origin {
	/* The origin device */
	struct block_device *bdev;

	struct list_head hash_list;

	/* List of rows for this origin */
	struct list_head rows;
};

/*
 * This structure is allocated for each origin target
 */
struct dm_origin {
	struct dm_dev *dev;
	struct dm_target *ti;
	struct list_head hash_list;
};

/*
 * Size of the hash table for origin volumes. If we make this
 * the size of the minors list then it should be nearly perfect
 */
#define ORIGIN_HASH_SIZE 256
#define ORIGIN_MASK      0xFF

static struct list_head *_origins;
static struct list_head *_dm_origins;
static struct rw_semaphore _origins_lock;

static inline int bdev_equal(struct block_device *lhs, struct block_device *rhs)
{
	/*
	 * There is only ever one instance of a particular block
	 * device so we can compare pointers safely.
	 */
	return lhs == rhs;
}

static int init_origin_hash(void)
{
	int i;

	_origins = kmalloc_array(ORIGIN_HASH_SIZE, sizeof(struct list_head),
			GFP_KERNEL);
	if (!_origins) {
		DMERR("unable to allocate memory for _origins");
		return -ENOMEM;
	}
	for (i = 0; i < ORIGIN_HASH_SIZE; i++)
		INIT_LIST_HEAD(_origins + i);

	_dm_origins = kmalloc_array(ORIGIN_HASH_SIZE, sizeof(struct list_head),
			GFP_KERNEL);
	if (!_dm_origins) {
		DMERR("unable to allocate memory for _dm_origins");
		kfree(_origins);
		return -ENOMEM;
	}
	for (i = 0; i < ORIGIN_HASH_SIZE; i++)
		INIT_LIST_HEAD(_dm_origins + i);

	init_rwsem(&_origins_lock);

	return 0;
}

static void exit_origin_hash(void)
{
	kfree(_origins);
	kfree(_dm_origins);
}

static unsigned int origin_hash(struct block_device *bdev)
{
	return (unsigned int)(bdev->bd_dev & ORIGIN_MASK);
}

static struct origin *__lookup_origin(struct block_device *origin)
{
	struct list_head *ol = NULL;
	struct origin *o = NULL;

	ol = &_origins[origin_hash(origin)];
	list_for_each_entry(o, ol, hash_list) {
		if (bdev_equal(o->bdev, origin))
			return o;
	}

	return NULL;
}

static void __insert_origin(struct origin *o)
{
	struct list_head *sl = &_origins[origin_hash(o->bdev)];

	list_add_tail(&o->hash_list, sl);
}

static void __insert_row(struct origin *o, struct dm_row *s)
{
	struct dm_row *l = NULL;

	/* Sort the list according to chunk size, largest-first smallest-last */
	list_for_each_entry(l, &o->rows, list) {
		if (l->chunk_size < s->chunk_size)
			break;
	}

	list_add_tail(&s->list, &l->list);
}

/*
 * Make a note of the row and its origin so we can look it
 * up when the origin has a write on it.
 *
 * Also validate row exception store handovers.
 * On success, returns 1 if this registration is a handover destination,
 * otherwise returns 0.
 */
static int register_row(struct dm_row *row)
{
	struct origin *o = NULL;
	struct origin *new_o = NULL;
	struct block_device *bdev = row->origin->bdev;
	int r = 0;

	new_o = kmalloc(sizeof(*new_o), GFP_KERNEL);
	if (!new_o)
		return -ENOMEM;

	down_write(&_origins_lock);

	o = __lookup_origin(bdev);
	if (o) {
		kfree(new_o);
	} else {
		/* New origin */
		o = new_o;

		/* Initialise the struct */
		INIT_LIST_HEAD(&o->rows);
		o->bdev = bdev;

		__insert_origin(o);
	}

	__insert_row(o, row);

	up_write(&_origins_lock);

	return r;
}

/*
 * Move row to correct place in list according to chunk size.
 */
static void reregister_row(struct dm_row *s)
{
	struct origin *o = NULL;
	struct block_device *bdev = s->origin->bdev;

	down_write(&_origins_lock);

	list_del(&s->list);

	o = __lookup_origin(bdev);
	if (o)
		__insert_row(o, s);

	up_write(&_origins_lock);
}

static void unregister_row(struct dm_row *s)
{
	struct origin *o = NULL;

	down_write(&_origins_lock);

	o = __lookup_origin(s->origin->bdev);

	list_del(&s->list);
	if (o && list_empty(&o->rows)) {
		list_del(&o->hash_list);
		kfree(o);
	}

	up_write(&_origins_lock);
}

/*
 * Methods implementing a ROW (Redirect On Write) type Device-Mapper
 */

#define HOOK_MAGIC      0x4B4F4F48

#define SECTORS_PER_PAGE_SHIFT  (PAGE_SHIFT - SECTOR_SHIFT)
#define SECTORS_PER_PAGE        (1 << SECTORS_PER_PAGE_SHIFT)

/*
 * Forward declaration and external reference
 */
static inline int dm_target_is_row_bio_target(struct target_type *tt);
static blk_qc_t row_make_request(struct request_queue *q, struct bio *bio);

static int dm_exception_store_build(struct dm_target *ti,
	int argc, char **argv, struct dm_row *row, unsigned int *args_used)
{
	int r = 0;
	char persistent;
	unsigned int chunk_size;

	if (argc < 2) {
		ti->error = "Insufficient exception store arguments";
		DMERR("Insufficient exception store arguments");
		return -EINVAL;
	}

	persistent = toupper(*argv[0]);
	if (persistent != 'P' && persistent != 'N') {
		ti->error = "Persistent flag is not P or N";
		DMERR("Persistent flag is not P or N");
		r = -EINVAL;
		goto bad_type;
	}

	if (kstrtouint(argv[1], 10, &chunk_size)) {
		ti->error = "Invalid chunk size";
		DMERR("Invalid chunk size");
		r = -EINVAL;
		goto bad_type;
	}

	if (!chunk_size) {
		row->chunk_size = 0;
		row->chunk_mask = 0;
		row->chunk_shift = 0;
		goto common_out;
	}

	/* Check chunk_size is a power of 2 */
	if (!is_power_of_2(chunk_size)) {
		ti->error = "Chunk size is not a power of 2";
		DMERR("Chunk size is not a power of 2");
		r = -EINVAL;
		goto bad_type;
	}

	if (chunk_size > INT_MAX >> SECTOR_SHIFT) {
		ti->error = "Chunk size is big";
		DMERR("Chunk size is big");
		r = -EINVAL;
		goto bad_type;
	}

	row->chunk_size = chunk_size;
	row->chunk_mask = chunk_size - 1;
	row->chunk_shift = ffs(chunk_size) - 1;

common_out:
	*args_used = 2;
	return 0;

bad_type:
	return r;
}

/*
 * Return the number of sectors in the device.
 */
static inline sector_t get_dev_size(struct block_device *bdev)
{
	return i_size_read(bdev->bd_inode) >> SECTOR_SHIFT;
}

static inline sector_t sector_to_chunk(struct dm_row *row, sector_t sector)
{
	return sector >> row->chunk_shift;
}

/*
 * Attach to the origin_dev request queue
 */
static void row_attach_queue(struct dm_row *row)
{
	struct request_queue *q = bdev_get_queue(row->origin->bdev);

	BUG_ON(q == NULL);

	if (dm_target_is_row_bio_target(row->ti->type)) {
		spin_lock_irq(q->queue_lock);
		if (row->mrfn_hook == 0) {
			row->mrfn = q->make_request_fn;
			q->make_request_fn = row_make_request;
			row->mrfn_hook = HOOK_MAGIC;
		}
		spin_unlock_irq(q->queue_lock);
	}
}

/*
 * Detach from the origin_dev request queue
 * stop mrfn entry and restore mrfn as the original mrfn.
 */
static void row_detach_queue(struct dm_row *row)
{
	struct request_queue *q = bdev_get_queue(row->origin->bdev);

	BUG_ON(q == NULL);

	if (dm_target_is_row_bio_target(row->ti->type)) {
		spin_lock_irq(q->queue_lock);
		if (row->mrfn_hook == HOOK_MAGIC) {
			q->make_request_fn = row->mrfn;
			row->mrfn_hook = 0;
		}
		spin_unlock_irq(q->queue_lock);
	}
}

static void row_fix_bio_cieinfo(struct bio *bio, const struct bio *base_bio)
{
#ifdef CONFIG_F2FS_FS_ENCRYPTION
	unsigned int bvec_off;
	sector_t sec_downrnd;

	bio->hisi_bio.ci_key = base_bio->hisi_bio.ci_key;
	bio->hisi_bio.ci_key_len = base_bio->hisi_bio.ci_key_len;
	bio->hisi_bio.ci_key_index = base_bio->hisi_bio.ci_key_index;
	bio->hisi_bio.index = 0;

	if (base_bio->hisi_bio.ci_key_len && base_bio->hisi_bio.ci_key) {
		/* bio vector offset, bio ci index is page aligned */
		sec_downrnd = base_bio->bi_iter.bi_sector & ~0x07;
		bvec_off = (bio->bi_iter.bi_sector - sec_downrnd) >> 3;
		bio->hisi_bio.index = base_bio->hisi_bio.index + bvec_off;
	}
#endif
}

static void row_skip_hisi_bio_count(struct bio *bio)
{
#ifdef CONFIG_HISI_BLK_CORE
	bio->hisi_bio.io_in_count &= ~HISI_IO_IN_COUNT_SET;
	bio->hisi_bio.io_in_count |= HISI_IO_IN_COUNT_SKIP_ENDIO;
#endif
}

/*
 * Origin device's make_request_fn replacer as bio transfer
 */
static blk_qc_t row_make_request(struct request_queue *q, struct bio *bio)
{
	struct origin *o = NULL;
	struct dm_row *s = NULL;
	struct mapped_device *md = NULL;
	struct request_queue *dm_q = NULL;

	/*
	 * if the bio has no sectors, blk_partition_remap does not do
	 * top partition remap, so we use bd_contains as top blkdev.
	 * while the bio has sectors, bi_sector and bi_bdev is top
	 * blkdev remapped.
	 */
	down_read(&_origins_lock);

	o = __lookup_origin(bio->bi_bdev->bd_contains);
	BUG_ON(o == NULL);

	/*
	 * make sure only one target now, or else bio should be
	 * duplicated to diverse to each row device.
	 */
	BUG_ON(!list_is_singular(&o->rows));

	s = list_first_entry(&o->rows, struct dm_row, list);
	md = dm_table_get_md(s->ti->table);
	BUG_ON(md == NULL);

	up_read(&_origins_lock);

	dm_q = bdev_get_queue(s->bd_self);
	dm_q->make_request_fn(dm_q, bio);

	return BLK_QC_T_NONE;
}

/*
 * non-exclusive get block_device
 */
static int dm_row_get_device(struct dm_target *ti,
	const char *path, fmode_t mode, struct dm_dev **result)
{
	dev_t dev;
	struct dm_dev *ddev = NULL;
	struct block_device *bdev = NULL;

	dev = dm_get_dev_t(path);
	if (!dev) {
		ti->error = "Get devt failed";
		DMERR("Get devt failed");
		return -ENODEV;
	}

	ddev = kmalloc(sizeof(*ddev), GFP_KERNEL);
	if (!ddev) {
		ti->error = "Alloc dm_dev failed";
		return -ENOMEM;
	}

	ddev->mode = mode;
	ddev->bdev = NULL;

	bdev = blkdev_get_by_dev(dev, ddev->mode, NULL);
	if (IS_ERR(bdev)) {
		ti->error = "Get blkdev by devt failed";
		DMERR("Get blkdev by devt failed");
		kfree(ddev);
		return PTR_ERR(bdev);
	}

	ddev->bdev = bdev;
	format_dev_t(ddev->name, dev);

	*result = ddev;
	return 0;
}

static void dm_row_put_device(struct dm_target *ti, struct dm_dev *d)
{
	(void)ti;

	if (!d->bdev)
		return;

	blkdev_put(d->bdev, d->mode);
	d->bdev = NULL;
	kfree(d);
}

/*
 * Construct a row mapping: <origin_dev> <COW-dev> <P/N> <chunk_size>
 */
static int row_ctr(struct dm_target *ti, unsigned int argc, char **argv)
{
	struct dm_row *s = NULL;
	char *origin_path = NULL;
	char *cow_path = NULL;
	dev_t origin_dev;
	dev_t cow_dev;
	struct origin *o = NULL;
	unsigned int args_used;
	unsigned long bmp_len;
	fmode_t origin_mode = FMODE_READ;
	int err = -EINVAL;

	DMINFO("%s: Construct row mapped-device enter", __func__);

	if (argc != 4) {
		ti->error = "DM row requires exactly 4 arguments";
		DMERR("DM row requires exactly 4 arguments");
		goto bad;
	}

	s = kmalloc(sizeof(struct dm_row), GFP_KERNEL);
	if (!s) {
		ti->error = "Cannot allocate private dm_row structure";
		DMERR("Cannot allocate private dm_row structure");
		err = -ENOMEM;
		goto bad;
	}

	origin_path = argv[0];
	argv++;
	argc--;

	err = dm_row_get_device(ti, origin_path, origin_mode, &s->origin);
	if (err) {
		ti->error = "Cannot get origin device";
		DMERR("Cannot get origin device");
		goto bad_origin;
	}
	origin_dev = s->origin->bdev->bd_dev;

	down_read(&_origins_lock);
	o = __lookup_origin(s->origin->bdev);
	up_read(&_origins_lock);
	if (o) {
		ti->error = "Existing dm-row for origin device";
		DMERR("Existing dm-row for origin device");
		goto bad_origin;
	}

	cow_path = argv[0];
	argv++;
	argc--;

	cow_dev = dm_get_dev_t(cow_path);
	if (cow_dev && cow_dev == origin_dev) {
		ti->error = "COW device cannot be the same as origin device";
		DMERR("COW device cannot be the same as origin device");
		goto bad_cow;
	}

	err = dm_get_device(ti, cow_path, dm_table_get_mode(ti->table),
		&s->cow);
	if (err) {
		ti->error = "Cannot get COW device";
		DMERR("Cannot get COW device");
		goto bad_cow;
	}

	err = dm_exception_store_build(ti, argc, argv, s, &args_used);
	if (err) {
		ti->error = "Couldn't create exception store";
		DMERR("Couldn't create exception store");
		goto bad_store;
	}
	if (s->chunk_size != SECTORS_PER_PAGE) {
		ti->error = "Chunk size must equal to PAGE_SIZE";
		DMERR("Chunk size must equal to PAGE_SIZE");
		err = -EINVAL;
		goto bad_store;
	}

	argv += args_used;
	argc -= args_used;

	s->ti = ti;
	s->valid = 1;
	s->active = 0;
	init_rwsem(&s->lock);
	INIT_LIST_HEAD(&s->list);

	/* Allocate bitmap table for COW data */
	bmp_len = BITS_TO_LONGS(sector_to_chunk(s, ti->len));
	/*
	 * for a 16TB disk, needed bitmap size is 16TB / (4K * 8) = 512MB.
	 * use that as an upper limit
	 */
#define ROW_BITMAP_LEN (512 * 1024 * 1024UL)
	if (bmp_len > (ROW_BITMAP_LEN / sizeof(long))) {
		ti->error = "Origin disk size is too large";
		DMERR("Origin disk size is too large");
		err = -EINVAL;
		goto bad_alloc_bitmap;
	}
	s->wr_bitmap = kcalloc(bmp_len, sizeof(long), GFP_KERNEL);
	if (!s->wr_bitmap) {
		ti->error = "Could not allocate block mapping bitmap";
		DMERR("Could not allocate block mapping bitmap");
		err = -ENOMEM;
		goto bad_alloc_bitmap;
	}

	s->bd_self = NULL;
	s->mrfn_hook = 0;

	ti->private = s;
	ti->num_flush_bios = 1;
	ti->flush_supported = true;
	ti->num_discard_bios = 1;
	ti->discards_supported = false;
	ti->split_discard_bios = false;
	ti->discard_zeroes_data_unsupported = false;
	ti->per_io_data_size = 0;

	/* Add row to the list of snapshots for this origin */
	err = register_row(s);
	if (err == -ENOMEM) {
		ti->error = "Register dm-row failed";
		DMERR("Register dm-row failed");
		goto bad_load_and_register;
	}

	err = dm_set_target_max_io_len(ti, s->chunk_size);
	if (err) {
		ti->error = "Set dm-row target max io len failed";
		DMERR("Set dm-row target max io len failed");
		goto bad_set_max;
	}

	DMINFO("%s: Construct row mapped-device ok", __func__);

	return 0;

bad_set_max:
	unregister_row(s);

bad_load_and_register:
	kfree(s->wr_bitmap);

bad_alloc_bitmap:
bad_store:
	dm_put_device(ti, s->cow);

bad_cow:
	dm_row_put_device(ti, s->origin);

bad_origin:
	kfree(s);

bad:
	return err;
}

static void row_status(struct dm_target *ti, status_type_t type,
	unsigned int status_flags, char *result, unsigned int maxlen)
{
	struct dm_row *row = ti->private;
	unsigned int sz = 0;

	DMINFO("%s: Status query row mapped-device", __func__);

	switch (type) {
	case STATUSTYPE_INFO:

		down_read(&row->lock);

		if (!row->valid)
			DMEMIT("Invalid");
		else
			DMEMIT("Unknown");

		up_read(&row->lock);

		break;

	case STATUSTYPE_TABLE:
		/*
		 * kdevname returns a static pointer so we need
		 * to make private copies if the output is to
		 * make sense.
		 */
		down_read(&row->lock);

		DMEMIT("%s %s", row->origin->name, row->cow->name);
		DMEMIT(" N %llu", (unsigned long long)row->chunk_size);

		up_read(&row->lock);

		break;
	}
}

static int row_iterate_devices(struct dm_target *ti,
	iterate_devices_callout_fn fn, void *data)
{
	struct dm_row *row = ti->private;
	int r;

	r = fn(ti, row->origin, 0, ti->len, data);

	if (!r)
		r = fn(ti, row->cow, 0, get_dev_size(row->cow->bdev), data);

	return r;
}

static int row_preresume(struct dm_target *ti)
{
	int r = 0;
	struct dm_row *s = ti->private;

	(void)s;
	return r;
}

static void row_resume(struct dm_target *ti)
{
	struct dm_row *s = ti->private;
	int err = 0;

	DMINFO("%s: Resume row mapped-device", __func__);

	/* Now we have correct chunk size, reregister */
	reregister_row(s);

	down_write(&s->lock);
	s->active = 1;
	up_write(&s->lock);

	/* Hold the dm device and its queue */
	down_write(&s->lock);

	if (s->bd_self != NULL)
		goto out_unlock;
	s->bd_self = bdget_disk(dm_disk(dm_table_get_md(ti->table)), 0);
	if (s->bd_self == NULL) {
		DMERR("Get dm-row device blockdev failed");
		goto out_unlock;
	}
	err = blkdev_get(s->bd_self, FMODE_READ | FMODE_WRITE, NULL);
	if (err) {
		DMERR("Open dm-row device itself failed");
		/* bd has been put in __blkdev_get if failed */
		s->bd_self = NULL;
		goto out_unlock;
	}

out_unlock:
	up_write(&s->lock);
}

/*
 * Release the row dm itself before suspend
 */
static void row_presuspend(struct dm_target *ti)
{
	struct dm_row *s = ti->private;

	DMINFO("%s: Presuspend row mapped-device", __func__);

	down_write(&s->lock);

	s->active = 0;

	if (s->bd_self) {
		blkdev_put(s->bd_self, FMODE_READ | FMODE_WRITE);
		s->bd_self = NULL;
	}

	up_write(&s->lock);
}

static void row_dtr(struct dm_target *ti)
{
	struct dm_row *s = ti->private;

	DMINFO("%s: Deconstruct row mapped-device", __func__);

	/* Invalidate the device first */
	down_write(&s->lock);
	s->valid = 0;
	up_write(&s->lock);

	/* Prevent further origin writes from using this row. */
	unregister_row(s);

	/*
	 * Ensure instructions in mempool_destroy aren't reordered
	 * before atomic_read.
	 */
	smp_mb();

	kfree(s->wr_bitmap);

	dm_put_device(ti, s->cow);

	/*
	 * detach the original device queue when destructed
	 * during row and original device put. have no other chance
	 * to get the detach_queue message.
	 */
	row_detach_queue(s);

	dm_row_put_device(ti, s->origin);

	kfree(s);
}

static int row_message(struct dm_target *ti, unsigned int argc, char **argv)
{
	struct dm_row *s = ti->private;

	DMINFO("%s: Message row mapped-device", __func__);

	if (argc != 1) {
		DMERR("%s: Invalid message count : %u", __func__, argc);
		return -EINVAL;
	}

	if (!strcasecmp(argv[0], "attach_queue")) {
		if (s->active == 0) {
			DMERR("Device inactive for message '%s'", argv[0]);
			return -EINVAL;
		}
		row_attach_queue(s);
		return 0;
	} else if (!strcasecmp(argv[0], "detach_queue")) {
		/* shall know what you are doing when using this message */
		if (s->active == 0) {
			DMERR("Device inactive for message '%s'", argv[0]);
			return -EINVAL;
		}
		row_detach_queue(s);
		return 0;
	} else {
		DMERR("%s: Unsupported message '%s'", __func__, argv[0]);
		return -EINVAL;
	}
}

/*
 * A copy version of same routine in bio.c
 * Even we call mrfn directly, we can not sure the lower does not
 * call generic_make_request.
 * The safe way is to call generic_make_request and use bi_end_io callback
 */
struct submit_bio_ret {
	struct completion event;
	int error;
};

static void submit_bio_wait_endio(struct bio *bio)
{
	struct submit_bio_ret *ret = bio->bi_private;

	ret->error = bio->bi_error;
	complete(&ret->event);
}

static int submit_bio_mkreq_wait(make_request_fn *mkreq_fn,
	struct request_queue *q, struct bio *bio)
{
	struct submit_bio_ret ret;

	init_completion(&ret.event);
	bio->bi_private = &ret;
	bio->bi_end_io = submit_bio_wait_endio;
	mkreq_fn(q, bio);
	wait_for_completion(&ret.event);

	return ret.error;
}

/*
 * bio_alloc_pages - allocates a single page for each bvec in a bio
 * @bio: bio to allocate pages for
 * @page_nr: number of pages to add, may less than @bio->bi_max_vecs
 *
 * Allocates pages and update @bio->bi_vcnt.
 *
 * Returns 0 on success, On failure, any allocated pages are freed.
 */
static int bio_add_pages(struct bio *bio, unsigned int pg_num)
{
	struct page *page = NULL;
	struct bio_vec *bvec = NULL;
	int i;
	int err = 0;

	for (i = 0; i < pg_num; ++i) {
		page = alloc_page(GFP_KERNEL);
		if (!page) {
			err = -ENOMEM;
			break;
		}

		if (!bio_add_page(bio, page, PAGE_SIZE, 0)) {
			__free_page(page);
			err = -EIO;
			break;
		}
	}

	if (err) {
		bio_for_each_segment_all(bvec, bio, i)
			__free_page(bvec->bv_page);
	}

	return err;
}

/*
 * A copy version of the same routine in bio.c
 */
static void bio_delete_pages(struct bio *bio)
{
	struct bio_vec *bvec = NULL;
	int i;

	bio_for_each_segment_all(bvec, bio, i)
		__free_page(bvec->bv_page);
}

/*
 * Only used to check the cloned split bio which has only one
 * bio_vec and is no more than PAGE_SZIE
 */
static inline int row_bio_full_chunk(struct dm_row *row, struct bio *bio)
{
	unsigned int chunk_size = row->chunk_size << SECTOR_SHIFT;

	/* redundant check condition */
	if (bio->bi_iter.bi_size == chunk_size &&
		bio_iter_len(bio, bio->bi_iter) == chunk_size &&
		bio_offset(bio) == 0)
		return 1;
	else
		return 0;
}

/*
 * Read the whole chunk from original device and fill the cow device.
 * just for special case of some superblock write and real vfat fs write.
 * eg. sb may not page aligned and only has 2 sectors.
 * Because the sectors of chunk is ensured same as SECTORS_PER_PAGE,
 * the cieinfo fix is safe.
 */
static int row_read_fill_chunk(struct dm_row *s, struct bio *ref)
{
	struct bio *bio = NULL;
	sector_t sector;
	unsigned int pg_cnt;
	struct request_queue *q = NULL;
	int err = 0;

	pg_cnt = s->chunk_size >> SECTORS_PER_PAGE_SHIFT;

	bio = bio_alloc(GFP_KERNEL, pg_cnt);
	if (!bio) {
		DMERR("%s: Allocate bio failed", __func__);
		return -ENOMEM;
	}

	/*
	 * bi_bdev and bi_iter.bi_sector must have values for
	 * linux version < 4.4
	 */
	sector = ref->bi_iter.bi_sector & ~s->chunk_mask;
	bio->bi_bdev = s->origin->bdev;
	bio_set_op_attrs(bio, REQ_OP_READ, READ_SYNC);
	bio->bi_iter.bi_sector = sector;

	err = bio_add_pages(bio, pg_cnt);
	if (err) {
		DMERR("%s: Allocate or add bio pages failed", __func__);
		goto free_bio;
	}

	row_fix_bio_cieinfo(bio, ref);
	row_skip_hisi_bio_count(bio);

	/* read from original device */
	q = bdev_get_queue(s->origin->bdev);
	err = submit_bio_mkreq_wait(s->mrfn, q, bio);
	if (err) {
		DMERR("%s: Queue bio[READ] to origin dev failed", __func__);
		goto free_pages;
	}

	/* write to cow device */
	bio_reset(bio);
	bio->bi_bdev = s->cow->bdev;
	bio_set_op_attrs(bio, REQ_OP_WRITE, WRITE_SYNC);
	bio->bi_iter.bi_sector = sector;
	bio->bi_iter.bi_size = pg_cnt * PAGE_SIZE;

	/* reassign bi_vcnt reset by bio_reset */
	bio->bi_vcnt = pg_cnt;

	row_fix_bio_cieinfo(bio, ref);

	/*
	 * invoke the dest target mrfn directly rather than
	 * call generic_make_request which put the request to
	 * current->bio_list and only return to wait
	 * when the current request is blocked
	 */
	row_skip_hisi_bio_count(bio);
	q = bdev_get_queue(s->cow->bdev);
	err = submit_bio_mkreq_wait(q->make_request_fn, q, bio);
	if (err)
		DMERR("%s: Queue bio[WRITE] to cow dev failed", __func__);

free_pages:
	bio_delete_pages(bio);

free_bio:
	bio_put(bio);
	return err;
}

static int row_map(struct dm_target *ti, struct bio *bio)
{
	struct dm_row *s = ti->private;
	int err = DM_MAPIO_REMAPPED;
	sector_t chunk;

	if (s->mrfn == NULL) {
		DMERR("%s: Original rfn shall NOT be null", __func__);
		BUG_ON(s->mrfn == NULL);
	}

	down_write(&s->lock);

	/* To get here the table must believe so s->active is always set */
	if (!s->valid || !s->active) {
		DMERR("%s: Device dm-row is NOT valid or active", __func__);
		err = -EIO;
		goto out_unlock;
	}

	/*
	 * zero count sectors condition includes condition of
	 * bio->bi_iter.bi_sector == (sector_t)-1
	 * has no data, ensured in __send_empty_flush @ dm.c
	 */
	if (bio_sectors(bio) == 0 || (bio->bi_opf & REQ_PREFLUSH))
		goto out_endio;

	chunk = sector_to_chunk(s, bio->bi_iter.bi_sector);

	if (unlikely(bio_op(bio) == REQ_OP_DISCARD)) {
		sector_t chunk_end;

		/*
		 * redirect discarded sectors chunk to cow dev with sector
		 * range as large as possible. no need to split bio because
		 * no sectors payload.
		 */
		chunk_end = sector_to_chunk(s, bio_end_sector(bio));
		while (chunk < chunk_end) {
			set_bit(chunk, s->wr_bitmap);
			++chunk;
		}

		bio->bi_bdev = s->cow->bdev;
		goto out_remapped;
	}

	/* Fix and add the inline cryption key info HERE to the cloned bio */
	row_fix_bio_cieinfo(bio, dm_get_tio_bio(bio));

	/* If the block is already remapped - use that, or else remap it */
	if (test_bit(chunk, s->wr_bitmap)) {
		bio->bi_bdev = s->cow->bdev;
		goto out_remapped;
	}
	/*
	 * the above is that if bitmap set, read/write data from/to cow.
	 * or else, set bitmap and write to cow device, read from
	 * original device for read request
	 */

	if (bio_data_dir(bio) == WRITE) {
		if (unlikely(row_bio_full_chunk(s, bio) == 0)) {
			int res;

			DMINFO("%s: Recved bio[WRITE] is not chunk aligned",
				__func__);
			res = row_read_fill_chunk(s, bio);
			if (res) {
				DMERR("%s: Read and fill chunk failed",
					__func__);
				err = res;
				goto out_unlock;
			}
		}

		set_bit(chunk, s->wr_bitmap);
		bio->bi_bdev = s->cow->bdev;
		goto out_remapped;
	} else {
		bio->bi_bdev = s->origin->bdev;
		row_skip_hisi_bio_count(bio);
		/*
		 * always SUBMITTED for avoid infinite remapping loop.
		 * we can do some duplicate check both in mrfn redirection pipe
		 * and in this map handler. that is not a problem,
		 * but we want to simply use the mrfn pipe implementation
		 */
		s->mrfn(bdev_get_queue(bio->bi_bdev), bio);
		goto out_submitted;
	}

out_endio:
	row_skip_hisi_bio_count(bio);
	bio->bi_error = 0;
	bio_endio(bio);
out_submitted:
	err = DM_MAPIO_SUBMITTED;
	goto out_unlock;

out_remapped:
	/*
	 * this out_remapped is remapped out to dm cow in fact.
	 * these small cloned BIOs including discard bios have not once
	 * entered the queue and can pass the generic_make_request_checks.
	 */
	err = DM_MAPIO_REMAPPED;

out_unlock:
	up_write(&s->lock);
	return err;
}

static struct target_type row_target = {
	.name    = "row",
	.version = {1, 0, 0},
	.module  = THIS_MODULE,
	.ctr     = row_ctr,
	.dtr     = row_dtr,
	.map     = row_map,
	.presuspend = row_presuspend,
	.preresume  = row_preresume,
	.message = row_message,
	.resume  = row_resume,
	.status  = row_status,
	.iterate_devices = row_iterate_devices,
};

static inline int dm_target_is_row_bio_target(struct target_type *tt)
{
	return (tt == &row_target);
}

static int __init dm_row_init(void)
{
	int r;

	r = dm_register_target(&row_target);
	if (r < 0) {
		DMERR("DM row target register failed, err %d", r);
		return r;
	}

	r = init_origin_hash();
	if (r) {
		DMERR("DM row init origin hash failed, err %d", r);
		goto bad_origin_hash;
	}

	return 0;

bad_origin_hash:
	dm_unregister_target(&row_target);

	return r;
}

static void __exit dm_row_exit(void)
{
	dm_unregister_target(&row_target);

	exit_origin_hash();
}

/* Module hooks */
module_init(dm_row_init);
module_exit(dm_row_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION(DM_NAME " row target driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
MODULE_ALIAS("dm-row");
