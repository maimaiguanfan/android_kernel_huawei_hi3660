/*
 * parade.h
 * Parade TrueTouch(TM) Standard Product Registers.
 * For use with Parade touchscreen controllers.
 * Supported parts include:
 * TMA5XX
 * TMA448
 * TMA445A
 * TT21XXX
 * TT31XXX
 * TT4XXXX
 *
 * Copyright (C) 2015-2018 Parade Technologies
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2, and only version 2, as published by the
 * Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * Contact Parade Technologies at www.paradetech.com <ttdrivers@paradetech.com>
 *
 */

#ifndef __PARADE_H__
#define __PARADE_H__

#include <linux/device.h>
#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#elif defined(CONFIG_FB)
#include <linux/notifier.h>
#include <linux/fb.h>
#endif
#include <asm/unaligned.h>
#include <linux/debugfs.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/gpio.h>
#include <linux/hid.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/kernel.h>
#include <linux/limits.h>
#include <linux/module.h>
#include <linux/of_device.h>
#include <linux/of.h>
#include <linux/pm_runtime.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/suspend.h>
#include <linux/stringify.h>
#include <linux/types.h>
#include <linux/uaccess.h>
#include <linux/workqueue.h>
#include <linux/version.h>

#include <linux/stringify.h>
#include "../../huawei_ts_kit_algo.h"
#include "../../huawei_ts_kit.h"
#include "../../huawei_ts_kit_platform.h"

#define PARADE_DRV_VER_MAJOR 1
#define PARADE_DRV_VER_MINOR 2
#define PARADE_DRV_LAST_CHANGE_DATE "2018.11.02"

#define PT_IRQ_ASSERTED_VALUE 0
#define SUPPORT_DIFF_DISTINGUISHABILITY 1

#define  FLAG_EXIST    1

/*
   Version History:
   1.1, Last Change Date: 2016.11.20, charlie.zhang@paradetech.com
   1.2, Last Change Date: 2018.11.02, daniel.duan@paradetech.com
changes:
1) Call start up after firmware upgrade boot/sd update
2) Add the set/get power on flag, make rc return no error to avoid the error processing
3) The get_chip_info function, does not read data from IC during the sleep state, use the cached value
 */

#define PARADE_TTHE_TUNER_FILE_NAME "tthe_tuner"

#define PT_FW_MANUAL_UPGRADE_FILE_NAME "pt_fw_manual_upgrade"
#define PARADE_FW_FROM_SD_NAME "/sdcard/parade_fw.bin"
#define PARADE_FIRMWARE_SIZE   (64*1024)


#define PARADE_OEM_WRITE_FINISHED            1
#define PARADE_OEM_WRITE_NOT_FINISHED    2

#define PARADE_ROI_ENABLE 1
#ifdef PARADE_ROI_ENABLE
#define PARADE_ROI_HEAD_DATA_LENGTH        4
#define PARADE_ROI_DATA_READ_LENGTH        102
#define PARADE_ROI_DATA_SEND_LENGTH        (PARADE_ROI_DATA_READ_LENGTH-PARADE_ROI_HEAD_DATA_LENGTH)
#endif
/* Doze related macro */
#ifdef TS_SWITCH_TYPE_DOZE
#define PARADE_DOZE_ENABLE     1
#else
#define TS_SWITCH_TYPE_DOZE 1
#define TS_SWITCH_DOZE_ENABLE  1
#define TS_SWITCH_DOZE_DISABLE 2
#endif
/* abs settings */
#define PT_IGNORE_VALUE             -1

#define PARADE_VDD     "parade_vci"
#define PARADE_VBUS     "parade_vddio"

#define PT_FW_FILE_NAME "parade_fw.bin"
#define PT_MAX_PRBUF_SIZE           PIPE_BUF
#define PARADE_COPY_SIZE            1024
#define PT_PR_TRUNCATED             " truncated..."

#define PARADE_PREALLOCATED_CMD_BUFFER 32
#define PARADE_WORK_WAIT_TIMEOUT   20000

#ifdef PT_ENABLE_MAX_ELEN
#define PT_MAX_ELEN 100
#endif

#define LCD_PANEL_INFO_MAX_LEN  128
#define LCD_PANEL_TYPE_DEVICE_NODE_NAME     "huawei,lcd_panel_type"

#define PT_MAX_CONFIG_BYTES    256
#define PARADE_TTHE_TUNER_GET_PANEL_DATA_FILE_NAME "get_panel_data"
#define TTHE_TUNER_MAX_BUF    (PT_MAX_PRBUF_SIZE * 3)

#define GLOVE_SWITCH_ON 1
#define GLOVE_SWITCH_OFF 0

#define PARADE_AFTER_RESUME_TIMEOUT 10 // wait 10ms for interrupt enable

#define PARADE_CMD_WAITTIME 50 // wait 10ms for each command
#define PARADE_CMD_WAITTIME_LONG 100 // wait 10ms for each command
#define PT_CORE_STARTUP_RETRY_COUNT        3
#define MAX_NAME_LENGTH        64
#define PAR_REG_BASE                 0x00
#define SET_CMD_OPCODE(byte, opcode) SET_CMD_LOW(byte, opcode)
#define SET_CMD_REPORT_TYPE(byte, type) SET_CMD_HIGH(byte, ((type) << 4))
#define SET_CMD_REPORT_ID(byte, id) SET_CMD_LOW(byte, id)

#define TS_CHIP_PROJECTID_TYPE 0
#define TS_CHIP_PROJECTID_REGION_LENGTH 1 // in 16 byte
#define TS_CHIP_PROJECTID_LENGTH 10

/* HID */
#define PT_HID_VERSION        0x0100
#define PT_HID_APP_REPORT_ID    0xF7
#define PT_HID_APP_REPORT_DESCRIPTOR_PACKET_ID    0xF6
#define PT_HID_BL_REPORT_ID    0xFF

#define HID_INVALID_REPORT_ID        0x0
#define HID_TOUCH_REPORT_ID        0x1
#define HID_BTN_REPORT_ID        0x3
#define HID_WAKEUP_REPORT_ID        0x4
#define HID_NOISE_METRIC_REPORT_ID    0x5
#define HID_KNUCKLE_MODE_REPOR_ID    0xD
#define HID_TRACKING_HEATMAP_REPOR_ID    0xE
#define HID_SENSOR_DATA_REPORT_ID    0xF
#define HID_APP_RESPONSE_REPORT_ID    0x1F
#define HID_APP_OUTPUT_REPORT_ID    0x2F
#define HID_BL_RESPONSE_REPORT_ID    0x30
#define HID_BL_OUTPUT_REPORT_ID        0x40
#define HID_RESPONSE_REPORT_ID        0xF0

#define HID_INPUT_REPORT_ID_OFFSET 2
#define HID_INPUT_REPORT_EASYWAKE_EVID_OFFSET 3

#define HID_OUTPUT_REGISTER_OFFSET 0
#define HID_OUTPUT_REPORT_ID_OFFSET 4
#define HID_OUTPUT_CMD_ID_OFFSET 6
#define HID_OUTPUT_SET_PARA_ID_OFFSET 7

#define HID_OUTPUT_RESPONSE_SET_POWER_STATE_OFFSET 3
#define HID_OUTPUT_RESPONSE_SET_POWER_OPCODE_OFFSET 4
#define HID_OUTPUT_RESPONSE_REPORT_OFFSET    2
#define HID_OUTPUT_RESPONSE_CMD_OFFSET        4
#define HID_OUTPUT_RESPONSE_CMD_MASK        0x7F
#define HID_OUTPUT_RESPONSE_LENGTH_OFFSET    0
#define HID_OUTPUT_RESPONSE_SET_PARAM_ID_OFFSET    5
#define HID_OUTPUT_RESPONSE_SET_PARAM_SIZE_OFFSET    6
#define HID_OUTPUT_RESPONSE_GET_PARAM_ID_OFFSET    5
#define HID_OUTPUT_RESPONSE_GET_PARAM_SIZE_OFFSET    6
#define HID_OUTPUT_RESPONSE_GET_PARAM_DATA_OFFSET    7
#define HID_OUTPUT_RESPONSE_CAL_STATUS_OFFSET    5
#define HID_OUTPUT_RESPONSE_INIT_BASELINE_STATUS_OFFSET    5
#define HID_OUTPUT_RESPONSE_RUN_SELFTEST_STATUS_OFFSET    5
#define HID_OUTPUT_RESPONSE_RUN_SELFTEST_SUMMARY_OFFSET    6
#define HID_OUTPUT_RESPONSE_RUN_SELFTEST_RESULT_OFFSET    7
#define HID_OUTPUT_RESPONSE_GET_SELFTEST_STATUS_OFFSET    5
#define HID_OUTPUT_RESPONSE_GET_SELFTEST_ID_OFFSET    6
#define HID_OUTPUT_RESPONSE_GET_SELFTEST_DATALEN_OFFSET    7
#define HID_OUTPUT_RESPONSE_GET_SELFTEST_DATA_OFFSET    10
#define HID_OUTPUT_RESPONSE_GET_PANEL_SCAN_STATUS_OFFSET    5
#define HID_OUTPUT_RESPONSE_GET_PANEL_SCAN_ID_OFFSET    6
#define HID_OUTPUT_RESPONSE_GET_PANEL_SCAN_DATALEN_OFFSET    7
#define HID_OUTPUT_RESPONSE_GET_PANEL_SCAN_ELEMENT_SIZE_OFFSET    9
#define HID_OUTPUT_RESPONSE_GET_PANEL_SCAN_DATA_OFFSET    10
#define HID_OUTPUT_RESPONSE_READ_CONF_BLOCK_STATUS_OFFSET    5
#define HID_OUTPUT_RESPONSE_READ_CONF_BLOCK_EBID_OFFSET    6
#define HID_OUTPUT_RESPONSE_READ_CONF_BLOCK_LEN_OFFSET    7
#define HID_OUTPUT_RESPONSE_READ_CONF_BLOCK_RSVD_OFFSET    9
#define HID_OUTPUT_RESPONSE_READ_CONF_BLOCK_DATA_OFFSET    10
#define HID_OUTPUT_RESPONSE_READ_CONF_BLOCK_CRC_OFFSET    10 /* Based on data length */
#define HID_OUTPUT_RESPONSE_GET_DATA_STRUCTURE_STATUS_OFFSET    5
#define HID_OUTPUT_RESPONSE_GET_DATA_STRUCTURE_DATALEN_LOW_OFFSET    7
#define HID_OUTPUT_RESPONSE_GET_DATA_STRUCTURE_DATALEN_HIGH_OFFSET    8
#define HID_OUTPUT_RESPONSE_GET_DATA_STRUCTURE_DATA_OFFSET 10
#define HID_OUTPUT_GET_SELFTEST_READ_OFFSET_LOW		0
#define HID_OUTPUT_GET_SELFTEST_READ_OFFSET_HIGH		1
#define HID_OUTPUT_GET_SELFTEST_READ_LENGTH_LOW		2
#define HID_OUTPUT_GET_SELFTEST_READ_LENGTH_HIGH		3
#define HID_OUTPUT_GET_SELFTEST_TESET_ID		4

#define HID_OUTPUT_RESPONSE_WRITE_NVDATA_STATUS_OFFSET    5
#define HID_OUTPUT_RESPONSE_READ_NVDATA_STATUS_OFFSET    5
#define HID_OUTPUT_RESPONSE_READ_NVDATA_ACTUAL_LEN_LOW_OFFSET    7
#define HID_OUTPUT_RESPONSE_READ_NVDATA_ACTUAL_LEN_HIGH_OFFSET    8
#define HID_OUTPUT_RESPONSE_READ_NVDATA_DATA_OFFSET    10
#define HID_OUTPUT_RESPONSE_GET_NVINFO_STATUS_OFFSET    5
#define HID_OUTPUT_RESPONSE_GET_NVINFO_FALSHSIZE_LOW_OFFSET    6
#define HID_OUTPUT_RESPONSE_GET_NVINFO_FLASHSIZE_HIGH_OFFSET    7
#define HID_OUTPUT_RESPONSE_GET_NVINFO_ROW_SETTING_OFFSET    8

#define HID_OUTPUT_CMD_OFFSET            6
#define HID_OUTPUT_CMD_MASK            0x7F
#define HID_OUTPUT_DATA_OFFSET    5

#define HID_OUTPUT_BL_RESPONSE_SOP_OFFSET    4
#define HID_OUTPUT_BL_RESPONSE_STATUS_OFFSET    5
#define HID_OUTPUT_BL_RESPONSE_CHECKSUM_RESULT_OFFSET    8

#define HID_SYSINFO_TTDATA_OFFSET    5
#define HID_SYSINFO_SENSING_OFFSET    33
#define HID_SYSINFO_BTN_OFFSET        48
#define HID_SYSINFO_BTN_MASK        0xFF
#define HID_SYSINFO_MAX_BTN        8

#define HID_POWER_ON            0x0
#define HID_POWER_SLEEP            0x1
#define HID_LENGTH_BYTES        2
#define HID_LENGTH_AND_REPORT_ID_BYTES    3

/*  Timeout in ms */
#define PT_REQUEST_EXCLUSIVE_TIMEOUT        8000
#define PT_WATCHDOG_TIMEOUT            1000
#define PT_HID_RESET_TIMEOUT            5000
#define PT_HID_AUTO_CALI_CPLT_TIMEOUT          2500
/* HID_DESCRIPTOR_TIMEOUT value based on FW spec (CAL_OS) */
#define PT_HID_GET_HID_DESCRIPTOR_TIMEOUT    4000
#define PT_HID_GET_REPORT_DESCRIPTOR_TIMEOUT    500
#define PT_HID_SET_POWER_TIMEOUT        500
#ifdef VERBOSE_DEBUG
#define PT_HID_OUTPUT_TIMEOUT            2000
#else
#define PT_HID_OUTPUT_TIMEOUT            200
#endif
#define PT_HID_OUTPUT_SET_RAM_NORMAL_PARAM 200
#define PT_HID_OUTPUT_SET_GET_POWER_ON_FLAG 4000

#define PT_HID_OUTPUT_START_BOOTLOADER_TIMEOUT    2000
#define PT_HID_OUTPUT_USER_TIMEOUT        8000
#define PT_HID_OUTPUT_GET_SYSINFO_TIMEOUT    3000
#define PT_HID_OUTPUT_GET_SYSINFO_TIMEOUT_LONGGEST (10*1000)
#define PT_HID_OUTPUT_CALIBRATE_IDAC_TIMEOUT    5000
#define PT_HID_OUTPUT_WRITE_CONF_BLOCK_TIMEOUT    400
#define PT_HID_OUTPUT_RUN_SELF_TEST_TIMEOUT    10000
#define PT_HID_OUTPUT_BL_INITIATE_BL_TIMEOUT    20000
#define PT_HID_OUTPUT_BL_PROGRAM_AND_VERIFY_TIMEOUT    400

#define PT_HID_RESUME_STARTUP_TIMEOUT 120

#define PT_WATCHDOG_RETRY_COUNT            60

#define HID_OUTPUT_READ_NV_DATA 0x62
#define HID_OUTPUT_WRITE_NV_DATA 0x63
#define HID_OUTPUT_GET_NV_INFO 0x64

#define PARADE_OEM_FLASH_ROW_SIZE_MAX 128
#define PARADE_OEM_FLASH_ONE_ROW_SIZE 16

/* drv_debug commands */
#define PT_DBG_SUSPEND                  4
#define PT_DBG_RESUME                   5
#define PT_DBG_SOFT_RESET               97
#define PT_DBG_RESET                    98
#define PT_DBG_HID_RESET                50
#define PT_DBG_HID_GET_REPORT           51
#define PT_DBG_HID_SET_REPORT           52
#define PT_DBG_HID_SET_POWER_ON         53
#define PT_DBG_HID_SET_POWER_SLEEP      54
#define PT_DBG_HID_NULL                 100
#define PT_DBG_HID_ENTER_BL             101
#define PT_DBG_HID_SYSINFO              102
#define PT_DBG_HID_SUSPEND_SCAN         103
#define PT_DBG_HID_RESUME_SCAN          104
#define PT_DBG_HID_STOP_WD              105
#define PT_DBG_HID_START_WD             106

#define PT_TTHE_TUNER_EXIT              107
#define    PT_TTHE_BUF_CLEAN               108

/* maximum number of concurrent tracks */
#define TOUCH_REPORT_SIZE_EXT       12
#define TOUCH_REPORT_SIZE           10
#define TOUCH_INPUT_HEADER_SIZE     7
#define TOUCH_COUNT_BYTE_OFFSET     5
#define BTN_REPORT_SIZE             9
#define BTN_INPUT_HEADER_SIZE       5
#define SENSOR_REPORT_SIZE          150
#define SENSOR_HEADER_SIZE          4

#define MAX_RETRY_TIMES        3
#define COLOR_INFO_INDEX    0

/* helpers */
#define GET_NUM_TOUCHES(x)          ((x) & 0x1F)
#define IS_LARGE_AREA(x)            ((x) & 0x20)
#define IS_BAD_PKT(x)               ((x) & 0x20)
#define IS_TMO(t)                   ((t) == 0)
#define HI_BYTE(x)                  (u8)(((x) >> 8) & 0xFF)
#define LOW_BYTE(x)                 (u8)((x) & 0xFF)
#define SET_CMD_LOW(byte, bits)    \
	((byte) = (((byte) & 0xF0) | ((bits) & 0x0F)))
#define SET_CMD_HIGH(byte, bits)\
	((byte) = (((byte) & 0x0F) | ((bits) & 0xF0)))

#define GET_MASK(length) \
	((1 << length) - 1)
#define GET_FIELD(name, length, shift) \
	((name >> shift) & GET_MASK(length))

#define HID_ITEM_SIZE_MASK    0x03
#define HID_ITEM_TYPE_MASK    0x0C
#define HID_ITEM_TAG_MASK    0xF0

#define HID_ITEM_SIZE_SHIFT    0
#define HID_ITEM_TYPE_SHIFT    2
#define HID_ITEM_TAG_SHIFT    4

#define HID_GET_ITEM_SIZE(x)  \
	((x & HID_ITEM_SIZE_MASK) >> HID_ITEM_SIZE_SHIFT)
#define HID_GET_ITEM_TYPE(x) \
	((x & HID_ITEM_TYPE_MASK) >> HID_ITEM_TYPE_SHIFT)
#define HID_GET_ITEM_TAG(x) \
	((x & HID_ITEM_TAG_MASK) >> HID_ITEM_TAG_SHIFT)

#define IS_DEEP_SLEEP_CONFIGURED(x) \
	((x) == 0 || (x) == 0xFF)

#define IS_PIP_VER_GE(p, maj, min) \
	((p)->ttdata.pip_ver_major < (maj) ? \
	 0 : \
	 ((p)->ttdata.pip_ver_minor < (min) ? \
	  0 : \
	  1))

/* Recognized usages */
/* undef them first for possible redefinition in Linux */
#undef HID_DI_PRESSURE
#undef HID_DI_TIP
#undef HID_DI_CONTACTID
#undef HID_DI_CONTACTCOUNT
#undef HID_DI_SCANTIME
#define HID_DI_PRESSURE        0x000d0030
#define HID_DI_TIP        0x000d0042
#define HID_DI_CONTACTID    0x000d0051
#define HID_DI_CONTACTCOUNT    0x000d0054
#define HID_DI_SCANTIME        0x000d0056

#define PANEL_ID_NOT_ENABLED    0xFF
#define PT_TOUCH_SETTINGS_MAX 32
#define PT_NUM_MFGID                8

#define PT_HID_MAX_REPORTS        8
#define PT_HID_MAX_FIELDS        128
#define PT_HID_MAX_COLLECTIONS        3
#define PT_HID_MAX_NESTED_COLLECTIONS    PT_HID_MAX_COLLECTIONS
#define PT_MAX_MSG_LEN    255
#define PT_MAX_INPUT        512
#define PT_PIP_1P7_EMPTY_BUF    0xFF00

#define PT_NUM_EXT_TCH_FIELDS   3
#define PT_WAKEUP_RETRY_TIMES 2

#define PT_APP_RETURN_RESPONSE_STATUS_BIT 5

/* TP Color block */
#define PT_TPCOLOR_ROW    0
#define PT_TPCOLOR_READ_SIZE    5
#define PT_TPCOLOR_WRITE_SIZE    128
#define PT_TPCOLOR_WRITE_OFFSET    8

/* ttconfig block */
#define PT_TTCONFIG_VERSION_OFFSET    8
#define PT_TTCONFIG_VERSION_SIZE    2
#define PT_TTCONFIG_VERSION_ROW        0

/* FW RAM parameters */
#define PT_RAM_ID_TOUCHMODE_ENABLED        0x02
#define PT_RAM_ID_PROXIMITY_ENABLE         0x20
#define PT_RAM_ID_TOUCHMODE_ENABLED_SIZE  1
#define PT_RAM_ID_PROXIMITY_ENABLE_SIZE   1
#define PT_RAM_ID_TOUCH_SWITCH             0x1A
#define PT_RAM_ID_TOUCH_MODE               0x02
#define PT_RAM_ID_POWER                    0x21
#define RAM_PARA_POWER_ON_FLAG        0x01
#define PT_RAM_ID_HOLSTER                  0x22
#define RAM_PARA_HOLSTER_ON           1
#define RAM_PARA_HOLSTER_OFF          0

/* MMI Report rate change */
#define PT_REPORT_RATE_DELAY    2000
#define PT_REPORT_RATE_RE_BUF    8
#define PT_REPORT_RATE_ERR        1000
#define PT_REPORT_RATE_TRCRQ    5
#define PT_REPORT_RATE_TRCTS    6
#define PT_CORRECT_REPORT_RATE    0
#define PT_REPORT_RATE_TRUE        true
#define PT_REPORT_RATE_FALSE    false
#define PT_REPORT_RATE_SUP        1
#define PT_REPORT_RATE_UNSUP    0
#define PT_REPORT_RATE_PASS        "4P-"
#define PT_REPORT_RATE_FAIL        "4F-"
/* MMI self noise test */
#define PT_RAW_RX_NUM            28
#define PT_RAW_TX_NUM            28
#define PT_CASE_EXIST            0
#define PT_DATA_NUM                0
#define PT_LINE_NUM                0
#define PT_SEE_IT_NEXT_ONE        1
#define PT_SELF_NOISE_UNSUP        0
#define PT_SELF_NOISE_SUP        1
#define PT_SELF_NOISE_PASS        "9P-"
#define PT_SELF_NOISE_FAIL        "9F-"
#define PT_CHECK_BULCKED_TRUE    1
#define PT_CHECK_BULCKED_FALSE    0
#define PT_CHECK_IRQ_STATE_TRUE        1
#define PT_CHECK_IRQ_STATE_FALSE    0

#define PT_DOUBLE_CLICK_WAKEUP    (1<<0)

#define PT_SHOW_BEFORE_SUSPEND        1
#define PT_AVOID_ROI_SWITCH            1

/*
 * The following macros are used to calculate the length to set for
 * hid_output structure.
 */
#define PT_SECURITY_KEY_LENGTH        8
#define PT_WRITE_NV_BLOCK_ID         0x01

#define PT_WRITE_LEN_ENTER_EASY_WAKEUP 1
#define PT_WRITE_LEN_WRITE_NV_DATA 15 /* 5+8+2 */
#define PT_WRITE_LEN_READ_NV_DATA 5
#define PT_WRITE_LEN_GET_PARAMETER 1
#define PT_WRITE_LEN_SET_PARAMETER 3
#define PT_WRITE_LEN_GET_DATA_STRUCTURE 5
#define PT_WRITE_LEN_CALIBRATE 1
#define PT_WRITE_LEN_READ_CONF_BLOCK 5
#define PT_WRITE_LEN_RETRIEVE_PANEL_SCAN 5
#define PT_WRITE_LEN_RUN_SELFTEST 1
#define PT_WRITE_LEN_GET_SELFTEST_RESULT 5
#define PT_WRITE_LEN_INIT_BASELINE 1

#define TS_NV_STRUCTURE_PROID_OFFSET 0
#define TS_NV_STRUCTURE_BAR_CODE_OFFSET1 1
#define TS_NV_STRUCTURE_BAR_CODE_OFFSET2 4
#define TS_NV_STRUCTURE_BRIGHTNESS_OFFSET1 7
#define TS_NV_STRUCTURE_BRIGHTNESS_OFFSET2 9
#define TS_NV_STRUCTURE_WHITE_POINT_OFFSET1 8
#define TS_NV_STRUCTURE_WHITE_POINT_OFFSET2 10
#define TS_NV_STRUCTURE_REPAIR_OFFSET1 11
#define TS_NV_STRUCTURE_REPAIR_OFFSET2 12
#define TS_NV_STRUCTURE_REPAIR_OFFSET3 13
#define TS_NV_STRUCTURE_REPAIR_OFFSET4 14
#define TS_NV_STRUCTURE_REPAIR_OFFSET5 15

#define PT_PIP_DEFAULT_EMPTY_BUF 2
#define PT_RESET_SENTINEL_BUF    0
#define TOUCH_DATA_START_INDEX     3
#define PT_UPGRADE_FW_RETRY        5
#define ABS_MT_WIDTH_MAX        100
#define PT_PROG_ROW_HEAD_LEN    3
/* firmware image  */
#define PT_FW_HEADER_SIZE_OFFSET    0
#define PT_FW_VERSION_OFFSET    3
#define PT_FW_REVISION_OFFSET    9

enum parade_module_id {
    MODULE_OFILM_ID = 0,
    MODULE_EELY_ID   = 1,
    MODULE_TRULY_ID = 2,
    MODULE_MUTTO_ID = 3,
    MODULE_JUNDA_ID = 5,
    MODULE_LENSONE_ID = 6,
    MODULE_TOPTOUCH_ID = 18,
    MODULE_UNKNOW_ID = 0xFF,
};

enum parade_chip_name {
    CHIP_TT41701_ID = 31,
    CHIP_CS448_ID   = 39,
    CHIP_CYTT21403_ID = 51,
};


enum parade_core_platform_flags {
    PT_CORE_FLAG_NONE,
    PT_CORE_FLAG_POWEROFF_ON_SLEEP = 0x02,
    PT_CORE_FLAG_RESTORE_PARAMETERS = 0x04,
};

enum parade_core_platform_easy_wakeup_gesture {
    PT_CORE_EWG_NONE,
    PT_CORE_EWG_TAP_TAP,
    PT_CORE_EWG_TWO_FINGER_SLIDE,
    PT_CORE_EWG_RESERVED,
    PT_CORE_EWG_WAKE_ON_INT_FROM_HOST = 0xFF,
};

enum parade_loader_platform_flags {
    PT_LOADER_FLAG_NONE,
    PT_LOADER_FLAG_CALIBRATE_AFTER_FW_UPGRADE,
    /* Use CONFIG_VER field in TT_CFG to decide TT_CFG update */
    PT_LOADER_FLAG_CHECK_TTCONFIG_VERSION,
    PT_LOADER_FLAG_CALIBRATE_AFTER_TTCONFIG_UPGRADE,
};



struct parade_touch_firmware {
    const uint8_t *img;
    uint32_t size;
    const uint8_t *ver;
    uint8_t vsize;
    uint8_t panel_id;
};

struct parade_touch_config {
    struct touch_settings *param_regs;
    struct touch_settings *param_size;
    const uint8_t *fw_ver;
    uint8_t fw_vsize;
    uint8_t panel_id;
};

struct parade_loader_platform_data {
    struct parade_touch_firmware *fw;
    struct parade_touch_config *ttconfig;
    struct parade_touch_firmware **fws;
    struct parade_touch_config **ttconfigs;
    u32 flags;
};

typedef int (*parade_platform_read)(struct device *dev, void *buf, int size);

struct parade_core_platform_data {
    int irq_gpio;
    int rst_gpio;
    int level_irq_udelay;
    u16 hid_desc_register;
    u16 vendor_id;
    u16 product_id;
    struct touch_settings *sett[PT_TOUCH_SETTINGS_MAX];
    u32 flags;
    u8 wakeup_gesture_enable;
    s8 irq_config; /* 0 - LOW LEVEL  1 - HIGH LEVEL  2 - RAISE EDGE  3 - FALL EDGE */
    int (*irq_stat)(struct ts_kit_device_data *pdata);
    bool check_irq_state;
};

struct touch_framework {
    const int16_t  *abs;
    uint8_t         size;
    uint8_t         enable_vkeys;
} __packed;

enum parade_mt_platform_flags {
    PT_MT_FLAG_NONE,
    PT_MT_FLAG_HOVER = 0x04,
    PT_MT_FLAG_FLIP = 0x08,
    PT_MT_FLAG_INV_X = 0x10,
    PT_MT_FLAG_INV_Y = 0x20,
    PT_MT_FLAG_VKEYS = 0x40,
    PT_MT_FLAG_NO_TOUCH_ON_LO = 0x80,
};

struct parade_mt_platform_data {
    struct touch_framework *frmwrk;
    unsigned short flags;
    char const *inp_dev_name;
    int vkeys_x;
    int vkeys_y;
};

enum parade_sleep_mode {
    PT_CMD_SLEEP_MODE,
    PT_POWER_DOWN_MODE,
    PT_EASY_WAKEUP_MODE
};

/* abs signal capabilities offsets in the frameworks array */
enum parade_sig_caps {
    PT_SIGNAL_OST,
    PT_MIN_OST,
    PT_MAX_OST,
    PT_FUZZ_OST,
    PT_FLAT_OST,
    PT_NUM_ABS_SET    /* number of signal capability fields */
};

/* helpers */
#define NUM_SIGNALS(frmwrk)        ((frmwrk)->size / PT_NUM_ABS_SET)
#define PARAM(frmwrk, sig_ost, cap_ost) \
	((frmwrk)->abs[((sig_ost) * PT_NUM_ABS_SET) + (cap_ost)])

#define PARAM_SIGNAL(frmwrk, sig_ost)    PARAM(frmwrk, sig_ost, PT_SIGNAL_OST)
#define PARAM_MIN(frmwrk, sig_ost)    PARAM(frmwrk, sig_ost, PT_MIN_OST)
#define PARAM_MAX(frmwrk, sig_ost)    PARAM(frmwrk, sig_ost, PT_MAX_OST)
#define PARAM_FUZZ(frmwrk, sig_ost)    PARAM(frmwrk, sig_ost, PT_FUZZ_OST)
#define PARAM_FLAT(frmwrk, sig_ost)    PARAM(frmwrk, sig_ost, PT_FLAT_OST)

enum hid_command {
    HID_CMD_RESERVED,
    HID_CMD_RESET,
    HID_CMD_GET_REPORT,
    HID_CMD_SET_REPORT,
    HID_CMD_GET_IDLE,
    HID_CMD_SET_IDLE,
    HID_CMD_GET_PROTOCOL,
    HID_CMD_SET_PROTOCOL,
    HID_CMD_SET_POWER,
    HID_CMD_VENDOR = 0xE,
};

enum hid_output_cmd_type {
    HID_OUTPUT_CMD_APP,
    HID_OUTPUT_CMD_BL,
};

enum hid_output {
    HID_OUTPUT_NULL,
    HID_OUTPUT_START_BOOTLOADER,
    HID_OUTPUT_GET_SYSINFO,
    HID_OUTPUT_SUSPEND_SCANNING,
    HID_OUTPUT_RESUME_SCANNING,
    HID_OUTPUT_GET_PARAM,
    HID_OUTPUT_SET_PARAM,
    HID_OUTPUT_GET_NOISE_METRICS,
    HID_OUTPUT_RESERVED,
    HID_OUTPUT_ENTER_EASYWAKE_STATE,
    HID_OUTPUT_VERIFY_CONFIG_BLOCK_CRC = 0x20,
    HID_OUTPUT_GET_CONFIG_ROW_SIZE,
    HID_OUTPUT_READ_CONF_BLOCK,
    HID_OUTPUT_WRITE_CONF_BLOCK,
    HID_OUTPUT_GET_DATA_STRUCTURE,
    HID_OUTPUT_LOAD_SELF_TEST_PARAM,
    HID_OUTPUT_RUN_SELF_TEST,
    HID_OUTPUT_GET_SELF_TEST_RESULT,
    HID_OUTPUT_CALIBRATE_IDACS,
    HID_OUTPUT_INITIALIZE_BASELINES,
    HID_OUTPUT_EXEC_PANEL_SCAN,
    HID_OUTPUT_RETRIEVE_PANEL_SCAN,
    HID_OUTPUT_START_SENSOR_DATA_MODE,
    HID_OUTPUT_STOP_SENSOR_DATA_MODE,
    HID_OUTPUT_START_TRACKING_HEATMAP_MODE,
    HID_OUTPUT_START_KNUCKLE_MODE,
    HID_OUTPUT_REPORT_RATE_SWITCH,
    HID_OUTPUT_INT_PIN_OVERRIDE = 0x40,
    HID_OUTPUT_STORE_PANEL_SCAN = 0x60,
    HID_OUTPUT_PROCESS_PANEL_SCAN,
    HID_OUTPUT_DISCARD_INPUT_REPORT,
    HID_OUTPUT_LAST,
    HID_OUTPUT_USER_CMD,
};

enum hid_output_bl {
    HID_OUTPUT_BL_VERIFY_APP_INTEGRITY = 0x31,
    HID_OUTPUT_BL_GET_INFO = 0x38,
    HID_OUTPUT_BL_PROGRAM_AND_VERIFY,
    HID_OUTPUT_BL_LAUNCH_APP = 0x3B,
    HID_OUTPUT_BL_GET_PANEL_ID = 0x3E,
    HID_OUTPUT_BL_INITIATE_BL = 0x48,
    HID_OUTPUT_BL_LAST,
};

#define HID_OUTPUT_BL_SOP    0x1
#define HID_OUTPUT_BL_EOP    0x17

enum hid_output_bl_status {
    ERROR_SUCCESS,
    ERROR_KEY,
    ERROR_VERIFICATION,
    ERROR_LENGTH,
    ERROR_DATA,
    ERROR_COMMAND,
    ERROR_CRC = 8,
    ERROR_FLASH_ARRAY,
    ERROR_FLASH_ROW,
    ERROR_FLASH_PROTECTION,
    ERROR_UKNOWN = 15,
    ERROR_INVALID,
};

enum parade_mode {
    PT_MODE_UNKNOWN,
    PT_MODE_BOOTLOADER,
    PT_MODE_OPERATIONAL,
};

enum {
    PT_IC_GRPNUM_RESERVED,
    PT_IC_GRPNUM_CMD_REGS,
    PT_IC_GRPNUM_TCH_REP,
    PT_IC_GRPNUM_DATA_REC,
    PT_IC_GRPNUM_TEST_REC,
    PT_IC_GRPNUM_PCFG_REC,
    PT_IC_GRPNUM_TCH_PARM_VAL,
    PT_IC_GRPNUM_TCH_PARM_SIZE,
    PT_IC_GRPNUM_RESERVED1,
    PT_IC_GRPNUM_RESERVED2,
    PT_IC_GRPNUM_OPCFG_REC,
    PT_IC_GRPNUM_DDATA_REC,
    PT_IC_GRPNUM_MDATA_REC,
    PT_IC_GRPNUM_TEST_REGS,
    PT_IC_GRPNUM_BTN_KEYS,
    PT_IC_GRPNUM_TTHE_REGS,
    PT_IC_GRPNUM_SENSING_CONF,
    PT_IC_GRPNUM_NUM,
};

enum parade_event_id {
    PT_EV_NO_EVENT,
    PT_EV_TOUCHDOWN,
    PT_EV_MOVE,        /* significant displacement (> act dist) */
    PT_EV_LIFTOFF,        /* record reports last position */
};
enum parade_object_id {
    PT_OBJ_STANDARD_FINGER,
    PT_OBJ_PROXIMITY,
    PT_OBJ_STYLUS,
    PT_OBJ_HOVER,
    PT_OBJ_GLOVE,
};


/* System Information interface definitions */
struct parade_ttdata_dev {
    u8 pip_ver_major;
    u8 pip_ver_minor;
    __le16 fw_pid;
    u8 fw_ver_major;
    u8 fw_ver_minor;
    __le32 revctrl;
    __le16 fw_ver_conf;
    u8 bl_ver_major;
    u8 bl_ver_minor;
    __le16 jtag_si_id_l;
    __le16 jtag_si_id_h;
    u8 mfg_id[PT_NUM_MFGID];
    __le16 post_code;
} __packed;

struct parade_sensing_conf_data_dev {
    u8 electrodes_x;
    u8 electrodes_y;
    __le16 len_x;
    __le16 len_y;
    __le16 res_x;
    __le16 res_y;
    __le16 max_z;
    u8 origin_x;
    u8 origin_y;
    u8 panel_id;
    u8 btn;
    u8 scan_mode;
    u8 max_num_of_tch_per_refresh_cycle;
} __packed;

struct parade_ttdata {
    u8 pip_ver_major;
    u8 pip_ver_minor;
    u8 bl_ver_major;
    u8 bl_ver_minor;
    u8 fw_ver_major;
    u8 fw_ver_minor;
    u16 fw_pid;
    u16 fw_ver_conf;
    u16 post_code;
    u32 revctrl;
    u16 jtag_id_l;
    u16 jtag_id_h;
    u8 mfg_id[PT_NUM_MFGID];
};

struct parade_sensing_conf_data {
    u16 res_x;
    u16 res_y;
    u16 max_z;
    u16 len_x;
    u16 len_y;
    u8 electrodes_x;
    u8 electrodes_y;
    u8 origin_x;
    u8 origin_y;
    u8 panel_id;
    u8 btn;
    u8 scan_mode;
    u8 max_tch;
    u8 rx_num;
    u8 tx_num;
};

enum parade_tch_abs {    /* for ordering within the extracted touch data array */
    PT_TCH_X,    /* X */
    PT_TCH_Y,    /* Y */
    PT_TCH_P,    /* P (Z) */
    PT_TCH_T,    /* TOUCH ID */
    PT_TCH_E,    /* EVENT ID */
    PT_TCH_O,    /* OBJECT ID */
    PT_TCH_TIP,    /* OBJECT ID */
    PT_TCH_MAJ,    /* TOUCH_MAJOR */
    PT_TCH_MIN,    /* TOUCH_MINOR */
    PT_TCH_OR,    /* ORIENTATION */
    PT_TCH_WX,    /* TOUCH_WX */
    PT_TCH_WY,    /* TOUCH_WY */
    PT_TCH_EWX,    /* TOUCH_EWX */
    PT_TCH_EWY,    /* TOUCH_EWY */
    PT_TCH_XER,    /* TOUCH_XER */
    PT_TCH_YER,    /* TOUCH_YER */
    PT_TCH_NUM_ABS,
};

enum parade_tch_hdr {
    PT_TCH_TIME,    /* SCAN TIME */
    PT_TCH_NUM,    /* NUMBER OF RECORDS */
    PT_TCH_LO,    /* LARGE OBJECT */
    PT_TCH_NOISE,    /* NOISE EFFECT */
    PT_TCH_COUNTER,    /* REPORT_COUNTER */
    PT_TCH_NUM_HDR,
};

static const char *const parade_tch_abs_string[] = {
    [PT_TCH_X]    = "X",
    [PT_TCH_Y]    = "Y",
    [PT_TCH_P]    = "P",
    [PT_TCH_T]    = "T",
    [PT_TCH_E]    = "E",
    [PT_TCH_O]    = "O",
    [PT_TCH_TIP]    = "TIP",
    [PT_TCH_MAJ]    = "MAJ",
    [PT_TCH_MIN]    = "MIN",
    [PT_TCH_OR]    = "OR",
    [PT_TCH_WX]    = "WX",
    [PT_TCH_WY]    = "WY",
    [PT_TCH_EWX]    = "EWX",
    [PT_TCH_EWY]    = "EWY",
    [PT_TCH_XER]    = "XER",
    [PT_TCH_YER]    = "YER",
    [PT_TCH_NUM_ABS] = "INVALID",
};

static const char *const parade_tch_hdr_string[] = {
    [PT_TCH_TIME]    = "SCAN TIME",
    [PT_TCH_NUM]    = "NUMBER OF RECORDS",
    [PT_TCH_LO]    = "LARGE OBJECT",
    [PT_TCH_NOISE]    = "NOISE EFFECT",
    [PT_TCH_COUNTER] = "REPORT_COUNTER",
    [PT_TCH_NUM_HDR] = "INVALID",
};

struct parade_tch_abs_params {
    size_t ofs;    /* abs byte offset */
    size_t size;    /* size in bits */
    size_t min;    /* min value */
    size_t max;    /* max value */
    size_t bofs;    /* bit offset */
    u8 report;
};

struct parade_touch {
    int hdr[PT_TCH_NUM_HDR];
    int abs[PT_TCH_NUM_ABS];
};

enum parade_btn_state {
    PT_BTN_RELEASED = 0,
    PT_BTN_PRESSED = 1,
    PT_BTN_NUM_STATE
};

struct parade_btn {
    bool enabled;
    int state;
    int key_code;
};

enum parade_ic_ebid {
    PT_TCH_PARM_EBID,
    PT_MDATA_EBID,
    PT_DDATA_EBID,
};

struct parade_ttconfig {
    u16 version;
    u16 crc;
};

struct parade_report_desc_data {
    u16 tch_report_id;
    u16 tch_record_size;
    u16 tch_header_size;
    u16 btn_report_id;
};

struct parade_sysinfo {
    bool ready;
    struct parade_ttdata ttdata;
    struct parade_sensing_conf_data sensing_conf_data;
    struct parade_report_desc_data desc;
    int num_btns;
    struct parade_btn *btn;
    struct parade_ttconfig ttconfig;
    struct parade_tch_abs_params tch_hdr[PT_TCH_NUM_HDR];
    struct parade_tch_abs_params tch_abs[PT_TCH_NUM_ABS];
    u8 *xy_mode;
    u8 *xy_data;
};

enum parade_fb_state {
    FB_ON,
    FB_OFF,
};

enum parade_sleep_state {
    SS_SLEEP_OFF,
    SS_SLEEP_ON,
    SS_SLEEPING,
    SS_WAKING,
};

enum parade_startup_state {
    STARTUP_NONE,
    STARTUP_QUEUED,
    STARTUP_RUNNING,
    STARTUP_ILLEGAL,
};

struct parade_hid_desc {
    __le16 hid_desc_len;
    u8 packet_id;
    u8 reserved_byte;
    __le16 bcd_version;
    __le16 report_desc_len;
    __le16 report_desc_register;
    __le16 input_register;
    __le16 max_input_len;
    __le16 output_register;
    __le16 max_output_len;
    __le16 command_register;
    __le16 data_register;
    __le16 vendor_id;
    __le16 product_id;
    __le16 version_id;
    u8 reserved[4];
} __packed;

struct parade_hid_core {
    u16 hid_vendor_id;
    u16 hid_product_id;
    __le16 hid_desc_register;
    u16 hid_report_desc_len;
    u16 hid_max_input_len;
    u16 hid_max_output_len;
};

struct parade_features {
    uint8_t easywake;
    uint8_t noise_metric;
    uint8_t tracking_heatmap;
    uint8_t sensor_data;
};

enum parade_process_state {
    PT_STATE_INIT = 0,
    PT_STATE_FW_UPDATE = 1,
    PT_STATE_NORMAL = 2,
};

struct parade_mt_data {
    struct device *dev;
    struct parade_sysinfo *si;
    struct input_dev *input;
    char phys[NAME_MAX];
    int num_prv_rec;
    int or_min;
    int or_max;
    int t_min;
    int t_max;
    int p_min;
    int p_max;
    int major_min;
    int major_max;
    int minor_min;
    int minor_max;
    int x_min;
    int x_max;
    int y_min;
    int y_max;
};

struct parade_hid_output {
    u8 cmd_type;
    u16 length;
    u8 command_code;
    size_t write_length;
    u8 *write_buf;
    u8 novalidate;
    u8 reset_expected;
    u16 timeout_ms;
};

// //////////////////////parade loader///////////
#define PT_MAX_STATUS_SIZE                32

struct touch_settings {
    const uint8_t   *data;
    uint32_t         size;
    uint8_t         tag;
};

struct pt_touch_firmware {
    const uint8_t *img;
    uint32_t size;
    const uint8_t *ver;
    uint8_t vsize;
    uint8_t panel_id;
};

struct pt_touch_config {
    struct touch_settings *param_regs;
    struct touch_settings *param_size;
    const uint8_t *fw_ver;
    uint8_t fw_vsize;
    uint8_t panel_id;
};

struct pt_loader_platform_data {
    struct pt_touch_firmware *fw;
    struct pt_touch_config *ttconfig;
    struct pt_touch_firmware **fws;
    struct pt_touch_config **ttconfigs;
    u32 flags;
};

struct parade_loader_data {
    struct device *dev;
    struct parade_sysinfo *si;
    u8 status_buf[PT_MAX_STATUS_SIZE];
    int builtin_bin_fw_status;
    bool is_manual_upgrade_enabled;
    struct work_struct fw_upgrade;
    struct parade_loader_platform_data *loader_pdata;
};
/* Mode */
#define FINGER_ONLY_MODE            1
#define FINGER_GLOVE_MODE           3
#define STYLUS_ONLY_MODE            4
#define FINGER_GLOVE_STYLUS_MODE     7

// ///////////////////device_access///////////////////////
#define PARADE_MAX_PRBUF_SIZE           PIPE_BUF
#define PARADE_MAX_CONFIG_BYTES    256
#define PARADE_TTHE_TUNER_GET_PANEL_DATA_FILE_NAME "get_panel_data"
#define PARADE_INPUT_ELEM_SZ (sizeof("0xHH") + 1)

// #ifdef TTHE_TUNER_SUPPORT
#define PT_CMD_RET_PANEL_IN_DATA_OFFSET    0
#define PT_CMD_RET_PANEL_ELMNT_SZ_MASK    0x07
#define PT_CMD_RET_PANEL_HDR        0x0A
#define PT_CMD_RET_PANEL_ELMNT_SZ_MAX    0x2

enum scan_data_type_list {
    PT_MUT_RAW,
    PT_MUT_BASE,
    PT_MUT_DIFF,
    PT_SELF_RAW,
    PT_SELF_BASE,
    PT_SELF_DIFF,
    PT_BAL_RAW,
    PT_BAL_BASE,
    PT_BAL_DIFF,
};
// #endif

#ifdef TTHE_TUNER_SUPPORT
#define PARADE_TTHE_TUNER_FILE_NAME "tthe_tuner"
#endif

// #ifdef TTHE_TUNER_SUPPORT
struct heatmap_param {
    bool scan_start;
    enum scan_data_type_list data_type; /* raw, base, diff */
    int num_element;
};
// #endif

struct result {
    int32_t sensor_assignment;
    int32_t config_ver;
    int32_t revision_ctrl;
    int32_t device_id_high;
    int32_t device_id_low;
    bool cm_test_run;
    bool cp_test_run;
    /* Sensor Cm validation */
    bool cm_test_pass;
    bool cm_sensor_validation_pass;
    bool cm_sensor_row_delta_pass;
    bool cm_sensor_col_delta_pass;
    bool cm_sensor_gd_row_pass;
    bool cm_sensor_gd_col_pass;
    bool cm_sensor_calibration_pass;
    bool cm_sensor_delta_pass;
    bool cm_button_validation_pass;
    bool cm_button_delta_pass;

    int32_t *cm_sensor_raw_data;
    int32_t cm_sensor_calibration;
    int32_t cm_sensor_delta;
    int32_t *cm_button_raw_data;
    int32_t cm_button_delta;

    /* Sensor Cp validation */
    bool cp_test_pass;
    bool cp_sensor_delta_pass;
    bool cp_sensor_rx_delta_pass;
    bool cp_sensor_tx_delta_pass;
    bool cp_sensor_average_pass;
    bool cp_button_delta_pass;
    bool cp_button_average_pass;
    bool cp_rx_validation_pass;
    bool cp_tx_validation_pass;
    bool cp_button_validation_pass;

    int32_t *cp_sensor_rx_raw_data;
    int32_t *cp_sensor_tx_raw_data;
    int32_t cp_sensor_rx_delta;
    int32_t cp_sensor_tx_delta;
    int32_t cp_sensor_rx_calibration;
    int32_t cp_sensor_tx_calibration;
    int32_t *cp_button_raw_data;
    int32_t cp_button_delta;

    /* other validation */
    bool short_test_pass;
    bool test_summary;
    uint8_t *cm_open_pwc;
    bool diff_test_pass;
    bool diff_self_rx_test_pass;
    bool diff_self_tx_test_pass;
    int32_t *sensor_diff_data;
    /* i2c */
    bool i2c_test_pass;
    /* change report rate */
    bool change_report_rate_pass;
};

#define MAX_CASE_NUM            (25)
/* ASCII */
#define ASCII_LF                (0x0A)
#define ASCII_CR                (0x0D)
#define ASCII_COMMA             (0x2C)
#define ASCII_ZERO              (0x30)
#define ASCII_NINE              (0x39)

/* Max characters of test case name */
#define NAME_SIZE_MAX           (50)
#define MAX_SENSORS             (1024)
#define MAX_BUTTONS             (HID_SYSINFO_MAX_BTN)

#define MAX_TX_SENSORS          (128)
#define MAX_RX_SENSORS          (128)

#define TABLE_BUTTON_MAX_SIZE   (MAX_BUTTONS * 2)
#define TABLE_SENSOR_MAX_SIZE   (MAX_SENSORS * 2)


#define TABLE_TX_MAX_SIZE       (MAX_TX_SENSORS*2)
#define TABLE_RX_MAX_SIZE       (MAX_RX_SENSORS*2)
#define TABLE_SENSOR_MAX_SIZE   (MAX_SENSORS * 2)

struct cmcp_data {
    struct gd_sensor *gd_sensor_col;
    struct gd_sensor *gd_sensor_row;
    int32_t *cm_data_panel;
    int32_t *cp_tx_data_panel;
    int32_t *cp_rx_data_panel;
    int32_t *cp_tx_cal_data_panel;
    int32_t *cp_rx_cal_data_panel;
    int32_t cp_sensor_rx_delta;
    int32_t cp_sensor_tx_delta;
    int32_t cp_button_delta;
    int32_t *cm_btn_data;
    int32_t *cp_btn_data;
    int32_t *cm_sensor_column_delta;
    int32_t *cm_sensor_row_delta;
    int32_t cp_btn_cal;
    int32_t cm_btn_cal;
    int32_t cp_button_ave;
    int32_t cm_ave_data_panel;
    int32_t cp_tx_ave_data_panel;
    int32_t cp_rx_ave_data_panel;
    int32_t cm_cal_data_panel;
    int32_t cm_ave_data_btn;
    int32_t cm_cal_data_btn;
    int32_t cm_delta_data_btn;
    int32_t cm_sensor_delta;
    int32_t *diff_sensor_data;
    int32_t *diff_self_data;

    int32_t tx_num;
    int32_t rx_num;
    int32_t btn_num;
};

/* cmcp csv file information */
struct configuration {
    u32 cm_range_limit_row;
    u32 cm_range_limit_col;
    u32 cm_range_limit_row_lattice[MAX_SENSORS];
    u32 cm_range_limit_col_lattice[MAX_SENSORS];
    u32 cm_min_limit_cal;
    u32 cm_max_limit_cal;
    u32 cm_max_delta_sensor_percent;
    u32 cm_max_delta_button_percent;
    u32 diff_noise_threshold;
    u32 diff_self_rx_noise_threshold;
    u32 diff_self_tx_noise_threshold;
    u32 min_sensor_rx;
    u32 max_sensor_rx;
    u32 min_sensor_tx;
    u32 max_sensor_tx;
    u32 min_button;
    u32 max_button;
    u32 max_delta_sensor;
    u32 cp_max_delta_sensor_rx_percent;
    u32 cp_max_delta_sensor_tx_percent;
    u32 cm_min_max_table_button[TABLE_BUTTON_MAX_SIZE];
    u32 cp_min_max_table_button[TABLE_BUTTON_MAX_SIZE];
    u32 cm_min_max_table_sensor[TABLE_SENSOR_MAX_SIZE];
    u32 cp_min_max_table_rx[TABLE_RX_MAX_SIZE];
    u32 cp_min_max_table_tx[TABLE_TX_MAX_SIZE];
    u32 cm_min_max_table_button_size;
    u32 cp_min_max_table_button_size;
    u32 cm_min_max_table_sensor_size;
    u32 cp_min_max_table_rx_size;
    u32 cp_min_max_table_tx_size;
    u32 cp_max_delta_button_percent;
    u32 cm_max_table_gradient_cols_percent[TABLE_TX_MAX_SIZE];
    u32 cm_max_table_gradient_cols_percent_size;
    u32 cm_max_table_gradient_rows_percent[TABLE_RX_MAX_SIZE];
    u32 cm_max_table_gradient_rows_percent_size;
    u32 cm_excluding_row_edge;
    u32 cm_excluding_col_edge;
    u32 rx_num;
    u32 tx_num;
    u32 btn_num;
    u32 cm_enabled;
    u32 cp_enabled;
    u32 is_valid_or_not;
};

/* Test case search definition */
struct test_case_search {
    char name[NAME_SIZE_MAX]; /* Test case name */
    u32 name_size;            /* Test case name size */
    u32 offset;               /* Test case offset */
};

/* Test case field definition */
struct test_case_field {
    char *name;     /* Test case name */
    u32 name_size;  /* Test case name size */
    u32 type;       /* Test case type */
    u32 *bufptr;    /* Buffer to store value information */
    u32 exist_or_not; /* Test case exist or not */
    u32 data_num;   /* Buffer data number */
    u32 line_num;   /* Buffer line number */
};

/* Test case type */
enum test_case_type {
    TEST_CASE_TYPE_NO,
    TEST_CASE_TYPE_ONE,
    TEST_CASE_TYPE_MUL,
    TEST_CASE_TYPE_MUL_LINES,
};

/* Test case order in test_case_field_array */
enum case_order {
    CM_TEST_INPUTS,
    CM_EXCLUDING_COL_EDGE,
    CM_EXCLUDING_ROW_EDGE,
    CM_GRADIENT_CHECK_COL,
    CM_GRADIENT_CHECK_ROW,
    DIFF_NOISE_THRESHOLD,
    DIFF_SELF_RX_NOISE_THRESHOLD,
    DIFF_SELF_TX_NOISE_THRESHOLD,
    CM_RANGE_LIMIT_ROW,
    CM_RANGE_LIMIT_COL,
    CM_MIN_LIMIT_CAL,
    CM_MAX_LIMIT_CAL,
    CM_MAX_DELTA_SENSOR_PERCENT,
    CM_MAX_DELTA_BUTTON_PERCENT,
    PER_ELEMENT_MIN_MAX_TABLE_BUTTON,
    PER_ELEMENT_MIN_MAX_TABLE_SENSOR,
    CP_TEST_INPUTS,
    CP_MAX_DELTA_SENSOR_RX_PERCENT,
    CP_MAX_DELTA_SENSOR_TX_PERCENT,
    CP_MAX_DELTA_BUTTON_PERCENT,
    CP_PER_ELEMENT_MIN_MAX_BUTTON,
    MIN_BUTTON,
    MAX_BUTTON,
    PER_ELEMENT_MIN_MAX_RX,
    PER_ELEMENT_MIN_MAX_TX,
    CASE_ORDER_MAX,
};

struct parade_device_access_data {
    struct device *dev;
    struct parade_sysinfo *si;
    struct mutex sysfs_lock;
    u8 status;
    u16 response_length;
    bool sysfs_nodes_created;
    struct kobject mfg_test;
    u8 panel_scan_data_id;
    u8 get_idac_data_id;
    u8 calibrate_sensing_mode;
    u8 calibrate_initialize_baselines;
    u8 baseline_sensing_mode;

    struct heatmap_param heatmap;
#ifdef TTHE_TUNER_SUPPORT
    struct dentry *tthe_get_panel_data_debugfs;
    struct mutex debugfs_lock;
    u8 tthe_get_panel_data_buf[TTHE_TUNER_MAX_BUF];
    u8 tthe_get_panel_data_is_open;
#endif
    u8 ic_buf[PARADE_MAX_PRBUF_SIZE];
    u8 response_buf[PARADE_MAX_PRBUF_SIZE];
    struct configuration *configs;
    struct cmcp_data *cmcp_info;
    struct result *result;
    struct test_case_search *test_search_array;
    struct test_case_field *test_field_array;
    struct work_struct cmcp_test;
};

// /////////////////////////////////////////////

struct gd_sensor {
    int32_t cm_min;
    int32_t cm_max;
    int32_t cm_ave;
    int32_t cm_min_exclude_edge;
    int32_t cm_max_exclude_edge;
    int32_t cm_ave_exclude_edge;
    int32_t gradient_val;
};

enum parade_self_test_id {
    PT_ST_ID_NULL,
    PT_ST_ID_BIST,
    PT_ST_ID_SHORTS,
    PT_ST_ID_OPENS,
    PT_ST_ID_AUTOSHORTS,
    PT_ST_ID_CM_PANEL,
    PT_ST_ID_CP_PANEL,
    PT_ST_ID_CM_BUTTON,
    PT_ST_ID_CP_BUTTON,
};

struct parade_nv_info {
    u16 flash_size;
    u8 row0_set;
    u8 row1_set;
    u8 row2_set;
    u8 row3_set;
};

enum parade_cmd_status {
    PT_CMD_STATUS_SUCCESS,
    PT_CMD_STATUS_FAILURE,
};
struct parade_oem_data;
struct parade_oem_ops {
    int (*suspend)(struct parade_oem_data *parade_oem_data);
    int (*resume)(struct parade_oem_data *parade_oem_data);
    int (*write_nv_data)(struct parade_oem_data *parade_oem_data, u8 buf[], int offset, int length);
    int (*read_nv_data)(struct parade_oem_data *parade_oem_data, u8 buf[], int offset, int length);
    int (*get_nv_info)(struct parade_oem_data *parade_oem_data);
    // int (*hid_send_output_and_wait)(struct parade_oem_data *parade_oem_data, struct parade_hid_output *hid_output);
    int (*hid_send_output_and_wait)(struct parade_oem_data *parade_oem_data, struct parade_hid_output *hid_output);

};

enum pt_selftest_result {
    PT_SELFTEST_STATUS_SUCCESS,
    PT_SELFTEST_STATUS_FAIL
};

enum pt_calibrate_mode {
    PT_CAL_IDAC_MUTUAL,
    PT_CAL_IDAC_BUTTON,
    PT_CAL_IDAC_SELF,
    PT_CAL_IDAC_MUTUAL_WITHOUT_HOP,
    PT_CAL_IDAC_ONLY
};

enum parade_self_test_result {
    PT_ST_RESULT_PASS,
    PT_ST_RESULT_FAIL,
    PT_ST_RESULT_HOST_MUST_INTERPRET = 0xFF,
};
enum parade_initialize_baselines_sensing_mode {
    PT_IB_SM_MUTCAP = 1,
    PT_IB_SM_BUTTON = 2,
    PT_IB_SM_SELFCAP = 4,
};
#define RETRY_OR_EXIT(retry_cnt, retry_label, exit_label) \
	do { \
		if (retry_cnt) \
		goto retry_label; \
		goto exit_label; \
	} while (0)


struct parade_oem_data {
    struct parade_oem_ops ops;
    struct parade_nv_info info;
    bool oem_info_ready;
    int flash_size;
    u8 *stored_flash_data;
};
// ///////////////////////////////////////
#define CM_PANEL_DATA_OFFSET    (6)
#define CM_BTN_DATA_OFFSET      (6)
#define CP_PANEL_DATA_OFFSET    (6)
#define CP_BTN_DATA_OFFSET      (6)
#define MAX_BUF_LEN             (50000)
#define TEMP_MAX_STR_LEN 128

enum TS_dsm_status {
    FWU_START_UP_FAIL = 0,
    FWU_GET_SYSINFO_FAIL,
    FWU_GET_DESC_FAIL,
    FWU_OPEN_FILE_FAIL,
    FWU_GENERATE_FW_NAME_FAIL,
    FWU_REQUEST_FW_FAIL,
    FWU_FW_CONT_BUILTIN_ERROR,
    FWU_FW_CONT_ERROR,
    FWU_REQUEST_EXCLUSIVE_FAIL,
    TS_WAKEUP_FAIL,
    TS_UPDATE_STATE_UNDEFINE = 255,
};

#define PT_DOUBULE_TAB_POINT_NUM 2
struct parade_gesture_position {
    unsigned int x;
    unsigned int y;
};
struct parade_double_tap_info {
    unsigned short id;
    unsigned int dtz_x0;
    unsigned int dtz_x1;
    unsigned int dtz_y0;
    unsigned int dtz_y1;
    struct parade_gesture_position postion[PT_DOUBULE_TAB_POINT_NUM];
};

struct parade_core_data {
    u8 panel_id;
    u8 cmd_buf[PARADE_PREALLOCATED_CMD_BUFFER];
    u8 input_buf[PT_MAX_INPUT];
    u8 response_buf[PT_MAX_INPUT];
    u16 startup_retry_count;
    int hid_cmd_state;
    int hid_reset_cmd_state; /* reset can happen any time */
    int num_hid_reports;
    wait_queue_head_t wait_q;
    wait_queue_head_t wait_q_bootup;
    int command_finished;
    enum parade_process_state process_state;
    enum parade_sleep_state sleep_state;
    enum parade_startup_state startup_state;
    spinlock_t spinlock;

    struct device *dev;
    struct ts_kit_device_data *parade_chip_data;
    struct platform_device *parade_dev;
    struct input_dev *input;
    struct regulator *vddd;
    struct regulator *vdda;
    struct list_head param_list;
    struct mutex system_lock;
    struct mutex hid_report_lock;
    bool force_fw_update;

    struct parade_mt_data md;
    struct parade_loader_data ld;
    struct parade_device_access_data dad;
    struct parade_core_platform_data *core_pdata;
    struct parade_mt_platform_data *mt_pdata;
    struct parade_loader_platform_data *loader_pdata;
    struct parade_oem_data parade_oem_data;
    struct notifier_block pm_notifier;
    struct parade_sysinfo sysinfo;
    struct parade_hid_core hid_core;
    struct parade_hid_desc hid_desc;
    struct parade_hid_report *hid_reports[PT_HID_MAX_REPORTS];
    struct parade_features features;
    struct firmware *fw_entry;

    bool isworkongoing;
    enum ts_cmd cmd_background;

    int    parade_power_ctrl_param;

#ifdef CONFIG_HAS_EARLYSUSPEND
    struct early_suspend es;
#elif defined(CONFIG_FB)
    struct notifier_block fb_notifier;
    enum parade_fb_state fb_state;
#endif
    bool force_wait;
    u8 pr_buf[PT_MAX_PRBUF_SIZE];
    u8 knuckle_data[PARADE_ROI_DATA_SEND_LENGTH];

#ifdef TTHE_TUNER_SUPPORT
    struct dentry *tthe_debugfs;
    u8 *tthe_buf;
    u32 tthe_buf_len;
    struct mutex tthe_lock;
    u8 tthe_exit;
#endif
    struct cmcp_data *cmcp_info;
#ifdef PARADE_ROI_ENABLE
    u8 roi_switch;
    struct ts_roi_info *roi_info;
#endif
    int pinctrl_set;
    struct pinctrl *pctrl;
    struct pinctrl_state *pins_default;
    struct pinctrl_state *pins_idle;

    int self_ctrl_power;
    int sleep_in_mode;

    enum parade_mode mode;

    struct parade_double_tap_info *double_tab_info;

    char *fw_and_config_name;
    char  project_id[MAX_STR_LEN];
    char  module_vendor[MAX_STR_LEN];
    char  lcd_module_name[MAX_STR_LEN];
    char  chip_name[MAX_STR_LEN];
    bool  is_need_check_mode_before_status_resume;

    int create_project_id_flag;
    int need_provide_projectID_for_sensor;
    int get_module_name_flag;
    int cm_delta_lattice_flag;
    u32 pt_hid_reset_timeout; /* get from dts setting for ESD reset time */
    /* for tp firmware broken */
    bool is_firmware_broken;
    bool is_firmware_restore;
    bool firmware_broken_update_flag; /* for limit firmware update number of times */
    char temporary_firmware_name[TEMP_MAX_STR_LEN];
    bool need_upgrade_again;
    bool need_distinguish_lcd; /* for agassi distinguish lcd */
    bool need_wait_after_cm_test;    /* for cm test,some waiting is not enough */
    bool support_get_tp_color; /* for tp color */
    u16 pt_hid_output_timeout;
    /* for mmi test item 4 report rate check support */
    int report_rate_check_supported;
    int self_noise_supported;
    int current_distinguishability; // false :use default distinguishability,true :second distinguishability
    int fw_only_depend_on_lcd ;; // 0 : fw depend on TP and others ,1 : fw only depend on lcd.
    char lcd_panel_info[LCD_PANEL_INFO_MAX_LEN] ;
    char *sign_of_second_distinguishability;
    /* after resume flag, do nothing in after resume when it is 1 */
    int no_need_after_resume_flag;
    int cp_delta_test_supported; /* self capacity delta test support */
    /* calibrate_fw_update_and_cmcp = 1, means do calibrate after fw update and cmcp */
    int calibrate_fw_update_and_cmcp;
    int show_before_suspend_flag;
    int avoid_roi_switch_flag;
    bool need_set_rst_after_iovcc_flag;
    bool need_check_report_descriptor_flag;
    bool fw_need_depend_on_lcd;
    bool need_delay_after_power_off;
    bool touch_report_with_grip;
    int check_fw_right_flag;
};

#endif

