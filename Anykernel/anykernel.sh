# AnyKernel2 Ramdisk Mod Script
# osm0sis @ XDA

## AnyKernel setup
# begin properties
properties() {
kernel.string=Pangu Kernel
do.devicecheck=0
do.modules=0
do.cleanup=1
do.cleanuponabort=1
device.name1=
device.name2=
} # end properties

## AnyKernel methods (DO NOT CHANGE)
# import patching functions/variables - see for reference
. /tmp/anykernel/tools/ak2-core.sh;

ui_print "                                    ";
ui_print "     Pangu kernel      ";
ui_print "                                    ";
ui_print "   by maimaiguanfan   ";
ui_print "                                    ";
ui_print "      for Honor 9      ";
ui_print "                                    ";

# shell variables
block=/dev/block/bootdevice/by-name/kernel;
ramdisk_compression=auto;

# reset for kernel patching
reset_ak;
mv /tmp/anykernel/kernel-Image.gz /tmp/anykernel/Image.gz;


## AnyKernel kernel install

ui_print " Done ";

split_boot;

flash_boot;

## end install
