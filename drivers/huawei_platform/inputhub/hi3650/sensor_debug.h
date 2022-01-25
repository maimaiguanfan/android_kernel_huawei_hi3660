#ifndef __SENSOR_DEBUG_H__
#define __SENSOR_DEBUG_H__

#define REGISTER_SENSORHUB_DEBUG_OPERATION(FUNC) \
register_sensorhub_debug_operation(#FUNC, FUNC);

#define UNREGISTER_SENSORHUB_DEBUG_OPERATION(FUNC) \
unregister_sensorhub_debug_operation(FUNC);

#define MAX_STR_SIZE (1024)

typedef int (*sensor_debug_pfunc) (int tag, int argv[], int argc);

struct t_sensor_debug_operations_list {
	struct list_head head;
	struct mutex mlock;
};

/*to find operation by str*/
struct sensor_debug_cmd {
	const char *str;
	sensor_debug_pfunc operation;
	struct list_head entry;
};

/*to find tag by str*/
struct sensor_debug_tag_map {
	const char *str;
	int tag;
};

/*to search info by tag*/
struct sensor_debug_search_info {
	const char *sensor;
	int tag;
	void *private_data;
};

struct sensor_cookie {
	int tag;
	const char *name;
	const struct attribute_group *attrs_group;
	struct device *dev;
};
#define AR_MAX_CONFIG_NUM 12
#pragma pack(1)
typedef struct ar_activity_event {
	unsigned int event_type;
	unsigned int activity;
	unsigned long long timestamp;
} ar_activity_event_t;

typedef struct ar_config_event {
    unsigned int   event_type;
    unsigned int   activity;
    unsigned int   len;
    char   buf[];
} ar_config_event_t;

typedef struct ar_config {
    unsigned int report_interval;
    unsigned int   num;
    ar_config_event_t   activity_list[];
} ar_config_t;

typedef struct {
	uint8_t	   core;
	uint8_t	   rsv1;
	uint8_t	   rsv2;
	uint8_t	   sub_cmd;
}core_subcmd_t;

typedef struct ar_start_cmd{
    core_subcmd_t core_cmd;
    ar_config_t   start_param;
}ar_start_cmd_t;

typedef struct ar_update_cmd{
    core_subcmd_t core_cmd;
    ar_config_t   update_param;
}ar_update_cmd_t;

typedef struct ar_stop_cmd{
    core_subcmd_t core_cmd;
    unsigned int   para;
}ar_stop_cmd_t;

typedef struct ar_flush_cmd{
    core_subcmd_t core_cmd;
}ar_flush_cmd_t;

typedef struct ar_data {
	ar_activity_event_t activity_list[48];
} ar_data_t;

typedef struct ar_data_cmd {
	pkt_header_t hd;
	ar_data_t data;
} ar_data_cmd_t;
#pragma pack()

typedef enum {
	EVENT_NONE = 0,
	EVENT_ENTER = 1,
	EVENT_EXIT = 2,
	EVENT_BOTH = 3,
} EVENT_TYPE;

typedef enum {
    AR_ACTIVITY_VEHICLE = 0x00,
    AR_ACTIVITY_RIDING = 0x01,
    AR_ACTIVITY_WALK_SLOW = 0x02,
    AR_ACTIVITY_RUN_FAST = 0x03,
    AR_ACTIVITY_STATIONARY = 0x04,
    AR_ACTIVITY_TILT = 0x05,
    AR_ACTIVITY_END = 0x10,
    AR_VE_BUS = 0x11,				/* ��� */
    AR_VE_CAR = 0x12,				/* С�� */
    AR_VE_METRO = 0x13,				/* ���� */
    AR_VE_HIGH_SPEED_RAIL = 0x14,	/* ���� */
    AR_VE_AUTO = 0x15,				/* ��·��ͨ */
    AR_VE_RAIL = 0x16,				/* ��·��ͨ */
    AR_CLIMBING_MOUNT = 0x17,          /*��ɽ*/
    AR_FAST_WALK = 0x18,                   /*����*/
    AR_STOP_VEHICLE = 0x19, 		/*ͣ��*/
    AR_UNKNOWN = 0x3F,
} AR_ACTIVITY_TYPE;

typedef enum
{
    AR_ENVIRONMENT_HOME,
    AR_ENVIRONMENT_OFFICE,
    AR_ENVIRONMENT_STATION,
    AR_ENVIRONMENT_UNKNOWN	= 0x1f,
    AR_ENVIRONMENT_END	= 0x20,
} AR_ENVIRONMENT_TYPE;

extern int register_sensorhub_debug_operation(const char *func_name,
					      sensor_debug_pfunc op);
extern int unregister_sensorhub_debug_operation(sensor_debug_pfunc op);
extern bool find_and_do_cmd(int tag, struct sensor_debug_search_info *tab,
			    int size,
			    int (*op) (struct sensor_debug_search_info *info));
extern struct device *get_sensor_device_by_name(const char *name);
extern void sensor_get_data(struct sensor_data *data);

//aod
enum {
	AOD_DRV_PIXEL_FORMAT_RGB_565,
	AOD_DRV_PIXEL_FORMAT_RGBX_4444,
	AOD_DRV_PIXEL_FORMAT_RGBA_4444,
	AOD_DRV_PIXEL_FORMAT_RGBX_5551,
	AOD_DRV_PIXEL_FORMAT_RGBA_5551,
	AOD_DRV_PIXEL_FORMAT_RGBX_8888,
	AOD_DRV_PIXEL_FORMAT_RGBA_8888,

	AOD_DRV_PIXEL_FORMAT_BGR_565,
	AOD_DRV_PIXEL_FORMAT_BGRX_4444,
	AOD_DRV_PIXEL_FORMAT_BGRA_4444,
	AOD_DRV_PIXEL_FORMAT_BGRX_5551,
	AOD_DRV_PIXEL_FORMAT_BGRA_5551,
	AOD_DRV_PIXEL_FORMAT_BGRX_8888,
	AOD_DRV_PIXEL_FORMAT_BGRA_8888,

	AOD_DRV_PIXEL_FORMAT_YUV_422_I,

	/* YUV Semi-planar */
	AOD_DRV_PIXEL_FORMAT_YCbCr_422_SP, /* NV16 */
	AOD_DRV_PIXEL_FORMAT_YCrCb_422_SP,
	AOD_DRV_PIXEL_FORMAT_YCbCr_420_SP,
	AOD_DRV_PIXEL_FORMAT_YCrCb_420_SP, /* NV21*/

	/* YUV Planar */
	AOD_DRV_PIXEL_FORMAT_YCbCr_422_P,
	AOD_DRV_PIXEL_FORMAT_YCrCb_422_P,
	AOD_DRV_PIXEL_FORMAT_YCbCr_420_P,
	AOD_DRV_PIXEL_FORMAT_YCrCb_420_P, /* AOD_DRV_PIXEL_FORMAT_YV12 */

	/* YUV Package */
	AOD_DRV_PIXEL_FORMAT_YUYV_422_Pkg,
	AOD_DRV_PIXEL_FORMAT_UYVY_422_Pkg,
	AOD_DRV_PIXEL_FORMAT_YVYU_422_Pkg,
	AOD_DRV_PIXEL_FORMAT_VYUY_422_Pkg,
};

#define TIME_DIGIT_HOUR_OFFSET	(10)
#define TIME_DIGIT_MINU_OFFSET	(50)
#define TIME_DIGIT_TIME_OFFSET	(20)
#define SCREEN_RES_X (144)
#define SCREEN_RES_Y (256)
#define AOD_SINGLE_CLOCK_OFFSET_X (0)
#define AOD_SINGLE_CLOCK_OFFSET_Y (0)
#define AOD_SINGLE_CLOCK_RES_X (144)
#define AOD_SINGLE_CLOCK_RES_Y (105)
#define AOD_SINGLE_CLOCK_DIGITS_RES_X (15)
#define AOD_SINGLE_CLOCK_DIGITS_RES_Y (32)
#define AOD_SINGLE_CLOCK_OFFSET_X_2 (2)
#define AOD_SINGLE_CLOCK_OFFSET_Y_2 (10)
#define AOD_SINGLE_CLOCK_RES_X_2 (140)

#define AOD_DRV_FB_PIXEL_ALIGN(val, al)    (((val) + ((al)-1)) & ~((al)-1))
#define AOD_DRV_X_RES_PIXELS_ALIGN	(16)
#define SINGLE_CLOCK_SPACES (4)
#define DIGITS_COUNT (10)
#define SINGLE_CLOCK_DIGITS_BITMAP_MAX_SIZE (AOD_SINGLE_CLOCK_DIGITS_RES_X * AOD_SINGLE_CLOCK_DIGITS_RES_Y * 2)
#define FRAMEBUFFER_SIZE (AOD_DRV_FB_PIXEL_ALIGN(SCREEN_RES_X * 2, AOD_DRV_X_RES_PIXELS_ALIGN) * SCREEN_RES_Y )
#define SINGLE_CLOCK_DIGITS_BITMAPS_SIZE (DIGITS_COUNT * SINGLE_CLOCK_DIGITS_BITMAP_MAX_SIZE)
#define DDR_MEMORY_SIZE (FRAMEBUFFER_SIZE + SINGLE_CLOCK_DIGITS_BITMAPS_SIZE + sizeof(uint32_t))
#define DIGITS_BITMAPS_OFFSET (FRAMEBUFFER_SIZE + sizeof(uint32_t))
#endif
