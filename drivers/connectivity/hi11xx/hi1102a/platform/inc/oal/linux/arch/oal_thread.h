

#ifndef __OAL_LINUX_THREAD_H__
#define __OAL_LINUX_THREAD_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*****************************************************************************
  1 ����ͷ�ļ�����
*****************************************************************************/
#include <asm/atomic.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/wait.h>
#include <linux/jiffies.h>
#include <linux/time.h>
#include <asm/param.h>
#include <linux/timer.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include "oal_schedule.h"

/*****************************************************************************
  2 STRUCT����
*****************************************************************************/
typedef struct task_struct          oal_task_stru;

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
  7 �궨��
*****************************************************************************/
#define oal_kthread_create        kthread_create
#define oal_kthread_bind          kthread_bind
#define oal_kthread_stop          kthread_stop
#define oal_kthread_run           kthread_run
#define oal_kthread_should_stop   kthread_should_stop
#define oal_schedule              schedule
#define oal_wake_up_process       wake_up_process
#define oal_set_current_state     set_current_state
#define oal_cond_resched          cond_resched

#if ((_PRE_CONFIG_TARGET_PRODUCT == _PRE_TARGET_PRODUCT_TYPE_5610EVB)\
    ||(_PRE_CONFIG_TARGET_PRODUCT == _PRE_TARGET_PRODUCT_TYPE_5610DMB)\
    ||(_PRE_CONFIG_TARGET_PRODUCT == _PRE_TARGET_PRODUCT_TYPE_VSPM310DMB)\
    ||(_PRE_CONFIG_TARGET_PRODUCT == _PRE_TARGET_PRODUCT_TYPE_WS835DMB))
#define oal_sched_setscheduler sched_setscheduler_export
#elif(_PRE_CONFIG_TARGET_PRODUCT == _PRE_TARGET_PRODUCT_TYPE_1102COMMON)
#define oal_sched_setscheduler sched_setscheduler
#endif
#define oal_set_user_nice          set_user_nice

/*****************************************************************************
  8 UNION����
*****************************************************************************/


/*****************************************************************************
  9 OTHERS����
*****************************************************************************/


/*****************************************************************************
  10 ��������
*****************************************************************************/

OAL_STATIC OAL_INLINE oal_void oal_mutex_init(oal_mutex_stru *pst_mutex)
{
    mutex_init(pst_mutex);
}



OAL_STATIC OAL_INLINE oal_void oal_mutex_lock(oal_mutex_stru *pst_mutex)
{
    mutex_lock(pst_mutex);
}



OAL_STATIC OAL_INLINE oal_void oal_mutex_unlock(oal_mutex_stru *pst_mutex)
{
    mutex_unlock(pst_mutex);
}

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif
