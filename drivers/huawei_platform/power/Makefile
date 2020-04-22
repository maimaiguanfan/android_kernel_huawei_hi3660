obj-$(CONFIG_HUAWEI_CHARGER)	+= charger/
obj-$(CONFIG_HUAWEI_POWER_MESG_INTERFACE)	+= power_mesg/
obj-$(CONFIG_HUAWEI_BATTERY_INFORMATION)    	+= battery/
obj-$(CONFIG_HUAWEI_COUL)	+= coul/
obj-$(CONFIG_HUAWEI_KSTATE)	+= hw_kstate/
obj-$(CONFIG_HUAWEI_FEATURE_PRINT_PID_NAME) += pid_socket/
obj-$(CONFIG_HUAWEI_SMPL)	+= smpl/
obj-$(CONFIG_BUCKBOOST)	+= buckboost/
obj-$(CONFIG_BUCKBOOST)	+= buck_boost/
obj-$(CONFIG_HUAWEI_USB_SHORT_CIRCUIT_PROTECT)	+= usb_short_circuit_protect/
obj-$(CONFIG_HUAWEI_CPU_BUCK)	+= cpu_buck/
obj-$(CONFIG_BOOST_5V)	+= boost_5v/
obj-$(CONFIG_HUAWEI_DSM)	+= power_dsm/
obj-$(CONFIG_HUAWEI_BATT_ACR_DETECT)	+= batt_acr_detect/
obj-$(CONFIG_HUAWEI_POWER_THERMALZONE)	+= power_thermalzone/
obj-$(CONFIG_HUAWEI_POWER_DEBUG)	+= power_debug/
obj-$(CONFIG_HUAWEI_VSYS_SWITCH) += vsys_switch/
obj-$(CONFIG_HUAWEI_SOC_CONTROL) += hardware_monitor/

ifeq ($(HLTHERM_RUNTEST),true)
CFLAGS_KERNEL += -DCONFIG_HLTHERM_RUNTEST
endif

obj-$(CONFIG_HUAWEI_WATER_CHECK) += water_check/
obj-$(CONFIG_DOZE_FILTER)	+= wifi_filter/
