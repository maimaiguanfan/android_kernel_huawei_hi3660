#!/bin/bash
#设置环境
echo " "
echo "***Setting environment...***"
rm -rf out/arch/arm64/boot/Image.gz
export PATH=$PATH:/home/leo-mak/aarch64-maestro-linux-android-05022020/bin
export CROSS_COMPILE=aarch64-maestro-linux-gnu-
export GCC_COLORS=auto
export ARCH=arm64
if [ ! -d "out" ];
then
	mkdir out
fi

#添加或更新AK3

if [ -f tools/AnyKernel3/README.md.osm0sis ];
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
	git remote add upstream https://github.com/osm0sis/AnyKernel3
	echo "***Updating AnyKernel3...***"
	echo " "
	cd tools/AnyKernel3
	git pull upstream master
fi
cd ../..
echo " "

#输入盘古内核版本号
printf "Please enter Pangu Kernel version number: "
read v
echo " "
echo "Setting EXTRAVERSION"

#构建骑士版内核
export EV=EXTRAVERSION=_骑士定制PanguV$v
echo " "
echo "***Building default version kernel...***"
echo " "
make ARCH=arm64 O=out $EV Pangu_defconfig  > /dev/null
make ARCH=arm64 O=out $EV -j64 > /dev/null

#打包骑士版内核
if [ -f out/arch/arm64/boot/Image.gz ];
then
	echo "***Packing default version kernel...***"
	tools/mkbootimg --kernel out/arch/arm64/boot/Image.gz --base 0x0 --cmdline "loglevel=4 initcall_debug=n page_tracker=on slub_min_objects=16 unmovable_isolate1=2:192M,3:224M,4:256M printktimer=0xfff0a000,0x534,0x538 androidboot.selinux=enforcing buildvariant=user" --tags_offset 0x07A00000 --kernel_offset 0x00080000 --ramdisk_offset 0x07c00000 --header_version 1 --os_version 9 --os_patch_level 2020-03-05  --output PK_V"$v"_9.1_骑士定制.img
	tools/mkbootimg --kernel out/arch/arm64/boot/Image.gz --base 0x0 --cmdline "loglevel=4 initcall_debug=n page_tracker=on slub_min_objects=16 unmovable_isolate1=2:192M,3:224M,4:256M printktimer=0xfff0a000,0x534,0x538 androidboot.selinux=permissive buildvariant=user" --tags_offset 0x07A00000 --kernel_offset 0x00080000 --ramdisk_offset 0x07c00000 --header_version 1 --os_version 9 --os_patch_level 2020-03-05  --output PK_V"$v"_9.1_骑士定制_PM.img
	cp out/arch/arm64/boot/Image.gz tools/AnyKernel3/Image.gz
	zip -r9 PK_V"$v"_9.1_骑士定制.zip tools/AnyKernel3/* > /dev/null
	rm -rf tools/AnyKernel3/Image.gz
	rm -rf out/arch/arm64/boot/Image.gz
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
export EV=EXTRAVERSION=_爵士定制PanguV$v
make ARCH=arm64 O=out $EV Pangu_P10_defconfig  > /dev/null
make ARCH=arm64 O=out $EV -j64 > /dev/null

#打包爵士版内核
if [ -f out/arch/arm64/boot/Image.gz ];
then
	echo "***Packing P10 version kernel...***"
	tools/mkbootimg --kernel out/arch/arm64/boot/Image.gz --base 0x0 --cmdline "loglevel=4 initcall_debug=n page_tracker=on slub_min_objects=16 unmovable_isolate1=2:192M,3:224M,4:256M printktimer=0xfff0a000,0x534,0x538 androidboot.selinux=enforcing buildvariant=user" --tags_offset 0x07A00000 --kernel_offset 0x00080000 --ramdisk_offset 0x07c00000 --header_version 1 --os_version 9 --os_patch_level 2020-03-05  --output PK_V"$v"_9.1_爵士定制.img
	tools/mkbootimg --kernel out/arch/arm64/boot/Image.gz --base 0x0 --cmdline "loglevel=4 initcall_debug=n page_tracker=on slub_min_objects=16 unmovable_isolate1=2:192M,3:224M,4:256M printktimer=0xfff0a000,0x534,0x538 androidboot.selinux=permissive buildvariant=user" --tags_offset 0x07A00000 --kernel_offset 0x00080000 --ramdisk_offset 0x07c00000 --header_version 1 --os_version 9 --os_patch_level 2020-03-05  --output PK_V"$v"_9.1_爵士定制_PM.img
	cp out/arch/arm64/boot/Image.gz tools/AnyKernel3/Image.gz
	zip -r9 PK_V"$v"_9.1_爵士定制.zip tools/AnyKernel3/* > /dev/null
	rm -rf tools/AnyKernel3/Image.gz
	echo " "
	echo "***Sucessfully built P10 version kernel...***"
	echo " "
else
	echo " "
	echo "***Failed!***"
	exit 0
fi

