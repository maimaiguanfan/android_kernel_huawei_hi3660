#!/bin/bash
tools/mkbootimg --kernel ../out/arch/arm64/boot/Image.gz --base 0x0 --cmdline "loglevel=4 initcall_debug=n page_tracker=on slub_min_objects=16 unmovable_isolate1=2:192M,3:224M,4:256M printktimer=0xfff0a000,0x534,0x538 androidboot.selinux=enforcing buildvariant=user" --tags_offset 0x07A00000 --kernel_offset 0x00080000 --ramdisk_offset 0x07c00000 --header_version 1 --os_version 9 --os_patch_level 2019-05-05  --output ../Pangu_kernel.img
tools/mkbootimg --kernel ../out/arch/arm64/boot/Image.gz --base 0x0 --cmdline "loglevel=4 initcall_debug=n page_tracker=on slub_min_objects=16 unmovable_isolate1=2:192M,3:224M,4:256M printktimer=0xfff0a000,0x534,0x538 androidboot.selinux=permissive buildvariant=user" --tags_offset 0x07A00000 --kernel_offset 0x00080000 --ramdisk_offset 0x07c00000 --header_version 1 --os_version 9 --os_patch_level 2019-05-05  --output ../Pangu_kernel-permissive.img
cp ../out/arch/arm64/boot/Image.gz Anykernel/Image.gz
cd Anykernel
zip -r9 Pangu_kernel.zip *
cd ..
cp Anykernel/Pangu_kernel.zip ../Pangu_kernel.zip
rm -rf Anykernel/Pangu_kernel.zip

