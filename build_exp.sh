

export PATH=$PATH:/home/leo-mak/aarch64-maestro-linux-android-05022020/bin
export CROSS_COMPILE=aarch64-maestro-linux-gnu-

export GCC_COLORS=auto
export ARCH=arm64

mkdir out

make ARCH=arm64 O=out merge_hi3660_defconfig  > /dev/null


make ARCH=arm64 O=out -j64 > /dev/null
