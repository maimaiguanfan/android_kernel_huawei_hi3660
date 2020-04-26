麒麟960盘古内核
===
特性
---
解锁selinux状态限制，可调为permissive

解锁官方隐藏的CPU调度器Schedutil

移植[荣耀9 EMUI8 Proto内核](http://github.com/JBolho/Proto)的CPU调度器Blu_Schedutil，并设为默认

添加Dynamic Stune Boost

添加WireGuard

新增I/O调度器ZEN，并设为默认

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
 支持EMUI9.0.1所有版本、以及基于这些版本刷入的类原生ROM

[ **EMUI9.1版本请戳我** ](http://gitee.com/maimaiguanfan/Pangu9.1EROFS)

关于发行版的说明
---
每个发行版本有6个文件，爵士和骑士各三个

三个文件中1个是.zip，2个是.img

zip是卡刷包，用[Anykernel3](http://gitee.com/maimaiguanfan/AnyKernel3)打包，适合第三方rec刷入

img是镜像文件，可以fastboot刷，也可以rec刷，刷到kernel分区

img中文件名带PM的SELinux状态默认为permissive模式，又称SELinux宽容模式（EROFS文件系统在permissive状态下可挂载读写，部分类原生需要permissive才能开机，如果你看不懂就别刷这个）

卡刷zip会保持上一个内核的默认SeLinux状态

编译教程
===

[请戳我](http://gitee.com/maimaiguanfan/Pangu9.1EROFS/blob/master/README.md)

鸣谢：
===
[ **kindle4jerry大佬** ](http://github.com/kindle4jerry)，提供指引，帮我修复编译错误

[ **JBolho大佬** ](http://github.com/JBolho)，他的Proto内核为我提供了大量帮助

[ **engstk大佬** ](https://github.com/engstk)，他的[荣耀v10 Blu_Spark内核](http://github.com/engstk/view10)为我提供了大量帮助

[ **joshuous大佬** ](http://github.com/joshuous/)，提供Dynamic Stune Boost源码

还有很多位测试人员以及提供帮助和建议的人