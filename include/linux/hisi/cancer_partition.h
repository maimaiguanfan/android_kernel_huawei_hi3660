#ifndef _CANCER_PARTITION_H_
#define _CANCER_PARTITION_H_

#include "hisi_partition.h"
#include "partition_def.h"

static const struct partition partition_table_emmc[] =
{
  {PART_XLOADER_A,                         0,           2*1024,        EMMC_BOOT_MAJOR_PART},
  {PART_XLOADER_B,                         0,           2*1024,        EMMC_BOOT_BACKUP_PART},
  {PART_PTABLE,                            0,              512,        EMMC_USER_PART},/* ptable          512K   */
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
  {PART_FW_LPM3_A,                  471*1024,             1024,        EMMC_USER_PART},/* fw_lpm3_a       256K   p27*/
  {PART_RESERVED3_A,                472*1024,           7*1024,        EMMC_USER_PART},/* reserved3A     7936K   p28*/
  {PART_IVP,                        479*1024,           2*1024,        EMMC_USER_PART},/* ivp               2M   p29*/
  {PART_HDCP_A,                     481*1024,           1*1024,        EMMC_USER_PART},/* PART_HDCP_A       1M   p30*/
  {PART_HISEE_IMG_A,                482*1024,           4*1024,        EMMC_USER_PART},/* part_hisee_img_a  4M   p31*/
  {PART_HHEE_A,                     486*1024,           4*1024,        EMMC_USER_PART},/* hhee_a            4M   p32*/
  {PART_HISEE_FS_A,                 490*1024,           8*1024,        EMMC_USER_PART},/* hisee_fs          8M   p33*/
  {PART_FASTBOOT_A,                 498*1024,          12*1024,        EMMC_USER_PART},/* fastboot         12M   p34*/
  {PART_VECTOR_A,                   510*1024,           4*1024,        EMMC_USER_PART},/* vector_a          4M   p35*/
  {PART_ISP_BOOT_A,                 514*1024,           2*1024,        EMMC_USER_PART},/* isp_boot          2M   p36*/
  {PART_ISP_FIRMWARE_A,             516*1024,          14*1024,        EMMC_USER_PART},/* isp_firmware     14M   p37*/
  {PART_FW_HIFI_A,                  530*1024,          12*1024,        EMMC_USER_PART},/* hifi             12M   p38*/
  {PART_TEEOS_A,                    542*1024,           8*1024,        EMMC_USER_PART},/* teeos             8M   p39*/
  {PART_SENSORHUB_A,                550*1024,          16*1024,        EMMC_USER_PART},/* sensorhub        16M   p40*/
#ifdef CONFIG_SANITIZER_ENABLE
  {PART_ERECOVERY_KERNEL_A,         566*1024,          24*1024,        EMMC_USER_PART},/* erecovery_kernel  24M    p41*/
  {PART_ERECOVERY_RAMDISK_A,        590*1024,          32*1024,        EMMC_USER_PART},/* erecovery_ramdisk 32M    p42*/
  {PART_ERECOVERY_VENDOR_A,         622*1024,           8*1024,        EMMC_USER_PART},/* erecovery_vendor   8M    p43*/
  {PART_KERNEL_A,                   630*1024,          32*1024,        EMMC_USER_PART},/* kernel            32M    p44*/
#else
  {PART_ERECOVERY_KERNEL_A,         566*1024,          24*1024,        EMMC_USER_PART},/* erecovery_kernel  24M    p41*/
  {PART_ERECOVERY_RAMDISK_A,        590*1024,          32*1024,        EMMC_USER_PART},/* erecovery_ramdisk 32M    p42*/
  {PART_ERECOVERY_VENDOR_A,         622*1024,          16*1024,        EMMC_USER_PART},/* erecovery_vendor  16M    p43*/
  {PART_KERNEL_A,                   638*1024,          24*1024,        EMMC_USER_PART},/* kernel            24M    p44*/
#endif
  {PART_ENG_SYSTEM_A,               662*1024,          12*1024,        EMMC_USER_PART},/* eng_system        12M    p45*/
  {PART_RECOVERY_RAMDISK_A,         674*1024,          32*1024,        EMMC_USER_PART},/* recovery_ramdisk  32M    p46*/
  {PART_RECOVERY_VENDOR_A,          706*1024,          16*1024,        EMMC_USER_PART},/* recovery_vendor   16M    p47*/
  {PART_DTS_A,                      722*1024,           1*1024,        EMMC_USER_PART},/* dtimage            1M    p48*/
  {PART_DTO_A,                      723*1024,          20*1024,        EMMC_USER_PART},/* dtoimage          20M    p49*/
  {PART_TRUSTFIRMWARE_A,            743*1024,           2*1024,        EMMC_USER_PART},/* trustfirmware      2M    p50*/
  {PART_MODEM_FW_A,                 745*1024,          56*1024,        EMMC_USER_PART},/* modem_fw          56M    p51*/
  {PART_ENG_VENDOR_A,               801*1024,          12*1024,        EMMC_USER_PART},/* eng_vendor        12M    p52*/
  {PART_MODEM_PATCH_NV_A,           813*1024,           4*1024,        EMMC_USER_PART},/* modem_patch_nv_a   4M    p53*/
  {PART_MODEM_DRIVER_A,             817*1024,          20*1024,        EMMC_USER_PART},/* modem_driver_a    20M    p54*/
  {PART_RESERVED4_A,                837*1024,          19*1024,        EMMC_USER_PART},/* reserved4A        11M    p55*/
  {PART_RECOVERY_VBMETA_A,          856*1024,           2*1024,        EMMC_USER_PART},/* recovery_vbmeta_a  2M    p56*/
  {PART_ERECOVERY_VBMETA_A,         858*1024,           2*1024,        EMMC_USER_PART},/* erecovery_vbmeta_a 2M    p57*/
  {PART_VBMETA_A,                   860*1024,           4*1024,        EMMC_USER_PART},/* PART_VBMETA_A      4M    p58*/
  {PART_MODEMNVM_UPDATE_A,          864*1024,          16*1024,        EMMC_USER_PART},/* modemnvm_update_a 16M    p59*/
  {PART_MODEMNVM_CUST_A,            880*1024,          16*1024,        EMMC_USER_PART},/* modemnvm_cust_a   16M    p60*/
#ifdef CONFIG_FACTORY_MODE
  {PART_PATCH_A,                    896*1024,          32*1024,        EMMC_USER_PART},/* patch             32M    p61*/
  {PART_PREAS,                      928*1024,        1216*1024,        EMMC_USER_PART},/* preas           1216M    p62*/
  {PART_PREAVS,                    2144*1024,          32*1024,        EMMC_USER_PART},/* preavs            32M    p63*/
  {PART_VENDOR_A,                  2176*1024,        1232*1024,        EMMC_USER_PART},/* vendor          1232M    p64*/
  {PART_ODM_A,                     3408*1024,         192*1024,        EMMC_USER_PART},/* odm              192M    p65*/
  {PART_CUST_A,                    3600*1024,         192*1024,        EMMC_USER_PART},/* cust             192M    p66*/
  {PART_SYSTEM_A,                  3792*1024,        3568*1024,        EMMC_USER_PART},/* system          3568M    p67*/
  {PART_PRODUCT_A,                 7360*1024,        2360*1024,        EMMC_USER_PART},/* product         2360M    p68*/
  {PART_VERSION_A,                 9720*1024,         576*1024,        EMMC_USER_PART},/* version          576M    p69*/
  {PART_PRELOAD_A,                10296*1024,        1144*1024,        EMMC_USER_PART},/* preload_a       1144M    p70*/
  {PART_HIBENCH_IMG,              11440*1024,         128*1024,        EMMC_USER_PART},/* hibench_img      128M    p71*/
  {PART_HIBENCH_DATA,             11568*1024,         512*1024,        EMMC_USER_PART},/* hibench_data     512M    p72*/
  {PART_FLASH_AGEING,             12080*1024,         512*1024,        EMMC_USER_PART},/* FLASH_AGEING     512M    p73*/
  {PART_USERDATA,                 12592*1024,  (4UL)*1024*1024,        EMMC_USER_PART},/* userdata           4G    p74*/
#else
#ifdef CONFIG_USE_EROFS
#ifdef CONFIG_UPDATE_PRODUCT_EROFS
#ifdef CONFIG_MARKET_INTERNAL
  {PART_PATCH_A,                    896*1024,         200*1024,        EMMC_USER_PART},/* patch            200M    p61*/
  {PART_PREAS,                     1096*1024,         360*1024,        EMMC_USER_PART},/* preas            360M    p62*/
  {PART_PREAVS,                    1456*1024,          32*1024,        EMMC_USER_PART},/* preavs            32M    p63*/
  {PART_VENDOR_A,                  1488*1024,        1064*1024,        EMMC_USER_PART},/* vendor          1064M    p64*/
  {PART_ODM_A,                     2552*1024,         192*1024,        EMMC_USER_PART},/* odm              192M    p65*/
  {PART_CUST_A,                    2744*1024,         192*1024,        EMMC_USER_PART},/* cust             192M    p66*/
  {PART_SYSTEM_A,                  2936*1024,        3760*1024,        EMMC_USER_PART},/* system          3760M    p67*/
  {PART_PRODUCT_A,                 6696*1024,        2104*1024,        EMMC_USER_PART},/* product         2104M    p68*/
  {PART_VERSION_A,                 8800*1024,         576*1024,        EMMC_USER_PART},/* version          576M    p69*/
  {PART_PRELOAD_A,                 9376*1024,        1144*1024,        EMMC_USER_PART},/* preload_a       1144M    p70*/
  {PART_USERDATA,                 10520*1024,  (4UL)*1024*1024,        EMMC_USER_PART},/* userdata           4G    p71*/
#else
  {PART_PATCH_A,                    896*1024,         200*1024,        EMMC_USER_PART},/* patch            200M    p61*/
  {PART_PREAS,                     1096*1024,        1216*1024,        EMMC_USER_PART},/* preas           1216M    p62*/
  {PART_PREAVS,                    2312*1024,          32*1024,        EMMC_USER_PART},/* preavs            32M    p63*/
  {PART_VENDOR_A,                  2344*1024,        1064*1024,        EMMC_USER_PART},/* vendor          1064M    p64*/
  {PART_ODM_A,                     3408*1024,         192*1024,        EMMC_USER_PART},/* odm              192M    p65*/
  {PART_CUST_A,                    3600*1024,         192*1024,        EMMC_USER_PART},/* cust             192M    p66*/
  {PART_SYSTEM_A,                  3792*1024,        3568*1024,        EMMC_USER_PART},/* system          3568M    p67*/
  {PART_PRODUCT_A,                 7360*1024,        2360*1024,        EMMC_USER_PART},/* product         2360M    p68*/
  {PART_VERSION_A,                 9720*1024,         576*1024,        EMMC_USER_PART},/* version          576M    p69*/
  {PART_PRELOAD_A,                10296*1024,        1144*1024,        EMMC_USER_PART},/* preload_a       1144M    p70*/
  {PART_USERDATA,                 11440*1024,  (4UL)*1024*1024,        EMMC_USER_PART},/* userdata           4G    p71*/
#endif
#else // CONFIG_UPDATE_PRODUCT_EROFS
#ifdef CONFIG_MARKET_INTERNAL
  {PART_PATCH_A,                    896*1024,          32*1024,        EMMC_USER_PART},/* patch             32M    p61*/
  {PART_PREAS,                      928*1024,         296*1024,        EMMC_USER_PART},/* preas            296M    p62*/
  {PART_PREAVS,                    1224*1024,          32*1024,        EMMC_USER_PART},/* preavs            32M    p63*/
  {PART_VENDOR_A,                  1256*1024,         864*1024,        EMMC_USER_PART},/* vendor           864M    p64*/
  {PART_ODM_A,                     2120*1024,         136*1024,        EMMC_USER_PART},/* odm              136M    p65*/
  {PART_CUST_A,                    2256*1024,          88*1024,        EMMC_USER_PART},/* cust              88M    p66*/
  {PART_SYSTEM_A,                  2344*1024,        2544*1024,        EMMC_USER_PART},/* system          2544M    p67*/
  {PART_PRODUCT_A,                 4888*1024,        2000*1024,        EMMC_USER_PART},/* product         2000M    p68*/
  {PART_VERSION_A,                 6888*1024,         576*1024,        EMMC_USER_PART},/* version          576M    p69*/
  {PART_PRELOAD_A,                 7464*1024,        1144*1024,        EMMC_USER_PART},/* preload_a       1144M    p70*/
  {PART_USERDATA,                  8608*1024,  (4UL)*1024*1024,        EMMC_USER_PART},/* userdata           4G    p71*/
#else
  {PART_PATCH_A,                    896*1024,          32*1024,        EMMC_USER_PART},/* patch             32M    p61*/
  {PART_PREAS,                      928*1024,         976*1024,        EMMC_USER_PART},/* preas            976M    p62*/
  {PART_PREAVS,                    1904*1024,          32*1024,        EMMC_USER_PART},/* preavs            32M    p63*/
  {PART_VENDOR_A,                  1936*1024,         864*1024,        EMMC_USER_PART},/* vendor           864M    p64*/
  {PART_ODM_A,                     2800*1024,         136*1024,        EMMC_USER_PART},/* odm              136M    p65*/
  {PART_CUST_A,                    2936*1024,          88*1024,        EMMC_USER_PART},/* cust              88M    p66*/
  {PART_SYSTEM_A,                  3024*1024,        2312*1024,        EMMC_USER_PART},/* system          2312M    p67*/
  {PART_PRODUCT_A,                 5336*1024,        2152*1024,        EMMC_USER_PART},/* product         2152M    p68*/
  {PART_VERSION_A,                 7488*1024,         576*1024,        EMMC_USER_PART},/* version          576M    p69*/
  {PART_PRELOAD_A,                 8064*1024,        1144*1024,        EMMC_USER_PART},/* preload_a       1144M    p70*/
  {PART_USERDATA,                  9208*1024,  (4UL)*1024*1024,        EMMC_USER_PART},/* userdata           4G    p71*/
#endif
#endif // CONFIG_UPDATE_PRODUCT_EROFS
#else // CONFIG_USE_EROFS
#ifdef CONFIG_MARKET_INTERNAL
  {PART_PATCH_A,                    896*1024,          32*1024,        EMMC_USER_PART},/* patch             32M    p61*/
  {PART_PREAS,                      928*1024,         360*1024,        EMMC_USER_PART},/* preas            360M    p62*/
  {PART_PREAVS,                    1288*1024,          32*1024,        EMMC_USER_PART},/* preavs            32M    p63*/
  {PART_VENDOR_A,                  1320*1024,        1232*1024,        EMMC_USER_PART},/* vendor          1232M    p64*/
  {PART_ODM_A,                     2552*1024,         192*1024,        EMMC_USER_PART},/* odm              192M    p65*/
  {PART_CUST_A,                    2744*1024,         192*1024,        EMMC_USER_PART},/* cust             192M    p66*/
  {PART_SYSTEM_A,                  2936*1024,        3760*1024,        EMMC_USER_PART},/* system          3760M    p67*/
  {PART_PRODUCT_A,                 6696*1024,        2104*1024,        EMMC_USER_PART},/* product         2104M    p68*/
  {PART_VERSION_A,                 8800*1024,         576*1024,        EMMC_USER_PART},/* version          576M    p69*/
  {PART_PRELOAD_A,                 9376*1024,        1144*1024,        EMMC_USER_PART},/* preload_a       1144M    p70*/
  {PART_USERDATA,                 10520*1024,  (4UL)*1024*1024,        EMMC_USER_PART},/* userdata           4G    p71*/
#else
  {PART_PATCH_A,                    896*1024,          32*1024,        EMMC_USER_PART},/* patch             32M    p61*/
  {PART_PREAS,                      928*1024,        1216*1024,        EMMC_USER_PART},/* preas           1216M    p62*/
  {PART_PREAVS,                    2144*1024,          32*1024,        EMMC_USER_PART},/* preavs            32M    p63*/
  {PART_VENDOR_A,                  2176*1024,        1232*1024,        EMMC_USER_PART},/* vendor          1232M    p64*/
  {PART_ODM_A,                     3408*1024,         192*1024,        EMMC_USER_PART},/* odm              192M    p65*/
  {PART_CUST_A,                    3600*1024,         192*1024,        EMMC_USER_PART},/* cust             192M    p66*/
  {PART_SYSTEM_A,                  3792*1024,        3568*1024,        EMMC_USER_PART},/* system          3568M    p67*/
  {PART_PRODUCT_A,                 7360*1024,        2360*1024,        EMMC_USER_PART},/* product         2360M    p68*/
  {PART_VERSION_A,                 9720*1024,         576*1024,        EMMC_USER_PART},/* version          576M    p69*/
  {PART_PRELOAD_A,                10296*1024,        1144*1024,        EMMC_USER_PART},/* preload_a       1144M    p70*/
  {PART_USERDATA,                 11440*1024,  (4UL)*1024*1024,        EMMC_USER_PART},/* userdata           4G    p71*/
#endif
#endif // CONFIG_USE_EROFS
#endif
  {"0", 0, 0, 0},                                        /* total 11848M*/
};
#ifdef CONFIG_HISI_STORAGE_UFS_PARTITION
static const struct partition partition_table_ufs[] =
{
  {PART_XLOADER_A,                         0,        2*1024,        UFS_PART_0},
  {PART_XLOADER_B,                         0,        2*1024,        UFS_PART_1},
  {PART_PTABLE,                            0,           512,        UFS_PART_2},/* ptable                    512K    */
  {PART_FRP,                             512,           512,        UFS_PART_2},/* frp                       512K    p1*/
  {PART_PERSIST,                      1*1024,        6*1024,        UFS_PART_2},/* persist                     6M    p2*/
  {PART_RESERVED1,                    7*1024,          1024,        UFS_PART_2},/* reserved1                1024K    p3*/
  {PART_PTABLE_LU3,                        0,           512,        UFS_PART_3},/* ptable_lu3                512K    p0*/
  {PART_VRL,                             512,           512,        UFS_PART_3},/* vrl                       512K    p1*/
  {PART_VRL_BACKUP,                     1024,           512,        UFS_PART_3},/* vrl backup                512K    p2*/
  {PART_MODEM_SECURE,                   1536,          8704,        UFS_PART_3},/* modem_secure             8704K    p3*/
  {PART_NVME,                        10*1024,        5*1024,        UFS_PART_3},/* nvme                        5M    p4*/
  {PART_CTF,                         15*1024,        1*1024,        UFS_PART_3},/* PART_CTF                    1M    p5*/
  {PART_OEMINFO,                     16*1024,       96*1024,        UFS_PART_3},/* oeminfo                    96M    p6*/
  {PART_SECURE_STORAGE,             112*1024,       32*1024,        UFS_PART_3},/* secure storage             32M    p7*/
  {PART_MODEMNVM_FACTORY,           144*1024,       16*1024,        UFS_PART_3},/* modemnvm factory           16M    p8*/
  {PART_MODEMNVM_BACKUP,            160*1024,       16*1024,        UFS_PART_3},/* modemnvm backup            16M    p9*/
  {PART_MODEMNVM_IMG,               176*1024,       34*1024,        UFS_PART_3},/* modemnvm img               34M    p10*/
  {PART_HISEE_ENCOS,                210*1024,        4*1024,        UFS_PART_3},/* hisee_encos                 4M    p11*/
  {PART_VERITYKEY,                  214*1024,        1*1024,        UFS_PART_3},/* veritykey                   1M    p12*/
  {PART_DDR_PARA,                   215*1024,        1*1024,        UFS_PART_3},/* ddr_para                    1M    p13*/
  {PART_LOWPOWER_PARA,              216*1024,        1*1024,        UFS_PART_3},/* lowpower_para               1M    p14*/
  {PART_BATT_TP_PARA,               217*1024,        1*1024,        UFS_PART_3},/* batt_tp_para                1M    p15*/
  {PART_RESERVED2,                  218*1024,       25*1024,        UFS_PART_3},/* reserved2                  25M    p16*/
  {PART_SPLASH2,                    243*1024,       80*1024,        UFS_PART_3},/* splash2                    80M    p17*/
  {PART_BOOTFAIL_INFO,              323*1024,        2*1024,        UFS_PART_3},/* bootfail info              2MB    p18*/
  {PART_MISC,                       325*1024,        2*1024,        UFS_PART_3},/* misc                        2M    p19*/
  {PART_DFX,                        327*1024,       16*1024,        UFS_PART_3},/* dfx                        16M    p20*/
  {PART_RRECORD,                    343*1024,       16*1024,        UFS_PART_3},/* rrecord                    16M    p21*/
  {PART_CACHE,                      359*1024,      104*1024,        UFS_PART_3},/* cache                     104M    p22*/
  {PART_FW_LPM3_A,                  463*1024,          1024,        UFS_PART_3},/* fw_lpm3_a                 256K    p23*/
  {PART_RESERVED3_A,                464*1024,        7*1024,        UFS_PART_3},/* reserved3A               7936K    p24*/
  {PART_IVP,                        471*1024,        2*1024,        UFS_PART_3},/* ivp                         2M    p25*/
  {PART_HDCP_A,                     473*1024,        1*1024,        UFS_PART_3},/* PART_HDCP_A                 1M    p26*/
  {PART_HISEE_IMG_A,                474*1024,        4*1024,        UFS_PART_3},/* part_hisee_img_a            4M    p27*/
  {PART_HHEE_A,                     478*1024,        4*1024,        UFS_PART_3},/* hhee_a                      4M    p28*/
  {PART_HISEE_FS_A,                 482*1024,        8*1024,        UFS_PART_3},/* hisee_fs                    8M    p29*/
  {PART_FASTBOOT_A,                 490*1024,       12*1024,        UFS_PART_3},/* fastboot                   12M    p30*/
  {PART_VECTOR_A,                   502*1024,        4*1024,        UFS_PART_3},/* avs vector                  4M    p31*/
  {PART_ISP_BOOT_A,                 506*1024,        2*1024,        UFS_PART_3},/* isp_boot                    2M    p32*/
  {PART_ISP_FIRMWARE_A,             508*1024,       14*1024,        UFS_PART_3},/* isp_firmware               14M    p33*/
  {PART_FW_HIFI_A,                  522*1024,       12*1024,        UFS_PART_3},/* hifi                       12M    p34*/
  {PART_TEEOS_A,                    534*1024,        8*1024,        UFS_PART_3},/* teeos                       8M    p35*/
  {PART_SENSORHUB_A,                542*1024,       16*1024,        UFS_PART_3},/* sensorhub                  16M    p36*/
#ifdef CONFIG_SANITIZER_ENABLE
  {PART_ERECOVERY_KERNEL_A,             558*1024,       24*1024,        UFS_PART_3},/* erecovery_kernel           24M    p37*/
  {PART_ERECOVERY_RAMDISK_A,            582*1024,       32*1024,        UFS_PART_3},/* erecovery_ramdisk          32M    p38*/
  {PART_ERECOVERY_VENDOR_A,             614*1024,        8*1024,        UFS_PART_3},/* erecovery_vendor            8M    p39*/
  {PART_KERNEL_A,                       622*1024,       32*1024,        UFS_PART_3},/* kernel                     32M    p40*/
#else
  {PART_ERECOVERY_KERNEL_A,             558*1024,       24*1024,        UFS_PART_3},/* erecovery_kernel           24M    p37*/
  {PART_ERECOVERY_RAMDISK_A,            582*1024,       32*1024,        UFS_PART_3},/* erecovery_ramdisk          32M    p38*/
  {PART_ERECOVERY_VENDOR_A,             614*1024,       16*1024,        UFS_PART_3},/* erecovery_vendor           16M    p39*/
  {PART_KERNEL_A,                       630*1024,       24*1024,        UFS_PART_3},/* kernel                     24M    p40*/
#endif
  {PART_ENG_SYSTEM_A,                   654*1024,       12*1024,        UFS_PART_3},/* eng_system                 12M    p41*/
  {PART_RECOVERY_RAMDISK_A,             666*1024,       32*1024,        UFS_PART_3},/* recovery_ramdisk           32M    p42*/
  {PART_RECOVERY_VENDOR_A,              698*1024,       16*1024,        UFS_PART_3},/* recovery_vendor            16M    p43*/
  {PART_DTS_A,                          714*1024,        1*1024,        UFS_PART_3},/* dtimage                     1M    p44*/
  {PART_DTO_A,                          715*1024,       20*1024,        UFS_PART_3},/* dtoimage                   20M    p45*/
  {PART_TRUSTFIRMWARE_A,                735*1024,        2*1024,        UFS_PART_3},/* trustfirmware               2M    p46*/
  {PART_MODEM_FW_A,                     737*1024,       56*1024,        UFS_PART_3},/* modem_fw                   56M    p47*/
  {PART_ENG_VENDOR_A,                   793*1024,       12*1024,        UFS_PART_3},/* eng_vendor                 12M    p48*/
  {PART_MODEM_PATCH_NV_A,               805*1024,        4*1024,        UFS_PART_3},/* modem_patch_nv              4M    p49*/
  {PART_MODEM_DRIVER_A,                 809*1024,       20*1024,        UFS_PART_3},/* modem_driver_a             20M    p50*/
  {PART_RESERVED4_A,                    829*1024,       19*1024,        UFS_PART_3},/* reserved4A                 19M    p51*/
  {PART_RECOVERY_VBMETA_A,              848*1024,        2*1024,        UFS_PART_3},/* recovery_vbmeta_a           2M    p52*/
  {PART_ERECOVERY_VBMETA_A,             850*1024,        2*1024,        UFS_PART_3},/* erecovery_vbmeta_a          2M    p53*/
  {PART_VBMETA_A,                       852*1024,        4*1024,        UFS_PART_3},/* vbmeta_a                    4M    p54*/
  {PART_MODEMNVM_UPDATE_A,              856*1024,       16*1024,        UFS_PART_3},/* modemnvm_update_a          16M    p55*/
  {PART_MODEMNVM_CUST_A,                872*1024,       16*1024,        UFS_PART_3},/* modemnvm_cust_a            16M    p56*/
#ifdef CONFIG_FACTORY_MODE
  {PART_PATCH_A,                          888*1024,          32*1024,        UFS_PART_3},/* patch                  32M    p57*/
  {PART_PREAS,                            920*1024,        1216*1024,        UFS_PART_3},/* preas                1216M    p58*/
  {PART_PREAVS,                          2136*1024,          32*1024,        UFS_PART_3},/* preavs                 32M    p59*/
  {PART_VENDOR_A,                        2168*1024,        1232*1024,        UFS_PART_3},/* vendor               1232M    p60*/
  {PART_ODM_A,                           3400*1024,         192*1024,        UFS_PART_3},/* odm                   192M    p61*/
  {PART_CUST_A,                          3592*1024,         192*1024,        UFS_PART_3},/* cust                  192M    p62*/
  {PART_SYSTEM_A,                        3784*1024,        3568*1024,        UFS_PART_3},/* system               3568M    p63*/
  {PART_PRODUCT_A,                       7352*1024,        2360*1024,        UFS_PART_3},/* product              2360M    p64*/
  {PART_VERSION_A,                       9712*1024,         576*1024,        UFS_PART_3},/* version               576M    p65*/
  {PART_PRELOAD_A,                      10288*1024,        1144*1024,        UFS_PART_3},/* preload              1144M    p66*/
  {PART_HIBENCH_IMG,                    11432*1024,         128*1024,        UFS_PART_3},/* hibench_img           128M    p67*/
  {PART_HIBENCH_DATA,                   11560*1024,         512*1024,        UFS_PART_3},/* hibench_data          512M    p68*/
  {PART_FLASH_AGEING,                   12072*1024,         512*1024,        UFS_PART_3},/* FLASH_AGEING          512M    p69*/
  {PART_USERDATA,                       12584*1024,  (4UL)*1024*1024,        UFS_PART_3},/* userdata                4G    p70*/
#else
#ifdef CONFIG_USE_EROFS
#ifdef CONFIG_UPDATE_PRODUCT_EROFS
#ifdef CONFIG_MARKET_INTERNAL
  {PART_PATCH_A,                         888*1024,          200*1024,        UFS_PART_3},/* patch                 200M    p57*/
  {PART_PREAS,                           1088*1024,         360*1024,        UFS_PART_3},/* preas                 360M    p58*/
  {PART_PREAVS,                          1448*1024,          32*1024,        UFS_PART_3},/* preavs                 32M    p59*/
  {PART_VENDOR_A,                        1480*1024,        1064*1024,        UFS_PART_3},/* vendor               1064M    p60*/
  {PART_ODM_A,                           2544*1024,         192*1024,        UFS_PART_3},/* odm                   192M    p61*/
  {PART_CUST_A,                          2736*1024,         192*1024,        UFS_PART_3},/* cust                  192M    p62*/
  {PART_SYSTEM_A,                        2928*1024,        3760*1024,        UFS_PART_3},/* system               3760M    p63*/
  {PART_PRODUCT_A,                       6688*1024,        2104*1024,        UFS_PART_3},/* product              2104M    p64*/
  {PART_VERSION_A,                       8792*1024,         576*1024,        UFS_PART_3},/* version               576M    p65*/
  {PART_PRELOAD_A,                       9368*1024,        1144*1024,        UFS_PART_3},/* preload              1144M    p66*/
  {PART_USERDATA,                       10512*1024,  (4UL)*1024*1024,        UFS_PART_3},/* userdata                4G    p67*/
#else
  {PART_PATCH_A,                          888*1024,         200*1024,        UFS_PART_3},/* patch                 200M    p57*/
  {PART_PREAS,                           1088*1024,        1216*1024,        UFS_PART_3},/* preas                1216M    p58*/
  {PART_PREAVS,                          2304*1024,          32*1024,        UFS_PART_3},/* preavs                 32M    p59*/
  {PART_VENDOR_A,                        2336*1024,        1064*1024,        UFS_PART_3},/* vendor               1064M    p60*/
  {PART_ODM_A,                           3400*1024,         192*1024,        UFS_PART_3},/* odm                   192M    p61*/
  {PART_CUST_A,                          3592*1024,         192*1024,        UFS_PART_3},/* cust                  192M    p62*/
  {PART_SYSTEM_A,                        3784*1024,        3568*1024,        UFS_PART_3},/* system               3568M    p63*/
  {PART_PRODUCT_A,                       7352*1024,        2360*1024,        UFS_PART_3},/* product              2360M    p64*/
  {PART_VERSION_A,                       9712*1024,         576*1024,        UFS_PART_3},/* version               576M    p65*/
  {PART_PRELOAD_A,                      10288*1024,        1144*1024,        UFS_PART_3},/* preload              1144M    p66*/
  {PART_USERDATA,                       11432*1024,  (4UL)*1024*1024,        UFS_PART_3},/* userdata                4G    p67*/
#endif
#else
#ifdef CONFIG_MARKET_INTERNAL
  {PART_PATCH_A,                        888*1024,          32*1024,        UFS_PART_3},/* patch                  32M    p57*/
  {PART_PREAS,                          920*1024,         296*1024,        UFS_PART_3},/* preas                 296M    p58*/
  {PART_PREAVS,                        1216*1024,          32*1024,        UFS_PART_3},/* preavs                 32M    p59*/
  {PART_VENDOR_A,                      1248*1024,         864*1024,        UFS_PART_3},/* vendor                864M    p60*/
  {PART_ODM_A,                         2112*1024,         136*1024,        UFS_PART_3},/* odm                   136M    p61*/
  {PART_CUST_A,                        2248*1024,          88*1024,        UFS_PART_3},/* cust                   88M    p62*/
  {PART_SYSTEM_A,                      2336*1024,        2544*1024,        UFS_PART_3},/* system               2544M    p63*/
  {PART_PRODUCT_A,                     4880*1024,        2000*1024,        UFS_PART_3},/* product              2000M    p64*/
  {PART_VERSION_A,                     6880*1024,         576*1024,        UFS_PART_3},/* version               576M    p65*/
  {PART_PRELOAD_A,                     7456*1024,        1144*1024,        UFS_PART_3},/* preload_a            1144M    p66*/
  {PART_USERDATA,                      8600*1024,  (4UL)*1024*1024,        UFS_PART_3},/* userdata                4G    p67*/
#else
  {PART_PATCH_A,                        888*1024,          32*1024,        UFS_PART_3},/* patch                  32M    p57*/
  {PART_PREAS,                          920*1024,         976*1024,        UFS_PART_3},/* preas                 976M    p58*/
  {PART_PREAVS,                        1896*1024,          32*1024,        UFS_PART_3},/* preavs                 32M    p59*/
  {PART_VENDOR_A,                      1928*1024,         864*1024,        UFS_PART_3},/* vendor                864M    p60*/
  {PART_ODM_A,                         2792*1024,         136*1024,        UFS_PART_3},/* odm                   136M    p61*/
  {PART_CUST_A,                        2928*1024,          88*1024,        UFS_PART_3},/* cust                   88M    p62*/
  {PART_SYSTEM_A,                      3016*1024,        2312*1024,        UFS_PART_3},/* system               2312M    p63*/
  {PART_PRODUCT_A,                     5328*1024,        2152*1024,        UFS_PART_3},/* product              2152M    p64*/
  {PART_VERSION_A,                     7480*1024,         576*1024,        UFS_PART_3},/* version               576M    p65*/
  {PART_PRELOAD_A,                     8056*1024,        1144*1024,        UFS_PART_3},/* preload_a            1144M    p66*/
  {PART_USERDATA,                      9200*1024,  (4UL)*1024*1024,        UFS_PART_3},/* userdata                4G    p67*/
#endif
#endif
#else
#ifdef CONFIG_MARKET_INTERNAL
  {PART_PATCH_A,                         888*1024,           32*1024,        UFS_PART_3},/* patch                  32M    p57*/
  {PART_PREAS,                            920*1024,         360*1024,        UFS_PART_3},/* preas                 360M    p58*/
  {PART_PREAVS,                          1280*1024,          32*1024,        UFS_PART_3},/* preavs                 32M    p59*/
  {PART_VENDOR_A,                        1312*1024,        1232*1024,        UFS_PART_3},/* vendor               1232M    p60*/
  {PART_ODM_A,                           2544*1024,         192*1024,        UFS_PART_3},/* odm                   192M    p61*/
  {PART_CUST_A,                          2736*1024,         192*1024,        UFS_PART_3},/* cust                  192M    p62*/
  {PART_SYSTEM_A,                        2928*1024,        3760*1024,        UFS_PART_3},/* system               3760M    p63*/
  {PART_PRODUCT_A,                       6688*1024,        2104*1024,        UFS_PART_3},/* product              2104M    p64*/
  {PART_VERSION_A,                       8792*1024,         576*1024,        UFS_PART_3},/* version               576M    p65*/
  {PART_PRELOAD_A,                       9368*1024,        1144*1024,        UFS_PART_3},/* preload              1144M    p66*/
  {PART_USERDATA,                       10512*1024,  (4UL)*1024*1024,        UFS_PART_3},/* userdata                4G    p67*/
#else
  {PART_PATCH_A,                          888*1024,          32*1024,        UFS_PART_3},/* patch                  32M    p57*/
  {PART_PREAS,                            920*1024,        1216*1024,        UFS_PART_3},/* preas                1216M    p58*/
  {PART_PREAVS,                          2136*1024,          32*1024,        UFS_PART_3},/* preavs                 32M    p59*/
  {PART_VENDOR_A,                        2168*1024,        1232*1024,        UFS_PART_3},/* vendor               1232M    p60*/
  {PART_ODM_A,                           3400*1024,         192*1024,        UFS_PART_3},/* odm                   192M    p61*/
  {PART_CUST_A,                          3592*1024,         192*1024,        UFS_PART_3},/* cust                  192M    p62*/
  {PART_SYSTEM_A,                        3784*1024,        3568*1024,        UFS_PART_3},/* system               3568M    p63*/
  {PART_PRODUCT_A,                       7352*1024,        2360*1024,        UFS_PART_3},/* product              2360M    p64*/
  {PART_VERSION_A,                       9712*1024,         576*1024,        UFS_PART_3},/* version               576M    p65*/
  {PART_PRELOAD_A,                      10288*1024,        1144*1024,        UFS_PART_3},/* preload              1144M    p66*/
  {PART_USERDATA,                       11432*1024,  (4UL)*1024*1024,        UFS_PART_3},/* userdata                4G    p67*/
#endif
#endif
#endif
  {"0", 0, 0, 0},
};
#endif

#endif
