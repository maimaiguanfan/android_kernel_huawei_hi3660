

#ifndef __OAL_WINDOWS_UTIL_H__
#define __OAL_WINDOWS_UTIL_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*****************************************************************************
  1 ����ͷ�ļ�����
*****************************************************************************/
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include "hsimu_reg.h"
#include <sys/stat.h>

/*****************************************************************************
  2 �궨��
*****************************************************************************/
/* 32�ֽ����С��ת�� */
#define OAL_SWAP_BYTEORDER_32(_val)        \
        ((((_val) & 0x000000FF) << 24) +     \
        (((_val) & 0x0000FF00) << 8) +       \
        (((_val) & 0x00FF0000) >> 8) +       \
        (((_val) & 0xFF000000) >> 24))

#define OAL_CONST                                   const
/* ��ȡCORE ID */
#define OAL_GET_CORE_ID()     (g_cpuid_register[0])

typedef LONG                    oal_bitops;

typedef FILE                    oal_file_stru;

typedef struct _stat            oal_file_stat_stru;

#define OAL_FILE_FAIL           OAL_PTR_NULL

#define OAL_LIKELY(_expr)       (_expr)
#define OAL_UNLIKELY(_expr)     (_expr)

#if defined(_PRE_PC_LINT)
#define OAL_FUNC_NAME           "pc_lint"
#else
#define OAL_FUNC_NAME           __FUNCTION__
#endif
#define OAL_RET_ADDR             OAL_PTR_NULL


/* �������ַ�������ָ����ʽ�ϳ�һ���ַ��� */
#define OAL_SPRINTF             sprintf_s

/* �ڴ������ */
#define OAL_RMB()

/* �ڴ�д���� */
#define OAL_WMB()

/* �ڴ����� */
#define OAL_MB()

#define OAL_OFFSET_OF(TYPE,MEMBER)  ((unsigned long) &((TYPE *)0)->MEMBER)

#define __OAL_DECLARE_PACKED

#define KERN_EMERG
#define KERN_ALERT
#define KERN_CRIT
#define KERN_ERR
#define KERN_WARNING
#define KERN_NOTICE
#define KERN_INFO
#define KERN_DEBUG

#define KERN_DEFAULT
#define KERN_CONT

#define OAL_IO_PRINT            printf

#define OAL_BUG_ON(_con)

#define OAL_WARN_ON(condition)  (condition)

#define OAL_VSPRINTF            vsprintf_s

/* �����ַת�����ַ */
#define OAL_VIRT_TO_PHY_ADDR(_virt_addr)            ((oal_uint32)(_virt_addr))
#define OAL_DSCR_VIRT_TO_PHY(_virt_addr)            ((oal_uint32)(_virt_addr))

/* �����ַת�����ַ */
#define OAL_PHY_TO_VIRT_ADDR(_phy_addr)             ((oal_uint32 *)(_phy_addr))
#define OAL_DSCR_PHY_TO_VIRT(_phy_addr)             ((oal_uint32 *)(_phy_addr))

#define OAL_CFG_FILE_PATH       ("C:\\1151_cfg.ini")

#ifndef current
#define current (0)
#endif

#define OAL_STRLEN                                  strlen
#define OAL_MEMCMP                                  memcmp
#define OAL_STRSTR                                  strstr
#define OAL_STRCMP                                  strcmp
#define OAL_STRNCMP                                 strncmp

/*���size1������size2, ����size1/2����ʹ��sizeof(a)*/
#define     SIZE_OF_SIZE1_NOT_LARGER_THAN_SIZE2_BY_NAME(name ,size1, size2)
/*���ṹ��Ĵ�С�Ƿ񲻴����ض�ֵ*/
#define    SIZE_OF_TYPE_NOT_LARGER_THAN_DETECT(type, size)

/*****************************************************************************
  3 ö�ٶ���
*****************************************************************************/

/*****************************************************************************
  4 ȫ�ֱ�������
*****************************************************************************/


/*****************************************************************************
  5 ��Ϣͷ����
*****************************************************************************/


/*****************************************************************************
  6 ��Ϣ����
*****************************************************************************/


/*****************************************************************************
  7 STRUCT����
*****************************************************************************/
typedef struct object{
    char* name;
}oal_kobject;

/*****************************************************************************
  8 UNION����
*****************************************************************************/


/*****************************************************************************
  9 OTHERS����
*****************************************************************************/
#define OAL_ROUND_DOWN(value, boundary)  ((value) & (~((boundary)-1)))
#define OAL_ROUND_UP(value, boundary)    ((((value)-1) | ((boundary)-1))+1)

/*us time cost sub*/
#define declare_time_cost_stru(name)

#define time_cost_var_start(name)
#define time_cost_var_end(name)
#define time_cost_var_sub(name) 0

#define oal_get_time_cost_start(name)
#define oal_get_time_cost_end(name)
#define oal_calc_time_cost_sub(name)

/*****************************************************************************
  10 ��������
*****************************************************************************/


OAL_INLINE oal_bitops  oal_bit_atomic_test_and_set(oal_bitops nr, OAL_VOLATILE oal_bitops *p_addr)
{
    return InterlockedBitTestAndSet(p_addr, nr);
}


OAL_INLINE oal_void  oal_bit_atomic_clear(oal_bitops nr, OAL_VOLATILE oal_bitops *p_addr)
{
    InterlockedBitTestAndReset(p_addr, nr);
}


OAL_INLINE oal_uint16  oal_byteorder_host_to_net_uint16(oal_uint16 us_byte)
{
    us_byte = ((us_byte & 0x00FF) << 8) + ((us_byte & 0xFF00) >> 8);

    return us_byte;
}


OAL_INLINE oal_uint16  oal_byteorder_net_to_host_uint16(oal_uint16 us_byte)
{
    us_byte = ((us_byte & 0x00FF) << 8) + ((us_byte & 0xFF00) >> 8);

    return us_byte;
}


OAL_STATIC OAL_INLINE oal_uint32  oal_byteorder_host_to_net_uint32(oal_uint32 ul_byte)
{
    ul_byte = OAL_SWAP_BYTEORDER_32(ul_byte);

    return ul_byte;
}


OAL_STATIC OAL_INLINE oal_uint32  oal_byteorder_net_to_host_uint32(oal_uint32 ul_byte)
{
    ul_byte = OAL_SWAP_BYTEORDER_32(ul_byte);

    return ul_byte;
}


OAL_INLINE oal_file_stru* oal_file_open_rw(const oal_int8 *pc_path)
{
    oal_file_stru *rlt;

    fopen_s(&rlt, pc_path, "w+");

    if (OAL_PTR_NULL == rlt)
    {
        return OAL_FILE_FAIL;
    }

    return rlt;
}


OAL_INLINE oal_file_stru* oal_file_open_append(const oal_int8 *pc_path)
{
    oal_file_stru *rlt;

    fopen_s(&rlt, pc_path, "a");

    if (OAL_PTR_NULL == rlt)
    {
        return OAL_FILE_FAIL;
    }

    return rlt;
}


OAL_INLINE oal_file_stru* oal_file_open_readonly(const oal_int8 *pc_path)
{
    oal_file_stru* rlt;

    fopen_s(&rlt, pc_path, "r");

    if (OAL_PTR_NULL == rlt)
    {
        return OAL_FILE_FAIL;
    }

    return rlt;
}


OAL_INLINE oal_file_stru* oal_file_write(oal_file_stru* file, oal_int8 *pc_string, oal_uint32 ul_length)
{
    oal_int32 l_rlt;

    l_rlt = fprintf(file, "%s", pc_string);

    if (l_rlt < 0)
    {
        return OAL_FILE_FAIL;
    }

    return file;
}


OAL_INLINE oal_int32 oal_file_close(oal_file_stru* file)
{
    oal_int32 l_rlt;

    l_rlt = fclose(file);

    if (EOF == l_rlt)
    {
        return -1;
    }

    return 0;
}


OAL_STATIC OAL_INLINE oal_int32  oal_file_read(oal_file_stru *file,
		                                         oal_int8 *puc_buf,
		                                         oal_uint32 ul_count)
{
    return (oal_int32)fread(puc_buf, 1, ul_count, file);
}



OAL_INLINE oal_int32  oal_file_size(oal_uint32 *pul_file_size)
{
    oal_file_stat_stru       st_file_stat;
    oal_int32                l_ret;

    l_ret = _stat(OAL_CFG_FILE_PATH, &st_file_stat);
    if (OAL_SUCC != l_ret)
    {
        return l_ret;
    }

    *pul_file_size = (oal_uint32)st_file_stat.st_size;

    return OAL_SUCC;
}


OAL_INLINE oal_int32  oal_atoi(const oal_int8 *c_string)
{
    return atoi(c_string);
}


OAL_INLINE oal_void  oal_itoa(oal_int32 l_val, oal_int8 *c_string, oal_uint8 uc_strlen)
{
    _itoa_s(l_val, c_string, uc_strlen, 10);   /* ���ַ���ת��10�������� */
}


OAL_STATIC OAL_INLINE oal_int8 *oal_strtok(oal_int8 *pc_token, OAL_CONST oal_int8 *pc_delemit, oal_int8 **ppc_context)
{
    return strtok_s(pc_token, pc_delemit, ppc_context);
}


OAL_STATIC OAL_INLINE oal_int  oal_strtol(OAL_CONST oal_int8 *pc_nptr, oal_int8 **ppc_endptr, oal_int32 l_base)
{
    return strtol(pc_nptr, ppc_endptr, l_base);
}


OAL_STATIC OAL_INLINE oal_void  oal_udelay(oal_uint u_loops)
{
}


OAL_STATIC OAL_INLINE oal_void  oal_mdelay(oal_uint u_loops)
{
}


OAL_STATIC OAL_INLINE oal_uint32  oal_kallsyms_lookup_name(OAL_CONST oal_uint8 *uc_var_name)
{
    return 0;
}


OAL_STATIC OAL_INLINE oal_void oal_dump_stack(oal_void)
{
    /* win32��, do nothing */
}


OAL_STATIC OAL_INLINE oal_void  oal_msleep(oal_uint32 ul_usecs)
{

}


OAL_STATIC OAL_INLINE oal_void  oal_usleep_range(oal_ulong min_us, oal_ulong max_us)
{
}


OAL_STATIC OAL_INLINE errno_t oal_strncpy(oal_int8 *pc_dest, OAL_CONST oal_int8 *pc_src, oal_uint32 ul_len)
{
    return strcpy_s((oal_int8 *)pc_dest, ul_len, pc_src);
}

OAL_STATIC OAL_INLINE oal_void oal_random_ether_addr(oal_uint8 *addr)
{
   addr[0] = 0x22;
   addr[1] = 0x22;
   addr[2] = 0x21;
   addr[3] = 0x22;
   addr[4] = 0x23;
   addr[5] = 0x22;
}

OAL_STATIC OAL_INLINE oal_void oal_print_hex_dump(oal_uint8*addr, oal_int32 len, oal_int32 groupsize,oal_int8* pre_str)
{
    OAL_REFERENCE(groupsize);
    OAL_REFERENCE(addr);
    OAL_REFERENCE(len);
    OAL_REFERENCE(pre_str);
}


OAL_STATIC OAL_INLINE oal_uint64 oal_div_u64(oal_uint64 dividend, oal_uint32 divisor)
{
    return (dividend/divisor);
}


OAL_STATIC OAL_INLINE oal_int64 oal_div_s64(oal_int64 dividend, oal_int32 divisor)
{
    return (dividend/divisor);
}


#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of oal_util.h */
