#!/bin/bash
echo " "
echo "***Setting environment...***"
echo " "
rm -rf out/arch/arm64/boot/Image.gz
export PATH=$PATH:/home/leo-mak/gcc-arm-9.2-2019.12-x86_64-aarch64-none-linux-gnu/bin
export CROSS_COMPILE=/home/leo-mak/gcc-arm-9.2-2019.12-x86_64-aarch64-none-linux-gnu/bin/aarch64-none-linux-gnu-
export GCC_COLORS=auto
export ARCH=arm64
mkdir out

echo " "
echo "***Building...***"
echo " "
make ARCH=arm64 O=out Pangu_defconfig  > /dev/null
make ARCH=arm64 O=out -j64 > /dev/null

if [ -f out/arch/arm64/boot/Image.gz ];
then
	echo "***Packing...***"
	tools/mkbootimg --kernel out/arch/arm64/boot/Image.gz --base 0x0 --cmdline "loglevel=4 initcall_debug=n page_tracker=on slub_min_objects=16 unmovable_isolate1=2:192M,3:224M,4:256M printktimer=0xfff0a000,0x534,0x538 androidboot.selinux=enforcing buildvariant=user" --tags_offset 0x07A00000 --kernel_offset 0x00080000 --ramdisk_offset 0x07c00000 --header_version 1 --os_version 9 --os_patch_level 2020-03-05  --output PK_EROFS.img
	tools/mkbootimg --kernel out/arch/arm64/boot/Image.gz --base 0x0 --cmdline "loglevel=4 initcall_debug=n page_tracker=on slub_min_objects=16 unmovable_isolate1=2:192M,3:224M,4:256M printktimer=0xfff0a000,0x534,0x538 androidboot.selinux=permissive buildvariant=user" --tags_offset 0x07A00000 --kernel_offset 0x00080000 --ramdisk_offset 0x07c00000 --header_version 1 --os_version 9 --os_patch_level 2020-03-05  --output PK_EROFS_PM.img
	rm -rf tools/AnyKernel3/*.zip
	cp out/arch/arm64/boot/Image.gz tools/AnyKernel3/Image.gz
	cd tools/AnyKernel3
	echo "***Updating AnyKernel3...***"
	echo " "
	git pull upstream master
	echo " "
	zip -r9 PK_EROFS.zip * ".git" > /dev/null
	cd ../..
	cp tools/AnyKernel3/PK_EROFS.zip PK_EROFS.zip
	rm -rf tools/AnyKernel3/PK_EROFS.zip
	rm -rf tools/AnyKernel3/Image.gz
	rm -rf out/arch/arm64/boot/Image.gz
	echo " "
	echo "***Sucessfully built default version kernel...***"
	echo " "
else
	echo "***Failed!***"
	exit 0
fi

echo "***Building WiFi part for P10 version...***"
make ARCH=arm64 O=out Pangu_P10_defconfig  > /dev/null
make ARCH=arm64 O=out -j64 > /dev/null

if [ -f out/arch/arm64/boot/Image.gz ];
then
	echo "***Packing...***"
	tools/mkbootimg --kernel out/arch/arm64/boot/Image.gz --base 0x0 --cmdline "loglevel=4 initcall_debug=n page_tracker=on slub_min_objects=16 unmovable_isolate1=2:192M,3:224M,4:256M printktimer=0xfff0a000,0x534,0x538 androidboot.selinux=enforcing buildvariant=user" --tags_offset 0x07A00000 --kernel_offset 0x00080000 --ramdisk_offset 0x07c00000 --header_version 1 --os_version 9 --os_patch_level 2020-03-05  --output PK_P10_EROFS.img
	tools/mkbootimg --kernel out/arch/arm64/boot/Image.gz --base 0x0 --cmdline "loglevel=4 initcall_debug=n page_tracker=on slub_min_objects=16 unmovable_isolate1=2:192M,3:224M,4:256M printktimer=0xfff0a000,0x534,0x538 androidboot.selinux=permissive buildvariant=user" --tags_offset 0x07A00000 --kernel_offset 0x00080000 --ramdisk_offset 0x07c00000 --header_version 1 --os_version 9 --os_patch_level 2020-03-05  --output PK_P10_EROFS_PM.img
	rm -rf tools/AnyKernel3/*.zip
	cp out/arch/arm64/boot/Image.gz tools/AnyKernel3/Image.gz
	cd tools/AnyKernel3
	echo "***Updating AnyKernel3...***"
	echo " "
	git pull upstream master
	echo " "
	zip -r9 PK_P10_EROFS.zip * ".git" > /dev/null
	cd ../..
	cp tools/AnyKernel3/PK_P10_EROFS.zip PK_P10_EROFS.zip
	rm -rf tools/AnyKernel3/PK_P10_EROFS.zip
	rm -rf tools/AnyKernel3/Image.gz
	rm -rf out/arch/arm64/boot/Image.gz
	echo " "
	echo "***Sucessfully built P10 version kernel...***"
	echo " "
else
	echo "***Failed!***"
	exit 0
fi

