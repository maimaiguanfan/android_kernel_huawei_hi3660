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
  1 Include HeadFile
*****************************************************************************/
#include  "vos.h"
#include  "msp_errno.h"
#include  "diag_fs_proc.h"
#include  "diag_common.h"
#include  "diag_debug.h"

#define    THIS_FILE_ID        MSP_FILE_ID_DIAG_FS_PROC_C

/*****************************************************************************
  2 Declare the Global Variable
*****************************************************************************/
const VOS_CHAR g_acDiagRootPath[] = "/";
const VOS_CHAR g_acDiagLockedRootPath[] = "/modem_log"; /* ����״̬�µĸ�Ŀ¼ */
const VOS_CHAR g_acDiagDumpDir[] = "/modem_log";
const VOS_CHAR g_acDiagLogDir[] = "/modem_log";

DIAG_FILE_INFO_STRU g_stDiagFileInfo = {DIAG_FILE_NULL, DIAG_FS_FOLDER_BUTT, VOS_NULL_PTR};

extern VOS_BOOL g_bAtDataLocked;
/*****************************************************************************
  3 Function
*****************************************************************************/
/*lint -e501 -esym(501,*)*/

VOS_VOID diag_FsClose(VOS_VOID)
{
    mm_segment_t old_fs;

    old_fs = get_fs();
    set_fs(KERNEL_DS);

    if (DIAG_FILE_NULL != g_stDiagFileInfo.lFile)
    {
        DIAG_FS_CLOSE(g_stDiagFileInfo.lFile);
        g_stDiagFileInfo.lFile = DIAG_FILE_NULL;
    }

    (VOS_VOID)DIAG_FS_FILE_SYNC(g_stDiagFileInfo.lFile);
    set_fs(old_fs);
}



VOS_UINT32  diag_FsGetDirInfo(VOS_CHAR *pDirName, VOS_UINT32 *pulTotalNum)
{
    VOS_INT32                 dir_handle =0;
    VOS_INT32                nRead =0;
    VOS_INT32                nCount =0;
    VOS_UINT32                i=0;
    VOS_UINT8*                buf =NULL;

    DIAG_DIRENT_STRU        *pstTmpDirent;
    mm_segment_t old_fs;

    old_fs = get_fs();
    set_fs(KERNEL_DS);

    /*��Ŀ¼*/
    if((dir_handle = DIAG_FS_OPENDIR((VOS_CHAR*)pDirName,DIAG_FS_RDONLY|DIAG_FS_DIRECTORY,0))< 0)
    {
        set_fs(old_fs);
        diag_printf( "[%s]DIAG_FS_OPENDIR error!",__FUNCTION__);
        return VOS_ERR;
    }

    buf =(VOS_UINT8*)VOS_MemAlloc(MSP_PID_DIAG_APP_AGENT, DYNAMIC_MEM_PT,MSP_DF_DIR_BUF_LEN);
    if(NULL == buf)
    {
        diag_printf( "[%s]Alloc mem error!",__FUNCTION__);
        DIAG_FS_CLOSEDIR(dir_handle);
        set_fs(old_fs);
        return VOS_ERR;
    }
    nRead = DIAG_FS_GETDENTS(dir_handle, (struct linux_dirent __user *)buf, MSP_DF_DIR_BUF_LEN);

    if(-1 == nRead)
    {
        diag_printf("[%s]dents error,nRead=%d!\n",__FUNCTION__,(VOS_INT)nRead);
        *pulTotalNum =0;
        VOS_MemFree(MSP_PID_DIAG_APP_AGENT, buf);
        DIAG_FS_CLOSEDIR(dir_handle);
        set_fs(old_fs);
        return VOS_ERR;
    }

    if(0 == nRead)
    {
        diag_printf("[%s]dents zero!\n",__FUNCTION__);
        *pulTotalNum = 0;
        VOS_MemFree(MSP_PID_DIAG_APP_AGENT, buf);
        DIAG_FS_CLOSEDIR(dir_handle);
        set_fs(old_fs);
        return VOS_OK;
    }

    for(i=0; i<(VOS_UINT32)nRead;)
    {
        pstTmpDirent = (DIAG_DIRENT_STRU*)(buf + i);
        i += pstTmpDirent->d_reclen;

        if((0 == VOS_StrCmp((char *) pstTmpDirent->d_name, "."))
            ||(0 == VOS_StrCmp ((char *) pstTmpDirent->d_name, "..")))
        {
            diag_printf("diag_FsGetDirInfo:d_name=%S!\n",pstTmpDirent->d_name);
            continue;
        }
        nCount++;
    }

    *pulTotalNum = nCount;

     /*�ر�Ŀ¼*/
    if (DIAG_FS_ERROR == DIAG_FS_CLOSEDIR(dir_handle))
    {
        VOS_MemFree(MSP_PID_DIAG_APP_AGENT, buf);
        diag_printf("[%s]DIAG_FS_CLOSEDIR zero!\n",__FUNCTION__);
        set_fs(old_fs);
        return VOS_ERR;
    }

    VOS_MemFree(MSP_PID_DIAG_APP_AGENT, buf);

    set_fs(old_fs);
    return VOS_OK;
}


VOS_UINT32 diag_FsGetItemInfo(VOS_CHAR *pDirectory,DIAG_DIR_FILE_INFO_STRU *pstDirFileInfo)
{
    DIAG_DIRENT_STRU        *pstTmpDirent;
    DIAG_STAT_STRU           stStat;
    VOS_UINT8                ucDirName[MSP_DF_DIR_MAX_LEN];
    VOS_INT32                dir_handle =0;
    VOS_INT32                nRead =0;
    VOS_INT32                i=0;
    VOS_UINT32               len;
    VOS_UINT8*               buf =NULL;
    mm_segment_t old_fs;

    old_fs = get_fs();
    set_fs(KERNEL_DS);

    /*�ٴδ�Ŀ¼*/
    if((dir_handle = DIAG_FS_OPENDIR(pDirectory,DIAG_FS_RDONLY|DIAG_FS_DIRECTORY,0))< 0)
    {
        diag_printf( "[%s]DIAG_FS_OPENDIR error!",__FUNCTION__);
        set_fs(old_fs);
        return VOS_ERR;

    }

    buf =(VOS_UINT8*)VOS_MemAlloc(MSP_PID_DIAG_APP_AGENT, DYNAMIC_MEM_PT,MSP_DF_DIR_BUF_LEN);
    if(NULL == buf)
    {
        diag_printf( "[%s]Alloc mem error!",__FUNCTION__);
        DIAG_FS_CLOSEDIR(dir_handle);
        set_fs(old_fs);
        return VOS_ERR;
    }
    nRead = DIAG_FS_GETDENTS(dir_handle, (struct linux_dirent __user *)buf, MSP_DF_DIR_BUF_LEN);
    if((-1 == nRead)||(0 == nRead))
    {
        diag_printf("[%s]dents error,nRead=%d!\n",__FUNCTION__,(VOS_INT)nRead);
        VOS_MemFree(MSP_PID_DIAG_APP_AGENT, buf);
        DIAG_FS_CLOSEDIR(dir_handle);
        set_fs(old_fs);
        return VOS_ERR;
    }

    /*��ѯ�ļ��н������ļ���������ȫ�ֱ���*/
    for(i=0; i<nRead; )
    {
        pstTmpDirent = (DIAG_DIRENT_STRU*)(buf + i);
        i += pstTmpDirent->d_reclen;

        if((0 == VOS_StrCmp((char *) pstTmpDirent->d_name, "."))
            ||(0 == VOS_StrCmp ((char *) pstTmpDirent->d_name, "..")))
        {
            diag_printf("[%s]:d_name=%S!\n",__FUNCTION__,pstTmpDirent->d_name);
            continue;
        }

        VOS_MemCpy_s(ucDirName, MSP_DF_DIR_MAX_LEN, 0, MSP_DF_DIR_MAX_LEN);
        (VOS_VOID)VOS_MemCpy_s(ucDirName, (MSP_DF_DIR_MAX_LEN-1), pDirectory, VOS_StrLen(pDirectory));
        
        len = VOS_StrLen(pDirectory);
        if(len >= (MSP_DF_DIR_MAX_LEN-1))
        {
            continue;
        }
        ucDirName[len] = '/';
        len++;
        
        if(len >= (MSP_DF_DIR_MAX_LEN-1))
        {
            continue;
        }
        
        (VOS_VOID)VOS_MemCpy_s((ucDirName+len), ((MSP_DF_DIR_MAX_LEN-1)-len), pstTmpDirent->d_name, VOS_StrLen(pstTmpDirent->d_name));

        /*ͨ��stat��ȡ�ļ����ļ��е���Ϣ*/
        if (DIAG_FS_ERROR == DIAG_FS_STAT((VOS_CHAR *)ucDirName, &stStat))
        {
            diag_printf("DIAG_FS_STAT: error!\n");
            pstDirFileInfo->ulItemType = DIAG_FS_ITEM_FILE;
            pstDirFileInfo->st_size    = 0;
            pstDirFileInfo->st_mode    = 0;
            pstDirFileInfo->st_atime   = 0;
            pstDirFileInfo->st_mtime   = 0;
            pstDirFileInfo->st_ctime   = 0;
        }
        else
        {
            /*Ŀ¼*/

            if (0 != (DIAG_IF_DIR&stStat.mode))
            {

                pstDirFileInfo->ulItemType = DIAG_FS_ITEM_FOLDER;
            }
            /*�ļ�*/
            else
            {
                pstDirFileInfo->ulItemType = DIAG_FS_ITEM_FILE;
            }
            pstDirFileInfo->st_size   = (stStat.size & 0xFFFFFFFF);/* Ŀǰ�ļ���С���ᳬ��32λ��С */
            pstDirFileInfo->st_atime  = stStat.atime.tv_sec;
            pstDirFileInfo->st_mtime  = stStat.mtime.tv_sec;
            pstDirFileInfo->st_ctime  = stStat.ctime.tv_sec;
            pstDirFileInfo->st_mode   = stStat.mode;
        }

        /*�ļ����ļ�����*/
        (VOS_VOID)VOS_MemSet_s(pstDirFileInfo->aucDirName,DIAG_CMD_FILE_NAME_LEN, 0,DIAG_CMD_FILE_NAME_LEN);
        (VOS_VOID)VOS_MemCpy_s(pstDirFileInfo->aucDirName, (DIAG_CMD_FILE_NAME_LEN-1), pstTmpDirent->d_name, VOS_StrLen(pstTmpDirent->d_name));

        pstDirFileInfo++;

     }

    VOS_MemFree(MSP_PID_DIAG_APP_AGENT, buf);
    DIAG_FS_CLOSEDIR(dir_handle);
    set_fs(old_fs);

    return VOS_OK;

}

VOS_INT32 diag_FsOpen(const VOS_CHAR *pcFileName, VOS_INT lFlag)
{
    VOS_INT32  fp = 0;
    mm_segment_t old_fs;

    old_fs = get_fs();
    set_fs(KERNEL_DS);

    fp = DIAG_FS_OPEN(pcFileName, lFlag,0755);
    diag_fs_log();

    set_fs(old_fs);

    return (VOS_INT32)fp;
}


VOS_UINT32 diag_FsQueryProc(VOS_UINT8* pstReq)
{
    DIAG_CMD_FS_QUERY_REQ* pstFsReq = NULL;
    DIAG_CMD_FS_QUERY_CNF stFsCnf = {0};
    VOS_UINT32 ret = ERR_MSP_SUCCESS;
    DIAG_FRAME_INFO_STRU *pstDiagHead = NULL;
    MSP_DIAG_CNF_INFO_STRU stDiagInfo = {0};

    pstDiagHead = (DIAG_FRAME_INFO_STRU*)(pstReq);

    pstFsReq = (DIAG_CMD_FS_QUERY_REQ*)(pstReq + DIAG_MESSAGE_DATA_HEADER_LEN);

    if (DIAG_FS_ROOT_FOLDER == pstFsReq->ulFolderType)
    {
        /* ����״̬�£���Ŀ¼������ */
        if(g_bAtDataLocked)
        {
            stFsCnf.ulLength = VOS_StrLen((VOS_CHAR*)g_acDiagLockedRootPath);
            (VOS_VOID)VOS_MemCpy_s(stFsCnf.aucDirPath,sizeof(stFsCnf.aucDirPath), g_acDiagLockedRootPath, stFsCnf.ulLength);
        }
        else
        {
            stFsCnf.ulLength = VOS_StrLen((VOS_CHAR*)g_acDiagRootPath);
            (VOS_VOID)VOS_MemCpy_s(stFsCnf.aucDirPath, sizeof(stFsCnf.aucDirPath), g_acDiagRootPath, stFsCnf.ulLength);
        }
        g_stDiagFileInfo.ulFileType = DIAG_FS_ROOT_FOLDER;

    }
    else if (DIAG_FS_LOG_FOLDER == pstFsReq->ulFolderType)
    {
        stFsCnf.ulLength = VOS_StrLen((VOS_CHAR*)g_acDiagLogDir);
        (VOS_VOID)VOS_MemCpy_s(stFsCnf.aucDirPath, sizeof(stFsCnf.aucDirPath), g_acDiagLogDir, stFsCnf.ulLength);
        g_stDiagFileInfo.ulFileType = DIAG_FS_LOG_FOLDER;
    }
    /*���������ļ����ڵ�·��*/
    else if (DIAG_FS_DUMP_FOLDER == pstFsReq->ulFolderType)
    {
        stFsCnf.ulLength = VOS_StrLen((VOS_CHAR*)g_acDiagDumpDir);
        (VOS_VOID)VOS_MemCpy_s(stFsCnf.aucDirPath, sizeof(stFsCnf.aucDirPath), g_acDiagDumpDir, stFsCnf.ulLength);
        g_stDiagFileInfo.ulFileType = DIAG_FS_DUMP_FOLDER;
    }
    else
    {
        ret = ERR_MSP_FAILURE;
    }
    stFsCnf.ulRet = ret;

    DIAG_MSG_COMMON_PROC(stDiagInfo, stFsCnf, pstDiagHead);

    stDiagInfo.ulMsgType    = DIAG_MSG_TYPE_BSP;

    return DIAG_MsgReport(&stDiagInfo, &stFsCnf, sizeof(stFsCnf));
}



VOS_UINT32 diag_FsScanProc(VOS_UINT8* pstReq)
{
    DIAG_CMD_FS_SCAN_DIR_REQ* pstFsReq = NULL;
    DIAG_CMD_FS_SCAN_DIR_CNF* pstFsCnf = NULL;
    VOS_UINT32 ret = ERR_MSP_SUCCESS;
    VOS_UINT32              ulTotalNum;
    VOS_UINT32              ulTotalSize;
    DIAG_FRAME_INFO_STRU *pstDiagHead = NULL;
    MSP_DIAG_CNF_INFO_STRU stDiagInfo = {0};

    pstDiagHead = (DIAG_FRAME_INFO_STRU*)(pstReq);

    pstFsReq = (DIAG_CMD_FS_SCAN_DIR_REQ*)(pstReq + DIAG_MESSAGE_DATA_HEADER_LEN);

   /*�õ�Ŀ¼�ڣ��ļ����ļ��е��ܸ������ܵ����ֳ���*/
    if(VOS_OK != diag_FsGetDirInfo(pstFsReq->szDirectory,&ulTotalNum))
    {
        return VOS_ERR;
    }

    /*���㷵�ظ����߲���Ϣ���ĳ���*/
    ulTotalSize = (ulTotalNum* sizeof(DIAG_DIR_FILE_INFO_STRU)+ sizeof(DIAG_CMD_FS_SCAN_DIR_CNF));

    pstFsCnf = (DIAG_CMD_FS_SCAN_DIR_CNF*)VOS_MemAlloc(MSP_PID_DIAG_APP_AGENT, DYNAMIC_MEM_PT, ulTotalSize);
    if (VOS_NULL_PTR == pstFsCnf)
    {
        return VOS_ERR;
    }

    /*��ȡÿ���ļ����ļ��е���Ϣ*/
    if(ulTotalNum>0)
    {
        ret = diag_FsGetItemInfo(pstFsReq->szDirectory,pstFsCnf->stDirInfo);
    }

    pstFsCnf->ulRet = ret;
    pstFsCnf->ulDirNum= ulTotalNum;
    pstFsCnf->ulDirInfoLen= ulTotalNum * sizeof(DIAG_DIR_FILE_INFO_STRU);

    DIAG_MSG_COMMON_PROC(stDiagInfo, (*pstFsCnf), pstDiagHead);

    stDiagInfo.ulMsgType    = DIAG_MSG_TYPE_BSP;

    ret = DIAG_MsgReport(&stDiagInfo, pstFsCnf, ulTotalSize);

    VOS_MemFree(MSP_PID_DIAG_APP_AGENT, pstFsCnf);

    return ret;

}


VOS_UINT32 diag_FsMkdirProc(VOS_UINT8* pstReq)
{
    DIAG_CMD_FS_MAKE_DIR_REQ* pstFsReq = NULL;
    DIAG_CMD_FS_MAKE_DIR_CNF stFsCnf = {0};
    VOS_UINT32 ret = ERR_MSP_SUCCESS;
    DIAG_FRAME_INFO_STRU *pstDiagHead = NULL;
    MSP_DIAG_CNF_INFO_STRU stDiagInfo = {0};

    mm_segment_t old_fs;

    pstDiagHead = (DIAG_FRAME_INFO_STRU*)(pstReq);

    pstFsReq = (DIAG_CMD_FS_MAKE_DIR_REQ*)(pstReq + DIAG_MESSAGE_DATA_HEADER_LEN);

    old_fs = get_fs();
    set_fs(KERNEL_DS);

    ret = (VOS_UINT32)DIAG_FS_MKDIR((VOS_CHAR*)pstFsReq->szDirectory,0755);

    stFsCnf.ulRet = ret;

    set_fs(old_fs);

    DIAG_MSG_COMMON_PROC(stDiagInfo, stFsCnf, pstDiagHead);

    stDiagInfo.ulMsgType    = DIAG_MSG_TYPE_BSP;

    return DIAG_MsgReport(&stDiagInfo, &stFsCnf, sizeof(stFsCnf));
}


VOS_UINT32 diag_FsOpenProc(VOS_UINT8* pstReq)
{
    DIAG_CMD_FS_OPEN_REQ* pstFsReq = NULL;
    DIAG_CMD_FS_OPEN_CNF stFsCnf = {0};
    VOS_UINT32 ret = ERR_MSP_SUCCESS;
    DIAG_FRAME_INFO_STRU *pstDiagHead = NULL;
    MSP_DIAG_CNF_INFO_STRU stDiagInfo = {0};

    pstDiagHead = (DIAG_FRAME_INFO_STRU*)(pstReq);

    pstFsReq = (DIAG_CMD_FS_OPEN_REQ*)(pstReq + DIAG_MESSAGE_DATA_HEADER_LEN);

    /*��֧�ֲ����ļ����������*/
    if (DIAG_FILE_NULL != g_stDiagFileInfo.lFile)
    {
        /* �ļ����������п����ļ�size�������ӣ�������һ���ļ�û���깤�߾Ϳ�ʼ�µ��ļ����� */
        diag_FsClose();
    }
    diag_fs_log();
    g_stDiagFileInfo.lFile = diag_FsOpen(pstFsReq->szDirectory, (VOS_INT)(pstFsReq->ulMode));

    if (g_stDiagFileInfo.lFile < 0)
    {
        g_stDiagFileInfo.lFile = DIAG_FILE_NULL;
        ret = VOS_ERR;
    }
    else
    {
        diag_fs_log();
    }
    stFsCnf.ulRet = ret;

    DIAG_MSG_COMMON_PROC(stDiagInfo, stFsCnf, pstDiagHead);

    stDiagInfo.ulMsgType    = DIAG_MSG_TYPE_BSP;

    diag_fs_log();

    return DIAG_MsgReport(&stDiagInfo, &stFsCnf, sizeof(stFsCnf));
}


VOS_UINT32 diag_FsImportProc(VOS_UINT8* pstReq)
{
    DIAG_CMD_FS_IMPORT_REQ* pstFsReq = NULL;
    DIAG_CMD_FS_IMPORT_CNF stFsCnf = {0};
    VOS_UINT32 ret = ERR_MSP_SUCCESS;
    VOS_INT32 lResult =0;
    mm_segment_t old_fs;
    DIAG_FRAME_INFO_STRU *pstDiagHead = NULL;
    MSP_DIAG_CNF_INFO_STRU stDiagInfo = {0};

    pstDiagHead = (DIAG_FRAME_INFO_STRU*)(pstReq);

    pstFsReq = (DIAG_CMD_FS_IMPORT_REQ*)(pstReq + DIAG_MESSAGE_DATA_HEADER_LEN);

    if (DIAG_FILE_NULL == g_stDiagFileInfo.lFile)
    {
        return ERR_MSP_FAILURE;
    }
    diag_fs_log();

    /*д�ļ���ϣ��ر��ļ�*/
    if (0 == pstFsReq->ulSize)
    {
        diag_fs_log();
        diag_FsClose();
        /*����ظ���FW*/
        stFsCnf.ulRet = ERR_MSP_SUCCESS;

        DIAG_MSG_COMMON_PROC(stDiagInfo, stFsCnf, pstDiagHead);

        stDiagInfo.ulMsgType    = DIAG_MSG_TYPE_BSP;

        return DIAG_MsgReport(&stDiagInfo, &stFsCnf, sizeof(stFsCnf));
    }

    old_fs = get_fs();
    set_fs(KERNEL_DS);
    lResult = DIAG_FS_WRITE(g_stDiagFileInfo.lFile, pstFsReq->acContent, pstFsReq->ulSize);

    set_fs(old_fs);

    if ((DIAG_FS_ERROR == lResult)||(lResult != (VOS_INT32)(pstFsReq->ulSize)))
    {
        diag_fs_log();
        diag_printf("[%s]!,lResult=%d\n",__FUNCTION__,lResult);
        diag_FsClose();
        return VOS_ERR;
    }

    stFsCnf.ulRet = ret;

    DIAG_MSG_COMMON_PROC(stDiagInfo, stFsCnf, pstDiagHead);

    stDiagInfo.ulMsgType    = DIAG_MSG_TYPE_BSP;

    return DIAG_MsgReport(&stDiagInfo, &stFsCnf, sizeof(stFsCnf));
}


VOS_UINT32 diag_FsExportProc(VOS_UINT8* pstReq)
{
    DIAG_CMD_FS_EXPORT_CNF *pstFsCnf = NULL;
    VOS_UINT32 ret = ERR_MSP_SUCCESS;
    VOS_UINT32 ulReadSize = 0;
    mm_segment_t old_fs;
    DIAG_FRAME_INFO_STRU *pstDiagHead = NULL;
    MSP_DIAG_CNF_INFO_STRU stDiagInfo = {0};

    pstDiagHead = (DIAG_FRAME_INFO_STRU*)(pstReq);

    if (DIAG_FILE_NULL == g_stDiagFileInfo.lFile)
    {
        diag_fs_log();
         return ERR_MSP_FAILURE;
    }


    pstFsCnf = (DIAG_CMD_FS_EXPORT_CNF*)VOS_MemAlloc(MSP_PID_DIAG_APP_AGENT, DYNAMIC_MEM_PT, sizeof(DIAG_CMD_FS_EXPORT_CNF)+DIAG_TRANSFER_BLOCK_MAX_SIZE);
    if (VOS_NULL_PTR == pstFsCnf)
    {

        diag_fs_log();
        diag_FsClose();
        return ERR_MSP_FAILURE;

    }

    old_fs = get_fs();
    set_fs(KERNEL_DS);

    ulReadSize = (VOS_UINT32)DIAG_FS_READ(g_stDiagFileInfo.lFile,(VOS_CHAR*)pstFsCnf + sizeof(DIAG_CMD_FS_EXPORT_CNF), DIAG_TRANSFER_BLOCK_MAX_SIZE);

    set_fs(old_fs);

    /*��ȡ�ļ����ִ���*/
    if (DIAG_FS_ERROR == (VOS_INT32)ulReadSize)
    {
        diag_fs_log();
        VOS_MemFree(MSP_PID_DIAG_APP_AGENT, pstFsCnf);
        diag_FsClose();
        return ERR_MSP_FAILURE;
    }

    /*�����Ѿ�û�����ݿ��Զ�ȡ���ļ�����ȫ������*/
    if (0 == ulReadSize)
    {
        diag_fs_log();
        diag_FsClose();
    }
    else
    {
        diag_fs_log();
    }

    pstFsCnf->ulRet = ret;
    pstFsCnf->ulSize = ulReadSize;

    DIAG_MSG_COMMON_PROC(stDiagInfo, (*pstFsCnf), pstDiagHead);

    stDiagInfo.ulMsgType  = DIAG_MSG_TYPE_BSP;

    ret = DIAG_MsgReport(&stDiagInfo, pstFsCnf, (sizeof(DIAG_CMD_FS_EXPORT_CNF) + ulReadSize));

    VOS_MemFree(MSP_PID_DIAG_APP_AGENT, pstFsCnf);
    diag_fs_log();
    return ret;
}


VOS_UINT32 diag_FsDeleteProc(VOS_UINT8* pstReq)
{
    DIAG_CMD_FS_DELETE_REQ* pstFsReq = NULL;
    DIAG_CMD_FS_DELETE_CNF stFsCnf = {0};
    VOS_UINT32 ret = ERR_MSP_SUCCESS;
    mm_segment_t old_fs;
    DIAG_FRAME_INFO_STRU *pstDiagHead = NULL;
    MSP_DIAG_CNF_INFO_STRU stDiagInfo = {0};

    pstDiagHead = (DIAG_FRAME_INFO_STRU*)(pstReq);

    pstFsReq = (DIAG_CMD_FS_DELETE_REQ*)(pstReq + DIAG_MESSAGE_DATA_HEADER_LEN);

    old_fs = get_fs();
    set_fs(KERNEL_DS);

    if (DIAG_FS_ITEM_FOLDER == pstFsReq->ulItemType)
    {
        ret = (VOS_UINT32)DIAG_FS_RMDIR(pstFsReq->szDirectory);
    }
    else if (DIAG_FS_ITEM_FILE == pstFsReq->ulItemType)
    {
        ret = (VOS_UINT32)DIAG_FS_RMFILE(pstFsReq->szDirectory);
    }

    set_fs(old_fs);

    stFsCnf.ulRet = ret;

    DIAG_MSG_COMMON_PROC(stDiagInfo, stFsCnf, pstDiagHead);

    stDiagInfo.ulMsgType  = DIAG_MSG_TYPE_BSP;

    return DIAG_MsgReport(&stDiagInfo, &stFsCnf, sizeof(stFsCnf));
}


VOS_UINT32 diag_FsSpaceProc(VOS_UINT8* pstReq)
{
    DIAG_CMD_FS_SPACE_REQ* pstFsReq = NULL;
    DIAG_CMD_FS_SPACE_CNF stFsCnf = {0};
    VOS_UINT32 ret = ERR_MSP_SUCCESS;
    mm_segment_t old_fs;
    DIAG_FRAME_INFO_STRU *pstDiagHead = NULL;
    MSP_DIAG_CNF_INFO_STRU stDiagInfo = {0};

    pstDiagHead = (DIAG_FRAME_INFO_STRU*)(pstReq);

    pstFsReq = (DIAG_CMD_FS_SPACE_REQ*)(pstReq + DIAG_MESSAGE_DATA_HEADER_LEN);

    old_fs = get_fs();
    set_fs(KERNEL_DS);

    ret = (VOS_UINT32)DRV_FILE_GET_DISKSPACE((VOS_CHAR*)(pstFsReq->szDirectory),(VOS_UINT*)&stFsCnf.ulDiskSpace,
                    (VOS_UINT*)&stFsCnf.ulUsedSpace,(VOS_UINT*)&stFsCnf.ulValidSpace);

    stFsCnf.ulRet = ret;
    set_fs(old_fs);

    DIAG_MSG_COMMON_PROC(stDiagInfo, stFsCnf, pstDiagHead);

    stDiagInfo.ulMsgType  = DIAG_MSG_TYPE_BSP;

    return DIAG_MsgReport(&stDiagInfo, &stFsCnf, sizeof(stFsCnf));
}
/*lint -e501 +esym(501,*)*/
VOS_VOID diag_fsInit(VOS_VOID)
{
    mdrv_hds_cmd_register(DIAG_CMD_FS_QUERY_DIR, (bsp_hds_func)diag_FsQueryProc);
    mdrv_hds_cmd_register(DIAG_CMD_FS_SCAN_DIR, (bsp_hds_func)diag_FsScanProc);
    mdrv_hds_cmd_register(DIAG_CMD_FS_MAKE_DIR, (bsp_hds_func)diag_FsMkdirProc);
    mdrv_hds_cmd_register(DIAG_CMD_FS_OPEN, (bsp_hds_func)diag_FsOpenProc);
    mdrv_hds_cmd_register(DIAG_CMD_FS_IMPORT, (bsp_hds_func)diag_FsImportProc);
    mdrv_hds_cmd_register(DIAG_CMD_FS_EXPORT, (bsp_hds_func)diag_FsExportProc);
    mdrv_hds_cmd_register(DIAG_CMD_FS_DELETE, (bsp_hds_func)diag_FsDeleteProc);
    mdrv_hds_cmd_register(DIAG_CMD_FS_SPACE, (bsp_hds_func)diag_FsSpaceProc);
}



