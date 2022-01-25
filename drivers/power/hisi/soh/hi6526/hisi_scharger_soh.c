
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_platform.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/of_irq.h>
#include <linux/delay.h>
#include <linux/syscalls.h>
#include <linux/module.h>
#include <linux/init.h>
#include "hisi_scharger_soh.h"
#include <./../hisi_soh_core.h>
#include <linux/gpio.h>
#include <linux/power/hisi/coul/hisi_coul_drv.h>
#ifndef STATIC
#define STATIC extern
#endif
/*acr mul and resistivity table for acr calculate, Magnified 100,000 times times*/
static long M_R_TABLE[ACR_MUL_MAX][ACR_DATA_FIFO_DEPTH] =
{
    {-9194, -22196, -22196, -9194, 9194, 22196, 22196, 9194},
    {-4597, -11098, -11098, -4597, 4597, 11098, 11098, 4597},
    {-2298, -5549,  -5549,  -2298, 2298, 5549,  5549,  2298},
    {-1149, -2774,  -2774,  -1149, 1149, 2774,  2774,  1149}
};
/*acr mul and range table
    ACR_MUL_35 -> [10moh,200moh]
	ACR_MUL_70 -> [10moh,100moh]
	ACR_MUL_140-> [10moh,50moh]
	ACR_MUL_280-> [10moh,25moh]
*/
static int M_RANGE_TABLE[ACR_MUL_MAX][2] =
{
   {10, 200},
   {10, 100},
   {10, 50},
   {10, 25}
};

static u32 acr_mul_sel = ACR_MUL_70;
static ATOMIC_NOTIFIER_HEAD(hisi_soh_drv_atomic_notifier_list);

/*******************************************************
  Function:        hisi_soh_scharger_drv_register_atomic_notifier
  Description:     soh drv register notifier for soh core
  Input:           notifier_block *nb
  Output:          NULL
  Return:          0:success,other:fail.
********************************************************/
int hisi_soh_drv_register_atomic_notifier(struct notifier_block *nb)
{
    return atomic_notifier_chain_register(&hisi_soh_drv_atomic_notifier_list, nb);
}

int hisi_soh_drv_unregister_atomic_notifier(struct notifier_block * nb)
{
    return atomic_notifier_chain_unregister(&hisi_soh_drv_atomic_notifier_list, nb);
}


/*******************************************************
  Function:      hisi_soh_scharger_enable_acr
  Description:   enable acr .
  Input:         0:disalbe ,other:enable
  Output:        NA
  Return:        NA
  Remart:        NA
********************************************************/
STATIC void hisi_soh_scharger_acr_enable(int enable)
{
    u8 reg= 0;
     /* set acr channel and model*/
    hi6526_set_adc_acr_mode(enable);
    hi6526_read(ACR_EN_ADDR,&reg);
    if (enable)
    reg |= ACR_EN;
    else
    reg&= ~ACR_EN;
    hi6526_write(ACR_EN_ADDR, reg);
}
/*******************************************************
  Function:      hisi_soh_scharger_acr_get_acr_flag
  Description:   get acr flag .
  Input:         NA
  Output:        NA
  Return:        1:acr flag Produce 0:no
  Remart:        NA
********************************************************/
STATIC u32 hisi_soh_scharger_acr_get_acr_flag(void)
{
    u8 reg=0;
    hi6526_read(ACR_FLAG_INT_ADDR,&reg);
    reg &= ACR_FLAG_INT_BIT;
    if (reg)
        return 1;
    else
        return 0;
}

/*******************************************************
  Function:      hisi_soh_scharger_acr_clear_acr_flag
  Description:   clear acr flag .
  Input:         NA
  Output:        NA
  Return:        NA
  Remart:        NA
********************************************************/
STATIC void hisi_soh_scharger_acr_clear_acr_flag(void)
{
       hi6526_write(ACR_FLAG_INT_ADDR, ACR_FLAG_INT_BIT);
}

/*******************************************************
  Function:      hisi_soh_scharger_acr_set_mul
  Description:   set acr cal Multiple .
  Input:         Multiple
  Output:        NA
  Return:        NA
  Remart:        NA
********************************************************/
STATIC void __hisi_soh_scharger_acr_set_mul(enum acr_mul mul)
{
    u8 reg=0;
    switch (mul) {
    case ACR_MUL_35:
        acr_mul_sel = ACR_MUL_35;
        break;
     case ACR_MUL_70:
        acr_mul_sel = ACR_MUL_70;
        break;
     case ACR_MUL_140:
        acr_mul_sel = ACR_MUL_140;
        break;
     case ACR_MUL_280:
        acr_mul_sel = ACR_MUL_280;
        break;
     default:
        acr_mul_sel = ACR_MUL_35;
        scharger_soh_err("[%s] mul is 0x%x,err!\n",__func__,mul);
        break;
        }
    hi6526_read(ACR_MUL_SEL_ADDR,&reg);
    hi6526_write(ACR_MUL_SEL_ADDR, ((reg & ACR_MUL_MASK) | (acr_mul_sel << ACR_MUL_SHIFT)));
    scharger_soh_inf("[%s]:mul = 0x%x\n",__func__,((reg & ACR_MUL_MASK) | (acr_mul_sel << ACR_MUL_SHIFT)));
}

/*******************************************************
  Function:      __hisi_soh_scharger_acr_get_fifo_data
  Description:   get acr fifo data .
  Input:         fifo_order :fifo Serial number
  Output:        NA
  Return:        NA
  Remart:        NA
********************************************************/
STATIC u16 __hisi_soh_scharger_acr_get_fifo_data(int fifo_order)
{
    u16 reg_val;
    int tmp_addr_l = 0;
    int tmp_addr_h = 0;
    u8 reg=0;

    if (fifo_order > ACR_DATA_FIFO_DEPTH) {
        scharger_soh_err("[%s]:fifo_order = %d,overflow!\n",__func__,fifo_order);
        return 0;
    }

    tmp_addr_l = (int)ACR_L_DATA_BASE + ACR_DATA_REG_NUM * fifo_order;
    tmp_addr_h = (int)ACR_H_DATA_BASE + ACR_DATA_REG_NUM * fifo_order;

    hi6526_read(tmp_addr_l,&reg);
    reg_val = (u16)reg;
    hi6526_read(tmp_addr_h,&reg);
    reg_val = (reg << ACR_DATA_H_SHIFT) | reg_val;
    return reg_val;
}

/*******************************************************
  Function:      hisi_soh_scharger_acr_calculate_acr
  Description:   cal acr value
  Input:         NA
  Output:        NA
  Return:        acr value (moh)
  Remart:        If the measurement exceeds the gear position, the lower one is lowered.
********************************************************/
STATIC int hisi_soh_scharger_acr_calculate_acr(void)
{
    u16 fifo[ACR_DATA_FIFO_DEPTH];
    int i;
    long acr_value = 0;

    for (i = 0; i < ACR_DATA_FIFO_DEPTH; i++) {
        /*acr adc vol [0.1v,1.7v], acr data is greater than zero */
        fifo[i] = __hisi_soh_scharger_acr_get_fifo_data(i);
        if (0 == fifo[i]) {
            scharger_soh_err("%s: fifo[%d] =0,err!\n",__func__,i);
            return -1;
        }

        acr_value += fifo[i] * M_R_TABLE[acr_mul_sel][i];
        scharger_soh_inf("[%s]fifo[%d] = %d,m_r_table[%d][%d] = %ld, acr_value = %ld \n",
                            __func__,i,fifo[i],acr_mul_sel,i,M_R_TABLE[acr_mul_sel][i],acr_value);
    }
    /*if exceeding the gear position, the gear position is lowered and acr is recalculated*/
    if (acr_value/ACR_CAL_MAGNIFICATION > M_RANGE_TABLE[acr_mul_sel][1]) {
        scharger_soh_inf("[%s]:acr_value = %ld,change [%d] range !\n",__func__,acr_value, acr_mul_sel);
        if (acr_mul_sel >= 1) {
            __hisi_soh_scharger_acr_set_mul(acr_mul_sel -1);
            return -1;
        }
    }
    return (int)(acr_value*ACR_MOHM_TO_UOHM/ACR_CAL_MAGNIFICATION);
}

/*******************************************************
  Function:      hisi_soh_scharger_acr_get_chip_temp
  Description:   get acr chip temperature
  Input:         NA
  Output:        NA
  Return:        temerature (бу)
  Remart:      NA
********************************************************/
STATIC int hisi_soh_scharger_acr_get_chip_temp(void)
{
    int t_acr = 0;
    hi6526_get_chip_temp(&t_acr);
    return t_acr;
}

/*******************************************************
  Function:      hisi_soh_scharger_acr_get_fault_status
  Description:   get acr fault status .
  Input:         NA
  Output:        NA
  Return:        1:acr fault 0: normal
  Remart:        Piling
********************************************************/
STATIC u32 hisi_soh_scharger_acr_get_fault_status(void)
{
    return 0;
}
/*******************************************************
  Function:      hisi_soh_scharger_acr_clear_fault
  Description: clear acr fault status .
  Return:        NA
  Remart:        NA
********************************************************/
STATIC void hisi_soh_scharger_acr_clear_fault(void)
{
    return;
}

/*******************************************************
  Function:      hisi_soh_scharger_acr_gpio_en
  Description: io ctrl acr enable.
  Input:         0:disalbe ,other:enable
  Output:        NA
  Return:        NA
  Remart:        Piling
********************************************************/
STATIC void hisi_soh_scharger_acr_gpio_en(int enable)
{
    return;
}
/*******************************************************
  Function:      hisi_soh_scharger_acr_get_acr_ocp
  Description:   schargerv600 has no ocp.
  Input:         NA
  Output:        NA
  Return:        NA
  Remart:        Piling
********************************************************/
STATIC u32 hisi_soh_scharger_acr_get_acr_ocp(void)
{
    return 0;
}
/*******************************************************
  Function:      hisi_soh_scharger_acr_clear_acr_ocp
  Description:   schargerv600 has no ocp.
  Input:         NA
  Output:        NA
  Return:        NA
  Remart:        Piling
********************************************************/
STATIC void hisi_soh_scharger_acr_clear_acr_ocp(void)
{
    return;
}
/*******************************************************
  Function:      parse_scharger_acr_dts
  Description:   parse dts.
  Input:         NA
  Output:        NA
  Return:        0:suc;other: fail.
  Remart:        NA
********************************************************/
static void parse_scharger_acr_dts(struct soh_scharger_device *di)
{
    int ret = 0;
    struct device_node* np;

    if (!di)
        return;

    np = di->dev->of_node;
    ret = of_property_read_u32(np, "scharger_acr_support", (u32 *)&di->scharger_acr_support);
    if (ret) {
      scharger_soh_err("[%s]:acr support dts not exist!\n",__func__);
      di->scharger_acr_support = 0;
    }
    scharger_soh_inf("[%s]:support =%d\n",__func__,di->scharger_acr_support);
}

static struct soh_acr_device_ops hisi_scharger_acr_ops =
{
    .enable_acr             = hisi_soh_scharger_acr_enable,
    .get_acr_flag           = hisi_soh_scharger_acr_get_acr_flag,
    .clear_acr_flag         = hisi_soh_scharger_acr_clear_acr_flag,
    .calculate_acr          = hisi_soh_scharger_acr_calculate_acr,
    .get_acr_chip_temp      = hisi_soh_scharger_acr_get_chip_temp,
    .get_acr_fault_status   = hisi_soh_scharger_acr_get_fault_status,
    .clear_acr_fault_status = hisi_soh_scharger_acr_clear_fault,
    .get_acr_ocp            = hisi_soh_scharger_acr_get_acr_ocp,
    .clear_acr_ocp          = hisi_soh_scharger_acr_clear_acr_ocp,
    .io_ctrl_acr_chip_en    = hisi_soh_scharger_acr_gpio_en,
};

/*******************************************************
  Function:      hisi_soh_scharger_acr_init
  Description:   acr init.
  Input:         soh_pmic_device *di :soh device struct.
  Output:        NA
  Return:        0:success; other:fail.
  Remart:        NA
********************************************************/
STATIC int hisi_soh_scharger_acr_init(struct soh_scharger_device *di)
{
    int ret;

    if (!di)
        return -1;

    /*get dts config*/
    parse_scharger_acr_dts(di);

    /*acr starts according dts config*/
    if (!di->scharger_acr_support)
        return 0;

    /*mask acr flag interrupt*/
   hi6526_write(ACR_FLAG_INT_MASK_REG, ACR_FLAG_INT_MASK_BIT);

    /*choose acr mul*/
    __hisi_soh_scharger_acr_set_mul(ACR_MUL_70);
    /*register acr ops to soh core*/
    ret = soh_core_drv_ops_register(&hisi_scharger_acr_ops, ACR_DRV_OPS);
    if (ret)
        goto acr_init_err;

    scharger_soh_inf("[%s] soh acr init success!\n", __func__);
    return 0;

acr_init_err:
    scharger_soh_err("[%s]fail!\n", __func__);
    return  ret;
}
/*******************************************************
  Function:        hisi_soh_scharger_probe
  Description:     hisi soh scharger probe function
  Input:           struct platform_device
  Output:          NULL
  Return:          NULL
********************************************************/
static int  hisi_soh_scharger_probe(struct platform_device *pdev)
{
    struct soh_scharger_device *di = NULL;
    int ret;

    di = (struct soh_scharger_device *)devm_kzalloc(&pdev->dev, sizeof(*di), GFP_KERNEL);
    if (!di) {
        scharger_soh_err("%s failed to alloc di struct\n",__func__);
        return -1;
    }
    di->dev =&pdev->dev;
    platform_set_drvdata(pdev, di);
    ret = hisi_soh_scharger_acr_init(di);
    if (ret)
        goto soh_prob_err;
    scharger_soh_inf("[%s]suc!\n", __func__);
    return 0;
soh_prob_err:
    platform_set_drvdata(pdev, NULL);
    soh_core_drv_ops_register(NULL,ACR_DRV_OPS);
    scharger_soh_err("[%s]fail!\n", __func__);
    return ret;
}

/*******************************************************
  Function:        hisi_soh_scharger_remove
  Description:     remove function
  Input:           platform_device
  Output:          NULL
  Return:          NULL
********************************************************/
static int  hisi_soh_scharger_remove(struct platform_device *pdev)
{
    struct soh_scharger_device *di =  platform_get_drvdata(pdev);
    if(!di)
    {
        scharger_soh_err("[%s]di is null\n",__func__);
        return -1;
    }
    platform_set_drvdata(pdev, NULL);
    soh_core_drv_ops_register(NULL,ACR_DRV_OPS);
    return 0;
}


static struct of_device_id hisi_soh_scharger_match_table[] =
{
    {
          .compatible = "hisi_soh_charger",
          },
    {},
};


static struct platform_driver hisi_scharger_soh_driver = {
    .probe         = hisi_soh_scharger_probe,
    .remove        = hisi_soh_scharger_remove,
    .driver        = {
       .name           = "hisi_soh_charger",
       .owner          = THIS_MODULE,
       .of_match_table = hisi_soh_scharger_match_table,
    },
};


int __init hisi_soh_scharger_init(void)
{
    scharger_soh_inf("[%s]suc!\n", __func__);
    return platform_driver_register(&hisi_scharger_soh_driver);
}

void __exit hisi_soh_scharger_exit(void)
{
    scharger_soh_inf("[%s]suc!\n", __func__);
    platform_driver_unregister(&hisi_scharger_soh_driver);
}

module_init(hisi_soh_scharger_init);
module_exit(hisi_soh_scharger_exit);


MODULE_AUTHOR("HISILICON");
MODULE_DESCRIPTION("hisi scharger soh driver");
MODULE_LICENSE("GPL");
