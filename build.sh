

export PATH=$PATH:/home/leo-mak/gcc-arm-9.2-2019.12-x86_64-aarch64-none-linux-gnu/bin
export CROSS_COMPILE=/home/leo-mak/gcc-arm-9.2-2019.12-x86_64-aarch64-none-linux-gnu/bin/aarch64-none-linux-gnu-

export GCC_COLORS=auto
export ARCH=arm64

mkdir out

make ARCH=arm64 O=out Pangu_defconfig  > /dev/null


make ARCH=arm64 O=out -j64 > /dev/null
