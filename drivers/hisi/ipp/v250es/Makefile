# Hisilicon IPP Makefile

obj-$(CONFIG_HISPIPP_V250) += adapter_common.o
obj-$(CONFIG_HISPIPP_V250) += adapter_mcf.o
obj-$(CONFIG_HISPIPP_V250) += adapter_mfnr.o
obj-$(CONFIG_HISPIPP_V250) += adapter_slam.o
obj-$(CONFIG_HISPIPP_V250) += adapter_slamtest.o
obj-$(CONFIG_HISPIPP_V250) += ipp-core.o
obj-$(CONFIG_HISPIPP_V250) += ipp-com.o
obj-$(CONFIG_HISPIPP_V250) += smc.o
obj-$(CONFIG_HISPIPP_V250) += memory.o

obj-$(CONFIG_HISPIPP_V250) += v250es/
obj-$(CONFIG_HISPIPP_V250) += device/


subdir-ccflags-$(CONFIG_HISPIPP_V250) += -Wall -Werror
subdir-ccflags-$(CONFIG_HISPIPP_V250) += -I$(srctree)/drivers/hisi/ipp/v250es \
                                    -I$(srctree)/drivers/hisi/ipp/v250es/v250es \
                                    -I$(srctree)/drivers/hisi/ipp/v250es/device \
                                    -I$(srctree)/drivers/hisi/ipp/v250es/include \
                                    -I$(srctree)/drivers/hisi/ipp/v250es/v250es/include \
                                    -I$(srctree)/drivers/hisi/ipp/v250es/device/include \
                                    -I$(srctree)/drivers/hisi/tzdriver

ifneq ($(TARGET_BUILD_VARIANT),user)
subdir-ccflags-$(CONFIG_HISPIPP_V250) += -DCONFIG_IPP_DEBUG
endif
