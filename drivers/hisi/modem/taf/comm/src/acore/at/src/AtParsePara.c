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

#include "AtCheckFunc.h"



#define    THIS_FILE_ID        PS_FILE_ID_AT_PARSEPARA_C


/* ����ƥ���ʼ��״̬�� */
AT_SUB_STATE_STRU AT_PARA_NONE_STATE_TAB[]=
{
    /* �����ǰ״̬��AT_PARA_NONE_STATE��atCheckLeftBracket�ɹ��������AT_PARA_LEFT_BRACKET_STATE */
    {    At_CheckLeftBracket    ,    AT_PARA_LEFT_BRACKET_STATE    },

    /* ��״̬����� */
    {    NULL    ,    AT_BUTT_STATE    },
};

/* ����ƥ��������״̬�� */
AT_SUB_STATE_STRU AT_PARA_LEFT_BRACKET_STATE_TAB[]=
{
    /* �����ǰ״̬��AT_PARA_LEFT_BRACKET_STATE��atCheckZero�ɹ��������AT_PARA_ZERO_STATE */
    {At_CheckChar0,   AT_PARA_ZERO_STATE},

    /* �����ǰ״̬��AT_PARA_LEFT_BRACKET_STATE��atCheckDigit�ɹ��������AT_PARA_NUM_STATE */
    {At_CheckDigit,  AT_PARA_NUM_STATE},

    /* �����ǰ״̬��AT_PARA_LEFT_BRACKET_STATE��at_NoQuotLetter�ɹ��������AT_PARA_NO_QUOT_LETTER_STATE */
    {atNoQuotLetter, AT_PARA_NO_QUOT_LETTER_STATE},

    /* �����ǰ״̬��AT_PARA_LEFT_BRACKET_STATE��atCheckLetter�ɹ��������AT_PARA_LETTER_STATE */
    {At_CheckLetter, AT_PARA_LETTER_STATE},

    /* �����ǰ״̬��AT_PARA_LEFT_BRACKET_STATE��atCheckQuot�ɹ��������AT_PARA_LEFT_QUOT_STATE */
    {At_CheckQuot,   AT_PARA_LEFT_QUOT_STATE},

    /* ��״̬����� */
    {NULL,           AT_BUTT_STATE},
};

/* ����ƥ�����״̬�� */
AT_SUB_STATE_STRU AT_PARA_NUM_STATE_TAB[]=
{
    /* �����ǰ״̬��AT_PARA_NUM_STATE��atCheckDigit�ɹ��������AT_PARA_NUM_STATE */
    {   At_CheckDigit   ,   AT_PARA_NUM_STATE    },

    /* �����ǰ״̬��AT_PARA_NUM_STATE��atCheckColon�ɹ��������AT_PARA_NUM_COLON_STATE */
    {   atCheckComma ,   AT_PARA_NUM_COLON_STATE  },

    /* �����ǰ״̬��AT_PARA_NUM_STATE��atCheckRightBracket�ɹ��������AT_PARA_RIGHT_BRACKET_STATE */
    {   At_CheckRightBracket  , AT_PARA_RIGHT_BRACKET_STATE   },

    /* �����ǰ״̬��AT_PARA_NUM_STATE��atCheckCharSub�ɹ��������AT_PARA_SUB_STATE */
    {   At_CheckCharSub   ,   AT_PARA_SUB_STATE    },

    /* ��״̬����� */
    {    NULL    ,    AT_BUTT_STATE    },
};

/* ����ƥ�䷶Χ״̬�� */
AT_SUB_STATE_STRU AT_PARA_SUB_STATE_TAB[]=
{
    /* �����ǰ״̬��AT_PARA_SUB_STATE��atCheckZero�ɹ��������AT_PARA_ZERO_SUB_STATE */
    {At_CheckChar0,  AT_PARA_ZERO_SUB_STATE},

    /* �����ǰ״̬��AT_PARA_SUB_STATE��atCheckDigit�ɹ��������AT_PARA_NUM_SUB_STATE */
    {At_CheckDigit, AT_PARA_NUM_SUB_STATE},

    /* ��״̬����� */
    {NULL,          AT_BUTT_STATE},
};

/* ����ƥ�䷶Χ����״̬�� */
AT_SUB_STATE_STRU AT_PARA_NUM_SUB_STATE_TAB[]=
{
    /* �����ǰ״̬��AT_PARA_NUM_SUB_STATE��atCheckDigit�ɹ��������AT_PARA_NUM_SUB_STATE */
    {   At_CheckDigit   ,   AT_PARA_NUM_SUB_STATE    },

    /* �����ǰ״̬��AT_PARA_NUM_SUB_STATE��atCheckColon�ɹ��������AT_PARA_NUM_SUB_COLON_STATE */
    {   atCheckComma ,   AT_PARA_NUM_SUB_COLON_STATE  },

    /* �����ǰ״̬��AT_PARA_NUM_SUB_STATE��atCheckRightBracket�ɹ��������AT_PARA_RIGHT_BRACKET_STATE */
    {   At_CheckRightBracket  ,   AT_PARA_RIGHT_BRACKET_STATE   },

    /* ��״̬����� */
    {    NULL    ,    AT_BUTT_STATE    },
};

/* ����ƥ�䷶Χ���ֶ���״̬�� */
AT_SUB_STATE_STRU AT_PARA_NUM_SUB_COLON_STATE_TAB[]=
{
    /* �����ǰ״̬��AT_PARA_NUM_SUB_COLON_STATE��atCheckZero�ɹ��������AT_PARA_ZERO_STATE */
    {At_CheckChar0,  AT_PARA_ZERO_STATE},

    /* �����ǰ״̬��AT_PARA_NUM_SUB_COLON_STATE��atCheckDigit�ɹ��������AT_PARA_NUM_STATE */
    {At_CheckDigit, AT_PARA_NUM_STATE},

    /* ��״̬����� */
    {NULL,          AT_BUTT_STATE},
};

/* ����ƥ��������״̬�� */
AT_SUB_STATE_STRU AT_PARA_RIGHT_BRACKET_STATE_TAB[]=
{
    /* �����ǰ״̬��AT_PARA_RIGHT_BRACKET_STATE��atCheckColon�ɹ��������AT_PARA_COLON_STATE */
    {   atCheckComma ,   AT_PARA_COLON_STATE  },

    /* ��״̬����� */
    {    NULL    ,    AT_BUTT_STATE    },
};

/* ����ƥ�䶺��״̬�� */
AT_SUB_STATE_STRU AT_PARA_COLON_STATE_TAB[]=
{
    /* �����ǰ״̬��AT_PARA_COLON_STATE��atCheckLeftBracket�ɹ��������AT_PARA_LEFT_BRACKET_STATE */
    {   At_CheckLeftBracket  ,   AT_PARA_LEFT_BRACKET_STATE   },

    /* ��״̬����� */
    {    NULL    ,    AT_BUTT_STATE    },
};

/* ����ƥ��������״̬�� */
AT_SUB_STATE_STRU AT_PARA_LEFT_QUOT_STATE_TAB[]=
{
    /* �����ǰ״̬��AT_PARA_LEFT_QUOT_STATE��atCheckNoQuot�ɹ��������AT_PARA_LEFT_QUOT_STATE */
    {   At_CheckNoQuot   ,   AT_PARA_LEFT_QUOT_STATE },

    /* �����ǰ״̬��AT_PARA_LEFT_QUOT_STATE��atCheckQuot�ɹ��������AT_PARA_RIGHT_QUOT_STATE */
    {   At_CheckQuot ,    AT_PARA_RIGHT_QUOT_STATE },

    /* ��״̬����� */
    {    NULL    ,    AT_BUTT_STATE    },
};

/* ����ƥ��������״̬�� */
AT_SUB_STATE_STRU AT_PARA_RIGHT_QUOT_STATE_TAB[]=
{
    /* �����ǰ״̬��AT_PARA_RIGHT_QUOT_STATE��atCheckRightBracket�ɹ��������AT_PARA_RIGHT_BRACKET_STATE */
    {   At_CheckRightBracket  ,   AT_PARA_RIGHT_BRACKET_STATE   },

    /* �����ǰ״̬��AT_PARA_RIGHT_QUOT_STATE��atCheckColon�ɹ��������AT_PARA_QUOT_COLON_STATE */
    {   atCheckComma ,   AT_PARA_QUOT_COLON_STATE },

    /* ��״̬����� */
    {    NULL    ,    AT_BUTT_STATE    },
};

/* ����ƥ����ĸ״̬�� */
AT_SUB_STATE_STRU AT_PARA_LETTER_STATE_TAB[]=
{
    /* �����ǰ״̬��AT_PARA_LETTER_STATE��atCheckLetter�ɹ��������AT_PARA_LETTER_STATE */
    {   At_CheckLetter    ,   AT_PARA_LETTER_STATE },

    /* �����ǰ״̬��AT_PARA_LETTER_STATE��atCheckRightBracket�ɹ��������AT_PARA_RIGHT_BRACKET_STATE */
    {   At_CheckRightBracket  ,   AT_PARA_RIGHT_BRACKET_STATE   },

    /* �����ǰ״̬��AT_PARA_LETTER_STATE��atCheckCharSub�ɹ��������AT_PARA_LETTER_STATE */
    {   At_CheckCharSub   ,   AT_PARA_LETTER_STATE },

    /* �����ǰ״̬��AT_PARA_LETTER_STATE��atCheckColon�ɹ��������AT_PARA_LETTER_STATE */
    {   atCheckColon     ,   AT_PARA_LETTER_STATE },

    /* �����ǰ״̬��AT_PARA_LETTER_STATE��atCheckblank�ɹ��������AT_PARA_LETTER_STATE */
    {   atCheckblank     ,   AT_PARA_LETTER_STATE },

    /* ��״̬����� */
    {    NULL    ,    AT_BUTT_STATE    },
};

/* ����ƥ�����ֶ���״̬�� */
AT_SUB_STATE_STRU AT_PARA_NUM_COLON_STATE_TAB[]=
{
    /* �����ǰ״̬��AT_PARA_NUM_COLON_STATE��atCheckZero�ɹ��������AT_PARA_ZERO_STATE */
    {At_CheckChar0,  AT_PARA_ZERO_STATE},

    /* �����ǰ״̬��AT_PARA_NUM_COLON_STATE��atCheckDigit�ɹ��������AT_PARA_NUM_STATE */
    {At_CheckDigit, AT_PARA_NUM_STATE},

    /* ��״̬����� */
    {NULL,          AT_BUTT_STATE},
};

/* ����ƥ�����Ŷ���״̬�� */
AT_SUB_STATE_STRU AT_PARA_QUOT_COLON_STATE_TAB[]=
{
    /* �����ǰ״̬��AT_PARA_QUOT_COLON_STATE��atCheckQuot�ɹ��������AT_PARA_LEFT_QUOT_STATE */
    {   At_CheckQuot  ,   AT_PARA_LEFT_QUOT_STATE },

    /* ��״̬����� */
    {    NULL    ,    AT_BUTT_STATE    },
};


/* ֧�ְ˽��ơ�ʮ�����Ʋ��� BEGIN */

/* ����ƥ������0״̬�� */
AT_SUB_STATE_STRU AT_PARA_ZERO_STATE_TAB[]=
{
    /* �����ǰ״̬��AT_PARA_ZERO_STATE��at_CheckHex�ɹ��������AT_PARA_HEX_STATE */
    {atCheckHex,          AT_PARA_HEX_STATE},

    /* �����ǰ״̬��AT_PARA_ZERO_STATE��atCheckDigit�ɹ��������AT_PARA_NUM_STATE */
    {At_CheckDigit,        AT_PARA_NUM_STATE},

    /* �����ǰ״̬��AT_PARA_ZERO_STATE��atCheckColon�ɹ��������AT_PARA_NUM_COLON_STATE */
    {atCheckComma,        AT_PARA_NUM_COLON_STATE},

    /* �����ǰ״̬��AT_PARA_ZERO_STATE��atCheckRightBracket�ɹ��������AT_PARA_RIGHT_BRACKET_STATE */
    {At_CheckRightBracket, AT_PARA_RIGHT_BRACKET_STATE},

    /* �����ǰ״̬��AT_PARA_ZERO_STATE��atCheckCharSub�ɹ��������AT_PARA_SUB_STATE */
    {At_CheckCharSub,      AT_PARA_SUB_STATE},

    /* ��״̬����� */
    {NULL,                 AT_BUTT_STATE},
};

/* ����ƥ��ʮ������״̬�� */
AT_SUB_STATE_STRU AT_PARA_HEX_STATE_TAB[]=
{
    /* �����ǰ״̬��AT_PARA_HEX_STATE��at_CheckHex�ɹ��������AT_PARA_HEX_NUM_STATE */
    {atCheckHexNum, AT_PARA_HEX_NUM_STATE},

    /* ��״̬����� */
    {NULL,           AT_BUTT_STATE},
};

/* ����ƥ��ʮ����������״̬�� */
AT_SUB_STATE_STRU AT_PARA_HEX_NUM_STATE_TAB[]=
{
    /* �����ǰ״̬��AT_PARA_ZERO_STATE��at_CheckHex�ɹ��������AT_PARA_HEX_STATE */
    {atCheckHexNum,       AT_PARA_HEX_NUM_STATE},

    /* �����ǰ״̬��AT_PARA_ZERO_STATE��atCheckColon�ɹ��������AT_PARA_NUM_COLON_STATE */
    {atCheckComma,        AT_PARA_NUM_COLON_STATE},

    /* �����ǰ״̬��AT_PARA_ZERO_STATE��atCheckRightBracket�ɹ��������AT_PARA_RIGHT_BRACKET_STATE */
    {At_CheckRightBracket, AT_PARA_RIGHT_BRACKET_STATE},

    /* �����ǰ״̬��AT_PARA_ZERO_STATE��atCheckCharSub�ɹ��������AT_PARA_SUB_STATE */
    {At_CheckCharSub,      AT_PARA_SUB_STATE},

    /* ��״̬����� */
    {NULL,                 AT_BUTT_STATE},
};

/* ����ƥ�䷶Χ����0״̬�� */
AT_SUB_STATE_STRU AT_PARA_ZERO_SUB_STATE_TAB[]=
{
    /* �����ǰ״̬��AT_PARA_ZERO_SUB_STATE��at_CheckHex�ɹ��������AT_PARA_HEX_SUB_STATE */
    {atCheckHex,          AT_PARA_HEX_SUB_STATE},

    /* �����ǰ״̬��AT_PARA_NUM_SUB_STATE��atCheckDigit�ɹ��������AT_PARA_NUM_SUB_STATE */
    {At_CheckDigit,        AT_PARA_NUM_SUB_STATE    },

    /* �����ǰ״̬��AT_PARA_NUM_SUB_STATE��atCheckColon�ɹ��������AT_PARA_NUM_SUB_COLON_STATE */
    {atCheckComma,        AT_PARA_NUM_SUB_COLON_STATE  },

    /* �����ǰ״̬��AT_PARA_NUM_SUB_STATE��atCheckRightBracket�ɹ��������AT_PARA_RIGHT_BRACKET_STATE */
    {At_CheckRightBracket, AT_PARA_RIGHT_BRACKET_STATE   },

    /* ��״̬����� */
    {NULL,                 AT_BUTT_STATE    },
};

/* ����ƥ�䷶Χʮ������״̬�� */
AT_SUB_STATE_STRU AT_PARA_HEX_SUB_STATE_TAB[]=
{
    /* �����ǰ״̬��AT_PARA_HEX_SUB_STATE��at_CheckHex�ɹ��������AT_PARA_HEX_NUM_SUB_STATE */
    {atCheckHexNum, AT_PARA_HEX_NUM_SUB_STATE},

    /* ��״̬����� */
    {NULL,           AT_BUTT_STATE},
};

/* ����ƥ�䷶Χʮ����������״̬�� */
AT_SUB_STATE_STRU AT_PARA_HEX_NUM_SUB_STATE_TAB[]=
{
    /* �����ǰ״̬��AT_PARA_HEX_NUM_SUB_STATE��at_CheckHexNum�ɹ��������AT_PARA_HEX_NUM_SUB_STATE */
    {atCheckHexNum,       AT_PARA_HEX_NUM_SUB_STATE},

    /* �����ǰ״̬��AT_PARA_NUM_SUB_STATE��atCheckColon�ɹ��������AT_PARA_NUM_SUB_COLON_STATE */
    {atCheckComma,        AT_PARA_NUM_SUB_COLON_STATE  },

    /* �����ǰ״̬��AT_PARA_NUM_SUB_STATE��atCheckRightBracket�ɹ��������AT_PARA_RIGHT_BRACKET_STATE */
    {At_CheckRightBracket, AT_PARA_RIGHT_BRACKET_STATE   },

    /* ��״̬����� */
    {NULL,                 AT_BUTT_STATE    },
};

/* ֧�ְ˽��ơ�ʮ�����Ʋ��� END */

/* ֧����˫���Ű����ַ��� BEGIN */

/* ����ƥ����˫���Ű�����ĸ״̬�� */
AT_SUB_STATE_STRU AT_PARA_NO_QUOT_LETTER_STATE_TAB[]=
{
    /* �����ǰ״̬��AT_PARA_NO_QUOT_LETTER_STATE��atCheckLetter�ɹ��������AT_PARA_NO_QUOT_LETTER_STATE */
    {At_CheckLetter,       AT_PARA_NO_QUOT_LETTER_STATE },

    /* �����ǰ״̬��AT_PARA_NO_QUOT_LETTER_STATE��atCheckRightBracket�ɹ��������AT_PARA_RIGHT_BRACKET_STATE */
    {At_CheckRightBracket, AT_PARA_RIGHT_BRACKET_STATE   },

    /* ��״̬����� */
    {NULL,                 AT_BUTT_STATE    },
};

/* ֧����˫���Ű����ַ��� END */

/* ����ƥ����״̬�� */
AT_MAIN_STATE_STRU AT_MAIN_PARA_STATE_TAB[] =
{
    /* �����ǰ״̬��AT_NONE_STATE�������AT_PARA_NONE_STATE_TAB��״̬�� */
    {   AT_NONE_STATE   ,   AT_PARA_NONE_STATE_TAB  },

    /* �����ǰ״̬��AT_PARA_LEFT_BRACKET_STATE�������AT_PARA_LEFT_BRACKET_STATE_TAB��״̬�� */
    {   AT_PARA_LEFT_BRACKET_STATE  , AT_PARA_LEFT_BRACKET_STATE_TAB    },

    /* �����ǰ״̬��AT_PARA_RIGHT_BRACKET_STATE�������AT_PARA_RIGHT_BRACKET_STATE_TAB��״̬�� */
    {   AT_PARA_RIGHT_BRACKET_STATE  , AT_PARA_RIGHT_BRACKET_STATE_TAB    },

    /* �����ǰ״̬��AT_PARA_LETTER_STATE�������AT_PARA_LETTER_STATE_TAB��״̬�� */
    {   AT_PARA_LETTER_STATE    ,   AT_PARA_LETTER_STATE_TAB    },

    /* �����ǰ״̬��AT_PARA_NUM_STATE�������AT_PARA_NUM_STATE_TAB��״̬�� */
    {   AT_PARA_NUM_STATE    ,   AT_PARA_NUM_STATE_TAB  },

    /* �����ǰ״̬��AT_PARA_NUM_COLON_STATE�������AT_PARA_NUM_COLON_STATE_TAB��״̬�� */
    {   AT_PARA_NUM_COLON_STATE ,    AT_PARA_NUM_COLON_STATE_TAB  },

    /* �����ǰ״̬��AT_PARA_SUB_STATE�������AT_PARA_SUB_STATE_TAB��״̬�� */
    {   AT_PARA_SUB_STATE    ,   AT_PARA_SUB_STATE_TAB  },

    /* �����ǰ״̬��AT_PARA_NUM_SUB_STATE�������AT_PARA_NUM_SUB_STATE_TAB��״̬�� */
    {   AT_PARA_NUM_SUB_STATE   ,  AT_PARA_NUM_SUB_STATE_TAB  },

    /* �����ǰ״̬��AT_PARA_NUM_SUB_COLON_STATE�������AT_PARA_NUM_SUB_COLON_STATE_TAB��״̬�� */
    {   AT_PARA_NUM_SUB_COLON_STATE  ,   AT_PARA_NUM_SUB_COLON_STATE_TAB  },

    /* �����ǰ״̬��AT_PARA_COLON_STATE�������AT_PARA_COLON_STATE_TAB��״̬�� */
    {   AT_PARA_COLON_STATE ,    AT_PARA_COLON_STATE_TAB  },

    /* �����ǰ״̬��AT_PARA_LEFT_QUOT_STATE�������AT_PARA_LEFT_QUOT_STATE_TAB��״̬�� */
    {   AT_PARA_LEFT_QUOT_STATE ,   AT_PARA_LEFT_QUOT_STATE_TAB    },

    /* �����ǰ״̬��AT_PARA_RIGHT_QUOT_STATE�������AT_PARA_RIGHT_QUOT_STATE_TAB��״̬�� */
    {   AT_PARA_RIGHT_QUOT_STATE    ,   AT_PARA_RIGHT_QUOT_STATE_TAB    },

    /* �����ǰ״̬��AT_PARA_QUOT_COLON_STATE�������AT_PARA_QUOT_COLON_STATE_TAB��״̬�� */
    {   AT_PARA_QUOT_COLON_STATE    ,   AT_PARA_QUOT_COLON_STATE_TAB    },

    /* �����ǰ״̬��AT_PARA_ZERO_STATE�������AT_PARA_ZERO_STATE_TAB��״̬�� */
    {AT_PARA_ZERO_STATE,           AT_PARA_ZERO_STATE_TAB},

    /* �����ǰ״̬��AT_PARA_ZERO_SUB_STATE�������AT_PARA_ZERO_SUB_STATE_TAB��״̬�� */
    {AT_PARA_ZERO_SUB_STATE,       AT_PARA_ZERO_SUB_STATE_TAB},

    /* �����ǰ״̬��AT_PARA_HEX_STATE�������AT_PARA_HEX_STATE_TAB��״̬�� */
    {AT_PARA_HEX_STATE,            AT_PARA_HEX_STATE_TAB},

    /* �����ǰ״̬��AT_PARA_HEX_NUM_STATE�������AT_PARA_HEX_NUM_STATE_TAB��״̬�� */
    {AT_PARA_HEX_NUM_STATE,        AT_PARA_HEX_NUM_STATE_TAB},

    /* �����ǰ״̬��AT_PARA_HEX_SUB_STATE�������AT_PARA_HEX_SUB_STATE_TAB��״̬�� */
    {AT_PARA_HEX_SUB_STATE,        AT_PARA_HEX_SUB_STATE_TAB},

    /* �����ǰ״̬��AT_PARA_HEX_NUM_SUB_STATE�������AT_PARA_HEX_NUM_SUB_STATE_TAB��״̬�� */
    {AT_PARA_HEX_NUM_SUB_STATE,    AT_PARA_HEX_NUM_SUB_STATE_TAB},

    /* �����ǰ״̬��AT_PARA_NO_QUOT_LETTER_STATE�������AT_PARA_NO_QUOT_LETTER_STATE_TAB��״̬�� */
    {AT_PARA_NO_QUOT_LETTER_STATE, AT_PARA_NO_QUOT_LETTER_STATE_TAB},

    /* ��״̬����� */
    {   AT_BUTT_STATE   ,   NULL  },
};

pAtChkFuncType      pgAtCheckFunc   = NULL;



VOS_UINT32 atCmparePara(VOS_VOID)
{
    /*����������*/
    if(NULL != pgAtCheckFunc)
    {
        /*�������б��Ӧ�Ĳ����Ƿ����*/
        if(0 < gastAtParaList[g_stATParseCmd.ucParaCheckIndex].usParaLen)
        {
            /*�������������*/
            if(AT_SUCCESS != pgAtCheckFunc(&gastAtParaList[g_stATParseCmd.ucParaCheckIndex]))
            {
                return AT_FAILURE;           /*  ���ش���*/
            }
        }

        /*��¼�Ѽ�������ȫ�ֱ�����1*/
        g_stATParseCmd.ucParaCheckIndex++;

        /*��գ��Ա��Ƚ���һ������,���򣬶ԱȲ�����������*/
        if(0 != g_stATParseCmd.ucParaNumRangeIndex)
        {
            g_stATParseCmd.ucParaNumRangeIndex = 0;
            TAF_MEM_SET_S(g_stATParseCmd.astParaNumRange,sizeof(g_stATParseCmd.astParaNumRange), 0x00, sizeof(g_stATParseCmd.astParaNumRange));
        }

        if(0 != g_stATParseCmd.ucParaStrRangeIndex)
        {
            g_stATParseCmd.ucParaStrRangeIndex = 0;
            TAF_MEM_SET_S(g_stATParseCmd.auStrRange, sizeof(g_stATParseCmd.auStrRange), 0x00, sizeof(g_stATParseCmd.auStrRange));
        }

        pgAtCheckFunc = NULL;

        return AT_SUCCESS;                   /*  ������ȷ*/
    }
    else
    {
        return AT_FAILURE;                   /*  ���ش���*/
    }
}



VOS_UINT32 atParsePara( VOS_UINT8 * pData, VOS_UINT16 usLen)
{
    VOS_UINT16 usLength = 0;                        /*  ��¼��ǰ�Ѿ�������ַ�����*/
    VOS_UINT8 *pucCurrPtr = pData;                  /*  ָ��ǰ���ڴ�����ַ�*/
    VOS_UINT8 *pucCopyPtr = pData;                  /*  �������ݵ���ʼָ�� */
    AT_STATE_TYPE_ENUM curr_state = AT_NONE_STATE;  /*  ���ó�ʼ״̬ */
    AT_STATE_TYPE_ENUM new_state = AT_NONE_STATE;   /*  ���ó�ʼ״̬ */

    /* ���η����ַ����е�ÿ���ַ�*/
    while( (usLength++ < usLen) && (g_stATParseCmd.ucParaNumRangeIndex < AT_MAX_PARA_NUMBER) && (g_stATParseCmd.ucParaStrRangeIndex < AT_MAX_PARA_NUMBER))        /* ���αȽ�ÿ���ַ� */
    {
        curr_state = new_state;                 /*  ��ǰ״̬����Ϊ��״̬*/

        /* ���ݵ�ǰ������ַ��͵�ǰ״̬���õ���״̬ */
        new_state = atFindNextMainState(AT_MAIN_PARA_STATE_TAB,*pucCurrPtr,curr_state);

        switch(new_state)                       /* ״̬����*/
        {
        case AT_PARA_NUM_STATE:                 /* ����״̬*/
        case AT_PARA_LETTER_STATE:              /* ��ĸ״̬*/
        case AT_PARA_LEFT_QUOT_STATE:           /* ����������״̬*/
        case AT_PARA_NUM_SUB_STATE:             /* ��Χ����״̬*/
        case AT_PARA_ZERO_STATE:                /* ����0״̬ */
        case AT_PARA_ZERO_SUB_STATE:            /* ��Χ����0״̬ */
        case AT_PARA_NO_QUOT_LETTER_STATE:      /* ��˫���Ű�����ĸ״̬ */
            if(curr_state != new_state)         /* ��״̬���ֲ����ڵ�ǰ״̬*/
            {
                pucCopyPtr = pucCurrPtr;            /*  ׼�����ղ���*/
            }
            break;

        case AT_PARA_HEX_STATE:                 /* ʮ������״̬ */
            /* continue */
        case AT_PARA_HEX_NUM_STATE:             /* ʮ����������״̬ */
            /* continue */
        case AT_PARA_HEX_SUB_STATE:             /* ƥ�䷶Χʮ������״̬ */
            /* continue */
        case AT_PARA_HEX_NUM_SUB_STATE:         /* ƥ�䷶Χʮ����������״̬ */
            break;

        case AT_PARA_QUOT_COLON_STATE:          /* ���Ŷ���״̬ */

            /* �������ַ������ű��б� */
            /* �ַ�����������1*/

            if ((VOS_UINT32)(pucCurrPtr - pucCopyPtr) < sizeof(g_stATParseCmd.auStrRange[g_stATParseCmd.ucParaStrRangeIndex]))
            {
                atRangeCopy(g_stATParseCmd.auStrRange[g_stATParseCmd.ucParaStrRangeIndex++],pucCopyPtr,pucCurrPtr);
            }
            else
            {
                return AT_FAILURE;                  /* ���ش��� */
            }
            break;

        case AT_PARA_NUM_SUB_COLON_STATE:

            /* ת�����ֿ��������ֲ����ű��б���Ͻ� */
            /* ���ֲ���������1*/
            g_stATParseCmd.astParaNumRange[g_stATParseCmd.ucParaNumRangeIndex++].ulBig = atRangeToU32(pucCopyPtr,pucCurrPtr);

            break;

        case AT_PARA_NUM_COLON_STATE:

            /* ת�����ֿ��������ֲ����ű��б���½� */
            g_stATParseCmd.astParaNumRange[g_stATParseCmd.ucParaNumRangeIndex].ulSmall = atRangeToU32(pucCopyPtr,pucCurrPtr);

            /* �Ͻ�����½�*/
            g_stATParseCmd.astParaNumRange[g_stATParseCmd.ucParaNumRangeIndex].ulBig = g_stATParseCmd.astParaNumRange[g_stATParseCmd.ucParaNumRangeIndex].ulSmall;

            /* ���ֲ���������1*/
            g_stATParseCmd.ucParaNumRangeIndex++;

            break;

        case AT_PARA_SUB_STATE:

            /* ת�����ֿ��������ֲ����ű��б���½� */
            g_stATParseCmd.astParaNumRange[g_stATParseCmd.ucParaNumRangeIndex].ulSmall = atRangeToU32(pucCopyPtr,pucCurrPtr);

            break;

        case AT_PARA_RIGHT_BRACKET_STATE:             /* ����ƥ��������״̬ */
            switch (curr_state)
            {
            case AT_PARA_NUM_SUB_STATE:          /* ����ƥ�䷶Χ����״̬ */
                /* continue */
            case AT_PARA_ZERO_SUB_STATE:
                /* continue */
            case AT_PARA_HEX_NUM_SUB_STATE:

                /* ת�����ֿ��������ֲ����ű��б���Ͻ� */
                /* ���ֲ���������1*/
                g_stATParseCmd.astParaNumRange[g_stATParseCmd.ucParaNumRangeIndex++].ulBig = atRangeToU32(pucCopyPtr,pucCurrPtr);

                pgAtCheckFunc = atCheckNumPara;    /* ���ò����ȽϺ��� */

                break;

            case AT_PARA_NUM_STATE:
                /* continue */
            case AT_PARA_ZERO_STATE:
                /* continue */
            case AT_PARA_HEX_NUM_STATE:

                /* ת�����ֿ��������ֲ����ű��б���½� */
                g_stATParseCmd.astParaNumRange[g_stATParseCmd.ucParaNumRangeIndex].ulSmall = atRangeToU32(pucCopyPtr,pucCurrPtr);

                /* �Ͻ�����½�*/
                g_stATParseCmd.astParaNumRange[g_stATParseCmd.ucParaNumRangeIndex].ulBig = g_stATParseCmd.astParaNumRange[g_stATParseCmd.ucParaNumRangeIndex].ulSmall;

                /* ���ֲ���������1*/
                g_stATParseCmd.ucParaNumRangeIndex++;

                pgAtCheckFunc = atCheckNumPara;    /* ���ò����ȽϺ��� */

                break;

            case AT_PARA_LETTER_STATE:

                /* �������ַ������ű��б� */
                if ((VOS_UINT32)(pucCurrPtr - pucCopyPtr) < sizeof(g_stATParseCmd.auStrRange[g_stATParseCmd.ucParaStrRangeIndex]))
                {
                    atRangeCopy(g_stATParseCmd.auStrRange[g_stATParseCmd.ucParaStrRangeIndex],pucCopyPtr,pucCurrPtr);
                }
                else
                {
                    return AT_FAILURE;                  /* ���ش��� */
                }

                /* ���ò����ȽϺ��� */
                pgAtCheckFunc = At_CheckStringPara;

                break;

            case AT_PARA_NO_QUOT_LETTER_STATE:      /* ��˫���Ű�����ĸ״̬ */

                /* �������ַ������ű��б� */
                if ((VOS_UINT32)(pucCurrPtr - pucCopyPtr) < sizeof(g_stATParseCmd.auStrRange[g_stATParseCmd.ucParaStrRangeIndex]))
                {
                    atRangeCopy(g_stATParseCmd.auStrRange[g_stATParseCmd.ucParaStrRangeIndex],pucCopyPtr,pucCurrPtr);
                }
                else
                {
                    return AT_FAILURE;                  /* ���ش��� */
                }

                /* ���ò����ȽϺ��� */
                pgAtCheckFunc = atCheckNoQuotStringPara;

                break;

            default:    /* AT_PARA_RIGHT_QUOT_STATE */

                /* �������ַ������ű��б� */
                if ((VOS_UINT32)(pucCurrPtr - pucCopyPtr) < sizeof(g_stATParseCmd.auStrRange[g_stATParseCmd.ucParaStrRangeIndex]))
                {
                    atRangeCopy(g_stATParseCmd.auStrRange[g_stATParseCmd.ucParaStrRangeIndex++],pucCopyPtr,pucCurrPtr);
                }
                else
                {
                    return AT_FAILURE;                  /* ���ش��� */
                }

                /* ���ò����ȽϺ��� */
                pgAtCheckFunc = atCheckCharPara;

                break;
            }

            if(AT_FAILURE == atCmparePara())       /* �����Ƚ� */
            {
                return AT_FAILURE;                   /* ���ش��� */
            }
            break;

        case AT_BUTT_STATE:                     /* ��Ч״̬ */
            return AT_FAILURE;                  /* ���ش��� */

        default:
            break;
        }
        pucCurrPtr++;                               /*  ����������һ���ַ�*/
    }

    if(AT_NONE_STATE == new_state)                  /* ��ʼ״̬ */
    {
        return AT_FAILURE;                          /* ���ش��� */
    }

    return AT_SUCCESS;                          /*  ������ȷ*/
}




