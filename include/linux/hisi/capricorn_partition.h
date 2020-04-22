#ifndef _CAPRICORN_PARTITION_H_
#define _CAPRICORN_PARTITION_H_

#include "hisi_partition.h"
#include "partition_def.h"

static const struct partition partition_table_emmc[] =
{
  {PART_XLOADER,                           0,           2*1024,        EMMC_BOOT_MAJOR_PART},
  {PART_RESERVED0,                         0,           2*1024,        EMMC_BOOT_BACKUP_PART},
  {PART_PTABLE,                            0,              512,        EMMC_USER_PART},/* ptable          512K */
  {PART_FRP,                             512,              512,        EMMC_USER_PART},/* frp             512K   p1*/
  {PART_PERSIST,                        1024,           6*1024,        EMMC_USER_PART},/* persist           6M   p2*/
  {PART_RESERVED1,                    7*1024,             1024,        EMMC_USER_PART},/* reserved1      1024K   p3*/
  {PART_RESERVED6,                    8*1024,              512,        EMMC_USER_PART},/* reserved6       512K   p4*/
  {PART_VRL,                            8704,              512,        EMMC_USER_PART},/* vrl             512K   p5*/
  {PART_VRL_BACKUP,                   9*1024,              512,        EMMC_USER_PART},/* vrl backup      512K   p6*/
  {PART_MODEM_SECURE,                   9728,             8704,        EMMC_USER_PART},/* modem_secure   8704K   p7*/
  {PART_NVME,                        18*1024,           5*1024,        EMMC_USER_PART},/* nvme              5M   p8*/
  {PART_CTF,                         23*1024,           1*1024,        EMMC_USER_PART},/* ctf               1M   p9*/
  {PART_OEMINFO,                     24*1024,          96*1024,        EMMC_USER_PART},/* oeminfo          96M   p10*/
  {PART_SECURE_STORAGE,             120*1024,          32*1024,        EMMC_USER_PART},/* secure storage   32M   p11*/
  {PART_MODEMNVM_FACTORY,           152*1024,          16*1024,        EMMC_USER_PART},/* modemnvm factory 16M   p12*/
  {PART_MODEMNVM_BACKUP,            168*1024,          16*1024,        EMMC_USER_PART},/* modemnvm backup  16M   p13*/
  {PART_MODEMNVM_IMG,               184*1024,          34*1024,        EMMC_USER_PART},/* modemnvm img     34M   p14*/
  {PART_HISEE_ENCOS,                218*1024,           4*1024,        EMMC_USER_PART},/* hisee_encos       4M   p15*/
  {PART_VERITYKEY,                  222*1024,           1*1024,        EMMC_USER_PART},/* veritykey         1M   p16*/
  {PART_DDR_PARA,                   223*1024,           1*1024,        EMMC_USER_PART},/* DDR_PARA          1M   p17*/
  {PART_LOWPOWER_PARA,              224*1024,           1*1024,        EMMC_USER_PART},/* lowpower_para     1M   p18*/
  {PART_BATT_TP_PARA,               225*1024,           1*1024,        EMMC_USER_PART},/* batt_tp_para      1M   p19*/
  {PART_RESERVED2,                  226*1024,          25*1024,        EMMC_USER_PART},/* reserved2        25M   p20*/
  {PART_SPLASH2,                    251*1024,          80*1024,        EMMC_USER_PART},/* splash2          80M   p21*/
  {PART_BOOTFAIL_INFO,              331*1024,           2*1024,        EMMC_USER_PART},/* bootfail info     2M   p22*/
  {PART_MISC,                       333*1024,           2*1024,        EMMC_USER_PART},/* misc              2M   p23*/
  {PART_DFX,                        335*1024,          16*1024,        EMMC_USER_PART},/* dfx              16M   p24*/
  {PART_RRECORD,                    351*1024,          16*1024,        EMMC_USER_PART},/* rrecord          16M   p25*/
  {PART_CACHE,                      367*1024,         104*1024,        EMMC_USER_PART},/* cache           104M   p26*/
  {PART_FW_LPM3,                    471*1024,             1024,        EMMC_USER_PART},/* fw_lpm3        1024K   p27*/
  {PART_RESERVED3,                  472*1024,           5*1024,        EMMC_USER_PART},/* reserved3      5120K   p28*/
  {PART_NPU,                        477*1024,           8*1024,        EMMC_USER_PART},/* npu                8M    p29*/
  {PART_HIEPS,                      485*1024,           2*1024,        EMMC_USER_PART},/* hieps              2M    p30*/
  {PART_IVP,                        487*1024,           2*1024,        EMMC_USER_PART},/* ivp                2M    p31*/
  {PART_HDCP,                       489*1024,           1*1024,        EMMC_USER_PART},/* PART_HDCP          1M    p32*/
  {PART_HISEE_IMG,                  490*1024,           4*1024,        EMMC_USER_PART},/* part_hisee_img     4M    p33*/
  {PART_HHEE,                       494*1024,           4*1024,        EMMC_USER_PART},/* hhee               4M    p34*/
  {PART_HISEE_FS,                   498*1024,           8*1024,        EMMC_USER_PART},/* hisee_fs           8M    p35*/
  {PART_FASTBOOT,                   506*1024,          12*1024,        EMMC_USER_PART},/* fastboot          12M    p36*/
  {PART_VECTOR,                     518*1024,           4*1024,        EMMC_USER_PART},/* vector             4M    p37*/
  {PART_ISP_BOOT,                   522*1024,           2*1024,        EMMC_USER_PART},/* isp_boot           2M    p38*/
  {PART_ISP_FIRMWARE,               524*1024,          14*1024,        EMMC_USER_PART},/* isp_firmware      14M    p39*/
  {PART_FW_HIFI,                    538*1024,          12*1024,        EMMC_USER_PART},/* hifi              12M    p40*/
  {PART_TEEOS,                      550*1024,           8*1024,        EMMC_USER_PART},/* teeos              8M    p41*/
  {PART_SENSORHUB,                  558*1024,          16*1024,        EMMC_USER_PART},/* sensorhub         16M    p42*/
#ifdef CONFIG_SANITIZER_ENABLE
  {PART_ERECOVERY_KERNEL,           574*1024,          24*1024,        EMMC_USER_PART},/* erecovery_kernel  24M    p43*/
  {PART_ERECOVERY_RAMDISK,          598*1024,          32*1024,        EMMC_USER_PART},/* erecovery_ramdisk 32M    p44*/
  {PART_ERECOVERY_VENDOR,           630*1024,           8*1024,        EMMC_USER_PART},/* erecovery_vendor   8M    p45*/
  {PART_KERNEL,                     638*1024,          32*1024,        EMMC_USER_PART},/* kernel            32M    p46*/
#else
  {PART_ERECOVERY_KERNEL,           574*1024,          24*1024,        EMMC_USER_PART},/* erecovery_kernel  24M    p43*/
  {PART_ERECOVERY_RAMDISK,          598*1024,          32*1024,        EMMC_USER_PART},/* erecovery_ramdisk 32M    p44*/
  {PART_ERECOVERY_VENDOR,           630*1024,          16*1024,        EMMC_USER_PART},/* erecovery_vendor  16M    p45*/
  {PART_KERNEL,                     646*1024,          24*1024,        EMMC_USER_PART},/* kernel            24M    p46*/
#endif
  {PART_ENG_SYSTEM,                 670*1024,          12*1024,        EMMC_USER_PART},/* eng_system        12M    p47*/
  {PART_RECOVERY_RAMDISK,           682*1024,          32*1024,        EMMC_USER_PART},/* recovery_ramdisk  32M    p48*/
  {PART_RECOVERY_VENDOR,            714*1024,          16*1024,        EMMC_USER_PART},/* recovery_vendor   16M    p49*/
  {PART_DTS,                        730*1024,           1*1024,        EMMC_USER_PART},/* dtimage            1M    p50*/
  {PART_DTO,                        731*1024,          20*1024,        EMMC_USER_PART},/* dtoimage          20M    p51*/
  {PART_TRUSTFIRMWARE,              751*1024,           2*1024,        EMMC_USER_PART},/* trustfirmware      2M    p52*/
  {PART_MODEM_FW,                   753*1024,          56*1024,        EMMC_USER_PART},/* modem_fw          56M    p53*/
  {PART_ENG_VENDOR,                 809*1024,          20*1024,        EMMC_USER_PART},/* eng_vendor        20M    p54*/
  {PART_MODEM_PATCH_NV,             829*1024,           4*1024,        EMMC_USER_PART},/* modem_patch_nv     4M    p55*/
  {PART_MODEM_DRIVER,               833*1024,          20*1024,        EMMC_USER_PART},/* modem_driver      20M    p56*/
  {PART_RESERVED4,                  853*1024,          11*1024,        EMMC_USER_PART},/* reserved4A        11M    p57*/
  {PART_RECOVERY_VBMETA,            864*1024,           2*1024,        EMMC_USER_PART},/* recovery_vbmeta    2M    p58*/
  {PART_ERECOVERY_VBMETA,           866*1024,           2*1024,        EMMC_USER_PART},/* erecovery_vbmeta   2M    p59*/
  {PART_VBMETA,                     868*1024,           4*1024,        EMMC_USER_PART},/* PART_VBMETA        4M    p60*/
  {PART_MODEMNVM_UPDATE,            872*1024,          16*1024,        EMMC_USER_PART},/* modemnvm_update   16M    p61*/
  {PART_MODEMNVM_CUST,              888*1024,          16*1024,        EMMC_USER_PART},/* modemnvm_cust     16M    p62*/
  {PART_PATCH,                      904*1024,          32*1024,        EMMC_USER_PART},/* patch             32M    p63*/
#ifdef CONFIG_FACTORY_MODE
  {PART_VENDOR,                     936*1024,        1232*1024,        EMMC_USER_PART},/* vendor          1232M    p64*/
  {PART_ODM,                       2168*1024,         192*1024,        EMMC_USER_PART},/* odm              192M    p65*/
  {PART_CUST,                      2360*1024,         192*1024,        EMMC_USER_PART},/* cust             192M    p66*/
  {PART_SYSTEM,                    2552*1024,        5520*1024,        EMMC_USER_PART},/* system          5520M    p67*/
  {PART_PRODUCT,                   8072*1024,        1328*1024,        EMMC_USER_PART},/* product         1328M    p68*/
  {PART_VERSION,                   9400*1024,         576*1024,        EMMC_USER_PART},/* version          576M    p69*/
  {PART_PRELOAD,                   9976*1024,        1144*1024,        EMMC_USER_PART},/* preload         1144M    p70*/
  {PART_HIBENCH_IMG,              11120*1024,         128*1024,        EMMC_USER_PART},/* hibench_img      128M    p71*/
  {PART_HIBENCH_DATA,             11248*1024,         512*1024,        EMMC_USER_PART},/* hibench_data     512M    p72*/
  {PART_FLASH_AGEING,             11760*1024,         512*1024,        EMMC_USER_PART},/* FLASH_AGEING     512M    p73*/
  {PART_USERDATA,                 12272*1024,  (4UL)*1024*1024,        EMMC_USER_PART},/* userdata           4G    p74*/
#else
  #ifdef CONFIG_USE_EROFS
  {PART_VENDOR_A,                   936*1024,         480*1024,        EMMC_USER_PART},/* vendor           480M    p64*/
  {PART_ODM_A,                     1416*1024,         136*1024,        EMMC_USER_PART},/* odm              136M    p65*/
  {PART_CUST_A,                    1552*1024,         136*1024,        EMMC_USER_PART},/* cust             136M    p66*/
  #ifdef CONFIG_MARKET_INTERNAL
  {PART_SYSTEM_A,                  1688*1024,        2040*1024,        EMMC_USER_PART},/* system          2040M    p67*/
  {PART_PRODUCT_A,                 3728*1024,        1024*1024,        EMMC_USER_PART},/* product         1024M    p68*/
  {PART_VERSION_A,                 4752*1024,         200*1024,        EMMC_USER_PART},/* version          200M    p69*/
  {PART_PRELOAD_A,                 4952*1024,         928*1024,        EMMC_USER_PART},/* preload_a        928M    p70*/
  {PART_USERDATA,                  5880*1024,  (4UL)*1024*1024,        EMMC_USER_PART},/* userdata         4G      p71*/
  #elif defined CONFIG_MARKET_OVERSEA
  {PART_SYSTEM_A,                  1688*1024,        2688*1024,        EMMC_USER_PART},/* system          2688M    p67*/
  {PART_PRODUCT_A,                 4376*1024,         736*1024,        EMMC_USER_PART},/* product          736M    p68*/
  {PART_VERSION_A,                 5112*1024,         200*1024,        EMMC_USER_PART},/* version          200M    p69*/
  {PART_PRELOAD_A,                 5312*1024,         928*1024,        EMMC_USER_PART},/* preload_a        928M    p70*/
  {PART_USERDATA,                  6240*1024,  (4UL)*1024*1024,        EMMC_USER_PART},/* userdata         4G      p71*/
  #elif defined CONFIG_MARKET_FULL_INTERNAL
  {PART_SYSTEM,                    1688*1024,        3352*1024,        EMMC_USER_PART},/* system          3352M    p67*/
  {PART_PRODUCT,                   5040*1024,        1472*1024,        EMMC_USER_PART},/* product         1472M    p68*/
  {PART_VERSION,                   6512*1024,         576*1024,        EMMC_USER_PART},/* version          576M    p69*/
  {PART_PRELOAD,                   7088*1024,        1144*1024,        EMMC_USER_PART},/* preload         1144M    p70*/
  {PART_USERDATA,                  8232*1024,  (4UL)*1024*1024,        EMMC_USER_PART},/* userdata         4G      p71*/
  #elif defined CONFIG_MARKET_FULL_OVERSEA
  {PART_SYSTEM,                    1688*1024,        4096*1024,        EMMC_USER_PART},/* system          4096M    p67*/
  {PART_PRODUCT,                   5784*1024,        1328*1024,        EMMC_USER_PART},/* product         1328M    p68*/
  {PART_VERSION,                   7112*1024,         576*1024,        EMMC_USER_PART},/* version          576M    p69*/
  {PART_PRELOAD,                   7688*1024,        1144*1024,        EMMC_USER_PART},/* preload         1144M    p70*/
  {PART_USERDATA,                  8832*1024,  (4UL)*1024*1024,        EMMC_USER_PART},/* userdata        4G       p71*/
  #else
  {PART_SYSTEM,                    1688*1024,        5520*1024,        EMMC_USER_PART},/* system          5520M    p67*/
  {PART_PRODUCT,                   7208*1024,        1328*1024,        EMMC_USER_PART},/* product         1328M    p68*/
  {PART_VERSION,                   8536*1024,         576*1024,        EMMC_USER_PART},/* version          576M    p69*/
  {PART_PRELOAD,                   9112*1024,        1144*1024,        EMMC_USER_PART},/* preload         1144M    p70*/
  {PART_USERDATA,                 10256*1024,  (4UL)*1024*1024,        EMMC_USER_PART},/* userdata           4G    p71*/
  #endif
  #else /* do not use erofs */
  {PART_VENDOR,                     936*1024,        1232*1024,        EMMC_USER_PART},/* vendor          1232M    p64*/
  {PART_ODM,                       2168*1024,         192*1024,        EMMC_USER_PART},/* odm              192M    p65*/
  {PART_CUST,                      2360*1024,         192*1024,        EMMC_USER_PART},/* cust             192M    p66*/
  #ifdef CONFIG_MARKET_INTERNAL
  {PART_SYSTEM,                    2552*1024,        3360*1024,        EMMC_USER_PART},/* system          3360M    p67*/
  {PART_PRODUCT,                   5912*1024,        1312*1024,        EMMC_USER_PART},/* product         1312M    p68*/
  {PART_VERSION,                   7224*1024,         576*1024,        EMMC_USER_PART},/* version          576M    p69*/
  {PART_PRELOAD,                   7800*1024,         928*1024,        EMMC_USER_PART},/* preload          928M    p70*/
  {PART_USERDATA,                  8728*1024,  (4UL)*1024*1024,        EMMC_USER_PART},/* userdata         4G      p71*/
  #elif defined CONFIG_MARKET_OVERSEA
  {PART_SYSTEM,                    2552*1024,        4384*1024,        EMMC_USER_PART},/* system          4384M    p67*/
  {PART_PRODUCT,                   6936*1024,         952*1024,        EMMC_USER_PART},/* product          952M    p68*/
  {PART_VERSION,                   7888*1024,         576*1024,        EMMC_USER_PART},/* version          576M    p69*/
  {PART_PRELOAD,                   8464*1024,         928*1024,        EMMC_USER_PART},/* preload          928M    p70*/
  {PART_USERDATA,                  9392*1024,  (4UL)*1024*1024,        EMMC_USER_PART},/* userdata          4G     p71*/
  #elif defined CONFIG_MARKET_FULL_INTERNAL
  {PART_SYSTEM,                    2552*1024,        4784*1024,        EMMC_USER_PART},/* system          4784M    p67*/
  {PART_PRODUCT,                   7336*1024,        1472*1024,        EMMC_USER_PART},/* product         1472M    p68*/
  {PART_VERSION,                   8808*1024,         576*1024,        EMMC_USER_PART},/* version          576M    p69*/
  {PART_PRELOAD,                   9384*1024,        1144*1024,        EMMC_USER_PART},/* preload         1144M    p70*/
  {PART_USERDATA,                 10528*1024,  (4UL)*1024*1024,        EMMC_USER_PART},/* userdata         4G      p71*/
  #elif defined CONFIG_MARKET_FULL_OVERSEA
  {PART_SYSTEM,                    2552*1024,        5848*1024,        EMMC_USER_PART},/* system          5848M    p67*/
  {PART_PRODUCT,                   8400*1024,        1328*1024,        EMMC_USER_PART},/* product         1328M    p68*/
  {PART_VERSION,                   9728*1024,         576*1024,        EMMC_USER_PART},/* version          576M    p69*/
  {PART_PRELOAD,                  10304*1024,        1144*1024,        EMMC_USER_PART},/* preload         1144M    p70*/
  {PART_USERDATA,                 11448*1024,  (4UL)*1024*1024,        EMMC_USER_PART},/* userdata         4G      p71*/
  #else
  {PART_SYSTEM,                    2552*1024,        5520*1024,        EMMC_USER_PART},/* system          5520M    p67*/
  {PART_PRODUCT,                   8072*1024,        1328*1024,        EMMC_USER_PART},/* product         1328M    p68*/
  {PART_VERSION,                   9400*1024,         576*1024,        EMMC_USER_PART},/* version          576M    p69*/
  {PART_PRELOAD,                   9976*1024,        1144*1024,        EMMC_USER_PART},/* preload         1144M    p70*/
  {PART_USERDATA,                 11120*1024,  (4UL)*1024*1024,        EMMC_USER_PART},/* userdata           4G    p71*/
  #endif
  #endif
  #endif
  {"0", 0, 0, 0},                                        /* total 11848M*/
};
#ifdef CONFIG_HISI_STORAGE_UFS_PARTITION
static const struct partition partition_table_ufs[] =
{
  {PART_XLOADER,                           0,           2*1024,        UFS_PART_0},
  {PART_RESERVED0,                         0,           2*1024,        UFS_PART_1},
  {PART_PTABLE,                            0,              512,        UFS_PART_2},/* ptable                    512K    */
  {PART_FRP,                             512,              512,        UFS_PART_2},/* frp                       512K    p1*/
  {PART_PERSIST,                      1*1024,           6*1024,        UFS_PART_2},/* persist                  6144K    p2*/
  {PART_RESERVED1,                    7*1024,             1024,        UFS_PART_2},/* reserved1                1024K    p3*/
  {PART_PTABLE_LU3,                        0,              512,        UFS_PART_3},/* ptable_lu3                512K    p0*/
  {PART_VRL,                             512,              512,        UFS_PART_3},/* vrl                       512K    p1*/
  {PART_VRL_BACKUP,                     1024,              512,        UFS_PART_3},/* vrl backup                512K    p2*/
  {PART_MODEM_SECURE,                   1536,             8704,        UFS_PART_3},/* modem_secure             8704K    p3*/
  {PART_NVME,                        10*1024,           5*1024,        UFS_PART_3},/* nvme                        5M    p4*/
  {PART_CTF,                         15*1024,           1*1024,        UFS_PART_3},/* PART_CTF                    1M    p5*/
  {PART_OEMINFO,                     16*1024,          96*1024,        UFS_PART_3},/* oeminfo                    96M    p6*/
  {PART_SECURE_STORAGE,             112*1024,          32*1024,        UFS_PART_3},/* secure storage             32M    p7*/
  {PART_MODEMNVM_FACTORY,           144*1024,          16*1024,        UFS_PART_3},/* modemnvm factory           16M    p8*/
  {PART_MODEMNVM_BACKUP,            160*1024,          16*1024,        UFS_PART_3},/* modemnvm backup            16M    p9*/
  {PART_MODEMNVM_IMG,               176*1024,          34*1024,        UFS_PART_3},/* modemnvm img               34M    p10*/
  {PART_HISEE_ENCOS,                210*1024,           4*1024,        UFS_PART_3},/* hisee_encos                 4M    p11*/
  {PART_VERITYKEY,                  214*1024,           1*1024,        UFS_PART_3},/* veritykey                   1M    p12*/
  {PART_DDR_PARA,                   215*1024,           1*1024,        UFS_PART_3},/* ddr_para                    1M    p13*/
  {PART_LOWPOWER_PARA,              216*1024,           1*1024,        UFS_PART_3},/* lowpower_para               1M    p14*/
  {PART_BATT_TP_PARA,               217*1024,           1*1024,        UFS_PART_3},/* batt_tp_para                1M    p15*/
  {PART_RESERVED2,                  218*1024,          25*1024,        UFS_PART_3},/* reserved2                  25M    p16*/
  {PART_SPLASH2,                    243*1024,          80*1024,        UFS_PART_3},/* splash2                    80M    p17*/
  {PART_BOOTFAIL_INFO,              323*1024,           2*1024,        UFS_PART_3},/* bootfail info              2MB    p18*/
  {PART_MISC,                       325*1024,           2*1024,        UFS_PART_3},/* misc                        2M    p19*/
  {PART_DFX,                        327*1024,          16*1024,        UFS_PART_3},/* dfx                        16M    p20*/
  {PART_RRECORD,                    343*1024,          16*1024,        UFS_PART_3},/* rrecord                    16M    p21*/
  {PART_CACHE,                      359*1024,         104*1024,        UFS_PART_3},/* cache                     104M    p22*/
  {PART_FW_LPM3,                    463*1024,             1024,        UFS_PART_3},/* fw_lpm3                  1024K    p23*/
  {PART_RESERVED3,                  464*1024,           5*1024,        UFS_PART_3},/* reserved3A               5120K    p24*/
  {PART_NPU,                        469*1024,           8*1024,        UFS_PART_3},/* npu                         8M    p25*/
  {PART_HIEPS,                      477*1024,           2*1024,        UFS_PART_3},/* hieps                       2M    p26*/
  {PART_IVP,                        479*1024,           2*1024,        UFS_PART_3},/* ivp                         2M    p27*/
  {PART_HDCP,                       481*1024,           1*1024,        UFS_PART_3},/* PART_HDCP                   1M    p28*/
  {PART_HISEE_IMG,                  482*1024,           4*1024,        UFS_PART_3},/* part_hisee_img              4M    p29*/
  {PART_HHEE,                       486*1024,           4*1024,        UFS_PART_3},/* hhee                        4M    p30*/
  {PART_HISEE_FS,                   490*1024,           8*1024,        UFS_PART_3},/* hisee_fs                    8M    p31*/
  {PART_FASTBOOT,                   498*1024,          12*1024,        UFS_PART_3},/* fastboot                   12M    p32*/
  {PART_VECTOR,                     510*1024,           4*1024,        UFS_PART_3},/* avs vector                  4M    p33*/
  {PART_ISP_BOOT,                   514*1024,           2*1024,        UFS_PART_3},/* isp_boot                    2M    p34*/
  {PART_ISP_FIRMWARE,               516*1024,          14*1024,        UFS_PART_3},/* isp_firmware               14M    p35*/
  {PART_FW_HIFI,                    530*1024,          12*1024,        UFS_PART_3},/* hifi                       12M    p36*/
  {PART_TEEOS,                      542*1024,           8*1024,        UFS_PART_3},/* teeos                       8M    p37*/
  {PART_SENSORHUB,                  550*1024,          16*1024,        UFS_PART_3},/* sensorhub                  16M    p38*/
#ifdef CONFIG_SANITIZER_ENABLE
  {PART_ERECOVERY_KERNEL,           566*1024,          24*1024,        UFS_PART_3},/* erecovery_kernel           24M    p39*/
  {PART_ERECOVERY_RAMDISK,          590*1024,          32*1024,        UFS_PART_3},/* erecovery_ramdisk          32M    p40*/
  {PART_ERECOVERY_VENDOR,           622*1024,           8*1024,        UFS_PART_3},/* erecovery_vendor            8M    p41*/
  {PART_KERNEL,                     630*1024,          32*1024,        UFS_PART_3},/* kernel                     32M    p42*/
#else
  {PART_ERECOVERY_KERNEL,           566*1024,          24*1024,        UFS_PART_3},/* erecovery_kernel           24M    p39*/
  {PART_ERECOVERY_RAMDISK,          590*1024,          32*1024,        UFS_PART_3},/* erecovery_ramdisk          32M    p40*/
  {PART_ERECOVERY_VENDOR,           622*1024,          16*1024,        UFS_PART_3},/* erecovery_vendor           16M    p41*/
  {PART_KERNEL,                     638*1024,          24*1024,        UFS_PART_3},/* kernel                     24M    p42*/
#endif
  {PART_ENG_SYSTEM,                 662*1024,          12*1024,        UFS_PART_3},/* eng_system                 12M    p43*/
  {PART_RECOVERY_RAMDISK,           674*1024,          32*1024,        UFS_PART_3},/* recovery_ramdisk           32M    p44*/
  {PART_RECOVERY_VENDOR,            706*1024,          16*1024,        UFS_PART_3},/* recovery_vendor            16M    p45*/
  {PART_DTS,                        722*1024,           1*1024,        UFS_PART_3},/* dtimage                     1M    p46*/
  {PART_DTO,                        723*1024,          20*1024,        UFS_PART_3},/* dtoimage                   20M    p47*/
  {PART_TRUSTFIRMWARE,              743*1024,           2*1024,        UFS_PART_3},/* trustfirmware               2M    p48*/
  {PART_MODEM_FW,                   745*1024,          56*1024,        UFS_PART_3},/* modem_fw                   56M    p49*/
  {PART_ENG_VENDOR,                 801*1024,          20*1024,        UFS_PART_3},/* eng_vendor                 20M    p50*/
  {PART_MODEM_PATCH_NV,             821*1024,           4*1024,        UFS_PART_3},/* modem_patch_nv              4M    p51*/
  {PART_MODEM_DRIVER,               825*1024,          20*1024,        UFS_PART_3},/* modem_driver               20M    p52*/
  {PART_RESERVED4,                  845*1024,          11*1024,        UFS_PART_3},/* reserved4A                 11M    p53*/
  {PART_RECOVERY_VBMETA,            856*1024,           2*1024,        UFS_PART_3},/* recovery_vbmeta             2M    p54*/
  {PART_ERECOVERY_VBMETA,           858*1024,           2*1024,        UFS_PART_3},/* erecovery_vbmeta            2M    p55*/
  {PART_VBMETA,                     860*1024,           4*1024,        UFS_PART_3},/* vbmeta                      4M    p56*/
  {PART_MODEMNVM_UPDATE,            864*1024,          16*1024,        UFS_PART_3},/* modemnvm_update            16M    p57*/
  {PART_MODEMNVM_CUST,              880*1024,          16*1024,        UFS_PART_3},/* modemnvm_cust              16M    p58*/
  {PART_PATCH,                      896*1024,          32*1024,        UFS_PART_3},/* patch                      32M    p59*/
#ifdef CONFIG_FACTORY_MODE
  {PART_VENDOR,                     928*1024,        1232*1024,        UFS_PART_3},/* vendor                   1232M    p60*/
  {PART_ODM,                       2160*1024,         192*1024,        UFS_PART_3},/* odm                       192M    p61*/
  {PART_CUST,                      2352*1024,         192*1024,        UFS_PART_3},/* cust                      192M    p62*/
  {PART_SYSTEM,                    2544*1024,        5520*1024,        UFS_PART_3},/* system                   5520M    p63*/
  {PART_PRODUCT,                   8064*1024,        1328*1024,        UFS_PART_3},/* product                  1328M    p64*/
  {PART_VERSION,                   9392*1024,         576*1024,        UFS_PART_3},/* version                   576M    p65*/
  {PART_PRELOAD,                   9968*1024,        1144*1024,        UFS_PART_3},/* preload                  1144M    p66*/
  {PART_HIBENCH_IMG,              11112*1024,         128*1024,        UFS_PART_3},/* hibench_img               128M    p67*/
  {PART_HIBENCH_DATA,             11240*1024,         512*1024,        UFS_PART_3},/* hibench_data              512M    p68*/
  {PART_FLASH_AGEING,             11752*1024,         512*1024,        UFS_PART_3},/* FLASH_AGEING              512M    p69*/
  {PART_USERDATA,                 12264*1024,  (4UL)*1024*1024,        UFS_PART_3},/* userdata                    4G    p70*/
#else
#ifdef CONFIG_USE_EROFS
  {PART_VENDOR,                     928*1024,         480*1024,        UFS_PART_3},/* vendor                    480M    p60*/
  {PART_ODM,                       1408*1024,         136*1024,        UFS_PART_3},/* odm                       136M    p61*/
  {PART_CUST,                      1544*1024,         136*1024,        UFS_PART_3},/* cust                      136M    p62*/
  #ifdef CONFIG_MARKET_INTERNAL
  {PART_SYSTEM,                    1680*1024,        2040*1024,        UFS_PART_3},/* system                   2040M    p63*/
  {PART_PRODUCT,                   3720*1024,        1024*1024,        UFS_PART_3},/* product                  1024M    p64*/
  {PART_VERSION,                   4744*1024,         200*1024,        UFS_PART_3},/* version                   200M    p65*/
  {PART_PRELOAD,                   4944*1024,         928*1024,        UFS_PART_3},/* preload                   928M    p66*/
  {PART_USERDATA,                  5872*1024,  (4UL)*1024*1024,        UFS_PART_3},/* userdata                    4G    p67*/
  #elif defined CONFIG_MARKET_OVERSEA
  {PART_SYSTEM,                    1680*1024,        2688*1024,        UFS_PART_3},/* system                   2688M    p63*/
  {PART_PRODUCT,                   4368*1024,         736*1024,        UFS_PART_3},/* product                   736M    p64*/
  {PART_VERSION,                   5104*1024,         200*1024,        UFS_PART_3},/* version                   200M    p65*/
  {PART_PRELOAD,                   5304*1024,         928*1024,        UFS_PART_3},/* preload                   928M    p66*/
  {PART_USERDATA,                  6232*1024,  (4UL)*1024*1024,        UFS_PART_3},/* userdata                    4G    p67*/
  #elif defined CONFIG_MARKET_FULL_INTERNAL
  {PART_SYSTEM,                    1680*1024,        3352*1024,        UFS_PART_3},/* system                   3352M    p63*/
  {PART_PRODUCT,                   5032*1024,        1472*1024,        UFS_PART_3},/* product                  1472M    p64*/
  {PART_VERSION,                   6504*1024,         576*1024,        UFS_PART_3},/* version                   576M    p65*/
  {PART_PRELOAD,                   7080*1024,        1144*1024,        UFS_PART_3},/* preload                  1144M    p66*/
  {PART_USERDATA,                  8224*1024,  (4UL)*1024*1024,        UFS_PART_3},/* userdata                    4G    p67*/
  #elif defined CONFIG_MARKET_FULL_OVERSEA
  {PART_SYSTEM,                    1680*1024,        4096*1024,        UFS_PART_3},/* system                   4096M    p63*/
  {PART_PRODUCT,                   5776*1024,        1328*1024,        UFS_PART_3},/* product                  1328M    p64*/
  {PART_VERSION,                   7104*1024,         576*1024,        UFS_PART_3},/* version                   576M    p65*/
  {PART_PRELOAD,                   7680*1024,        1144*1024,        UFS_PART_3},/* preload                  1144M    p66*/
  {PART_USERDATA,                  8824*1024,  (4UL)*1024*1024,        UFS_PART_3},/* userdata                    4G    p67*/
  #else
  {PART_SYSTEM,                    1680*1024,        5520*1024,         UFS_PART_3},/* system                   912M    p63*/
  {PART_PRODUCT,                   7200*1024,        1328*1024,         UFS_PART_3},/* product                 1328M    p64*/
  {PART_VERSION,                   8528*1024,         576*1024,         UFS_PART_3},/* version                  576M    p65*/
  {PART_PRELOAD,                   9104*1024,        1144*1024,         UFS_PART_3},/* preload                 1144M    p66*/
  {PART_USERDATA,                 10248*1024,  (4UL)*1024*1024,         UFS_PART_3},/* userdata                   4G    p67*/
  #endif
  #else /* do not use erofs */
  {PART_VENDOR,                     928*1024,        1232*1024,         UFS_PART_3},/* vendor                  1232M    p60*/
  {PART_ODM,                       2160*1024,         192*1024,         UFS_PART_3},/* odm                      192M    p61*/
  {PART_CUST,                      2352*1024,         192*1024,         UFS_PART_3},/* cust                     192M    p62*/
  #ifdef CONFIG_MARKET_INTERNAL
  {PART_SYSTEM,                    2544*1024,        3360*1024,         UFS_PART_3},/* system                  3360M    p63*/
  {PART_PRODUCT,                   5904*1024,        1312*1024,         UFS_PART_3},/* product                 1312M    p64*/
  {PART_VERSION,                   7216*1024,         576*1024,         UFS_PART_3},/* version                  576M    p65*/
  {PART_PRELOAD,                   7792*1024,         928*1024,         UFS_PART_3},/* preload                  928M    p66*/
  {PART_USERDATA,                  8720*1024,  (4UL)*1024*1024,         UFS_PART_3},/* userdata                   4G    p67*/
  #elif defined CONFIG_MARKET_OVERSEA
  {PART_SYSTEM,                    2544*1024,        4384*1024,         UFS_PART_3},/* system                  4384M    p63*/
  {PART_PRODUCT,                   6928*1024,         952*1024,         UFS_PART_3},/* product                  952M    p64*/
  {PART_VERSION,                   7880*1024,         576*1024,         UFS_PART_3},/* version                  576M    p65*/
  {PART_PRELOAD,                   8456*1024,         928*1024,         UFS_PART_3},/* preload                  928M    p66*/
  {PART_USERDATA,                  9384*1024,  (4UL)*1024*1024,         UFS_PART_3},/* userdata                   4G    p67*/
  #elif defined CONFIG_MARKET_FULL_INTERNAL
  {PART_SYSTEM,                    2544*1024,        4784*1024,         UFS_PART_3},/* system                  4784M    p63*/
  {PART_PRODUCT,                   7328*1024,        1472*1024,         UFS_PART_3},/* product                 1472M    p64*/
  {PART_VERSION,                   8800*1024,         576*1024,         UFS_PART_3},/* version                  576M    p65*/
  {PART_PRELOAD,                   9376*1024,        1144*1024,         UFS_PART_3},/* preload                 1144M    p66*/
  {PART_USERDATA,                 10520*1024,  (4UL)*1024*1024,         UFS_PART_3},/* userdata                   4G    p67*/
  #elif defined CONFIG_MARKET_FULL_OVERSEA
  {PART_SYSTEM,                    2544*1024,        5848*1024,         UFS_PART_3},/* system                  5848M    p63*/
  {PART_PRODUCT,                   8392*1024,        1328*1024,         UFS_PART_3},/* product                 1328M    p64*/
  {PART_VERSION,                   9720*1024,         576*1024,         UFS_PART_3},/* version                  576M    p65*/
  {PART_PRELOAD,                  10296*1024,        1144*1024,         UFS_PART_3},/* preload                 1144M    p66*/
  {PART_USERDATA,                 11440*1024,  (4UL)*1024*1024,         UFS_PART_3},/* userdata                   4G    p67*/
  #else
  {PART_SYSTEM,                    2544*1024,        5520*1024,         UFS_PART_3},/* system                  5520M    p63*/
  {PART_PRODUCT,                   8064*1024,        1328*1024,         UFS_PART_3},/* product                 1328M    p64*/
  {PART_VERSION,                   9392*1024,         576*1024,         UFS_PART_3},/* version                  576M    p65*/
  {PART_PRELOAD,                   9968*1024,        1144*1024,         UFS_PART_3},/* preload                 1144M    p66*/
  {PART_USERDATA,                 11112*1024,  (4UL)*1024*1024,         UFS_PART_3},/* userdata                   4G    p67*/
  #endif
  #endif
  #endif
  {"0", 0, 0, 0},
};
#endif

#endif
