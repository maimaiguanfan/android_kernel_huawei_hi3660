#ifndef _HI6250_PARTITION_H_
#define _HI6250_PARTITION_H_

#include "hisi_partition.h"
#include "partition_def.h"

static const struct partition partition_table_emmc[] =
{
    {PART_XLOADER,           0,         256,      EMMC_BOOT_MAJOR_PART},
    {PART_PTABLE,            0,         512,      EMMC_USER_PART}, /* ptable           512K */
    {PART_VRL,               512,       256,      EMMC_USER_PART}, /* VRL              256K p1*/
    {PART_VRL_BACKUP,        768,       256,      EMMC_USER_PART}, /* VRL backup       256K p2*/
    {PART_FW_LPM3,           1024,      256,      EMMC_USER_PART}, /* mcuimage         256K p3*/
    {PART_FRP,               1280,      768,      EMMC_USER_PART}, /* frp              768K p4*/
    {PART_FASTBOOT,          2*1024,    4*1024,   EMMC_USER_PART}, /* fastboot         4M   p5*/
    {PART_MODEMNVM_FACTORY,  6*1024,    4*1024,   EMMC_USER_PART}, /* modemnvm factory 4M   p6*/
    {PART_NVME,              10*1024,   6*1024,   EMMC_USER_PART}, /* nvme             6M   p7*/
    {PART_OEMINFO,           16*1024,   64*1024,  EMMC_USER_PART}, /* oeminfo          64M  p8*/
    {PART_RESERVED3,         80*1024,   4*1024,   EMMC_USER_PART}, /* reserved3        4M   p9*/
    {PART_MODEMNVM_BACKUP,   84*1024,   4*1024,   EMMC_USER_PART}, /* modemnvm backup  4M   p10*/
    {PART_MODEMNVM_IMG,      88*1024,   8*1024,   EMMC_USER_PART}, /* modemnvm img     8M   p11*/
    {PART_MODEMNVM_SYSTEM,   96*1024,   4*1024,   EMMC_USER_PART}, /* modemnvm system  4M   p12*/
    {PART_SECURE_STORAGE,    100*1024,  32*1024,  EMMC_USER_PART}, /* secure storage   32M  p13*/
    {PART_RESERVED4,         132*1024,  2*1024,   EMMC_USER_PART}, /* reserved4        2M   p14*/
    {PART_RESERVED5,         134*1024,  2*1024,   EMMC_USER_PART}, /* reserved5        2M   p15*/
    {PART_PERSIST,           136*1024,  2*1024,   EMMC_USER_PART}, /* persist          2M   p16*/
#ifdef SECDOG_SUPPORT_RSA_2048
    {PART_MODEM_SECURE,      138*1024,  14*1024,  EMMC_USER_PART}, /* modem_secure     14M  p17*/
#else
    {PART_RESERVED1,         138*1024,  14*1024,  EMMC_USER_PART}, /* reserved1        14M  p17*/
#endif
    {PART_MODEM_OM,          152*1024,  32*1024,  EMMC_USER_PART}, /* modem om         32M  p18*/
    {PART_SPLASH2,           184*1024,  64*1024,  EMMC_USER_PART}, /* splash2          64M  p19*/
    {PART_MISC,              248*1024,  2*1024,   EMMC_USER_PART}, /* misc             2M   p20*/
    {PART_MODEMNVM_UPDATE,   250*1024,  24*1024,  EMMC_USER_PART}, /* modemnvm update  24M  p21*/
    {PART_RESERVED2,         274*1024,  46*1024,  EMMC_USER_PART}, /* reserved2       46M  p22*/
    {PART_PRELOAD,           320*1024,  8*1024,   EMMC_USER_PART}, /* preload          8M   p23*/
    {PART_TEEOS,             328*1024,  4*1024,   EMMC_USER_PART}, /* teeos            4M   p24*/
    {PART_TRUSTFIRMWARE,     332*1024,  2*1024,   EMMC_USER_PART}, /* trustfirmware    2M   p25*/
    {PART_SENSORHUB,         334*1024,  16*1024,  EMMC_USER_PART}, /* sensorhub        16M  p26*/
    {PART_FW_HIFI,           350*1024,  12*1024,  EMMC_USER_PART}, /* hifi             12M  p27*/
#ifdef CONFIG_SANITIZER_ENABLE
    {PART_ERECOVERY_KERNEL,  362*1024,  24*1024,  EMMC_USER_PART},/* erecovery_kernel  24M  p28*/
    {PART_ERECOVERY_RAMDISK, 386*1024,  32*1024,  EMMC_USER_PART},/* erecovery_ramdisk 32M  p29*/
    {PART_ERECOVERY_VENDOR,  418*1024,  8*1024,  EMMC_USER_PART},/* erecovery_vendor 8M   p30*/
    {PART_KERNEL,            426*1024,  32*1024,  EMMC_USER_PART},/* kernel          32M    p31*/
#else
    {PART_ERECOVERY_KERNEL,  362*1024,  24*1024,  EMMC_USER_PART},/* erecovery_kernel  24M  p28*/
    {PART_ERECOVERY_RAMDISK, 386*1024,  32*1024,  EMMC_USER_PART},/* erecovery_ramdisk 32M  p29*/
    {PART_ERECOVERY_VENDOR,  418*1024,  16*1024,  EMMC_USER_PART},/* erecovery_vendor 16M   p30*/
    {PART_KERNEL,            434*1024,  24*1024,  EMMC_USER_PART},/* kernel          24M    p31*/
#endif
    {PART_ENG_SYSTEM,      458*1024,  12*1024,  EMMC_USER_PART},/* eng_system        12M    p32*/
    {PART_RECOVERY_RAMDISK,  470*1024,  32*1024,  EMMC_USER_PART},/* recovery_ramdisk 32M   p33*/
    {PART_RECOVERY_VENDOR,   502*1024,  16*1024,  EMMC_USER_PART},/* recovery_vendor 16M    p34*/
    {PART_DTS,               518*1024,  28*1024,  EMMC_USER_PART}, /* dtimage          28M  p35*/
    {PART_DTO,               546*1024,  4*1024,  EMMC_USER_PART}, /* dtoimage          4M   p36*/
    {PART_MODEM_FW,          550*1024,  96*1024,  EMMC_USER_PART}, /* modem_fw         96M  p37*/
    {PART_RECOVERY_VBMETA, 646*1024,  1*1024,   EMMC_USER_PART}, /* recovery_vbmeta   1M    p38*/
    {PART_ERECOVERY_VBMETA,647*1024,  1*1024,   EMMC_USER_PART}, /* erecovery_vbmeta  1M    p39*/
    {PART_ENG_VENDOR,      648*1024,  12*1024,   EMMC_USER_PART}, /* eng_vendor     12M     p40*/
    {PART_DFX,               660*1024,  16*1024,  EMMC_USER_PART}, /* dfx              16M  p41*/
    {PART_VBMETA,            676*1024,  4*1024,   EMMC_USER_PART}, /* PART_VBMETA      4M   p42*/
    {PART_CACHE,             680*1024,  128*1024, EMMC_USER_PART}, /* cache            128M p43*/
    {PART_ODM,               808*1024,  128*1024, EMMC_USER_PART}, /* odm              128M p44*/
    {PART_HISITEST0,         936*1024,  2*1024,   EMMC_USER_PART}, /* hisitest0        2M   p45*/
    {PART_HISITEST1,         938*1024,  2*1024,   EMMC_USER_PART}, /* hisitest1        2M   p46*/
    {PART_HISITEST2,         940*1024,  4*1024,   EMMC_USER_PART}, /* hisitest2        4M   p47*/
#if (defined(CONFIG_MARKET_OVERSEA) || defined(CONFIG_MARKET_INTERNAL) \
        || defined(CONFIG_MARKET_16G_OVERSEA) || defined(CONFIG_MARKET_16G_INTERNAL) \
        || defined(CONFIG_MARKET_INTERNAL_LITE) || defined(CONFIG_MARKET_HANDEL_OVERSEA) \
		|| defined(CONFIG_MARKET_BACH2_OVERSEA))
    {PART_PATCH,             944*1024,  32*1024,  EMMC_USER_PART}, /* patch            32M  p48*/
    {PART_BOOTFAIL_INFO,     976*1024,  2*1024,   EMMC_USER_PART}, /* bootfail_info    2M   p49*/
    {PART_RRECORD,           978*1024,  16*1024,  EMMC_USER_PART}, /* rrecord          16M  p50*/
    {PART_RESERVED9,         994*1024,  30*1024,  EMMC_USER_PART}, /* reserved9       30M  p51*/
#endif
#ifdef CONFIG_MARKET_OVERSEA
#ifdef CONFIG_PARTITION_ROMUPGRADE_HI6250
    {PART_SYSTEM,            1024*1024, 3184*1024,EMMC_USER_PART}, /* system           3184M p52*/
    {PART_PREAS,             4208*1024, 1128*1024, EMMC_USER_PART},/* preas            1128M p53*/
    {PART_PREAVS,            5336*1024, 32*1024,  EMMC_USER_PART}, /* preavs           32M   p54*/
    {PART_CUST,              5368*1024, 136*1024, EMMC_USER_PART}, /* cust             136M  p55*/
    {PART_VERSION,           5504*1024, 32*1024,  EMMC_USER_PART}, /* version          32M   p56*/
    {PART_VENDOR,            5536*1024, 784*1024, EMMC_USER_PART}, /* vendor           784M  p57*/
    {PART_PRODUCT,           6320*1024, 592*1024, EMMC_USER_PART}, /* product          592M  p58*/
    {PART_USERDATA,          6912*1024, 4096*1024,EMMC_USER_PART}, /* userdata         4096M p59*/
#elif defined CONFIG_BOARD_CHARGER_DRM
    {PART_SYSTEM,            1024*1024, 2400*1024,EMMC_USER_PART}, /* system           2400M p52*/
    {PART_PREAS,             3424*1024, 1144*1024, EMMC_USER_PART},/* preas            1144M p53*/
    {PART_PREAVS,            4568*1024, 32*1024,  EMMC_USER_PART}, /* preavs           32M   p54*/
    {PART_CUST,              4600*1024, 136*1024, EMMC_USER_PART}, /* cust             136M  p55*/
    {PART_VERSION,           4736*1024, 32*1024,  EMMC_USER_PART}, /* version          32M   p56*/
    {PART_VENDOR,            4768*1024, 536*1024, EMMC_USER_PART}, /* vendor           536M  p57*/
    {PART_PRODUCT,           5304*1024, 456*1024, EMMC_USER_PART}, /* product          456M  p58*/
    {PART_USERDATA,          5760*1024, 4096*1024,EMMC_USER_PART}, /* userdata         4096M p59*/
#else
    {PART_SYSTEM,            1024*1024, 2312*1024,EMMC_USER_PART}, /* system           2312M p52*/
    {PART_PREAS,             3336*1024, 1104*1024, EMMC_USER_PART},/* preas            1104M p53*/
    {PART_PREAVS,            4440*1024, 32*1024,  EMMC_USER_PART}, /* preavs           32M   p54*/
    {PART_CUST,              4472*1024, 136*1024, EMMC_USER_PART}, /* cust             136M  p53*/
    {PART_VERSION,           4608*1024, 32*1024,  EMMC_USER_PART}, /* version          32M   p54*/
    {PART_VENDOR,            4640*1024, 584*1024, EMMC_USER_PART}, /* vendor           584M  p55*/
    {PART_PRODUCT,           5224*1024, 536*1024, EMMC_USER_PART}, /* product          536M  p56*/
    {PART_USERDATA,          5760*1024, 4096*1024,EMMC_USER_PART}, /* userdata         4096M p57*/
#endif
#elif defined CONFIG_MARKET_INTERNAL
#ifdef CONFIG_PARTITION_ROMUPGRADE_HI6250
    {PART_SYSTEM,            1024*1024, 3352*1024,EMMC_USER_PART}, /* system           3352M p52*/
    {PART_PREAS,             4376*1024, 168*1024, EMMC_USER_PART}, /* preas            168M  p53*/
    {PART_PREAVS,            4544*1024, 32*1024,  EMMC_USER_PART}, /* preavs           32M   p54*/
    {PART_CUST,              4576*1024, 192*1024, EMMC_USER_PART}, /* cust             192M  p55*/
    {PART_VERSION,           4768*1024, 32*1024,  EMMC_USER_PART}, /* version          32M   p56*/
    {PART_VENDOR,            4800*1024, 624*1024, EMMC_USER_PART}, /* vendor           624M  p57*/
    {PART_PRODUCT,           5424*1024, 352*1024, EMMC_USER_PART}, /* product          352M  p58*/
    {PART_USERDATA,          5776*1024, 4096*1024,EMMC_USER_PART}, /* userdata         4096M p59*/
#elif defined CONFIG_BOARD_CHARGER_DRM
    {PART_SYSTEM,            1024*1024, 2888*1024,EMMC_USER_PART}, /* system           2888M p52*/
    {PART_PREAS,             3912*1024, 168*1024, EMMC_USER_PART}, /* preas            168M  p53*/
    {PART_PREAVS,            4080*1024, 32*1024,  EMMC_USER_PART}, /* preavs           32M   p54*/
    {PART_CUST,              4112*1024, 192*1024, EMMC_USER_PART}, /* cust             192M  p55*/
    {PART_VERSION,           4304*1024, 32*1024,  EMMC_USER_PART}, /* version          32M   p56*/
    {PART_VENDOR,            4336*1024, 524*1024, EMMC_USER_PART}, /* vendor           524M  p57*/
    {PART_PRODUCT,           4860*1024, 452*1024, EMMC_USER_PART}, /* product          452M  p58*/
    {PART_USERDATA,          5312*1024, 4096*1024,EMMC_USER_PART}, /* userdata         4096M p59*/
#else
    {PART_SYSTEM,            1024*1024, 2888*1024,EMMC_USER_PART}, /* system           2888M p52*/
    {PART_PREAS,             3912*1024, 168*1024, EMMC_USER_PART}, /* preas            168M  p53*/
    {PART_PREAVS,            4080*1024, 32*1024,  EMMC_USER_PART}, /* preavs           32M   p54*/
    {PART_CUST,              4112*1024, 192*1024, EMMC_USER_PART}, /* cust             192M  p55*/
    {PART_VERSION,           4304*1024, 32*1024,  EMMC_USER_PART}, /* version          32M   p56*/
    {PART_VENDOR,            4336*1024, 624*1024, EMMC_USER_PART}, /* vendor           624M  p57*/
    {PART_PRODUCT,           4960*1024, 352*1024, EMMC_USER_PART}, /* product          352M  p58*/
    {PART_USERDATA,          5312*1024, 4096*1024,EMMC_USER_PART}, /* userdata         4096M p59*/
#endif
#elif defined CONFIG_MARKET_BACH2_OVERSEA
    {PART_SYSTEM,            1024*1024, 2400*1024,EMMC_USER_PART}, /* system           2400M p52*/
    {PART_PREAS,             3424*1024, 1144*1024,EMMC_USER_PART}, /* preas            1144M p53*/
    {PART_PREAVS,            4568*1024, 32*1024,  EMMC_USER_PART}, /* preavs           32M   p54*/
    {PART_CUST,              4600*1024, 136*1024, EMMC_USER_PART}, /* cust             136M  p55*/
    {PART_VERSION,           4736*1024, 32*1024,  EMMC_USER_PART}, /* version          32M   p56*/
    {PART_VENDOR,            4768*1024, 536*1024, EMMC_USER_PART}, /* vendor           536M  p57*/
    {PART_PRODUCT,           5304*1024, 712*1024, EMMC_USER_PART}, /* product          712M  p58*/
    {PART_USERDATA,          6016*1024, 4096*1024,EMMC_USER_PART}, /* userdata         4096M p59*/
#elif defined CONFIG_MARKET_HANDEL_OVERSEA
#ifdef CONFIG_EN_PRETS_PRETVS
    {PART_SYSTEM,            1024*1024, 2500*1024,EMMC_USER_PART}, /* system           2500M p52*/
    {PART_PRETS,             3524*1024, 124*1024, EMMC_USER_PART}, /* prets            124M  p53*/
    {PART_PRETVS,            3648*1024, 16*1024,  EMMC_USER_PART}, /* pretvs           16M   p54*/
    {PART_PREAS,             3664*1024, 1064*1024,EMMC_USER_PART}, /* preas            1064M p55*/
    {PART_PREAVS,            4728*1024, 32*1024,  EMMC_USER_PART}, /* preavs           32M   p56*/
    {PART_CUST,              4760*1024, 136*1024, EMMC_USER_PART}, /* cust             136M  p57*/
    {PART_VERSION,           4896*1024, 32*1024,  EMMC_USER_PART}, /* version          32M   p58*/
    {PART_VENDOR,            4928*1024, 584*1024, EMMC_USER_PART}, /* vendor           584M  p59*/
    {PART_PRODUCT,           5512*1024, 248*1024, EMMC_USER_PART}, /* product          248M  p60*/
    {PART_USERDATA,          5760*1024, 4096*1024,EMMC_USER_PART}, /* userdata         4096M p61*/
#else
    {PART_SYSTEM,            1024*1024, 2640*1024,EMMC_USER_PART}, /* system           2640M p52*/
    {PART_PREAS,             3664*1024, 1064*1024,EMMC_USER_PART}, /* preas            1064M p53*/
    {PART_PREAVS,            4728*1024, 32*1024,  EMMC_USER_PART}, /* preavs           32M   p54*/
    {PART_CUST,              4760*1024, 136*1024, EMMC_USER_PART}, /* cust             136M  p55*/
    {PART_VERSION,           4896*1024, 32*1024,  EMMC_USER_PART}, /* version          32M   p56*/
    {PART_VENDOR,            4928*1024, 584*1024, EMMC_USER_PART}, /* vendor           584M  p57*/
    {PART_PRODUCT,           5512*1024, 248*1024, EMMC_USER_PART}, /* product          248M  p58*/
    {PART_USERDATA,          5760*1024, 4096*1024,EMMC_USER_PART}, /* userdata         4096M p59*/
#endif
#elif defined CONFIG_MARKET_INTERNAL_LITE
    {PART_SYSTEM,            1024*1024, 2712*1024,EMMC_USER_PART}, /* system           2712M p52*/
    {PART_PREAS,             3736*1024, 152*1024, EMMC_USER_PART}, /* preas            152M  p53*/
    {PART_PREAVS,            3888*1024, 32*1024,  EMMC_USER_PART}, /* preavs           32M   p54*/
    {PART_CUST,              3920*1024, 192*1024, EMMC_USER_PART}, /* cust             192M  p55*/
    {PART_VERSION,           4112*1024, 32*1024,  EMMC_USER_PART}, /* version          32M   p56*/
    {PART_VENDOR,            4144*1024, 656*1024, EMMC_USER_PART}, /* vendor           656M  p57*/
    {PART_PRODUCT,           4800*1024, 512*1024, EMMC_USER_PART}, /* product          512M  p58*/
    {PART_USERDATA,          5312*1024, 4096*1024,EMMC_USER_PART}, /* userdata         4096M p59*/
#elif defined CONFIG_MARKET_16G_OVERSEA
    {PART_SYSTEM,            1024*1024, 3008*1024,EMMC_USER_PART}, /* system           3008M p52*/
    {PART_CUST,              4032*1024, 192*1024, EMMC_USER_PART}, /* cust             192M  p53*/
    {PART_VERSION,           4224*1024, 32*1024,  EMMC_USER_PART}, /* version          32M   p54*/
    {PART_VENDOR,            4256*1024, 784*1024, EMMC_USER_PART}, /* vendor           784M  p55*/
    {PART_PRODUCT,           5040*1024, 192*1024, EMMC_USER_PART}, /* product          192M  p56*/
    {PART_USERDATA,          5232*1024, 4096*1024,EMMC_USER_PART}, /* userdata         4096M p57*/
#elif defined CONFIG_MARKET_16G_INTERNAL
    {PART_SYSTEM,            1024*1024, 2720*1024,EMMC_USER_PART}, /* system           2720M p52*/
    {PART_CUST,              3744*1024, 192*1024, EMMC_USER_PART}, /* cust             192M  p53*/
    {PART_VERSION,           3936*1024, 32*1024,  EMMC_USER_PART}, /* version          32M   p54*/
    {PART_VENDOR,            3968*1024, 784*1024, EMMC_USER_PART}, /* vendor           784M  p55*/
    {PART_PRODUCT,           4752*1024, 192*1024, EMMC_USER_PART}, /* product          192M  p56*/
    {PART_USERDATA,          4944*1024, 4096*1024,EMMC_USER_PART}, /* userdata         4096M p57*/
#elif defined CONFIG_MARKET_BERLIN_OVERSEA
    {PART_PATCH,             944*1024,  32*1024,  EMMC_USER_PART}, /* patch             32M   p48*/
    {PART_BOOTFAIL_INFO,     976*1024,  2*1024,   EMMC_USER_PART}, /* bootfail_info     2M    p49*/
    {PART_RRECORD,           978*1024,  16*1024,  EMMC_USER_PART}, /* rrecord           16M   p50*/
    {PART_RESERVED9,         994*1024,  14*1024,  EMMC_USER_PART}, /* reserved9         14M    p51*/
    {PART_SYSTEM,            1008*1024, 3008*1024,EMMC_USER_PART}, /* system            3008M p52*/
    {PART_CUST,              4016*1024, 192*1024, EMMC_USER_PART}, /* cust              192M  p53*/
    {PART_VERSION,           4208*1024, 32*1024,  EMMC_USER_PART}, /* version           24M   p54*/
    {PART_VENDOR,            4240*1024, 608*1024, EMMC_USER_PART}, /* vendor            608M  p55*/
    {PART_PRODUCT,           4848*1024, 192*1024, EMMC_USER_PART}, /* product           200M  p56*/
    {PART_USERDATA,          5040*1024, 4096*1024,EMMC_USER_PART}, /* userdata          4096M p57*/
#elif defined CONFIG_MARKET_BERLIN_INTERNAL
    {PART_PATCH,             944*1024,  32*1024,  EMMC_USER_PART}, /* patch             32M   p48*/
    {PART_BOOTFAIL_INFO,     976*1024,  2*1024,   EMMC_USER_PART}, /* bootfail_info     2M    p49*/
    {PART_RRECORD,           978*1024,  16*1024,  EMMC_USER_PART}, /* rrecord           16M   p50*/
    {PART_RESERVED9,         994*1024,  14*1024,  EMMC_USER_PART}, /* reserved9         14M    p51*/
    {PART_SYSTEM,            1008*1024, 2688*1024,EMMC_USER_PART}, /* system            2688M p52*/
    {PART_CUST,              3696*1024, 192*1024, EMMC_USER_PART}, /* cust              192M  p53*/
    {PART_VERSION,           3888*1024, 32*1024,  EMMC_USER_PART}, /* version           32M   p54*/
    {PART_VENDOR,            3920*1024, 608*1024, EMMC_USER_PART}, /* vendor            608M  p55*/
    {PART_PRODUCT,           4528*1024, 192*1024, EMMC_USER_PART}, /* product           192M  p56*/
    {PART_USERDATA,          4720*1024, 4096*1024,EMMC_USER_PART}, /* userdata          4096M p57*/
#elif defined CONFIG_MARKET_ANNE_OVERSEA
    {PART_PATCH,             944*1024,  32*1024,  EMMC_USER_PART}, /* patch             32M   p48*/
    {PART_BOOTFAIL_INFO,     976*1024,  2*1024,   EMMC_USER_PART}, /* bootfail_info     2M    p49*/
    {PART_RRECORD,           978*1024,  16*1024,  EMMC_USER_PART}, /* rrecord           16M   p50*/
    {PART_RESERVED9,         994*1024, 14*1024,   EMMC_USER_PART}, /* reserved9         14M    p51*/
    {PART_SYSTEM,            1008*1024, 3728*1024,EMMC_USER_PART}, /* system            3728M p52*/
    {PART_PREAS,             4736*1024, 1128*1024,EMMC_USER_PART}, /* preas             1128M  p53*/
    {PART_PREAVS,            5864*1024, 32*1024,  EMMC_USER_PART}, /* preavs            32M   p54*/
    {PART_CUST,              5896*1024, 192*1024, EMMC_USER_PART}, /* cust              192M  p55*/
    {PART_VERSION,           6088*1024, 32*1024,  EMMC_USER_PART}, /* version           32M   p56*/
    {PART_VENDOR,            6120*1024, 784*1024, EMMC_USER_PART}, /* vendor            784M  p57*/
    {PART_PRODUCT,           6904*1024, 936*1024, EMMC_USER_PART}, /* product           936M  p58*/
    {PART_USERDATA,          7840*1024, 4096*1024,EMMC_USER_PART}, /* userdata          4096M p59*/
#elif defined CONFIG_MARKET_ANNE_INTERNAL
    {PART_PATCH,             944*1024,  32*1024,  EMMC_USER_PART}, /* patch             32M   p48*/
    {PART_BOOTFAIL_INFO,     976*1024,  2*1024,   EMMC_USER_PART}, /* bootfail_info     2M    p49*/
    {PART_RRECORD,           978*1024,  16*1024,  EMMC_USER_PART}, /* rrecord           16M   p50*/
    {PART_RESERVED9,         994*1024,  14*1024,  EMMC_USER_PART}, /* reserved9         14M    p51*/
    {PART_SYSTEM,            1008*1024, 4152*1024,EMMC_USER_PART}, /* system            4152M p52*/
    {PART_PREAS,             5160*1024, 168*1024, EMMC_USER_PART}, /* preas             168M  p53*/
    {PART_PREAVS,            5328*1024, 32*1024,  EMMC_USER_PART}, /* preavs            32M   p54*/
    {PART_CUST,              5360*1024, 192*1024, EMMC_USER_PART}, /* cust              192M  p55*/
    {PART_VERSION,           5552*1024, 32*1024,  EMMC_USER_PART}, /* version           32M   p56*/
    {PART_VENDOR,            5584*1024, 624*1024, EMMC_USER_PART}, /* vendor            624M  p57*/
    {PART_PRODUCT,           6208*1024, 752*1024, EMMC_USER_PART}, /* product           752M  p58*/
    {PART_USERDATA,          6960*1024, 4096*1024,EMMC_USER_PART}, /* userdata          4096M p59*/
#elif defined CONFIG_MARKET_ANNE_INDIA
    {PART_PATCH,             944*1024,  32*1024,  EMMC_USER_PART}, /* patch             32M   p48*/
    {PART_BOOTFAIL_INFO,     976*1024,  2*1024,   EMMC_USER_PART}, /* bootfail_info     2M    p49*/
    {PART_RRECORD,           978*1024,  16*1024,  EMMC_USER_PART}, /* rrecord           16M   p50*/
    {PART_RESERVED9,         994*1024,  14*1024,  EMMC_USER_PART}, /* reserved9         14M   p51*/
    {PART_SYSTEM,            1008*1024, 3192*1024,EMMC_USER_PART}, /* system            3192M p52*/
    {PART_PREAS,             4200*1024, 1128*1024,EMMC_USER_PART}, /* preas             1128M p53*/
    {PART_PREAVS,            5328*1024, 32*1024,  EMMC_USER_PART}, /* preavs            32M   p54*/
    {PART_CUST,              5360*1024, 192*1024, EMMC_USER_PART}, /* cust              192M  p55*/
    {PART_VERSION,           5552*1024, 32*1024,  EMMC_USER_PART}, /* version           32M   p56*/
    {PART_VENDOR,            5584*1024, 624*1024, EMMC_USER_PART}, /* vendor            624M  p57*/
    {PART_PRODUCT,           6208*1024, 752*1024, EMMC_USER_PART}, /* product           752M  p58*/
    {PART_USERDATA,          6960*1024, 4096*1024,EMMC_USER_PART}, /* userdata          4096M p59*/
#elif defined CONFIG_MARKET_FULL_OVERSEA
    {PART_PATCH,             944*1024,  32*1024,  EMMC_USER_PART}, /* patch             32M   p48*/
    {PART_BOOTFAIL_INFO,     976*1024,  2*1024,   EMMC_USER_PART}, /* bootfail_info     2M    p49*/
    {PART_RRECORD,           978*1024,  16*1024,  EMMC_USER_PART}, /* rrecord           16M   p50*/
    {PART_RESERVED9,         994*1024, 14*1024,  EMMC_USER_PART}, /* reserved9          14M    p51*/
    {PART_SYSTEM,            1008*1024, 5632*1024,EMMC_USER_PART}, /* system            5632M p52*/
    {PART_CUST,              6640*1024, 192*1024, EMMC_USER_PART}, /* cust              192M  p53*/
    {PART_VERSION,           6832*1024, 32*1024,  EMMC_USER_PART}, /* version           32M   p54*/
    {PART_VENDOR,            6864*1024, 784*1024, EMMC_USER_PART}, /* vendor            784M  p55*/
    {PART_PRODUCT,           7648*1024, 192*1024, EMMC_USER_PART}, /* product           192M  p56*/
    {PART_USERDATA,          7840*1024, 4096*1024,EMMC_USER_PART}, /* userdata          4096M p57*/
#elif defined CONFIG_MARKET_FULL_INTERNAL
    {PART_PATCH,             944*1024,  32*1024,  EMMC_USER_PART}, /* patch             32M   p48*/
    {PART_BOOTFAIL_INFO,     976*1024,  2*1024,   EMMC_USER_PART}, /* bootfail_info     2M    p49*/
    {PART_RRECORD,           978*1024,  16*1024,  EMMC_USER_PART}, /* rrecord           16M   p50*/
    {PART_RESERVED9,         994*1024,  14*1024,  EMMC_USER_PART}, /* reserved9         14M    p51*/
    {PART_SYSTEM,            1008*1024, 4752*1024,EMMC_USER_PART}, /* system            4752M p52*/
    {PART_CUST,              5760*1024, 192*1024, EMMC_USER_PART}, /* cust              192M  p53*/
    {PART_VERSION,           5952*1024, 32*1024,  EMMC_USER_PART}, /* version           32M   p54*/
    {PART_VENDOR,            5984*1024, 784*1024, EMMC_USER_PART}, /* vendor            784M  p55*/
    {PART_PRODUCT,           6768*1024, 192*1024, EMMC_USER_PART}, /* product           192M  p56*/
    {PART_USERDATA,          6960*1024, 4096*1024,EMMC_USER_PART}, /* userdata          4096M p57*/
#elif defined CONFIG_MARKET_INTERNAL_HM
    {PART_PATCH,             944*1024,  96*1024,  EMMC_USER_PART}, /* patch             96M   p48*/
    {PART_BOOTFAIL_INFO,     1040*1024,  2*1024,  EMMC_USER_PART}, /* bootfail_info     2M    p49*/
    {PART_RRECORD,           1042*1024,  16*1024, EMMC_USER_PART}, /* rrecord           16M   p50*/
    {PART_RESERVED9,         1058*1024,  2*1024,  EMMC_USER_PART}, /* reserved9         2M    p51*/
    {PART_SYSTEM,            1060*1024, 2328*1024,EMMC_USER_PART}, /* system            2328M p52*/
    {PART_VENDOR,            3388*1024, 524*1024, EMMC_USER_PART}, /* vendor            524M  p53*/
    {PART_PREAS,             3912*1024, 168*1024, EMMC_USER_PART}, /* preas             168M  p54*/
    {PART_PREAVS,            4080*1024, 32*1024,  EMMC_USER_PART}, /* preavs            32M   p55*/
    {PART_CUST,              4112*1024, 88*1024,  EMMC_USER_PART}, /* cust              88M   p56*/
    {PART_VERSION,           4200*1024, 32*1024,  EMMC_USER_PART}, /* version           32M   p57*/
    {PART_PRODUCT,           4232*1024, 1080*1024,EMMC_USER_PART}, /* product           1080M p58*/
    {PART_USERDATA,          5312*1024, 4096*1024,EMMC_USER_PART}, /* userdata          4096M p59*/
#else
#ifdef CONFIG_VENDORIMAGE_FILE_SYSTEM_TYPE
    {PART_PATCH,             944*1024,  32*1024,  EMMC_USER_PART}, /* patch             32M   p48*/
    {PART_BOOTFAIL_INFO,     976*1024,  2*1024,   EMMC_USER_PART}, /* bootfail_info     2M    p49*/
    {PART_RRECORD,           978*1024,  16*1024,  EMMC_USER_PART}, /* rrecord           16M   p50*/
    {PART_RESERVED9,         994*1024,  14*1024,  EMMC_USER_PART}, /* reserved9         14M    p51*/
    {PART_SYSTEM,            1008*1024, 2560*1024,EMMC_USER_PART}, /* system            2560M p52*/
    {PART_CUST,              3568*1024, 192*1024, EMMC_USER_PART}, /* cust              192M  p53*/
    {PART_VERSION,           3760*1024, 32*1024,  EMMC_USER_PART}, /* version           32M   p54*/
    {PART_VENDOR,            3792*1024, 608*1024, EMMC_USER_PART}, /* vendor            608M  p55*/
    {PART_PRODUCT,           4400*1024, 192*1024, EMMC_USER_PART}, /* product           192M  p56*/
    {PART_USERDATA,          4592*1024, 4096*1024,EMMC_USER_PART}, /* userdata          4096M p57*/
#else
    {PART_PATCH,             944*1024,  32*1024,  EMMC_USER_PART}, /* patch             32M   p48*/
    {PART_BOOTFAIL_INFO,     976*1024,  2*1024,   EMMC_USER_PART}, /* bootfail_info     2M    p49*/
    {PART_RRECORD,           978*1024,  16*1024,  EMMC_USER_PART}, /* rrecord           16M   p50*/
    {PART_RESERVED9,         994*1024,  14*1024,  EMMC_USER_PART}, /* reserved9         14M    p51*/
    {PART_SYSTEM,            1008*1024, 2560*1024,EMMC_USER_PART}, /* system            2560M p52*/
    {PART_CUST,              3568*1024, 192*1024, EMMC_USER_PART}, /* cust              192M  p53*/
    {PART_VERSION,           3760*1024, 32*1024,  EMMC_USER_PART}, /* version           32M   p54*/
    {PART_VENDOR,            3792*1024, 608*1024, EMMC_USER_PART}, /* vendor            608M  p55*/
    {PART_PRODUCT,           4400*1024, 192*1024, EMMC_USER_PART}, /* product           192M  p56*/
    {PART_USERDATA,          4592*1024, 4096*1024,EMMC_USER_PART}, /* userdata          4096M p57*/
#endif
#endif
    {"0", 0, 0, 0},                                       /* */
};

#endif
