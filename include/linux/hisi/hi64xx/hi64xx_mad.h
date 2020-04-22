/*
 * hi64xx_mad.h -- hi64xx mad driver
 *
 * Copyright (c) 2019 Hisilicon Technologies CO., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef _HI64XX_MAD_H
#define _HI64XX_MAD_H

#include <sound/soc.h>
#include <linux/hisi/hi64xx/hi64xx_irq.h>

void hi64xx_hook_pcm_handle(void);
int hi64xx_mad_init(struct hi64xx_irq *irq);
void hi64xx_mad_request_irq(void);
void hi64xx_mad_free_irq(void);
void hi64xx_mad_deinit(void);

#endif
