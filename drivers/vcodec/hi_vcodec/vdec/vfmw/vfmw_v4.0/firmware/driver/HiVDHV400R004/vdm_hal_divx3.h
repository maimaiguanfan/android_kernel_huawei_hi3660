
/***********************************************************************
*
* Copyright (c) 2012 Hisilicon - All Rights Reserved
*
* File: $vdm_hal_divx3.h$
* Date: $2012/01/06$
* Revision: $v1.0$
* Purpose: HiVDHV100 IP DIVX3 VDM HAL Module.
*
* Change History:
*
* Date       Author            Change
* ======     ======            ======
* 20120106   q45134            Original
*
************************************************************************/

#ifndef __VDM_HAL_DIVX3_HEADER__
#define __VDM_HAL_DIVX3_HEADER__


#include "basedef.h"

// REG & MSG START
//�Ĵ�����
typedef struct
{
    USIGN dec_start:                            1;
    USIGN reserved:                             31;
} DIVX3_VDH_START;

typedef struct
{
    USIGN repair_start:                         1;
    USIGN reserved:                             31;
} DIVX3_VDH_REPAIR;

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
} DIVX3_BASIC_CFG0;

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
} DIVX3_BASIC_CFG1;

typedef struct
{
    USIGN av_msg_addr:                          32;
} DIVX3_AVM_ADDR;

typedef struct
{
    USIGN va_msg_addr:                          32;
} DIVX3_VAM_ADDR;

typedef struct
{
    USIGN stream_base_addr:                     32;
} DIVX3_STREAM_BASE_ADDR;

typedef struct
{
    USIGN decoded_slice_num:                    17;
    USIGN intrs_vdh_dec_over:                   1;
    USIGN intrs_vdh_dec_err:                    1;
    USIGN intrs_rpr_dec_over:                   1;
    USIGN vdh_rst_busy:                         1;
    USIGN reserved:                             3;
    USIGN version_id:                           8;
} DIVX3_VDH_STATE;

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
} DIVX3_REF_PIC_TYPE;

typedef struct
{
    USIGN ff_apt_en:                    1;
    USIGN reserved:                     31;
} DIVX3_FF_APT_EN;

typedef struct
{
    USIGN rpr_start:                            1;
    USIGN reserved:                             31;
} DIVX3_RPR_START;

//PIC ������Ϣ��
typedef struct
{
    USIGN pic_coding_type:                      1;
    USIGN reserved:                             31;
} DIVX3_PICDNMSG_D0;

typedef struct
{
    USIGN pic_width_in_mb:                      9;
    USIGN reserved0:                            7;
    USIGN pic_height_in_mb:                     9;
    USIGN reserved1:                            7;
} DIVX3_PICDNMSG_D1;

typedef struct
{
    USIGN quantizer:                            5;
    USIGN rounding_type:                        1;
    USIGN has_skip:                             1;
    USIGN reserved:                             25;
} DIVX3_PICDNMSG_D2;

typedef struct
{
    USIGN alternative_I_AC_chrom_DCT:           2;
    USIGN alternative_I_AC_lum_DCT:             2;
    USIGN alternative_I_DC_DCT:                 1;
    USIGN alternative_P_AC_DCT:                 2;
    USIGN alternative_P_DC_DCT:                 1;
    USIGN alternative_MV:                       1;
    USIGN reserved:                             23;
} DIVX3_PICDNMSG_D3;


typedef struct
{
    USIGN bit_stream_address_0:                 32;
} DIVX3_PICDNMSG_D16;

typedef struct
{
    USIGN bit_offset_0:                         7;
    USIGN reserved:                             25;
} DIVX3_PICDNMSG_D17;

typedef struct
{
    USIGN bit_len_0:                            32;
} DIVX3_PICDNMSG_D18;

typedef struct
{
    USIGN bit_stream_address_1:                 32;
} DIVX3_PICDNMSG_D19;

typedef struct
{
    USIGN bit_offset_1:                         7;
    USIGN reserved:                             25;
} DIVX3_PICDNMSG_D20;

typedef struct
{
    USIGN bit_len_1:                            32;
} DIVX3_PICDNMSG_D21;



typedef struct
{
    USIGN curr_rcn_addr:                        32;
} DIVX3_PICDNMSG_D32;

typedef struct
{
    USIGN fwd_ref_addr:                         32;
} DIVX3_PICDNMSG_D33;

typedef struct
{
    USIGN sed_top_addr:                         32;
} DIVX3_PICDNMSG_D34;

typedef struct
{
    USIGN pmv_top_addr:                         32;
} DIVX3_PICDNMSG_D35;

typedef struct
{
    USIGN it_top_addr:                          32;
} DIVX3_PICDNMSG_D36;

typedef struct
{
    USIGN pmv_colmb_addr:                       32;
} DIVX3_PICDNMSG_D37;

typedef struct
{
    USIGN next_slice_para_addr:                 32;
} DIVX3_PICDNMSG_D63;


//SLICE ������Ϣ��
typedef struct
{
    USIGN slice_start_mbn:                      20;
    USIGN reserved:                             12;
} DIVX3_SLCDNMSG_D0;

typedef struct
{
    USIGN slice_end_mbn:                        20;
    USIGN reserved:                             12;
} DIVX3_SLCDNMSG_D1;

typedef struct
{
    USIGN next_slice_para_addr:                 32;
} DIVX3_SLCDNMSG_D63;


//������Ϣ��
typedef struct
{
    USIGN error_mb_num:                         16;
    USIGN error_flag:                           1;
    USIGN endofvop_flag:                        1;
    USIGN pic_coding_type:                      2;
    USIGN reserved:                             1;
    USIGN error_mode:                           11;
} DIVX3_UPMSG_D0;

typedef struct
{
    USIGN slice0_start_mb_num:                  20;
    USIGN reserved:                             12;
} DIVX3_UPMSG_D1;

typedef struct
{
    USIGN slice0_end_mb_num:                    20;
    USIGN reserved:                             12;
} DIVX3_UPMSG_D2;

typedef struct
{
    USIGN reserved:                             32;
} DIVX3_UPMSG_D3;

//PIC��Ϣ�ؽṹ��
typedef struct
{
    DIVX3_PICDNMSG_D0  *d0;
    DIVX3_PICDNMSG_D1  *d1;
    DIVX3_PICDNMSG_D2  *d2;
    DIVX3_PICDNMSG_D3  *d3;

    DIVX3_PICDNMSG_D16 *d16;
    DIVX3_PICDNMSG_D17 *d17;
    DIVX3_PICDNMSG_D18 *d18;
    DIVX3_PICDNMSG_D19 *d19;
    DIVX3_PICDNMSG_D20 *d20;
    DIVX3_PICDNMSG_D21 *d21;

    DIVX3_PICDNMSG_D32 *d32;
    DIVX3_PICDNMSG_D33 *d33;
    DIVX3_PICDNMSG_D34 *d34;
    DIVX3_PICDNMSG_D35 *d35;
    DIVX3_PICDNMSG_D36 *d36;
    DIVX3_PICDNMSG_D37 *d37;

    DIVX3_PICDNMSG_D63 *d63;
} DIVX3_PICDNMSG_S;


//SLICE��Ϣ�ؽṹ��
typedef struct
{
    DIVX3_SLCDNMSG_D0  *d0;
    DIVX3_SLCDNMSG_D1  *d1;

    DIVX3_SLCDNMSG_D63 *d63;
} DIVX3_SLCDNMSG_S;


#ifdef __cplusplus
extern "C" {
#endif

SINT32 DIVX3HAL_InitHal( VOID );
SINT32 DIVX3HAL_StartDec( DIVX3_DEC_PARAM_S *pDivx3DecParam, SINT32 VdhId );

#ifdef __cplusplus
}
#endif


#endif //__VDM_HAL_DIVX3_H__
