/*
 * PCIe host controller driver for Kirin SoCs
 *
 * Copyright (C) 2015 Hilisicon Electronics Co., Ltd.
 *		http://www.huawei.com
 *
 * Author: Xiaowei Song <songxiaowei@huawei.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include "pcie-kirin.h"
#include "pcie-kirin-common.h"
#include <hitest_slt.h>
#include <linux/module.h>
#include <linux/pci-aspm.h>

/*lint -e438 -e550 -e647 -e679 -e713 -e732 -e737 -e774 -e838  -esym(438,*) -esym(550,*) -esym(647,*) -esym(679,*) -esym(713,*) -esym(732,*) -esym(737,*) -esym(774,*) -esym(838,*) */
unsigned int g_rc_num;
unsigned int dbi_debug_flag = 0;
struct kirin_pcie g_kirin_pcie[] = {
	{
		.irq = {
				{
					.name = "kirin-pcie0-inta",
				},
				{
					.name = "kirin-pcie0-msi",
				},
				{
					.name = "kirin-pcie0-intc",
				},
				{
					.name = "kirin-pcie0-intd",
				},
				{
					.name = "kirin-pcie0-linkdown",
				},
				{
					.name = "kirin-pcie0-cpltimeout",
				}
			},
		.rc_dev = NULL,
		.ep_dev = NULL,
		.is_ready = ATOMIC_INIT(0),
		.is_enumerated = ATOMIC_INIT(0),
		.is_power_on = ATOMIC_INIT(0),
		.usr_suspend = ATOMIC_INIT(0),
		.is_removed = ATOMIC_INIT(0),
	},
	{
		.irq = {
				{
					.name = "kirin-pcie1-inta",
				},
				{
					.name = "kirin-pcie1-msi",
				},
				{
					.name = "kirin-pcie1-intc",
				},
				{
					.name = "kirin-pcie1-intd",
				},
				{
					.name = "kirin-pcie1-linkdown",
				},
				{
					.name = "kirin-pcie1-cpltimeout",
				}
			},
		.rc_dev = NULL,
		.ep_dev = NULL,
		.is_ready = ATOMIC_INIT(0),
		.is_enumerated = ATOMIC_INIT(0),
		.is_power_on = ATOMIC_INIT(0),
		.usr_suspend = ATOMIC_INIT(0),
		.is_removed = ATOMIC_INIT(0),
	},
};

void kirin_elb_writel(struct kirin_pcie *pcie, u32 val, u32 reg)
{
	writel(val, pcie->apb_base + reg);
}

u32 kirin_elb_readl(struct kirin_pcie *pcie, u32 reg)
{
	return readl(pcie->apb_base + reg);
}

void kirin_apb_phy_writel(struct kirin_pcie *pcie, u32 val, u32 reg)
{
	writel(val, pcie->phy_base + pcie->apb_phy_offset + reg);
}

u32 kirin_apb_phy_readl(struct kirin_pcie *pcie, u32 reg)
{
	return readl(pcie->phy_base + pcie->apb_phy_offset + reg);
}

void kirin_natural_phy_writel(struct kirin_pcie *pcie, u32 val, u32 reg)
{
	if (pcie->dtsinfo.board_type != BOARD_FPGA)
		writel(val, pcie->phy_base + pcie->natural_phy_offset + reg * 4);
}

u32 kirin_natural_phy_readl(struct kirin_pcie *pcie, u32 reg)
{
	if (pcie->dtsinfo.board_type != BOARD_FPGA)
		return readl(pcie->phy_base + pcie->natural_phy_offset + reg * 4);
	return 0;
}

void kirin_sram_phy_writel(struct kirin_pcie *pcie, u32 val, u32 reg)
{
	if (pcie->dtsinfo.board_type != BOARD_FPGA)
		writel(val, pcie->phy_base + pcie->sram_phy_offset + reg * 4);
}

u32 kirin_sram_phy_readl(struct kirin_pcie *pcie, u32 reg)
{
	if (pcie->dtsinfo.board_type != BOARD_FPGA)
		return readl(pcie->phy_base + pcie->sram_phy_offset + reg * 4);
	return 0;
}


static int32_t kirin_pcie_get_clk(struct kirin_pcie *pcie, struct platform_device *pdev)
{
	pcie->pcie_aux_clk = devm_clk_get(&pdev->dev, "pcie_aux");
	if (IS_ERR(pcie->pcie_aux_clk)) {
		PCIE_PR_ERR("Failed to get pcie_aux clock");
		return PTR_ERR(pcie->pcie_aux_clk);
	}

	pcie->apb_phy_clk = devm_clk_get(&pdev->dev, "pcie_apb_phy");
	if (IS_ERR(pcie->apb_phy_clk)) {
		PCIE_PR_ERR("Failed to get pcie_apb_phy clock");
		return PTR_ERR(pcie->apb_phy_clk);
	}

	pcie->apb_sys_clk = devm_clk_get(&pdev->dev, "pcie_apb_sys");
	if (IS_ERR(pcie->apb_sys_clk)) {
		PCIE_PR_ERR("Failed to get pcie_apb_sys clock");
		return PTR_ERR(pcie->apb_sys_clk);
	}

	pcie->pcie_aclk = devm_clk_get(&pdev->dev, "pcie_aclk");
	if (IS_ERR(pcie->pcie_aclk)) {
		PCIE_PR_ERR("Failed to get pcie_aclk clock");
		return PTR_ERR(pcie->pcie_aclk);
	}

	PCIE_PR_INFO("Successed to get all clock");

	return 0;
}

static int32_t get_phy_layout(struct kirin_pcie *pcie, struct device_node *np)
{
	u32 val[3] = {0};
	size_t size = 3;

	if (of_property_read_u32_array(np, "phy_layout_info", val, size)) {
		PCIE_PR_ERR("Failed to get phy layout info");
		return -1;
	}

	pcie->natural_phy_offset = val[0];
	pcie->sram_phy_offset = val[1];
	pcie->apb_phy_offset = val[2];

	return 0;
}

/*Calling by kirin_pcie_get_baseaddr */
static long kirin_pcie_get_baseaddr_of_dbi(struct kirin_pcie *pcie,
					struct platform_device *pdev)
{
	struct resource *dbi;
	dbi = platform_get_resource_byname(pdev, IORESOURCE_MEM, "dbi");
	/* dbi physical address */
	if(NULL == dbi){
		PCIE_PR_ERR("Failed to get dbi [NULL]");
		return -1;
	}
	pcie->dtsinfo.dbi_base = lower_32_bits(dbi->start);

	/* dbi virtual address */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 69))
	pcie->pp.dbi_base = devm_ioremap_resource(&pdev->dev, dbi);
	if (IS_ERR(pcie->pp.dbi_base)) {
		PCIE_PR_ERR("Failed to get PCIe dbi base");
		return PTR_ERR(pcie->pp.dbi_base);
	}
#else
	pcie->pci->dbi_base = devm_ioremap_resource(&pdev->dev, dbi);
	if (IS_ERR(pcie->pci->dbi_base)) {
		PCIE_PR_ERR("Failed to get PCIe dbi base");
		return PTR_ERR(pcie->pci->dbi_base);
	}
#endif
	return 0;
}

static int32_t kirin_pcie_get_baseaddr(struct kirin_pcie *pcie,
					struct platform_device *pdev)
{
	struct resource *apb;
	struct resource *phy;
	struct device_node *np;

	apb = platform_get_resource_byname(pdev, IORESOURCE_MEM, "apb");
	pcie->apb_base = devm_ioremap_resource(&pdev->dev, apb);
	if (IS_ERR(pcie->apb_base)) {
		PCIE_PR_ERR("Failed to get PCIeCTRL apb base");
		return PTR_ERR(pcie->apb_base);
	}

	phy = platform_get_resource_byname(pdev, IORESOURCE_MEM, "phy");
	pcie->phy_base = devm_ioremap_resource(&pdev->dev, phy);
	if (IS_ERR(pcie->phy_base)) {
		PCIE_PR_ERR("Failed to get PCIePHY base");
		return PTR_ERR(pcie->phy_base);
	}

	if(kirin_pcie_get_baseaddr_of_dbi(pcie, pdev))
		return -1;

	np = pdev->dev.of_node;
	if (of_property_read_u32(np, "iatu_base_offset", &pcie->dtsinfo.iatu_base_offset)) {
		PCIE_PR_ERR("Failed to get iatu_base_offset info");
		return -1;
	}

	if (get_phy_layout(pcie, np))
		return -1;

	np = of_find_compatible_node(NULL, NULL, "hisilicon,sysctrl");
	if (!np) {
		PCIE_PR_ERR("Failed to get sysctrl Node ");
		return -1;
	}
	pcie->sctrl_base = of_iomap(np, 0);
	if (!pcie->sctrl_base) {
		PCIE_PR_ERR("Failed to iomap sctrl_base");
		return -1;
	}

	np = of_find_compatible_node(NULL, NULL, "hisilicon,pmctrl");
	if (!np) {
		PCIE_PR_ERR("Failed to get pmctrl Node ");
		return -1;
	}
	pcie->pmctrl_base = of_iomap(np, 0);
	if (!pcie->pmctrl_base) {
		PCIE_PR_ERR("Failed to iomap sctrl_base");
		return -1;
	}
	pcie->pme_base = ioremap(pcie->dtsinfo.dbi_base + MSG_CPU_ADDR, MSG_CPU_ADDR_SIZE);
	if (!pcie->pme_base) {
		PCIE_PR_ERR("Failed to ioremap pme addr");
		return -1;
	}

	PCIE_PR_DEBUG("Successed to get all resource");
	return 0;
}

static int32_t kirin_pcie_get_pinctrl(struct kirin_pcie *pcie,
					struct platform_device *pdev)
{
	pcie->gpio_id_reset = of_get_named_gpio(pdev->dev.of_node, "reset-gpio", 0);
	if (pcie->gpio_id_reset < 0) {
		PCIE_PR_ERR("Failed to get perst gpio number");
		return -1;
	}

	return 0;
}
static void kirin_pcie_get_boardtype(struct kirin_pcie *pcie,
				struct platform_device *pdev)
{
	int len;
	struct device_node *np;
	struct kirin_pcie_dtsinfo *dtsinfo;

	np = pdev->dev.of_node;
	dtsinfo = &pcie->dtsinfo;
	if (of_property_read_u32(np, "board_type", &dtsinfo->board_type)) {
		dtsinfo->board_type = BOARD_ASIC;
		PCIE_PR_INFO("Failed to get board_type, set default[0x%x]", dtsinfo->board_type);
	}

	if (of_property_read_u32(np, "chip_type", &dtsinfo->chip_type)) {
		dtsinfo->chip_type = CHIP_TYPE_CS;
		PCIE_PR_INFO("Failed to get chip_type, set default[0x%x]", dtsinfo->chip_type);
	}

	if (of_property_read_u32(np, "ep_device_type", &dtsinfo->ep_device_type)) {
		dtsinfo->ep_device_type = EP_DEVICE_NODEV;
		PCIE_PR_INFO("Failed to get ep_device_type, set default[0x%x]", dtsinfo->ep_device_type);
	}

	if (of_find_property(np, "ep_flag", &len)) {
		dtsinfo->ep_flag = 1;
		if (of_find_property(np, "loopback_flag", &len))
			dtsinfo->loopback_flag = 1;
		else
			dtsinfo->loopback_flag = 0;
	}
	else
		dtsinfo->ep_flag = 0;
}

static int32_t kirin_pcie_get_isoinfo(struct kirin_pcie *pcie,
					struct platform_device *pdev)
{
	size_t array_num = 2;
	struct device_node *np;
	struct kirin_pcie_dtsinfo *dtsinfo;

	np = pdev->dev.of_node;
	dtsinfo = &pcie->dtsinfo;

	if (of_property_read_u32_array(np, "iso_info", dtsinfo->iso_info, array_num)) {
		PCIE_PR_ERR("Failed to get isoen info");
		return -1;
	}

	return 0;
}

void kirin_pcie_iso_ctrl(struct kirin_pcie *pcie, int en_flag)
{
	if (en_flag)
		writel(pcie->dtsinfo.iso_info[1],
			pcie->sctrl_base + pcie->dtsinfo.iso_info[0]);
	else
		writel(pcie->dtsinfo.iso_info[1],
			pcie->sctrl_base + pcie->dtsinfo.iso_info[0]
			+ ADDR_OFFSET_4BYTE);
}

static int32_t kirin_pcie_get_assertinfo(struct kirin_pcie *pcie,
					struct platform_device *pdev)
{
	size_t array_num = 2;
	struct device_node *np;
	struct kirin_pcie_dtsinfo *dtsinfo;

	np = pdev->dev.of_node;
	dtsinfo = &pcie->dtsinfo;

	if (of_property_read_u32_array(np, "assert_info", dtsinfo->assert_info, array_num)) {
		PCIE_PR_ERR("Failed to get assert info");
		return -1;
	}

	return 0;
}

void kirin_pcie_reset_ctrl(struct kirin_pcie *pcie, enum RST_TYPE rst)
{
	if (rst == RST_DISABLE)
		writel(pcie->dtsinfo.assert_info[1],
			pcie->crg_base + pcie->dtsinfo.assert_info[0]
			+ ADDR_OFFSET_4BYTE);
	else
		writel(pcie->dtsinfo.assert_info[1],
			pcie->crg_base + pcie->dtsinfo.assert_info[0]);
}

static void kirin_pcie_get_linkstate(struct kirin_pcie *pcie,
					struct platform_device *pdev)
{
	int ret;
	struct kirin_pcie_dtsinfo *dtsinfo;

	dtsinfo = &pcie->dtsinfo;

	ret = of_property_read_u32(pdev->dev.of_node,
				"ep_ltr_latency", &dtsinfo->ep_ltr_latency);
	if (ret) {
		dtsinfo->ep_ltr_latency = 0x0;
		PCIE_PR_INFO("Not set ep_ltr_latency, set default[0x%x]",
			dtsinfo->ep_ltr_latency);
	}

	ret = of_property_read_u32(pdev->dev.of_node,
				"ep_l1ss_ctrl2", &dtsinfo->ep_l1ss_ctrl2);
	if (ret) {
		dtsinfo->ep_l1ss_ctrl2 = 0x0;
		PCIE_PR_INFO("Not set ep_l1ss_ctrl2, set default[0x%x]",
			dtsinfo->ep_l1ss_ctrl2);
	}

	ret = of_property_read_u32(pdev->dev.of_node,
				"l1ss_ctrl1", &dtsinfo->l1ss_ctrl1);
	if (ret) {
		dtsinfo->l1ss_ctrl1 = 0x0;
		PCIE_PR_INFO("Not set L1ss_ctrl1, set default[0x%x]",
			dtsinfo->l1ss_ctrl1);
	}

	ret = of_property_read_u32(pdev->dev.of_node,
				"aspm_state", &dtsinfo->aspm_state);
	if (ret) {
		dtsinfo->aspm_state = ASPM_L1;
		PCIE_PR_INFO("Not set aspm_state, set default[0x%x]",
			dtsinfo->aspm_state);
	}
}

static void kirin_pcie_get_eco(struct kirin_pcie *pcie,
				struct platform_device *pdev)
{
	int ret;
	struct kirin_pcie_dtsinfo *dtsinfo;

	dtsinfo = &pcie->dtsinfo;

	ret = of_property_read_u32(pdev->dev.of_node, "eco", &dtsinfo->eco);
	if (ret) {
		dtsinfo->eco = 0x0;
	}

	PCIE_PR_INFO("SRAM ECO [0x%x]", dtsinfo->eco);
}

static void pcie_get_clk_source(struct kirin_pcie *pcie,
			struct platform_device *pdev)
{
	int ret;
	struct kirin_pcie_dtsinfo *dtsinfo;
	struct device_node *np = pdev->dev.of_node;

	dtsinfo = &pcie->dtsinfo;

	ret = of_property_read_u32(np, "pll_source", &dtsinfo->pll_source);
	if (ret)
		dtsinfo->pll_source = PLL_TYPE_FN;

	ret = of_property_read_u32(np, "ioref_clk_source", &dtsinfo->ioref_clk_source);
	if (ret)
		dtsinfo->ioref_clk_source = REFCLK_FROM_PLL;

	PCIE_PR_INFO("Pll source:%s, Refclk from:%s",
			dtsinfo->pll_source == PLL_TYPE_FN ? "FN_pll": "HP_pll",
			dtsinfo->ioref_clk_source == REFCLK_FROM_PLL ? "FromPll": "FromPhy");
}

static void pcie_get_time_params(struct kirin_pcie *pcie,
					struct platform_device *pdev)
{
	size_t array_num = 2;
	struct device_node *np = pdev->dev.of_node;

	if (of_property_read_u32_array(np, "t_ref2perst",
		pcie->dtsinfo.t_ref2perst, array_num)) {
		PCIE_PR_INFO("Fail: ref2perst time");
		pcie->dtsinfo.t_ref2perst[0] = 20000;
		pcie->dtsinfo.t_ref2perst[1] = 21000;
	}

	if (of_property_read_u32_array(np, "t_perst2access",
		pcie->dtsinfo.t_perst2access, array_num)){
		PCIE_PR_INFO("Fail: perst2access time");
		pcie->dtsinfo.t_perst2access[0] = 7000;
		pcie->dtsinfo.t_perst2access[1] = 8000;
	}

	if (of_property_read_u32_array(np, "t_perst2rst",
		pcie->dtsinfo.t_perst2rst, array_num)) {
		PCIE_PR_INFO("Fail: perst2rst time");
		pcie->dtsinfo.t_perst2rst[0] = 10000;
		pcie->dtsinfo.t_perst2rst[1] = 11000;
	}
}

static int get_rc_num(void)
{
	struct device_node *np = of_find_node_by_name(NULL, "kirin_pcie");
	if (!np) {
		PCIE_PR_ERR("Failed to get kirin_pcie info");
		return -1;
	}

	if (of_property_read_u32(np, "rc_num", &g_rc_num)) {
		PCIE_PR_ERR("Failed to get rc_num info");
		return -1;
	}

	return 0;
}

static void pcie_get_noc_id(struct kirin_pcie *pcie,
					struct platform_device *pdev)
{
	struct device_node *np = pdev->dev.of_node;

	if (of_property_read_u32(np, "noc_target_id", &pcie->dtsinfo.noc_target_id)) {
		pcie->dtsinfo.noc_target_id = 0x0;
		PCIE_PR_INFO("Fail: noc target id, set default[0x%x]",
			pcie->dtsinfo.noc_target_id);
	}

	pcie->dtsinfo.noc_mntn = 0;
}

static void kirin_pcie_get_eyeparam(struct kirin_pcie *pcie, struct platform_device *pdev)
{
	int ret;
	u32 eye_param = 0;
	struct device_node *np = pdev->dev.of_node;
	struct kirin_pcie_dtsinfo *dtsinfo = &pcie->dtsinfo;

	ret = of_property_read_u32_array(np, "io_driver", dtsinfo->io_driver, TRIPLE_TUPLE);
	if (ret)
		dtsinfo->io_driver[2] = 0x0;

	ret = of_property_read_u32(np, "eye_param_nums", &eye_param);
	if (ret) {
		dtsinfo->eye_param_nums = 0;
		PCIE_PR_INFO("Default eye params");
		return;
	} else {
		dtsinfo->eye_param_nums = eye_param;
	}

	/* All params: default val */
	if (!dtsinfo->eye_param_nums)
		return;

	PCIE_PR_DEBUG("Update eye params: %d", dtsinfo->eye_param_nums);

	dtsinfo->eye_param_data = (u32 *)kzalloc(dtsinfo->eye_param_nums * TRIPLE_TUPLE * sizeof(u32), GFP_KERNEL);
	if (!dtsinfo->eye_param_data)  {
		PCIE_PR_ERR("Failed to alloc mem");
		return;
	}

	ret = of_property_read_u32_array(np, "eye_param_details",
			dtsinfo->eye_param_data, dtsinfo->eye_param_nums * TRIPLE_TUPLE);
	if (ret) {
		kfree(dtsinfo->eye_param_data);
		dtsinfo->eye_param_data = NULL;
	}
}

int32_t kirin_pcie_get_dtsinfo(struct kirin_pcie *pcie, struct platform_device *pdev)
{
	if (!pdev->dev.of_node) {
		PCIE_PR_ERR("Of_node is null");
		return -EINVAL;
	}

	kirin_pcie_get_boardtype(pcie, pdev);

	kirin_pcie_get_linkstate(pcie, pdev);

	kirin_pcie_get_eco(pcie, pdev);

	pcie_get_clk_source(pcie, pdev);

	pcie_get_time_params(pcie, pdev);

	pcie_get_noc_id(pcie, pdev);

	kirin_pcie_get_eyeparam(pcie, pdev);

	if (kirin_pcie_get_isoinfo(pcie, pdev))
		return -ENODEV;

	if (kirin_pcie_get_assertinfo(pcie, pdev))
		return -ENODEV;

	if (kirin_pcie_get_clk(pcie, pdev))
		return -ENODEV;

	if (kirin_pcie_get_pinctrl(pcie, pdev))
		return -ENODEV;

	if (kirin_pcie_get_baseaddr(pcie, pdev))
		return -ENODEV;

	return 0;
}

static int perst_from_pciectrl(struct kirin_pcie *pcie, int pull_up)
{
	u32 val;
	val = kirin_elb_readl(pcie, SOC_PCIECTRL_CTRL12_ADDR);
	val |= PERST_FUN_SEC;
	if (pull_up)
		val |= PERST_ASSERT_EN;
	else
		val &= ~PERST_ASSERT_EN;
	kirin_elb_writel(pcie, val, SOC_PCIECTRL_CTRL12_ADDR);

	return 0;
}

static int perst_from_gpio(struct kirin_pcie *pcie, int pull_up)
{
	return gpio_direction_output((unsigned int)pcie->gpio_id_reset, pull_up);
}

int kirin_pcie_perst_cfg(struct kirin_pcie *pcie, int pull_up)
{
	int ret;

	if (pull_up)
		usleep_range(pcie->dtsinfo.t_ref2perst[0], pcie->dtsinfo.t_ref2perst[1]);

	if (pcie->dtsinfo.board_type == BOARD_FPGA)
		ret = perst_from_pciectrl(pcie, pull_up);
	else
		ret = perst_from_gpio(pcie, pull_up);

	if (ret)
		PCIE_PR_ERR("Failed to pulse perst signal");

	if (pull_up)
		usleep_range(pcie->dtsinfo.t_perst2access[0], pcie->dtsinfo.t_perst2access[1]);
	else
		usleep_range(pcie->dtsinfo.t_perst2rst[0], pcie->dtsinfo.t_perst2rst[1]);

	return ret;
}


static void kirin_pcie_sideband_dbi_w_mode(struct kirin_pcie *pcie, bool on)
{
	u32 val;

	val = kirin_elb_readl(pcie, SOC_PCIECTRL_CTRL0_ADDR);
	if (on)
		val = val | PCIE_ELBI_SLV_DBI_ENABLE;
	else
		val = val & ~PCIE_ELBI_SLV_DBI_ENABLE;

	kirin_elb_writel(pcie, val, SOC_PCIECTRL_CTRL0_ADDR);
}

static void kirin_pcie_sideband_dbi_r_mode(struct kirin_pcie *pcie, bool on)
{
	u32 val;

	val = kirin_elb_readl(pcie, SOC_PCIECTRL_CTRL1_ADDR);
	if (on){
		if (IS_ENABLED(CONFIG_KIRIN_PCIE_TEST) && dbi_debug_flag == PCIE_ENABLE_DBI_READ_FLAG){
			PCIE_PR_ERR("switch to local space");
			WARN_ON(1);
		}
		val = val | PCIE_ELBI_SLV_DBI_ENABLE;
	}else
		val = val & ~PCIE_ELBI_SLV_DBI_ENABLE;

	kirin_elb_writel(pcie, val, SOC_PCIECTRL_CTRL1_ADDR);
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 69))
static int kirin_pcie_link_up(struct pcie_port *pp)
{
  	struct kirin_pcie *pcie = to_kirin_pcie(pp);
#else
static int kirin_pcie_link_up(struct dw_pcie *pci)
{
  	struct kirin_pcie *pcie = to_kirin_pcie(pci);
#endif
	u32 val = 0;

	if (!atomic_read(&(pcie->is_power_on)) || atomic_read(&(pcie->usr_suspend)))
		return 0;

	val = kirin_elb_readl(pcie, PCIE_ELBI_RDLH_LINKUP);

	if (((val & PCIE_LINKUP_ENABLE) == PCIE_LINKUP_ENABLE)
		&& pcie->ep_link_status == DEVICE_LINK_UP)
		return 1;

	return 0;
}

static int kirin_pcie_establish_link(struct kirin_pcie *pcie)
{
	int count = 0;

	PCIE_PR_INFO("++");

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 69))
	if (kirin_pcie_link_up(&pcie->pp)) {
#else
	if (kirin_pcie_link_up(pcie->pci)) {
#endif
		PCIE_PR_ERR("Link already up");
		return 0;
	}

	/* setup root complex */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 69))
	dw_pcie_setup_rc(&pcie->pp);
#else
	dw_pcie_setup_rc(&(pcie->pci->pp));
#endif
	PCIE_PR_DEBUG("Setup rc done ");

	kirin_pcie_host_speed_limit(pcie);

	/* assert LTSSM enable */
	kirin_elb_writel(pcie, PCIE_LTSSM_ENABLE_BIT,
			  PCIE_APP_LTSSM_ENABLE);
	/* check if the link is up or not */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 69))
	while (!kirin_pcie_link_up(&pcie->pp)) {
		mdelay(1);
		count++;
		if (count == PCIE_LINK_UP_TIME) {
			PCIE_PR_ERR("Link Fail, status is [0x%x]",
				 kirin_elb_readl(pcie, SOC_PCIECTRL_STATE4_ADDR));
			dsm_pcie_dump_info(pcie, DSM_ERR_ESTABLISH_LINK);
			return -EINVAL;
		}
	}
#else
	if (dw_pcie_wait_for_link(pcie->pci)) {
		PCIE_PR_ERR("Link Fail, status is [0x%x]",
				 kirin_elb_readl(pcie, SOC_PCIECTRL_STATE4_ADDR));
		dsm_pcie_dump_info(pcie, DSM_ERR_ESTABLISH_LINK);
		return -ETIMEDOUT;
	}
#endif
	PCIE_PR_INFO("PCIe Link success after %dms", count);

	if (pcie->plat_ops->cal_alg_adjust)
		pcie->plat_ops->cal_alg_adjust(pcie, true);

	return 0;
}

/*EP register hook fun for link event notification*/
int kirin_pcie_register_event(struct kirin_pcie_register_event *reg)
{
	int ret = 0;
	struct pci_dev *dev;
	struct pcie_port *pp;
	struct kirin_pcie *pcie;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 69))
	struct dw_pcie *pci;
#endif

	if (!reg || !reg->user) {
		PCIE_PR_INFO("Event registration or user of event is null");
		return -ENODEV;
	}

	dev = (struct pci_dev *)reg->user;
	pp = (struct pcie_port *)(dev->bus->sysdata);

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 69))
	pcie = to_kirin_pcie(pp);
#else
	pci = to_dw_pcie_from_pp(pp);
	pcie = to_kirin_pcie(pci);
#endif

	if (pp) {
		pcie->event_reg = reg;
		PCIE_PR_INFO("Event 0x%x is registered for RC", reg->events);
	} else {
		PCIE_PR_INFO("did not find RC for pci endpoint device");
		ret = -ENODEV;
	}
	return ret;
}
EXPORT_SYMBOL_GPL(kirin_pcie_register_event);

int kirin_pcie_deregister_event(struct kirin_pcie_register_event *reg)
{
	int ret = 0;
	struct pci_dev *dev;
	struct pcie_port *pp;
	struct kirin_pcie *pcie;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 69))
	struct dw_pcie *pci;
#endif

	if (!reg || !reg->user) {
		PCIE_PR_INFO("Event registration or user of event is NULL");
		return -ENODEV;
	}

	dev = (struct pci_dev *)reg->user;
	pp = (struct pcie_port *)(dev->bus->sysdata);

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 69))
	pcie = to_kirin_pcie(pp);
#else
	pci = to_dw_pcie_from_pp(pp);
	pcie = to_kirin_pcie(pci);
#endif

	if (pp) {
		pcie->event_reg = NULL;
		PCIE_PR_INFO("deregistered ");
	} else {
		PCIE_PR_INFO("No RC for this EP device ");
		ret = -ENODEV;
	}
	return ret;
}
EXPORT_SYMBOL_GPL(kirin_pcie_deregister_event);

/*notify EP about link-down event*/
static void kirin_pcie_notify_callback(struct kirin_pcie *pcie,
				enum kirin_pcie_event event)
{
	if ((pcie->event_reg != NULL) && (pcie->event_reg->callback != NULL) &&
			(pcie->event_reg->events & event)) {
		struct kirin_pcie_notify *notify = &pcie->event_reg->notify;
		notify->event = event;
		notify->user = pcie->event_reg->user;
		PCIE_PR_INFO("Callback for the event : %d", event);
		pcie->event_reg->callback(notify);
	} else {
		PCIE_PR_INFO("EP does not register this event : %d", event);
	}
}

static void kirin_handle_work(struct work_struct *work)
{
	/*lint -e826 -esym(826,*)*/
	struct kirin_pcie *pcie = container_of(work, struct kirin_pcie, handle_work);
	/*lint -e826 +esym(826,*)*/

	dsm_pcie_dump_info(pcie, DSM_ERR_LINK_DOWN);

	dump_apb_register(pcie);

	kirin_pcie_notify_callback(pcie, KIRIN_PCIE_EVENT_LINKDOWN);
}

static irqreturn_t kirin_pcie_linkdown_irq_handler(int irq, void *arg)
{
	struct kirin_pcie *pcie = arg;

	PCIE_PR_ERR("Triggle linkdown irq[%d]", irq);

	schedule_work(&pcie->handle_work);

	return IRQ_HANDLED;
}

static irqreturn_t kirin_pcie_msi_irq_handler(int irq, void *arg)
{
	struct kirin_pcie *pcie = arg;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 69))
	return dw_handle_msi_irq(&pcie->pp);
#else
	return dw_handle_msi_irq(&(pcie->pci->pp));
#endif
}

static void kirin_handle_cpltimeout_work(struct work_struct *work)
{
	/*lint -e826 -esym(826,*)*/
	struct kirin_pcie *pcie = container_of(work, struct kirin_pcie, handle_cpltimeout_work);
	/*lint -e826 +esym(826,*)*/

	dsm_pcie_dump_info(pcie, DSM_ERR_CPL_TIMEOUT);

	dump_apb_register(pcie);

	kirin_pcie_notify_callback(pcie, KIRIN_PCIE_EVENT_CPL_TIMEOUT);
}

#ifdef CONFIG_KIRIN_PCIE_CPLTIMEOUT_INT
static irqreturn_t kirin_pcie_cpltimeout_irq_handler(int irq, void *arg)
{
	struct kirin_pcie *pcie = arg;

	PCIE_PR_ERR("Triggle CPL timeout irq[%d]", irq);
	/* clear interrupt*/
	kirin_elb_writel(pcie, CLR_COMP_TIMEOUT_BIT, SOC_PCIECTRL_CTRL11_ADDR);

	atomic_set(&(pcie->is_enumerated), 0);
	schedule_work(&pcie->handle_cpltimeout_work);

	return IRQ_HANDLED;
}
#endif

#ifdef CONFIG_KIRIN_PCIE_TEST
static irqreturn_t kirin_pcie_intx_irq_handler(int irq, void *arg)
{
	PCIE_PR_ERR("Triggle intx irq[%d]", irq);
	return IRQ_HANDLED;
}
#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 69))
u32 kirin_pcie_readl_rc(struct pcie_port *pp,
				u32 reg)
{
	struct kirin_pcie *pcie = to_kirin_pcie(pp);
	u32 val = 0xFFFFFFFF;

	if (!atomic_read(&(pcie->is_power_on))) {
		PCIE_PR_ERR("PCIe is power off");
		return val;
	}

	kirin_pcie_sideband_dbi_r_mode(pcie, true);
	val = readl(pp->dbi_base + reg);
	kirin_pcie_sideband_dbi_r_mode(pcie, false);
	return val;
}

void kirin_pcie_writel_rc(struct pcie_port *pp,
				u32 reg, u32 val)
{
	struct kirin_pcie *pcie = to_kirin_pcie(pp);
	if (!atomic_read(&(pcie->is_power_on))) {
		PCIE_PR_ERR("PCIe is power off");
		return;
	}

	kirin_pcie_sideband_dbi_w_mode(pcie, true);
	writel(val, pp->dbi_base + reg);
	kirin_pcie_sideband_dbi_w_mode(pcie, false);
}
#else
u32	kirin_pcie_read_dbi(struct dw_pcie *pci, void __iomem *base, u32 reg,
			    size_t size)
{
	struct kirin_pcie *pcie = to_kirin_pcie(pci);
	u32 val = 0xFFFFFFFF;

	if (!atomic_read(&(pcie->is_power_on))) {
		PCIE_PR_ERR("PCIe is power off");
		return val;
	}

	kirin_pcie_sideband_dbi_r_mode(pcie, true);
	dw_pcie_read(base + reg, size, &val);
	kirin_pcie_sideband_dbi_r_mode(pcie, false);
	return val;
}

void kirin_pcie_write_dbi(struct dw_pcie *pci, void __iomem *base, u32 reg,
		     size_t size, u32 val)
{
	struct kirin_pcie *pcie = to_kirin_pcie(pci);

	if (!atomic_read(&(pcie->is_power_on))) {
		PCIE_PR_ERR("PCIe is power off");
		return;
	}

	kirin_pcie_sideband_dbi_w_mode(pcie, true);
	dw_pcie_write(base + reg, size, val);
	kirin_pcie_sideband_dbi_w_mode(pcie, false);
}
#endif

int kirin_pcie_rd_own_conf(struct pcie_port *pp, int where, int size,
				u32 *val)
{
	int ret;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 69))
	struct kirin_pcie *pcie = to_kirin_pcie(pp);
#else
	struct dw_pcie *pci = to_dw_pcie_from_pp(pp);
	struct kirin_pcie *pcie = to_kirin_pcie(pci);
#endif
	if (!atomic_read(&(pcie->is_power_on)))
		return -EINVAL;

	kirin_pcie_sideband_dbi_r_mode(pcie, true);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 69))
	ret = dw_pcie_cfg_read(pp->dbi_base + where, size, val);
#else
	ret = dw_pcie_read(pcie->pci->dbi_base + where, size, val);
#endif
	kirin_pcie_sideband_dbi_r_mode(pcie, false);
	return ret;
}

int kirin_pcie_wr_own_conf(struct pcie_port *pp, int where, int size,
				u32 val)
{
	int ret;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 69))
	struct kirin_pcie *pcie = to_kirin_pcie(pp);
#else
	struct dw_pcie *pci = to_dw_pcie_from_pp(pp);
	struct kirin_pcie *pcie = to_kirin_pcie(pci);
#endif

	if (!atomic_read(&(pcie->is_power_on)))
		return -EINVAL;

	kirin_pcie_sideband_dbi_w_mode(pcie, true);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 69))
	ret = dw_pcie_cfg_write(pp->dbi_base + where, size, val);
#else
	ret = dw_pcie_write(pcie->pci->dbi_base + where, size, val);
#endif
	kirin_pcie_sideband_dbi_w_mode(pcie, false);
	return ret;
}

static int kirin_pcie_host_init(struct pcie_port *pp)
{
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 69))
	struct kirin_pcie *pcie = to_kirin_pcie(pp);
#else
	struct dw_pcie *pci = to_dw_pcie_from_pp(pp);
	struct kirin_pcie *pcie = to_kirin_pcie(pci);
#endif

	if (kirin_pcie_establish_link(pcie))
		return -1;

	if (IS_ENABLED(CONFIG_PCI_MSI))
		dw_pcie_msi_init(pp);

	return 0;
}


#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 69))
static struct pcie_host_ops kirin_pcie_host_ops = {
	.readl_rc = kirin_pcie_readl_rc,
	.writel_rc = kirin_pcie_writel_rc,
	.link_up = kirin_pcie_link_up,
#else
static struct dw_pcie_host_ops kirin_pcie_host_ops = {
#endif
	.rd_own_conf = kirin_pcie_rd_own_conf,
	.wr_own_conf = kirin_pcie_wr_own_conf,
	.host_init = kirin_pcie_host_init,
};

static int __init kirin_add_pcie_port(struct kirin_pcie *pcie,
					   struct platform_device *pdev)
{
	int ret;
	int index;
	struct pcie_port *pp;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 69))
	pp = &pcie->pp;
#else
	pp = &(pcie->pci->pp);
#endif

	PCIE_PR_INFO("++");
	for (index = 0; index < MAX_IRQ_NUM; index++) {
		pcie->irq[index].num = platform_get_irq(pdev, index);
		if (!pcie->irq[index].num) {
			PCIE_PR_ERR("Failed to get [%s] irq ,num = [%d]", pcie->irq[index].name,
				pcie->irq[index].num);
			return -ENODEV;
		}
	PCIE_PR_INFO("Succeed to get [%s] irq ,num = [%d]", pcie->irq[index].name,
			pcie->irq[index].num);
	}
#ifdef CONFIG_KIRIN_PCIE_TEST
	ret = devm_request_irq(&pdev->dev, (unsigned int)pcie->irq[IRQ_INTC].num,
				kirin_pcie_intx_irq_handler,
				(unsigned long)IRQF_TRIGGER_RISING, pcie->irq[IRQ_INTC].name, pp);
	if (ret) {
		PCIE_PR_ERR("Failed to request %s irq", pcie->irq[IRQ_INTC].name);
		return ret;
	}

	ret = devm_request_irq(&pdev->dev, (unsigned int)pcie->irq[IRQ_INTD].num,
				kirin_pcie_intx_irq_handler,
				(unsigned long)IRQF_TRIGGER_RISING, pcie->irq[IRQ_INTD].name, pp);
	if (ret) {
		PCIE_PR_ERR("Failed to request %s irq", pcie->irq[IRQ_INTD].name);
		return ret;
	}
#endif

#ifdef CONFIG_KIRIN_PCIE_CPLTIMEOUT_INT
	pcie->irq[IRQ_CPLTIMEOUT].num = platform_get_irq(pdev, IRQ_CPLTIMEOUT);
	if (!pcie->irq[IRQ_CPLTIMEOUT].num) {
		PCIE_PR_ERR("Failed to get [%s] irq ,num = [%d]", pcie->irq[IRQ_CPLTIMEOUT].name,
			pcie->irq[IRQ_CPLTIMEOUT].num);
		return -ENODEV;
	}
	PCIE_PR_INFO("Completion timeout interrupt number is %d", pcie->irq[IRQ_CPLTIMEOUT].num);

	ret = devm_request_irq(&pdev->dev, (unsigned int)pcie->irq[IRQ_CPLTIMEOUT].num,
				kirin_pcie_cpltimeout_irq_handler,
				(unsigned long)IRQF_TRIGGER_RISING, pcie->irq[IRQ_CPLTIMEOUT].name, pcie);
	if (ret) {
		PCIE_PR_ERR("Failed to request cpltimeout irq");
		return ret;
	}

	PCIE_PR_INFO("Request completion timeout interrupt done!");
#endif

	ret = devm_request_irq(&pdev->dev, pcie->irq[IRQ_LINKDOWN].num,
				kirin_pcie_linkdown_irq_handler,
				IRQF_TRIGGER_RISING, pcie->irq[IRQ_LINKDOWN].name, pcie);
	if (ret) {
		PCIE_PR_ERR("Failed to request linkdown irq");
		return ret;
	}

	PCIE_PR_INFO("pcie->irq[%d].name = [%s], pcie->irq[%d].name = [%s]", IRQ_LINKDOWN,
		pcie->irq[IRQ_LINKDOWN].name, IRQ_LINKDOWN, pcie->irq[IRQ_LINKDOWN].name);

	if (IS_ENABLED(CONFIG_PCI_MSI)) {
		pp->msi_irq = pcie->irq[IRQ_MSI].num;
		ret = devm_request_irq(&pdev->dev, pp->msi_irq,
					kirin_pcie_msi_irq_handler,
					IRQF_SHARED | IRQF_NO_THREAD,
					pcie->irq[IRQ_MSI].name, pcie);
		if (ret) {
			PCIE_PR_ERR("Failed to request msi irq");
			return ret;
		}
	}

	PCIE_PR_INFO("--");
	return 0;
}

static int kirin_pcie_pwron_default(void *data)
{
	struct kirin_pcie *pcie = (struct kirin_pcie *)data;
	return kirin_pcie_perst_cfg(pcie, ENABLE);
}

static int kirin_pcie_pwroff_default(void *data)
{
	struct kirin_pcie *pcie = (struct kirin_pcie *)data;
	return kirin_pcie_perst_cfg(pcie, DISABLE);
}
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 69))
static const struct dw_pcie_ops dw_pcie_ops = {
	.read_dbi = kirin_pcie_read_dbi,
	.write_dbi = kirin_pcie_write_dbi,
	.link_up = kirin_pcie_link_up,
};
#endif

int kirin_pcie_get_pcie(struct kirin_pcie **pcie, struct platform_device *pdev)
{
	u32 rc_id;

	if (!pdev->dev.of_node) {
		PCIE_PR_ERR("Of_node is null");
		return -EINVAL;
	}

	if (of_property_read_u32(pdev->dev.of_node, "rc-id", &rc_id)) {
		PCIE_PR_ERR("Failed to get rc_id info");
		return -EINVAL;
	}

	if (get_rc_num()) {
		PCIE_PR_ERR("Failed to get rc_num");
		return -EINVAL;
	}

	if (rc_id >= g_rc_num) {
		PCIE_PR_ERR("There is no rc_id = %d", rc_id);
		return -EINVAL;
	}

	*pcie = &g_kirin_pcie[rc_id];

	(*pcie)->rc_id = rc_id;

	return 0;
}

static int kirin_pcie_probe(struct platform_device *pdev)
{
	struct kirin_pcie *pcie;
	struct pcie_port *pp;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 69))
	struct dw_pcie *pci;
#endif
	int ret = 0;

	struct kirin_pcie_dtsinfo *dtsinfo;

	PCIE_PR_INFO("++");

	ret = kirin_pcie_get_pcie(&pcie, pdev);
	if (ret) {
		PCIE_PR_ERR("Failed to get kirin pcie from dts");
		return ret;
	}

	PCIE_PR_INFO("PCIe No.%d probe", pcie->rc_id);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 69))
	pci = devm_kzalloc(&pdev->dev, sizeof(*pci), GFP_KERNEL);
	if (!pci)
		return -ENOMEM;
	pcie->pci = pci;
#endif

	if (kirin_pcie_get_dtsinfo(pcie, pdev)) {
		PCIE_PR_ERR("Failed to get dts info");
		ret = -EINVAL;
		goto FAIL;
	}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 69))
	pp = &pcie->pp;
	pp->dev = &(pdev->dev);
#else
	pp = &(pcie->pci->pp);
	pcie->pci->dev = &(pdev->dev);
	pcie->pci->ops = &dw_pcie_ops;
#endif

	dtsinfo = &pcie->dtsinfo;

	ret = kirin_pcie_power_notifiy_register(pcie->rc_id, kirin_pcie_pwron_default,
					kirin_pcie_pwroff_default, (void *)pcie);
	if (ret)
		PCIE_PR_INFO("Failed to register default pwr_callback_funcs");

	ret = gpio_request((unsigned int)pcie->gpio_id_reset, "pcie_reset");
	if (ret) {
		PCIE_PR_ERR("Failed to request gpio-%d", pcie->gpio_id_reset);
		goto FAIL;
	}

	pp->ops = &kirin_pcie_host_ops;

	INIT_WORK(&pcie->handle_work, kirin_handle_work);
	INIT_WORK(&pcie->handle_cpltimeout_work, kirin_handle_cpltimeout_work);

	platform_set_drvdata(pdev, pcie);

	ret = pcie_plat_init(pdev, pcie);
	if (ret) {
		PCIE_PR_ERR("Failed to get platform info");
		goto FAIL;
	}

	ret = kirin_add_pcie_port(pcie, pdev);
	if (ret < 0) {
		PCIE_PR_ERR("Failed to assign resource, ret=[%d]", ret);
		goto FAIL;
	}

	atomic_set(&(pcie->is_ready), 1);
	spin_lock_init(&pcie->ep_ltssm_lock);
	mutex_init(&pcie->power_lock);
	mutex_init(&pcie->pm_lock);

	(void)kirin_pcie_debugfs_init(pcie);

	PCIE_PR_INFO("--");
	return 0;
FAIL:
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 69))
	devm_kfree(&pdev->dev, pcie->pci);
#endif
	return ret;
}

static int kirin_pcie_save_rc_cfg(struct kirin_pcie *pcie)
{
	int ret;
	u32 val = 0;
	int aer_pos;
	struct pcie_port *pp;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 69))
	pp = &(pcie->pp);
#else
	pp = &(pcie->pci->pp);
#endif

	kirin_pcie_rd_own_conf(pp, PORT_MSI_CTRL_ADDR, 4, &val);
	pcie->msi_controller_config[0] = val;
	kirin_pcie_rd_own_conf(pp, PORT_MSI_CTRL_UPPER_ADDR, 4, &val);
	pcie->msi_controller_config[1] = val;
	kirin_pcie_rd_own_conf(pp, PORT_MSI_CTRL_INT0_ENABLE, 4, &val);
	pcie->msi_controller_config[2] = val;

	aer_pos = pci_find_ext_capability(pcie->rc_dev, PCI_EXT_CAP_ID_ERR);
	if (!aer_pos ) {
		PCIE_PR_ERR("Failed to get RC PCI_EXT_CAP_ID_ERR");
		return -1;
	}

	pci_read_config_dword(pcie->rc_dev, aer_pos + PCI_ERR_ROOT_COMMAND,
		&pcie->aer_config);

	ret = pci_save_state(pcie->rc_dev);
	if (ret) {
		PCIE_PR_ERR("Failed to save state of RC.");
		return -1;
	}
	pcie->rc_saved_state = pci_store_saved_state(pcie->rc_dev);

	return 0;
}

static int kirin_pcie_restore_rc_cfg(struct kirin_pcie *pcie)
{
	struct pcie_port *pp;
	int aer_pos;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 69))
	pp = &(pcie->pp);
#else
	pp = &(pcie->pci->pp);
#endif

	if (!pcie->rc_dev) {
		PCIE_PR_ERR("Failed to get RC dev");
		return -1;
	}

	kirin_pcie_wr_own_conf(pp, PORT_MSI_CTRL_ADDR,
			4, pcie->msi_controller_config[0]);
	kirin_pcie_wr_own_conf(pp, PORT_MSI_CTRL_UPPER_ADDR,
			4, pcie->msi_controller_config[1]);
	kirin_pcie_wr_own_conf(pp, PORT_MSI_CTRL_INT0_ENABLE,
			4, pcie->msi_controller_config[2]);

	aer_pos = pci_find_ext_capability(pcie->rc_dev, PCI_EXT_CAP_ID_ERR);
	if (!aer_pos ) {
		PCIE_PR_ERR("Failed to get RC PCI_EXT_CAP_ID_ERR");
		return -1;
	}

	pci_write_config_dword(pcie->rc_dev, aer_pos + PCI_ERR_ROOT_COMMAND,
		pcie->aer_config);

	pci_load_saved_state(pcie->rc_dev, pcie->rc_saved_state);
	pci_restore_state(pcie->rc_dev);
	pci_load_saved_state(pcie->rc_dev, pcie->rc_saved_state);

	return 0;
}

static void send_pme_turn_off_msg(struct kirin_pcie *pcie)
{

#if defined(CONFIG_KIRIN_PCIE_JAN)
	u32 val;
	val = kirin_elb_readl(pcie, SOC_PCIECTRL_CTRL7_ADDR);
	val |= PME_TURN_OFF_BIT;
	kirin_elb_writel(pcie, val, SOC_PCIECTRL_CTRL7_ADDR);
#else
	u32 iatu_offset = pcie->dtsinfo.iatu_base_offset;
	kirin_pcie_generate_msg(pcie->rc_id, PCIE_ATU_REGION_INDEX0, iatu_offset,
				MSG_TYPE_ROUTE_BROADCAST, MSG_CODE_PME_TURN_OFF);
#endif
}

static int kirin_pcie_shutdown_prepare(struct pci_dev *dev)
{
	u32 val;
	u32 pm;
	int index = 0;
	struct pcie_port *pp;
	struct kirin_pcie *pcie;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 69))
	struct dw_pcie *pci;
#endif

	PCIE_PR_INFO("++");

	if (!dev) {
		PCIE_PR_ERR("pci_dev is null");
		return -1;
	}
	pp = dev->sysdata;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 69))
	pcie = to_kirin_pcie(pp);
#else
	pci = to_dw_pcie_from_pp(pp);
	pcie = to_kirin_pcie(pci);
#endif

	/*Enable PME*/
	pm = pci_find_capability(dev, PCI_CAP_ID_PM);
	if (!pm) {
		PCIE_PR_ERR("Failed to get PCI_CAP_ID_PM");
		return -1;
	}
	kirin_pcie_rd_own_conf(pp, pm + PCI_PM_CTRL, 4, &val);
	val |= 0x100;
	kirin_pcie_wr_own_conf(pp, pm + PCI_PM_CTRL, 4, val);

	send_pme_turn_off_msg(pcie);

	do {
		val = kirin_elb_readl(pcie, SOC_PCIECTRL_STATE1_ADDR);
		val = val & PME_ACK_BIT;
		if (index >= 1000) {
			PCIE_PR_ERR("Failed to get PME_TO_ACK");
			return -1;
		}
		index++;
		udelay((unsigned long)10);
	} while (val != PME_ACK_BIT);

	PCIE_PR_INFO("Get PME ACK ");

	PCIE_PR_INFO("--");
	return 0;
}

static void kirin_pcie_shutdown(struct platform_device *pdev)
{
	struct kirin_pcie *pcie;

	PCIE_PR_INFO("++");

	pcie = dev_get_drvdata(&(pdev->dev));
	if (pcie == NULL) {
		PCIE_PR_ERR("Failed to get drvdata");
		return;
	}

	if (atomic_read(&(pcie->is_power_on))) {
		if (kirin_pcie_power_ctrl(pcie, RC_POWER_OFF)) {
			PCIE_PR_ERR("Failed to power off");
			return;
		}
	}

	PCIE_PR_INFO("--");
}

#ifdef CONFIG_PM
static int kirin_pcie_resume_noirq(struct device *dev)
{
	struct kirin_pcie *pcie;

	PCIE_PR_INFO("++");

	pcie = dev_get_drvdata(dev);
	if (!pcie) {
		PCIE_PR_ERR("Failed to get drvdata");
		return -EINVAL;
	}

	if (pcie->dtsinfo.ep_device_type == EP_DEVICE_MODEM)
		return 0;

	if (atomic_read(&(pcie->is_enumerated)) && (!atomic_read(&(pcie->usr_suspend)))) {
		if (kirin_pcie_power_ctrl(pcie, RC_POWER_RESUME)) {
			PCIE_PR_ERR("Failed to power on ");
			goto FAIL;
		}

		if (kirin_pcie_establish_link(pcie)) {
			PCIE_PR_ERR("Failed to link up ");
			goto FAIL;
		}

		PCIE_PR_DEBUG("Begin to recover RC cfg ");
		if (pcie->rc_dev)
			kirin_pcie_restore_rc_cfg(pcie);
	}

	PCIE_PR_INFO("--");
	return 0;

FAIL:
	schedule_work(&pcie->handle_work);

	return -EINVAL;
}


static int kirin_pcie_suspend_noirq(struct device *dev)
{
	struct kirin_pcie *pcie;

	PCIE_PR_INFO("++");

	pcie = dev_get_drvdata(dev);
	if (pcie == NULL) {
		PCIE_PR_ERR("Failed to get drvdata");
		return -EINVAL;
	}

	if (pcie->dtsinfo.ep_device_type == EP_DEVICE_MODEM)
		return 0;

	if (atomic_read(&(pcie->is_power_on))) {
		if (!atomic_read(&(pcie->usr_suspend))) {
			kirin_pcie_lp_ctrl(pcie->rc_id, DISABLE);
			kirin_pcie_shutdown_prepare(pcie->rc_dev);
		}
		if (kirin_pcie_power_ctrl(pcie, RC_POWER_SUSPEND)) {
			PCIE_PR_ERR("Failed to power off ");
			return -EINVAL;
		}
	}

	PCIE_PR_INFO("--");

	return 0;
}

#else

#define kirin_pcie_suspend_noirq NULL
#define kirin_pcie_resume_noirq NULL

#endif

static const struct dev_pm_ops kirin_pcie_dev_pm_ops = {
	.suspend_noirq	= kirin_pcie_suspend_noirq,
	.resume_noirq	= kirin_pcie_resume_noirq,
};

static const struct of_device_id kirin_pcie_match_table[] = {
	{
		.compatible = "hisilicon,kirin-pcie",
		.data = NULL,
	},
	/*lint -e785 -esym(785,*)*/
	{},
	/*lint -e785 +esym(785,*)*/
};

static struct platform_driver kirin_pcie_driver = {
	.probe			= kirin_pcie_probe,
	.shutdown		= kirin_pcie_shutdown,
	.driver			= {
		.name			= "Kirin-pcie",
		.pm				= &kirin_pcie_dev_pm_ops,
		.of_match_table = kirin_pcie_match_table,
		.suppress_bind_attrs = true
	},
};

static int kirin_pcie_usr_suspend(u32 rc_idx, int power_off_ops)
{
	int ret;
	u32 val;
	struct pci_dev *rc_dev;
	struct kirin_pcie *pcie = &g_kirin_pcie[rc_idx];

	PCIE_PR_INFO("++");

	if (atomic_read(&(pcie->usr_suspend)) || !atomic_read(&(pcie->is_power_on))) {
		PCIE_PR_ERR("Already suspend by EP ");
		return -EINVAL;
	}

	rc_dev = pcie->rc_dev;

	if (!rc_dev) {
		PCIE_PR_ERR("Failed to get RC dev");
		return -1;
	}

	if (power_off_ops != POWEROFF_BUSDOWN) {
		kirin_pcie_lp_ctrl(rc_idx, DISABLE);
		ret = kirin_pcie_shutdown_prepare(rc_dev);
		if (ret)
			PCIE_PR_ERR("device do not reply ACK");
	}
	/*phy rst from sys to pipe */
	val = kirin_apb_phy_readl(pcie, SOC_PCIEPHY_CTRL1_ADDR);
	val |= 0x1 << 17;
	kirin_apb_phy_writel(pcie, val, SOC_PCIEPHY_CTRL1_ADDR);

	ret = kirin_pcie_power_ctrl(pcie, RC_POWER_OFF);
	if (ret) {
		PCIE_PR_ERR("Failed to power off ");
		return -EINVAL;
	}

	atomic_set(&(pcie->usr_suspend), 1);

	PCIE_PR_INFO("--");
	return 0;
}

static int kirin_pcie_usr_resume(u32 rc_idx)
{
	int ret;
	struct kirin_pcie_dtsinfo *dtsinfo;
	struct kirin_pcie *pcie = &g_kirin_pcie[rc_idx];

	PCIE_PR_INFO("++");

	dtsinfo = &pcie->dtsinfo;

	atomic_set(&(pcie->usr_suspend), 0);

	ret = kirin_pcie_power_ctrl(pcie, RC_POWER_ON);
	if (ret) {
		PCIE_PR_ERR("Failed to power on");

		atomic_set(&(pcie->usr_suspend), 1);
		return -EINVAL;
	}

	ret = kirin_pcie_establish_link(pcie);
	if (ret) {
		if (kirin_pcie_power_ctrl(pcie, RC_POWER_OFF))
			PCIE_PR_ERR("Failed to power off");

		atomic_set(&(pcie->usr_suspend), 1);
		return -EINVAL;
	}

	kirin_pcie_restore_rc_cfg(pcie);

	kirin_pcie_lp_ctrl(rc_idx, ENABLE);

	PCIE_PR_INFO("--");

	return 0;
}

/*
* EP Power ON/OFF callback Function:
* param: rc_idx---which rc the EP link with
* power_ops: 0---PowerOff normally
*            1---Poweron normally
*            2---PowerOFF without PME
*            3---Poweron without LINK
*/
int kirin_pcie_pm_control(int power_ops, u32 rc_idx)
{
	int ret = 0;
	struct kirin_pcie *pcie;

	PCIE_PR_INFO("RC = [%u], power_ops[%d]", rc_idx, power_ops);

	if (rc_idx >= g_rc_num) {
		PCIE_PR_ERR("There is no rc_id = %d", rc_idx);
		return -EINVAL;
	}

	pcie = &g_kirin_pcie[rc_idx];

	if (!atomic_read(&(g_kirin_pcie[rc_idx].is_ready))) {
		PCIE_PR_ERR("PCIe driver is not ready");
		return -1;
	}

	mutex_lock(&(g_kirin_pcie[rc_idx].pm_lock));

	switch (power_ops) {
	case POWERON:
		dsm_pcie_clear_info();
		ret = kirin_pcie_usr_resume(rc_idx);
		break;

	case POWEROFF_BUSON:
	case POWEROFF_BUSDOWN:
		ret = kirin_pcie_usr_suspend(rc_idx, power_ops);
		break;

	case POWERON_CLK:
		ret = pcie->plat_ops->plat_on(pcie, RC_POWER_ON);
		break;

	default:
		PCIE_PR_ERR("Invalid power_ops[%d]", power_ops);
		ret = -EINVAL;
		break;
	}

	mutex_unlock(&(g_kirin_pcie[rc_idx].pm_lock));
	return ret;
}
EXPORT_SYMBOL_GPL(kirin_pcie_pm_control);

static void kirin_pcie_aspm_enable(struct kirin_pcie *pcie)
{
#ifdef CONFIG_KIRIN_PCIE_TEST
	if (pcie->aspm_ctrl_debug_flag) {
		kirin_pcie_aspm_special_state(pcie);
		return;
	}
#endif

	kirin_pcie_config_l0sl1(pcie->rc_id,
		(enum link_aspm_state)pcie->dtsinfo.aspm_state);
	if (pcie->dtsinfo.ep_device_type != EP_DEVICE_MODEM)
		kirin_pcie_config_l1ss(pcie->rc_id, L1SS_PM_ASPM_ALL);
}

#define ASPM_STATE_L0S_L1		(7)

/*
* API FOR EP to control L1&L1-substate
* param: rc_idx---which rc the EP link with
* enable: KIRIN_PCIE_LP_ON---enable L1 and L1-substate,
*         KIRIN_PCIE_LP_Off---disable,
*         others---illegal
*/
int kirin_pcie_lp_ctrl(u32 rc_idx, u32 enable)
{
#ifdef CONFIG_PCIEASPM_POWER_SUPERSAVE
	u32 val;
#endif
	struct kirin_pcie *pcie;
	struct kirin_pcie_dtsinfo *dtsinfo;

	PCIE_PR_INFO("+%s+", enable == ENABLE? "enable":"disable");

	if (rc_idx >= g_rc_num) {
		PCIE_PR_ERR("There is no rc_id = %d", rc_idx);
		return -EINVAL;
	}

	pcie = &g_kirin_pcie[rc_idx];

	if (!atomic_read(&(pcie->is_ready))) {
		PCIE_PR_ERR("PCIe driver is not ready");
		return -EINVAL;
	}

	if (!atomic_read(&(pcie->is_power_on))) {
		PCIE_PR_ERR("PCIe%d is power off ", rc_idx);
		return -EINVAL;
	}

	dtsinfo = &(pcie->dtsinfo);

#ifndef CONFIG_PCIEASPM_POWER_SUPERSAVE
	if (enable) {
		if (pcie->dtsinfo.board_type == BOARD_ASIC)
			kirin_pcie_aspm_enable(pcie);
	} else {
		kirin_pcie_config_l1ss(pcie->rc_id, L1SS_CLOSE);
		kirin_pcie_config_l0sl1(pcie->rc_id, ASPM_CLOSE);
	}
#else
	if (enable) {
		enable_req_clk(pcie, DISABLE);

		pci_enable_link_state(pcie->rc_dev, ASPM_STATE_L0S_L1);

		pcie_capability_read_dword(pcie->rc_dev, PCI_EXP_DEVCTL2, &val);
		val |= (0x1 << 10);
		pcie_capability_write_dword(pcie->rc_dev, PCI_EXP_DEVCTL2, val);

		pcie_capability_read_dword(pcie->ep_dev, PCI_EXP_DEVCTL2, &val);
		val |= (0x1 << 10);
		pcie_capability_write_dword(pcie->ep_dev, PCI_EXP_DEVCTL2, val);
	} else {
		pci_disable_link_state(pcie->rc_dev, ASPM_STATE_L0S_L1);
		enable_req_clk(pcie, ENABLE);
	}
#endif

	PCIE_PR_INFO("-%s-", enable == ENABLE? "enable":"disable");

	return 0;
}
EXPORT_SYMBOL_GPL(kirin_pcie_lp_ctrl);

static int kirin_pcie_reenumerate(u32 rc_idx)
{
	int ret = 0;

	ret = kirin_pcie_pm_control(POWERON, rc_idx);
	if (!ret)
		ret = kirin_pcie_rescan_ep(rc_idx);

	return ret;
}

/*
* Enumerate Function:
* param: rc_idx---which rc the EP link with
*/
int kirin_pcie_enumerate(u32 rc_idx)
{
	int ret;
	u32 val;
	u32 dev_id;
	u32 vendor_id;
	struct pcie_port *pp;
	struct pci_dev *dev;
	struct kirin_pcie *pcie;

	PCIE_PR_INFO("+RC[%u]+", rc_idx);

	if (rc_idx >= g_rc_num) {
		PCIE_PR_ERR("There is no rc_id = %d", rc_idx);
		return -EINVAL;
	}
	pcie = &g_kirin_pcie[rc_idx];
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 69))
	pp = &(pcie->pp);
#else
	pp = &(pcie->pci->pp);
#endif

	if (!atomic_read(&(pcie->is_ready))) {
		PCIE_PR_ERR("PCIe driver is not ready");
		return -1;
	}

	if (atomic_read(&(pcie->is_enumerated))) {
		if (pcie->dtsinfo.ep_device_type == EP_DEVICE_MODEM) {
			return kirin_pcie_reenumerate(rc_idx);
		} else {
			PCIE_PR_ERR("Enumeration was done successed before");
			return 0;
		}
	}

	/*clk on*/
	ret = kirin_pcie_power_ctrl(pcie, RC_POWER_ON);
	if (ret) {
		PCIE_PR_ERR("Failed to power RC");
		dsm_pcie_dump_info(pcie, DSM_ERR_POWER_ON);
		return ret;
	}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 69))
	val = kirin_pcie_readl_rc(pp, 0);
#else
	val = kirin_pcie_read_dbi(pcie->pci, pcie->pci->dbi_base, 0, 4);
#endif

	val += rc_idx;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 69))
	kirin_pcie_writel_rc(pp, 0, val);
#else
	kirin_pcie_write_dbi(pcie->pci, pcie->pci->dbi_base, 0, 4, val);
#endif

	ret = dw_pcie_host_init(pp);
	if (ret) {
		PCIE_PR_ERR("Failed to initialize host");
		dsm_pcie_dump_info(pcie, DSM_ERR_ENUMERATE);
		goto FAIL_TO_POWEROFF;
	}

	kirin_pcie_rd_own_conf(pp, PCI_VENDOR_ID, 2, &vendor_id);
	kirin_pcie_rd_own_conf(pp, PCI_DEVICE_ID, 2, &dev_id);
	pcie->rc_dev = pci_get_device(vendor_id, dev_id, pcie->rc_dev);
	if (!pcie->rc_dev) {
		PCIE_PR_ERR("Failed to get RC device");
		goto FAIL_TO_POWEROFF;
	}

	ret = kirin_pcie_save_rc_cfg(pcie);
	if (ret)
		goto FAIL_TO_POWEROFF;

	if (pcie->rc_dev->subordinate) {
		list_for_each_entry(dev, &pcie->rc_dev->subordinate->devices, bus_list) {
			if (pci_is_pcie(dev)) {
				pcie->ep_dev = dev;
				atomic_set(&(pcie->is_enumerated), 1);
			} else {
				PCIE_PR_ERR("No PCIe EP found!");
				pci_stop_and_remove_bus_device(pcie->rc_dev);
				goto FAIL_TO_POWEROFF;
			}
		}
	} else {
		PCIE_PR_ERR("Bus1 is null");
		pcie->ep_dev = NULL;
		pci_stop_and_remove_bus_device(pcie->rc_dev);
		goto FAIL_TO_POWEROFF;
	}

	kirin_pcie_lp_ctrl(pcie->rc_id, ENABLE);

	atomic_set(&(pcie->usr_suspend), 0);

	PCIE_PR_INFO("-RC[%u]-", rc_idx);
	return 0;

FAIL_TO_POWEROFF:
	if (kirin_pcie_power_ctrl(pcie, RC_POWER_OFF)) {
			PCIE_PR_ERR("Failed to power off.");
	}
	return -1;
}
EXPORT_SYMBOL(kirin_pcie_enumerate);

/*
* Remove EP Function:
* param: rc_idx---which rc the EP link with
*/
int kirin_pcie_remove_ep(u32 rc_idx)
{
	struct kirin_pcie *pcie;
	struct pci_dev *dev, *temp;

	PCIE_PR_INFO("+RC[%u]+", rc_idx);

	if (rc_idx >= g_rc_num) {
		PCIE_PR_ERR("There is no rc_id = %d", rc_idx);
		return -EINVAL;
	}
	pcie = &g_kirin_pcie[rc_idx];

	if (!atomic_read(&(pcie->is_ready))) {
		PCIE_PR_ERR("PCIe driver is not ready");
		return ERROR;
	}

	if (!atomic_read(&(pcie->is_enumerated))) {
		PCIE_PR_ERR("Enumeration was not done");
		return ERROR;
	}

	if (atomic_read(&(pcie->is_removed))) {
		PCIE_PR_ERR("Remove was done before");
		return OK;
	}

	(void)kirin_pcie_lp_ctrl(pcie->rc_id, DISABLE);
	list_for_each_entry_safe(dev, temp, &pcie->rc_dev->subordinate->devices, bus_list) {
		if (pci_is_pcie(dev)) {
			pci_stop_and_remove_bus_device_locked(dev);
			if (pcie->ep_dev)
				pcie->ep_dev = NULL;
		} else {
			PCIE_PR_ERR("No PCIe EP found!");
			return ERROR;
		}
	}

	(void)atomic_inc_return(&(pcie->is_removed));

	PCIE_PR_INFO("-RC[%u]-", rc_idx);
	return OK;
}
EXPORT_SYMBOL(kirin_pcie_remove_ep);

/*
* Rescan EP Function:
* param: rc_idx---which rc the EP link with
*/
int kirin_pcie_rescan_ep(u32 rc_idx)
{
	struct kirin_pcie *pcie;
	struct pci_dev *dev, *temp;
	u32 max = 0;

	PCIE_PR_INFO("+RC[%u]+", rc_idx);

	if (rc_idx >= g_rc_num) {
		PCIE_PR_ERR("There is no rc_id = %d", rc_idx);
		return -EINVAL;
	}
	pcie = &g_kirin_pcie[rc_idx];

	if (!atomic_read(&(pcie->is_ready))) {
		PCIE_PR_ERR("PCIe driver is not ready");
		return ERROR;
	}

	if (!atomic_read(&(pcie->is_enumerated))) {
		PCIE_PR_ERR("Enumeration was not done");
		return ERROR;
	}

	if (!atomic_read(&(pcie->is_removed))) {
		PCIE_PR_ERR("Rescan was done before or Remove was not done");
		return OK;
	}

	pci_lock_rescan_remove();
	max = pci_rescan_bus_bridge_resize(pcie->rc_dev);
	pci_unlock_rescan_remove();
	if (max) {
		if (kirin_pcie_save_rc_cfg(pcie))
			return ERROR;

		list_for_each_entry_safe(dev, temp, &pcie->rc_dev->subordinate->devices, bus_list) {
			if (pci_is_pcie(dev)) {
				if (!pcie->ep_dev)
					pcie->ep_dev = dev;
			} else {
				PCIE_PR_ERR("No PCIe EP found!");
				return ERROR;
			}
		}
	} else {
		PCIE_PR_ERR("Bus1 is null");
		pcie->ep_dev = NULL;
		return ERROR;
	}

	(void)kirin_pcie_lp_ctrl(pcie->rc_id, ENABLE);
	(void)atomic_dec_return(&(pcie->is_removed));

	PCIE_PR_INFO("-RC[%u]-", rc_idx);
	return OK;
}
EXPORT_SYMBOL(kirin_pcie_rescan_ep);

int pcie_ep_link_ltssm_notify(u32 rc_id, u32 link_status)
{
	struct kirin_pcie *pcie;

	if (rc_id >= g_rc_num) {
		PCIE_PR_ERR("There is no rc_id = %d", rc_id);
		return -EINVAL;
	}

	if (link_status >= DEVICE_LINK_MAX || link_status <= DEVICE_LINK_MIN) {
		PCIE_PR_ERR("Invalid Device link status[%d]", link_status);
		return -EINVAL;
	}

	pcie = &g_kirin_pcie[rc_id];

	if (!atomic_read(&pcie->is_power_on)) {
		PCIE_PR_ERR("PCIe is Poweroff");
		return -EINVAL;
	}

	pcie->ep_link_status = link_status;

	return 0;
}
EXPORT_SYMBOL(pcie_ep_link_ltssm_notify);

int kirin_pcie_power_notifiy_register(u32 rc_id, int (*poweron)(void* data),
				int (*poweroff)(void* data), void* data)
{
	struct kirin_pcie *pcie;

	if (rc_id >= g_rc_num) {
		PCIE_PR_ERR("There is no rc_id = %d", rc_id);
		return -EINVAL;
	}

	pcie = &g_kirin_pcie[rc_id];
	pcie->callback_poweron  = poweron;
	pcie->callback_poweroff = poweroff;
	pcie->callback_data = data;
	return 0;
}
EXPORT_SYMBOL_GPL(kirin_pcie_power_notifiy_register);

void pcie_set_dbi_flag(void)
{
	if (IS_ENABLED(CONFIG_KIRIN_PCIE_TEST)){
		PCIE_PR_ERR("set dbi_debug_flag 0x5a5aa5a5");
		dbi_debug_flag = PCIE_ENABLE_DBI_READ_FLAG;
	}

	return;
}
EXPORT_SYMBOL_GPL(pcie_set_dbi_flag);

/*lint -e438 -e550 -e647 -e679 -e713 -e732 -e737 -e774 -e838 +esym(438,*) +esym(647,*) +esym(679,*) +esym(713,*) +esym(732,*) +esym(737,*) +esym(774,*) +esym(550,*) +esym(838,*) */

builtin_platform_driver(kirin_pcie_driver);

MODULE_AUTHOR("Xiaowei Song<songxiaowei@huawei.com>");
MODULE_DESCRIPTION("Hisilicon Kirin pcie driver");
MODULE_LICENSE("GPL");
