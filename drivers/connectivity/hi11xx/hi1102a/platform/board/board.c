

/*****************************************************************************
  1 Header File Including
*****************************************************************************/
#ifdef _PRE_CONFIG_USE_DTS
#include <linux/of.h>
#include <linux/of_gpio.h>
#endif
/*lint -e322*//*lint -e7*/
#include <linux/clk.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/timer.h>
#include <linux/delay.h>
#ifdef CONFIG_PINCTRL
#include <linux/pinctrl/consumer.h>
#endif
#include <linux/fs.h>

/*lint +e322*//*lint +e7*/

#include "board.h"
#include "plat_debug.h"
#include "oal_ext_if.h"
#include "board-hi1102.h"
#include "board-hi1103.h"
#include "board_hi1102a.h"
#include "oal_sdio_host_if.h"
#include "plat_firmware.h"
#include "oal_hcc_bus.h"
#include "plat_pm.h"
#include "oam_ext_if.h"

/*****************************************************************************
  2 Global Variable Definition
*****************************************************************************/
BOARD_INFO g_board_info = {.ssi_gpio_clk = 0, .ssi_gpio_data = 0};
EXPORT_SYMBOL(g_board_info);

OAL_STATIC int32 board_probe_ret = 0;
OAL_STATIC  struct completion  board_driver_complete;

int hi11xx_kernel_crash = 0;/*linux kernel crash*/
EXPORT_SYMBOL_GPL(hi11xx_kernel_crash);

#define DSM_CPU_INFO_SIZE   (256)

char str_gpio_ssi_dump_path[100] = HISI_TOP_LOG_DIR"/wifi/memdump";
int  ssi_is_logfile = 0;
int  ssi_is_pilot = -1;
int ssi_dfr_bypass = 0;
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
module_param_string(gpio_ssi_dump_path, str_gpio_ssi_dump_path,
        sizeof(str_gpio_ssi_dump_path), S_IRUGO|S_IWUSR);
MODULE_PARM_DESC(gpio_ssi_dump_path, "gpio_ssi dump path");
module_param(ssi_is_logfile, int, S_IRUGO | S_IWUSR);
module_param(ssi_is_pilot, int, S_IRUGO | S_IWUSR);
module_param(ssi_dfr_bypass, int, S_IRUGO | S_IWUSR);
#endif

OAL_DEFINE_SPINLOCK(g_ssi_lock);
oal_uint32 g_ssi_lock_state = 0x0;

DEVICE_BOARD_VERSION device_board_version_list[BOARD_VERSION_BOTT] = {
    {.index = BOARD_VERSION_HI1102, .name = BOARD_VERSION_NAME_HI1102},
    {.index = BOARD_VERSION_HI1102A, .name = BOARD_VERSION_NAME_HI1102A},
    {.index = BOARD_VERSION_HI1103, .name = BOARD_VERSION_NAME_HI1103},
};

DOWNLOAD_MODE device_download_mode_list[MODE_DOWNLOAD_BUTT] = {
    {.index = MODE_SDIO, .name = DOWNlOAD_MODE_SDIO},
    {.index = MODE_PCIE, .name = DOWNlOAD_MODE_PCIE},
    {.index = MODE_UART, .name = DOWNlOAD_MODE_UART},
};

#ifdef _PRE_CONFIG_GPIO_TO_SSI_DEBUG
#if 0
/*full version*/
ssi_file_st g_aSsiFile[2] =
{
    {"/system/vendor/firmware/CPU_RAM_SCHED.bin", 0x00010000},
    {"/system/vendor/firmware/BOOT_CALLBACK.bin", 0x0001d200},
};
#else
/*gnss_only uart_cfg*/
ssi_file_st g_aSsiFile[] =
{
#ifdef BFGX_UART_DOWNLOAD_SUPPORT
    /*gnss only*/
    {"/system/vendor/firmware/RAM_VECTOR.bin",    0x80100800},
    {"/system/vendor/firmware/CPU_RAM_SCHED.bin", 0x80004000},
#else
    /*mpw2*/
    {"/system/vendor/firmware/BCPU_ROM.bin",           0x80000000},
    {"/system/vendor/firmware/VECTORS.bin",            0x80010000},
    {"/system/vendor/firmware/RAM_VECTOR.bin",         0x80105c00},
    {"/system/vendor/firmware/WCPU_ROM.bin",           0x4000},
    {"/system/vendor/firmware/WL_ITCM.bin",            0x10000},
    {"/system/vendor/firmware/PLAT_RAM_EXCEPTION.bin", 0x20002800},
#endif
};
#endif
#endif
/*
*
***************************************************************************
  3
 Function Definition
***
**************************************************************************/
extern irqreturn_t bfg_wake_host_isr(int irq, void *dev_id);
int ssi_check_wcpu_is_working(void);
int ssi_check_bcpu_is_working(void);
inline BOARD_INFO * get_hi110x_board_info(void)
{
    return &g_board_info;
}

int isAsic(void)
{
    if (VERSION_ASIC == g_board_info.is_asic)
    {
        return VERSION_ASIC;
    }
    else
    {
        return VERSION_FPGA;
    }
}
EXPORT_SYMBOL_GPL(isAsic);

int32 get_hi110x_subchip_type(void)
{
    BOARD_INFO * bd_info = NULL;

    bd_info = get_hi110x_board_info();
    if (unlikely(NULL == bd_info))
    {
        PS_PRINT_ERR("board info is err\n");
        return -EFAIL;
    }

    return bd_info->chip_nr;
}

#ifdef _PRE_CONFIG_USE_DTS
int32 get_board_dts_node(struct device_node ** np, const char * node_prop)
{
    if (NULL ==np || NULL == node_prop)
    {
        PS_PRINT_ERR("func has NULL input param!!!, np=%p, node_prop=%p\n", np, node_prop);
        return BOARD_FAIL;
    }

    *np = of_find_compatible_node(NULL, NULL, node_prop);
    if (NULL == *np)
    {
        PS_PRINT_ERR("HISI IPC:No compatible node found.\n");
        return BOARD_FAIL;
    }

    return BOARD_SUCC;
}

int32 get_board_dts_prop(struct device_node *np, const char * dts_prop, const char ** prop_val)
{
    int32 ret = BOARD_FAIL;

    if (NULL == np || NULL == dts_prop || NULL == prop_val)
    {
        PS_PRINT_ERR("func has NULL input param!!!, np=%p, dts_prop=%p, prop_val=%p\n", np, dts_prop, prop_val);
        return BOARD_FAIL;
    }

    ret = of_property_read_string(np, dts_prop, prop_val);
    if (ret)
    {
        PS_PRINT_ERR("can't get dts_prop value: dts_prop=%s\n", dts_prop);
        return ret;
    }

    PS_PRINT_SUC("have get dts_prop and prop_val: %s=%s\n", dts_prop, *prop_val);

    return BOARD_SUCC;
}

int32 get_board_dts_gpio_prop(struct device_node *np, const char * dts_prop, int32 * prop_val)
{
    int32 ret = BOARD_FAIL;

    if (NULL == np || NULL == dts_prop || NULL == prop_val)
    {
        PS_PRINT_ERR("func has NULL input param!!!, np=%p, dts_prop=%p, prop_val=%p\n", np, dts_prop, prop_val);
        return BOARD_FAIL;
    }

    ret = of_get_named_gpio(np, dts_prop, 0);
    if (ret < 0)
    {
        PS_PRINT_ERR("can't get dts_prop value: dts_prop=%s, ret=%d\n", dts_prop, ret);
        return ret;
    }

    *prop_val = ret;
    PS_PRINT_SUC("have get dts_prop and prop_val: %s=%d\n", dts_prop, *prop_val);

    return BOARD_SUCC;
}

#endif

int32 get_board_gpio(const char * gpio_node, const char * gpio_prop, int32 *physical_gpio)
{
#ifdef _PRE_CONFIG_USE_DTS
    int32 ret= BOARD_FAIL;
    struct device_node * np = NULL;

    ret = get_board_dts_node(&np, gpio_node);
    if(BOARD_SUCC != ret)
    {
        return BOARD_FAIL;
    }

    ret = get_board_dts_gpio_prop(np, gpio_prop, physical_gpio);
    if(BOARD_SUCC != ret)
    {
        return BOARD_FAIL;
    }

    return BOARD_SUCC;
#else
    return BOARD_SUCC;
#endif
}

int32 get_board_custmize(const char * cust_node, const char * cust_prop, const char **cust_prop_val)
{
#ifdef _PRE_CONFIG_USE_DTS
    int32 ret= BOARD_FAIL;
    struct device_node * np = NULL;

    if (NULL == cust_node || NULL == cust_prop || NULL == cust_prop_val)
    {
        PS_PRINT_ERR("func has NULL input param!!!\n");
        return BOARD_FAIL;
    }

    ret = get_board_dts_node(&np, cust_node);
    if(BOARD_SUCC != ret)
    {
        return BOARD_FAIL;
    }

    ret = get_board_dts_prop(np, cust_prop, cust_prop_val);
    if(BOARD_SUCC != ret)
    {
        return BOARD_FAIL;
    }

    PS_PRINT_INFO("get board customize info %s=%s\n", cust_prop, *cust_prop_val);

    return BOARD_SUCC;
#else
    return BOARD_FAIL;
#endif
}

int32 get_board_pmu_clk32k(void)
{
    return g_board_info.bd_ops.get_board_pmu_clk32k();
}

int32 set_board_pmu_clk32k(struct platform_device *pdev)
{
#ifdef _PRE_CONFIG_USE_DTS
    int32 ret= BOARD_FAIL;
    const char * clk_name = NULL;
    struct clk* clk = NULL;
    struct device *dev = NULL;

    dev = &pdev->dev;
    clk_name = g_board_info.clk_32k_name;
    if (BOARD_VERSION_HI1102 == get_hi110x_subchip_type())
    {
        clk = devm_clk_get(dev, "clk_pmu32kb");
    }
    else
    {
        clk = devm_clk_get(dev, clk_name);
    }

    if (NULL == clk)
    {
        PS_PRINT_ERR("Get 32k clk %s failed!!!\n", clk_name);
        CHR_EXCEPTION_REPORT(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_WIFI, CHR_LAYER_DEV, CHR_WIFI_DEV_EVENT_CHIP, CHR_WIFI_DEV_ERROR_32K_CLK);
        return BOARD_FAIL;
    }
    g_board_info.clk_32k = clk;

    ret = clk_prepare_enable(clk);
    if (unlikely(ret < 0))
    {
        devm_clk_put(dev, clk);
        PS_PRINT_ERR("enable 32K clk %s failed!!!", clk_name);
        CHR_EXCEPTION_REPORT(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_WIFI, CHR_LAYER_DEV, CHR_WIFI_DEV_EVENT_CHIP, CHR_WIFI_DEV_ERROR_32K_CLK);
        return BOARD_FAIL;
    }
#endif
    return BOARD_SUCC;
}

int32 get_board_uart_port(void)
{
#ifdef _PRE_CONFIG_USE_DTS
    return g_board_info.bd_ops.get_board_uart_port();
#else
    return BOARD_SUCC;
#endif
}

int32 check_evb_or_fpga(void)
{
#ifdef _PRE_CONFIG_USE_DTS
    return g_board_info.bd_ops.check_evb_or_fpga();
#else
    return BOARD_SUCC;
#endif
}

int32 board_get_power_pinctrl(struct platform_device *pdev)
{
#ifdef _PRE_CONFIG_USE_DTS
    return g_board_info.bd_ops.board_get_power_pinctrl(pdev);
#else
    return BOARD_SUCC;
#endif
}

int32 board_power_gpio_init(void)
{
    return g_board_info.bd_ops.get_board_power_gpio();
}
void free_board_power_gpio(void)
{
    g_board_info.bd_ops.free_board_power_gpio();
}
#ifdef HAVE_HISI_IR
void free_board_ir_gpio(void)
{
    if (g_board_info.bfgx_ir_ctrl_gpio > -1)
    {
        gpio_free(g_board_info.bfgx_ir_ctrl_gpio);
    }
}
#endif
void free_board_wakeup_gpio(void)
{
    g_board_info.bd_ops.free_board_wakeup_gpio();
}

int32 board_wakeup_gpio_init(void)
{
    return g_board_info.bd_ops.board_wakeup_gpio_init();
}

void free_board_flowctrl_gpio(void)
{
    g_board_info.bd_ops.free_board_flowctrl_gpio();
}

int32 board_flowctrl_gpio_init(void)
{
    return g_board_info.bd_ops.board_flowctrl_gpio_init();
}

void board_flowctrl_irq_init(void)
{
    g_board_info.flowctrl_irq = gpio_to_irq(g_board_info.flowctrl_gpio);
}

#ifdef HAVE_HISI_IR
int32 board_ir_ctrl_init(struct platform_device *pdev)
{
    return g_board_info.bd_ops.board_ir_ctrl_init(pdev);
}
#endif

int32 board_gpio_init(struct platform_device *pdev)
{
    int32 ret= BOARD_FAIL;

    /*power on gpio request*/
    ret = board_power_gpio_init();
    if(BOARD_SUCC != ret)
    {
        PS_PRINT_ERR("get power_on dts prop failed\n");
        goto err_get_power_on_gpio;
    }

    ret = board_wakeup_gpio_init();
    if(BOARD_SUCC != ret)
    {
        PS_PRINT_ERR("get wakeup prop failed\n");
        goto oal_board_wakup_gpio_fail;
    }

#ifdef HAVE_HISI_IR
    ret = board_ir_ctrl_init(pdev);
    if(BOARD_SUCC != ret)
    {
        PS_PRINT_ERR("get ir dts prop failed\n");
        goto err_get_ir_ctrl_gpio;
    }
#endif

    return BOARD_SUCC;

#ifdef HAVE_HISI_IR
err_get_ir_ctrl_gpio:
    free_board_wakeup_gpio();
#endif
oal_board_wakup_gpio_fail:
    free_board_power_gpio();
err_get_power_on_gpio:

    CHR_EXCEPTION_REPORT(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_WIFI, CHR_LAYER_DEV, CHR_WIFI_DEV_EVENT_CHIP, CHR_WIFI_DEV_ERROR_GPIO);

    return BOARD_FAIL;
}

int board_get_bwkup_gpio_val(void)
{
    return gpio_get_value(g_board_info.bfgn_wakeup_host);
}

int board_get_wlan_wkup_gpio_val(void)
{
    return gpio_get_value(g_board_info.wlan_wakeup_host);
}

int32 board_irq_init(void)
{
    uint32 irq = 0;
    int32 gpio = 0;

#ifndef BFGX_UART_DOWNLOAD_SUPPORT
    gpio = g_board_info.wlan_wakeup_host;
    irq = gpio_to_irq(gpio);
    g_board_info.wlan_irq = irq;

    PS_PRINT_INFO("wlan_irq is %d\n", g_board_info.wlan_irq);
#endif

    gpio = g_board_info.bfgn_wakeup_host;
    irq = gpio_to_irq(gpio);
    g_board_info.bfgx_irq = irq;

    PS_PRINT_INFO("bfgx_irq is %d\n", g_board_info.bfgx_irq);

    return BOARD_SUCC;
}

int32 board_clk_init(struct platform_device *pdev)
{
    int32 ret= BOARD_FAIL;

    if (NULL == pdev)
    {
        PS_PRINT_ERR("func has NULL input param!!!\n");
        return BOARD_FAIL;
    }

    ret = g_board_info.bd_ops.get_board_pmu_clk32k();
    if(BOARD_SUCC != ret)
    {
        return BOARD_FAIL;
    }

    ret = set_board_pmu_clk32k(pdev);
    if(BOARD_SUCC != ret)
    {
        return BOARD_FAIL;
    }

    return BOARD_SUCC;
}

void power_state_change(int32 gpio, int32 flag)
{
    if(unlikely(0 == gpio))
    {
        PS_PRINT_WARNING("gpio is 0, flag=%d\n", flag);
        return;
    }

    PS_PRINT_INFO("power_state_change gpio %d to %s\n", gpio, (BOARD_POWER_ON == flag) ? "low2high":"low");

    if (BOARD_POWER_ON == flag)
    {
        gpio_direction_output(gpio, GPIO_LOWLEVEL);
        mdelay(10);
        gpio_direction_output(gpio, GPIO_HIGHLEVEL);
        mdelay(20);
    }
    else if (BOARD_POWER_OFF == flag)
    {
        gpio_direction_output(gpio, GPIO_LOWLEVEL);
    }
}

int32 board_wlan_gpio_power_on(void* data)
{
    int32 gpio = (int32)(long)(data);
    if(g_board_info.host_wakeup_wlan)
    {
        /*host wakeup dev gpio pinmux to jtag when w boot,
          must gpio low when bootup*/
        board_host_wakeup_dev_set(0);
    }
    power_state_change(gpio, BOARD_POWER_ON);
    board_host_wakeup_dev_set(1);
    return 0;
}

int32 board_wlan_gpio_power_off(void* data)
{
    int32 gpio = (int32)(long)(data);
    power_state_change(gpio, BOARD_POWER_OFF);
    return 0;
}

int32 board_host_wakeup_dev_set(int value)
{
    if(0 == g_board_info.host_wakeup_wlan)
    {
        PS_PRINT_INFO("host_wakeup_wlan gpio is 0\n");
        return 0;
    }
    PS_PRINT_DBG("host_wakeup_wlan set %s %pF\n", value ? "high":"low", (void*)_RET_IP_);
    if(value)
    {
        return gpio_direction_output(g_board_info.host_wakeup_wlan, GPIO_HIGHLEVEL);
    }
    else
    {
        return gpio_direction_output(g_board_info.host_wakeup_wlan, GPIO_LOWLEVEL);
    }
}

int32 board_get_host_wakeup_dev_stat(void)
{
    return gpio_get_value(g_board_info.host_wakeup_wlan);
}

int32 board_power_on(uint32 ul_subsystem)
{
    return g_board_info.bd_ops.board_power_on(ul_subsystem);
}
int32 board_power_off(uint32 ul_subsystem)
{
    return g_board_info.bd_ops.board_power_off(ul_subsystem);
}

int32 board_power_reset(uint32 ul_subsystem)
{
    return g_board_info.bd_ops.board_power_reset(ul_subsystem);
}
EXPORT_SYMBOL(board_wlan_gpio_power_off);
EXPORT_SYMBOL(board_wlan_gpio_power_on);

int32 find_device_board_version(void)
{
    int32 ret= BOARD_FAIL;
    const char *device_version = NULL;

    ret = get_board_custmize(DTS_NODE_HISI_HI110X, DTS_PROP_SUBCHIP_TYPE_VERSION, &device_version);
    if(BOARD_SUCC != ret)
    {
        return BOARD_FAIL;
    }

    g_board_info.chip_type = device_version;
    return BOARD_SUCC;
}

int32 board_chiptype_init(void)
{
    int32 ret= BOARD_FAIL;

    ret = find_device_board_version();
    if(BOARD_SUCC != ret)
    {
        PS_PRINT_ERR("can not find device_board_version\n");
        return BOARD_FAIL;
    }

    ret = check_device_board_name();
    if (BOARD_SUCC != ret)
    {
        PS_PRINT_ERR("check device name fail\n");
        return BOARD_FAIL;
    }

    return BOARD_SUCC;
}


int32 board_func_init(void)
{
    int32 ret= BOARD_FAIL;
    //board init
    memset(&g_board_info, 0 ,sizeof(BOARD_INFO));

    ret = find_device_board_version();
    if(BOARD_SUCC != ret)
    {
        /*兼容1102*/
        g_board_info.chip_nr = BOARD_VERSION_HI1102;
        g_board_info.chip_type = device_board_version_list[g_board_info.chip_nr].name;
        PS_PRINT_WARNING("can not find device_board_version ,choose default:%s\n", g_board_info.chip_type);
    }
    else
    {
        ret = check_device_board_name();
        if (BOARD_SUCC != ret)
        {
            return BOARD_FAIL;
        }
    }

    switch (g_board_info.chip_nr)
    {
        case  BOARD_VERSION_HI1102:
            g_board_info.bd_ops.get_board_power_gpio       =hi1102_get_board_power_gpio;
            g_board_info.bd_ops.free_board_power_gpio      =hi1102_free_board_power_gpio;
            g_board_info.bd_ops.board_wakeup_gpio_init     =hi1102_board_wakeup_gpio_init;
            g_board_info.bd_ops.free_board_wakeup_gpio     =hi1102_free_board_wakeup_gpio;
            g_board_info.bd_ops.bfgx_dev_power_on          =hi1102_bfgx_dev_power_on;
            g_board_info.bd_ops.bfgx_dev_power_off         =hi1102_bfgx_dev_power_off;
            g_board_info.bd_ops.wlan_power_off             =hi1102_wlan_power_off;
            g_board_info.bd_ops.wlan_power_on              =hi1102_wlan_power_on;
            g_board_info.bd_ops.board_power_on             =hi1102_board_power_on;
            g_board_info.bd_ops.board_power_off            =hi1102_board_power_off;
            g_board_info.bd_ops.board_power_reset          =hi1102_board_power_reset;
            g_board_info.bd_ops.get_board_pmu_clk32k       =hi1102_get_board_pmu_clk32k;
            g_board_info.bd_ops.get_board_uart_port        =hi1102_get_board_uart_port;
            g_board_info.bd_ops.board_ir_ctrl_init         =hi1102_board_ir_ctrl_init;
            g_board_info.bd_ops.check_evb_or_fpga          =hi1102_check_evb_or_fpga;
            g_board_info.bd_ops.board_get_power_pinctrl    =hi1102_board_get_power_pinctrl;
            g_board_info.bd_ops.get_ini_file_name_from_dts =hi1102_get_ini_file_name_from_dts;
            break;
        case BOARD_VERSION_HI1102A:
            g_board_info.bd_ops.get_board_power_gpio       =hi1102a_get_board_power_gpio;
            g_board_info.bd_ops.free_board_power_gpio      =hi1102a_free_board_power_gpio;
            g_board_info.bd_ops.free_board_flowctrl_gpio   =hi1102a_free_board_flowctrl_gpio;
            g_board_info.bd_ops.board_wakeup_gpio_init     =hi1102a_board_wakeup_gpio_init;
            g_board_info.bd_ops.board_flowctrl_gpio_init   =hi1102a_board_flowctrl_gpio_init;
            g_board_info.bd_ops.free_board_wakeup_gpio     =hi1102a_free_board_wakeup_gpio;
            g_board_info.bd_ops.bfgx_dev_power_on          =hi1102a_bfgx_dev_power_on;
            g_board_info.bd_ops.bfgx_dev_power_off         =hi1102a_bfgx_dev_power_off;
            g_board_info.bd_ops.hitalk_power_off           =hi1102a_hitalk_power_off;
            g_board_info.bd_ops.hitalk_power_on            =hi1102a_hitalk_power_on;
            g_board_info.bd_ops.wlan_power_off             =hi1102a_wlan_power_off;
            g_board_info.bd_ops.wlan_power_on              =hi1102a_wlan_power_on;
            g_board_info.bd_ops.board_power_on             =hi1102a_board_power_on;
            g_board_info.bd_ops.board_power_off            =hi1102a_board_power_off;
            g_board_info.bd_ops.board_power_reset          =hi1102a_board_power_reset;
            g_board_info.bd_ops.get_board_pmu_clk32k       =hi1102a_get_board_pmu_clk32k;
            g_board_info.bd_ops.get_board_uart_port        =hi1102a_get_board_uart_port;
            g_board_info.bd_ops.board_ir_ctrl_init         =hi1102a_board_ir_ctrl_init;
            g_board_info.bd_ops.check_evb_or_fpga          =hi1102a_check_evb_or_fpga;
            g_board_info.bd_ops.board_get_power_pinctrl    =hi1102a_board_get_power_pinctrl;
            g_board_info.bd_ops.get_ini_file_name_from_dts =hi1102a_get_ini_file_name_from_dts;
            break;
        case BOARD_VERSION_HI1103:
            g_board_info.bd_ops.get_board_power_gpio       =hi1103_get_board_power_gpio;
            g_board_info.bd_ops.free_board_power_gpio      =hi1103_free_board_power_gpio;
            g_board_info.bd_ops.free_board_flowctrl_gpio   =hi1103_free_board_flowctrl_gpio;
            g_board_info.bd_ops.board_wakeup_gpio_init     =hi1103_board_wakeup_gpio_init;
            g_board_info.bd_ops.board_flowctrl_gpio_init   =hi1103_board_flowctrl_gpio_init;
            g_board_info.bd_ops.free_board_wakeup_gpio     =hi1103_free_board_wakeup_gpio;
            g_board_info.bd_ops.bfgx_dev_power_on          =hi1103_bfgx_dev_power_on;
            g_board_info.bd_ops.bfgx_dev_power_off         =hi1103_bfgx_dev_power_off;
            g_board_info.bd_ops.wlan_power_off             =hi1103_wlan_power_off;
            g_board_info.bd_ops.wlan_power_on              =hi1103_wlan_power_on;
            g_board_info.bd_ops.board_power_on             =hi1103_board_power_on;
            g_board_info.bd_ops.board_power_off            =hi1103_board_power_off;
            g_board_info.bd_ops.board_power_reset          =hi1103_board_power_reset;
            g_board_info.bd_ops.get_board_pmu_clk32k       =hi1103_get_board_pmu_clk32k;
            g_board_info.bd_ops.get_board_uart_port        =hi1103_get_board_uart_port;
            g_board_info.bd_ops.board_ir_ctrl_init         =hi1103_board_ir_ctrl_init;
            g_board_info.bd_ops.check_evb_or_fpga          =hi1103_check_evb_or_fpga;
            g_board_info.bd_ops.board_get_power_pinctrl    =hi1103_board_get_power_pinctrl;
            g_board_info.bd_ops.get_ini_file_name_from_dts =hi1103_get_ini_file_name_from_dts;
            break;
        default:
            PS_PRINT_ERR("g_board_info.chip_nr=%d is illegal\n", g_board_info.chip_nr);
            return BOARD_FAIL;
    }


    PS_PRINT_INFO("g_board_info.chip_nr=%d, device_board_version is %s\n", g_board_info.chip_nr, g_board_info.chip_type);
    return BOARD_SUCC;
}

int32 check_download_channel_name(uint8* wlan_buff, int32* index)
{
    int32 i = 0;
    for (i = 0; i < MODE_DOWNLOAD_BUTT; i++)
    {
        if (0 == strncmp(device_download_mode_list[i].name, wlan_buff, strlen(device_download_mode_list[i].name)))
        {
            *index = i;
            return BOARD_SUCC;
        }
    }
    return BOARD_FAIL;
}

int32 get_download_channel(void)
{
    int32 ret= BOARD_FAIL;
    uint8 wlan_mode[DOWNLOAD_CHANNEL_LEN]={0};
    uint8 bfgn_mode[DOWNLOAD_CHANNEL_LEN]={0};

    /*wlan channel*/
    ret = find_download_channel(wlan_mode, INI_WLAN_DOWNLOAD_CHANNEL);
    if (BOARD_SUCC != ret)
    {
        /*兼容1102,1102无此配置项*/
        g_board_info.wlan_download_channel = MODE_SDIO;
        PS_PRINT_WARNING("can not find wlan_download_channel ,choose default:%s\n", device_download_mode_list[0].name);
        hcc_bus_cap_init(HCC_CHIP_110X_DEV, NULL);
    }
    else
    {
        if (BOARD_SUCC != check_download_channel_name(wlan_mode, &(g_board_info.wlan_download_channel)))
        {
            PS_PRINT_ERR("check wlan download channel:%s error\n", bfgn_mode);
            return BOARD_FAIL;
        }
        hcc_bus_cap_init(HCC_CHIP_110X_DEV, wlan_mode);
    }


    /*bfgn channel*/
    ret = find_download_channel(bfgn_mode, INI_BFGX_DOWNLOAD_CHANNEL);
    if (BOARD_SUCC != ret)
    {
        /*如果不存在该项，则默认保持和wlan一致*/
        g_board_info.bfgn_download_channel = g_board_info.wlan_download_channel;
        PS_PRINT_WARNING("can not find bfgn_download_channel ,choose default:%s\n", device_download_mode_list[0].name);
        return BOARD_SUCC;
    }

    if (BOARD_SUCC != check_download_channel_name(bfgn_mode, &(g_board_info.bfgn_download_channel)))
    {
        PS_PRINT_ERR("check bfgn download channel:%s error\n", bfgn_mode);
        return BOARD_FAIL;
    }

    PS_PRINT_INFO("wlan_download_channel index:%d, bfgn_download_channel index:%d\n",
                        g_board_info.wlan_download_channel, g_board_info.bfgn_download_channel);

    return BOARD_SUCC;
}

int32 check_device_board_name(void)
{
    int32 i = 0;
    for (i = 0; i < BOARD_VERSION_BOTT; i++)
    {
        if (strlen(g_board_info.chip_type) != strlen(device_board_version_list[i].name))
            continue;
        if (0 == strncmp(device_board_version_list[i].name, g_board_info.chip_type, strlen(device_board_version_list[i].name)))
        {
            g_board_info.chip_nr = device_board_version_list[i].index;
            return BOARD_SUCC;
        }
    }

    return BOARD_FAIL;
}

int32 get_uart_pclk_source(void)
{
    return g_board_info.uart_pclk;
}

#ifdef _PRE_PLAT_FEATURE_HI110X_PCIE
extern oal_int32 pcie_memcopy_type;
STATIC void board_cci_bypass_init(void)
{
#ifdef _PRE_CONFIG_USE_DTS
    int32 ret= BOARD_FAIL;
    struct device_node * np = NULL;

    if( -1 == pcie_memcopy_type)
    {
        PS_PRINT_INFO("skip pcie mem burst control\n");
        return;
    }

    ret = get_board_dts_node(&np, DTS_NODE_HISI_CCIBYPASS);
    if(BOARD_SUCC != ret)
    {
        /*cci enable*/
        pcie_memcopy_type = 0;
        PS_PRINT_INFO("cci enable, pcie use mem burst 8 bytes\n");
    }
    else
    {
        /*cci bypass*/
        pcie_memcopy_type = 1;
        PS_PRINT_INFO("cci bypass, pcie use mem burst 4 bytes\n");
    }
#endif
}
#endif

STATIC int32 hi110x_board_probe(struct platform_device *pdev)
{
    int ret = BOARD_FAIL;

    PS_PRINT_INFO("hi110x board init\n");
    ret = board_func_init();
    if (BOARD_SUCC != ret)
    {
        goto err_init;
    }

    ret = ini_cfg_init();
    if (BOARD_SUCC != ret)
    {
        goto err_init;
    }

    ret = check_evb_or_fpga();
    if (BOARD_SUCC != ret)
    {
        goto err_init;
    }

    ret = get_download_channel();
    if (BOARD_SUCC != ret)
    {
        goto err_init;
    }

    ret = board_clk_init(pdev);
    if (BOARD_SUCC != ret)
    {
        goto err_init;
    }
#ifdef _PRE_PLAT_FEATURE_HI110X_PCIE
    board_cci_bypass_init();
#endif

    ret = get_board_uart_port();
    if (BOARD_SUCC != ret)
    {
        goto err_init;
    }

    ret = board_gpio_init(pdev);
    if (BOARD_SUCC != ret)
    {
        goto err_init;
    }

    ret = board_irq_init();
    if (BOARD_SUCC != ret)
    {
        goto err_gpio_source;
    }

    ret = board_get_power_pinctrl(pdev);
    if (BOARD_SUCC != ret)
    {
        goto err_get_power_pinctrl;
    }

    PS_PRINT_INFO("board init ok\n");

    board_probe_ret = BOARD_SUCC;
    complete(&board_driver_complete);

    return BOARD_SUCC;

/* gpio free in hi110x_board_remove */
err_get_power_pinctrl:
    //free_irq(g_board_info.bfgx_irq, NULL);
err_gpio_source:
#ifdef HAVE_HISI_IR
    //free_board_ir_gpio();
#endif
    //free_board_wakeup_gpio();
    //free_board_power_gpio();
    //free_board_flowctrl_gpio();

err_init:
    board_probe_ret = BOARD_FAIL;
    complete(&board_driver_complete);
    CHR_EXCEPTION_REPORT(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_WIFI, CHR_LAYER_DEV, CHR_WIFI_DEV_EVENT_CHIP, CHR_WIFI_DEV_ERROR_POWER_GPIO);
    return BOARD_FAIL;
}

STATIC int32 hi110x_board_remove(struct platform_device *pdev)
{
    PS_PRINT_INFO("hi110x board exit\n");

#ifdef _PRE_CONFIG_USE_DTS
    if (NEED_POWER_PREPARE == g_board_info.need_power_prepare)
    {
        devm_pinctrl_put(g_board_info.pctrl);
    }
#endif

    //free_irq(g_board_info.bfgx_irq, NULL);

#ifdef HAVE_HISI_IR
    free_board_ir_gpio();
#endif

    free_board_wakeup_gpio();
    free_board_power_gpio();
    free_board_flowctrl_gpio();

    return BOARD_SUCC;
}

int32 hi110x_board_suspend(struct platform_device *pdev, pm_message_t state)
{
    return BOARD_SUCC;
}

int32 hi110x_board_resume(struct platform_device *pdev)
{
    return BOARD_SUCC;
}


/*********************************************************************/
/********************   SSI调试代码start   ***************************/
/*********************************************************************/
#ifdef _PRE_CONFIG_GPIO_TO_SSI_DEBUG
#define HI110X_SSI_CLK_GPIO_NAME  ("hi110x ssi clk")
#define HI110X_SSI_DATA_GPIO_NAME ("hi110x ssi data")
#define INTERVAL_TIME             (10)
#define SSI_DATA_LEN              (16)

#ifdef BFGX_UART_DOWNLOAD_SUPPORT
#define SSI_CLK_GPIO  89
#define SSI_DATA_GPIO 91
#else
#define SSI_CLK_GPIO  75
#define SSI_DATA_GPIO 77
#endif

char* g_ssi_hi1103_mpw2_cpu_st_str[] =
{
    "OFF",/*0x0*/
    "SLEEP",/*0x1*/
    "IDLE",/*0x2*/
    "WORK"/*0x3*/
};
char* g_ssi_hi1103_pilot_cpu_st_str[] =
{
    "OFF",/*0x0*/
    "BOOTING",/*0x1*/
    "SLEEPING",/*0x2*/
    "WORK",/*0x3*/
    "SAVING",/*0x4*/
    "PROTECT(ocp/scp)",/*0x5*/
    "SLEEP",/*0x6*/
    "PROTECTING"/*0x7*/
};

char* g_ssi_hi1102a_cpu_pwr_st_str[] =
{
    "OFF",/*0x0*/
    "OPEN",/*0x1*/
};

char* g_ssi_hi1102a_cpu_sys_st_str[] =
{
    "SLEEP",/*0x0*/
    "WORKING",/*0x1*/
};

/*过流过压状态*/
char* g_ssi_hi1102a_pmu_protect_st_str[] =
{
    "ocp_cldo1_grm_1stick",/*0x0*/
    "ocp_rfldo1_grm_1stick",/*0x1*/
    "ocp_rfldo2_grm_1stick",/*0x2*/
    "ocp_paldo_grm_1stick",/*0x3*/
    "buck_scp_grm_1stick",/*0x4*/
    "buck_ocp_grm_1stick",/*0x5*/
    "buck_ovp_grm_1stick",/*0x6*/
    "uvlo_grm_1stick",/*0x7*/
    "ovp_grm_1stick",/*0x8*/
    "ocp_ir_paldo_grm_1stick",/*0x9*/
    "ocp_cldo2_grm_1stick",/*0xa*/
    "paldo_scp_grm_1stick",/*0xb*/
    "paldo_ovp_grm_1stick",/*0xc*/
    "ir_paldo_scp_grm_1stick",/*0xd*/
    "ir_paldo_ovp_grm_1stick",/*0xe*/
    "reserved",/*0xf*/
};

/*下电状态*/
char* g_ssi_hi1102a_pmu_protect_rpt_str[] =
{
    "buck_scp_off",/*0x0*/
    "buck_scp_off_rpt",/*0x1*/
    "ovp_off",/*2*/
    "ovp_off_rpt",/*3*/
    "tsensor_overtemp_pd_off_sts",/*4*/
    "tsensor_overtemp_pd_off_sts_rpt",/*5*/
    "buck_ovp_off",/*6*/
    "buck_ovp_off_rpt",/*7*/
    "paldo_scp_off",/*8*/
    "paldo_scp_off_rpt",/*9*/
    "paldo_ovp_off",/*10*/
    "paldo_ovp_off_rpt",/*11*/
    "ir_paldo_scp_off",/*12*/
    "ir_paldo_scp_off_rpt",/*13*/
    "ir_paldo_ovp_off",/*14*/
    "ir_paldo_ovp_off_rpt",/*15*/
};

#define SSI_CPU_ARM_REG_DUMP_CNT    (2)
static uint32 g_halt_det_cnt = 0;/*检测soc异常次数*/
typedef struct _ssi_cpu_info_
{
    uint32 cpu_state;
    uint32 pc[SSI_CPU_ARM_REG_DUMP_CNT];
    uint32 lr[SSI_CPU_ARM_REG_DUMP_CNT];
    uint32 sp[SSI_CPU_ARM_REG_DUMP_CNT];
    uint32 reg_flag[SSI_CPU_ARM_REG_DUMP_CNT];
}ssi_cpu_info;

typedef struct _ssi_cpu_infos_
{
    ssi_cpu_info wcpu_info;
    ssi_cpu_info bcpu_info;
}ssi_cpu_infos;

static ssi_cpu_infos g_st_ssi_cpu_infos;

#define SSI_WRITE_DATA 0x5a5a
ssi_trans_test_st ssi_test_st = {0};

uint32 g_ssi_clk  = 0;              /*模拟ssi时钟的GPIO管脚号*/
uint32 g_ssi_data = 0;              /*模拟ssi数据线的GPIO管脚号*/
uint16 g_ssi_base = 0x8000;         /*ssi基址*/
uint32 g_interval = INTERVAL_TIME;  /*GPIO拉出来的波形保持时间，单位us*/
uint32 g_delay    = 5;

/*ssi 工作时必须切换ssi clock,
  此时aon会受到影响，
  BCPU/WCPU 有可能异常，慎用!*/
int32 ssi_try_lock(void)
{
    oal_ulong flags;
    oal_spin_lock_irq_save(&g_ssi_lock, &flags);
    if(g_ssi_lock_state)
    {
        /*lock failed*/
        oal_spin_unlock_irq_restore(&g_ssi_lock, &flags);
        return 1;
    }
    g_ssi_lock_state = 1;
    oal_spin_unlock_irq_restore(&g_ssi_lock, &flags);
    return 0;
}

int32 ssi_unlock(void)
{
    oal_ulong flags;
    oal_spin_lock_irq_save(&g_ssi_lock, &flags);
    g_ssi_lock_state = 0;
    oal_spin_unlock_irq_restore(&g_ssi_lock, &flags);
    return 0;
}
int32 wait_for_ssi_idle_timeout(int32 mstimeout)
{
    int32 can_sleep = 0;
    int32 timeout=mstimeout;
    if (oal_in_interrupt() || oal_in_atomic() || irqs_disabled()) {
        can_sleep = 0;
    }
    else {
        can_sleep = 1;
    }
    /*考虑效率，这里需要判断是否可以睡眠*/
    while(ssi_try_lock()){
        if (can_sleep) {
            msleep(1);
        }else {
            mdelay(1);
        }
        if(!(--timeout)) {
                PS_PRINT_ERR("wait for ssi timeout:%dms\n",mstimeout);
                return 0;
        }
    }
    ssi_unlock();
    return timeout;
}
int32 ssi_show_setup(void)
{
    PS_PRINT_INFO("clk=%d, data=%d, interval=%d us, ssi base=0x%x, r/w delay=%d cycle\n",
                    g_ssi_clk, g_ssi_data, g_interval, g_ssi_base, g_delay);
    return BOARD_SUCC;
}

int32 ssi_setup(uint32 interval, uint32 delay, uint16 ssi_base)
{
    g_interval    = interval;
    g_delay       = delay;
    g_ssi_base    = ssi_base;

    return BOARD_SUCC;
}

int32 ssi_request_gpio(uint32 clk, uint32 data)
{
    int32 ret = BOARD_FAIL;

    PS_PRINT_DBG("request hi110x ssi GPIO\n");
    PS_PRINT_ERR("clk = %u, data = %u\n", clk, data);
#ifdef GPIOF_OUT_INIT_LOW
    ret = gpio_request_one(clk, GPIOF_OUT_INIT_LOW, HI110X_SSI_CLK_GPIO_NAME);
    if (ret)
    {
        PS_PRINT_ERR("%s gpio_request_one failed ret=%d\n", HI110X_SSI_CLK_GPIO_NAME, ret);
        goto err_get_ssi_clk_gpio;
    }

    g_ssi_clk = clk;

    ret = gpio_request_one(data, GPIOF_OUT_INIT_LOW, HI110X_SSI_DATA_GPIO_NAME);
    if (ret)
    {
        PS_PRINT_ERR("%s gpio_request_one failed ret=%d\n", HI110X_SSI_DATA_GPIO_NAME, ret);
        goto err_get_ssi_data_gpio;
    }
#else
    ret = gpio_request(clk,  HI110X_SSI_CLK_GPIO_NAME);
    if(ret)
    {
        PS_PRINT_ERR("%s gpio_request failed  ret=%d\n", HI110X_SSI_CLK_GPIO_NAME, ret);
        goto err_get_ssi_clk_gpio;
    }

    gpio_direction_output(clk, 0);

    ret = gpio_request(data, HI110X_SSI_DATA_GPIO_NAME);
    if (ret)
    {
        PS_PRINT_ERR("%s gpio_request failed  ret=%d\n", HI110X_SSI_DATA_GPIO_NAME, ret);
        goto err_get_ssi_data_gpio;
    }

    gpio_direction_output(data, 0);
#endif
    g_ssi_data = data;

    return BOARD_SUCC;

err_get_ssi_data_gpio:
    gpio_free(clk);
    g_ssi_clk = 0;
err_get_ssi_clk_gpio:

    CHR_EXCEPTION_REPORT(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_WIFI, CHR_LAYER_DEV, CHR_WIFI_DEV_EVENT_CHIP, CHR_WIFI_DEV_ERROR_GPIO);

    return ret;
}

int32 ssi_free_gpio(void)
{
    PS_PRINT_DBG("free hi110x ssi GPIO\n");

    if (0 != g_ssi_clk)
    {
        gpio_free(g_ssi_clk);
        g_ssi_clk = 0;
    }

    if (0 != g_ssi_data)
    {
        gpio_free(g_ssi_data);
        g_ssi_data = 0;
    }

    return BOARD_SUCC;
}

void ssi_clk_output(void)
{
    gpio_direction_output(g_ssi_clk, GPIO_LOWLEVEL);
    SSI_DELAY(g_interval);
    gpio_direction_output(g_ssi_clk, GPIO_HIGHLEVEL);
}

void ssi_data_output(uint16 data)
{
    SSI_DELAY(5);
    if (data)
    {
        gpio_direction_output(g_ssi_data, GPIO_HIGHLEVEL);
    }
    else
    {
        gpio_direction_output(g_ssi_data, GPIO_LOWLEVEL);
    }

    SSI_DELAY(g_interval);
}

int32 ssi_write_data(uint16 addr, uint16 value)
{
    uint16 tx;
    uint32 i;

    for (i = 0; i < g_delay; i++)
    {
        ssi_clk_output();
        ssi_data_output(0);
    }

    /*发送SYNC位*/
    PS_PRINT_DBG("tx sync bit\n");
    ssi_clk_output();
    ssi_data_output(1);

    /*指示本次操作为写，高读低写*/
    PS_PRINT_DBG("tx r/w->w\n");
    ssi_clk_output();
    ssi_data_output(0);

    /*发送地址*/
    PS_PRINT_DBG("write addr:0x%x\n", addr);
    for (i = 0; i < SSI_DATA_LEN; i++)
    {
        tx = (addr >> (SSI_DATA_LEN - i - 1)) & 0x0001;
        PS_PRINT_DBG("tx addr bit %d:%d\n", SSI_DATA_LEN - i - 1, tx);
        ssi_clk_output();
        ssi_data_output(tx);
    }

    /*发送数据*/
    PS_PRINT_DBG("write value:0x%x\n", value);
    for (i = 0; i < SSI_DATA_LEN; i++)
    {
        tx = (value >> (SSI_DATA_LEN - i - 1)) & 0x0001;
        PS_PRINT_DBG("tx data bit %d:%d\n", SSI_DATA_LEN - i - 1, tx);
        ssi_clk_output();
        ssi_data_output(tx);
    }

    /*数据发送完成以后，保持delay个周期的0*/
    PS_PRINT_DBG("ssi write:finish, delay %d cycle\n", g_delay);
    for (i = 0; i < g_delay; i++)
    {
        ssi_clk_output();
        ssi_data_output(0);
    }

    return BOARD_SUCC;
}

uint16 ssi_read_data(uint16 addr)
{
#define SSI_READ_RETTY (1000)
    uint16 tx;
    uint32 i;
    uint32 retry = 0;
    uint16 rx;
    uint16 data = 0;

    for (i = 0; i < g_delay; i++)
    {
        ssi_clk_output();
        ssi_data_output(0);
    }

    /*发送SYNC位*/
    PS_PRINT_DBG("tx sync bit\n");
    ssi_clk_output();
    ssi_data_output(1);

    /*指示本次操作为读，高读低写*/
    PS_PRINT_DBG("tx r/w->r\n");
    ssi_clk_output();
    ssi_data_output(1);

    /*发送地址*/
    PS_PRINT_DBG("read addr:0x%x\n", addr);
    for (i = 0; i < SSI_DATA_LEN; i++)
    {
        tx = (addr >> (SSI_DATA_LEN - i - 1)) & 0x0001;
        PS_PRINT_DBG("tx addr bit %d:%d\n", SSI_DATA_LEN - i - 1, tx);
        ssi_clk_output();
        ssi_data_output(tx);
    }

    /*延迟一个clk，否则上一个数据只保持了半个时钟周期*/
    ssi_clk_output();

    /*设置data线GPIO为输入，准备读取数据*/
    gpio_direction_input(g_ssi_data);

    PS_PRINT_DBG("data in mod, current gpio level is %d\n", gpio_get_value(g_ssi_data));

    /*读取SYNC同步位*/
    do
    {
        ssi_clk_output();
        SSI_DELAY(g_interval);
        if(gpio_get_value(g_ssi_data))
        {
            PS_PRINT_DBG("read data sync bit ok, retry=%d\n", retry);
            break;
        }
        retry++;
    }while(SSI_READ_RETTY != retry);

    if (SSI_READ_RETTY == retry)
    {
        PS_PRINT_ERR("ssi read sync bit timeout\n");
        ssi_data_output(0);
        return data;
    }

    for (i = 0; i < SSI_DATA_LEN; i++)
    {
        ssi_clk_output();
        SSI_DELAY(g_interval);
        rx = gpio_get_value(g_ssi_data);
        PS_PRINT_DBG("rx data bit %d:%d\n", SSI_DATA_LEN - i - 1, rx);
        data = data | (rx << (SSI_DATA_LEN - i - 1));
    }

    /*恢复data线GPIO为输出，并输出0*/
    ssi_data_output(0);

    return data;
}

int32 ssi_write16(uint16 addr, uint16 value)
{
#define write_retry   (3)
    uint32 retry = 0;
    uint16 read_v;

    do
    {
        ssi_write_data(addr, value);
        read_v = ssi_read_data(addr);
        if (value == read_v)
        {
            PS_PRINT_DBG("ssi write: 0x%x=0x%x succ\n", addr, value);
            return BOARD_SUCC;
        }
        retry++;
    }while(retry < write_retry);

    PS_PRINT_ERR("ssi write: 0x%x=0x%x ,read=0x%x fail\n", addr, value, read_v);

    return BOARD_FAIL;
}

uint16 ssi_read16(uint16 addr)
{
    uint16 data;

    data = ssi_read_data(addr);

    PS_PRINT_SUC("ssi read: 0x%x=0x%x\n", addr, data);

    return data;
}

int32 ssi_write32(uint32 addr, uint16 value)
{
    uint16 addr_half_word_high;
    uint16 addr_half_word_low;

    addr_half_word_high = (addr >> 16) & 0xffff;
    addr_half_word_low  = (addr & 0xffff) >> 1;

    /*往基地址写地址的高16位*/
    if (ssi_write16(g_ssi_base, addr_half_word_high) < 0)
    {
        PS_PRINT_ERR("ssi write: 0x%x=0x%x fail\n", addr, value);
        return BOARD_FAIL;
    }

    /*低地址写实际要写入的value*/
    if (ssi_write16(addr_half_word_low, value) < 0)
    {
        PS_PRINT_ERR("ssi write: 0x%x=0x%x fail\n", addr, value);
        return BOARD_FAIL;
    }

    PS_PRINT_DBG("ssi write: 0x%x=0x%x succ\n", addr, value);

    return BOARD_SUCC;
}

int32 ssi_read32(uint32 addr)
{
    uint16  data = 0;
    uint16 addr_half_word_high;
    uint16 addr_half_word_low;

    addr_half_word_high = (addr >> 16) & 0xffff;
    addr_half_word_low  = (addr & 0xffff) >> 1;

    if (ssi_write16(g_ssi_base, addr_half_word_high) < 0)
    {
        PS_PRINT_ERR("ssi read 0x%x fail\n", addr);
        return BOARD_FAIL;
    }

    data = ssi_read_data(addr_half_word_low);

    PS_PRINT_DBG("ssi read: 0x%x=0x%x\n", addr, data);

    return data;
}

int32 ssi_read_data16(uint16 addr, uint16 *value)
{
#define SSI_READ_RETTY (1000)
    uint16 tx;
    uint32 i;
    uint32 retry = 0;
    uint16 rx;
    uint16 data = 0;

    for (i = 0; i < g_delay; i++)
    {
        ssi_clk_output();
        ssi_data_output(0);
    }

    /*发送SYNC位*/
    PS_PRINT_DBG("tx sync bit\n");
    ssi_clk_output();
    ssi_data_output(1);

    /*指示本次操作为读，高读低写*/
    PS_PRINT_DBG("tx r/w->r\n");
    ssi_clk_output();
    ssi_data_output(1);

    /*发送地址*/
    PS_PRINT_DBG("read addr:0x%x\n", addr);
    for (i = 0; i < SSI_DATA_LEN; i++)
    {
        tx = (addr >> (SSI_DATA_LEN - i - 1)) & 0x0001;
        PS_PRINT_DBG("tx addr bit %d:%d\n", SSI_DATA_LEN - i - 1, tx);
        ssi_clk_output();
        ssi_data_output(tx);
    }

    /*延迟一个clk，否则上一个数据只保持了半个时钟周期*/
    ssi_clk_output();

    /*设置data线GPIO为输入，准备读取数据*/
    gpio_direction_input(g_ssi_data);

    PS_PRINT_DBG("data in mod, current gpio level is %d\n", gpio_get_value(g_ssi_data));

    /*读取SYNC同步位*/
    do
    {
        ssi_clk_output();
        SSI_DELAY(g_interval);
        if(gpio_get_value(g_ssi_data))
        {
            PS_PRINT_DBG("read data sync bit ok, retry=%d\n", retry);
            break;
        }
        retry++;
    }while(SSI_READ_RETTY != retry);

    if (SSI_READ_RETTY == retry)
    {
        PS_PRINT_ERR("ssi read sync bit timeout\n");
        ssi_data_output(0);
        return -OAL_EFAIL;
    }

    for (i = 0; i < SSI_DATA_LEN; i++)
    {
        ssi_clk_output();
        SSI_DELAY(g_interval);
        rx = gpio_get_value(g_ssi_data);
        PS_PRINT_DBG("rx data bit %d:%d\n", SSI_DATA_LEN - i - 1, rx);
        data = data | (rx << (SSI_DATA_LEN - i - 1));
    }

    /*恢复data线GPIO为输出，并输出0*/
    ssi_data_output(0);

    *value = data;

    return OAL_SUCC;
}

/*32bits address,
  32bits value*/
int32 ssi_read_value16(uint32 addr, uint16* value, int16 last_high_addr)
{
    int32 ret;
    uint16 addr_half_word_high;
    uint16 addr_half_word_low;

    addr_half_word_high = (addr >> 16) & 0xffff;
    addr_half_word_low  = (addr & 0xffff) >> 1;

    if(last_high_addr != addr_half_word_high)
    {
        if (ssi_write16(g_ssi_base, addr_half_word_high) < 0)
        {
            PS_PRINT_ERR("ssi read 0x%x fail\n", addr);
            return BOARD_FAIL;
        }
    }

    ret = ssi_read_data16(addr_half_word_low, value);

    PS_PRINT_DBG("ssi read: 0x%x=0x%x\n", addr, *value);

    return ret;
}

/*32bits address,
  32bits value
  gpio模拟SSI 读32BIT value
  1.配置SSI 为32BIT模式
  2.第一次读16BIT操作，SOC发起32BIT操作，返回低16BIT给HOST
  3.第二次读同一地址16BIT操作，SOC不发起总线操作，返回高16BIT给HOST
  4.如果跳过步骤3 读其他地址，SOC侧高16BIT 会被丢弃*/
int32 ssi_read_value32(uint32 addr, uint32* value, int16 last_high_addr)
{
    int32 ret;
    uint16 reg;

    ret = ssi_read_value16(addr, &reg, last_high_addr);
    if(ret)
    {
        PS_PRINT_ERR("read addr 0x%x low 16 bit failed, ret=%d\n", addr, ret);
        return ret;
    }
    *value = (uint32)reg;

    ret = ssi_read_value16(addr + 0x2, &reg, (addr >> 16));
    if(ret)
    {
        PS_PRINT_ERR("read addr 0x%x high 16 bit failed, ret=%d\n", addr, ret);
        return ret;
    }

    *value = ((reg << 16) | *value);

    return OAL_SUCC;
}

/*16bits/32bits switch mode*/
int32 ssi_switch_ahb_mode(oal_int32 is_32bit_mode)
{
    return ssi_write16(0x8001, !!is_32bit_mode);
}

int32 ssi_clear_ahb_highaddr(void)
{
    return ssi_write16(g_ssi_base, 0x0);;
}

int32 do_ssi_file_test(ssi_file_st *file_st, ssi_trans_test_st* pst_ssi_test)
{
    OS_KERNEL_FILE_STRU        *fp;
    uint16 data_buf = 0;
    int32 rdlen = 0;
    uint32 ul_addr = 0;
    int32 l_ret = BOARD_FAIL;

    if ((NULL == pst_ssi_test) || ( NULL == file_st))
    {
        return BOARD_FAIL;
    }
    fp = filp_open(file_st->file_name, O_RDONLY, 0);
    if (IS_ERR(fp))
    {
        fp = NULL;
        PS_PRINT_ERR("filp_open %s fail!!\n",  file_st->file_name);
        return -EFAIL;
    }
    ul_addr = file_st->write_addr;
    PS_PRINT_INFO("begin file:%s", file_st->file_name);
    while(1)
    {
        data_buf = 0;
        rdlen = kernel_read(fp, fp->f_pos, (uint8 *)&data_buf, 2);
        if (rdlen > 0)
        {
            fp->f_pos += rdlen;
        }
        else if (0 == rdlen)
        {
            PS_PRINT_INFO("file read over:%s!!\n",  file_st->file_name);
            break;
        }
        else
        {
            PS_PRINT_ERR("file read ERROR:%d!!\n", rdlen);
            goto test_fail;
        }
        l_ret = ssi_write32(ul_addr, data_buf);
        if (BOARD_SUCC != l_ret)
        {
            PS_PRINT_ERR(" write data error, ul_addr=0x%x, l_ret=%d\n", ul_addr, l_ret);
            goto test_fail;
        }
        pst_ssi_test->trans_len += 2;
        ul_addr +=2;
    }
    filp_close(fp, NULL);
    fp = NULL;
    PS_PRINT_INFO("%s send finish\n",  file_st->file_name);
    return BOARD_SUCC;
test_fail:
    filp_close(fp, NULL);
    fp = NULL;
    return BOARD_FAIL;
}
typedef struct ht_test_s {
    int32 add;
    int32 data;
}ht_test_t;

ht_test_t ht_cnt[]={
    {0x50000314,    0x0D00},
    {0x50002724,    0x0022},
    {0x50002720,    0x0033},
};
int32 test_hd_ssi_write(void)
{
    int32 i;
    if (BOARD_SUCC != ssi_request_gpio(SSI_CLK_GPIO, SSI_DATA_GPIO))
    {
        PS_PRINT_ERR("ssi_request_gpio fail\n");
        return BOARD_FAIL;
    }

    if (BOARD_SUCC != ssi_write16(0x8007,1))
    {
         PS_PRINT_ERR("set ssi clk fail\n");
         goto err_exit;
    }
    for (i=0;i<sizeof(ht_cnt)/sizeof(ht_test_t);i++)
    {
        if (0 != ssi_write32(ht_cnt[i].add, ht_cnt[i].data))
        {
            PS_PRINT_ERR("error: ssi write fail s_addr:0x%x s_data:0x%x\n", ht_cnt[i].add,ht_cnt[i].data);
            //return BOARD_FAIL;
        }
        else
        {
            //PS_PRINT_ERR("0x%x:0x%x succ\n", ht_cnt[i].add,ht_cnt[i].data);
        }
    }

    /*reset clk*/
    if (BOARD_SUCC != ssi_write16(0x8007,0))
    {
         PS_PRINT_ERR("set ssi clk fail\n");
         goto err_exit;
    }
    if (BOARD_SUCC != ssi_free_gpio())
    {
        PS_PRINT_ERR("ssi_request_gpio fail\n");
        return BOARD_FAIL;
    }
    PS_PRINT_ERR("ALL reg finish---------------------");
    return 0;
err_exit:
    PS_PRINT_ERR("test reg fail---------------------");
    ssi_free_gpio();
    return BOARD_FAIL;

}
int32 ssi_single_write(int32 addr, int16 data)
{
    if (BOARD_SUCC != ssi_request_gpio(SSI_CLK_GPIO, SSI_DATA_GPIO))
    {
        PS_PRINT_ERR("ssi_request_gpio fail\n");
        return BOARD_FAIL;
    }

    if (BOARD_SUCC != ssi_write16(0x8007,1))
    {
         PS_PRINT_ERR("set ssi clk fail\n");
         goto err_exit;
    }
    /*set wcpu wait*/
    if (BOARD_SUCC != ssi_write32(addr, data))
    {
         goto err_exit;
    }
    /*reset clk*/
    if (BOARD_SUCC != ssi_write16(0x8007,0))
    {
         PS_PRINT_ERR("set ssi clk fail\n");
         goto err_exit;
    }
    if (BOARD_SUCC != ssi_free_gpio())
    {
        PS_PRINT_ERR("ssi_request_gpio fail\n");
        return BOARD_FAIL;
    }
    return 0;
err_exit:
    ssi_free_gpio();
    return BOARD_FAIL;
}
int32 ssi_single_read(int32 addr)
{
    int32 ret;
    if (BOARD_SUCC != ssi_request_gpio(SSI_CLK_GPIO, SSI_DATA_GPIO))
    {
        PS_PRINT_ERR("ssi_request_gpio fail\n");
        return BOARD_FAIL;
    }
    if (BOARD_SUCC != ssi_write16(0x8007,1))
    {
         PS_PRINT_ERR("set ssi clk fail\n");
         goto err_exit;
    }
    ret = ssi_read32(addr);
    /*reset clk*/
    if (BOARD_SUCC != ssi_write16(0x8007,0))
    {
         PS_PRINT_ERR("set ssi clk fail\n");
         goto err_exit;
    }
    if (BOARD_SUCC != ssi_free_gpio())
    {
        PS_PRINT_ERR("ssi_request_gpio fail\n");
        return BOARD_FAIL;
    }
    return ret;
err_exit:
    ssi_free_gpio();
    return BOARD_FAIL;
}
int32 ssi_file_test(ssi_trans_test_st* pst_ssi_test)
{
    int32 i = 0;
    if (NULL == pst_ssi_test)
    {
        return BOARD_FAIL;
    }
    pst_ssi_test->trans_len = 0;

#ifndef BFGX_UART_DOWNLOAD_SUPPORT
    hi1103_chip_power_on();
    hi1103_bfgx_enable();
    if(hi1103_wifi_enable())
    {
        PS_PRINT_ERR("hi1103_wifi_enable failed!\n");
        return BOARD_FAIL;
    }
#endif

    //ssi_setup(20, 10, g_ssi_base);
    //waring: fpga version should set 300801c0 1 to let host control ssi
    /*first set ssi clk ctl*/
    if (BOARD_SUCC != ssi_write16(0x8007,1))
    {
         PS_PRINT_ERR("set ssi clk fail\n");
         return BOARD_FAIL;
    }
    //env init
#ifdef BFGX_UART_DOWNLOAD_SUPPORT
    /*set bootloader deadbeaf*/
    if (BOARD_SUCC != ssi_write32(0x8010010c, 0xbeaf))
    {
         PS_PRINT_ERR("set flag:beaf fail\n");
         return BOARD_FAIL;
    }
    if (BOARD_SUCC != ssi_write32(0x8010010e, 0xdead))
    {
         PS_PRINT_ERR("set flag:dead fail\n");
         return BOARD_FAIL;
    }
#else
    /*set wcpu wait*/
    if (BOARD_SUCC != ssi_write32(0x50000e00, 0x1))
    {
         PS_PRINT_ERR("set wcpu wait fail\n");
         return BOARD_FAIL;
    }

    /*reset wcpu */
    if (BOARD_SUCC != ssi_write32(0x40000030, 0xfe5e))
    {
         //脉冲复位
         //PS_PRINT_ERR("reset wcpu fail\n");
         //return BOARD_FAIL;
    }
    /*boot flag*/
    if (BOARD_SUCC != ssi_write32(0x50000200, 0xbeaf))
    {
         PS_PRINT_ERR("set boot flag fail\n");
         return BOARD_FAIL;
    }
    /*dereset bcpu*/
    if (BOARD_SUCC != ssi_write32(0x50000094, 1))
    {
         PS_PRINT_ERR("dereset bcpu\n");
         return BOARD_FAIL;
    }
#endif
    /*file download*/
    for (i = 0; i < sizeof(g_aSsiFile)/sizeof(ssi_file_st); i++)
    {
        if (BOARD_SUCC != do_ssi_file_test(&g_aSsiFile[i], pst_ssi_test))
        {
            PS_PRINT_ERR("%s write %d error\n", g_aSsiFile[i].file_name, g_aSsiFile[i].write_addr);
            return BOARD_FAIL;
        }
    }
    /*let cpu go*/
#ifdef BFGX_UART_DOWNLOAD_SUPPORT
    /*reset bcpu*/
    if (BOARD_SUCC != ssi_write32(0x50000094, 0))
    {
         PS_PRINT_ERR("reset bcpu set 0 fail\n");
         return BOARD_FAIL;
    }
    if (BOARD_SUCC != ssi_write32(0x50000094, 1))
    {
         PS_PRINT_ERR("reset bcpu set 1 fail\n");
         return BOARD_FAIL;
    }
#else
    /*clear b wait*/
    if (BOARD_SUCC != ssi_write32(0x50000e04, 0x0))
    {
         PS_PRINT_ERR("clear b wait\n");
         return BOARD_FAIL;
    }
    /*clear w wait*/
    if (BOARD_SUCC != ssi_write32(0x50000e00, 0x0))
    {
         PS_PRINT_ERR("clear w wait\n");
         return BOARD_FAIL;
    }
#endif
    /*reset clk*/
    if (BOARD_SUCC != ssi_write16(0x8007,0))
    {
         PS_PRINT_ERR("set ssi clk fail\n");
         return BOARD_FAIL;
    }
    return BOARD_SUCC;
}
int32 do_ssi_mem_test(ssi_trans_test_st* pst_ssi_test)
{
    uint32 i = 0;
    uint32 ul_write_base = 0x0;
    uint32 ul_addr;
    int32 l_ret = BOARD_FAIL;
    if (NULL == pst_ssi_test)
    {
        return BOARD_FAIL;
    }

    for (i = 0; i < pst_ssi_test->trans_len; i++ )
    {
        ul_addr = ul_write_base + 2*i;  //按2字节读写
        l_ret = ssi_write32(ul_addr, SSI_WRITE_DATA);
        if (BOARD_SUCC != l_ret)
        {
            PS_PRINT_ERR(" write data error, ul_addr=0x%x, l_ret=%d\n", ul_addr, l_ret);
            return l_ret;
        }
        l_ret = ssi_read32(ul_addr);
        if (SSI_WRITE_DATA != l_ret)
        {
            PS_PRINT_ERR("read write 0x%x error, expect:0x5a5a,actual:0x%x\n",ul_addr, l_ret);
            return l_ret;
        }
    }
    return BOARD_SUCC;
}
int32 ssi_download_test(ssi_trans_test_st* pst_ssi_test)
{

    int32 l_ret = BOARD_FAIL;

    struct timeval stime,etime;

    if (NULL == pst_ssi_test)
    {
        return BOARD_FAIL;
    }
    pst_ssi_test->trans_len = 1024;
    if (BOARD_SUCC != ssi_request_gpio(SSI_CLK_GPIO, SSI_DATA_GPIO))
    {
        PS_PRINT_ERR("ssi_request_gpio fail\n");
        goto fail_process;
    }

    do_gettimeofday(&stime);
    switch (pst_ssi_test->test_type)
    {
        case SSI_MEM_TEST:
            l_ret = do_ssi_mem_test(pst_ssi_test);
            break;
        case SSI_FILE_TEST:
            l_ret = ssi_file_test(pst_ssi_test);
            break;
        default:
            PS_PRINT_ERR("error type=%d\n", pst_ssi_test->test_type);
            break;
    }
    do_gettimeofday(&etime);
    ssi_free_gpio();
    if (BOARD_SUCC != l_ret)
    {
        goto fail_process;
    }
    pst_ssi_test->used_time = (etime.tv_sec - stime.tv_sec)*1000 + (etime.tv_usec - stime.tv_usec)/1000;
    pst_ssi_test->send_status = 0;
    return BOARD_SUCC;
fail_process:
    pst_ssi_test->used_time = 0;
    pst_ssi_test->send_status = -1;
    return BOARD_FAIL;

}

ssi_reg_info hi1103_glb_ctrl_full     = {0x50000000, 0x1000, SSI_RW_WORD_MOD};
ssi_reg_info hi1103_pmu_cmu_ctrl_full = {0x50002000, 0xb00, SSI_RW_WORD_MOD};
ssi_reg_info hi1103_pmu2_cmu_ir_ctrl_full = {0x50003000, 0xa20, SSI_RW_WORD_MOD};
ssi_reg_info hi1103_pmu2_cmu_ir_ctrl_tail = {0x50003a80, 0xc, SSI_RW_WORD_MOD};
ssi_reg_info hi1103_w_ctrl_full =    {0x40000000, 0x408, SSI_RW_WORD_MOD};
ssi_reg_info hi1103_w_key_mem  =    {0x2001e620, 0x80, SSI_RW_DWORD_MOD};
ssi_reg_info hi1103_b_ctrl_full =    {0x48000000, 0x40c, SSI_RW_WORD_MOD};
ssi_reg_info hi1103_pcie_ctrl_full = {0x40007000, 0x488, SSI_RW_DWORD_MOD};
ssi_reg_info hi1103_pcie_dbi_full = {0x40102000, 0x900, SSI_RW_DWORD_MOD};/*没建链之前不能读*/
ssi_reg_info hi1103_pcie_pilot_iatu_full = {0x40104000, 0x2000, SSI_RW_DWORD_MOD};/*8KB*/
ssi_reg_info hi1103_pcie_pilot_dma_full = {0x40106000, 0x1000, SSI_RW_DWORD_MOD};/*4KB*/
ssi_reg_info hi1103_pcie_dma_ctrl_full = {0x40008000, 0x34, SSI_RW_DWORD_MOD};
ssi_reg_info hi1103_pcie_sdio_ctrl_full = {0x40101000, 0x180, SSI_RW_DWORD_MOD};


ssi_reg_info hi1102a_w_ctrl_full        = {0x40000000, 0x8A0, SSI_RW_WORD_MOD};
ssi_reg_info hi1102a_sdio_ctrl_full     = {0x40201000, 0x180, SSI_RW_DWORD_MOD};
ssi_reg_info hi1102a_b_ctrl_full        = {0x48000000, 0x70c, SSI_RW_WORD_MOD};

ssi_reg_info hi1102a_int_vector_full    = {0x70000, 0x200, SSI_RW_DWORD_MOD};

ssi_reg_info hi1102a_glb_ctrl_full      = {0x50000000, 0xF68+0x4, SSI_RW_WORD_MOD};
ssi_reg_info hi1102a_pmu_cmu_ctrl_full  = {0x50002000, 0xD38+0x4, SSI_RW_WORD_MOD};

ssi_reg_info hi1102a_com_ctl_full  = {0x50014000, 0x650+0x4, SSI_RW_WORD_MOD};

ssi_reg_info *hi1102a_aon_reg_full[] = {
    &hi1102a_glb_ctrl_full,
    &hi1102a_pmu_cmu_ctrl_full,
};

ssi_reg_info hi1102a_tcxo_detect_reg1  = {0x500000c0, 0x28, SSI_RW_WORD_MOD};
ssi_reg_info hi1102a_tcxo_detect_reg2  = {0x50000600, 0x14, SSI_RW_WORD_MOD};
ssi_reg_info *hi1102a_tcxo_detect_regs[] = {
    &hi1102a_tcxo_detect_reg1,
    &hi1102a_tcxo_detect_reg2,
};

ssi_reg_info *hi1103_aon_reg_full[] = {
    &hi1103_glb_ctrl_full,
    &hi1103_pmu_cmu_ctrl_full,
    &hi1103_pmu2_cmu_ir_ctrl_full,
    &hi1103_pmu2_cmu_ir_ctrl_tail
};
ssi_reg_info *hi1103_pcie_cfg_reg_full[] = {
    &hi1103_pcie_ctrl_full,
    &hi1103_pcie_dma_ctrl_full
};

ssi_reg_info *hi1103_pcie_dbi_mpw2_reg_full[] = {
    &hi1103_pcie_dbi_full,
};

ssi_reg_info *hi1103_pcie_dbi_pilot_reg_full[] = {
    &hi1103_pcie_dbi_full,
};

int ssi_check_device_isalive(void)
{
    int i;
    uint32 reg;
    for(i = 0; i < 2; i++)
    {
        reg = (uint32)ssi_read32(0x50000000);
        if(0x101 == reg)
        {
            PS_PRINT_INFO("reg is 0x%x\n", reg);
            break;
        }
    }

    if(2 == i)
    {
        PS_PRINT_INFO("ssi is fail, gpio-ssi did't support, reg=0x%x\n", reg);
        return -1;
    }
    return 0;
}

int ssi_check_is_pilot(void)
{
    //int32 ret   = 1;
    //uint16 value = 0;
    /*pilot pmuctrl 0x598 is reserved*/

    return 1;
#if 0
    if(-1 == ssi_is_pilot)
    {
        ret = ssi_read_value16(0x50002598, &value, 0x0);
        if(ret)
        {
            PS_PRINT_ERR("read 0x50002598 failed\n");
            return ret;
        }
        else
        {
            PS_PRINT_INFO("value=0x%x [%s]\n", value, value ? "mpw2":"pilot");
            if(value)
            {
                ssi_is_pilot = 0;
                return 0;
            }
            else
            {
                ssi_is_pilot = 1;
                return 1;
            }
        }
    }
    else
    {
        PS_PRINT_INFO("%s\n", ssi_is_pilot ? "pilot":"mpw2");
        return ssi_is_pilot;
    }
#endif
}

int ssi_read_wpcu_pc_lr_sp(int trace_en)
{
    int i;
    uint32 reg_low, reg_high, pc, lr, sp;

    /*1102A 采PC逻辑处于com_ctl*/
    for(i = 0; i < SSI_CPU_ARM_REG_DUMP_CNT; i++)
    {
        //ssi_write32(0x40020804, 0x1);
        //oal_mdelay(1);

        /*w/b clk en*/
        ssi_write32(0x50013264, 0x18);/*debug uart ,IPC, RF，ABB(ADC),coex,etc..(单系统不在com_ctl)*/
        oal_mdelay(1);

        /* wcpu pc load */
        ssi_write32(0x500132C0, 0x1);
        oal_mdelay(1);

        reg_low = (uint32)ssi_read32(0x500132C8);
        reg_high = (uint32)ssi_read32(0x500132CC);
        //PS_PRINT_INFO("low:0x%x, high:0x%x\n", reg_low, reg_high);
        pc = reg_low | (reg_high << 16);

        reg_low = (uint32)ssi_read32(0x500132D0);
        reg_high = (uint32)ssi_read32(0x500132D4);
        //PS_PRINT_INFO("low:0x%x, high:0x%x\n", reg_low, reg_high);
        lr = reg_low | (reg_high << 16);

        reg_low = (uint32)ssi_read32(0x500132D8);
        reg_high = (uint32)ssi_read32(0x500132DC);
        //PS_PRINT_INFO("low:0x%x, high:0x%x\n", reg_low, reg_high);
        sp = reg_low | (reg_high << 16);

        PS_PRINT_INFO("gpio-ssi:read wcpu[%i], pc:0x%x, lr:0x%x, sp:0x%x \n", i, pc, lr, sp);
        if(!pc && !lr && !sp)
        {
            PS_PRINT_INFO("wcpu pc lr sp all zero\n");
            if(trace_en)
            {
                if(ssi_check_wcpu_is_working())
                {
                    PS_PRINT_INFO("wcpu try to enable trace en\n");
                    ssi_write32(0x40020804, 0x1);
                    oal_mdelay(1);
                }
                trace_en = 0;
                i = -1;
            }
        }
        else
        {
            if(0 == g_st_ssi_cpu_infos.wcpu_info.reg_flag[i])
            {
                g_st_ssi_cpu_infos.wcpu_info.reg_flag[i] = 1;
                g_st_ssi_cpu_infos.wcpu_info.pc[i] = pc;
                g_st_ssi_cpu_infos.wcpu_info.lr[i] = lr;
                g_st_ssi_cpu_infos.wcpu_info.sp[i] = sp;
            }
        }
        oal_mdelay(10);
    }

    return 0;
}

int ssi_read_bpcu_pc_lr_sp(int trace_en)
{
    int i;
    uint32 reg_low, reg_high, pc, lr, sp;

    /*read pc twice check whether wcpu is runing*/
    for(i = 0; i < SSI_CPU_ARM_REG_DUMP_CNT; i++)
    {
        //ssi_write32(0x4B080c00, 0x1);
        //oal_mdelay(1);

        ssi_write32(0x50013264, 0xc);
        oal_mdelay(1);

        ssi_write32(0x500132E0, 0x1);
        oal_mdelay(1);

        reg_low = (uint32)ssi_read32(0x500132E8);
        reg_high = (uint32)ssi_read32(0x500132EC);
        //PS_PRINT_INFO("low:0x%x, high:0x%x\n", reg_low, reg_high);
        pc = reg_low | (reg_high << 16);

        reg_low = (uint32)ssi_read32(0x500132F0);
        reg_high = (uint32)ssi_read32(0x500132F4);
        //PS_PRINT_INFO("low:0x%x, high:0x%x\n", reg_low, reg_high);
        lr = reg_low | (reg_high << 16);

        reg_low = (uint32)ssi_read32(0x500132F8);
        reg_high = (uint32)ssi_read32(0x500132FC);
        //PS_PRINT_INFO("low:0x%x, high:0x%x\n", reg_low, reg_high);
        sp = reg_low | (reg_high << 16);

        PS_PRINT_INFO("gpio-ssi:read bcpu[%i], pc:0x%x, lr:0x%x, sp:0x%x \n", i, pc, lr, sp);
        if(!pc && !lr && !sp)
        {
            PS_PRINT_INFO("bcpu pc lr sp all zero\n");
            if(trace_en)
            {
                if(ssi_check_bcpu_is_working())
                {
                    PS_PRINT_INFO("bcpu try to enable trace en\n");
                    ssi_write32(0x4B080c00, 0x1);
                    oal_mdelay(1);
                }
                trace_en = 0;
                i = -1;
            }
        }
        else
        {
            if(0 == g_st_ssi_cpu_infos.bcpu_info.reg_flag[i])
            {
                g_st_ssi_cpu_infos.bcpu_info.reg_flag[i] = 1;
                g_st_ssi_cpu_infos.bcpu_info.pc[i] = pc;
                g_st_ssi_cpu_infos.bcpu_info.lr[i] = lr;
                g_st_ssi_cpu_infos.bcpu_info.sp[i] = sp;
            }
        }
        oal_mdelay(10);
    }

    return 0;
}
extern int ssi_read_reg_info(ssi_reg_info* pst_reg_info,void* buf , int32 buf_len,oal_int32 is_logfile);

/*1102a pmu status string bit 0~15*/

int ssi_detect_pmu_protect_status(void)
{
    uint32 i;
    int ret = 0;
    uint32 reg, reg_off_rpt, raw, raw_off_rpt;
    int32 count = 0;
    char buf[DSM_CPU_INFO_SIZE];

    reg = (uint32)ssi_read32(0x50002324);
    reg_off_rpt = (uint32)ssi_read32(0x50002380);

    if((0 == reg)&&(0 == reg_off_rpt))
    {
        return 0;
    }

    /*g_ssi_hi1102a_pmu_protect_rpt_str*/

    PS_PRINT_ERR("[ERROR]pmu error detect=0x%x, reg_off_rt=0x%x!\n", reg, reg_off_rpt);
    DECLARE_DFT_TRACE_KEY_INFO("pmu_error_detected", OAL_DFT_TRACE_FAIL);

    /*ocp,ovp status*/
    raw = reg;

    for(i = 0; i < 16; i++)
    {
        if(0 == reg)
        {
            break;
        }
        if(reg & (1 << i))
        {
            uint32 num = sizeof(g_ssi_hi1102a_pmu_protect_st_str)/sizeof(g_ssi_hi1102a_pmu_protect_st_str[0]);
            if(i >= num)
                break;
            PS_PRINT_ERR("[ERROR]pmu error= %s\n", g_ssi_hi1102a_pmu_protect_st_str[i]);
            if(0x70 & (1 << i))
            {
                /*buck ovp or scp*/
                if(DSM_CPU_INFO_SIZE > count)
                {
                    ret = snprintf(buf + count, DSM_CPU_INFO_SIZE - count, " %s", g_ssi_hi1102a_pmu_protect_st_str[i]);
                    if(ret > 0)
                    {
                        count += ret;
                    }
                }
            }
        }

        reg &=~(1 << i);
    }

    /*ocp off rpt*/
    raw_off_rpt = reg_off_rpt;

    for(i = 0; i < 16; i++)
    {
        if(0 == reg_off_rpt)
        {
            break;
        }
        if(reg_off_rpt & (1 << i))
        {
            uint32 num = sizeof(g_ssi_hi1102a_pmu_protect_rpt_str)/sizeof(g_ssi_hi1102a_pmu_protect_rpt_str[0]);
            if(i >= num)
                break;
            PS_PRINT_ERR("[ERROR]pmu error= %s\n", g_ssi_hi1102a_pmu_protect_rpt_str[i]);
            if(0xc3 & (1 << i))
            {
                /*buck ovp or scp*/
                if(DSM_CPU_INFO_SIZE > count)
                {
                    ret = snprintf(buf + count, DSM_CPU_INFO_SIZE - count, " %s", g_ssi_hi1102a_pmu_protect_rpt_str[i]);
                    if(ret > 0)
                    {
                        count += ret;
                    }
                }
            }
        }

        reg_off_rpt &=~(1 << i);
    }

    if(0 == count)
    {
        return 0;
    }

#ifdef CONFIG_HUAWEI_DSM
    hw_1102a_dsm_client_notify(DSM_BUCK_PROTECTED, "buck pmu error=0x%x off_rpt_status=0x%x[%s]\n", raw, raw_off_rpt, buf);
    g_halt_det_cnt++;
#else
    PS_PRINT_ERR("[no-dsm]buck pmu error=0x%x off_rpt_status=0x%x[%s]\n", raw, raw_off_rpt, buf);
#endif
    return 0;
}

void ssi_read_low_power_state(void)
{
    uint32 reg;

    reg = (uint32)ssi_read32(0x50000f68);
    PS_PRINT_INFO("0x50000f68 = 0x%x\n", reg);
    /* sleep stat */
    reg = (uint32)ssi_read32(0x50002b00);
    PS_PRINT_INFO("SYS_STATUS[0x50002b00] = 0x%x\n", reg);
    reg = (uint32)ssi_read32(0x50000aa0);
    PS_PRINT_INFO("BFGN_WAKEUP_EVENT_EN[0x50000aa0] = 0x%x\n", reg);
    reg = (uint32)ssi_read32(0x50000aa4);
    PS_PRINT_INFO("BFGN_WAKEUP_EVENT_STS[0x50000aa4] = 0x%x\n", reg);
    reg = (uint32)ssi_read32(0x50000ab0);
    PS_PRINT_INFO("BFGN_WAKEUP_INT_EN[0x50000ab0] = 0x%x\n", reg);
    reg = (uint32)ssi_read32(0x50000ab4);
    PS_PRINT_INFO("BFGN_WAKEUP_INT_STS[0x50000ab4] = 0x%x\n", reg);

    reg = (uint32)ssi_read32(0x50002300);
    PS_PRINT_INFO("'PMU_STATUS_RAW[0x50002300] = 0x%x\n", reg);
    reg = (uint32)ssi_read32(0x50002304);
    PS_PRINT_INFO("'PMU_STATUS_RAW_STICK[0x50002304] = 0x%x\n", reg);
    reg = (uint32)ssi_read32(0x50002320);
    PS_PRINT_INFO("PMU_STATUS_GRM[0x50002320] = 0x%x\n", reg);
    reg = (uint32)ssi_read32(0x50002324);
    PS_PRINT_INFO("PMU_STATUS_GRM_STICK[0x50002324] = 0x%x\n", reg);
    reg = (uint32)ssi_read32(0x50002380);
    PS_PRINT_INFO("'PMU_PROTECT_STATUS[0x50002380] = 0x%x\n", reg);
    reg = (uint32)ssi_read32(0x50002390);
    PS_PRINT_INFO("PMU_PROTECT_DISABLE[0x50002390] = 0x%x\n", reg);

    reg = (uint32)ssi_read32(0x50002114);
    PS_PRINT_INFO("PMU_STS_0[0x50002114] = 0x%x\n", reg);
    reg = (uint32)ssi_read32(0x50002134);
    PS_PRINT_INFO("PMU_STS_1[0x50002134] = 0x%x\n", reg);
    reg = (uint32)ssi_read32(0x50002150);
    PS_PRINT_INFO("PMU_MAN_STS_2[0x50002150] = 0x%x\n", reg);
    reg = (uint32)ssi_read32(0x50002170);
    PS_PRINT_INFO("PMU_STS_3[0x50002170] = 0x%x\n", reg);
    reg = (uint32)ssi_read32(0x5000218c);
    PS_PRINT_INFO("EN_PALDO_STS[0x5000218c] = 0x%x\n", reg);

    reg = (uint32)ssi_read32(0x50002190);
    PS_PRINT_INFO("'EN_PALDO_W[0x50002190] = 0x%x\n", reg);

    reg = (uint32)ssi_read32(0x50002194);
    PS_PRINT_INFO("'EN_PALDO_B[0x50002194] = 0x%x\n", reg);

    reg = (uint32)ssi_read32(0x50000a60);
    PS_PRINT_INFO("WL_WAKEUP_EVENT_EN[0x50000a60] = 0x%x\n", reg);
    reg = (uint32)ssi_read32(0x50000a64);
    PS_PRINT_INFO("WL_WAKEUP_EVENT_STS[0x50000a64] = 0x%x\n", reg);
    reg = (uint32)ssi_read32(0x50000a70);
    PS_PRINT_INFO("WL_WAKEUP_INT_EN[0x50000a70] = 0x%x\n", reg);
    reg = (uint32)ssi_read32(0x50000a74);
    PS_PRINT_INFO("WL_WAKEUP_INT_STS[0x50000a74] = 0x%x\n", reg);
    reg = (uint32)ssi_read32(0x50000070);
    PS_PRINT_INFO("AON_PERP_CLKSEL_W[0x50000070] = 0x%x\n", reg);
    reg = (uint32)ssi_read32(0x50000074);
    PS_PRINT_INFO("AON_PERP_CLKSEL_B[0x50000074] = 0x%x\n", reg);
    reg = (uint32)ssi_read32(0x5000007C);
    PS_PRINT_INFO("CLK_STS[0x5000007C] = 0x%x\n", reg);

    reg = (uint32)ssi_read32(0x5000026c);
    PS_PRINT_INFO("H2D pinux PINMUX_CFG_RSV_MODE[0x5000026c] = 0x%x\n", reg);

    reg = (uint32)ssi_read32(0x50000294);
    PS_PRINT_INFO("sensorhub wakeup PINMUX_CFG_GNSSHUB2_MODE[0x50000294] = 0x%x\n", reg);
}

int ssi_check_wcpu_is_working(void)
{
    uint32 reg, mask;
    int32 ret = ssi_check_is_pilot();
    if( ret < 0 )
        return ret;

    /*pilot*/
    reg = (uint32)ssi_read32(0x50000f68);
    mask = (reg & 0x2)>>1;
    PS_PRINT_INFO("cpu pwr state=0x%8x, wcpu pwr state is %s\n", reg, g_ssi_hi1102a_cpu_pwr_st_str[mask]);
    g_st_ssi_cpu_infos.wcpu_info.cpu_state = mask;
    return mask;
}

int ssi_check_bcpu_is_working(void)
{
    uint32 reg, mask;
    int32 ret = ssi_check_is_pilot();
    if( ret < 0 )
        return ret;

    /*pilot*/
    reg = (uint32)ssi_read32(0x50000f68);
    mask = (reg & 0x8)>>3;
    PS_PRINT_INFO("cpu pwr state=0x%8x, bcpu pwr state is %s\n", reg, g_ssi_hi1102a_cpu_pwr_st_str[mask]);
    g_st_ssi_cpu_infos.bcpu_info.cpu_state = mask;
    return mask;
}


/*1102a pc采样属于comm_ctl 区域，不在AON电源域，
  当WBUS异常时访问可能会导致AON也无法正常读写，
  所以顺序调整到AON读完后*/
int ssi_read_device_arm_register(int trace_en)
{
    int32 ret;

#if 0
    uint32 reg = (uint32)ssi_read32(0x50003a88);

    if(0x3 == reg)
    {
        PS_PRINT_ERR("0x50003a88 is 0x3, wifi chip maybe enter dft mode , please check!\n");
    }
#endif

    /*read PC*/
    ret = ssi_check_wcpu_is_working();
    if(ret < 0)
        return ret;
    if(ret)
    {
        ssi_read_wpcu_pc_lr_sp(trace_en);
    }
    bfgx_print_subsys_state();
    ret = ssi_check_bcpu_is_working();
    if(ret < 0)
        return ret;
    if(ret)
    {
        ssi_read_bpcu_pc_lr_sp(trace_en);
    }

    return 0;
}

int32 ssi_tcxo_mux(uint32 flag)
{
    int ret;
    int32 is_pilot;

    if((0 == g_board_info.ssi_gpio_clk) || (0 == g_board_info.ssi_gpio_data))
    {
        PS_PRINT_ERR("reset aon, gpio ssi don't support\n");
        return -1;
    }

    ret = ssi_request_gpio(g_board_info.ssi_gpio_clk, g_board_info.ssi_gpio_data);
    if(ret)
    {
        PS_PRINT_ERR("ssi_force_reset_aon request failed:%d, data:%d, ret=%d\n",
                    g_board_info.ssi_gpio_clk, g_board_info.ssi_gpio_data, ret);
        return ret;
    }

    PS_PRINT_INFO("SSI start set\n");

    ssi_write16(0x8007, 0x1);

    is_pilot = ssi_check_is_pilot();
    if(1 != is_pilot)
    {
        PS_PRINT_INFO("not pilot chip, return\n");
        ssi_write16(0x8007, 0x0);
        ssi_free_gpio();
        return 0;
    }

    if (1 == flag)
    {
        ssi_write16(0x8009, 0x0);
        ssi_write16(0x8008, 0x60);
        ssi_write16(0x8009, 0x60);
        ssi_write32(0x50003338, 0x100);
        PS_PRINT_INFO("SSI set 0x50003338 to 0x100\n");
    }
    else
    {
        ssi_write16(0x8008, 0x0);
    }

    ssi_write16(0x8007, 0x0);

    PS_PRINT_INFO("SSI set OK\n");

    ssi_free_gpio();

    return 0;
}

int ssi_read_reg_info(ssi_reg_info* pst_reg_info,
                                void* buf , int32 buf_len,
                                oal_int32 is_logfile)
{
    int ret;
    int step = 4;
    oal_int32 is_atomic = 0;
    uint32 reg;
    uint16 reg16;
    uint32 ssi_address;
    uint32 realloc = 0;
    mm_segment_t fs;
    uint16 last_high_addr;
    int i,j,k,seg_size,seg_nums, left_size;

    struct timeval tv;
    struct rtc_time tm = {0};
    OS_KERNEL_FILE_STRU *fp = NULL;
    char filename[200] = {0};

    fs = get_fs();

    if(oal_in_interrupt() || oal_in_atomic() || irqs_disabled())
    {
        is_logfile = 0;
        is_atomic = 1;
    }

    if(is_logfile)
    {
        do_gettimeofday(&tv);
        rtc_time_to_tm(tv.tv_sec, &tm);
        snprintf(filename, sizeof(filename) - 1, "%s/gpio_ssi_%08x_%08x_%04d%02d%02d%02d%02d%02d.bin",
                                                 str_gpio_ssi_dump_path,
                                                 pst_reg_info->base_addr,
                                                 pst_reg_info->base_addr + pst_reg_info->len -1,
                                                 tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
                                                 tm.tm_hour, tm.tm_min, tm.tm_sec);
    }

    ret = ssi_check_device_isalive();
    if(ret)
    {
        PS_PRINT_INFO("gpio-ssi maybe dead before read 0x%x:%u\n", pst_reg_info->base_addr, pst_reg_info->len);
        return -OAL_EFAIL;
    }

    if(NULL == buf)
    {
        if(is_atomic)
        {
            buf = kmalloc(pst_reg_info->len, GFP_ATOMIC);
        }
        else
        {
            buf = OS_VMALLOC_GFP(pst_reg_info->len);
        }

        if(NULL == buf)
        {
            PS_PRINT_INFO("alloc mem failed before read 0x%x:%u\n", pst_reg_info->base_addr, pst_reg_info->len);
            return -OAL_ENOMEM;
        }
        buf_len = pst_reg_info->len;
        realloc = 1;
    }

    PS_PRINT_INFO("dump reg info 0x%x:%u, buf len:%u \n", pst_reg_info->base_addr, pst_reg_info->len, buf_len);

    if(is_logfile)
    {
        fp = filp_open(filename, O_RDWR | O_CREAT, 0644);
        if (IS_ERR(fp))
        {
            PS_PRINT_INFO("open file %s failed ret=%ld\n", filename, PTR_ERR(fp));
            is_logfile = 0;
        }
        else
        {
            PS_PRINT_INFO("open file %s succ\n", filename);
            fs = get_fs();
            set_fs(KERNEL_DS);
            vfs_llseek(fp, 0, SEEK_SET);
        }
    }

    last_high_addr = 0x0;
    ssi_clear_ahb_highaddr();

    if(SSI_RW_DWORD_MOD == pst_reg_info->rw_mod)
    {
        /*switch 32bits mode*/
        ssi_switch_ahb_mode(1);
    }
    else
    {
        ssi_switch_ahb_mode(0);
    }

retry:

    seg_nums = (pst_reg_info->len - 1 / buf_len) + 1;
    left_size = pst_reg_info->len;

    for(i = 0; i < seg_nums; i++)
    {
        seg_size = OAL_MIN(left_size, buf_len);
        for(j = 0; j < seg_size; j += step)
        {
            ssi_address = pst_reg_info->base_addr + i*buf_len + j;

            for(k = 0; k < 3; k++)
            {
                reg = 0x0;
                reg16 = 0x0;
                if(SSI_RW_DWORD_MOD == pst_reg_info->rw_mod)
                {
                    ret = ssi_read_value32(ssi_address, &reg, last_high_addr);
                }
                else
                {
                    ret = ssi_read_value16(ssi_address, &reg16, last_high_addr);
                    reg = reg16;
                }

                if(0 == ret)
                    break;
            }
            if(3 == k)
            {
                PS_PRINT_ERR("ssi read address 0x%x failed, retry %d times", ssi_address, k);
                goto fail_read;
            }
            last_high_addr = (ssi_address >> 16);
            oal_writel(reg, buf + j);
        }

        left_size -= seg_size;

        if(is_logfile)
        {
            ret = vfs_write(fp, buf, seg_size, &fp->f_pos);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35))
            vfs_fsync(fp, 0);
#else
            vfs_fsync(fp, fp->f_path.dentry, 0);
#endif
            if(ret != seg_size)
            {
                PS_PRINT_ERR("ssi print file failed, request %d, write %d actual\n", seg_size, ret);
                is_logfile = 0;
                set_fs(fs);
                filp_close(fp, NULL);
                goto retry;
            }
        }
        else
        {
#ifdef CONFIG_PRINTK
            /*print to kenrel msg*/
            print_hex_dump(KERN_DEBUG, "gpio-ssi: ", DUMP_PREFIX_OFFSET, 32, 4,
               buf, seg_size, false);
#endif
        }
    }

    if(is_logfile)
    {
        set_fs(fs);
        filp_close(fp, NULL);
    }

    if(realloc)
    {
        if(is_atomic)
        {
            kfree(buf);
        }
        else
        {
            OS_MEM_VFREE(buf);
        }
    }

    if(SSI_RW_DWORD_MOD == pst_reg_info->rw_mod)
    {
        /*switch 16bits mode*/
        ssi_switch_ahb_mode(0);
    }

    return 0;
fail_read:
    if(ssi_address != pst_reg_info->base_addr)
    {
        if(ssi_address > pst_reg_info->base_addr)
        {
#ifdef CONFIG_PRINTK
            /*print the read buf before errors*/
            print_hex_dump(KERN_DEBUG, "gpio-ssi: ", DUMP_PREFIX_OFFSET, 32, 4,
               buf, OAL_MIN(buf_len, ssi_address - pst_reg_info->base_addr), false);
#endif
        }
    }

    if(is_logfile)
    {
        set_fs(fs);
        filp_close(fp, NULL);
    }

    if(realloc)
    {
        if(is_atomic)
        {
            kfree(buf);
        }
        else
        {
            OS_MEM_VFREE(buf);
        }
    }

    if(SSI_RW_DWORD_MOD == pst_reg_info->rw_mod)
    {
        /*switch 16bits mode*/
        ssi_switch_ahb_mode(0);
    }
    return ret;
}

int ssi_read_reg_info_test(uint32 base_addr, uint32 len, uint32 is_logfile, uint32 rw_mode)
{
    int ret;
    ssi_reg_info reg_info;

    struct st_exception_info *pst_exception_data = NULL;
    get_exception_info_reference(&pst_exception_data);
    if (NULL == pst_exception_data)
    {
        PS_PRINT_ERR("get exception info reference is error\n");
        return -OAL_EBUSY;
    }
    if((!ssi_dfr_bypass) &&
            ( oal_work_is_busy(&pst_exception_data->wifi_excp_worker)
               || oal_work_is_busy(&pst_exception_data->wifi_excp_recovery_worker)
               || (PLAT_EXCEPTION_RESET_IDLE != atomic_read(&pst_exception_data->is_reseting_device))))
    {
        PS_PRINT_ERR("dfr is doing ,not do ssi read\n");
        return -OAL_EBUSY;
    }

    OAL_MEMZERO(&reg_info, sizeof(reg_info));

    reg_info.base_addr = base_addr;
    reg_info.len = len;
    reg_info.rw_mod = rw_mode;

    if((0 == g_board_info.ssi_gpio_clk) || (0 == g_board_info.ssi_gpio_data))
    {
        PS_PRINT_INFO("gpio ssi don't support, check dts\n");
        return -1;
    }

    /*get ssi lock*/
    if(ssi_try_lock())
    {
        PS_PRINT_INFO("ssi is locked, request return\n");
        return -OAL_EFAIL;
    }

    ret = ssi_request_gpio(g_board_info.ssi_gpio_clk, g_board_info.ssi_gpio_data);
    if(ret)
    {
        ssi_unlock();
        return ret;
    }

    ssi_read16(0x8009);
    ssi_read16(0x8008);

    ssi_write16(0x8007, 0x1);/*switch to ssi clk, wcpu hold*/
    PS_PRINT_INFO("switch ssi clk %s", (ssi_read16(0x8007) == 0x1) ? "ok":"failed");

    ret = ssi_read_device_arm_register(1);
    if(ret)
    {
        goto ssi_fail;
    }

    PS_PRINT_INFO("ssi is ok, glb_ctrl is ready\n");

    ret = ssi_check_device_isalive();
    if(ret)
    {
        goto ssi_fail;
    }

    ret = ssi_read_reg_info(&reg_info, NULL, 0, is_logfile);
    if(ret)
    {
        goto ssi_fail;
    }

    ssi_write16(0x8007, 0x0);/*switch from ssi clk, wcpu continue*/

    ssi_free_gpio();
    ssi_unlock();

    return 0;
ssi_fail:
    ssi_write16(0x8007, 0x0);/*switch from ssi clk, wcpu continue*/
    ssi_free_gpio();
    ssi_unlock();
    return ret;
}

int ssi_read_reg_info_arry(ssi_reg_info** pst_reg_info, oal_uint32 reg_nums, oal_int32 is_logfile)
{
    int ret;
    int i;

    if(OAL_UNLIKELY(NULL == pst_reg_info))
    {
        return -OAL_EFAIL;
    }

    for(i = 0; i < reg_nums; i++)
    {
        ret = ssi_read_reg_info(pst_reg_info[i], NULL, 0, is_logfile);
        if(ret)
        {
            return ret;
        }
    }

    return 0;
}

int ssi_force_reset_aon(void)
{
    int ret;
    /*request  ssi's gpio */

    if((0 == g_board_info.ssi_gpio_clk) || (0 == g_board_info.ssi_gpio_data))
    {
        PS_PRINT_INFO("reset aon, gpio ssi don't support\n");
        return -1;
    }

    ret = ssi_request_gpio(g_board_info.ssi_gpio_clk, g_board_info.ssi_gpio_data);
    if(ret)
    {
        PS_PRINT_INFO("ssi_force_reset_aon request failed:%d, data:%d, ret=%d\n",
                    g_board_info.ssi_gpio_clk, g_board_info.ssi_gpio_data, ret);
        return ret;
    }

    /*try to reset aon*/
    ssi_write16(0x8009, 0x60);
    ssi_write16(0x8008, 0x60);

    PS_PRINT_INFO("ssi_force_reset_aon");

    ssi_free_gpio();

    return 0;
}

int ssi_set_gpio_pins(int32 clk, int32 data)
{
    g_board_info.ssi_gpio_clk = clk;
    g_board_info.ssi_gpio_data = data;
    PS_PRINT_INFO("set ssi gpio clk:%d , gpio data:%d\n", clk, data);
    return 0;
}
EXPORT_SYMBOL_GPL(ssi_set_gpio_pins);

static void dsm_cpu_info_dump(void)
{
    int32 i;
    int32 ret = 0;
    int32 count = 0;
    char buf[DSM_CPU_INFO_SIZE];
    /*dsm cpu信息上报*/
    if(g_halt_det_cnt || (hi11xx_kernel_crash))
    {
        PS_PRINT_INFO("g_halt_det_cnt=%u hi11xx_kernel_crash=%d dsm_cpu_info_dump return\n", g_halt_det_cnt, hi11xx_kernel_crash);
        return;
    }

    /*没有检测到异常，上报记录的CPU信息*/
    oal_memset((void*)buf, 0, sizeof(buf));
    ret = snprintf(buf + count, DSM_CPU_INFO_SIZE - count, "wcpu_state=0x%x %s, bcpu_state=0x%x %s ",
       g_st_ssi_cpu_infos.wcpu_info.cpu_state, g_ssi_hi1102a_cpu_pwr_st_str[g_st_ssi_cpu_infos.wcpu_info.cpu_state&0x1],
       g_st_ssi_cpu_infos.bcpu_info.cpu_state, g_ssi_hi1102a_cpu_pwr_st_str[g_st_ssi_cpu_infos.bcpu_info.cpu_state&0x1]);
    if(ret <= 0)
    {
        goto done;
    }
    count += ret;

    for(i = 0; i < SSI_CPU_ARM_REG_DUMP_CNT; i++)
    {
        if(0 == g_st_ssi_cpu_infos.wcpu_info.reg_flag[i])
        {
            continue;
        }
        ret = snprintf(buf + count, DSM_CPU_INFO_SIZE - count, "wcpu[%d] pc:0x%x lr:0x%x sp:0x%x ", i,
              g_st_ssi_cpu_infos.wcpu_info.pc[i], g_st_ssi_cpu_infos.wcpu_info.lr[i], g_st_ssi_cpu_infos.wcpu_info.sp[i]);
        if(ret <= 0)
        {
            goto done;
        }
        count += ret;
    }

done:
#ifdef CONFIG_HUAWEI_DSM
    hw_1102a_dsm_client_notify(DSM_1102A_HALT, "%s\n", buf);
#else
    OAL_IO_PRINT("[non-dsm]%s\n", buf);
#endif
}

#define TCXO_32K_DET_VALUE  (10)
/*[+-x%]*/
#define TCXO_LIMIT_THRESHOLD (5)
int ssi_detect_tcxo_is_normal(void)
{
    /*tcxo detect 依赖tcxo时钟，
       如果在启动后tcxo 异常那么tcxo_32k_det_result 为旧值
       如果在启动后32k异常 那么sytem_tick为旧值*/
    int ret  = 0;
    char* tcxo_str = "";
    int tcxo_is_abnormal = 0;
    uint32 reg;
    uint32 tcxo_enable;
    uint32 tcxo_det_value_src, tcxo_det_value_target;
    oal_uint32 clock_32k = 0;
    uint32 sys_tick_old, sys_tick_new, pmu_sts_0;
    uint32 tcxo_det_res_old, tcxo_det_res_new;
    oal_uint64 clock_tcxo = 0;
    oal_uint64 div_clock = 0;
    oal_uint64 tcxo_limit_low, tcxo_limit_high, tcxo_tmp;
    oal_uint64 base_tcxo_clock = 38400000;

    declare_time_cost_stru(cost);

    pmu_sts_0 = (uint32)ssi_read32(0x50002114);
    tcxo_det_value_src = (uint32)ssi_read32(0x50000604);

    if(pmu_sts_0 & ( 1 << 3))
    {
        /*tcxo enable*/
        tcxo_enable = 1;
    }
    else
    {
        /*system maybe sleep, tcxo disable*/
        tcxo_enable = 0;
        PS_PRINT_ERR("tcxo gating normal\n");
    }

    tcxo_det_value_target = TCXO_32K_DET_VALUE;
    if(tcxo_det_value_src == tcxo_det_value_target)
    {
        /*刚做过detect,改变det_value，观测值是否改变*/
        tcxo_det_value_target = TCXO_32K_DET_VALUE + 2;
    }

    tcxo_tmp = div_u64(base_tcxo_clock, 100);
    tcxo_limit_low  = (tcxo_tmp*(100-TCXO_LIMIT_THRESHOLD));
    tcxo_limit_high = (tcxo_tmp*(100+TCXO_LIMIT_THRESHOLD));

    sys_tick_old = (uint32)ssi_read32(0x500000d0);
    tcxo_det_res_old = (uint32)ssi_read32(0x50000608);

    ssi_write32(0x500000c0, 0x2);/*清零w systick*/
    oal_get_time_cost_start(cost);

    if(tcxo_enable)
    {
        ssi_write32(0x50000604, tcxo_det_value_target);
        ssi_write32(0x50000600, 0x0);/*tcxo_det_en disable*/

        /*to tcxo*/
        ssi_write16(0x8007, 0x0);

        oal_udelay(150);

        /*to ssi*/
        ssi_write16(0x8007, 0x1);
        ssi_write32(0x50000600, 0x1);/*tcxo_det_en enable*/
        /*to tcxo*/
        ssi_write16(0x8007, 0x0);
        oal_udelay(31*tcxo_det_value_target*2);/*wait detect done*/

        /*to ssi*/
        ssi_write16(0x8007, 0x1);
    }
    else
    {
        oal_udelay(300);
    }

    ret = ssi_read_reg_info_arry(hi1102a_tcxo_detect_regs, sizeof(hi1102a_tcxo_detect_regs)/sizeof(ssi_reg_info*), ssi_is_logfile);
    if(ret)
    {
        return ret;
    }

    oal_udelay(1000);/*wait 32k count more*/

    oal_get_time_cost_end(cost);
    oal_calc_time_cost_sub(cost);

    sys_tick_new = (uint32)ssi_read32(0x500000d0);

    reg = (uint32)ssi_read32(0x50000600);
    tcxo_det_res_new = (uint32)ssi_read32(0x50000608);

    /*32k detect*/
    if(sys_tick_new == sys_tick_old)
    {
        PS_PRINT_ERR("32k sys_tick don't change after detect, 32k maybe abnormal, sys_tick=0x%x\n", sys_tick_new);
    }
    else
    {
        oal_uint64  us_to_s;
        us_to_s = time_cost_var_sub(cost);
        us_to_s += 5010;/*经验值,误差5010us*/
        clock_32k = (sys_tick_new*1000)/(oal_uint32)us_to_s;
        PS_PRINT_ERR("32k runtime:%llu us , sys_tick:%u\n", us_to_s, sys_tick_new);
        PS_PRINT_ERR("32k realclock real= %u Khz[base=32768]\n", clock_32k);
    }


    /*tcxo enabled*/
    if(tcxo_enable)
    {
        if(tcxo_det_res_new == tcxo_det_res_old)
        {
            /*tcxo 软件配置为打开此时应该有时钟*/
            PS_PRINT_ERR("tcxo don't change after detect, tcxo or 32k maybe abnormal, tcxo=0x%x\n", tcxo_det_res_new);
            if(0 == tcxo_det_res_new)
            {
                tcxo_is_abnormal = 1;
                tcxo_str = "non-tcxo";
            }
            else
            {
                /*这里可能是无效的探测，
                   要结合详细日志分析，此处DSM忽略改分支，不上报*/
                tcxo_is_abnormal = 0;
                tcxo_str = "tcxo-detect-invalid";
                DECLARE_DFT_TRACE_KEY_INFO("tcxo-detect-invalid", OAL_DFT_TRACE_FAIL);
            }
        }
        else
        {
            /*tcxo_det_res_new read from 16bit width register  <= 0xffff*/
            clock_tcxo = (oal_uint64)((tcxo_det_res_new*32768)/(tcxo_det_value_target));
            div_clock = clock_tcxo;
            div_clock = div_u64(div_clock, 1000000);
            if((clock_tcxo < tcxo_limit_low) || (clock_tcxo > tcxo_limit_high))
            {
                /*时钟误差超过阈值*/
                tcxo_is_abnormal = 2;
                tcxo_str = "tcxo clock-abnormal";
            }
            else
            {
                tcxo_is_abnormal = 0;
                tcxo_str = "tcxo normal";
            }
            PS_PRINT_ERR("%s real=%llu hz,%llu Mhz[base=%llu][limit:%llu~%llu]\n", tcxo_str, clock_tcxo, div_clock, base_tcxo_clock, tcxo_limit_low, tcxo_limit_high);
        }

        if(tcxo_is_abnormal)
        {
            /*Check Reg Config DCXO or TCXO*/
            reg = (uint32)ssi_read32(0x50000268);
            if(0x2 == reg)
            {
                /*dcxo mode, 共时钟方案*/
                PS_PRINT_ERR("dcxo mode, tcxo abnormal, please check the clk req io pin & tcxo device\n");
            }
            else if(0x8 == reg)
            {
                /*tcxo mode*/
                PS_PRINT_ERR("tcxo mode, tcxo abnormal, please check the xldo output & tcxo device\n");
            }
            else
            {
                PS_PRINT_ERR("undef mode, tcxo abnormal, cr_mode=0x%x\n", reg);
            }

            DECLARE_DFT_TRACE_KEY_INFO("tcxo_is_abnormal", OAL_DFT_TRACE_FAIL);
        }

        /*tcxo detect abnormal, dmd report*/
        if(0 == hi11xx_kernel_crash)
        {
            /*kernel is normal*/
            if(tcxo_is_abnormal)
            {
#ifdef CONFIG_HUAWEI_DSM
                hw_1102a_dsm_client_notify(DSM_1102A_TCXO_ERROR, "%s: tcxo=%llu[%llu][limit:%llu~%llu] 32k_clock=%lu,det_tick=0x%x value=0x%x\n",
                                    tcxo_str, clock_tcxo, base_tcxo_clock, tcxo_limit_low, tcxo_limit_high, clock_32k,
                                    tcxo_det_value_target, tcxo_det_res_new);
                g_halt_det_cnt++;
#endif
            }

        }
    }

    return ret;
}

int ssi_read_com_ctl_reg(void)
{
    int ret;
    uint32 reg, sys_stat;
    ret = ssi_check_device_isalive();
    if(ret)
    {
        PS_PRINT_ERR("ssi_read_com_ctl_reg return, ssi failed\n");
        return ret;
    }

    reg = (uint32)ssi_read32(0x50000f68);
    sys_stat = reg & ((1 << 0)|(1 << 3));
    if(0 == sys_stat)
    {
        PS_PRINT_ERR("system is off, bypass com_ctl dump\n");
        return 0;
    }

    ret = ssi_read_reg_info(&hi1102a_com_ctl_full, NULL, 0 , ssi_is_logfile);
    return ret;
}

/*Try to dump all reg,
  ssi used to debug, we should*/
int ssi_dump_device_regs(unsigned long long module_set)
{
    int ret;
    struct st_exception_info *pst_exception_data = NULL;
    get_exception_info_reference(&pst_exception_data);
    if (NULL == pst_exception_data)
    {
        PS_PRINT_ERR("get exception info reference is error\n");
        return -OAL_EBUSY;
    }
    if((!ssi_dfr_bypass) &&
            ( oal_work_is_busy(&pst_exception_data->wifi_excp_worker)
               || oal_work_is_busy(&pst_exception_data->wifi_excp_recovery_worker)
               || (PLAT_EXCEPTION_RESET_IDLE != atomic_read(&pst_exception_data->is_reseting_device))))
    {
        PS_PRINT_ERR("dfr is doing ,not do ssi read\n");
        return -OAL_EBUSY;
    }
    if((0 == g_board_info.ssi_gpio_clk) || (0 == g_board_info.ssi_gpio_data))
    {
        PS_PRINT_ERR("gpio ssi don't support, check dts\n");
        return -1;
    }

    /*get ssi lock*/
    if(ssi_try_lock())
    {
        PS_PRINT_INFO("ssi is locked, request return\n");
        return -OAL_EBUSY;
    }

    if(0 == gpio_get_value(g_board_info.power_on_enable))
    {
        PS_PRINT_INFO("110x power off,ssi return,power_on=%d\n", g_board_info.power_on_enable);
        ssi_unlock();
        return -OAL_ENODEV;
    }

    DECLARE_DFT_TRACE_KEY_INFO("ssi_dump_device_regs", OAL_DFT_TRACE_FAIL);

    ret = ssi_request_gpio(g_board_info.ssi_gpio_clk, g_board_info.ssi_gpio_data);
    if(ret)
    {
        ssi_unlock();
        return ret;
    }

    g_halt_det_cnt = 0;
    oal_memset(&g_st_ssi_cpu_infos, 0, sizeof(g_st_ssi_cpu_infos));

    ssi_read16(0x8009);
    ssi_read16(0x8008);

    ssi_write16(0x8007, 0x1);/*switch to ssi clk, wcpu hold*/

    PS_PRINT_INFO("module_set = 0x%llx\n", module_set);
    PS_PRINT_INFO("switch ssi clk %s", (ssi_read16(0x8007) == 0x1) ? "ok":"failed");

    ret = ssi_check_device_isalive();
    if(ret)
    {
        /*try to reset aon*/
        ssi_write16(0x8008, 0x60);
        ssi_write16(0x8009, 0x60);
        ssi_read16(0x8009);
        ssi_read16(0x8008);
        ssi_write16(0x8007, 0x1);
        if(ssi_check_device_isalive())
        {
            PS_PRINT_INFO("after reset aon, ssi still can't work\n");
            goto ssi_fail;
        }
        else
        {
            PS_PRINT_INFO("after reset aon, ssi ok, dump pmu_ctl reg\n");
            ssi_detect_pmu_protect_status();
            ssi_read_reg_info(&hi1102a_pmu_cmu_ctrl_full, NULL, 0 , ssi_is_logfile);
            goto ssi_fail;
        }
    }

    ssi_detect_pmu_protect_status();
    ssi_read_low_power_state();

    ret = ssi_check_device_isalive();
    if(ret)
    {
        goto ssi_fail;
    }

    if(module_set & SSI_MODULE_MASK_AON)
    {
        ret = ssi_read_reg_info_arry(hi1102a_aon_reg_full, sizeof(hi1102a_aon_reg_full)/sizeof(ssi_reg_info*), ssi_is_logfile);
        if(ret)
        {
            goto ssi_fail;
        }
    }

    /*detect tcxo clock is normal, trigger*/
    if(module_set & (SSI_MODULE_MASK_AON))
    {
        ret = ssi_detect_tcxo_is_normal();
        if(ret)
        {
            PS_PRINT_INFO("tcxo detect failed, continue dump\n");
        }
    }

    /*com_ctl不在AON,假如失败可能是总线异常*/
    if(module_set & SSI_MODULE_MASK_ARM_REG)
    {
        ret = ssi_read_device_arm_register(0);
        if(ret)
        {
            goto ssi_fail;
        }
        ret = ssi_read_device_arm_register(1);
        if(ret)
        {
            goto ssi_fail;
        }
    }

    /*com_ctl,当wcpu on or*/
    ret = ssi_read_com_ctl_reg();
    if(ret)
    {
        PS_PRINT_ERR("ssi_read_com_ctl_reg return, ssi failed\n");
        goto ssi_fail;
    }

    /*Debug*/
#if 0
    if(module_set & SSI_MODULE_MASK_WCPU_KEY_DTCM)
    {
        if(ssi_check_wcpu_is_working())
        {
            ret = ssi_read_reg_info(&hi1103_w_key_mem, NULL, 0 , ssi_is_logfile);
            if(ret)
            {
                PS_PRINT_INFO("wcpu key mem read failed, continue try aon\n");
            }
        }
        else
        {
            PS_PRINT_INFO("wctrl can't dump, wcpu down\n");
        }
    }
#endif


    if(module_set & SSI_MODULE_MASK_WCTRL)
    {
        if(ssi_check_wcpu_is_working())
        {
            ret = ssi_read_reg_info(&hi1102a_w_ctrl_full, NULL, 0 , ssi_is_logfile);
            if(ret)
            {
                goto ssi_fail;
            }
        }
        else
        {
            PS_PRINT_INFO("wctrl can't dump, wcpu down\n");
        }
    }

    if(module_set & SSI_MODULE_MASK_SDIO)
    {
        if(ssi_check_wcpu_is_working())
        {
            ret = ssi_read_reg_info(&hi1102a_sdio_ctrl_full, NULL, 0, ssi_is_logfile);
            if(ret)
                goto ssi_fail;
        }
        else
        {
            PS_PRINT_INFO("sdio can't dump, wcpu down\n");
        }
    }

    if(module_set & SSI_MODULE_MASK_BCTRL)
    {
        if(ssi_check_bcpu_is_working())
        {
            PS_PRINT_INFO("g_lIntDisCnt 0x4b005abc = 0x%x\n", (uint32)ssi_read32(0x4b005abc));
            ret = ssi_read_reg_info(&hi1102a_b_ctrl_full, NULL, 0 , ssi_is_logfile);
            if(ret)
            {
                goto ssi_fail;
            }
        }
    }

    ssi_write16(0x8007, 0x0);/*switch from ssi clk, wcpu continue*/

    ssi_free_gpio();
    dsm_cpu_info_dump();
    ssi_unlock();

    DECLARE_DFT_TRACE_KEY_INFO("succ_ssi_dump_device_regs", OAL_DFT_TRACE_FAIL);

    return 0;
ssi_fail:
    ssi_write16(0x8007, 0x0);/*switch from ssi clk, wcpu continue*/

    ssi_free_gpio();
    dsm_cpu_info_dump();
    ssi_unlock();
    DECLARE_DFT_TRACE_KEY_INFO("failed_ssi_dump_device_regs", OAL_DFT_TRACE_FAIL);
    return ret;
}
#endif

/*********************************************************************/
/********************   SSI调试代码end    ****************************/
/*********************************************************************/

#ifdef _PRE_CONFIG_USE_DTS
static struct of_device_id hi110x_board_match_table[] = {
    {
        .compatible = DTS_COMP_HI110X_BOARD_NAME,
        .data = NULL,
    },
    {
        .compatible = DTS_COMP_HISI_HI110X_BOARD_NAME,
        .data = NULL,
    },
    { },
};
#endif

STATIC struct platform_driver hi110x_board_driver = {
        .probe      = hi110x_board_probe,
        .remove     = hi110x_board_remove,
        .suspend    = hi110x_board_suspend,
        .resume     = hi110x_board_resume,
        .driver     = {
            .name   = "hi110x_board",
            .owner  = THIS_MODULE,
#ifdef _PRE_CONFIG_USE_DTS
            .of_match_table = hi110x_board_match_table,
#endif
        },
};

int32 hi110x_board_init(void)
{
    int32 ret = BOARD_FAIL;

    board_probe_ret = BOARD_FAIL;
    init_completion(&board_driver_complete);

    ret = platform_driver_register(&hi110x_board_driver);
    if (ret)
    {
        PS_PRINT_ERR("Unable to register hisi connectivity board driver.\n");
        return ret;
    }

    if(wait_for_completion_timeout(&board_driver_complete, 10*HZ))
    {
        /*completed*/
        if(BOARD_SUCC != board_probe_ret)
        {
            PS_PRINT_ERR("hi110x_board probe failed=%d\n", board_probe_ret);
            return board_probe_ret;
        }
    }
    else
    {
        /*timeout*/
        PS_PRINT_ERR("hi110x_board probe timeout\n");
        return BOARD_FAIL;
    }

    PS_PRINT_INFO("hi110x_board probe succ\n");

    return ret;
}

void hi110x_board_exit(void)
{
    platform_driver_unregister(&hi110x_board_driver);
}


oal_uint8 g_bus_type = HCC_BUS_SDIO;
oal_int32 g_wifi_plat_dev_probe_state;

#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_HOST)
extern int create_hwconn_proc_file(void);
#endif
static int hisi_wifi_plat_dev_drv_probe(struct platform_device *pdev)
{
   int ret = 0;
    /*TBD:bus type should be defined in dts and read during probe*/
   if(HCC_BUS_SDIO == g_bus_type)
   {
        ret = oal_wifi_platform_load_sdio();
        if(ret)
        {
            printk(KERN_ERR "[HW_CONN] oal_wifi_platform_load_sdio failed.\n");
            g_wifi_plat_dev_probe_state = -OAL_FAIL;
            return ret;
        }
   }

#ifdef CONFIG_HWCONNECTIVITY
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_HOST)
    ret = create_hwconn_proc_file();
    if (ret)
    {
        printk(KERN_ERR "[HW_CONN] create proc file failed.\n");
        g_wifi_plat_dev_probe_state = -OAL_FAIL;
        return ret;
    }
#endif
#endif
  return ret;
}

static int hisi_wifi_plat_dev_drv_remove(struct platform_device *pdev)
{
    printk(KERN_ERR "[HW_CONN] hisi_wifi_plat_dev_drv_remove.\n");
    return  OAL_SUCC;
}


#ifdef _PRE_CONFIG_USE_DTS
static const struct of_device_id hisi_wifi_match_table[] = {
    {
        .compatible = DTS_NODE_HI110X_WIFI,   // compatible must match with which defined in dts
        .data = NULL,
    },
    {},
};
#endif

static struct platform_driver hisi_wifi_platform_dev_driver = {
    .probe          = hisi_wifi_plat_dev_drv_probe,
    .remove         = hisi_wifi_plat_dev_drv_remove,
    .suspend        = NULL,
    .shutdown       = NULL,
    .driver = {
        .name = DTS_NODE_HI110X_WIFI,
        .owner = THIS_MODULE,
#ifdef _PRE_CONFIG_USE_DTS
        .of_match_table = hisi_wifi_match_table, // dts required code
#endif
    },
};

int32 hisi_wifi_platform_register_drv(void)
{
    int32 ret = BOARD_FAIL;
    PS_PRINT_FUNCTION_NAME;

    g_wifi_plat_dev_probe_state = OAL_SUCC;

    ret = platform_driver_register(&hisi_wifi_platform_dev_driver);
    if (ret)
    {
        PS_PRINT_ERR("Unable to register hisi wifi driver.\n");
    }
    /*platform_driver_register return always true*/
    return g_wifi_plat_dev_probe_state;
}

void hisi_wifi_platform_unregister_drv(void)
{
    PS_PRINT_FUNCTION_NAME;

    platform_driver_unregister(&hisi_wifi_platform_dev_driver);

    return;
}


