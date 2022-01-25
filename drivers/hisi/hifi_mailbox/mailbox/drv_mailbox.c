

/*****************************************************************************
  1 ͷ�ļ�����
*****************************************************************************/
#include "drv_mailbox_cfg.h"



/*****************************************************************************
  2 ��������
*****************************************************************************/
unsigned int mailbox_send_msg(
                unsigned int            mailcode,
                const void              *data,
                unsigned int            length);

unsigned int mailbox_reg_msg_cb(
                unsigned int             mailcode,
                mb_msg_cb                 func,
                void                     *data);

unsigned int mailbox_read_msg_data(
                void                   *mail_handle,
                 char                  *buff,
                unsigned int          *size);
/*****************************************************************************
  3 ����ʵ��
*****************************************************************************/


unsigned int DRV_MAILBOX_SENDMAIL(
                unsigned int           MailCode,
                const void             *pData,
                unsigned int           Length)
{
	return mailbox_send_msg(MailCode, pData, Length);
}


unsigned int DRV_MAILBOX_REGISTERRECVFUNC(
                unsigned int           MailCode,
                mb_msg_cb               pFun,
                void                   *UserHandle)
{
	return mailbox_reg_msg_cb(MailCode, pFun, UserHandle);
}


unsigned int DRV_MAILBOX_READMAILDATA(
                void                   *MailHandle,
                unsigned char          *pData,
                unsigned int          *pSize)
{
	return mailbox_read_msg_data(MailHandle, (char *)pData, pSize);
}


