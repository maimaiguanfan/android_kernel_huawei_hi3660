荣耀9盘古内核特性
===
解锁selinux状态限制，可调为permissive

解锁官方隐藏的Schedutil调度器

Upstream至Linux4.9.194

移植970的JPEG Processing引擎

使用GCC9.2.1编译

fsync开关

说明
===
修改SHT（华为平板M5 8.4英寸）的EMUI9.1.0.327的内核，并修复华为P10、P10 Plus、Mate9上出现不能使用WiFi的bug

（由于WiFi硬件不同，华为P10、P10 Plus、Mate9和荣耀9、v9（8Pro）、华为Nova2S、平板M5 8.4英寸内核不通刷。另外华为平板M5 10.8英寸为麒麟960s，未测试内核是否通刷）

 **支持的版本：** EMUI9.1.0所有版本、以及基于这些版本刷入的类原生ROM

[ **EMUI9.0.1版本请戳我** ](http://gitee.com/maimaiguanfan/Pangu9.0)

感谢[kindle4jerry大佬](http://github.com/kindle4jerry)的指导

编译教程
===
第零步
---
Linux环境（Windows子系统也可以）

熟悉Linux终端操作

学习git知识


第一步：下载
---
克隆源码到本地

克隆或下载[GCC 9.2](http://gitee.com/maimaiguanfan/arm-gcc)到本地

另外还有[GCC 10 Experiment](http://github.com/baalajimaestro/aarch64-maestro-linux-android/tree/05022020)可以使用，后面步骤用的就是[build_exp.sh](http://gitee.com/maimaiguanfan/Pangu9.1/blob/master/build_exp.sh)。但GCC10跑出来的包触摸屏有问题，可能是我忽略了某步

第二步：配置
---
打开终端，cd到源码路径

修改[build.sh](http://gitee.com/maimaiguanfan/Pangu9.1/blob/master/build.sh)，改第一、二条

`export PATH=$PATH:<你的GCC路径>/bin`

`export CROSS_COMPILE=<你的GCC路径>/bin/aarch64-none-linux-gnu-`

第三部：编译
---
运行`sh build.sh`（如果你用的是WIndows子系统，请把[build.sh](http://gitee.com/maimaiguanfan/Pangu9.1EROFS/blob/master/build.sh)的代码复制出来运行，因为Windows的环境变量会影响WSL）

如果你要编译的是P10（Plus）或Mate9的版本，那么运行的是`sh build_P10.sh`

编译过程可能会卡住，那就按一下回车

如果出错，那就再运行一次`sh build.sh`，不要怕，可以断点传输

如果编译成功，会输出在out/arch/arm64/boot/Image.gz

第四部：打包
---
运行`sh pack.sh`

打包输出三个文件，都在源码目录里，两个img和一个zip

zip是卡刷包，用[Anykernel3](http://gitee.com/maimaiguanfan/AnyKernel3/tree/hi3660/)打包，支持所有版本

img是镜像文件，可线刷可卡刷，刷到kernel分区，仅支持9.1.0.219版本！文件名带PM的SELinux模式默认为permissive模式，又称SELinux宽容模式（如果你看不到就别刷这个）