#ifndef POWER_MESG_SRV_H
#define POWER_MESG_SRV_H

enum {
    POWER_GENL_UNUSED = 0, /* 0 is invalid attr type for kernel in nla_parse*/
    POWER_GENL_ATTR0,
    POWER_GENL_ATTR1,
    __POWER_GENL_ATTR_NUM,
};
#define TOTAL_POWER_GENL_ATTR       (__POWER_GENL_ATTR_NUM - 1)
#define BATT_INFO_DEVICE_ATTR       POWER_GENL_ATTR0
#define BATT_INFO_DATA_ATTR         POWER_GENL_ATTR1
#define POWER_GENL_RAW_DATA_ATTR    POWER_GENL_ATTR1
#define POWER_GENL_MAX_ATTR_INDEX   (__POWER_GENL_ATTR_NUM - 1)

typedef enum {
    CT_PREPARE = 0,
    SN_PREPARE,
    //MAX is 255
}batt_info_subcmd_t;

typedef struct {
    unsigned char id_in_grp;
    unsigned char chks_in_grp;
    unsigned char id_of_grp;
    unsigned char ic_type;
    unsigned char subcmd;
} nl_dev_info;

typedef enum {
    LOCAL_IC_TYPE = 0,
    MAXIM_DS28EL15_TYPE = 1,
    MAXIM_DS28EL16_TYPE = 2,
    NXP_A1007_TYPE = 3,
    /* should not larger than 255 */
} batt_ic_type;

typedef struct {
    const unsigned char *data;
    unsigned int len;
    int type;
} resource;

/* cmd: 00-49 is for battery
 *      50-99 is for wireless charging
 *      100-149 is for directly charging
 *      max is 255
 */
typedef enum {
    BATT_INFO_CMD = 0,
	BATT_DMD_CMD = 10,
//    BATT_MAXIM_EEPROM_MAC,
//    BATT_MAXIM_STATUS_MAC,
    BOARD_INFO_CMD = 40,
    BATT_FINAL_RESULT_CMD = 49,
    POWER_CMD_ADAPTOR_ANTIFAKE_HASH = 50,
    POWER_CMD_WC_ANTIFAKE_HASH = 60,
    POWER_CMD_TOTAL_NUM = 256,
} power_genl_cmd_t;

enum RESULT_STATUS {
    FINAL_RESULT_PASS = 0,
    FINAL_RESULT_CRASH,
    FINAL_RESULT_FAIL,
    __FINAL_RESULT_MAX,
};

enum {
    NEW_BOARD_MESG_INDEX = 0,
    OLD_BOARD_MESG_INDEX,
};

#ifdef BATTCT_KERNEL_SRV_SHARE_VAR
const char* result_status_mesg[] = {
    [FINAL_RESULT_PASS]  = "AuthenticationPass_",
    [FINAL_RESULT_CRASH] = "AuthenticationCrash",
    [FINAL_RESULT_FAIL]  = "AuthenticationFail_",
    /* all element should be same strlen */
};
const char* board_info_mesg = {
    "BoardInformationMessage",
};
const char* board_info_cb_mesg[] = {
    [NEW_BOARD_MESG_INDEX] = "IFeelYong",
    [OLD_BOARD_MESG_INDEX] = "IFeelOld_",
};
#endif

#endif