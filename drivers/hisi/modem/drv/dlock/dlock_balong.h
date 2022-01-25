
#ifndef __DLOCK_BALONG_H__
#define __DLOCK_BALONG_H__

#ifdef __cplusplus
    extern "C" {
#endif

/*****************************************************************************
  1 ����ͷ�ļ�����
*****************************************************************************/
#include "mdrv_errno.h"
#include "bsp_om.h"
#include "bsp_om_enum.h"
#include "osl_types.h"
#include "osl_malloc.h"
#include <linux/interrupt.h>

/*****************************************************************************
  2 �궨��
*****************************************************************************/
#define dlock_ok (0)
#define dlock_error (-1)
#define dlock_print(fmt, ...)   (bsp_trace(BSP_LOG_LEVEL_FATAL, BSP_MODU_DLOCK, "[dlock]: <%s> "fmt, __FUNCTION__, ##__VA_ARGS__))
#define dlock_malloc(size)      osl_malloc(size)
#define dlock_free(ptr)         osl_free(ptr)
#define SYSCTRL_ADDR_SIZE      (0x1000)

/*****************************************************************************
  3 ö�ٶ���
*****************************************************************************/
enum
{
    DLOCK_DISABLE = 0,
    DLOCK_ENABLE = 1
};

/*****************************************************************************
  3 STRUCT����
*****************************************************************************/
struct mst_port_info
{
    u32 mst_port;                           /*mst��С�˿ں�ֵ*/
    char mst_name[32];                      /*��mst��С�˿ںŶ�Ӧ��mst_name*/
};

struct mst_id_info
{
    u32 id;                                 /*mst_idֵ*/
    char id_mst_name[32];                   /*��mst_id��Ӧ��mst_name*/
};

struct bus_reset_info
{
    u32 pd_reset_reg[3];                    /*pd����λ�Ĵ���ƫ�Ƶ�ַ����ʼλ����ֹλ*/
    u32 mdm_reset_reg[3];                   /*mdm����λ�Ĵ���ƫ�Ƶ�ַ����ʼλ����ֹλ*/
    u32 pcie_reset_reg[3];                  /*pcie����λ�Ĵ���ƫ�Ƶ�ַ����ʼλ����ֹλ*/
    u32 cnt_div_num_reg[3];                 /*����������Ƶϵ���Ĵ���ƫ�Ƶ�ַ����ʼλ����ֹλ*/
    void* pd_vir_addr;
    void* mdm_vir_addr;
    void* pcie_vir_addr;
    char version[32];
};

struct bus_dlock_info
{
    u32 slv_port_reg[3];                    /*����slave�˿ں�ƫ�Ƶ�ַ����ʼλ����ֹλ*/
    u32 wr_reg[3];                          /*������дָʾ�Ĵ���ƫ�Ƶ�ַ����ʼλ����ֹλ*/
    u32 addr_reg[3];                        /*������ַ�Ĵ���ƫ�Ƶ�ַ����ʼλ����ֹλ*/
    u32 mst_port_reg[3];                    /*����master��С�˿ں�ƫ�Ƶ�ַ����ʼλ����ֹλ*/
    u32 mst_id_reg[3];                      /*����mst_idƫ�Ƶ�ַ����ʼλ����ֹλ*/
    u32 mst_port_num;                       /*��Ҫ��¼mst_name�Ķ˿ڸ���*/
    u32 mst_id_num;                         /*��Ҫ��¼mst_name��mst_id����*/
    u32 mst_port_id;                        /*��Ҫ�鿴mst_id��mst�˿ں�*/
    struct mst_port_info *mst_port_info;    /*��Ҫ��¼mst_name��mst�˿���Ϣ*/
    struct mst_id_info *mst_id_info;        /*��Ҫ��¼mst_name��mst_id��Ϣ*/
};

struct bus_state
{
    void* bus_vir_addr;                     /*�����������ַ*/
    char bus_name[32];                      /*��������*/
    u32 bus_state_reg[3];                   /*����״̬�Ĵ���ƫ�Ƶ�ַ����ʼλ����ֹλ*/
    u32 clear_irq_reg[3];
};

struct bus_state_info
{
    struct bus_state bus_state;             /*����״̬�Ĵ�����Ϣ*/
    struct bus_dlock_info bus_dlock_info;   /*��������dlock�Ĵ�����Ϣ*/
};

struct bus_info
{
    u32 bus_num;                            /*��Ҫ��¼���������߸���*/
    struct bus_state_info *bus_state_info;  /*���������Ĵ�����Ϣ*/
};

#ifdef __cplusplus
}
#endif

#endif



