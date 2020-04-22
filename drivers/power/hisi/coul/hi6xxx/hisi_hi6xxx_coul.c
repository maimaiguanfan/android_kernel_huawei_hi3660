

#include "hisi_hi6xxx_coul.h"
#include <../hisi_coul_core.h>
#include <linux/hisi-spmi.h>
#include <linux/of_hisi_spmi.h>
#include "securec.h"

extern struct atomic_notifier_head coul_fault_notifier_list;
struct hi6xxx_coul_device_info *g_hi6xxx_dev = NULL;

static u64 last_eco_in  = 0;
static u64 last_eco_out = 0;
static int hisi_saved_abs_cc_mah = 0;
static int r_coul_mohm = R_COUL_MOHM;

static void hi6xxx_coul_cc_write_pro(u8 lock)
{
#if defined(CONFIG_HISI_COUL_HI6421V700) ||\
    defined(CONFIG_HISI_COUL_HI6421V600) || defined(CONFIG_HISI_COUL_HI6555V200)
#else
    u8 val = COUL_WRITE_LOCK;
    if(UNLOCK == lock)
        val = COUL_WRITE_UNLOCK;

    HI6XXX_REG_WRITE(HI6XXX_DEBUG_WRITE_PRO, val);
#endif
}

/*******************************************************
  Function:        hi6xxx_coul_set_nv_save_flag
  Description:     set coul nv save success flag
  Input:           nv_flag: success 1, fail 0
  Output:          NA
  Return:          NA
  Remark:          the flag is read by fastboot
*******************************************************/
void hi6xxx_coul_set_nv_save_flag(int nv_flag)
{
    unsigned char val = 0;

    val = HI6XXX_REG_READ(HI6XXX_NV_SAVE_SUCCESS);
    if (NV_SAVE_SUCCESS == nv_flag){
        HI6XXX_REG_WRITE(HI6XXX_NV_SAVE_SUCCESS, (val | NV_SAVE_BITMASK));
    } else {
        HI6XXX_REG_WRITE(HI6XXX_NV_SAVE_SUCCESS, (val & (~NV_SAVE_BITMASK)));
    }
}
/*******************************************************
  Function:        hi6xxx_coul_get_nv_read_flag
  Description:     get coul nv read success flag
  Input:           NA
  Output:          NA
  Return:          success:1 fail:0
  Remark:         the flag is written by fastboot
*******************************************************/
int hi6xxx_coul_get_nv_read_flag(void)
{
    unsigned char val = 0;

    val = HI6XXX_REG_READ(HI6XXX_NV_READ_SUCCESS);
    if (val & NV_READ_BITMASK) {
        return NV_READ_SUCCESS;
    } else {
        return NV_READ_FAIL;
    }
}
/*******************************************************
  Function:        hi6xxx_coul_get_use_saved_ocv_flag
  Description:     get the flag that use saved ocv
  Input:           NA
  Output:          NA
  Return:          1:saved ocv
                   0:not saved ocv
  Remark:          the flag is written by fastboot
*******************************************************/
int hi6xxx_coul_get_use_saved_ocv_flag(void)
{
    unsigned char val = 0;
    val = HI6XXX_REG_READ(HI6XXX_OCV_CHOOSE);
    if (val & USE_SAVED_OCV_FLAG){
        return 1;
    }else {
        return 0;
    }
}

/*******************************************************
  Function:        hi6xxx_coul_get_delta_rc_ignore_flag
  Description:     get delta rc ignore flag
  Input:           NA
  Output:          NA
  Return:          1:not calculate delta rc
                   0:calculate delta rc
  Remark:          the flag is written by fastboot
*******************************************************/
int hi6xxx_coul_get_delta_rc_ignore_flag(void)
{
    unsigned char use_delta_rc_flag = 0;
    use_delta_rc_flag = HI6XXX_REG_READ(HI6XXX_DELTA_RC_SCENE);
    if(use_delta_rc_flag & DELTA_RC_SCENE_BITMASK)
    {
        use_delta_rc_flag &= ~DELTA_RC_SCENE_BITMASK;//clear the flag after read
        HI6XXX_REG_WRITE(HI6XXX_DELTA_RC_SCENE,use_delta_rc_flag);
        return 1;
    }
    return 0;
}

/*******************************************************
  Function:        hi6xxx_coul_is_battery_moved
  Description:     whether battery is moved
  Input:           NULL
  Output:          NULL
  Return:          0:not moved, 1: moved
********************************************************/
int hi6xxx_coul_is_battery_moved(void)
{
    unsigned char val;
    val = HI6XXX_REG_READ(HI6XXX_BATTERY_MOVE_ADDR);

    if (val == BATTERY_MOVE_MAGIC_NUM){
        HI6XXX_COUL_INF("Battey not moved \n");
        return 0;
    }
    else {
        HI6XXX_COUL_INF("Battey  moved \n");
        HI6XXX_REG_WRITE(HI6XXX_BATTERY_MOVE_ADDR, BATTERY_MOVE_MAGIC_NUM);
        return 1;
    }
}

/*******************************************************
  Function:        hi6xxx_coul_set_battery_move_magic
  Description:     set battery move magic num
  Input:           move flag 1:plug out 0:plug in
  Output:          NULL
  Return:          NULL
********************************************************/
void hi6xxx_coul_set_battery_move_magic(int move_flag)
{
    unsigned char val;
    if (move_flag){
        val = BATTERY_PLUGOUT_SHUTDOWN_MAGIC_NUM;
    } else {
        val = BATTERY_MOVE_MAGIC_NUM;
    }
    HI6XXX_REG_WRITE(HI6XXX_BATTERY_MOVE_ADDR, val);
}

/*******************************************************
  Function:      hi6xxx_coul_get_fifo_depth
  Description:   get coul fifo depth
  Input:         NULL
  Output:        NULL
  Return:        depth of fifo
********************************************************/
static int hi6xxx_coul_get_fifo_depth(void)
{
    return FIFO_DEPTH;
}

/*******************************************************
  Function:      hi6xxx_coul_get_coul_time
  Description:   get coulomb total(in and out) time
  Input:         NULL
  Output:        NULL
  Return:        sum of total time
********************************************************/
static unsigned int hi6xxx_coul_get_coul_time(void)
{
    unsigned int cl_in_time = 0;
    unsigned int cl_out_time = 0;

    udelay(110);
    HI6XXX_REGS_READ(HI6XXX_CHG_TIMER_BASE,&cl_in_time,4);
    HI6XXX_REGS_READ(HI6XXX_LOAD_TIMER_BASE,&cl_out_time,4);

    return (cl_in_time + cl_out_time);
}

/*******************************************************
  Function:      hi6xxx_coul_get_coul_time
  Description:   clear coulomb total(in and out) time
  Input:         NULL
  Output:        NULL
  Return:        sum of total time
********************************************************/

static void hi6xxx_coul_clear_coul_time(void)
{
    unsigned int cl_time = 0;
    udelay(110);
    HI6XXX_REGS_WRITE(HI6XXX_CHG_TIMER_BASE,&cl_time, 4);
    HI6XXX_REGS_WRITE(HI6XXX_LOAD_TIMER_BASE,&cl_time, 4);
}
/*******************************************************
  Function:      hi6xxx_coul_convert_ocv_regval2uv
  Description:   convert register value to uv
  Input:         reg_val:voltage reg value
  Output:        NULL
  Return:        value of register in uV
********************************************************/
static  int hi6xxx_coul_convert_ocv_regval2uv(short reg)
{
    s64 temp;
    unsigned short reg_val;
    reg_val = (unsigned short) reg;

    if (reg_val & INVALID_TO_UPDATE_FCC) {
        reg_val &= (~INVALID_TO_UPDATE_FCC);
    }

#if defined(CONFIG_HISI_COUL_HI6421V600) || defined(CONFIG_HISI_COUL_HI6555V200)
    /*
        1 bit = 225.1196 uv = 4.8/21322 V = 4.8/21322 * 1000 * 1000 uV
                = 24 * 1000 * 100/ 10661 uV
    */
    temp = (s64)((s64)(reg_val)  * (s64)(24 * 1000 * 100));

    temp = div_s64(temp, 10661);

    temp = (s64) v_offset_a *temp;
    temp = div_s64(temp, 1000000);
    temp += v_offset_b;

#else
    /*
       code(15bit) * 1.3 * 5 *a / 2^15 + b
       = code * 13 *a / 2^16 + b (a = 1000000, b = 0)
       1bit = 198.364 uV (High 16bit)
    */
    temp = (s64)reg_val * 13;
    temp = temp * (s64)(v_offset_a);
    temp = div_s64(temp, 65536);
    temp += v_offset_b;
#endif
    return (int)temp;
}


/*******************************************************
  Function:      hi6xxx_coul_convert_ocv_uv2regval
  Description:   convert uv value to Bit for register
  Input:         reg_val: uv
  Output:        NULL
  Return:        value of register in uV
********************************************************/
static  unsigned short  hi6xxx_coul_convert_ocv_uv2regval(int uv_val)
{
    unsigned short ret;
    s64 temp;
#if defined(CONFIG_HISI_COUL_HI6421V600) || defined(CONFIG_HISI_COUL_HI6555V200)
    /*
        1 bit = 225.1196 uv = 4.8/21322 V
            = 4.8/21322 * 1000 * 1000 uV
            =10661 /  24 * 1000 * 100 uV
    */
    temp = (s64)(uv_val - v_offset_b);
    temp = temp * 1000000;
    temp = div_s64(temp, v_offset_a);

    temp = (s64)(temp * (s64)10661);
    temp = div_s64(temp,2400000);

    ret = (short)temp;
#else
    /*
        uv_val = code(15bit) * 1.3 * 5 *a / 2^15 + b
        = code * 13 *a / 2^16 + b
    */
    temp = (s64)(uv_val - v_offset_b)*65536;
    temp = div_s64(temp, 13);
    temp = div_s64(temp, v_offset_a);
    ret = (unsigned short)temp;
#endif
    return ret;
}

/*******************************************************
  Function:      hi6xxx_coul_convert_ocv_regval2ua
  Description:   convert register value to current(uA)
  Input:         reg_val:current reg value
  Output:        NULL
  Return:        value of register in uA
********************************************************/
static int hi6xxx_coul_convert_ocv_regval2ua(short reg_val)
{
    int ret;
    s64 temp;

#if defined(CONFIG_HISI_COUL_HI6421V600) || defined(CONFIG_HISI_COUL_HI6555V200)
    /*
      10 mohm resistance: 1 bit = 5/10661 A = 5*1000*1000 / 10661 uA
      20 mohm resistance: 1 bit = 10 mohm / 2
      30 mohm resistance: 1 bit = 10 mohm / 3
       high bit = 1 is in, 0 is out
    */
    ret = reg_val;
    temp = (s64)(ret) * (s64)(1000 * 1000 * 5);
    temp = div_s64(temp, 10661);

    ret = 10 * temp / r_coul_mohm;
    temp = (s64) c_offset_a *ret;
    ret = div_s64(temp, 1000000);
    ret += c_offset_b;

#else
    /*
    6421V700:Current
        = code * / 2^(n-1) * 1.3 * (1000/10) * (1000/Rsense)
        = code *130000 / Rsense / 2^(n-1)
    6421V800:Current
        = code * / 2^(n-1) * 0.65 * (1000/10) * (1000/Rsense)
        = code *130000 / Rsense / 2^(n-1)
        if Rsense in mohm, Current in mA
        if Rsense in uohm, Current in uA
        high bit = 1 is in, 0 is out
    */
    temp =  (s64)reg_val * (s64)130000;
    temp = div_s64(temp, r_coul_mohm);
    temp = temp * 1000; // mA to uA
    temp = div_s64(temp, 32768);

    temp = (s64) c_offset_a *temp;
    ret = div_s64(temp, 1000000);
    ret += c_offset_b;

#ifndef CONFIG_HISI_COUL_HI6421V700
        ret = ret / 2;
#endif
#endif

    HI6XXX_COUL_DBG("[%s] reg_val 0x%lx, ret 0x%lx\n", __func__, reg_val, ret);

    return ret;
}

/*******************************************************
  Function:      hi6xxx_coul_convert_regval2uv
  Description:   convert register value to uv
  Input:         reg_val:voltage reg value
  Output:        NULL
  Return:        value of register in uV
********************************************************/
static int hi6xxx_coul_convert_regval2uv(unsigned int reg_val)
{
    s64 temp;
#if defined(CONFIG_HISI_COUL_HI6421V600) || defined(CONFIG_HISI_COUL_HI6555V200)
    short reg_val_temp = reg_val;

    if (reg_val_temp & INVALID_TO_UPDATE_FCC) {
        reg_val_temp &= (~INVALID_TO_UPDATE_FCC);
        //return -1;
    }

    /*
        1 bit = 225.1196 uv = 4.8/21322 V = 4.8/21322 * 1000 * 1000 uV
                = 24 * 1000 * 100/ 10661 uV
    */
    temp = (s64)((s64)(reg_val_temp)  * (s64)(24 * 1000 * 100));

    temp = div_s64(temp, 10661);

    temp = (s64) v_offset_a *temp;
    temp = div_s64(temp, 1000000);
    temp += v_offset_b;

    HI6XXX_COUL_DBG("[%s] reg_val 0x%lx, temp %d\n", __func__, reg_val, temp);

    return (int)temp;

#else
    int val = 0;
    int ret;
    /*
        uv_val = code(23bit) * 1.3 * 5 *a / 2^23 + b
        = code * 13 *a / 2^24 + b (a = 1000000, b = 0)
        1bit = 0.77486 uV (Total 24bit)
    */
    if(reg_val & 0x800000) {
        reg_val |= ((unsigned int)0xff << 24);
        val = ((~reg_val)+1) & (~0x800000);
    } else
        val = reg_val;

    temp = (s64)val * 13;
    temp = temp * (s64)(v_offset_a);
    temp = div_s64(temp, 16777216);
    temp += v_offset_b;

    ret = (int)temp;

    HI6XXX_COUL_DBG("[%s] reg_val 0x%lx, temp %d\n", __func__, reg_val, ret);

    if(reg_val & 0x800000)
        return -ret;
    else
        return ret;
#endif
}
#if defined(CONFIG_HISI_COUL_HI6421V600) || defined(CONFIG_HISI_COUL_HI6555V200)
#else

/*******************************************************
  Function:      hi6xxx_coul_convert_regval2temp
  Description:   convert register value to uv
  Input:         reg_val:voltage reg value
  Output:        NULL
  Return:        value of register in  ¡æ
  Remark:     vol = code(23bit) * 1.3 / 2^23
              temp = (vol - 358.68mv )/ 1.3427 ¡æ (vol = 1.3427 * temp + 358.68)
              For 6421v700/6421v800
********************************************************/
static  int hi6xxx_coul_convert_regval2temp(unsigned int reg_val)
{
    s64 temp;
    int val = 0;

    struct hi6xxx_coul_device_info *di = g_hi6xxx_dev;
    if(NULL == di) {
        HI6XXX_COUL_ERR("[%s]di is null\n",__FUNCTION__);
        return -ENODEV;
    }

    if(reg_val & 0x800000) {
        reg_val |= ((unsigned int)0xff << 24);
        val = ((~reg_val)+1) & (~0x800000);
        val = -val;
    } else
        val = reg_val;

    /* reg2uv*/
    temp = (s64)val * 1300000;
    temp = div_s64(temp, 8388608);

    if(COUL_HI6421V700 != di->chip_version) {
        temp += 650000;
    }

    /* uv2temp */
    temp = (temp - 358680) * 1000;
    temp = div_s64(temp, 1342700);
    HI6XXX_COUL_DBG("[%s] reg_val 0x%lx, temp %d\n", __func__, reg_val, temp);

    return (int)temp;
}
#endif
/*******************************************************
  Function:      hi6xxx_coul_convert_uv2regval
  Description:   convert uv value to Bit for register
  Input:         reg_val: uv
  Output:        NULL
  Return:        value of register in uV
  Remark:      mv_val = code(23bit) * 1.3 * 5 *a / 2^23 + b  = code * 13 *a / 2^24 + b
********************************************************/
static  unsigned int  hi6xxx_coul_convert_uv2regval(int uv_val)
{
    unsigned int ret;
    s64 temp;

#if defined(CONFIG_HISI_COUL_HI6421V600) || defined(CONFIG_HISI_COUL_HI6555V200)
    temp = (s64)(uv_val - v_offset_b);
    temp = temp * 1000000;
    temp = div_s64(temp, v_offset_a);

    temp = (s64)(temp * (s64)10661);
    temp = div_s64(temp,2400000);

    ret = (unsigned int)temp;
#else
    temp = (s64)(uv_val - v_offset_b)*16777216;
    temp = div_s64(temp, 13);
    temp = div_s64(temp, v_offset_a);

    ret = (unsigned int)temp;
#endif
    HI6XXX_COUL_DBG("[%s] uv_val %d, ret 0x%lx\n", __func__, uv_val, ret);

    return ret;
}

/*******************************************************
  Function:      hi6xxx_coul_convert_regval2ua
  Description:   convert register value to current(uA)
  Input:         reg_val:current reg value
  Output:        NULL
  Return:        value of register in uA
  Remark:
  6421V700:Current = code * / 2^(n-1) * 1.3 * (1000/10) * (1000/Rsense)
                        = code *130000 / Rsense / 2^(n-1)
  6421V800:Current = code * / 2^(n-1) * 0.65 * (1000/10) * (1000/Rsense)
                   = code *130000 / Rsense / 2^(n-1)


                        if Rsense in mohm, Current in mA
                        if Rsense in uohm, Current in uA
                     high bit = 1 is in, 0 is out
********************************************************/
static  int hi6xxx_coul_convert_regval2ua(unsigned int reg_val)
{
    int ret;
    s64 temp;

#if defined (CONFIG_HISI_COUL_HI6421V600) || defined(CONFIG_HISI_COUL_HI6555V200)
    ret = (short)reg_val;
    temp = (s64)(ret) * (s64)(1000 * 1000 * 5);
    temp = div_s64(temp, 10661);

    ret = 10 * temp / r_coul_mohm;
    temp = (s64) c_offset_a *ret;
    ret = div_s64(temp, 1000000);
    ret += c_offset_b;

    HI6XXX_COUL_DBG("[%s] h%d reg_val 0x%lx, ret %d\n", __func__, __LINE__, reg_val, ret);
    return ret;
#else
    int val = 0;

    if(reg_val & 0x800000) {
        reg_val |= ((unsigned int)0xff << 24);
        val = ((~reg_val)+1) & (~0x800000);
    } else
        val = reg_val;

    temp =  (s64)val * 130000;
    temp = div_s64(temp, r_coul_mohm);
    temp = temp * 1000; // mA to uA
    temp = div_s64(temp, 8388608);

    temp = (s64) c_offset_a *temp;
    ret = div_s64(temp, 1000000);
    ret += c_offset_b;

    #ifndef CONFIG_HISI_COUL_HI6421V700
    ret = ret / 2;
    #endif

    HI6XXX_COUL_DBG("[%s] h%d reg_val 0x%lx, ret %d\n", __func__, __LINE__, reg_val, ret);

    if(reg_val & 0x800000)
        return -ret;
    else
        return ret;
#endif
}

/*******************************************************
  Function:      hi6xxx_coul_convert_regval2uah
  Description:   hi6xxx convert register value to uah
  Input:         reg_val:cc reg val
  Output:        NULL
  Return:        uah value of reg
********************************************************/
static int hi6xxx_coul_convert_regval2uah(u64 reg_val)
{
    int ret;
    s64 temp;

#if defined(CONFIG_HISI_COUL_HI6421V600)
    temp = (s64)reg_val;
    temp = 10 * temp / r_coul_mohm;
    temp = temp * BIT_FOR_UAH_DCXO_586;
    temp = (s64)div_s64(temp, 10000000);

    temp = (s64) c_offset_a * temp;
    ret = div_s64(temp, 1000000);

#elif defined(CONFIG_HISI_COUL_HI6555V200)

    u8 coul_clk_mode = 0;
    temp = reg_val;
    coul_clk_mode = HI6XXX_REG_READ(COUL_CLK_MODE_ADDR);
    if((coul_clk_mode & XO32K_MODE_MSK) == COUL_32K_CLK_MODE){
        temp = temp * BIT_FOR_UAH_32K;
    } else {
        temp = temp * BIT_FOR_UAH_DCXO_586;
    }
    temp = (s64)div_s64(temp, 10000000);

    temp = (s64) c_offset_a * temp;
    ret = div_s64(temp, 1000000);

#else
    /*
    6421V700: temp * 10763  / r_coul_mohm  * 10E-10   (mAh)
                    temp * 10763  / r_coul_mohm  * 10E-7   (uAh)
    6421V800: temp * 5381.5  / r_coul_mohm  * 10E-10   (mAh)
    */
    temp = reg_val;
    temp = temp * 10763 / r_coul_mohm;
    temp = (s64)div_s64(temp, 10000000);

    temp = (s64) c_offset_a * temp;
    ret = (int)div_s64(temp, 1000000);

#ifndef CONFIG_HISI_COUL_HI6421V700
        ret = ret / 2;
#endif
#endif
    HI6XXX_COUL_DBG("[%s] reg_val 0x%lx, ret %d\n", __func__, reg_val, ret);

    return ret;
}

/*******************************************************
  Function:      hi6xxx_coul_convert_uah2regval
  Description:   hi6xxx convert uah value to reg
  Input:         reg_val:uah
  Output:        NULL
  Return:        uah value in register
********************************************************/
static u64 hi6xxx_coul_convert_uah2regval(unsigned int uah)
{
    u64 ret = 0;
    u64 temp;

#if defined(CONFIG_HISI_COUL_HI6421V600)
    /*
        1bit = 1bit current * 0.11 c = 5/10661 * 11/100 c
             = 5/10661 * 11/100 * 1000/3600 mAh
             = 11 / (10661*2*36) mAh = 11 * 1000/ (10661 *2 *36) uAh
             = 11 * 125/ (10661* 9) uAh
    */

    temp = uah;
    temp = temp * 1000000;
    temp = div_s64(temp, c_offset_a);

    temp = temp * 10000000;
    temp = div_s64(temp, BIT_FOR_UAH_DCXO_586);
    ret = (int)(temp * r_coul_mohm /10);

#elif defined(CONFIG_HISI_COUL_HI6555V200)
    u8 coul_clk_mode = 0;

    temp = uah;
    temp = temp * 1000000;
    temp = div_s64(temp, c_offset_a);

    temp = temp * 10000000;
    coul_clk_mode = HI6XXX_REG_READ(COUL_CLK_MODE_ADDR);
    if( (coul_clk_mode & XO32K_MODE_MSK) == COUL_32K_CLK_MODE){
        temp =div_s64( temp, BIT_FOR_UAH_32K);
    } else {
        temp = div_s64(temp, BIT_FOR_UAH_DCXO_586);
    }

    ret = (int)temp;

#else
    /*
    6421V700: mAh = temp * 10763  / r_coul_mohm  * 10E-10   (mAh)
          uAh = temp * 10763  / r_coul_mohm  * 10E-7   (uAh)
          code = uAh * 10^7 * r_coul_mohm / 10763
    6421V800: mAh = temp * 5381.5  / r_coul_mohm  * 10E-10   (mAh)
    */

    #ifndef CONFIG_HISI_COUL_HI6421V700
    uah = uah * 2;
    #endif

    temp = uah;
    temp = temp * 1000000;
    temp = div_s64(temp, c_offset_a);

    temp = temp * 10000000;
    temp = temp * r_coul_mohm;
    ret = (u64)div_s64(temp, 10763);
#endif
    HI6XXX_COUL_DBG("[%s] uah %d, ret 0x%lx\n", __func__, uah, ret);

    return ret;
}


/*******************************************************
  Function:      hi6xxx_coul_calculate_cc_uah
  Description:   value of out_uah - in_uah recorded by  coulomb
  Input:         NULL
  Output:        NULL
  Return:        value of  uah through coulomb
  Remark:        adjusted by offset integrated on time
********************************************************/
static int hi6xxx_coul_calculate_cc_uah(void)
{
    u64 cc_in = 0;
    u64 cc_out = 0;
    unsigned int cl_in_time = 0, cl_out_time = 0;
    int cc_uah_in = 0;
    int cc_uah_out = 0;
    int cc_uah = 0;
    HI6XXX_REGS_READ(HI6XXX_CL_IN_BASE, &cc_in, CC_REG_NUM);
    HI6XXX_REGS_READ(HI6XXX_CL_OUT_BASE, &cc_out, CC_REG_NUM);

    cc_uah_out = hi6xxx_coul_convert_regval2uah(cc_out);
    cc_uah_in  = hi6xxx_coul_convert_regval2uah(cc_in);

    HI6XXX_REGS_READ(HI6XXX_CHG_TIMER_BASE,&cl_in_time,4);
    HI6XXX_REGS_READ(HI6XXX_LOAD_TIMER_BASE,&cl_out_time,4);
    /* uah = uas/3700 = ua*s/3700 */
    cc_uah_in  -= div_s64((s64) cl_in_time * c_offset_b, 3600);
    cc_uah_out += div_s64((s64) cl_out_time * c_offset_b, 3600);
    cc_uah = cc_uah_out - cc_uah_in;

    HI6XXX_COUL_INF("ccout_reg=0x%llx,ccin_reg = 0x%llx, cl_out_time=%d, cl_in_time=%d, cc_adjust=%d uah\n",
           cc_out, cc_in, cl_out_time, cl_in_time, cc_uah);

    return cc_uah;
}

/*******************************************************
  Function:      hi6xxx_coul_save_cc_uah
  Description:   get coulomb total(in and out) time
  Input:         cc_uah:save cc uah
  Output:        NULL
  Return:        sum of total time
********************************************************/
static void hi6xxx_coul_save_cc_uah(int cc_uah)
{
    u64 reg = 0;
    hi6xxx_coul_cc_write_pro(UNLOCK);
    if (cc_uah > 0){
        reg = hi6xxx_coul_convert_uah2regval(cc_uah);
        udelay(110);
        HI6XXX_REGS_WRITE(HI6XXX_CL_OUT_BASE, &reg, CC_REG_NUM);
        reg = 0;
        HI6XXX_REGS_WRITE(HI6XXX_CL_IN_BASE, &reg, CC_REG_NUM);
    } else {
        reg = hi6xxx_coul_convert_uah2regval(-cc_uah);
        udelay(110);
        HI6XXX_REGS_WRITE(HI6XXX_CL_IN_BASE, &reg, CC_REG_NUM);
        reg = 0;
        HI6XXX_REGS_WRITE(HI6XXX_CL_OUT_BASE, &reg, CC_REG_NUM);
    }
    hi6xxx_coul_cc_write_pro(LOCK);
}

/*******************************************************
  Function:        hi6xxx_coul_save_ocv
  Description:     coul save ocv
  Input:           ocv:ocv vol uv
                   invalid_fcc_up_flag: no update fcc 1,update 0
  Output:          NA
  Return:          NA
*******************************************************/
void hi6xxx_coul_save_ocv(int ocv, int invalid_fcc_up_flag)
{
    unsigned short ocvreg = hi6xxx_coul_convert_ocv_uv2regval(ocv);

    if (invalid_fcc_up_flag){
        ocvreg |= INVALID_TO_UPDATE_FCC;
    }
    HI6XXX_REGS_WRITE(HI6XXX_SAVE_OCV_ADDR,&ocvreg, 2);
    HI6XXX_COUL_INF("save ocv, ocv=%d,reg=%d", ocv, ocvreg);
}

/*******************************************************
  Function:        hi6xxx_coul_clear_ocv
  Description:     clear ocv
  Input:           NA
  Output:          NA
  Return:          NA
*******************************************************/
void hi6xxx_coul_clear_ocv(void)
{
    unsigned short ocvreg = 0;
    HI6XXX_REGS_WRITE(HI6XXX_SAVE_OCV_ADDR,&ocvreg, 2);
}

/*******************************************************
  Function:        hi6xxx_coul_get_ocv
  Description:     get saved ocv
  Input:           NA
  Output:          NA
  Return:          OCV(reg value)
*******************************************************/
short hi6xxx_coul_get_ocv(void)
{
    unsigned short ocvreg = 0;

    HI6XXX_REGS_READ(HI6XXX_SAVE_OCV_ADDR,&ocvreg, 2);
    return (short) ocvreg;
}

/*******************************************************
  Function:        hi6xxx_coul_save_ocv_temp
  Description:     coul save ocv temp
  Input:           ocv_temp: temp*10
  Output:          NA
  Return:          NA
*******************************************************/
void hi6xxx_coul_save_ocv_temp(short ocv_temp)
{
    HI6XXX_REGS_WRITE(HI6XXX_SAVE_OCV_TEMP_ADDR, &ocv_temp, 2);
    HI6XXX_COUL_INF("save ocv temp, ocv_temp=%d\n",ocv_temp);
}

/*******************************************************
  Function:        hi6xxx_coul_clear_ocv_temp
  Description:     clear ocv temp
  Input:           NA
  Output:          NA
  Return:          NA
*******************************************************/
void hi6xxx_coul_clear_ocv_temp(void)
{
    short ocv_temp = 0;
    HI6XXX_REGS_WRITE(HI6XXX_SAVE_OCV_TEMP_ADDR,&ocv_temp,2);
}

/*******************************************************
  Function:        hi6xxx_coul_get_ocv_temp
  Description:     get saved ocv temp
  Input:           NA
  Output:          NA
  Return:          OCV temp(temp*10)
*******************************************************/
short hi6xxx_coul_get_ocv_temp(void)
{
    short ocv_temp = 0;
    HI6XXX_REGS_READ(HI6XXX_SAVE_OCV_TEMP_ADDR,&ocv_temp,2);
    return ocv_temp;
}
/*******************************************************
  Function:        hi6xxx_coul_get_fcc_valid_up_flag
  Description:     get fcc update flag
  Input:           NA
  Output:          NA
  Return:          no up:1 is up:1
*******************************************************/
int  hi6xxx_coul_get_fcc_invalid_up_flag(void)
{
    unsigned short ocvreg = 0;
    ocvreg = hi6xxx_coul_get_ocv();
    if (ocvreg & INVALID_TO_UPDATE_FCC){
        return 1;
    }
    return 0;
}
/*******************************************************
  Function:      hi6xxx_coul_get_battery_voltage_uv
  Description:   get battery voltage in uV
  Input:         NULL
  Output:        NULL
  Return:        battery voltage in uV
********************************************************/
int hi6xxx_coul_get_battery_voltage_uv(void)
{
    int uv = 0;
    unsigned int regval = 0;
    HI6XXX_REGS_READ(HI6XXX_V_OUT,&regval, REG_NUM);
    uv = hi6xxx_coul_convert_regval2uv(regval);

    HI6XXX_COUL_DBG("[%s] 0x%x, uv %d\n", __func__, regval, uv);
    return uv;
}

/*******************************************************
  Function:      hi6xxx_coul_get_battery_current_ua
  Description:   get battery current in uA
  Intput:        NULL
  Output:        NULL
  Return:        battery current in uA
********************************************************/
int hi6xxx_coul_get_battery_current_ua(void)
{
    int ua = 0;
    unsigned int regval = 0;
    HI6XXX_REGS_READ(HI6XXX_CURRENT,&regval,REG_NUM);
    ua = hi6xxx_coul_convert_regval2ua(regval);
    HI6XXX_COUL_DBG("[%s] 0x%x, ma %d\n", __func__, regval, ua);
    return ua;
}
 /*******************************************************
  Function:      hi6xxx_coul_get_battery_vol_uv_from_fifo
  Description:   get battery vol in uv from fifo
  Intput:        fifo_order:fifo serial number 0-9
  Output:        NULL
  Return:        battery voltage in uv
********************************************************/
int hi6xxx_coul_get_battery_vol_uv_from_fifo(unsigned int fifo_order)
{
    int uv = 0;
    unsigned int regval = 0;
    if (fifo_order > FIFO_DEPTH){
        fifo_order = 0;
    }
    HI6XXX_REGS_READ((HI6XXX_VOL_FIFO_BASE + REG_NUM*fifo_order),&regval,REG_NUM);/*lint !e647 */

    uv = hi6xxx_coul_convert_regval2uv(regval);

    HI6XXX_COUL_DBG("[%s] 0x%x, uv\n", __func__, regval, uv);
    return uv;
}

/*******************************************************
  Function:      hi6xxx_coul_get_battery_cur_ua_from_fifo
  Description:   get battery cur in ua from fifo
  Intput:        fifo_order:fifo serial number 0-9
  Output:        NULL
  Return:        battery current in ua
********************************************************/
int hi6xxx_coul_get_battery_cur_ua_from_fifo(unsigned int fifo_order)
{
    int ua = 0;
    unsigned int regval = 0;
    if (fifo_order > FIFO_DEPTH){
        fifo_order = 0;
    }
    HI6XXX_REGS_READ((HI6XXX_CUR_FIFO_BASE + REG_NUM*fifo_order),&regval, REG_NUM);/*lint !e647 */

    ua = hi6xxx_coul_convert_regval2ua(regval);

    HI6XXX_COUL_DBG("[%s] 0x%x, ua %d\n", __func__, regval, ua);
    return ua;
}

/*******************************************************
    Function:      hi6xxx_coul_get_offset_current_mod
    Description:   get current offset mod
    Intput:        NULL
    Output:        NULL
    Return:        current offset mod value
********************************************************/
short hi6xxx_coul_get_offset_current_mod(void)
{
    short regval = 0;
#if defined(CONFIG_HISI_COUL_HI6421V600) || defined(CONFIG_HISI_COUL_HI6555V200)
    HI6XXX_REGS_READ(HI6XXX_OFFSET_CUR_MODIFY_BASE, &regval, REG_NUM);
#endif
    return regval;
}

/*******************************************************
  Function:      hi6xxx_coul_get_offset_vol_mod
  Description:   get vol offset mod
  Intput:        NULL
  Output:        NULL
  Return:        vol offset mod value
********************************************************/
short hi6xxx_coul_get_offset_vol_mod(void)
{
    short regval = 0;
#if defined(CONFIG_HISI_COUL_HI6421V600)
    HI6XXX_REGS_READ(HI6XXX_OFFSET_VOL_MODIFY_BASE, &regval, REG_NUM);
#endif
    return regval;
}

/*******************************************************
  Function:      hi6xxx_coul_set_offset_vol_mod
  Description:   set vol offset mod
  Intput:        NULL
  Output:        NULL
  Return:        vol offset mod value
********************************************************/
void hi6xxx_coul_set_offset_vol_mod(void)
{
#if defined(CONFIG_HISI_COUL_HI6421V600)
    short regval = 0;
    HI6XXX_REGS_WRITE(HI6XXX_OFFSET_VOL_MODIFY_BASE, &regval, REG_NUM);
#endif
}
/*******************************************************
  Function:        hi6xxx_coul_get_fifo_avg_data
  Description:     get coul fifo average vol/current value(uv/ua)
  Input:           NULL
  Output:          struct vcdata:avg , max and min cur, vol
  Return:          NULL
********************************************************/
static void hi6xxx_coul_get_fifo_avg_data(struct vcdata *vc)
{
    int i;
    unsigned int vol_reset_value = 0xffffff;/*lint !e569 */
    int abnormal_value_cnt = 0;
    static unsigned int vol_fifo[FIFO_DEPTH];
    static unsigned int cur_fifo[FIFO_DEPTH];
    int vol,cur;
    int max_cur, min_cur;
    int vols, curs;
    if( NULL == vc )
    {
        HI6XXX_COUL_INF("NULL point in [%s]\n", __func__);
           return;
    }
    for (i=0; i<FIFO_DEPTH; i++) {
        HI6XXX_REGS_READ(HI6XXX_VOL_FIFO_BASE+i*REG_NUM, &vol_fifo[i], REG_NUM);/*lint !e647 */
        HI6XXX_REGS_READ(HI6XXX_CUR_FIFO_BASE+i*REG_NUM, &cur_fifo[i], REG_NUM);/*lint !e647 */
    }

    if(vol_fifo[0] != vol_reset_value) {
        vol=hi6xxx_coul_convert_regval2uv(vol_fifo[0])/1000;
        cur=hi6xxx_coul_convert_regval2ua(cur_fifo[0])/1000;
    } else {
        vol = 0;
        cur = 0;
    }

    vols=vol;
    curs=cur;

    max_cur = min_cur = cur;

    for (i=1; i<FIFO_DEPTH; i++){
        if(vol_fifo[i] != vol_reset_value)
        {
            vol = hi6xxx_coul_convert_regval2uv(vol_fifo[i])/1000;
            cur = hi6xxx_coul_convert_regval2ua(cur_fifo[i])/1000;

            vols += vol;
            curs += cur;

            if (cur > max_cur){
                max_cur = cur;
            } else if (cur < min_cur){
                min_cur = cur;
            }
        } else {
            abnormal_value_cnt++;
        }
    }

    vol = vols/(FIFO_DEPTH - abnormal_value_cnt);
    cur = curs/(FIFO_DEPTH - abnormal_value_cnt);

    vc->avg_v = vol;
    vc->avg_c = cur;
    vc->max_c = max_cur;
    vc->min_c = min_cur;

    HI6XXX_COUL_INF("avg_v = %d, avg_c = %d, max_c = %d, min_c = %d \n", vc->avg_v,
                                     vc->avg_c, vc->max_c, vc->min_c);
}

/*******************************************************
  Function:      hi6xxx_coul_clear_cc_register
  Description:    clear coulomb uah record
  Input:          NULL
  Output:         NULL
  Return:         NULL
********************************************************/
static int hi6xxx_coul_get_abs_cc(void)
{
    return hisi_saved_abs_cc_mah;
}

/*******************************************************
  Function:      hi6xxx_coul_get_ate_a
  Description:   get v_offset a value
  Input:         NULL
  Output:        NULL
  Return:        v_offset a value
********************************************************/
static int  hi6xxx_coul_get_ate_a(void)
{
    unsigned short regval = 0;
    unsigned char  a_low  = 0;
    unsigned char  a_high = 0;
    a_low  = HI6XXX_REG_READ(HI6XXX_VOL_OFFSET_A_ADDR_0);
    a_high = HI6XXX_REG_READ(HI6XXX_VOL_OFFSET_A_ADDR_1);

#if defined(CONFIG_HISI_COUL_HI6421V600) || defined(CONFIG_HISI_COUL_HI6555V200)
    regval = (((a_low >> 6) & VOL_OFFSET_A_LOW_VALID_MASK) | ((a_high << 2) & VOL_OFFSET_A_HIGH_VALID_MASK)) & VOL_OFFSET_A_VALID_MASK;
#else
    regval = ((a_low & VOL_OFFSET_A_LOW_VALID_MASK) | ((a_high << 1) & VOL_OFFSET_A_HIGH_VALID_MASK)) & VOL_OFFSET_A_VALID_MASK;
#endif
    return (VOL_OFFSET_A_BASE + regval*VOL_OFFSET_A_STEP);
}
/*******************************************************
  Function:      hi6xxx_coul_get_ate_b
  Description:   get v_offset b value
  Input:         NULL
  Output:        NULL
  Return:        v_offset b value
********************************************************/
static int hi6xxx_coul_get_ate_b(void)
{
    unsigned char regval = 0;
    regval = HI6XXX_REG_READ(HI6XXX_VOL_OFFSET_B_ADDR);
    /*6421v700v800 :bit[1-7]
      6421v600 :bit[0-5]*/
    regval &= VOL_OFFSET_B_VALID_MASK;

#if defined(CONFIG_HISI_COUL_HI6421V600) || defined(CONFIG_HISI_COUL_HI6555V200)
    return (VOL_OFFSET_B_BASE + regval*VOL_OFFSET_B_STEP);
#else
    regval = (regval >> 1);
    return ((VOL_OFFSET_B_BASE + regval*VOL_OFFSET_B_STEP) / 1000);  /* uv */
#endif
}

/*******************************************************
  Function:      hi6xxx_coul_clear_cc_register
  Description:    clear coulomb uah record
  Input:          NULL
  Output:         NULL
  Return:         NULL
********************************************************/
static void hi6xxx_coul_clear_cc_register(void)
{
    u64 ccregval = 0;

    hisi_saved_abs_cc_mah += (hi6xxx_coul_calculate_cc_uah() / 1000);
    udelay(110);
    hi6xxx_coul_cc_write_pro(UNLOCK);
    HI6XXX_REGS_WRITE(HI6XXX_CL_IN_BASE,&ccregval, CC_REG_NUM);
    HI6XXX_REGS_WRITE(HI6XXX_CL_OUT_BASE,&ccregval, CC_REG_NUM);
    hi6xxx_coul_cc_write_pro(LOCK);
}

/*******************************************************
  Function:        hi6xxx_coul_set_low_vol_val
  Description:     set low int vol val
  Input:           vol_value:low int vol val(mV)
  Output:          NA
  Return:          NA
********************************************************/
static void hi6xxx_coul_set_low_vol_val(int vol_mv)
{
    unsigned int regval = 0;
    regval = hi6xxx_coul_convert_uv2regval(vol_mv*1000);

    HI6XXX_COUL_DBG("[%s] mv %d, reg 0x%x\n", __func__, vol_mv, regval);
    udelay(110);
    HI6XXX_REGS_WRITE(HI6XXX_VOL_INT_BASE, &regval, REG_NUM);
}
/*******************************************************
  Function:        hi6xxx_coul_check_version
  Description:     check coul version
  Input:           NA
  Output:          NA
  Return:          0:success -1:fail.
********************************************************/
static int hi6xxx_coul_check_version(struct hi6xxx_coul_device_info *di)
{
    int tryloop = 0;
    u8 ver[6] = {0};
    do {
        HI6XXX_REGS_READ(HI6XXX_COUL_VERSION_ADDR, &ver, 6);
        HI6XXX_COUL_INF("do a dummy read, version is 0x%x\n", ver[0]);
        usleep_range(500,510);
        if ((tryloop++) > 5){
            HI6XXX_COUL_ERR("version is not correct!\n");
            return -1;
        }
    } while(COUL_HI6XXX != (ver[0] & 0xff));

    di->chip_proj = ((ver[0]&0x0f) << 12) | ((ver[1]&0x0f) << 8) | ((ver[2]&0x0f) << 4)| (ver[3]&0x0f);
    di->chip_version = (ver[4] << 8) | ver[5];

    return 0;
}

/*******************************************************
  Function:        hi6xxx_coul_check_debug
  Description:     check coul debug reg
  Input:           NA
  Output:          NA
  Return:          0:success -1:fail.
********************************************************/
static int hi6xxx_coul_check_debug(void)
{
#if defined(CONFIG_HISI_COUL_HI6421V600)
    u8    val    = 0;
    short ocvreg = 0;
    val = HI6XXX_REG_READ(HI6XXX_DEBUG_REG2);
    if (val){
        HI6XXX_COUL_ERR("debug reg2 is 0x%x\n",val);
    }
    val = HI6XXX_REG_READ(HI6XXX_DEBUG_REG);
    if(val){
        HI6XXX_COUL_ERR("debug reg is not 0x0\n");
	HI6XXX_REG_WRITE(HI6XXX_DEBUG_WRITE_PRO,COUL_WRITE_UNLOCK);
        HI6XXX_REG_WRITE(HI6XXX_DEBUG_REG, 0x0);
	HI6XXX_REG_WRITE(HI6XXX_DEBUG_WRITE_PRO,COUL_WRITE_LOCK);
        usleep_range(500,510);
        HI6XXX_REGS_WRITE(HI6XXX_SAVE_OCV_ADDR, &ocvreg, 2);
        usleep_range(500,510);
        return -1;
    }

#endif
    return 0;
}

/*******************************************************
  Function:        hi6xxx_coul_cali_adc
  Description:     coul calibration
  Input:           NA
  Output:          NA
  Return:          NA.
********************************************************/
static void hi6xxx_coul_cali_adc(void)
{
    unsigned char reg_val = 0;
    reg_val = HI6XXX_REG_READ(HI6XXX_COUL_STATE_REG);

    if (COUL_CALI_ING == reg_val){
        HI6XXX_COUL_INF("cali ing, don't do it again!\n");
        return;
    }
    HI6XXX_COUL_INF("calibrate!\n");
    reg_val = HI6XXX_REG_READ(HI6XXX_COUL_CTRL_REG);
    /* Mode */
    reg_val = reg_val | COUL_CALI_ENABLE;
    udelay(110);
    HI6XXX_REG_WRITE(HI6XXX_COUL_CTRL_REG, reg_val);
}
/*******************************************************
  Function:        hi6xxx_coul_clear_irq
  Description:     clear coul interrupt
  Input:           NULL
  Output:          NULL
  Remark:          clear low vol/capacity INT before coul
                   self_adjust when start up
********************************************************/
static void hi6xxx_coul_clear_irq(void)
{
#if defined(CONFIG_HISI_COUL_HI6421V600) || defined(CONFIG_HISI_COUL_HI6555V200)
    char val = 0x0F;
#else
    char val = 0x3F;
#endif
    HI6XXX_REG_WRITE(HI6XXX_COUL_IRQ_REG, val);
}
/*******************************************************
  Function:        hi6xxx_coul_enable_irq
  Description:     enable coul irq
  Input:           NULL
  Output:          NULL
  Return:          NULL
********************************************************/
 static void hi6xxx_coul_enable_irq(void)
 {
    unsigned char irq_enable_flag = ~((unsigned char)(COUL_VBAT_INT_MASK | COUL_CL_IN_MASK | COUL_CL_OUT_MASK));/*disable cl_int interrupt*/ /*disable i out/in */
    HI6XXX_REG_WRITE(HI6XXX_COUL_IRQ_MASK_REG, irq_enable_flag);
    HI6XXX_COUL_INF("Enable coul irq!\n");
 }
/*******************************************************
  Function:        hi6xxx_coul_disable_irq
  Description:     disable coul irq
  Input:           NULL
  Output:          NULL
  Return:          NULL
********************************************************/
 static void hi6xxx_coul_disable_irq(void)
 {
     unsigned char irq_disable_flag = (COUL_INT_MASK_ALL);
     HI6XXX_REG_WRITE(HI6XXX_COUL_IRQ_MASK_REG, irq_disable_flag);
     HI6XXX_COUL_INF("Mask coul irq!\n");
 }
#if defined(CONFIG_HISI_COUL_HI6421V600) || defined(CONFIG_HISI_COUL_HI6555V200)
#else
/*******************************************************
  Function:      hi6xxx_coul_convert_ua2regval
  Description:   convert register value to current(uA)
  Input:         reg_val:current reg value
  Output:        NULL
  Return:        value of register in uA
  Remark:
  6421V700: Code = mA * 2^23 / 1.3 * 10 * Rsense / 1000 / 1000
  6421V700: Code = mA * 2^23 / 0.65 * 10 * Rsense / 1000 / 1000
                     high bit = 1 is in, 0 is out,  Rsense in mohm
           only for 6421V700 & V800
********************************************************/
static  unsigned int hi6xxx_coul_convert_ua2regval(int ua)
{
    unsigned int ret;
    s64 temp;
    int val = ua;

#ifndef CONFIG_HISI_COUL_HI6421V700
    val = val * 2;
#endif

    temp = (s64)val * 1000000;
    temp = div_s64(temp, c_offset_a);

    temp = temp * 8388608;
    temp = temp *100 * r_coul_mohm;
    temp = div_s64(temp, 13);
    temp = div_s64(temp , 1000000000);

    ret = (unsigned int)temp;

    HI6XXX_COUL_DBG("[%s] ua 0x%lx, ret %d\n", __func__, ua, ret);

    return ret;
}
#endif
/*******************************************************
  Function:        hi6xxx_coul_set_i_in_event_gate
  Description:     set i in gate
  Input:           ma, should < 0
  Output:          NA
  Return:          NA
*******************************************************/
static void hi6xxx_coul_set_i_in_event_gate(int ma)
{
#if defined(CONFIG_HISI_COUL_HI6421V600) || defined(CONFIG_HISI_COUL_HI6555V200)
#else
    unsigned int reg_val = 0;

    if(ma > 0)
        ma = -ma;
    reg_val = hi6xxx_coul_convert_ua2regval(ma * 1000);
    udelay(110);
    HI6XXX_REGS_WRITE(HI6XXX_I_IN_GATE, &reg_val, REG_NUM);
#endif
}

/*******************************************************
  Function:        hi6xxx_coul_i_out_event_gate_set
  Description:     set i out gate
  Input:           ma, should > 0
  Output:          NA
  Return:          NA
*******************************************************/
static void hi6xxx_coul_set_i_out_event_gate(int ma)
{
#if defined(CONFIG_HISI_COUL_HI6421V600) || defined(CONFIG_HISI_COUL_HI6555V200)
#else
    unsigned int reg_val = 0;

    if(ma < 0)
        ma = -ma;
    reg_val = hi6xxx_coul_convert_ua2regval(ma * 1000);
    udelay(110);
    HI6XXX_REGS_WRITE(HI6XXX_I_OUT_GATE, &reg_val, REG_NUM);
#endif
}

static void hi6xxx_coul_eco_delay_enable(u8 enable)
{
#if defined(CONFIG_HISI_COUL_HI6421V700) || defined(CONFIG_HISI_COUL_HI6421V600) ||\
    defined(CONFIG_HISI_COUL_HI6555V200)
#else
        u8 val;
        val = HI6XXX_REG_READ(HI6XXX_COUL_ECO_CONFIG_ADDR);
        val = (val & ~(ECO_DELAY_EN_MASK)) | (enable << ECO_DELAY_EN_SHIFT);
        HI6XXX_REG_WRITE(HI6XXX_COUL_ECO_CONFIG_ADDR, val);
#endif
}

static void hi6xxx_coul_wait_comp_enable(u8 enable)
{
#if defined(CONFIG_HISI_COUL_HI6421V700) || defined(CONFIG_HISI_COUL_HI6421V600) || \
    defined(CONFIG_HISI_COUL_HI6555V200)
#else
        u8 val;
        val = HI6XXX_REG_READ(HI6XXX_COUL_WAIT_COMP_ADDR);
        val = (val & ~(WAIT_COMP_EN_MASK)) | (enable << WAIT_COMP_EN_SHIFT);
        HI6XXX_REG_WRITE(HI6XXX_COUL_WAIT_COMP_ADDR, val);
#endif
}
/*******************************************************
  Function:        hi6xxx_coul_config_init
  Description:     hi6xxx config init
  Input:           NULL
  Output:          NULL
  Return:          NULL
********************************************************/
static void hi6xxx_coul_chip_init(void)
{
    hi6xxx_coul_clear_irq();
    hi6xxx_coul_disable_irq();

    hi6xxx_coul_set_i_in_event_gate(DEFULT_I_GATE_VALUE);
    hi6xxx_coul_set_i_out_event_gate(DEFULT_I_GATE_VALUE);

    /*unmask coul eco*/
    udelay(110);
    HI6XXX_REG_WRITE(HI6XXX_COUL_ECO_MASK, 0);
    /* config coul Mode */
    udelay(110);
    HI6XXX_REG_WRITE(HI6XXX_COUL_CTRL_REG,DEFAULT_COUL_CTRL_VAL);

#if defined(CONFIG_HISI_COUL_HI6421V600) || defined(CONFIG_HISI_COUL_HI6555V200)
    /* open coul cali auto*/
    udelay(110);
    HI6XXX_REG_WRITE(HI6XXX_CLJ_CTRL,CALI_CLJ_DEFAULT_VALUE);
#endif
    hi6xxx_coul_wait_comp_enable(PMU_ENABLE);
}

/*******************************************************
  Function:        hi6xxx_coul_show_key_reg
  Description:     show key register info for bug
  Input:           NULL
  Output:          NULL
  Return:          NULL
********************************************************/
void hi6xxx_coul_show_key_reg(void)
{
    unsigned char reg0=0;
    unsigned int reg1=0;
    unsigned int reg2=0;
    unsigned int reg3=0;
    unsigned char reg4=0;
    unsigned int reg5=0;

    udelay(50);
    HI6XXX_REGS_READ(HI6XXX_COUL_STATE_REG,&reg0,1);
    HI6XXX_REGS_READ(HI6XXX_VOL_FIFO_BASE,&reg1, REG_NUM);
    HI6XXX_REGS_READ(HI6XXX_VOL_FIFO_BASE+REG_NUM,&reg2,REG_NUM);
    HI6XXX_REGS_READ(HI6XXX_OFFSET_VOLTAGE,&reg3,REG_NUM);
    HI6XXX_REGS_READ(HI6XXX_COUL_CTRL_REG,&reg4,1);
    HI6XXX_REGS_READ(HI6XXX_V_OUT,&reg5,REG_NUM);

    HI6XXX_COUL_INF("\n"
              "0x4033(state)        = 0x%x, 0x4059-0x405b(vol fifo0) = 0x%x,  0x405c-0x405e(vol fifo1) = 0x%x\n"
              "0x403d-0x403f(vol offset) = 0x%x, 0x4003(ctrl)        = 0x%x\n"
              "0x4037-0x4039(cur vol)= 0x%x\n",
               reg0,reg1,reg2,reg3,reg4,reg5);

}

/*******************************************************
  Function:        interrupt_notifier_work
  Description:     interrupt_notifier_work - send a notifier event to battery monitor.
  Input:           struct work_struct *work           ---- work queue
  Output:          NULL
  Return:          NULL
  Remark:          capacity INT : low level and shutdown level.
                   need to check by Test
********************************************************/
 static void hi6xxx_coul_interrupt_notifier_work(struct work_struct *work)
{
    struct hi6xxx_coul_device_info *di = container_of(work,
                struct hi6xxx_coul_device_info,
                irq_work.work);
    unsigned char intstat = 0;

    if( NULL == di || NULL == work)
    {
          HI6XXX_COUL_ERR(" [%s] NULL point in \n", __func__);
         return;
    }
    intstat = di->irq_mask;
    di->irq_mask = 0;

    hi6xxx_coul_show_key_reg();

       if (intstat & COUL_I_OUT_MASK){
           HI6XXX_COUL_INF("IRQ: COUL_I_OUT_INT\n");
           atomic_notifier_call_chain(&coul_fault_notifier_list,COUL_FAULT_I_OUT, NULL);
       }
       if (intstat & COUL_I_IN_MASK){
           HI6XXX_COUL_INF("IRQ: COUL_I_IN_INT\n");
           atomic_notifier_call_chain(&coul_fault_notifier_list,COUL_FAULT_I_IN, NULL);
       }
       if (intstat & COUL_VBAT_INT_MASK){
           HI6XXX_COUL_INF("IRQ: COUL_CCOUT_LOW_VOL_INT\n");
           atomic_notifier_call_chain(&coul_fault_notifier_list,COUL_FAULT_LOW_VOL, NULL);
       }
    if (intstat & COUL_CL_INT_MASK)
    {
          HI6XXX_COUL_INF("IRQ: COUL_CCOUT_BIG_INT\n");
          atomic_notifier_call_chain(&coul_fault_notifier_list,COUL_FAULT_CL_INT, NULL);
    }
    if (intstat & COUL_CL_IN_MASK)
    {
        HI6XXX_COUL_INF("IRQ: COUL_CCIN_CNT_INT\n");
        atomic_notifier_call_chain(&coul_fault_notifier_list,COUL_FAULT_CL_IN, NULL);
    }
    if (intstat & COUL_CL_OUT_MASK)
    {
        HI6XXX_COUL_INF("IRQ: COUL_CCOUT_CNT_INT\n");
        atomic_notifier_call_chain(&coul_fault_notifier_list,COUL_FAULT_CL_OUT, NULL);
    }

}

/*******************************************************
  Function:        hi6xxx_coul_irq_cb
  Description:     hi6xxx coul irq handler
  Input:            int irq                 ---- irq number
                       void *_di            ---- coul device
  Output:          NULL
  Return:          IRQ_NONE: irq not exist,  IRQ_HANDLED: be handled
********************************************************/
static irqreturn_t hi6xxx_coul_irq_cb(int irq,  void *_di)
{
    struct hi6xxx_coul_device_info *di = _di;
    unsigned char val = 0;

    HI6XXX_COUL_INF("coul_irq_cb irq=%d\n", irq);

    val = HI6XXX_REG_READ(HI6XXX_COUL_IRQ_REG);
    HI6XXX_COUL_INF("HI6XXX_IRQ_ADDR reg=%d\n", val);

    HI6XXX_REG_WRITE(HI6XXX_COUL_IRQ_REG,val);

    di->irq_mask |= val;

    queue_delayed_work(system_power_efficient_wq, &di->irq_work, msecs_to_jiffies(0));

    return IRQ_HANDLED;
}
/*******************************************************
  Function:        hi6xxx_coul_calculate_eco_leak_uah
  Description:     calculate capacity leak from existing ECO MODE to calc soc first time
  Input:           NULL
  Output:          NULL
  Return:          leak capacity
  Remark:          ECO uah register keep the same value after exist from ECO
********************************************************/
static int hi6xxx_coul_calculate_eco_leak_uah(void)
{
    int rst_uah          = 0;
    int eco_uah          = 0;
    int cur_uah          = 0;
    int eco_in_uah       = 0;
    int eco_out_uah      = 0;
    int present_in_uah   = 0;
    int present_out_uah  = 0;
    u64 in_val  = 0;
    u64 out_val = 0;

    HI6XXX_REGS_READ(HI6XXX_ECO_OUT_CLIN_REG_BASE, &in_val, CC_REG_NUM);
    HI6XXX_REGS_READ(HI6XXX_ECO_OUT_CLOUT_REG_BASE, &out_val, CC_REG_NUM);
    /*if: first time to calc soc after exiting from ECO Mode */
    if ((last_eco_in != in_val) || (last_eco_out != out_val)) {
        eco_out_uah     = hi6xxx_coul_convert_regval2uah(out_val);
        eco_in_uah      = hi6xxx_coul_convert_regval2uah(in_val);
        eco_uah         = eco_out_uah - eco_in_uah;
        /* current cc  */
        HI6XXX_REGS_READ(HI6XXX_CL_OUT_BASE, &out_val, CC_REG_NUM);
        HI6XXX_REGS_READ(HI6XXX_CL_IN_BASE, &in_val, CC_REG_NUM);
        present_in_uah  = hi6xxx_coul_convert_regval2uah(in_val);
        present_out_uah = hi6xxx_coul_convert_regval2uah(out_val);
        cur_uah         = present_out_uah - present_in_uah;
        /* leak cc from exisingt eco mode to first calc soc */
        rst_uah         = cur_uah - eco_uah;

        HI6XXX_COUL_ERR("eco_in=%d,eco_out=%d,present_in=%d,present_out=%d,leak cc=%d .\n",
                                       eco_in_uah, eco_out_uah,present_in_uah, present_out_uah, rst_uah);
    } else {
        rst_uah = 0;
        HI6XXX_COUL_INF("Not the FIRST time to calc soc after exiting from ECO Model, leak cc=0 .\n");
    }
    return rst_uah;
}

/*******************************************************
  Function:        hi6xxx_coul_clear_fifo
  Description:     clear coul vol/current fifo value
  Input:           NULL
  Output:          NULL
  Return:          NULL
  Remark:          NA
********************************************************/
static void hi6xxx_coul_clear_fifo(void)
{
    unsigned char reg_value = 0;
    reg_value = HI6XXX_REG_READ(HI6XXX_FIFO_CLEAR);
    HI6XXX_REG_WRITE(HI6XXX_FIFO_CLEAR, (reg_value | COUL_FIFO_CLEAR));
}

/*******************************************************
  Function:        hi6xxx_coul_clear_enable_eco_fifo
  Description:     clear coul vol/current eco fifo value
  Input:           NULL
  Output:          NULL
  Return:          NULL
  Remark:          NA
********************************************************/
static void hi6xxx_coul_clear_enable_eco_fifo(void)
{
#if defined(CONFIG_HISI_COUL_HI6421V700) ||\
    defined(CONFIG_HISI_COUL_HI6421V600) ||defined(CONFIG_HISI_COUL_HI6555V200)
#else
    unsigned char reg_value = 0;
    reg_value = HI6XXX_REG_READ(HI6XXX_ECO_FIFO_CLEAR);
    HI6XXX_REG_WRITE(HI6XXX_ECO_FIFO_CLEAR, (reg_value | ECO_FIFO_CLEAR | ECO_FIFO_EN));
#endif
}

/*******************************************************
  Function:        hi6xxx_coul_enter_eco
  Description:     coul enter eco
  Input:           NULL
  Output:          NULL
  Return:          NULL
  Remark:          coul eco follow pmu eco
********************************************************/
static void hi6xxx_coul_enter_eco(void)
{
    unsigned char reg_val;
    u64 eco_in_reg = 0;
    u64 eco_out_reg = 0;

    HI6XXX_REGS_READ(HI6XXX_ECO_OUT_CLIN_REG_BASE, &eco_in_reg, CC_REG_NUM);
    HI6XXX_REGS_READ(HI6XXX_ECO_OUT_CLOUT_REG_BASE, &eco_out_reg, CC_REG_NUM);

    last_eco_in = eco_in_reg;
    last_eco_out = eco_out_reg;

#if defined(CONFIG_HISI_COUL_HI6555V200)
    reg_val = HI6XXX_REG_READ(HI6XXX_COUL_STATE_REG);
    if (COUL_CALI_ING == reg_val) {
	HI6XXX_COUL_INF("cali ing, don't do it again!\n");
	reg_val= ECO_COUL_CTRL_VAL;
    } else {
        HI6XXX_COUL_INF("calibrate!\n");
        reg_val= (ECO_COUL_CTRL_VAL | COUL_CALI_ENABLE);
    }
#else
    reg_val= ECO_COUL_CTRL_VAL;

#endif

    udelay(110);
    HI6XXX_REG_WRITE(HI6XXX_COUL_CTRL_REG,reg_val);
    hi6xxx_coul_clear_fifo();

    hi6xxx_coul_clear_enable_eco_fifo();

    hi6xxx_coul_eco_delay_enable(PMU_ENABLE);
}

/*******************************************************
  Function:        hi6xxx_coul_exit_eco
  Description:     coul exit eco
  Input:           NULL
  Output:          NULL
  Return:          NULL
  Remark:          coul eco follow pmu eco
********************************************************/
static void hi6xxx_coul_exit_eco(void)
{
    hi6xxx_coul_clear_fifo();
    udelay(110);
    HI6XXX_REG_WRITE(HI6XXX_COUL_CTRL_REG,DEFAULT_COUL_CTRL_VAL);

    hi6xxx_coul_eco_delay_enable(PMU_DISABLE);
}

/*******************************************************
  Function:        hi6xxx_coul_set_hltherm_flag
  Description:     set coul hltherm flag for high and low temperature test
  Input:           temp_protect_flag: protect 1, no protect 0
  Output:          NA
  Return:          0:success
                   other :fail
*******************************************************/
int hi6xxx_coul_set_hltherm_flag(int temp_protect_flag)
{
    unsigned char val = 0;
    val = HI6XXX_REG_READ(HI6XXX_COUL_TEMP_PROTECT);
    if (1 == temp_protect_flag){
        HI6XXX_REG_WRITE(HI6XXX_COUL_TEMP_PROTECT, (val | TEMP_PROTECT_BITMASK));
    } else {
        HI6XXX_REG_WRITE(HI6XXX_COUL_TEMP_PROTECT, (val & (~TEMP_PROTECT_BITMASK)));
    }
    return 0;
}
/*******************************************************
  Function:        hi6xxx_coul_get_hltherm_flag
  Description:     get hltherm flag
  Input:           NA
  Output:          NA
  Return:          1: tmep protect  0:no protect
*******************************************************/
int hi6xxx_coul_get_hltherm_flag(void)
{
    unsigned char val = 0;

    val = HI6XXX_REG_READ(HI6XXX_COUL_TEMP_PROTECT);
    if (val & TEMP_PROTECT_BITMASK)
    {
        return 1;
    } else {
        return 0;
    }
}
static void hi6xxx_coul_save_last_soc(short soc)
{
    unsigned short val;

    if(soc < 0)
        soc = 0;
    val = (unsigned short)soc;
    HI6XXX_REG_WRITE(HI6XXX_SAVE_LAST_SOC, SAVE_LAST_SOC_FALG | (val & SAVE_LAST_SOC));
}

static void hi6xxx_coul_get_last_soc(short *soc)
{
    unsigned short val = 0;
    val = HI6XXX_REG_READ(HI6XXX_SAVE_LAST_SOC );
    *soc = (short) (val & SAVE_LAST_SOC);
}

static void hi6xxx_coul_clear_last_soc_flag(void)
{
    HI6XXX_REG_WRITE(HI6XXX_SAVE_LAST_SOC, 0);
    HI6XXX_COUL_ERR("%s clear last soc flag !\n", __FUNCTION__);
}

static void hi6xxx_coul_get_last_soc_flag(bool *valid)
{
    bool val;
    val = SAVE_LAST_SOC_FALG & HI6XXX_REG_READ(HI6XXX_SAVE_LAST_SOC);
    *valid = val;
}

static void hi6xxx_coul_cancle_auto_cali(void)
{
    u8 val=0;
    val = HI6XXX_REG_READ(HI6XXX_CLJ_CTRL);
    val = val & (~MASK_CALI_AUTO_OFF);
    udelay(110);
    HI6XXX_REG_WRITE(HI6XXX_CLJ_CTRL,val);
}

static void hi6xxx_coul_save_ocv_level(u8 level)
{
    u8 val;
    val = HI6XXX_REG_READ(HI6XXX_OCV_LEVEL_ADDR);
	val &= ~SAVE_OCV_LEVEL;
	val |= ((level << OCV_LEVEL_SHIFT) & SAVE_OCV_LEVEL);
    HI6XXX_REG_WRITE(HI6XXX_OCV_LEVEL_ADDR, val);
}

static void hi6xxx_coul_get_ocv_level(u8 *level)
{
    u8 val = 0;
    val = HI6XXX_REG_READ(HI6XXX_OCV_LEVEL_ADDR);
    val &= SAVE_OCV_LEVEL;
    *level = val >> OCV_LEVEL_SHIFT;
}
static int hi6xxx_coul_get_drained_battery_flag(void)
{
    u8 val = 0;
#if defined(CONFIG_HISI_COUL_HI6555V200)
#else
    val = HI6XXX_REG_READ(DRAINED_BATTERY_FLAG_ADDR);
    HI6XXX_COUL_ERR("%s get reg value %d!!!\n", __FUNCTION__,val);
    val &= DRAINED_BATTERY_FLAG_BIT;
#endif
    return val;
}

static void hi6xxx_coul_clear_drained_battery_flag(void)
{
#if defined(CONFIG_HISI_COUL_HI6555V200)
#else
    u8 val = 0;
    val = HI6XXX_REG_READ(DRAINED_BATTERY_FLAG_ADDR);
    HI6XXX_REG_WRITE(DRAINED_BATTERY_FLAG_ADDR,val & (~DRAINED_BATTERY_FLAG_BIT));
    val = HI6XXX_REG_READ(DRAINED_BATTERY_FLAG_ADDR);
    HI6XXX_COUL_ERR("%s after clear reg value %d!!!\n", __FUNCTION__,val);
#endif
}
static void hi6xxx_coul_set_eco_sample(u8 set_val)
{
#if defined(CONFIG_HISI_COUL_HI6421V600) || defined(CONFIG_HISI_COUL_HI6555V200)
#else
    u8 val;
    val = HI6XXX_REG_READ(HI6XXX_ECO_OCV_ADDR);
    if (set_val)
        val |= EN_ECO_SAMPLE;
    else
        val &= (~EN_ECO_SAMPLE);
    HI6XXX_REG_WRITE(HI6XXX_OCV_LEVEL_ADDR, val);
    val = HI6XXX_REG_READ(HI6XXX_ECO_OCV_ADDR);
#endif
}

static void hi6xxx_coul_get_eco_sample(u8 *get_val)
{
#if defined(CONFIG_HISI_COUL_HI6421V600) || defined(CONFIG_HISI_COUL_HI6555V200)
#else
    u8 val = 0;
    val = HI6XXX_REG_READ(HI6XXX_ECO_OCV_ADDR);
    val &= EN_ECO_SAMPLE;
    if (val)
        *get_val = 1;
    else
        *get_val = 0;
#endif
}

static void hi6xxx_coul_clr_eco_sample(u8 set_val)
{
#if defined(CONFIG_HISI_COUL_HI6421V600) || defined(CONFIG_HISI_COUL_HI6555V200)
#else
    u8 val;
    val = HI6XXX_REG_READ(HI6XXX_ECO_OCV_ADDR);
    if (set_val)
        val |= CLR_ECO_SAMPLE;
    else
        val &= (~CLR_ECO_SAMPLE);
    HI6XXX_REG_WRITE(HI6XXX_ECO_OCV_ADDR, val);
    val = HI6XXX_REG_READ(HI6XXX_ECO_OCV_ADDR);
#endif
}
static void hi6xxx_coul_set_bootocv_sample(u8 set_val)
{
#if defined(CONFIG_HISI_COUL_HI6421V600) || defined(CONFIG_HISI_COUL_HI6555V200)
#else
    u8 val;
    val = HI6XXX_REG_READ(BOOT_OCV_ADDR);
    if (set_val)
        val |= EN_BOOT_OCV_SAMPLE;
    else
        val &= (~EN_BOOT_OCV_SAMPLE);
    HI6XXX_REG_WRITE(BOOT_OCV_ADDR, val);
    val = HI6XXX_REG_READ(BOOT_OCV_ADDR);
    HI6XXX_COUL_ERR("%s set_bootocv:%d!!!\n", __FUNCTION__, val);
#endif
}
#if defined(CONFIG_HISI_COUL_HI6421V600) || defined(CONFIG_HISI_COUL_HI6555V200)
#else
static int  hi6xxx_get_coul_calibration_status(void)
{

    u8 val = 0;
    val = HI6XXX_REG_READ(HI6XXX_COUL_STATE_REG);
    val &= COUL_MSTATE_MASK;
    if(COUL_CALI_ING == val)
        return val;
    else
        return 0;
}
#endif

#if defined(CONFIG_HISI_COUL_HI6421V600) || defined(CONFIG_HISI_COUL_HI6555V200)
#else
static int hi6xxx_coul_get_eco_out_chip_temp(void)
{
    int ret = 0;
    unsigned int temp_reg = 0;
    int reg_addr = 0;

    reg_addr = HI6XXX_COUL_ECOOUT_TEMP_DATA;
         /* read temp data*/
    HI6XXX_REGS_READ(reg_addr, &temp_reg, 3);
    ret = hi6xxx_coul_convert_regval2temp(temp_reg);

    HI6XXX_COUL_ERR("%s , temp_reg is 0x%x, temp is %d \n", __func__, temp_reg, ret);
    return ret;
}

static int hi6xxx_coul_get_normal_mode_chip_temp(void)
{
    unsigned char val = 0;
    unsigned int temp_reg = 0;
    int retry;
    int ret = 0;

    /* read mstat is normal */
    retry = 30;  /* From cali mode to normal mode takes up to 2.75s  */
    do{
        val = HI6XXX_REG_READ(HI6XXX_COUL_STATE_REG);

        retry--;
        mdelay(100);

        if(retry == 0) {
            HI6XXX_COUL_ERR("%s  val is 0x%x, wait coul working fail !\n", __func__, val);
            ret = INVALID_TEMP;
            goto reset_auto_cali;
        }
    } while ((val & COUL_MSTATE_MASK) != COUL_WORKING);


    HI6XXX_REG_WRITE(HI6XXX_COUL_TEMP_CTRL, TEMP_EN);

    /* read temp_rdy = 1*/
    retry = 10; /* Collecting temperature takes 200ms  */
    do{
        val = HI6XXX_REG_READ(HI6XXX_COUL_TEMP_CTRL);

        retry--;
        mdelay(25);

        if(retry == 0) {
            HI6XXX_COUL_ERR("%s  val is 0x%x, wait temp ready fail !\n", __func__, val);
            ret = INVALID_TEMP;
            goto reset_vol_sel;
        }
    } while (!(val & TEMP_RDY));

     /* read temp data*/
    HI6XXX_REGS_READ(HI6XXX_COUL_TEMP_DATA, &temp_reg, 3);
    ret = hi6xxx_coul_convert_regval2temp(temp_reg);

reset_vol_sel:
     /* set temp vol channel = 0 */
    HI6XXX_REG_WRITE(HI6XXX_COUL_TEMP_CTRL, ~TEMP_EN);

     /* Collecting voltage takes 750ms  */
    retry = 20;
    do{
        val = HI6XXX_REG_READ(HI6XXX_COUL_TEMP_CTRL);

        retry--;
        mdelay(50);

        if(retry == 0) {
            HI6XXX_COUL_ERR("%s  val is 0x%x, wait coul vout ready fail !\n", __func__, val);
            ret = INVALID_TEMP;
            goto reset_auto_cali;
        }
    } while (!(val & VOUT_RDY));

    /* auto cali on  */
reset_auto_cali:
    //HI6XXX_REG_WRITE(HI6XXX_CLJ_CTRL, ctr_val);

    return ret;

}
/*******************************************************
  Function:        hi6xxx_coul_get_chip_temp
  Description:    Get the coul chip temperature at different times
  Input:            type: start up ocv chip temp; eco out chip temp; normal mode chip temp
  Output:          NA
  Return:           ret: chip temp or INVALID_TEMP
  Remark:         When the type is equal to Normal,
                        you need to add wakelock, to avoid the PMU into ECO mode
*******************************************************/
static int hi6xxx_coul_get_chip_temp(enum CHIP_TEMP_TYPE type)
{
    int ret = 0;

   if (ECO_OUT == type) {
        ret = hi6xxx_coul_get_eco_out_chip_temp();
    }else if (NORMAL == type) {
        ret = hi6xxx_coul_get_normal_mode_chip_temp();
    } else {
        ret = INVALID_TEMP;
        HI6XXX_COUL_ERR("%s  type is %d error!\n", __func__, type);
    }

    return ret;
}
#endif

#if defined(CONFIG_HISI_COUL_HI6421V700) || \
    defined(CONFIG_HISI_COUL_HI6421V600) || defined(CONFIG_HISI_COUL_HI6555V200)
#else
/*******************************************************
  Function:      hi6xxx_coul_get_eco_fifo_depth
  Description:   get coul eco fifo depth
  Input:         NULL
  Output:        NULL
  Return:        depth of eco fifo
********************************************************/
static int hi6xxx_coul_get_eco_fifo_depth(void)
{
    return ECO_FIFO_DEPTH;
}

/*******************************************************
    Function:      hi6xxx_coul_get_eco_vol_uv
    Description:   get eco vol in uv from fifo
    Intput:        fifo_order:fifo serial number 0-4
    Output:        NULL
    Return:        battery voltage in uv
********************************************************/
int hi6xxx_coul_get_eco_vol_uv_from_fifo(unsigned int fifo_order)
{
    unsigned int regval = 0;
    if (fifo_order > ECO_FIFO_DEPTH){
        fifo_order = ECO_FIFO_DEPTH;
    }
    HI6XXX_REGS_READ((HI6XXX_ECO_VOL_FIFO_BASE + 3*fifo_order),&regval,3);/*lint !e647 */
    return (hi6xxx_coul_convert_regval2uv(regval));
}
/*******************************************************
    Function:      hi6xxx_coul_get_eco_current_ua
    Description:   get eco current in ua from eco fifo
    Intput:        fifo_order:fifo serial number 0-4
    Output:        NULL
    Return:        battery voltage in uv
********************************************************/
int hi6xxx_coul_get_eco_cur_ua_from_fifo(unsigned int fifo_order)
{
    unsigned int regval = 0;
    if (fifo_order > ECO_FIFO_DEPTH){
        fifo_order = ECO_FIFO_DEPTH;
    }
    HI6XXX_REGS_READ((HI6XXX_ECO_I_FIFO_BASE + REG_NUM*fifo_order),&regval,REG_NUM);/*lint !e647 */
    return (hi6xxx_coul_convert_regval2ua(regval));
}
/*******************************************************
    Function:      hi6xxx_coul_get_eco_temp
    Description:   get eco current from eco fifo
    Intput:        fifo_order:fifo serial number 0-4
    Output:        NULL
    Return:        battery temp
********************************************************/
int hi6xxx_coul_get_eco_temp_from_fifo(unsigned int fifo_order)
{
    unsigned int regval = 0;
    if (fifo_order > ECO_FIFO_DEPTH){
        fifo_order = ECO_FIFO_DEPTH;
    }
    HI6XXX_REGS_READ((HI6XXX_ECO_TEMP_FIFO_BASE + 3*fifo_order),&regval,3);/*lint !e647 */
    return (hi6xxx_coul_convert_regval2temp(regval));
}

/*******************************************************
  Function:        hi6xxx_coul_get_tbat
  Description:    Get the temp of battery by soh
  Input:          NA
  Output:         NA
  Return:         temp of battery
*******************************************************/
static int hi6xxx_coul_get_tbat(void)
{
        u16 tbat_code = 0;

        /* get adc data */
        HI6XXX_REGS_READ(HI6XXX_SOH_TBAT_DATA_BASE, &tbat_code, 2);

        return (int)tbat_code;
}
#endif

#ifdef CONFIG_SYSFS

static long g_reg_addr = 0;
ssize_t hi6xxx_coul_set_reg_sel(struct device *dev,
                  struct device_attribute *attr,
                  const char *buf, size_t count)
{
    int status = count;
    g_reg_addr = 0;
    if (strict_strtol(buf, 0, &g_reg_addr) < 0)
        return -EINVAL;
    return status;
}

ssize_t hi6xxx_coul_set_reg_value(struct device *dev,
                  struct device_attribute *attr,
                  const char *buf, size_t count)
{
    long val = 0;
    size_t status = count;
    if (strict_strtol(buf, 0, &val) < 0)
        return -EINVAL;
    #ifdef CONFIG_HISI_DEBUG_FS
    HI6XXX_REG_WRITE(g_reg_addr,(char)val);
    #endif
    return status;

}

ssize_t hi6xxx_coul_show_reg_info(struct device *dev,
                  struct device_attribute *attr,
                  char *buf)
{
    u8 val = 0;
    #ifdef CONFIG_HISI_DEBUG_FS
    val = HI6XXX_REG_READ(g_reg_addr);
    #endif
    return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE, "reg[0x%x]=0x%x\n", (u32)g_reg_addr, val);
}

static DEVICE_ATTR(sel_reg, (S_IWUSR | S_IRUGO),
                NULL,
                hi6xxx_coul_set_reg_sel);
static DEVICE_ATTR(set_reg, (S_IWUSR | S_IRUGO),
                hi6xxx_coul_show_reg_info,
                hi6xxx_coul_set_reg_value);

static struct attribute *hi6xxx_coul_attributes[] = {
    &dev_attr_sel_reg.attr,
    &dev_attr_set_reg.attr,
    NULL,
};

static const struct attribute_group hi6xxx_coul_attr_group = {
    .attrs = hi6xxx_coul_attributes,
};
#endif

struct coul_device_ops hi6xxx_coul_ops =
{
    .calculate_cc_uah             = hi6xxx_coul_calculate_cc_uah,
    .save_cc_uah                  = hi6xxx_coul_save_cc_uah,
    .convert_ocv_regval2ua        = hi6xxx_coul_convert_ocv_regval2ua,
    .convert_ocv_regval2uv        = hi6xxx_coul_convert_ocv_regval2uv,
    .is_battery_moved             = hi6xxx_coul_is_battery_moved,
    .set_battery_moved_magic_num  = hi6xxx_coul_set_battery_move_magic,
    .get_fifo_depth               = hi6xxx_coul_get_fifo_depth,
    .get_fifo_avg_data            = hi6xxx_coul_get_fifo_avg_data,
    .get_delta_rc_ignore_flag     = hi6xxx_coul_get_delta_rc_ignore_flag,
    .get_nv_read_flag             = hi6xxx_coul_get_nv_read_flag,
    .set_nv_save_flag             = hi6xxx_coul_set_nv_save_flag,
    .set_hltherm_flag             = hi6xxx_coul_set_hltherm_flag,
    .get_hltherm_flag             = hi6xxx_coul_get_hltherm_flag,
    .get_use_saved_ocv_flag       = hi6xxx_coul_get_use_saved_ocv_flag,
    .get_fcc_invalid_up_flag      = hi6xxx_coul_get_fcc_invalid_up_flag,
    .save_ocv                     = hi6xxx_coul_save_ocv,
    .get_ocv                      = hi6xxx_coul_get_ocv,
    .clear_ocv                    = hi6xxx_coul_clear_ocv,
    .save_ocv_temp                = hi6xxx_coul_save_ocv_temp,
    .get_ocv_temp                 = hi6xxx_coul_get_ocv_temp,
    .clear_ocv_temp               = hi6xxx_coul_clear_ocv_temp,
    .set_low_low_int_val          = hi6xxx_coul_set_low_vol_val,
    .get_abs_cc                   = hi6xxx_coul_get_abs_cc,
    .get_coul_time                = hi6xxx_coul_get_coul_time,
    .clear_coul_time              = hi6xxx_coul_clear_coul_time,
    .clear_cc_register            = hi6xxx_coul_clear_cc_register,
    .cali_adc                     = hi6xxx_coul_cali_adc,
    .get_battery_voltage_uv       = hi6xxx_coul_get_battery_voltage_uv,
    .get_battery_current_ua       = hi6xxx_coul_get_battery_current_ua,
    .get_battery_vol_uv_from_fifo = hi6xxx_coul_get_battery_vol_uv_from_fifo,
    .get_battery_cur_ua_from_fifo = hi6xxx_coul_get_battery_cur_ua_from_fifo,
    .get_offset_current_mod       = hi6xxx_coul_get_offset_current_mod,
    .get_offset_vol_mod           = hi6xxx_coul_get_offset_vol_mod,
    .set_offset_vol_mod           = hi6xxx_coul_set_offset_vol_mod,
    .get_ate_a                    = hi6xxx_coul_get_ate_a,
    .get_ate_b                    = hi6xxx_coul_get_ate_b,
    .irq_enable                   = hi6xxx_coul_enable_irq,
    .irq_disable                  = hi6xxx_coul_disable_irq,
    .show_key_reg                 = hi6xxx_coul_show_key_reg,
    .enter_eco                    = hi6xxx_coul_enter_eco,
    .exit_eco                     = hi6xxx_coul_exit_eco,
    .calculate_eco_leak_uah       = hi6xxx_coul_calculate_eco_leak_uah,
    .get_last_soc                 = hi6xxx_coul_get_last_soc,
    .save_last_soc                = hi6xxx_coul_save_last_soc,
    .get_last_soc_flag            = hi6xxx_coul_get_last_soc_flag,
    .clear_last_soc_flag          = hi6xxx_coul_clear_last_soc_flag,
    .cali_auto_off                = hi6xxx_coul_cancle_auto_cali,
    .save_ocv_level               = hi6xxx_coul_save_ocv_level,
    .get_ocv_level                = hi6xxx_coul_get_ocv_level,
    .get_drained_battery_flag     = hi6xxx_coul_get_drained_battery_flag,
    .clear_drained_battery_flag   = hi6xxx_coul_clear_drained_battery_flag,
    .set_bootocv_sample           = hi6xxx_coul_set_bootocv_sample,
    .set_eco_sample_flag          = hi6xxx_coul_set_eco_sample,
    .get_eco_sample_flag          = hi6xxx_coul_get_eco_sample,
    .clr_eco_data                 = hi6xxx_coul_clr_eco_sample,
#if defined(CONFIG_HISI_COUL_HI6421V600) || defined(CONFIG_HISI_COUL_HI6555V200)
#else
    .set_i_in_event_gate          = hi6xxx_coul_set_i_in_event_gate,
    .set_i_out_event_gate         = hi6xxx_coul_set_i_out_event_gate,
    .convert_regval2uv            = hi6xxx_coul_convert_regval2uv,
    .convert_regval2ua            = hi6xxx_coul_convert_regval2ua,
    .convert_regval2temp          = hi6xxx_coul_convert_regval2temp,
    .convert_uv2regval            = hi6xxx_coul_convert_uv2regval,
    .convert_regval2uah           = hi6xxx_coul_convert_regval2uah,
    .get_coul_calibration_status     = hi6xxx_get_coul_calibration_status,
#endif

#if defined(CONFIG_HISI_COUL_HI6421V600) || defined(CONFIG_HISI_COUL_HI6555V200)
#else
    .get_chip_temp                = hi6xxx_coul_get_chip_temp,
#endif

#if defined(CONFIG_HISI_COUL_HI6421V700) || \
    defined(CONFIG_HISI_COUL_HI6421V600) || defined(CONFIG_HISI_COUL_HI6555V200)
#else
    .get_eco_fifo_depth           = hi6xxx_coul_get_eco_fifo_depth,
    .get_eco_vol_uv_from_fifo     = hi6xxx_coul_get_eco_vol_uv_from_fifo,
    .get_eco_cur_ua_from_fifo     = hi6xxx_coul_get_eco_cur_ua_from_fifo,
    .get_eco_temp_from_fifo       = hi6xxx_coul_get_eco_temp_from_fifo,
    .get_bat_temp                 = hi6xxx_coul_get_tbat,
#endif
};

static int hi6xxx_coul_create_sysfs(struct hi6xxx_coul_device_info *di)
{
    int retval = 0;
    struct class *power_class = NULL;

    retval = sysfs_create_group(&di->dev->kobj, &hi6xxx_coul_attr_group);
    if (retval) {
        HI6XXX_COUL_ERR("%s failed to create sysfs group!!!\n", __FUNCTION__);
        return -1;
    }
    power_class = hw_power_get_class();
    if (power_class)
    {
        if (NULL == coul_dev){
            coul_dev = device_create(power_class, NULL, 0, "%s", "coul");
            if(IS_ERR(coul_dev)){
                coul_dev = NULL;
            }
        }
        if (coul_dev) {
            retval = sysfs_create_link(&coul_dev->kobj, &di->dev->kobj, "hi6xxx_coul");
            if(0 != retval)
                HI6XXX_COUL_ERR("%s failed to create sysfs link!!!\n", __FUNCTION__);
        } else {
            HI6XXX_COUL_ERR("%s failed to create new_dev!!!\n", __FUNCTION__);
        }
    }
    return 0;
}

/*******************************************************
  Function:        hi6xxx_coul_probe
  Description:     hi6xxx probe function
  Input:           struct spmi_device *pdev  ---- platform device
  Output:          NULL
  Return:          NULL
********************************************************/
static int  hi6xxx_coul_probe(struct spmi_device *pdev)
{
    struct coul_device_ops *coul_core_ops = NULL;
    struct hi6xxx_coul_device_info *di = NULL;
    struct device_node* np;
    int retval = 0;

    di = (struct hi6xxx_coul_device_info *)devm_kzalloc(&pdev->dev, sizeof(*di), GFP_KERNEL);
    if (!di) {
        HI6XXX_COUL_ERR("%s failed to alloc di struct\n",__FUNCTION__);
        return -1;
    }
    di->dev =&pdev->dev;
    np = di->dev->of_node;
    if(NULL == np){
        HI6XXX_COUL_ERR("%s np is null!\n",__FUNCTION__);
        return -1;/*lint !e429 */
    }
    di->irq = spmi_get_irq_byname(pdev,NULL,"coulirq");
    if(di->irq < 0) {
        HI6XXX_COUL_ERR("failed to get irq resource!\n");
        return -1;/*lint !e429 */
    }

    spmi_set_devicedata(pdev, di);

    if (hi6xxx_coul_check_version(di)){
        retval = -EINVAL;
        goto hi6xxx_failed_0;
    }

    if (hi6xxx_coul_check_debug()){
        retval = -EINVAL;
        goto hi6xxx_failed_0;
    }

    /*config coul ctrl and irq */
    hi6xxx_coul_chip_init();

    /* Init interrupt notifier work */
    INIT_DELAYED_WORK(&di->irq_work, hi6xxx_coul_interrupt_notifier_work);
    retval = request_irq(di->irq, hi6xxx_coul_irq_cb, IRQF_NO_SUSPEND, "hi6xxx_coul_irq", di);
    if (retval){
        HI6XXX_COUL_ERR("Failed to request coul irq\n");
        goto hi6xxx_failed_1;
    }
    /* set shutdown vol level */
    hi6xxx_coul_set_low_vol_val(DEFAULT_BATTERY_VOL_0_PERCENT);

    //set_low_capacity_int_reg(di);
    hi6xxx_coul_enable_irq();

    coul_core_ops = &hi6xxx_coul_ops;
     retval = coul_core_ops_register(coul_core_ops);
    if (retval) {
        HI6XXX_COUL_ERR("failed to register coul ops\n");
        goto hi6xxx_failed_2;
    }

    retval = of_property_read_u32(of_find_compatible_node(NULL, NULL, "hisi,coul_core"),
            "r_coul_mohm", (u32 *)&r_coul_mohm);
    if (retval) {
        r_coul_mohm = R_COUL_MOHM;
        HI6XXX_COUL_ERR("get r_coul_mohm fail, use default value 10 mohm!\n");
    }

    retval = hi6xxx_coul_create_sysfs(di);
    if(retval) {
        HI6XXX_COUL_ERR("%s failed to create sysfs group!!!\n", __FUNCTION__);
        goto hi6xxx_failed_3;
    }

    g_hi6xxx_dev = di;
    HI6XXX_COUL_INF("hi6xxx coul probe ok, chip %x, version is v%x !\n", di->chip_proj, di->chip_version);

    return 0;/*lint !e429 */

hi6xxx_failed_3:
    sysfs_remove_group(&di->dev->kobj, &hi6xxx_coul_attr_group);
hi6xxx_failed_2:
    hi6xxx_coul_disable_irq();
    free_irq(di->irq, di);
hi6xxx_failed_1:
    cancel_delayed_work(&di->irq_work);
hi6xxx_failed_0:
    spmi_set_devicedata(pdev,NULL);
    HI6XXX_COUL_ERR("hi6xxx coul probe failed!\n");
    return retval;/*lint !e429 */
}

/*******************************************************
  Function:        hi6xxx_coul_remove
  Description:     remove function
  Input:           struct spmi_device *pdev        ---- platform device
  Output:          NULL
  Return:          NULL
********************************************************/
static int  hi6xxx_coul_remove(struct spmi_device *pdev)
{
    struct hi6xxx_coul_device_info *di = spmi_get_devicedata(pdev);

    if(NULL == di)
    {
        HI6XXX_COUL_ERR("[%s]di is null\n",__FUNCTION__);
        return -ENODEV;
    }
    //cancel_delayed_work(&di->irq_work);
    kfree(di);

    return 0;
}


static struct of_device_id hi6xxx_coul_match_table[] =
{
    {
          .compatible = "hisilicon_hi6xxx_coul",
    },
    {},
};



static const struct spmi_device_id hi6xxx_coul_spmi_id[] = {
    {"hisilicon_hi6xxx_coul", 0},
    {}
};

static struct spmi_driver hi6xxx_coul_driver = {
    .probe        = hi6xxx_coul_probe,
    .remove        = hi6xxx_coul_remove,
    .driver        = {
       .name           = "hi6xxx_coul",
       .owner          = THIS_MODULE,
       .of_match_table = hi6xxx_coul_match_table,
    },
    .id_table = hi6xxx_coul_spmi_id,
};

int __init hi6xxx_coul_init(void)
{
    return spmi_driver_register(&hi6xxx_coul_driver);
}

void __exit hi6xxx_coul_exit(void)
{
    spmi_driver_unregister(&hi6xxx_coul_driver);
}

fs_initcall(hi6xxx_coul_init);
module_exit(hi6xxx_coul_exit);


MODULE_AUTHOR("HISILICON");
MODULE_DESCRIPTION("hisi hi6xxx coul driver");
MODULE_LICENSE("GPL");
