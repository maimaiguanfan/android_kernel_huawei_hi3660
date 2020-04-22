#ifndef _HISI_SCHARGER_SOH_H_
#define _HISI_SCHARGER_SOH_H_

#include <soc_schargerV600_interface.h>
#include <./../../charger/schargerV600/hisi_scharger_v600.h>

#ifndef BIT
#define BIT(x)      (1 << (x))
#endif
#define scharger_soh_err(fmt,args...)              do { printk(KERN_ERR    "[hisi_soh_scharger]" fmt, ## args); } while (0)
#define scharger_soh_evt(fmt,args...)              do { printk(KERN_WARNING"[hisi_soh_scharger]" fmt, ## args); } while (0)
#define scharger_soh_inf(fmt,args...)              do { printk(KERN_INFO   "[hisi_soh_scharger]" fmt, ## args); } while (0)
//#define scharger_soh_debug(fmt,args...)            /*do { printk(KERN_ERR    "[scharger_soh:debug]" fmt, ## args); } while (0)*/

/*acr macro definition start */
#define ACR_EN_ADDR                                 SOC_SCHARGER_ACR_CTRL_ADDR(0)
#define ACR_EN                                BIT(SOC_SCHARGER_ACR_CTRL_sc_acr_en_START)
#define ACR_FLAG_INT_ADDR                          SOC_SCHARGER_IRQ_FLAG_5_ADDR(0)
#define ACR_FLAG_INT_BIT                            BIT(1)

#define ACR_FLAG_INT_MASK_REG                       SOC_SCHARGER_IRQ_MASK_5_ADDR(0)
#define ACR_FLAG_INT_MASK_BIT                       BIT(1)

#define ACR_MUL_SEL_ADDR                          SOC_SCHARGER_ACR_TOP_CFG_REG_0_ADDR(0)
#define ACR_MUL_MASK                                0xfc
#define ACR_MUL_SHIFT                               (SOC_SCHARGER_ACR_TOP_CFG_REG_0_da_acr_mul_sel_START)

#define ACR_H_DATA_BASE                             SOC_SCHARGER_ACR_DATA0_H_ADDR(0)
#define ACR_L_DATA_BASE                             SOC_SCHARGER_ACR_DATA0_L_ADDR(0)
#define ACR_DATA_REG_NUM                            2

#define ACR_DATA_L_SHIFT                            4
#define ACR_DATA_H_SHIFT                            8
#define ACR_DATA_FIFO_DEPTH                         8

#define ACR_CAL_MAGNIFICATION                       1000000
#define ACR_MOHM_TO_UOHM                            1000
/*acr macro definition end */

/*acr mul sel*/
enum acr_mul {
    ACR_MUL_35  = 0,
    ACR_MUL_70  = 1,
    ACR_MUL_140 = 2,
    ACR_MUL_280 = 3,
    ACR_MUL_MAX,
};
struct soh_scharger_device {
    int scharger_acr_support;
    struct device *dev;
};

#endif


