#!/bin/bash
echo " "
echo "***Packing Images***"
echo " "
tools/mkbootimg --kernel out/arch/arm64/boot/Image.gz --base 0x0 --cmdline "loglevel=4 initcall_debug=n page_tracker=on slub_min_objects=16 unmovable_isolate1=2:192M,3:224M,4:256M printktimer=0xfff0a000,0x534,0x538 androidboot.selinux=enforcing buildvariant=user" --tags_offset 0x07A00000 --kernel_offset 0x00080000 --ramdisk_offset 0x07c00000 --header_version 1 --os_version 9 --os_patch_level 2019-05-05  --output PK_9.0.img
tools/mkbootimg --kernel out/arch/arm64/boot/Image.gz --base 0x0 --cmdline "loglevel=4 initcall_debug=n page_tracker=on slub_min_objects=16 unmovable_isolate1=2:192M,3:224M,4:256M printktimer=0xfff0a000,0x534,0x538 androidboot.selinux=permissive buildvariant=user" --tags_offset 0x07A00000 --kernel_offset 0x00080000 --ramdisk_offset 0x07c00000 --header_version 1 --os_version 9 --os_patch_level 2019-05-05  --output PK_9.0_PM.img
echo "***Done!***"
echo " "
rm -rf tools/AnyKernel3/*.zip
cp out/arch/arm64/boot/Image.gz tools/AnyKernel3/Image.gz
cd tools/AnyKernel3
echo "***Updating AnyKernel3...***"
echo " "
git pull upstream master
echo " "
echo "***Packing Zip***"
echo " "
zip -r9 PK_9.0.zip * ".git" > /dev/null
cd ../..
cp tools/AnyKernel3/PK_9.0.zip PK_9.0.zip
rm -rf tools/AnyKernel3/PK_9.0.zip
rm -rf tools/AnyKernel3/Image.gz
echo "***Done!***"
echo " "
