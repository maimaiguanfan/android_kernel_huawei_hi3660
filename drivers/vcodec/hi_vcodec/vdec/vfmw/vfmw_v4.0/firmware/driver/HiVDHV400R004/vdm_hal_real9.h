
/***********************************************************************
*
* Copyright (c) 2010 Hisilicon - All Rights Reserved
*
* File: $vdm_hal_real9.h$
* Date: $2010/06/09$
* Revision: $v1.0$
* Purpose: HiVDHV100 IP REAL9 VDM HAL Module.
*
* Change History:
*
* Date       Author            Change
* ======     ======            ======
* 20100609   q45134            Original
*
************************************************************************/

#ifndef __VDM_HAL_REAL9_HEADER__
#define __VDM_HAL_REAL9_HEADER__


#include "basedef.h"

// REG & MSG START
//�Ĵ�����
typedef struct
{
    USIGN dec_start:                            1;
    USIGN reserved:                             31;
} RV9_VDH_START;

typedef struct
{
    USIGN repair_start:                         1;
    USIGN reserved:                             31;
} RV9_VDH_REPAIR;

typedef struct
{
    USIGN mbamt_to_dec:                         20;
    USIGN memory_clock_gating_en:               1;
    USIGN module_clock_gating_en:               1;
    USIGN marker_bit_detect_en:                 1;
    USIGN ac_last_detect_en:                    1;
    USIGN coef_idx_detect_en:                   1;
    USIGN vop_type_detect_en:                   1;
    USIGN reserved:                             4;
    USIGN ld_qmatrix_flag:                      1;
    USIGN sec_mode_en:                          1;
} RV9_BASIC_CFG0;

typedef struct
{
    USIGN video_standard:                       4;
    USIGN ddr_stride:                           9;
    USIGN uv_order_en:							1;
    USIGN fst_slc_grp:                          1;
    USIGN mv_output_en:                         1;
    USIGN max_slcgrp_num:                       12;
    USIGN line_num_output_en:                   1;
    USIGN vdh_2d_en:                           	1;
    USIGN compress_en:                          1;
    USIGN ppfd_en:                              1;
} RV9_BASIC_CFG1;

typedef struct
{
    USIGN av_msg_addr:                          32;
} RV9_AVM_ADDR;

typedef struct
{
    USIGN va_msg_addr:                          32;
} RV9_VAM_ADDR;

typedef struct
{
    USIGN stream_base_addr:                     32;
} RV9_STREAM_BASE_ADDR;

typedef struct
{
    USIGN decoded_slice_num:                    17;
    USIGN intrs_vdh_dec_over:                   1;
    USIGN intrs_vdh_dec_err:                    1;
    USIGN intrs_rpr_dec_over:                   1;
    USIGN vdh_rst_busy:                         1;
    USIGN reserved:                             3;
    USIGN version_id:                           8;
} RV9_VDH_STATE;

typedef struct
{
    USIGN ref_pic_type_0:               2;
    USIGN ref_pic_type_1:               2;
    USIGN ref_pic_type_2:               2;
    USIGN ref_pic_type_3:               2;
    USIGN ref_pic_type_4:               2;
    USIGN ref_pic_type_5:               2;
    USIGN ref_pic_type_6:               2;
    USIGN ref_pic_type_7:               2;

    USIGN ref_pic_type_8:               2;
    USIGN ref_pic_type_9:               2;
    USIGN ref_pic_type_10:               2;
    USIGN ref_pic_type_11:               2;
    USIGN ref_pic_type_12:               2;
    USIGN ref_pic_type_13:               2;
    USIGN ref_pic_type_14:               2;
    USIGN ref_pic_type_15:               2;
} RV9_REF_PIC_TYPE;

typedef struct
{
    USIGN ff_apt_en:                    1;
    USIGN reserved:                     31;
} RV9_FF_APT_EN;

typedef struct
{
    USIGN rpr_start:                            1;
    USIGN reserved:                             31;
} RV9_RPR_START;

typedef struct
{
    USIGN ystaddr_1d:                           32;
} RV9_RPR_SRC_YSTADDR_1D;

typedef struct
{
    USIGN ystride_1d:                           16;
    USIGN reserved:                             16;
} RV9_RPR_SRC_YSTRIDE_1D;

typedef struct
{
    USIGN uvoffset_1d:                          32;
} RV9_RPR_SRC_UVOFFSET_1D;

typedef struct
{
    USIGN pic_width_in_mb:                      9;
    USIGN reserved0:                            7;
    USIGN pic_height_in_mb:                     9;
    USIGN reserved1:                            7;
} RV9_RPR_SRC_WIDTH_HEIGHT;

typedef struct
{
    USIGN ystaddr_1d:                           32;
} RV9_RPR_DST_YSTADDR_1D;

typedef struct
{
    USIGN ystride_1d:                           16;
    USIGN reserved:                             16;
} RV9_RPR_DST_YSTRIDE_1D;

typedef struct
{
    USIGN uvoffset_1d:                          32;
} RV9_RPR_DST_UVOFFSET_1D;

typedef struct
{
    USIGN pic_width_in_mb:                      9;
    USIGN reserved0:                            7;
    USIGN pic_height_in_mb:                     9;
    USIGN reserved1:                            7;
} RV9_RPR_DST_WIDTH_HEIGHT;


//PIC ������Ϣ��
typedef struct
{
    USIGN pic_coding_type:                      2;
    USIGN reserved:                             30;
} RV9_PICDNMSG_D0;

typedef struct
{
    USIGN pic_width_in_mb:                      9;
    USIGN reserved0:                            7;
    USIGN pic_height_in_mb:                     9;
    USIGN reserved1:                            7;
} RV9_PICDNMSG_D1;

typedef struct
{
    USIGN Ratio0:                               16;
    USIGN Ratio1:                               16;
} RV9_PICDNMSG_D2;

typedef struct
{
    USIGN rounding:                             1;   //no use
    USIGN rv_deblocking_strength:               3;   //no use
    USIGN reserved:                             28;
} RV9_PICDNMSG_D3;

typedef struct
{
    USIGN currpicqp:                            5;
    USIGN reserved:                             27;
} RV9_PICDNMSG_D4;

typedef struct
{
    USIGN prevpicqp:                            5;
    USIGN reserved:                             27;
} RV9_PICDNMSG_D5;

typedef struct
{
    USIGN prevpicmb0qp:                         5;
    USIGN LesoreqQcif:                          1;
    USIGN reserved:                             26;
} RV9_PICDNMSG_D6;

typedef struct
{
    USIGN curr_rcn_addr:                        32;
} RV9_PICDNMSG_D16;

typedef struct
{
    USIGN fwd_ref_addr:                         32;
} RV9_PICDNMSG_D17;

typedef struct
{
    USIGN bwd_ref_addr:                         32;
} RV9_PICDNMSG_D18;

typedef struct
{
    USIGN curr_colmb_addr:                      32;
} RV9_PICDNMSG_D19;

typedef struct
{
    USIGN bwd_colmb_addr:                       32;
} RV9_PICDNMSG_D20;

typedef struct
{
    USIGN sed_top_addr:                         32;
} RV9_PICDNMSG_D21;

typedef struct
{
    USIGN pmv_top_addr:                         32;
} RV9_PICDNMSG_D22;

typedef struct
{
    USIGN rcn_top_addr:                         32;
} RV9_PICDNMSG_D23;

typedef struct
{
    USIGN dblk_top_addr:                        32;
} RV9_PICDNMSG_D24;
typedef struct
{
    USIGN next_slice_para_addr:                 32;
} RV9_PICDNMSG_D63;


//SLICE ������Ϣ��
typedef struct
{
    USIGN bit_len_0:                            24;
    USIGN bit_offset_0:                         7;
    USIGN reserved:                             1;
} RV9_SLCDNMSG_D0;

typedef struct
{
    USIGN bit_stream_addr_0:                    24;
    USIGN reserved:                             8;
} RV9_SLCDNMSG_D1;

typedef struct
{
    USIGN bit_len_1:                            24;
    USIGN bit_offset_1:                         7;
    USIGN reserved:                             1;
} RV9_SLCDNMSG_D2;

typedef struct
{
    USIGN bit_stream_addr_1:                    24;
    USIGN reserved:                             8;
} RV9_SLCDNMSG_D3;

typedef struct
{
    USIGN reserved0:                            16;
    USIGN dblk_filter_passthrough:              1;
    USIGN osvquant:                             2;
    USIGN sliceqp:                              5;
    USIGN reserved1:                            8;
} RV9_SLCDNMSG_D4;

typedef struct
{
    USIGN slice_start_mbn:                      16;
    USIGN slice_end_mbn:                        16;
} RV9_SLCDNMSG_D5;

typedef struct
{
    USIGN next_slice_para_addr:                 32;
} RV9_SLCDNMSG_D63;

//������Ϣ��
typedef struct
{
    USIGN error_mb_num:                         16;
    USIGN error_flag:                           1;
    USIGN endofvop_flag:                        1;
    USIGN pic_coding_type:                      2;
    USIGN reserved:                             1;
    USIGN error_mode:                           11;
} RV9_UPMSG_D0;

typedef struct
{
    USIGN slice_start_mb_num:                   16;
    USIGN slice_end_mb_num:                     16;
} RV9_UPMSG_D1;


//PIC��Ϣ�ؽṹ��
typedef struct
{
    RV9_PICDNMSG_D0  *d0;
    RV9_PICDNMSG_D1  *d1;
    RV9_PICDNMSG_D2  *d2;
    RV9_PICDNMSG_D3  *d3;
    RV9_PICDNMSG_D4  *d4;
    RV9_PICDNMSG_D5  *d5;
    RV9_PICDNMSG_D6  *d6;

    RV9_PICDNMSG_D16 *d16;
    RV9_PICDNMSG_D17 *d17;
    RV9_PICDNMSG_D18 *d18;
    RV9_PICDNMSG_D19 *d19;
    RV9_PICDNMSG_D20 *d20;
    RV9_PICDNMSG_D21 *d21;
    RV9_PICDNMSG_D22 *d22;
    RV9_PICDNMSG_D23 *d23;
    RV9_PICDNMSG_D24 *d24;

    RV9_PICDNMSG_D63 *d63;
} RV9_PICDNMSG_S;


//SLICE��Ϣ�ؽṹ��
typedef struct
{
    RV9_SLCDNMSG_D0  *d0;
    RV9_SLCDNMSG_D1  *d1;
    RV9_SLCDNMSG_D2  *d2;
    RV9_SLCDNMSG_D3  *d3;
    RV9_SLCDNMSG_D4  *d4;
    RV9_SLCDNMSG_D5  *d5;

    RV9_SLCDNMSG_D63 *d63;
} RV9_SLCDNMSG_S;


#ifdef __cplusplus
extern "C" {
#endif

SINT32 RV9HAL_InitHal( VOID );
SINT32 RV9HAL_StartDec( RV9_DEC_PARAM_S *pRv9DecParam, SINT32 VdhId );

#ifdef __cplusplus
}
#endif


#endif //__VDM_HAL_REAL9_H__
