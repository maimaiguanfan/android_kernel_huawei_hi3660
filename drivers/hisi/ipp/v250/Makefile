# Hisilicon IPP v250cs Makefile

obj-$(CONFIG_HISPIPP_V250) += ipp-core.o ipp-com.o memory.o smc.o
obj-$(CONFIG_HISPIPP_V250) += v250/ device/
obj-$(CONFIG_HISPIPP_V250) += adapter_common.o \
                            adapter_mcf.o \
                            adapter_mfnr.o \
                            adapter_slam.o \
                            adapter_vbk.o \
                            adapter_reorder.o\
                            adapter_compare.o

subdir-ccflags-$(CONFIG_HISPIPP_V250) += -Wall -Werror
subdir-ccflags-$(CONFIG_HISPIPP_V250) += -I$(srctree)/drivers/hisi/ipp/v250 \
                                    -I$(srctree)/drivers/hisi/ipp/v250/v250 \
                                    -I$(srctree)/drivers/hisi/ipp/v250/device \
                                    -I$(srctree)/drivers/hisi/ipp/v250/include \
                                    -I$(srctree)/drivers/hisi/ipp/v250/v250/include \
                                    -I$(srctree)/drivers/hisi/ipp/v250/device/include \
                                    -I$(srctree)/drivers/hisi/tzdriver
ifneq ($(TARGET_BUILD_VARIANT),user)
subdir-ccflags-$(CONFIG_HISPIPP_V250) += -DCONFIG_IPP_DEBUG
endif
