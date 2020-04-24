荣耀9盘古内核
===
特性
---
解锁selinux状态限制，可调为permissive

EROFS文件系统在permissive状态下可挂载读写

解锁官方隐藏的CPU调度器Schedutil

移植[荣耀9 EMUI8 Proto内核](http://github.com/JBolho/Proto)的CPU调度器Blu_Schedutil，并设为默认

新增I/O调度器ZEN，并设为默认

Upstream至Linux4.9.155

移植970的JPEG Processing引擎

使用GCC9.2.1编译

fsync开关

支持的设备
---
 **爵士定制版:**  华为P10、P10 Plus、Mate9

 **骑士定制版:**  荣耀9、v9（8Pro）、华为Nova2S、平板M5 8.4英寸
 
		另外华为平板M5 10.8英寸为麒麟960s，待测试

支持的系统
---
 支持EMUI9.1.0所有版本、以及基于这些版本刷入的类原生ROM。虽然这个内核虽然叫9.1EROFS内核，但不是EROFS的9.1版本也能用

[ **EMUI9.0.1版本请戳我** ](http://gitee.com/maimaiguanfan/Pangu9.0)

关于发行版的说明
---
每个发行版本有6个文件，爵士和骑士各三个

三个文件中1个是.zip，2个是.img

zip是卡刷包，用[Anykernel3](http://gitee.com/maimaiguanfan/AnyKernel3)打包，适合第三方rec刷入

img是镜像文件，可以fastboot刷，也可以rec刷，刷到kernel分区

img中文件名带PM的SELinux状态默认为permissive模式，又称SELinux宽容模式（EROFS文件系统在permissive状态下可挂载读写，部分类原生需要permissive才能开机，如果你看不懂就别刷这个）

卡刷zip会保持上一个内核的默认SeLinux状态

 **前方高能！！！** 

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

修改[build.sh](http://gitee.com/maimaiguanfan/Pangu9.1/blob/master/build.sh)第七、八行

`export PATH=$PATH:<你的GCC路径>/bin`

`export CROSS_COMPILE=<你的GCC路径>/bin/aarch64-none-linux-gnu-`

第三部：编译
---
运行`sh build.sh`（如果你用的是WIndows子系统，请把第七行注释掉并复制出来单独运行，因为Windows的环境变量会影响WSL）

编译过程可能会卡住，那就按一下回车

如果出错，那就再运行一次`sh build.sh`，不要怕，可以断点传输

如果编译成功，输出的6个文件都在源码目录里

鸣谢：
===
[ **kindle4jerry大佬** ](http://github.com/kindle4jerry)

[ **JBolho大佬** ](http://github.com/JBolho)
