/*
* Copyright (C) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
* foss@huawei.com
*
* If distributed as part of the Linux kernel, the following license terms
* apply:
*
* * This program is free software; you can redistribute it and/or modify
* * it under the terms of the GNU General Public License version 2 and 
* * only version 2 as published by the Free Software Foundation.
* *
* * This program is distributed in the hope that it will be useful,
* * but WITHOUT ANY WARRANTY; without even the implied warranty of
* * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* * GNU General Public License for more details.
* *
* * You should have received a copy of the GNU General Public License
* * along with this program; if not, write to the Free Software
* * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
*
* Otherwise, the following license terms apply:
*
* * Redistribution and use in source and binary forms, with or without
* * modification, are permitted provided that the following conditions
* * are met:
* * 1) Redistributions of source code must retain the above copyright
* *    notice, this list of conditions and the following disclaimer.
* * 2) Redistributions in binary form must reproduce the above copyright
* *    notice, this list of conditions and the following disclaimer in the
* *    documentation and/or other materials provided with the distribution.
* * 3) Neither the name of Huawei nor the names of its contributors may 
* *    be used to endorse or promote products derived from this software 
* *    without specific prior written permission.
* 
* * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*
*/

/*****************************************************************************
   1 ͷ�ļ�����
*****************************************************************************/
#include "ATCmdProc.h"
#include "AtCheckFunc.h"
#include "AtParseCmd.h"
#include "AtCheckFunc.h"




/*****************************************************************************
    Э��ջ��ӡ��㷽ʽ�µ�.C�ļ��궨��
*****************************************************************************/
#define    THIS_FILE_ID        PS_FILE_ID_AT_PARSEEXTENDCMD_C

/*****************************************************************************
   2 ȫ�ֱ�������
*****************************************************************************/
/* AT��չ�����ʼ״̬״̬�� */
AT_SUB_STATE_STRU AT_E_CMD_NONE_STATE_TAB[]=
{
    /* �����ǰ״̬��ATCMD_T_STATE��atCheckCharPlus�ɹ��������AT_E_CMD_NAME_STATE */
    {    At_CheckCharPlus    ,    AT_E_CMD_NAME_STATE    },

    /* ��״̬����� */
    {    NULL    ,    AT_BUTT_STATE    },
};

/* AT��չ������״̬�� */
AT_SUB_STATE_STRU AT_E_CMD_NAME_STATE_TAB[]=
{
    /* �����ǰ״̬��AT_E_CMD_NAME_STATE��atCheck_char�ɹ��������AT_E_CMD_NAME_STATE */
    {    At_CheckChar    ,    AT_E_CMD_NAME_STATE    },

    /* �����ǰ״̬��AT_E_CMD_NAME_STATE��atCheck_equ�ɹ��������AT_E_CMD_SET_STATE */
    {    At_CheckEqu    ,    AT_E_CMD_SET_STATE    },

    /* �����ǰ״̬��AT_E_CMD_NAME_STATE��atCheck_req�ɹ��������AT_E_CMD_READ_STATE */
    {    At_CheckReq    ,    AT_E_CMD_READ_STATE    },

    /* ��״̬����� */
    {    NULL    ,    AT_BUTT_STATE    },
};

/* AT��չ����Ⱥ�״̬�� */
AT_SUB_STATE_STRU AT_E_CMD_SET_STATE_TAB[]=
{
    /* �����ǰ״̬��AT_E_CMD_SET_STATE��atCheck_req�ɹ��������AT_E_CMD_TEST_STATE */
    {    At_CheckReq    ,    AT_E_CMD_TEST_STATE    },

    /* �����ǰ״̬��AT_E_CMD_SET_STATE��atCheck_char�ɹ��������AT_E_CMD_PARA_STATE */
    {    At_CheckChar    ,    AT_E_CMD_PARA_STATE    },

    /* �����ǰ״̬��AT_E_CMD_SET_STATE��atCheck_colon�ɹ��������AT_E_CMD_COLON_STATE */
    {    atCheckComma    ,    AT_E_CMD_COLON_STATE    },

    /* �����ǰ״̬��AT_E_CMD_SET_STATE��atCheck_quot�ɹ��������AT_E_CMD_LEFT_QUOT_STATE */
    {    At_CheckQuot    ,    AT_E_CMD_LEFT_QUOT_STATE    },

    /* ��״̬����� */
    {    NULL    ,    AT_BUTT_STATE    },
};

/* AT��չ�������״̬�� */
AT_SUB_STATE_STRU AT_E_CMD_PARA_STATE_TAB[]=
{
    /* �����ǰ״̬��AT_E_CMD_PARA_STATE��atCheck_char�ɹ��������AT_E_CMD_PARA_STATE */
    {    At_CheckChar    ,    AT_E_CMD_PARA_STATE    },

    /* �����ǰ״̬��AT_E_CMD_PARA_STATE��atCheck_char�ɹ��������AT_E_CMD_PARA_STATE */
    {    atCheckblank    ,   AT_E_CMD_PARA_STATE    },

    /* �����ǰ״̬��AT_E_CMD_PARA_STATE��atCheckColon�ɹ��������AT_E_CMD_PARA_STATE */
    {    atCheckColon    ,   AT_E_CMD_PARA_STATE    },

    /* �����ǰ״̬��AT_E_CMD_PARA_STATE��atCheckComma�ɹ��������AT_E_CMD_COLON_STATE */
    {    atCheckComma    ,   AT_E_CMD_COLON_STATE    },

    /* ��״̬����� */
    {    NULL    ,    AT_BUTT_STATE    },
};

/* AT��չ�����״̬�� */
AT_SUB_STATE_STRU AT_E_CMD_COLON_STATE_TAB[]=
{
    /* �����ǰ״̬��AT_E_CMD_COLON_STATE��atCheck_char�ɹ��������AT_E_CMD_PARA_STATE */
    {    At_CheckChar    ,    AT_E_CMD_PARA_STATE    },

    /* �����ǰ״̬��AT_E_CMD_COLON_STATE��atCheck_colon�ɹ��������AT_E_CMD_COLON_STATE */
    {    atCheckComma    ,    AT_E_CMD_COLON_STATE    },

    /* �����ǰ״̬��AT_E_CMD_COLON_STATE��atCheck_quot�ɹ��������AT_E_CMD_LEFT_QUOT_STATE */
    {    At_CheckQuot    ,    AT_E_CMD_LEFT_QUOT_STATE    },

    /* ��״̬����� */
    {    NULL    ,    AT_BUTT_STATE    },
};

/* AT��չ����������״̬�� */
AT_SUB_STATE_STRU AT_E_CMD_LEFT_QUOT_STATE_TAB[]=
{
    /* �����ǰ״̬��AT_E_CMD_LEFT_QUOT_STATE��atCheck_char�ɹ��������AT_E_CMD_LEFT_QUOT_STATE */
    {    At_CheckNoQuot    ,    AT_E_CMD_LEFT_QUOT_STATE    },

    /* �����ǰ״̬��AT_E_CMD_LEFT_QUOT_STATE��atCheck_quot�ɹ��������AT_E_CMD_RIGHT_QUOT_STATE */
    {    At_CheckQuot    ,    AT_E_CMD_RIGHT_QUOT_STATE    },

    /* ��״̬����� */
    {    NULL    ,    AT_BUTT_STATE    },
};

/* AT��չ����������״̬�� */
AT_SUB_STATE_STRU AT_E_CMD_RIGHT_QUOT_STATE_TAB[]=
{
    /* �����ǰ״̬��AT_E_CMD_RIGHT_QUOT_STATE��atCheck_colon�ɹ��������AT_E_CMD_COLON_STATE */
    {    atCheckComma    ,    AT_E_CMD_COLON_STATE    },

    /* �����ǰ״̬��AT_E_CMD_RIGHT_QUOT_STATE��atCheck_char�ɹ��������AT_E_CMD_RIGHT_QUOT_STATE */
    {    At_CheckChar     ,    AT_E_CMD_RIGHT_QUOT_STATE    },

    /* �����ǰ״̬��AT_E_CMD_RIGHT_QUOT_STATE��atCheck_quot�ɹ��������AT_E_CMD_RIGHT_QUOT_STATE */
    {    At_CheckQuot     ,    AT_E_CMD_RIGHT_QUOT_STATE    },

    /* ��״̬����� */
    {    NULL    ,    AT_BUTT_STATE    },
};

/* AT������״̬�� */
AT_MAIN_STATE_STRU AT_E_CMD_MAIN_STATE_TAB[] =
{
    /* �����ǰ״̬��AT_NONE_STATE�������AT_E_CMD_NONE_STATE_TAB��״̬�� */
    {    AT_NONE_STATE    ,    AT_E_CMD_NONE_STATE_TAB    },

    /* �����ǰ״̬��AT_E_CMD_NAME_STATE�������AT_E_CMD_NAME_STATE_TAB��״̬�� */
    {    AT_E_CMD_NAME_STATE    ,    AT_E_CMD_NAME_STATE_TAB    },

    /* �����ǰ״̬��AT_E_CMD_SET_STATE�������AT_E_CMD_SET_STATE_TAB��״̬�� */
    {    AT_E_CMD_SET_STATE    ,    AT_E_CMD_SET_STATE_TAB    },

    /* �����ǰ״̬��AT_E_CMD_PARA_STATE�������AT_E_CMD_PARA_STATE_TAB��״̬�� */
    {    AT_E_CMD_PARA_STATE    ,    AT_E_CMD_PARA_STATE_TAB    },

    /* �����ǰ״̬��AT_E_CMD_COLON_STATE�������AT_E_CMD_COLON_STATE_TAB��״̬�� */
    {    AT_E_CMD_COLON_STATE    ,    AT_E_CMD_COLON_STATE_TAB    },

    /* �����ǰ״̬��AT_E_CMD_LEFT_QUOT_STATE�������AT_E_CMD_LEFT_QUOT_STATE_TAB��״̬�� */
    {    AT_E_CMD_LEFT_QUOT_STATE    ,    AT_E_CMD_LEFT_QUOT_STATE_TAB    },

    /* �����ǰ״̬��AT_E_CMD_RIGHT_QUOT_STATE�������AT_E_CMD_RIGHT_QUOT_STATE_TAB��״̬�� */
    {    AT_E_CMD_RIGHT_QUOT_STATE    ,    AT_E_CMD_RIGHT_QUOT_STATE_TAB    },

    /* ��״̬����� */
    {    AT_BUTT_STATE    ,    NULL    },
};



VOS_UINT32 atParseExtendCmd( VOS_UINT8 * pData, VOS_UINT16 usLen)
{
    AT_STATE_TYPE_ENUM curr_state = AT_NONE_STATE;  /*  ���ó�ʼ״̬ */
    AT_STATE_TYPE_ENUM new_state = AT_NONE_STATE;   /*  ���ó�ʼ״̬ */
    VOS_UINT8 *pucCurrPtr = pData;                  /*  ָ��ǰ���ڴ�����ַ�*/
    VOS_UINT8 *pucCopyPtr = pData;                  /*  �������ݵ���ʼָ�� */
    VOS_UINT16 usLength = 0;                        /*  ��¼��ǰ�Ѿ�������ַ�����*/

    /* ���η����ַ����е�ÿ���ַ�*/
    while(usLength++ < usLen)                  /* ���αȽ�ÿ���ַ� */
    {
        curr_state = new_state;                 /* ��ǰ״̬����Ϊ��״̬*/

        /*  ���ݵ�ǰ������ַ��͵�ǰ״̬���õ���״̬*/
        new_state = atFindNextMainState(AT_E_CMD_MAIN_STATE_TAB,*pucCurrPtr,curr_state);

        switch(new_state)                       /* ״̬����*/
        {
        case AT_E_CMD_NAME_STATE:               /* ��չ������״̬*/
            if(curr_state != new_state)         /* ��״̬���ֲ����ڵ�ǰ״̬*/
            {
                g_stATParseCmd.ucCmdFmtType = AT_EXTEND_CMD_TYPE;           /* ������������*/
                g_stATParseCmd.ucCmdOptType = AT_CMD_OPT_SET_CMD_NO_PARA;   /* ���������������*/

                pucCopyPtr = pucCurrPtr;            /* ׼��������չ����������*/
            }
            break;

        case AT_E_CMD_SET_STATE:                /* AT��չ����Ⱥ�״̬ */
            /* �洢��������ȫ�ֱ����� */
            g_stATParseCmd.ucCmdOptType = AT_CMD_OPT_SET_PARA_CMD;  /* ���������������*/
            g_stATParseCmd.stCmdName.usCmdNameLen = (VOS_UINT16)(pucCurrPtr - pucCopyPtr);
            if(g_stATParseCmd.stCmdName.usCmdNameLen < sizeof(g_stATParseCmd.stCmdName.aucCmdName))
            {
                atRangeCopy(g_stATParseCmd.stCmdName.aucCmdName,pucCopyPtr,pucCurrPtr);
            }
            else
            {
                return AT_ERROR;                  /* ���ش��� */
            }
            break;

        case AT_E_CMD_PARA_STATE:               /* AT��չ�������״̬ */
        case AT_E_CMD_LEFT_QUOT_STATE:          /* AT��չ����������״̬ */
            if(curr_state != new_state)         /* ��״̬���ֲ����ڵ�ǰ״̬*/
            {
                pucCopyPtr = pucCurrPtr;            /* ׼�����ղ�������*/
            }
            break;

        case AT_E_CMD_COLON_STATE:              /* AT��չ�����״̬ */
            if((curr_state != new_state) && (AT_E_CMD_SET_STATE != curr_state))         /* ��״̬���ֲ����ڵ�ǰ״̬*/
            {
                /* �洢������ȫ�ֱ����� */
                if(ERR_MSP_SUCCESS != atfwParseSaveParam(pucCopyPtr, (VOS_UINT16)(pucCurrPtr - pucCopyPtr)))
                {
                    return AT_ERROR;
                }

            }
            else
            {
                /*������������,����������*/
                if(ERR_MSP_SUCCESS != atfwParseSaveParam(pucCopyPtr, 0))
                {
                    return AT_ERROR;
                }

            }
            break;

        case AT_E_CMD_READ_STATE:                   /* AT��չ�����ѯ����״̬ */

            /* �洢��������ȫ�ֱ����� */
            g_stATParseCmd.stCmdName.usCmdNameLen = (VOS_UINT16)(pucCurrPtr - pucCopyPtr);
            if (g_stATParseCmd.stCmdName.usCmdNameLen < sizeof(g_stATParseCmd.stCmdName.aucCmdName))
            {
                atRangeCopy(g_stATParseCmd.stCmdName.aucCmdName,pucCopyPtr,pucCurrPtr);
            }
            else
            {
                return AT_ERROR;                  /* ���ش��� */
            }

            g_stATParseCmd.ucCmdOptType = AT_CMD_OPT_READ_CMD;   /* ���������������*/

            break;

        case AT_E_CMD_TEST_STATE:                   /* AT��չ������Բ���״̬ */
            g_stATParseCmd.ucCmdOptType = AT_CMD_OPT_TEST_CMD;   /* ���������������*/
            break;

        case AT_BUTT_STATE:                         /* ��Ч״̬ */
            return AT_ERROR;                      /* ���ش��� */

        default:
            break;
        }
        pucCurrPtr++;                                 /* ����������һ���ַ�*/
    }

    switch(new_state)                               /* ��������״̬�ж�*/
    {
    case AT_E_CMD_PARA_STATE:                       /* AT��չ�������״̬ */
    case AT_E_CMD_RIGHT_QUOT_STATE:                 /* AT��չ����������״̬ */

        /* �洢������ȫ�ֱ����� */
        if (ERR_MSP_SUCCESS != atfwParseSaveParam(pucCopyPtr, (VOS_UINT16)(pucCurrPtr - pucCopyPtr)))
        {
            return AT_ERROR;
        }

        break;

    case AT_E_CMD_NAME_STATE:                   /* AT��չ�����ѯ����״̬ */

        /* �洢��������ȫ�ֱ����� */
        g_stATParseCmd.stCmdName.usCmdNameLen = (VOS_UINT16)(pucCurrPtr - pucCopyPtr);
        if (g_stATParseCmd.stCmdName.usCmdNameLen < sizeof(g_stATParseCmd.stCmdName.aucCmdName))
        {
            atRangeCopy(g_stATParseCmd.stCmdName.aucCmdName,pucCopyPtr,pucCurrPtr);
        }
        else
        {
            return AT_ERROR;                  /* ���ش��� */
        }

        g_stATParseCmd.ucCmdOptType = AT_CMD_OPT_SET_CMD_NO_PARA;   /* ���������������*/

        break;

    case AT_E_CMD_COLON_STATE:                      /* AT�����������޲εĶ��� */

        if (ERR_MSP_SUCCESS != atfwParseSaveParam(pucCopyPtr, 0))
        {
            return AT_ERROR;
        }

        break;

    case AT_E_CMD_SET_STATE:                        /* AT��չ����Ⱥ�״̬ */
    case AT_E_CMD_TEST_STATE:                       /* AT��չ�����ѯ����״̬ */
    case AT_E_CMD_READ_STATE:                       /* AT��չ������Բ���״̬ */
        break;

    default:
        return AT_ERROR;                          /* ���ش���*/
    }
    return AT_SUCCESS;                              /*  ������ȷ*/
}

/* Modified by s62952 for BalongV300R002 Build�Ż���Ŀ 2012-02-28, begin */

VOS_UINT32 AT_ParseSetDockCmd(
    VOS_UINT8                          *pucData,
    VOS_UINT16                          usLen
)
{
    VOS_INT8                            cRet;
    VOS_UINT16                          usCmdlen;
    VOS_UINT32                          ulPos;
    VOS_UINT8                          *pucDataPara = VOS_NULL_PTR;
    /* Modified by l60609 for DSDA Phase III, 2013-2-25, Begin */
    AT_PARSE_CMD_NAME_TYPE_STRU         stAtCmdName;

    TAF_MEM_SET_S(&stAtCmdName, sizeof(stAtCmdName), 0x00, sizeof(stAtCmdName));
    /* Modified by l60609 for DSDA Phase III, 2013-2-25, End */

    if (0 == usLen)
    {
        return AT_FAILURE;
    }

    pucDataPara = (VOS_UINT8*)PS_MEM_ALLOC(WUEPS_PID_AT, usLen);

    if (VOS_NULL_PTR == pucDataPara)
    {
        AT_ERR_LOG("AT_ParseSetDockCmd: pucDataPara Memory malloc failed!");
        return AT_FAILURE;
    }

    TAF_MEM_CPY_S(pucDataPara, usLen, pucData, usLen);

    /* ȥ���ַ����е��˸�� */
    if (AT_FAILURE == At_ScanDelChar(pucDataPara, &usLen))
    {
        PS_MEM_FREE(WUEPS_PID_AT, pucDataPara);
        return AT_FAILURE;
    }

    /* ��������ַ�������С��"AT^DOCK="����ֱ�ӷ���AT_FAILURE */
    usCmdlen = (VOS_UINT16)VOS_StrLen("AT^DOCK=");
    if (usLen < usCmdlen)
    {
        PS_MEM_FREE(WUEPS_PID_AT, pucDataPara);
        return AT_FAILURE;
    }

    /* AT����ͷ�ַ�ת��д */
    At_UpString(pucDataPara, usCmdlen);

    /* ��������ַ���ͷ������"AT^DOCK="ֱ�ӷ���AT_FAILURE */
    cRet = VOS_StrNiCmp((VOS_CHAR *)pucDataPara, "AT^DOCK=", usCmdlen);
    if (0 != cRet)
    {
        PS_MEM_FREE(WUEPS_PID_AT, pucDataPara);
        return AT_FAILURE;
    }

    /* ��ȡ����(����������ǰ׺AT)���Ƽ����� */
    ulPos = VOS_StrLen("AT");
    /* Modified by l60609 for DSDA Phase III, 2013-2-25, Begin */
    stAtCmdName.usCmdNameLen = (VOS_UINT16)VOS_StrLen("^DOCK");
    TAF_MEM_CPY_S(stAtCmdName.aucCmdName,
               sizeof(stAtCmdName.aucCmdName),
               (pucDataPara + ulPos),
               stAtCmdName.usCmdNameLen);
    stAtCmdName.aucCmdName[stAtCmdName.usCmdNameLen] = '\0';
    ulPos += stAtCmdName.usCmdNameLen;
    /* Modified by l60609 for DSDA Phase III, 2013-2-25, End */

    ulPos += VOS_StrLen("=");

    /* ��ȡ����Ĳ����ַ��������� */
    gastAtParaList[0].usParaLen = usLen - (VOS_UINT16)ulPos;
    TAF_MEM_CPY_S(gastAtParaList[0].aucPara,
               sizeof(gastAtParaList[0].aucPara),
               (pucDataPara + ulPos),
               gastAtParaList[0].usParaLen);

    /* �����������ͣ��������ͺͲ������� */
    gucAtParaIndex  = 1;
    g_stATParseCmd.ucCmdOptType = AT_CMD_OPT_SET_PARA_CMD;
    gucAtCmdFmtType = AT_EXTEND_CMD_TYPE;

    PS_MEM_FREE(WUEPS_PID_AT, pucDataPara);
    return AT_SUCCESS;
}
/* Modified by s62952 for BalongV300R002 Build�Ż���Ŀ 2012-02-28, end */


