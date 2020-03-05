
export PATH=$PATH:/home/mmgf/aarch64-linux-android-4.9/bin
export CROSS_COMPILE=aarch64-linux-android-


mkdir ../out

make ARCH=arm64 O=../out merge_hi3660_defconfig

make ARCH=arm64 O=../out -j96
