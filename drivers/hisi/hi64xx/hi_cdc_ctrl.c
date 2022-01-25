#include <linux/module.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/device.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/of_irq.h>
#include <linux/clk.h>
#include <linux/regulator/consumer.h>
#include <linux/pm_runtime.h>
#include <linux/mfd/hisi_pmic.h>
#include <rdr_hisi_audio_adapter.h>

#include <linux/hisi/hi64xx/hi_cdc_ctrl.h>

/*lint -e838 -e730 -e774 -e747 -e529 -e438 -e826 -e778 -e527 -e574*/

struct reg_ops {
	unsigned int (*read8)(unsigned int reg);
	unsigned int (*read32)(unsigned int reg);
	void (*write8)(unsigned int reg, unsigned int val);
	void (*write32)(unsigned int reg, unsigned int val);
};

/* record the register operations for dump */
struct reg_op_item {
	struct timespec tm;
	int    rw;
	unsigned int reg;
	unsigned int val;
};

struct hi_cdc_ctrl_priv {
	struct hi_cdc_ctrl  cdc_ctrl;
	int irq;
	unsigned int regaddr8_begin;
	unsigned int regaddr8_end;
	unsigned int pmu_codec_mclk_addr;
	struct mutex io_mutex;
	struct reg_ops reg_ops;
	struct clk *cdc_mclk;
	struct regulator *supplies[CDC_SUP_MAX];
	struct regulator *ldo8;
	/* for debug */
	struct reg_op_item op_rec[128];
	int rec_wr_idx;
};
static struct hi_cdc_ctrl_priv *cdc_ctrl_priv = NULL;

/* TODO: should use common interfaces, for both 6402 & 6403 */
extern unsigned int slimbus_read_1byte(unsigned int reg);
extern unsigned int slimbus_read_4byte(unsigned int reg);
extern void slimbus_write_1byte(unsigned int reg, unsigned int val);
extern void slimbus_write_4byte(unsigned int reg, unsigned int val);

extern unsigned int ssi_reg_read8(unsigned int reg);
extern unsigned int ssi_reg_read32(unsigned int reg);
extern void ssi_reg_write8(unsigned int reg, unsigned int val);
extern void ssi_reg_write32(unsigned int reg, unsigned int val);

/* this function can only be called when io_mutex is locked */
static void _record_reg_op(struct hi_cdc_ctrl_priv *priv, int rw,
	unsigned int reg_addr, unsigned int reg_val)
{
	int idx = priv->rec_wr_idx;
	struct reg_op_item *item = &priv->op_rec[idx];

	get_monotonic_boottime(&item->tm);
	item->rw = rw;
	item->reg = reg_addr;
	item->val = reg_val;

	if (++priv->rec_wr_idx == ARRAY_SIZE(priv->op_rec))
		priv->rec_wr_idx = 0;
}

static void _record_reg_dump(struct hi_cdc_ctrl_priv *priv)
{
	int i;
	struct reg_op_item *item;
	pr_info("!=== codec register operations dump begin: ===\n");
	mutex_lock(&priv->io_mutex);
	for (i = priv->rec_wr_idx; i < ARRAY_SIZE(priv->op_rec); i++) {
		item = &priv->op_rec[i];
		pr_info("[%6ld.%09ld]%c: %08x, %08x\n", item->tm.tv_sec,
			item->tm.tv_nsec, item->rw ? 'w' : 'r', item->reg, item->val);
	}

	for (i = 0; i < priv->rec_wr_idx; i++) {
		item = &priv->op_rec[i];
		pr_info("[%6ld.%09ld]%c: %08x, %08x\n", item->tm.tv_sec,
			item->tm.tv_nsec, item->rw ? 'w' : 'r', item->reg, item->val);
	}
	mutex_unlock(&priv->io_mutex);
	pr_info("!=== codec register operations dump end ===\n");
}

void hi_cdcctrl_dump(struct hi_cdc_ctrl *cdc_ctrl)
{
	struct hi_cdc_ctrl_priv *priv = (struct hi_cdc_ctrl_priv *)cdc_ctrl;
	_record_reg_dump(priv);
}
EXPORT_SYMBOL(hi_cdcctrl_dump);

unsigned int hi_cdcctrl_reg_read(struct hi_cdc_ctrl *cdc_ctrl,
								 unsigned int reg_addr)
{
	struct hi_cdc_ctrl_priv *priv = (struct hi_cdc_ctrl_priv *)cdc_ctrl;
	int pm_ret = 0;
	unsigned int ret = 0;

	mutex_lock(&priv->io_mutex);

	if (priv->cdc_ctrl.pm_runtime_support) {
		pm_ret = pm_runtime_get_sync(cdc_ctrl->dev);
		if (pm_ret < 0) {
			pr_err("[%s:%d] pm resume error, reg_addr:0x%pK pm_ret:%d\n", __FUNCTION__, __LINE__, (void *)(unsigned long)reg_addr, pm_ret);
			mutex_unlock(&priv->io_mutex);
			rdr_system_error(RDR_AUDIO_RUNTIME_SYNC_FAIL_MODID, 0, 0);
			return 0;
		}
	}

	if (reg_addr >= priv->regaddr8_begin && reg_addr <= priv->regaddr8_end) {
		if (priv->cdc_ctrl.reg_read_twice) {
			(void)priv->reg_ops.read8(reg_addr);
		}
		ret = priv->reg_ops.read8(reg_addr);
	} else {
		if (priv->cdc_ctrl.reg_read_twice) {
			(void)priv->reg_ops.read32(reg_addr);
		}
		ret = priv->reg_ops.read32(reg_addr);
	}
	_record_reg_op(priv, 0, reg_addr, ret);

	if (priv->cdc_ctrl.pm_runtime_support) {
		pm_runtime_mark_last_busy(cdc_ctrl->dev);
		pm_runtime_put_autosuspend(cdc_ctrl->dev);
	}

	mutex_unlock(&priv->io_mutex);

	return ret;
}
EXPORT_SYMBOL(hi_cdcctrl_reg_read);

int hi_cdcctrl_reg_write(struct hi_cdc_ctrl *cdc_ctrl,
						 unsigned int reg_addr, unsigned int reg_val)
{
	struct hi_cdc_ctrl_priv *priv = (struct hi_cdc_ctrl_priv *)cdc_ctrl;
	int pm_ret = 0;

	mutex_lock(&priv->io_mutex);

	if (priv->cdc_ctrl.pm_runtime_support) {
		pm_ret = pm_runtime_get_sync(cdc_ctrl->dev);
		if (pm_ret < 0) {
			pr_err("[%s:%d] pm resume error, reg_addr:0x%pK pm_ret:%d\n", __FUNCTION__, __LINE__, (void *)(unsigned long)reg_addr, pm_ret);
			mutex_unlock(&priv->io_mutex);
			rdr_system_error(RDR_AUDIO_RUNTIME_SYNC_FAIL_MODID, 0, 0);
			return 0;
		}
	}

	_record_reg_op(priv, 1, reg_addr, reg_val);

	if (reg_addr >= priv->regaddr8_begin && reg_addr <= priv->regaddr8_end)
		priv->reg_ops.write8(reg_addr, reg_val);
	else
		priv->reg_ops.write32(reg_addr, reg_val);

	if (priv->cdc_ctrl.pm_runtime_support) {
		pm_runtime_mark_last_busy(cdc_ctrl->dev);
		pm_runtime_put_autosuspend(cdc_ctrl->dev);
	}

	mutex_unlock(&priv->io_mutex);

	return 0;
}
EXPORT_SYMBOL(hi_cdcctrl_reg_write);

void hi_cdcctrl_reg_update_bits(struct hi_cdc_ctrl *cdc_ctrl, unsigned int reg,
								unsigned int mask, unsigned int value)
{
	struct hi_cdc_ctrl_priv *priv = (struct hi_cdc_ctrl_priv *)cdc_ctrl;
	unsigned int old, new;
	int pm_ret = 0;

	if (priv->cdc_ctrl.pm_runtime_support) {
		pm_ret = pm_runtime_get_sync(cdc_ctrl->dev);
		if (pm_ret < 0) {
			pr_err("[%s:%d] pm resume error, reg:0x%pK pm_ret:%d\n", __FUNCTION__, __LINE__, (void *)(unsigned long)reg, pm_ret);
			rdr_system_error(RDR_AUDIO_RUNTIME_SYNC_FAIL_MODID, 0, 0);
			return ;
		}
	}
	old = hi_cdcctrl_reg_read(cdc_ctrl, reg);
	new = (old & ~mask) | (value & mask);
	hi_cdcctrl_reg_write(cdc_ctrl, reg, new);

	if (priv->cdc_ctrl.pm_runtime_support) {
		pm_runtime_mark_last_busy(cdc_ctrl->dev);
		pm_runtime_put_autosuspend(cdc_ctrl->dev);
	}
}
EXPORT_SYMBOL(hi_cdcctrl_reg_update_bits);

int hi_cdcctrl_hw_reset(struct hi_cdc_ctrl *cdc_ctrl)
{
	return 0;
}
EXPORT_SYMBOL(hi_cdcctrl_hw_reset);

int hi_cdcctrl_get_irq(struct hi_cdc_ctrl *cdc_ctrl)
{
	struct hi_cdc_ctrl_priv *priv = (struct hi_cdc_ctrl_priv *)cdc_ctrl;
	return priv->irq;
}
EXPORT_SYMBOL(hi_cdcctrl_get_irq);

int hi_cdcctrl_enable_supply(struct hi_cdc_ctrl *cdc_ctrl,
							 enum hi_cdcctrl_supply sup_type, bool enable)
{
	struct hi_cdc_ctrl_priv *priv = (struct hi_cdc_ctrl_priv *)cdc_ctrl;

	if (priv == NULL) {
		/* may be used by test program, just return */
		return 0;
	}

	if (!priv->supplies[sup_type]) {
		/* Some supplies don't need to be controlled, it's not an error */
		return 0;
	}

	if (enable)
		return regulator_enable(priv->supplies[sup_type]);
	else
		return regulator_disable(priv->supplies[sup_type]);

}
EXPORT_SYMBOL(hi_cdcctrl_enable_supply);

int hi_cdcctrl_enable_clk(struct hi_cdc_ctrl *cdc_ctrl,
						  enum hi_cdcctrl_clk clk_type, bool enable)
{
	struct hi_cdc_ctrl_priv *priv = (struct hi_cdc_ctrl_priv *)cdc_ctrl;

	if (priv == NULL) {
		/* may be used by test program, just return */
		return 0;
	}

	if (CDC_MCLK == clk_type) {
		if (!priv->cdc_mclk) {
			/* No mclk, it's not an error,  */
			return 0;
		}

		if (enable)
			return clk_prepare_enable(priv->cdc_mclk);
		else
			clk_disable_unprepare(priv->cdc_mclk);
	} else {
		/* do nothing */
	}

	return 0;
}
EXPORT_SYMBOL(hi_cdcctrl_enable_clk);

unsigned int hi_cdcctrl_get_pmu_mclk_status()
{
	if (!cdc_ctrl_priv) {
		pr_err("[%s:%d] cdc ctrl priv is null\n", __FUNCTION__, __LINE__);
		return 0;
	}

	if (cdc_ctrl_priv->pmu_codec_mclk_addr == 0) {
		pr_err("[%s:%d] codec mclk addr get error\n", __FUNCTION__, __LINE__);
		return 0;
	}

	return hisi_pmic_reg_read(cdc_ctrl_priv->pmu_codec_mclk_addr);
}

static struct of_device_id of_codec_controller_child_match_tbl[] = {
	/* hi64xx_irq */
	{
		.compatible = "hisilicon,hi64xx-irq",
	},
	{ /* end */ }
};

void hi_cdc_bus_type_select(struct hi_cdc_ctrl_priv *priv,
						struct device *dev, struct device_node *np)
{
	int ret = 0;
	const char *str;

	ret = of_property_read_string(np, "hisilicon,bus-sel", &str);
	if (ret == 0 && !strncmp(str, "slimbus", 7)) {
		priv->cdc_ctrl.bus_sel = BUSTYPE_SELECT_SLIMBUS;
		priv->reg_ops.read8 = slimbus_read_1byte;
		priv->reg_ops.read32 = slimbus_read_4byte;
		priv->reg_ops.write8 = slimbus_write_1byte;
		priv->reg_ops.write32 = slimbus_write_4byte;
	} else {
		priv->cdc_ctrl.bus_sel = BUSTYPE_SELECT_SSI;
		priv->reg_ops.read8 = ssi_reg_read8;
		priv->reg_ops.read32 = ssi_reg_read32;
		priv->reg_ops.write8 = ssi_reg_write8;
		priv->reg_ops.write32 = ssi_reg_write32;
		if (of_property_read_bool(dev->of_node, "pm_runtime_support")) {
			priv->cdc_ctrl.pm_runtime_support = true;
		}

		if (of_property_read_bool(dev->of_node, "reg_read_twice")) {
			priv->cdc_ctrl.reg_read_twice = true;
		}
	}

}

void hi_cdc_ioparam_read(struct platform_device *pdev, struct hi_cdc_ctrl_priv *priv)
{
	uint32_t   slimbusclk_drv = 0;
	uint32_t   slimbusdata_drv = 0;

	/* default 16ma */
	priv->cdc_ctrl.slimbusclk_cdc_drv = 0x5;
	priv->cdc_ctrl.slimbusdata_cdc_drv = 0x5;

	if (!of_property_read_u32(pdev->dev.of_node, "slimbusclk_io_driver", &slimbusclk_drv)) {
		priv->cdc_ctrl.slimbusclk_cdc_drv = slimbusclk_drv;
	}

	if (!of_property_read_u32(pdev->dev.of_node, "slimbusdata_io_driver", &slimbusdata_drv)) {
		priv->cdc_ctrl.slimbusdata_cdc_drv = slimbusdata_drv;
	}

	return;
}

#define LDO8_VOLTAGE 1800000
#define LDO8_ENABLE 1
static void hi_cdc_get_regulator(struct device *dev, struct hi_cdc_ctrl_priv *priv)
{
	unsigned int supply_val = 0;
	unsigned int set_val = 0;
	bool set_flag = false;
	struct device_node *np = dev->of_node;

	if (!of_property_read_u32(np, "codec_ldo8_set_voltage", &set_val)) {
		dev_info(dev, "codec_ldo8_set_voltage is %d\n", set_val);
		if (LDO8_ENABLE == set_val) {
			set_flag = true;
		}
	}

	if (!of_property_read_u32(np, "hisilicon,ldo8_supply", &supply_val)) {
		dev_info(dev, "ldo8 supply is %d\n", supply_val);
		if (LDO8_ENABLE == supply_val) {
			priv->ldo8 = devm_regulator_get(dev, "codec_ldo8");
			if (IS_ERR(priv->ldo8)) {
				priv->ldo8 = NULL;
			} else {
				if (set_flag) {
					if (regulator_set_voltage(priv->ldo8, LDO8_VOLTAGE, LDO8_VOLTAGE)) {
						dev_err(dev, "failed to regulator_set_voltage\n");
						return;
					}
				}
				if (regulator_enable(priv->ldo8)) {
					dev_err(dev, "failed to enable ldo8 supply\n");
				}
			}
		}
	} else {
		dev_info(dev, "ldo8 is not support\n");
	}
}

static int hi_cdcctrl_probe(struct platform_device *pdev)
{
	int ret;
	struct device *dev = &pdev->dev;
	struct device_node *np = dev->of_node;
	struct hi_cdc_ctrl_priv *priv;
	int gpio;
	enum of_gpio_flags flags;

	dev_info(dev, "probe begin");
	priv = devm_kzalloc(dev, sizeof(struct hi_cdc_ctrl_priv), GFP_KERNEL);
	if (!priv) {
		return -ENOMEM;
	}

	platform_set_drvdata(pdev, priv);
	priv->cdc_ctrl.dev = dev;
	cdc_ctrl_priv = priv;

	hi_cdc_bus_type_select(priv, dev, np);

	hi_cdc_ioparam_read(pdev, priv);

	ret = of_property_read_u32(np, "hisilicon,reg-8bit-begin-addr", &priv->regaddr8_begin);
	if (ret) {
		dev_err(dev, "read reg-8bit-begin-addr error\n");
		goto err_exit;
	}

	ret = of_property_read_u32(np, "hisilicon,reg-8bit-end-addr", &priv->regaddr8_end);
	if (ret) {
		dev_err(dev, "read reg-8bit-end-addr error\n");
		goto err_exit;
	}

	priv->supplies[CDC_SUP_MAIN] = devm_regulator_get(dev, "codec-main");
	if (IS_ERR(priv->supplies[CDC_SUP_MAIN]))
		priv->supplies[CDC_SUP_MAIN] = NULL;
	else
		if (regulator_enable(priv->supplies[CDC_SUP_MAIN]))
			dev_err(dev, "failed to enable codec main supply\n");

	priv->supplies[CDC_SUP_ANLG] = devm_regulator_get(dev, "codec-anlg");
	if (IS_ERR(priv->supplies[CDC_SUP_ANLG]))
		priv->supplies[CDC_SUP_ANLG] = NULL;

	priv->cdc_mclk = devm_clk_get(dev, "clk_pmuaudioclk");
	if (IS_ERR(priv->cdc_mclk)) {
		dev_err(dev, "cdc mclk not found!\n");
		ret = PTR_ERR(priv->cdc_mclk);
		goto err_exit;
	}

	/*TODO: should enable/disable during read/write? */
	ret = clk_prepare_enable(priv->cdc_mclk);
	if (ret) {
		dev_err(dev, "clk prepare enable failed!\n");
		goto err_exit;
	} else {
		mdelay(1);
	}

	gpio = of_get_gpio_flags(np, 0, &flags);
	if (gpio < 0) {
		dev_err(dev, "get gpio flags failed\n");
		ret = gpio;
		goto init_gpio_err;
	}

	dev_info(dev, "probe gpio %d", gpio);

	if (!gpio_is_valid(gpio)) {
		dev_err(dev, "gpio%d is invalid\n", gpio);
		ret = -EINVAL;
		goto init_gpio_err;
	}

	ret = gpio_request_one((unsigned int)gpio, GPIOF_IN, "codec_interrupt");
	if (ret < 0) {
		dev_err(dev, "failed to request gpio%d\n", gpio);
		goto init_gpio_err;
	}

	priv->irq = gpio_to_irq((unsigned int)gpio);

	dev_info(dev, "probe irq %d", priv->irq);

	mutex_init(&priv->io_mutex);

	/* populate sub nodes */
	of_platform_populate(np, of_codec_controller_child_match_tbl, NULL, dev);

	if (priv->cdc_ctrl.pm_runtime_support) {
		pm_runtime_use_autosuspend(&pdev->dev);
		pm_runtime_set_autosuspend_delay(&pdev->dev, 200); /* 200ms */
		pm_runtime_set_active(&pdev->dev);
		pm_runtime_enable(&pdev->dev);
	}

	hi_cdc_get_regulator(dev, priv);

	(void)of_property_read_u32(np, "pmu_clkcodec_addr", &priv->pmu_codec_mclk_addr);

	dev_info(dev, "codec-controller probe ok, slimbusclk_drv:%d, slimbusdata_drv:%d, pm runtime support:%d, read twice:%d\n",
				priv->cdc_ctrl.slimbusclk_cdc_drv, priv->cdc_ctrl.slimbusdata_cdc_drv, priv->cdc_ctrl.pm_runtime_support,\
				priv->cdc_ctrl.reg_read_twice);

	return 0;

init_gpio_err:
	clk_disable_unprepare(priv->cdc_mclk);
err_exit:
	if (priv->supplies[CDC_SUP_MAIN]) {
		ret = regulator_disable(priv->supplies[CDC_SUP_MAIN]);
		if (ret != 0)
			dev_err(dev, "regulator_disable: %d", ret);
	}

	dev_err(dev, "codec-controller probe fail\n");
	return ret;
}

static int hi_cdcctrl_remove(struct platform_device *pdev)
{
	struct hi_cdc_ctrl_priv *priv =
		(struct hi_cdc_ctrl_priv *)platform_get_drvdata(pdev);
	struct device *dev = &pdev->dev;

	mutex_destroy(&priv->io_mutex);

	if (priv->cdc_ctrl.pm_runtime_support) {
		pm_runtime_resume(&pdev->dev);
		pm_runtime_disable(&pdev->dev);
		pm_runtime_set_suspended(&pdev->dev);
	}

	if (priv->supplies[CDC_SUP_MAIN]) {
		int ret = regulator_disable(priv->supplies[CDC_SUP_MAIN]);
		if (ret != 0)
			dev_err(dev, "regulator_disable: %d", ret);
	}

	if (priv->ldo8) {
		if (regulator_disable(priv->ldo8))
			dev_err(dev, "lodo8 regulator disable failed");
	}

	clk_disable_unprepare(priv->cdc_mclk);

	cdc_ctrl_priv = NULL;

	return 0;
}

static int hi_cdcctrl_suspend(struct device *device)
{
	/*TODO: should enable/disable during read/write? */
	struct platform_device *pdev = to_platform_device(device);
	struct hi_cdc_ctrl_priv *priv =
		(struct hi_cdc_ctrl_priv *)platform_get_drvdata(pdev);
	int pm_ret = 0;

	WARN_ON(NULL == priv);

	if (priv->cdc_ctrl.pm_runtime_support) {
		pm_ret = pm_runtime_get_sync(priv->cdc_ctrl.dev);
		if (pm_ret < 0) {
			pr_err("[%s:%d] pm resume error, pm_ret:%d\n", __FUNCTION__, __LINE__, pm_ret);
			rdr_system_error(RDR_AUDIO_RUNTIME_SYNC_FAIL_MODID, 0, 0);
			return pm_ret;
		}
	}

	pr_info("[%s:%d] usage_count:0x%x status:0x%x disable_depth:%d clk:%d \n",__FUNCTION__, __LINE__,
		atomic_read(&(device->power.usage_count)), device->power.runtime_status, device->power.disable_depth, clk_get_enable_count(priv->cdc_mclk));

	clk_disable_unprepare(priv->cdc_mclk);

	return 0;
}

static int hi_cdcctrl_resume(struct device *device)
{
	/*TODO: should enable/disable during read/write? */
	struct platform_device *pdev = to_platform_device(device);
	struct hi_cdc_ctrl_priv *priv =
		(struct hi_cdc_ctrl_priv *)platform_get_drvdata(pdev);
	int ret = 0;

	WARN_ON(NULL == priv);

	ret = clk_prepare_enable(priv->cdc_mclk);
	if (ret) {
		dev_err(&pdev->dev, "clk prepare enable failed!\n");
		return ret;
	}

	mdelay(1);

	if (priv->cdc_ctrl.pm_runtime_support) {
		pm_runtime_mark_last_busy(priv->cdc_ctrl.dev);
		pm_runtime_put_autosuspend(priv->cdc_ctrl.dev);

		pm_runtime_disable(priv->cdc_ctrl.dev);
		pm_runtime_set_active(priv->cdc_ctrl.dev);
		pm_runtime_enable(priv->cdc_ctrl.dev);
	}

	pr_info("[%s:%d] usage_count:0x%x status:0x%x disable_depth:%d clk:%d \n",__FUNCTION__, __LINE__,
		atomic_read(&(device->power.usage_count)), device->power.runtime_status, device->power.disable_depth, clk_get_enable_count(priv->cdc_mclk));

	return 0;
}

void hi_cdcctrl_pm_get(void)
{
	int pm_ret = 0;

	if (!cdc_ctrl_priv) {
		pr_err("[%s:%d] cdc ctrl priv is null\n", __FUNCTION__, __LINE__);
		return;
	}

	if (cdc_ctrl_priv->cdc_ctrl.pm_runtime_support) {
		pm_ret = pm_runtime_get_sync(cdc_ctrl_priv->cdc_ctrl.dev);
		if (pm_ret < 0) {
			pr_err("[%s:%d] pm resume error, pm_ret:%d\n", __FUNCTION__, __LINE__, pm_ret);
			rdr_system_error(RDR_AUDIO_RUNTIME_SYNC_FAIL_MODID, 0, 0);
		}
	}
}
EXPORT_SYMBOL(hi_cdcctrl_pm_get);

void hi_cdcctrl_pm_put(void)
{
	if (!cdc_ctrl_priv) {
		pr_err("[%s:%d] cdc ctrl priv is null\n", __FUNCTION__, __LINE__);
		return;
	}

	if (cdc_ctrl_priv->cdc_ctrl.pm_runtime_support) {
		pm_runtime_mark_last_busy(cdc_ctrl_priv->cdc_ctrl.dev);
		pm_runtime_put_autosuspend(cdc_ctrl_priv->cdc_ctrl.dev);
	}
}
EXPORT_SYMBOL(hi_cdcctrl_pm_put);

static int hi_cdcctrl_runtime_suspend(struct device *device)
{
	struct platform_device *pdev = to_platform_device(device);
	struct hi_cdc_ctrl_priv *priv =
		(struct hi_cdc_ctrl_priv *)platform_get_drvdata(pdev);

	clk_disable_unprepare(priv->cdc_mclk);

	pr_info("[%s:%d] usage_count:0x%x status:0x%x disable_depth:%d clk:%d --\n",__FUNCTION__, __LINE__,
		atomic_read(&(device->power.usage_count)), device->power.runtime_status, device->power.disable_depth, clk_get_enable_count(priv->cdc_mclk));

	return 0;
}

static int hi_cdcctrl_runtime_resume(struct device *device)
{
	struct platform_device *pdev = to_platform_device(device);
	struct hi_cdc_ctrl_priv *priv =
		(struct hi_cdc_ctrl_priv *)platform_get_drvdata(pdev);
	int ret = 0;

	ret = clk_prepare_enable(priv->cdc_mclk);
	if (ret)
		dev_err(&pdev->dev, "clk prepare enable failed!\n");

	mdelay(5);

	pr_info("[%s:%d] usage_count:0x%x status:0x%x disable_depth:%d clk:%d ++\n",__FUNCTION__, __LINE__,
		atomic_read(&(device->power.usage_count)), device->power.runtime_status, device->power.disable_depth, clk_get_enable_count(priv->cdc_mclk));

	return ret;
}

static struct of_device_id of_hi_cdcctrl_match[] = {
	{
		.compatible = "hisilicon,codec-controller",
	},
	{ /* end */ }
};

static const struct dev_pm_ops hi_cdcctrl_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(hi_cdcctrl_suspend, hi_cdcctrl_resume)
	SET_RUNTIME_PM_OPS(hi_cdcctrl_runtime_suspend, hi_cdcctrl_runtime_resume, NULL)
};

static struct platform_driver hi_cdc_ctrl_driver = {
	.driver = {
		.name	= "codec_controller",
		.owner  = THIS_MODULE,
		.pm = &hi_cdcctrl_pm_ops,
		.of_match_table = of_hi_cdcctrl_match,
	},
	.probe		= hi_cdcctrl_probe,
	.remove		= hi_cdcctrl_remove,
};

static int __init hi_cdcctrl_init(void)
{
	return platform_driver_register(&hi_cdc_ctrl_driver);
}

static void __exit hi_cdcctrl_exit(void)
{
	platform_driver_unregister(&hi_cdc_ctrl_driver);
}
fs_initcall_sync(hi_cdcctrl_init);
module_exit(hi_cdcctrl_exit);

MODULE_DESCRIPTION("hisi codec controller");
MODULE_AUTHOR("dingqing <dingqing@hisilicon.com>");
MODULE_LICENSE("GPL");
