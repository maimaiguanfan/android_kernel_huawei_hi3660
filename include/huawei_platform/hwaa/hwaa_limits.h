
#ifndef _HWAA_LIMITS_H
#define _HWAA_LIMITS_H

#ifdef __KERNEL__
#include <linux/limits.h>
#else
#include <sys/limits.h>
#endif

#ifndef HWAA_USER_KEY_DESC_MIN
#define HWAA_USER_KEY_DESC_MIN 1
#endif

#ifndef HWAA_USER_KEY_DESC_MAX
#define HWAA_USER_KEY_DESC_MAX 8
#endif

#ifndef HWAA_ENCODED_WFEK_SIZE
#define HWAA_ENCODED_WFEK_SIZE 251
#endif

#ifndef HWAA_FEK_SIZE_MAX
#define HWAA_FEK_SIZE_MAX 64
#define HWAA_FEK_SIZE 64
#endif

#ifndef HWAA_AES_IV_LENGTH
#define HWAA_AES_IV_LENGTH 16
#endif

#endif
