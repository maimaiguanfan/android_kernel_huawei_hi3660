/*
 *  mntn_factory.h
 */

#ifndef __MNTN_FACTORY_H__
#define __MNTN_FACTORY_H__

struct mntn_factory_stru {
	unsigned char *mntn_mem_addr;
};

/* 1K DUMP SIZE FOR MNTN FACTORY */
/* ---- head ---- | ---- 32 ---- */
/* -- testpoint - | ---- 128 --- */
/* -- reserved -- | ---- 864 --- */
#define MNTN_FACTORY_HEAD_OFFSET  0
#define MNTN_FACTORY_HEAD_SIZE    32

#define MNTN_FACTORY_TESTPOINT_RECORD_OFFSET (MNTN_FACTORY_HEAD_OFFSET + MNTN_FACTORY_HEAD_SIZE)
#define MNTN_FACTORY_TESTPOINT_RECORD_SIZE   128

#define MNTN_FACTORY_RESERVED_OFFSET (MNTN_FACTORY_TESTPOINT_RECORD_OFFSET + MNTN_FACTORY_TESTPOINT_RECORD_SIZE)
#define MNTN_FACTORY_RESERVED_SIZE   864

#endif
