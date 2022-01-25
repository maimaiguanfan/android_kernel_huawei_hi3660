#
# Makefile for hisilicon camera fpga driver
#

EXTRA_CFLAGS += -Idrivers/media/huawei/camera/
EXTRA_CFLAGS += -fno-pic

obj-$(CONFIG_HISI_CAMERA_FPGA) += hwfpga.o
obj-$(CONFIG_HISI_CAMERA_FPGA_ICE40) += ice40/
obj-$(CONFIG_HISI_CAMERA_FPGA_STM32) += stm32/
