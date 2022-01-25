
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/module.h>
#include <linux/io.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/clk.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/pinctrl/pinctrl.h>
#include <linux/pinctrl/consumer.h>
#include <linux/printk.h>
#include <huawei_platform/connectivity/huawei_gps.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/stat.h>
#include <linux/rfkill.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/workqueue.h>
#include <linux/uaccess.h>
#include <linux/poll.h>
#include <linux/miscdevice.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/platform_device.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/list.h>
#include <linux/version.h>
#include <linux/workqueue.h>
#include <linux/unistd.h>
#include <linux/bug.h>
#include <linux/mutex.h>
#include <linux/wakelock.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/of_device.h>

#ifdef CONFIG_HWCONNECTIVITY
#include <huawei_platform/connectivity/hw_connectivity.h>
#endif

extern int get_gps_ic_type(void);

#define DTS_COMP_GPS_POWER_NAME "huawei,gps_power"
#define HOST_WAKE_MODULE_NAME "gps_geofence_wake"
#define GPS_REF_CLK_FREQ_19M   (19200000L)

#define GPS_STANDBY         0
#define GPS_ACTIVE          1

#define GPS_IC_TYPE_4752 4752
#define GPS_IC_TYPE_47531 47531
#define GPS_IC_TYPE_4774 4774

#define GPSINFO(fmt, args...)   pr_info("[DRV_GPS, I %s:%d]" fmt "\n",  __func__,  __LINE__, ## args)
#define GPSDBG(fmt, args...)    pr_debug("[DRV_GPS, D %s:%d]" fmt "\n",  __func__,  __LINE__, ## args)
#define GPSERR(fmt, args...)    pr_err("[DRV_GPS, E %s:%d]" fmt "\n",  __func__, __LINE__, ## args)

typedef struct gps_bcm_info {
    struct gpio gpioid_en;
    struct clk *clk;
    struct clk *refclk;
    struct pinctrl *pctrl;
    struct pinctrl_state *pins_normal;
    struct pinctrl_state *pins_idle;
    struct gpio gpioid_hostwake;
    struct gpio mcu_req;
    struct gpio mcu_req_rsp;
    bool is_hostwake;
} GPS_BCM_INFO;

struct gps_geofence_wake {
    /* irq from gpio_to_irq()*/
    int irq;
    /* HOST_WAKE_GPIO*/
    int host_req_pin;
    /* misc driver structure*/
    struct miscdevice misc;
    /* wake_lock*/
    struct wake_lock wake_lock;
};

enum procs {
    GPS_PROC_MCUREQ,
    GPS_PROC_MCUREQ_RSP,
    GPS_PROC_EN
};

static struct clk *gps_ref_clk;
static GPS_BCM_INFO *g_gps_bcm;
static struct proc_dir_entry *gps_dir;
static struct gps_geofence_wake g_geofence_wake;

char *Gps_DtsNodeName[] = {
    "huawei,mcu_req",
    "huawei,mcu_req_rsp",
    "huawei,gps_en"
};

char *Gps_DeviceNodeName[] = {
    "mcu_req",
    "mcu_req_rsp",
    "nstandby"
};

static int gps_geofence_wake_open(struct inode *inode, struct file *filp)
{
    GPSINFO("be called.");
    return 0;
}

static int gps_geofence_wake_release(struct inode *inode, struct file *filp)
{
    GPSINFO("be called.");
    return 0;
}

static long gps_geofence_wake_ioctl(struct file *filp, unsigned int cmd,
                   unsigned long arg)
{
    GPSINFO("be called.");
    return (long)0;
}

static const struct file_operations gps_geofence_wake_fops = {
    .owner = THIS_MODULE,
    .open = gps_geofence_wake_open,
    .release = gps_geofence_wake_release,
    .unlocked_ioctl = gps_geofence_wake_ioctl
};

/*set/reset wake lock by HOST_WAKE level
 \param gpio the value of HOST_WAKE_GPIO*/
static void gps_geofence_wake_lock(int gpio)
{
    struct gps_geofence_wake *ac_data = &g_geofence_wake;

    if (gpio) {
        /*wake_lock(&ac_data->wake_lock);*/
        wake_lock_timeout(&ac_data->wake_lock, 5 * HZ);
    } else {
        wake_unlock(&ac_data->wake_lock);
    }
}

static irqreturn_t gps_host_wake_isr(int irq, void *dev)
{
    struct gps_geofence_wake *ac_data = &g_geofence_wake;
    int gps_host_wake = ac_data->host_req_pin;
    char gpio_value = 0x00;

    gpio_value = gpio_get_value(gps_host_wake);

    /*wake_lock*/
    gps_geofence_wake_lock(gpio_value);

    return IRQ_HANDLED;
}

/*initialize GPIO and IRQ
 \param gpio the GPIO of HOST_WAKE
 \return if SUCCESS, return the id of IRQ, if FAIL, return -EIO*/
static int gps_gpio_irq_init(int gpio)
{
    int ret = 0;
    int irq = 0;

    GPSDBG("be called.");
    /*1. Set GPIO*/
    if ((gpio_request(gpio, "gps_host_wake"))) {
        GPSERR
            ("[gps]Can't request HOST_REQ GPIO %d.It may be already registered in init.xyz.3rdparty.rc/init.xyz.rc\n",
             gpio);
        return -EIO;
    }
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 13, 0))
    gpiod_export(gpio_to_desc(gpio), false);
#else
    gpio_export(gpio, false);
#endif
    gpio_direction_input(gpio);

    /*2. Set IRQ*/
    irq = gpio_to_irq(gpio);
    if (irq < 0) {
        GPSERR("Could not get HOST_WAKE_GPIO = %d!, err = %d.",
               gpio, irq);
        gpio_free(gpio);
        return -EIO;
    }

    ret =
        request_irq(irq, gps_host_wake_isr,
            IRQF_TRIGGER_RISING | IRQF_NO_SUSPEND |
            IRQF_TRIGGER_FALLING | IRQF_ONESHOT, "gps_host_wake",
            NULL);
    if (ret) {
        GPSERR("[gps]Request_host wake irq failed.");
        gpio_free(gpio);
        return -EIO;
    }

    ret = irq_set_irq_wake(irq, 1);

    if (ret) {
        GPSERR("Set_irq_wake failed.");
        gpio_free(gpio);
        free_irq(irq, NULL);
        return -EIO;
    }

    return irq;
}

static ssize_t gps_write_proc_nstandby(struct file *filp,
                       const char __user *buffer, size_t len,
                       loff_t *off)
{
    char gps_nstandby = '0';
    GPSINFO("gps_write_proc_nstandby.");

    if(buffer == NULL || off == NULL )
    {
        pr_err("[GPS]NULL Pointer to platform device\n");
        return -EINVAL;
    }
    if ((len < 1) || (NULL == g_gps_bcm)) {
        GPSERR("gps_write_proc_nstandby g_gps_bcm is NULL or read length = 0.");
        return -EINVAL;
    }

    if (copy_from_user(&gps_nstandby, buffer, sizeof(gps_nstandby))) {
        GPSERR("gps_write_proc_nstandby copy_from_user failed!");
        return -EFAULT;
    }

    if (gps_nstandby == '0') {
        GPSINFO("gps nstandby.");
        gpio_set_value(g_gps_bcm->gpioid_en.gpio, GPS_STANDBY);
    } else if (gps_nstandby == '1') {
        GPSINFO("[GPS] gps active.");
        gpio_set_value(g_gps_bcm->gpioid_en.gpio, GPS_ACTIVE);
    } else {
        GPSERR(" gps nstandby write error code[%d].",
               gps_nstandby);
    }

    return len;
}

static ssize_t gps_read_proc_nstandby(struct file *filp,
                      char __user *buffer, size_t len,
                      loff_t *off)
{
    int gps_nstandby = 0;
    char tmp[2];
    if(buffer == NULL  || off == NULL )
    {
        pr_err("[GPS]NULL Pointer to platform device\n");
        return -EINVAL;
    }
    memset(tmp, 0, sizeof(tmp));
    GPSINFO(" gps_read_proc_nstandby.");
    if (len < 1 || NULL == g_gps_bcm) {
        GPSERR(" gps_read_proc_nstandby g_gps_bcm is NULL or read length = 0.");
        return -EINVAL;
    }
    len = 1;
    GPSINFO(" gps_read_proc_nstandby off = %d.",
           (unsigned int)*off);
    if ((unsigned int)*off > 0) {
        return 0;
    }
    gps_nstandby = gpio_get_value(g_gps_bcm->gpioid_en.gpio);
    sprintf(tmp, "%d", gps_nstandby);
    GPSINFO(" gps nstandby status[%s]", tmp);

    if (copy_to_user((void __user *)buffer, (void *)tmp, len)) {
        GPSERR("[GPS] gps_read_proc_nstandby copy_to_user failed!");
        return -EFAULT;
    }
    *off += len;
    return len;
}

static const struct file_operations gps_proc_fops_nstandby = {
    .owner = THIS_MODULE,
    .write = gps_write_proc_nstandby,
    .read = gps_read_proc_nstandby,
};

/*  For write   */
static ssize_t gps_write_proc_mcureq(struct file *filp,
                     const char __user *buffer, size_t len,
                     loff_t *off)
{
    char gps_nstandby = '0';

    GPSINFO(" gps_write_proc_nstandby");
    if(buffer == NULL  || off == NULL )
    {
        pr_err("[GPS]NULL Pointer to platform device\n");
        return -EINVAL;
    }
    if ((len < 1) || (NULL == g_gps_bcm)) {
        GPSERR(" gps_write_proc_mcureq g_gps_bcm is NULL or read length = 0.");
        return -EINVAL;
    }

    if (copy_from_user(&gps_nstandby, buffer, sizeof(gps_nstandby))) {
        GPSERR(" gps_write_proc_mcureq copy_from_user failed!");
        return -EFAULT;
    }

    if (gps_nstandby == '0') {
        GPSINFO(" gps nstandby.");
        gpio_set_value(g_gps_bcm->mcu_req.gpio, GPS_STANDBY);
    } else if (gps_nstandby == '1') {
        GPSINFO(" gps active.");
        gpio_set_value(g_gps_bcm->mcu_req.gpio, GPS_ACTIVE);
    } else {
        GPSERR(" gps nstandby write error code[%d].",
               gps_nstandby);
    }

    return len;
}

static ssize_t gps_read_proc_mcureq(struct file *filp,
                    char __user *buffer, size_t len,
                    loff_t *off)
{
    int gps_nstandby = 0;
    char tmp[2];
    if(buffer == NULL  || off == NULL )
    {
        pr_err("[GPS]NULL Pointer to platform device\n");
        return -EINVAL;
    }
    memset(tmp, 0, sizeof(tmp));
    GPSINFO("gps_read_proc_nstandby.");
    if (len < 1 || NULL == g_gps_bcm) {
        GPSERR(" gps_read_proc_mcureq g_gps_bcm is NULL or read length = 0.");
        return -EINVAL;
    }
    len = 1;
    GPSINFO(" gps_read_proc_mcureq off = %d.",
           (unsigned int)*off);
    if ((unsigned int)*off > 0) {
        return 0;
    }
    gps_nstandby = gpio_get_value(g_gps_bcm->mcu_req.gpio);
    sprintf(tmp, "%d", gps_nstandby);
    GPSINFO(" gps mcu_req status[%s]", tmp);

    if (copy_to_user((void __user *)buffer, (void*)tmp, len)) {
        GPSERR(" mcu_req copy_to_user failed!");
        return -EFAULT;
    }
    *off += len;
    return len;
}

static const struct file_operations gps_proc_fops_mcureq = {
    .owner = THIS_MODULE,
    .write = gps_write_proc_mcureq,
    .read = gps_read_proc_mcureq,
};

/* For read */
static ssize_t gps_read_proc_mcureq_rsp(struct file *filp,
                    char __user *buffer, size_t len,
                    loff_t *off)
{
    int gps_reqrsp_data = 0;
    char tmp[2] = {0};

    if(buffer == NULL  || off == NULL )
    {
        pr_err("[GPS]NULL Pointer to platform device\n");
        return -EINVAL;
    }

    if (1 > len || NULL == g_gps_bcm) {
        GPSERR(" gps_read_proc_mcureq  g_gps_bcm is NULL or read length = 0.");
        return -EINVAL;
    }

    len = 1;
    GPSINFO(" gps_read_proc_nstandby off = %d",
           (unsigned int)*off);
    if ((unsigned int)*off > 0) {
        return 0;
    }
    gps_reqrsp_data = gpio_get_value(g_gps_bcm->mcu_req_rsp.gpio);
    if (0 == gps_reqrsp_data)
    {
        tmp[0] = '0';
    }
    else if (1 == gps_reqrsp_data)
    {
        tmp[0] = '1';
    }

    GPSINFO(" gps gps_reqrsp_data status[%s]", tmp);

    if (copy_to_user((void __user *)buffer, (void*)tmp, len)) {
        GPSERR(" gps_read_proc_nstandby copy_to_user failed!");
        return -EFAULT;
    }
    *off += len;
    return len;
}

/*  For write   */
static ssize_t gps_write_proc_mcureq_rsp(struct file *filp,
                     const char __user *buffer, size_t len,
                     loff_t *off)
{
    char gps_nstandby = '0';

    GPSINFO("[GPS] gps_write_proc_nstandby");
    if(buffer == NULL  || off == NULL )
    {
        pr_err("[GPS]NULL Pointer to platform device\n");
        return -EINVAL;
    }

    if ((len < 1) || (NULL == g_gps_bcm)) {
        GPSERR(" gps_write_proc_mcureq g_gps_bcm is NULL or read length = 0.");
        return -EINVAL;
    }

    if (copy_from_user(&gps_nstandby, buffer, sizeof(gps_nstandby))) {
        GPSERR(" gps_write_proc_mcureq copy_from_user failed!");
        return -EFAULT;
    }

    if (gps_nstandby == '0') {
        GPSINFO(" gps nstandby.");
        gpio_set_value(g_gps_bcm->mcu_req_rsp.gpio, GPS_STANDBY);
    } else if (gps_nstandby == '1') {
        GPSINFO("[GPS] gps active.");
        gpio_set_value(g_gps_bcm->mcu_req_rsp.gpio, GPS_ACTIVE);
    } else {
        GPSERR(" gps nstandby write error code[%d].",
               gps_nstandby);
    }

    return len;
}

static const struct file_operations gps_proc_fops_mcureq_rsp = {
    .owner = THIS_MODULE,
    .read = gps_read_proc_mcureq_rsp,
    .write = gps_write_proc_mcureq_rsp,
};

int gps_bcm4774_node_init(struct device_node *np, struct gpio *Pgpio, int node,
              struct file_operations *gps_proc_fops)
{
    int ret = 0;
    struct proc_dir_entry* file;

    if(np == NULL  || Pgpio == NULL  || gps_proc_fops == NULL )
    {
        pr_err("[GPS]NULL Pointer to platform device\n");
        return -EINVAL;
    }

    Pgpio->gpio = of_get_named_gpio(np, Gps_DtsNodeName[node], 0);
    if (!gpio_is_valid(Pgpio->gpio)) {
        GPSERR(" of_get_named_gpio  %s failed.",
               Gps_DtsNodeName[node]);
        ret = -1;
        return ret;
    }

    ret = gpio_request(Pgpio->gpio, Gps_DeviceNodeName[node]);
    if (ret) {
        GPSERR(" gpio_requestt [%d]%s  failed, ret:%d",
               Pgpio->gpio, Gps_DeviceNodeName[node], ret);
        return ret;
    }
/*
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 13, 0))
    ret = gpiod_export(gpio_to_desc(Pgpio->gpio), false);
#else
    ret = gpio_export(Pgpio->gpio, false);
#endif
    if (ret) {
        GPSERR(" gpio_export %d failed, ret:%d",
               Pgpio->gpio, ret);
        return ret;
    }
*/
    /* For req */
    ret = gpio_direction_input(Pgpio->gpio);
    if (ret) {
        GPSERR(" gpio_direction_input %d failed, ret:0x%X",
               Pgpio->gpio, ret);
        return ret;
    }
    GPSINFO(" finish gpio_direction_input ");

    ret = gpio_direction_output(Pgpio->gpio, 0);
    if (ret) {
        GPSERR(" gpio_direction_output %d failed, ret:0x%X",
               Pgpio->gpio, ret);
        return ret;
    }

    file =
            proc_create(Gps_DeviceNodeName[node],
            S_IRUGO | S_IWUSR | S_IWGRP | S_IFREG, gps_dir,
            gps_proc_fops);
    if (!file) {
        GPSERR(" gps create proc file [%s] failed. ret = %d",
               Gps_DeviceNodeName[node], ret);
        ret = -ENOMEM;
        return ret;
    }

    return 0;
}

static int k3_gps_bcm_probe(struct platform_device *pdev)
{
    GPS_BCM_INFO *gps_bcm = NULL;
    struct device *gps_power_dev = NULL;
    struct device_node *np = NULL;
    int ret = 0;
    int irq = 0;
    struct gps_geofence_wake *ac_data = NULL;
    if(pdev == NULL )
    {
        pr_err("[GPS]NULL Pointer to platform device\n");
        return -EINVAL;
    }

    gps_power_dev = &pdev->dev;
    np = gps_power_dev->of_node;
    ac_data = &g_geofence_wake;

    GPSINFO(" start find gps_power and ic type is 4774");
    gps_bcm = kzalloc(sizeof(GPS_BCM_INFO), GFP_KERNEL);
    if (!gps_bcm) {
        GPSERR("[GPS] Alloc memory failed");
        return -ENOMEM;
    }

    gps_dir = proc_mkdir("gps", NULL);
    if (!gps_dir) {
        GPSERR("[GPS] proc dir create failed");
        ret = -ENOMEM;
        goto err_free_gps;
    }

    ret =
        gps_bcm4774_node_init(np, &gps_bcm->gpioid_en, GPS_PROC_EN,
                  (struct file_operations*)&gps_proc_fops_nstandby);
    if (0 != ret) {
        GPSERR(" gps_bcm4774_node_init  gps_bcm  failed.");
        ret = -1;
        goto err_free_nstandby;
    }

    gps_bcm->gpioid_hostwake.gpio =
        of_get_named_gpio(np, "huawei,gps_hostwake", 0);
    if (!gpio_is_valid(gps_bcm->gpioid_hostwake.gpio)) {
        ret = -1;
        GPSERR(" get huawei,gps_hostwake failed.");
        goto err_free_gps_en;
    }
    /*1. Init GPIO and IRQ for HOST_WAKE*/
    GPSINFO("gps_bcm->gpioid_hostwake.gpio=%d",
           gps_bcm->gpioid_hostwake.gpio);

    /*2. Register Driver*/
    memset(ac_data, 0, sizeof(struct gps_geofence_wake));

    /*2.1 Misc device setup*/
    ac_data->misc.minor = MISC_DYNAMIC_MINOR;
    ac_data->misc.name = HOST_WAKE_MODULE_NAME;
    ac_data->misc.fops = &gps_geofence_wake_fops;

    /*2.2 Information that be used later*/
    ac_data->irq = irq;
    ac_data->host_req_pin = gps_bcm->gpioid_hostwake.gpio;

    GPSINFO("misc register, name %s, irq %d, host req pin num %d",
           ac_data->misc.name, irq, ac_data->host_req_pin);
    /*2.3 Register misc driver*/
    ret = misc_register(&ac_data->misc);
    if (0 != ret) {
        printk
            (" cannot register gps geofence wake miscdev on minor=%d (%d)",
             MISC_DYNAMIC_MINOR, ret);
        goto err_free_host_wake;
    }
    /*3. Init wake_lock*/
    wake_lock_init(&ac_data->wake_lock, WAKE_LOCK_SUSPEND,
               "gps_geofence_wakelock");
    GPSINFO("[gps]wake_lock_init done");
    irq = gps_gpio_irq_init(gps_bcm->gpioid_hostwake.gpio);
    if (irq < 0) {
        GPSINFO("[gps]hostwake irq error");
        goto err_free_misc_register;
    }

    /* Set 32KC clock */
    gps_bcm->clk = of_clk_get_by_name(np, "gps_32k");
    if (IS_ERR(gps_bcm->clk)) {
        GPSERR("clk_32k get failed!");
        ret = -1;
        goto err_free_misc_register;
    }
    ret = clk_prepare_enable(gps_bcm->clk);
    if (ret) {
        GPSERR(" clk_32k enable is failed");
        goto err_free_clk;
    }
    GPSINFO(" clk_32k is finished");

    ret =
        gps_bcm4774_node_init(np, &gps_bcm->mcu_req, GPS_PROC_MCUREQ,
                  (struct file_operations*)&gps_proc_fops_mcureq);
    if (0 != ret) {
        GPSERR(" gps_bcm4774_node_init  mcu_req  failed");
        goto err_free_mcureq;
    }

    ret =
        gps_bcm4774_node_init(np, &gps_bcm->mcu_req_rsp,
                  GPS_PROC_MCUREQ_RSP,
                  (struct file_operations*)&gps_proc_fops_mcureq_rsp);
    if (0 != ret) {
        GPSERR("gps_bcm4774_node_init  mcu_req_rsp  failed");
        goto err_free_mcureq_rsp;
    }

    gps_bcm->pctrl = devm_pinctrl_get(gps_power_dev);
    if (IS_ERR(gps_bcm->pctrl)) {
        GPSERR(" pinctrl get error!");
        ret = -1;
        goto err_free_clk;
    }

    gps_bcm->pins_normal = pinctrl_lookup_state(gps_bcm->pctrl, "default");
    if (IS_ERR(gps_bcm->pins_normal)) {
        GPSERR(" gps_bcm->pins_normal lookup error!");
        ret = -1;
        goto err_free_pinctrl;
    }

    gps_bcm->pins_idle = pinctrl_lookup_state(gps_bcm->pctrl, "idle");
    if (IS_ERR(gps_bcm->pins_idle)) {
        GPSERR(" gps_bcm->pins_idle lookup error!");
        ret = -1;
        goto err_free_pinctrl;
    }

    ret = pinctrl_select_state(gps_bcm->pctrl, gps_bcm->pins_normal);
    if (ret) {
        GPSERR("[GPS] pinctrl_select_state error!");
        goto err_free_pinctrl;
    }
    GPSINFO("[GPS] pinctrl is finish");

    gps_bcm->refclk = of_clk_get_by_name(np, "clk_gps");
    if (IS_ERR(gps_bcm->refclk)) {
        GPSERR("@@@ref_clk get failed!");
        ret = -1;
        goto err_free_pinctrl;
    }

    ret = clk_set_rate(gps_bcm->refclk, GPS_REF_CLK_FREQ_19M);
    if (ret < 0) {
        GPSERR(" clk_set_rate HI3635_CLK_RATE failed");
        goto err_free_refclk;
    }

    gps_ref_clk = gps_bcm->refclk;

    GPSINFO(" ref clk is finished!");
    platform_set_drvdata(pdev, gps_bcm);
    g_gps_bcm = gps_bcm;

    return 0;

err_free_refclk:
    clk_put(gps_bcm->refclk);
err_free_pinctrl:
    pinctrl_put(gps_bcm->pctrl);

err_free_mcureq:
err_free_mcureq_rsp:
err_free_clk:
    clk_put(gps_bcm->clk);
err_free_misc_register:
    misc_deregister(&ac_data->misc);
    wake_lock_destroy(&ac_data->wake_lock);
    GPSERR(" misc_deregister!");
err_free_host_wake:
    gpio_free(gps_bcm->gpioid_hostwake.gpio);
    GPSERR(" err_free_host_wake!");

err_free_gps_en:
    gpio_free(gps_bcm->gpioid_en.gpio);

err_free_nstandby:
err_free_gps:
    kfree(gps_bcm);
    gps_bcm = NULL;
    g_gps_bcm = NULL;
    return ret;
}

static void K3_gps_bcm_shutdown(struct platform_device *pdev)
{
    GPS_BCM_INFO *gps_bcm = NULL;
    if(pdev == NULL )
    {
        pr_err("[GPS]NULL Pointer to platform device\n");
        return;
    }
    gps_bcm = platform_get_drvdata(pdev);
    GPSINFO(" K3_gps_bcm_shutdown!");

    if (!gps_bcm) {
        GPSERR("gps_bcm is NULL,just return.");
        return;
    }

    platform_set_drvdata(pdev, NULL);
    kfree(gps_bcm);
    gps_bcm = NULL;
    g_gps_bcm = NULL;
    return;
}

static const struct of_device_id gps_power_match_table[] = {
    {
     .compatible = DTS_COMP_GPS_POWER_NAME, /*compatible must match with which defined in dts*/
     .data = NULL,
     },
    {
     },
};

MODULE_DEVICE_TABLE(of, gps_power_match_table);

int bcm4774_resume(struct platform_device *p);
int bcm4774_suspend(struct platform_device *p, pm_message_t state);
static struct platform_driver k3_gps_bcm_driver = {
    .probe = k3_gps_bcm_probe,
    .suspend = bcm4774_suspend,
    .remove = NULL,
    .resume = bcm4774_resume,
    .shutdown = K3_gps_bcm_shutdown,
    .driver = {
           .name = "huawei,gps_power_4774",
           .owner = THIS_MODULE,
           .of_match_table = of_match_ptr(gps_power_match_table),   /*dts required code*/
    },
};

static int __init k3_gps_bcm_init(void)
{
#ifdef CONFIG_HWCONNECTIVITY
    /*For OneTrack, we need check it's the right chip type or not.
     If it's not the right chip type, don't init the driver*/
    if (!isMyConnectivityChip(CHIP_TYPE_BCM)) {
        GPSERR("gps chip type is not match, skip driver init");
        return -EINVAL;
    } else {
        GPSINFO(
               "gps chip type is matched with Broadcom, continue");
    }
#endif

    if (GPS_IC_TYPE_4774 != get_gps_ic_type()) {
        GPSINFO(
               "gps chip type is matched with Broadcom, but is not 4774");
        return -EINVAL;
    }
    GPSINFO(
           "gps chip type is matched with Broadcom, and it is 4774");
    return platform_driver_register(&k3_gps_bcm_driver);
}

static void __exit k3_gps_bcm_exit(void)
{
    platform_driver_unregister(&k3_gps_bcm_driver);
}

int set_gps_ref_clk_enable_bcm4774(bool enable)
{
    int ret = 0;

    GPSINFO("[GPS] set_gps_ref_clk_enable(%d)", enable);
    if (IS_ERR(gps_ref_clk)) {
        GPSERR(" ERROR: refclk is invalid");
        return -1;
    }

    if (enable) {
        ret = clk_prepare_enable(gps_ref_clk);
        if (ret < 0) {
            GPSERR("ERROR: refclk enable failed!");
            return -1;
        }
    } else {
        clk_disable_unprepare(gps_ref_clk);
    }
    GPSINFO(" set_gps_ref_clk_enable finish");

    return 0;
}

module_init(k3_gps_bcm_init);
module_exit(k3_gps_bcm_exit);

MODULE_AUTHOR("DRIVER_AUTHOR");
MODULE_DESCRIPTION("GPS Boardcom 4774 driver");
MODULE_LICENSE("GPL");

