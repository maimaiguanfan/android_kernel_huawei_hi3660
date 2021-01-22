#!/bin/bash
#设置环境
echo " "
echo "***Setting environment...***"
rm -rf out/arch/arm64/boot/Image.gz
export PATH=$PATH:~/gcc-arm-9.2-2019.12-x86_64-aarch64-none-linux-gnu/bin
export CROSS_COMPILE=~/gcc-arm-9.2-2019.12-x86_64-aarch64-none-linux-gnu/bin/aarch64-none-linux-gnu-
export GCC_COLORS=auto
export ARCH=arm64
if [ ! -d "out" ];
then
	mkdir out
fi

#添加或更新AK3

if [ -f tools/AnyKernel3/README.md ];
then
	cd tools/AnyKernel3
	echo " "
	echo "***Updating AnyKernel3...***"
	echo " "
	git pull upstream master
else
	echo " "
	echo "***Adding AnyKernel3...***"
	echo " "
	git submodule update --init --recursive
	cd tools/AnyKernel3
	git remote add upstream https://github.com/osm0sis/AnyKernel3
	echo "***Updating AnyKernel3...***"
	echo " "
	git pull upstream master
fi
cd ../..
echo " "

#输入盘古内核版本号
printf "Please enter Pangu Kernel version number: "
read v
echo " "
echo "Setting EXTRAVERSION"
export EV=EXTRAVERSION=_Kirin960_Pangu_V$v

#构建骑士版内核
echo " "
echo "***Building default version kernel...***"
echo " "
make ARCH=arm64 O=out $EV Pangu_defconfig
make ARCH=arm64 O=out $EV -j64

#打包骑士版内核
if [ -f out/arch/arm64/boot/Image.gz ];
then
	echo "***Packing default version kernel...***"
	tools/mkbootimg --kernel out/arch/arm64/boot/Image.gz --base 0x0 --cmdline "loglevel=4 initcall_debug=n page_tracker=on slub_min_objects=16 unmovable_isolate1=2:192M,3:224M,4:256M printktimer=0xfff0a000,0x534,0x538 androidboot.selinux=enforcing buildvariant=user" --tags_offset 0x07A00000 --kernel_offset 0x00080000 --ramdisk_offset 0x07c00000 --header_version 1 --os_version 9 --os_patch_level 2020-09-05  --output PK_V"$v"_9.1_骑士定制.img
	tools/mkbootimg --kernel out/arch/arm64/boot/Image.gz --base 0x0 --cmdline "loglevel=4 initcall_debug=n page_tracker=on slub_min_objects=16 unmovable_isolate1=2:192M,3:224M,4:256M printktimer=0xfff0a000,0x534,0x538 androidboot.selinux=permissive buildvariant=user" --tags_offset 0x07A00000 --kernel_offset 0x00080000 --ramdisk_offset 0x07c00000 --header_version 1 --os_version 9 --os_patch_level 2020-09-05  --output PK_V"$v"_9.1_骑士定制_PM.img
	mv out/arch/arm64/boot/Image.gz tools/AnyKernel3/Image.gz
	cd tools/AnyKernel3
	zip -r9 PK_V"$v"_9.1_骑士定制.zip * > /dev/null
	cd ../..
	mv tools/AnyKernel3/PK_V"$v"_9.1_骑士定制.zip PK_V"$v"_9.1_骑士定制.zip
	rm -rf tools/AnyKernel3/Image.gz
	echo " "
	echo "***Sucessfully built default version kernel...***"
	echo " "
else
	echo " "
	echo "***Failed!***"
	exit 0
fi

#构建爵士内核WiFi部分
echo "***Building WiFi drivers for P10 version...***"
make ARCH=arm64 O=out $EV Pangu_P10_defconfig
make ARCH=arm64 O=out $EV -j64

#打包爵士版内核
if [ -f out/arch/arm64/boot/Image.gz ];
then
	echo "***Packing P10 version kernel...***"
	tools/mkbootimg --kernel out/arch/arm64/boot/Image.gz --base 0x0 --cmdline "loglevel=4 initcall_debug=n page_tracker=on slub_min_objects=16 unmovable_isolate1=2:192M,3:224M,4:256M printktimer=0xfff0a000,0x534,0x538 androidboot.selinux=enforcing buildvariant=user" --tags_offset 0x07A00000 --kernel_offset 0x00080000 --ramdisk_offset 0x07c00000 --header_version 1 --os_version 9 --os_patch_level 2020-09-05  --output PK_V"$v"_9.1_爵士定制.img
	tools/mkbootimg --kernel out/arch/arm64/boot/Image.gz --base 0x0 --cmdline "loglevel=4 initcall_debug=n page_tracker=on slub_min_objects=16 unmovable_isolate1=2:192M,3:224M,4:256M printktimer=0xfff0a000,0x534,0x538 androidboot.selinux=permissive buildvariant=user" --tags_offset 0x07A00000 --kernel_offset 0x00080000 --ramdisk_offset 0x07c00000 --header_version 1 --os_version 9 --os_patch_level 2020-09-05  --output PK_V"$v"_9.1_爵士定制_PM.img
	mv out/arch/arm64/boot/Image.gz tools/AnyKernel3/Image.gz
	cd tools/AnyKernel3
	zip -r9 PK_V"$v"_9.1_爵士定制.zip * > /dev/null
	cd ../..
	mv tools/AnyKernel3/PK_V"$v"_9.1_爵士定制.zip PK_V"$v"_9.1_爵士定制.zip
	rm -rf tools/AnyKernel3/Image.gz
	echo " "
	echo "***Sucessfully built P10 version kernel...***"
	echo " "
else
	echo " "
	echo "***Failed!***"
	exit 0
fi

