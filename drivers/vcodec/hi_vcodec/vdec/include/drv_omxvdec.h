


#ifndef __DRV_OMXVDEC_H__
#define __DRV_OMXVDEC_H__

#include <linux/ioctl.h>

#include "hi_type.h"

#define OMXVDEC_NAME                    "hi_vdec"
#define DRIVER_PATH                     "/dev/hi_vdec"

#define PATH_LEN                        (256)
#define NAME_LEN                        (128)

#define UHD_FRAME_WIDTH	                (4096)
#define UHD_FRAME_HEIGHT                (2304)
#define HD_FRAME_WIDTH                  (1920)
#define HD_FRAME_HEIGHT                 (1088)
#define SD_FRAME_WIDTH                  (1280)
#define SD_FRAME_HEIGHT                 (768)

#define MAX_SUPPORT_WIDTH               (8192)
#define MAX_SUPPORT_HEIGHT              (8192)

#define DEFAULT_ALIGN_STEP              (1)
#define NATIVE_ALIGN_STEP               (128) //stride align step when native buffer enable
#define ALIGN_UP(val, align)            (((val) + ((align)-1)) & ~((align)-1))

#define MAX(a, b)                       (((a) > (b)) ?  (a) : (b))
#define MIN(a, b)                       (((a) > (b)) ?  (b) : (a))

// omx color space enum
typedef enum hi_OMX_PIX_FMT {
    OMX_PIX_FMT_NV12,          /* YUV 4:2:0 semiplanar */
    OMX_PIX_FMT_NV21,          /* YVU 4:2:0 semiplanar */
    OMX_PIX_FMT_YUV420Planar,  /* YUV 4:2:0 planar     */
    OMX_PIX_FMT_YUV420Tile,    /* YUV 4:2:0 tile       */
} HI_OMX_PIX_FMT;

typedef enum hi_VDEC_S_TATUS {
	VDEC_S_SUCCESS = 0,
	VDEC_S_FAILED  = 1,
} HI_VDEC_S_TATUS;

/* omxvdec msg response types */
#define VDEC_MSG_RESP_BASE              (0x10000)
#define VDEC_MSG_RESP_OPEN              (VDEC_MSG_RESP_BASE + 0x1)
#define VDEC_MSG_RESP_START_DONE        (VDEC_MSG_RESP_BASE + 0x2)
#define VDEC_MSG_RESP_STOP_DONE         (VDEC_MSG_RESP_BASE + 0x3)
#define VDEC_MSG_RESP_PAUSE_DONE        (VDEC_MSG_RESP_BASE + 0x4)
#define VDEC_MSG_RESP_RESUME_DONE       (VDEC_MSG_RESP_BASE + 0x5)
#define VDEC_MSG_RESP_FLUSH_INPUT_DONE  (VDEC_MSG_RESP_BASE + 0x6)
#define VDEC_MSG_RESP_FLUSH_OUTPUT_DONE (VDEC_MSG_RESP_BASE + 0x7)
#define VDEC_MSG_RESP_INPUT_DONE        (VDEC_MSG_RESP_BASE + 0x8)
#define VDEC_MSG_RESP_OUTPUT_DONE       (VDEC_MSG_RESP_BASE + 0x9)
#define VDEC_MSG_RESP_MSG_STOP_DONE     (VDEC_MSG_RESP_BASE + 0xa)

/* omxvdec error code types */
#define VDEC_S_ERR_BASE                 (0x20000)
#define VDEC_ERR_FAIL                   (VDEC_S_ERR_BASE + 0x01)
#define VDEC_ERR_DYNAMIC_ALLOC_FAIL     (VDEC_S_ERR_BASE + 0x02)
#define VDEC_ERR_ILLEGAL_OP             (VDEC_S_ERR_BASE + 0x03)
#define VDEC_ERR_ILLEGAL_PARM           (VDEC_S_ERR_BASE + 0x04)
#define VDEC_ERR_BAD_POINTER            (VDEC_S_ERR_BASE + 0x05)
#define VDEC_ERR_BAD_HANDLE             (VDEC_S_ERR_BASE + 0x06)
#define VDEC_ERR_NOT_SUPPORTED          (VDEC_S_ERR_BASE + 0x07)
#define VDEC_ERR_BAD_STATE              (VDEC_S_ERR_BASE + 0x08)
#define VDEC_ERR_BUSY                   (VDEC_S_ERR_BASE + 0x09)
#define VDEC_ERR_HW_FATAL               (VDEC_S_ERR_BASE + 0x0a)
#define VDEC_ERR_BITSTREAM_ERR          (VDEC_S_ERR_BASE + 0x0b)
#define VDEC_ERR_QEMPTY                 (VDEC_S_ERR_BASE + 0x0c)
#define VDEC_ERR_QFULL                  (VDEC_S_ERR_BASE + 0x0d)
#define VDEC_ERR_INPUT_NOT_PROCESSED    (VDEC_S_ERR_BASE + 0x0e)
#define VDEC_ERR_INDEX_NOMORE           (VDEC_S_ERR_BASE + 0x0f)

#define VDEC_EVT_REPORT_BASE            (0x30000)
#define VDEC_EVT_REPORT_IMG_SIZE_CHG	(VDEC_EVT_REPORT_BASE + 0x1)
#define VDEC_EVT_REPORT_FRAME_RATE_CHG	(VDEC_EVT_REPORT_BASE + 0x2)
#define VDEC_EVT_REPORT_SCAN_TYPE_CHG	(VDEC_EVT_REPORT_BASE + 0x3)
#define VDEC_EVT_REPORT_HW_ERROR        (VDEC_EVT_REPORT_BASE + 0x4)
#define VDEC_EVT_REPORT_CROP_RECT_CHG	(VDEC_EVT_REPORT_BASE + 0x5)
#define VDEC_EVT_REPORT_SEQ_INFO_CHG    (VDEC_EVT_REPORT_BASE + 0x6)
#define VDEC_EVT_REPORT_ERR_OVER_REFTHR (VDEC_EVT_REPORT_BASE + 0x7)
#define VDEC_EVT_REPORT_COLOR_ASPECTS_CHG  (VDEC_EVT_REPORT_BASE + 0x9)

/* ========================================================
 * internal struct enum definition
 * ========================================================*/
typedef enum hi_ePORT_DIR {
    PORT_DIR_INPUT,
    PORT_DIR_OUTPUT,
    PORT_DIR_BOTH = 0xFFFFFFFF
} ePORT_DIR;

typedef enum hi_eBUFFER_TYPE {
    OMX_USE_OTHER,            // use outer buffer not native (unused in Android platform)
    OMX_USE_NATIVE,           // use outer native buffer
    OMX_ALLOCATE_DRV,         // allocate buffer in driver side
} eBUFFER_TYPE;

typedef enum hi_ePATH_MODE {
    PATH_MODE_NORMAL,
    PATH_MODE_NATIVE,
} ePATH_MODE;

/* CodecType */
typedef enum hi_OMXVDEC_CODEC_TYPE {
    OMXVDEC_H264 = 0,
    OMXVDEC_AVS,
    OMXVDEC_MPEG1,
    OMXVDEC_MPEG2,
    OMXVDEC_MPEG4,
    OMXVDEC_VC1,
    OMXVDEC_DIVX3,
    OMXVDEC_REAL8,
    OMXVDEC_REAL9,
    OMXVDEC_VP6,
    OMXVDEC_VP6F,
    OMXVDEC_VP6A,
    OMXVDEC_VP8,
    OMXVDEC_H263,
    OMXVDEC_SORENSON,
    OMXVDEC_MVC,
    OMXVDEC_HEVC,
} OMXVDEC_CODEC_TYPE;

typedef union hi_OMXVDEC_STD_EXT
{
    struct hi_vc1_ext
    {
        HI_S32         is_adv_profile;
        HI_S32         codec_version;
    } vc1_ext;

    struct hi_vp6_ext
    {
        /**<if the image need to be reversed, set to 1, otherwise set to 0 */
        HI_S32         need_reverse;
    } vp6_ext;
} OMXVDEC_STD_EXT;

typedef struct hi_OMXVDEC_CHAN_CFG {
    HI_BOOL            is_tvp;	
    HI_BOOL            lowdly_enable;
    HI_BOOL            compress_enable;
    ePATH_MODE         path_mode;
    HI_S8              spec_mode;
    HI_S8              dec_mode;
    HI_S8              output_order;
    HI_S32             priority;
    HI_S32             err_threshold;
    HI_S32             act_outbuf_num;
    HI_S32             act_inbuf_num;
    HI_S32             act_inbuf_size;
    HI_U32             cfg_width;
    HI_U32             cfg_height;
    HI_U32             cfg_stride;
    HI_U32             cfg_inbuf_num;
    HI_U32             cfg_outbuf_num;
    HI_U32             cfg_color_format;
    HI_U32             cfg_frame_rate;
    HI_U32             cfg_video_scenario;
    OMXVDEC_CODEC_TYPE cfg_codec_type;
    OMXVDEC_STD_EXT    cfg_codec_ext;
    HI_BOOL            cfg_conflict;
} OMXVDEC_CHAN_CFG;

/* video frame buffer description */
typedef struct hi_OMXVDEC_FRAME_S {
    HI_U32             phyaddr_Y;
    HI_U32             phyaddr_C;
    HI_U32             stride;
    HI_U32             frame_width;
    HI_U32             frame_height;
    HI_U32             disp_width;
    HI_U32             disp_height;
    HI_U32             left_offset;
    HI_U32             right_offset;
    HI_U32             top_offset;
    HI_U32             bottom_offset;
    HI_U32             format;
    HI_U32             act_crc[2];
    HI_BOOL            save_yuv;
    HI_CHAR            save_path[PATH_LEN];
} OMXVDEC_FRAME_S;

typedef struct hi_OMXVDEC_IMG_SIZE {
	HI_U32             frame_width;
	HI_U32             frame_height;
	HI_U32             frame_stride;
} OMXVDEC_IMG_SIZE;

typedef struct hi_OMXVDEC_SEQ_INFO {
	HI_U32             dec_width;
	HI_U32             dec_height;
	HI_U32             disp_width;
	HI_U32             disp_height;
	HI_U32             stride;
	HI_U32             frame_size;
    HI_U32             max_frame_num;
    HI_U32             min_frame_num;
} OMXVDEC_SEQ_INFO;

typedef struct
{
    HI_U8 video_full_range_flag;
    HI_U8 colour_primaries;
    HI_U8 transfer_characteristics;
    HI_U8 matrix_coeffs;
} OMXVDEC_COLOUR_DESCRIPTION_INFO_S;

/* For Compat USER 32 -> KER 64, all pointer members set in last in structure.
    NOTICE: MUST NOT ALTER member sequence of this structure */
typedef struct hi_OMXVDEC_BUF_DESC {
    HI_U8              is_secure;
	HI_U32             phyaddr;
	HI_U32             buffer_len;
	HI_U32             data_offset;
	HI_U32             data_len;
	HI_U32             flags;
    HI_U32             max_frm_num;
    HI_S32             share_fd;
	HI_S64             timestamp;
	eBUFFER_TYPE       buffer_type;
	ePORT_DIR          dir;
    OMXVDEC_FRAME_S    out_frame;
	HI_U64             kerneladdr;
	HI_VOID*           bufferaddr;
	HI_VOID*           client_data;
} OMXVDEC_BUF_DESC;

#ifdef CONFIG_COMPAT //Modified for 64-bit platform
typedef struct hi_COMPAT_BUF_DESC {
	HI_U32             phyaddr;
	HI_U32             buffer_len;
	HI_U32             data_offset;
	HI_U32             data_len;
	HI_U32             flags;
    HI_U32             max_frm_num;
    HI_S32             share_fd;
	HI_S64             timestamp;
	eBUFFER_TYPE       buffer_type;
	ePORT_DIR          dir;
    OMXVDEC_FRAME_S    out_frame;
	HI_U64             kerneladdr;
	compat_ulong_t     bufferaddr;
	compat_ulong_t     client_data;
} COMPAT_BUF_DESC;
#endif

/* For Compat USER 32 -> KER 64, all pointer menbers set in last in structure.
    NOTICE: MUST NOT ALTER the member sequence of this structure */
typedef struct hi_OMXVDEC_MSG_INFO {
	HI_U32             status_code;
	HI_U32             msgcode;
	HI_U32             msgdatasize;

    union hi_msgdata_vdec {
	OMXVDEC_IMG_SIZE   img_size;
	OMXVDEC_SEQ_INFO   seq_info;
	OMXVDEC_BUF_DESC   buf;
    OMXVDEC_COLOUR_DESCRIPTION_INFO_S colour_aspect_info;
    } msgdata;
} OMXVDEC_MSG_INFO;

#ifdef CONFIG_COMPAT //Modified for 64-bit platform
typedef struct hi_COMPAT_MSG_INFO {
	HI_U32             status_code;
	HI_U32             msgcode;
	HI_U32             msgdatasize;

  union hi_msgdata_compat {
	OMXVDEC_IMG_SIZE   img_size;
	OMXVDEC_SEQ_INFO   seq_info;
	COMPAT_BUF_DESC    buf;
    OMXVDEC_COLOUR_DESCRIPTION_INFO_S colour_aspect_info;
  } msgdata;
} COMPAT_MSG_INFO;
#endif

/* ========================================================
 * IOCTL for interaction with omx components
 * ========================================================*/
/* For Compat USER 32 -> KER 64, all pointer menbers set in last in structure.
    NOTICE: MUST NOT ALTER the member sequence of this structure */
typedef struct hi_OMXVDEC_IOCTL_MSG {
	HI_S32             chan_num;
	HI_VOID*           in;
	HI_VOID*           out;
} OMXVDEC_IOCTL_MSG;

#ifdef CONFIG_COMPAT //Modified for 64-bit platform
typedef struct hi_COMPAT_IOCTL_MSG {
    HI_S32             chan_num;
    compat_ulong_t     in;
    compat_ulong_t     out;
} COMPAT_IOCTL_MSG;
#endif

#define VDEC_IOCTL_MAGIC 'v'

#define VDEC_IOCTL_CHAN_CREATE   \
	_IO(VDEC_IOCTL_MAGIC, 1)

#define VDEC_IOCTL_CHAN_RELEASE  \
	_IO(VDEC_IOCTL_MAGIC, 2)

#define VDEC_IOCTL_SET_EXTRADATA \
	_IO(VDEC_IOCTL_MAGIC, 3)

#define VDEC_IOCTL_GET_EXTRADATA \
	_IO(VDEC_IOCTL_MAGIC, 4)

#define VDEC_IOCTL_FLUSH_PORT \
	_IO(VDEC_IOCTL_MAGIC, 5)

#define VDEC_IOCTL_CHAN_BIND_BUFFER \
	_IO(VDEC_IOCTL_MAGIC, 6)

#define VDEC_IOCTL_CHAN_UNBIND_BUFFER \
	_IO(VDEC_IOCTL_MAGIC, 7)

#define VDEC_IOCTL_FILL_OUTPUT_FRAME \
	_IO(VDEC_IOCTL_MAGIC, 8)

#define VDEC_IOCTL_EMPTY_INPUT_STREAM \
	_IO(VDEC_IOCTL_MAGIC, 9)

#define VDEC_IOCTL_CHAN_START \
	_IO(VDEC_IOCTL_MAGIC, 10)

#define VDEC_IOCTL_CHAN_STOP  \
	_IO(VDEC_IOCTL_MAGIC, 11)

#define VDEC_IOCTL_CHAN_PAUSE \
	_IO(VDEC_IOCTL_MAGIC, 12)

#define VDEC_IOCTL_CHAN_RESUME \
	_IO(VDEC_IOCTL_MAGIC, 13)

#define VDEC_IOCTL_CHAN_GET_MSG \
	_IO(VDEC_IOCTL_MAGIC, 14)

#define VDEC_IOCTL_CHAN_STOP_MSG \
	_IO(VDEC_IOCTL_MAGIC, 15)

#define VDEC_IOCTL_CHAN_ALLOC_BUFFER	\
	_IO(VDEC_IOCTL_MAGIC, 16)

#define VDEC_IOCTL_CHAN_FREE_BUFFER	\
	_IO(VDEC_IOCTL_MAGIC, 17)

#endif

