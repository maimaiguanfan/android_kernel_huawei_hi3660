
#ifndef _HWAA_FILE_PERM_H
#define _HWAA_FILE_PERM_H

#include <linux/types.h>

/*
 * permissions format
 *
 * 0                   1                   2                   3
 * 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |usr|g0 |  g0id   |g1 |  g1id   |oth|g2 |  g2id   |g3 |  g3id   |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 */

#define HWAA_FILE_PERM_GET_USER(perms) ((perms) & 0x03UL)

#define HWAA_FILE_PERM_SET_USER(perms, other_perms) ((perms) = \
	((other_perms) | (perms)))

#define HWAA_FILE_PERM_SET_USER_READ(perms) ((perms) = (0x01UL | (perms)))

#define HWAA_FILE_PERM_SET_USER_WRITE(perms) ((perms) = (0x02UL | (perms)))

#define HWAA_FILE_PERM_GET_OTHER(perms) (((perms) & (0x03UL << 16)) >> 16)

#define HWAA_FILE_PERM_SET_OTHER(perms, other_perms) ((perms) = \
	(((other_perms) << 16) | (perms)))

#define HWAA_FILE_PERM_SET_OTHER_READ(perms) ((perms) = \
	((0x01UL << 16) | (perms)))

#define HWAA_FILE_PERM_SET_OTHER_WRITE(perms) ((perms) = ((0x02UL << 16) | \
	(perms)))

#endif
