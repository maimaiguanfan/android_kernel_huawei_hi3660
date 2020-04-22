#ifndef __SENSOR_FEIMA_H__
#define __SENSOR_FEIMA_H__

#include <linux/mtd/hisi_nve_interface.h>

#define ALS_DBG_PARA_SIZE (8)
#define BUF_SIZE (128)
#define ALS_MCU_HAL_CONVER (10)
#define ACC_CONVERT_COFF 1000

extern struct hisi_nve_info_user user_info;
extern uint8_t als_support_under_screen_cali;
extern uint8_t ps_support_cali_after_sale;

struct sensor_cookie {
	int tag;
	const char *name;
	const struct attribute_group *attrs_group;
	struct device *dev;
};
typedef struct {
    uint16_t sub_cmd;
    uint16_t sar_info;
} rpc_ioctl_t;

typedef enum{
	ALS_UD_CMD_START,
	ALS_UD_CMD_SET_ADDR,
	ALS_UD_CMD_BUFFER_UPDATE,
}obj_als_ud_cmd_t;

typedef struct als_ud_cmd_map {
	const char *str;
	int cmd;
}als_ud_cmd_map_t;

void als_ud_block_release(void);
void wake_up_als_ud_block(void);
int als_underscreen_calidata_save(void);

#endif //__SENSOR_FEIMA_H__
