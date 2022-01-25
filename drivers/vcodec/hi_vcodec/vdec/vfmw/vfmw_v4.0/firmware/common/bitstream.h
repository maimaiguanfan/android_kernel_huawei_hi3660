/*--------------------------------------------------------------------------------------------------------------------------*/
/*!!Warning: This is a key information asset of Huawei Tech Co.,Ltd                                                         */
/*CODEMARK:kOyQZYzjDpyGdBAEC2GaWinjiDDUykL9e8pckESWBbMVmSWkBuyJO01cTiy3TdzKxGk0oBQa
mSMf7J4FkTpfv/JzwTgKEHe/9N4yU5rqhEDlG50G/Vcvyq2txYFjqt/X5GLe/SnYbE4r/A06
+TaRlx2W5uih7dpOjmsh0QFtZgPFr+yOxvAZYtV5MKEJUCQSwV8d5Ulijbg3dGznOyZ1Jj3t
S5E4wdXbruKx98lAKSo+GonUA4k41zkvmpO8qhDLOzhUTMyz8/8ayzvOtS1hsQ==*/
/*--------------------------------------------------------------------------------------------------------------------------*/

/***********************************************************************
*
* Copyright (c) 2006 HUAWEI - All Rights Reserved
*
* File: $bitstream.h$
* Date: $2006/11/30$
* Revision: $v1.0$
* Purpose: a general bit stream manipulation tool, 
*          can be adopted by multi video standards.
*
* Change History:
*
* Date             Author            Change
* ====             ======            ====== 
* 2006/11/30       z56361            Original
*
* Dependencies:
*
************************************************************************/

#ifndef __BITSTREAM_HEAD__
#define	__BITSTREAM_HEAD__

#ifdef __cplusplus
extern "C" {
#endif

#include "basedef.h"


#define  REVERSE_ENDIAN32( x )						\
		( (x)<<24 ) |						\
		( ( (x) & 0x0000ff00 ) << 8 ) |		\
		( ( (x) & 0x00ff0000 ) >> 8 ) |		\
		( ( (x) >> 24 ) & 0x000000ff ) 

typedef struct _tag_BITSTREAM
{
	UINT8  *pHead;
	UINT8  *pTail;
	UINT32  Bufa;
	UINT32  Bufb;
	SINT32  BsLen;
	SINT32  BufPos;
	SINT32  TotalPos;
    SINT32  HistoryPos;
}BS, *LPBS;


VOID   BsInit( BS *pBS, UINT8 *pInput, SINT32 length );
SINT32  BsGet( BS *pBS, SINT32 nBits );
SINT32  BsShow( BS *pBS, SINT32 nBits );
SINT32  BsPos( BS *pBS );
SINT32  BsSkip( BS *pBS, SINT32 nBits );
SINT32  BsBack( BS *pBS, SINT32 nBits );
SINT32  BsToNextByte( BS *pBS );
SINT32  BsBitsToNextByte( BS *pBS );
SINT32  BsResidBits( BS *pBS );
SINT32  BsIsByteAligned( BS *pBS );
SINT32  BsNextBitsByteAligned( BS *pBS, SINT32 nBits );
SINT32  BsLongSkip(BS *pBS, SINT32 nBits);
SINT32  BsSkipWithoutCount( BS *pBS, SINT32 nBits );
UINT8*  BsGetNextBytePtr( BS *pBS );
UINT32 ZerosMS_32(UINT32 data);


#ifdef __cplusplus
}
#endif

#endif

