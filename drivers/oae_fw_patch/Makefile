#
# Makefile for the linux kernel.
#

ccflags-y := -fno-function-sections -fno-data-sections

obj-y                       := oae_dm.o
obj-y                       += oae_dm_log.o
obj-y                       += do_mounts_oae_patch.o
obj-$(CONFIG_OAE_DM_ROOT)   += do_mounts_oae_dm.o
