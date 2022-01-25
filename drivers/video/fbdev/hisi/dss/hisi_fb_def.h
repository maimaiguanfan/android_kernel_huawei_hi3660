/* Copyright (c) 2013-2014, Hisilicon Tech. Co., Ltd. All rights reserved.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 and
* only version 2 as published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
* GNU General Public License for more details.
*
*/
#ifndef HISI_FB_DEF_H
#define HISI_FB_DEF_H

#include <linux/delay.h>
#include <linux/string.h>
#include <linux/platform_device.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <asm/bug.h>
#include <linux/printk.h>

#ifndef MAX
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#endif

#ifndef MIN
#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#endif

/* align */
#ifndef ALIGN_DOWN
#define ALIGN_DOWN(val, al)  ((val) & ~((typeof(val))(al)-1))
#endif
#ifndef ALIGN_UP
#define ALIGN_UP(val, al)    (((val) + ((al)-1)) & ~((typeof(val))(al)-1))
#endif

#ifndef BIT
#define BIT(x)  (1<<(x))
#endif

#ifndef IS_EVEN
#define IS_EVEN(x)  ((x) % 2 == 0)
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define KHZ	(1000)
#define MHZ	(1000 * 1000)

enum {
	WAIT_TYPE_US = 0,
	WAIT_TYPE_MS,
};

/*--------------------------------------------------------------------------*/
extern uint32_t hisi_fb_msg_level;

/*
 * Message printing priorities:
 * LEVEL 0 KERN_EMERG (highest priority)
 * LEVEL 1 KERN_ALERT
 * LEVEL 2 KERN_CRIT
 * LEVEL 3 KERN_ERR
 * LEVEL 4 KERN_WARNING
 * LEVEL 5 KERN_NOTICE
 * LEVEL 6 KERN_INFO
 * LEVEL 7 KERN_DEBUG (Lowest priority)
 */
#define HISI_FB_EMERG(msg, ...)    \
	do { if (hisi_fb_msg_level > 0)  \
		pr_emerg("[hisifb E]:%s: "msg, __func__, ## __VA_ARGS__); } while (0)
#define HISI_FB_ALERT(msg, ...)    \
	do { if (hisi_fb_msg_level > 1)  \
		pr_alert("[hisifb A]:%s: "msg, __func__, ## __VA_ARGS__); } while (0)
#define HISI_FB_CRIT(msg, ...)    \
	do { if (hisi_fb_msg_level > 2)  \
		pr_crit("[hisifb C]:%s: "msg, __func__, ## __VA_ARGS__); } while (0)
#define HISI_FB_ERR(msg, ...)    \
	do { if (hisi_fb_msg_level > 3)  \
		pr_err("[hisifb E]:%s: "msg, __func__, ## __VA_ARGS__); } while (0)
#define HISI_FB_WARNING(msg, ...)    \
	do { if (hisi_fb_msg_level > 4)  \
		pr_warning("[hisifb W]:%s: "msg, __func__, ## __VA_ARGS__); } while (0)
#define HISI_FB_NOTICE(msg, ...)    \
	do { if (hisi_fb_msg_level > 5)  \
		pr_info("[hisifb N]:%s: "msg, __func__, ## __VA_ARGS__); } while (0)
#define HISI_FB_INFO(msg, ...)    \
	do { if (hisi_fb_msg_level > 6)  \
		pr_info("[hisifb I]:%s: "msg, __func__, ## __VA_ARGS__); } while (0)
#define HISI_FB_DEBUG(msg, ...)    \
	do { if (hisi_fb_msg_level > 7)  \
		pr_info("[hisifb D]:%s: "msg, __func__, ## __VA_ARGS__); } while (0)

//pr_debug("[hisifb]:%s: "msg, __func__, ## __VA_ARGS__);

#define assert(expr) \
	if(!(expr)) { \
		pr_err("[hisifb]: assertion failed! %s,%s,%s,line=%d\n",\
			#expr, __FILE__, __func__, __LINE__); \
	}

#define HISI_FB_ASSERT(x)   assert(x)


/*--------------------------------------------------------------------------*/
//#define CONFIG_HISI_FB_DUMP_DSS_REG

#ifdef CONFIG_HISI_FB_DUMP_DSS_REG
#define outp32(addr, val) \
	do {\
		writel(val, addr);\
		pr_info("writel(0x%x, 0x%x);\n", val, addr);\
	} while (0)
#else
#define outp32(addr, val) writel(val, addr)
#endif

#define outp16(addr, val) writew(val, addr)
#define outp8(addr, val) writeb(val, addr)
#define outp(addr, val) outp32(addr, val)

#define inp32(addr) readl(addr)
#define inp16(addr) readw(addr)
#define inp8(addr) readb(addr)
#define inp(addr) inp32(addr)

#define inpw(port) readw(port)
#define outpw(port, val) writew(val, port)
#define inpdw(port) readl(port)
#define outpdw(port, val) writel(val, port)


#endif /* HISI_FB_DEF_H */
