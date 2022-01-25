# Hisilicon IPP Makefile
ccflags-$(CONFIG_HISPIPP_V200) += -Idrivers/hisi/ipp/v200
ccflags-$(CONFIG_HISPIPP_V210) += -Idrivers/hisi/ipp/v210
ifeq ($(chip_type),es)
ccflags-$(CONFIG_HISPIPP_V250) += -Idrivers/hisi/ipp/v250es
else
ccflags-$(CONFIG_HISPIPP_V250) += -Idrivers/hisi/ipp/v250
endif
obj-$(CONFIG_HISPIPP_V200) += v200/
obj-$(CONFIG_HISPIPP_V210) += v210/
ifeq ($(chip_type),es)
obj-$(CONFIG_HISPIPP_V250) += v250es/
else
obj-$(CONFIG_HISPIPP_V250) += v250/
endif
