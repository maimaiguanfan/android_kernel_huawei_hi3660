#
# Copyright (c) Huawei Technologies Co., Ltd. 2016-2018. All rights reserved.
# Description: Makefile for the Huawei root scanner.
# Author: likun <quentin.lee@huawei.com>
# Create: 2016-06-18
#

obj-$(CONFIG_HW_ROOT_SCAN) += hw_rscan_utils.o
obj-$(CONFIG_HW_ROOT_SCAN) += kcode.o
obj-$(CONFIG_HW_ROOT_SCAN) += rproc.o
obj-$(CONFIG_HW_ROOT_SCAN) += sescan.o
obj-$(CONFIG_HW_ROOT_SCAN) += setids.o
obj-$(CONFIG_HW_ROOT_SCAN) += hw_rscan_scanner.o
obj-$(CONFIG_HW_ROOT_SCAN) += hw_rscan_module.o
ccflags-$(CONFIG_HW_ROOT_SCAN) += -Idrivers/hisi/tzdriver/ca_antiroot
