#ifndef _HISI_PMIC_SOH_H_
#define _HISI_PMIC_SOH_H_


#include <linux/mfd/hisi_pmic.h>
#include <pmic_interface.h>

#ifdef CONFIG_HISI_COUL_HI6421V700
#include "hisi_hi6421v700_soh.h"
#elif defined(CONFIG_HISI_COUL_HI6421V800)
#include "hisi_hi6421v800_soh.h"
#elif defined(CONFIG_HISI_COUL_HI6555V300)
#include "hisi_hi6555v300_soh.h"
#endif

/*reg read and write interface macro */
#define HISI_SOH_PMIC_REG_READ(regAddr)             hisi_pmic_reg_read(regAddr)
#define HISI_SOH_PMIC_REG_WRITE(regAddr,regval)     hisi_pmic_reg_write((int)(regAddr),(int)regval)
#define HISI_SOH_PMIC_REGS_READ(regAddr,buf,size)   hisi_pmic_array_read((int)(regAddr),(char*)(buf),(int)(size))
#define HISI_SOH_PMIC_REGS_WRITE(regAddr,buf,size)  hisi_pmic_array_write((int)(regAddr),(char*)(buf),(int)(size))
#define SOH_PMIC_INFO
#ifndef SOH_PMIC_INFO
/*pimc soh print interface*/
#define hisi_pmic_soh_err(fmt,args...)              do {} while (0)
#define hisi_pmic_soh_evt(fmt,args...)              do {} while (0)
#define hisi_pmic_soh_inf(fmt,args...)              do {} while (0)
#define hisi_pmic_soh_debug(fmt,args...)            /*do { printk(KERN_ERR    "[hisi_soh_pmic:debug]" fmt, ## args); } while (0) */

#else
/*pimc soh print interface*/
#define hisi_pmic_soh_err(fmt,args...)              do { printk(KERN_ERR    "[hisi_soh_pmic]" fmt, ## args); } while (0)
#define hisi_pmic_soh_evt(fmt,args...)              do { printk(KERN_WARNING"[hisi_soh_pmic]" fmt, ## args); } while (0)
#define hisi_pmic_soh_inf(fmt,args...)              do { printk(KERN_INFO   "[hisi_soh_pmic]" fmt, ## args); } while (0)
#define hisi_pmic_soh_debug(fmt,args...)            /*do { printk(KERN_ERR    "[hisi_soh_pmic:debug]" fmt, ## args); } while (0) */

#endif

/*acr mul sel*/
enum acr_mul {
	ACR_MUL_35  = 0,
	ACR_MUL_70  = 1,
	ACR_MUL_140 = 2,
	ACR_MUL_280 = 3,
	ACR_MUL_MAX,
};

enum soh_thd_type {
    VOL_MV = 0,
    TEMP,
};

struct soh_pmic_device {
    int pmic_acr_support;
    int pmic_dcr_support;
    int pmic_pd_leak_support;
    int pmic_soh_ovp_support;
    int soh_ovh_irq;
    int soh_ovh_dis_irq;
    struct device *dev;
};

#endif


