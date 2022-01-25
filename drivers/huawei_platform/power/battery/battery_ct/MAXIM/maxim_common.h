#ifndef MAXIM_ONEWIRE_H
#define MAXIM_ONEWIRE_H

#include <linux/wakelock.h>
#include <huawei_platform/power/batt_info_pub.h>
#include "onewire_common.h"
#include "onewire_phy_common.h"
#include "../batt_aut_checker.h"


//random challenge length
#define RANDOM_NUM_BYTES	32

/* Command success response */
#define MAXIM_ONEWIRE_COMMAND_SUCCESS		0xAA

/* CRC result */
#define MAXIM_CRC16_RESULT				0xB001
#define MAXIM_CRC8_RESULT					0

//sn config
#define SN_LENGTH_BITS						108
#define PRINTABLE_SN_SIZE					17
#define SN_CHAR_PRINT_SIZE					11
#define SN_HEX_PRINT_SIZE					5

#define BATTERY_CELL_FACTORY				4
#define BATTERY_PACK_FACTORY				5

//basic
#define BYTES2BITS(x)			((x)<<3)
#define IS_ODD(a)				((a) & 0x1)
#define IS_EVEN(a)				(!IS_ODD(a))
#define NOT_MUTI8(a)			((a) & 0x7)
#define IS_MUTI8(a)				(!NOT_MUTI8(a))
#define DOUBLE(x)				((x)<<1)

/* MAXIM 1-wire rom function command */
#define READ_ROM							0x33
#define MATCH_ROM							0x55
#define SEARCH_ROM							0xF0
#define SKIP_ROM							0xCC
#define RESUME_COMMAND					0xA5


/* 1-wire function operation return signals */
#define MAXIM_ONEWIRE_SUCCESS			ONEWIRE_SUCCESS
#define MAXIM_ERROR_BIT					0x80
#define MAXIM_ONEWIRE_DTS_ERROR			(MAXIM_ERROR_BIT | \
	ONEWIRE_DTS_FAIL)
#define MAXIM_ONEWIRE_COM_ERROR			(MAXIM_ERROR_BIT | \
	ONEWIRE_COM_FAIL)
#define MAXIN_ONEWIRE_CRC8_ERROR		(MAXIM_ERROR_BIT | \
	ONEWIRE_CRC8_FAIL)
#define MAXIN_ONEWIRE_CRC16_ERROR		(MAXIM_ERROR_BIT | \
	ONEWIRE_CRC16_FAIL)
#define MAXIM_ONEWIRE_NO_SLAVE			(MAXIM_ERROR_BIT | \
	ONEWIRE_NO_SLAVE)
#define MAXIM_ONEWIRE_ILLEGAL_PARAM		(MAXIM_ERROR_BIT | \
	ONEWIRE_ILLEGAL_PARAM)

//retry times config
#define SET_SRAM_RETRY						4
#define GET_USER_MEMORY_RETRY			8
#define GET_PERSONALITY_RETRY				8
#define GET_ROM_ID_RETRY					8
#define GET_BLOCK_STATUS_RETRY			8
#define SET_BLOCK_STATUS_RETRY			8
#define GET_MAC_RETRY						8

#define CURRENT_MAXIM_TASK				0

//hmac
#define MAX_MAC_SOURCE_SIZE				128

//maxim return value
#define MAXIM_SUCCESS						0
#define MAXIM_FAIL							1

/*
 * for ds28el16 1page == 1block
 * for ds28el15 1page == 2block
 */
struct maxim_onewire_mem {
	unsigned char *block_status;
	unsigned char *rom_id;
	unsigned char *pages;
	unsigned char *mac;
	unsigned char *sn;
	unsigned char *mac_datum;
	unsigned char main_id;
	unsigned char version;
	unsigned int  ic_type;
	unsigned int  rom_id_length;
	unsigned int  page_number;
	unsigned int  page_size;
	unsigned int  block_number;
	unsigned int  block_size;
	unsigned int  mac_length;
	unsigned int  sn_length_bits;
	unsigned int  sn_page;
	unsigned int  sn_offset_bits;
	unsigned char batt_type[BATTERY_TYPE_BUFF_SIZE];
	//protocol process mutual exclusion
	struct mutex  lock;
};

struct maxim_onewire_time_request {
	onewire_time_request ow_trq;
	/*dts node--read-time-request*/
	unsigned int t_read;
	/*dts node--programe-time*/
	unsigned int t_write_memory;
	/*dts node--write-status-time*/
	unsigned int t_write_status;
	/*dts node--compute-mac-time*/
	unsigned int t_compute;
};


struct maxim_onewire_des {
	struct maxim_onewire_time_request trq;
	struct maxim_onewire_mem memory;
	onewire_phy_ops phy_ops;
};

struct maxim_onewire_com_stat {
	unsigned int   cmds;
	unsigned int *totals;
	unsigned int *errs;
	const char **cmd_str;
};

struct battery_constraint {
	unsigned char *id_mask;
	unsigned char *id_example;
	unsigned char *id_chk;
};

struct maxim_onewire_drv_data {
	struct maxim_onewire_des ic_des;
	struct battery_constraint batt_cons;
	struct maxim_onewire_com_stat com_stat;
	struct attribute_group *attr_group;
	const struct attribute_group **attr_groups;
	struct wake_lock write_lock;
	struct mutex  batt_safe_info_lock;
	ct_ops_reg_list ct_node;
	unsigned char random_bytes[RANDOM_NUM_BYTES];
};

//mac mem
#define PUT_MAC_DATA(drv_data, ic_mac) do { \
	memcpy((drv_data)->ic_des.memory.mac + 1, \
		(ic_mac), (drv_data)->ic_des.memory.mac_length); \
	*((drv_data)->ic_des.memory.mac) = 1; \
} while (0)

#define GET_MAC_DATA(drv_data) ((drv_data)->ic_des.memory.mac + 1)
 #define GET_MAC_DATA_LEN(drv_data) ((drv_data)->ic_des.memory.mac_length)
 #define CHECK_MAC_DATA_SIGN(drv_data) (1 == *((drv_data)->ic_des.memory.mac))
 #define CLEAR_MAC_DATA_SIGN(drv_data) (*((drv_data)->ic_des.memory.mac) = 0)
 #define SET_MAC_DATA_SIGN(drv_data) (*((drv_data)->ic_des.memory.mac) = 1)

//rom id mem
#define PUT_ROM_ID(drv_data, id) do { \
	memcpy((drv_data)->ic_des.memory.rom_id + 1, \
		id, (drv_data)->ic_des.memory.rom_id_length); \
	*((drv_data)->ic_des.memory.rom_id) = 1; \
} while (0)

#define GET_ROM_ID(drv_data)  ((drv_data)->ic_des.memory.rom_id + 1)
#define GET_ROM_ID_LEN(drv_data)  ((drv_data)->ic_des.memory.rom_id_length)
#define CHECK_ROM_ID_SIGN(drv_data) (1 == *((drv_data)->ic_des.memory.rom_id))
#define ClEAR_ROM_ID_SIGN(drv_data) (*((drv_data)->ic_des.memory.rom_id) = 0)
#define SET_ROM_ID_SIGN(drv_data) (*((drv_data)->ic_des.memory.rom_id) = 1)

 //block status mem
#define PUT_BLOCK_STATUS(drv_data, id) do { \
	memcpy((drv_data)->ic_des.memory.block_status + 1, \
		id, (drv_data)->ic_des.memory.block_number); \
	*((drv_data)->ic_des.memory.block_status) = 1; \
} while (0)

#define PUT_ONE_BLOCK_STATUS(drv_data, no, status) do { \
	*((drv_data)->ic_des.memory.block_status + 1 + no) = status; \
	*((drv_data)->ic_des.memory.block_status) = 1; \
} while (0)

#define GET_BLOCK_STATUS(drv_data)  ((drv_data)->ic_des.memory.block_status + 1)
#define CHECK_BLOCK_STATUS_SIGN(drv_data) \
	(1 == *((drv_data)->ic_des.memory.block_status))
#define CLEAR_BLOCK_STATUS_SIGN(drv_data) \
	(*((drv_data)->ic_des.memory.block_status) = 0)
#define SET_BLOCK_STATUS_SIGN(drv_data) \
	(*((drv_data)->ic_des.memory.block_status) = 1)

#define GET_BLOCK_NUM(drv_data)  ((drv_data)->ic_des.memory.block_number)
#define GET_PAGE_NUM(drv_data)  ((drv_data)->ic_des.memory.page_number)


//page data mem
#define GET_PAGE_SIZE(drv_data) ((drv_data)->ic_des.memory.page_size)
#define GET_PAGE_DATA(drv_data, page_no) \
	((((GET_PAGE_SIZE(drv_data) + 1) * (page_no)) + \
	(drv_data)->ic_des.memory.pages) + 1)
#define PUT_PAGE_DATA(drv_data, page_no, data) do { \
	memcpy(GET_PAGE_DATA(drv_data, page_no), data, \
		GET_PAGE_SIZE(drv_data)); \
	*(GET_PAGE_DATA(drv_data, page_no) - 1) = 1; \
} while (0)
#define CHECK_PAGE_DATA_SIGN(drv_data, page_no) \
	(1 == *(GET_PAGE_DATA(drv_data, page_no) - 1))
#define CLEAR_PAGE_DATA_SIGN(drv_data, page_no) \
	(*(GET_PAGE_DATA(drv_data, page_no) - 1) = 0)
#define SET_PAGE_DATA_SIGN(drv_data, page_no) \
	(*(GET_PAGE_DATA(drv_data, page_no) - 1) = 1)

//man id
#define PUT_MAN_ID(drv_data, id) \
	((drv_data)->ic_des.memory.main_id = (id))
#define GET_MAIN_ID(drv_data) ((drv_data)->ic_des.memory.main_id)

//version
#define PUT_VERSION(drv_data, ver) \
	((drv_data)->ic_des.memory.version = (ver))
#define GET_VERSION(drv_data) ((drv_data)->ic_des.memory.version)

//com state
#define INCR_TOTAL_COM_STAT(drv_data, index) do { \
	if ((drv_data)->com_stat.totals && \
		((index) < (drv_data)->com_stat.cmds)) { \
		((drv_data)->com_stat.totals[index])++; \
	} \
} while (0)
#define INCR_ERR_COM_STAT(drv_data, index) do { \
	if ((drv_data)->com_stat.errs && \
		((index) < (drv_data)->com_stat.cmds)) { \
		((drv_data)->com_stat.errs[index])++; \
	} \
} while (0)


/* Slave presence signal is low */
#define NO_SLAVE_RESPONSE(x)		  ((x) != 0)

/* detect onewire slaves and notify all slaves */
#define ONEWIRE_NOTIFY_ALL_SLAVES(x) do { \
	if (NO_SLAVE_RESPONSE(x->reset())) { \
		hwlog_err("maxim: no slave response, found in %s\n", \
			__func__); \
		return MAXIM_ONEWIRE_NO_SLAVE; \
	} \
	x->write_byte(SKIP_ROM); \
} while (0)

/* Maxim CRC16 check */
#define DO_CRC16(a, b) do { \
	if (check_crc16(a, b) != MAXIM_CRC16_RESULT) { \
		hwlog_err("maxim: CRC16 failed in %s\n",  __func__); \
		return MAXIN_ONEWIRE_CRC16_ERROR; \
	} else { \
		hwlog_info("maxim: CRC16 success in %s\n", __func__); \
	} \
} while (0)

/* Maxim CRC8 check */
#define CRC8_ERROR_PROCESS(x) do { \
	if (x != MAXIM_CRC8_RESULT) { \
		hwlog_err("maxim: CRC8 failed in %s\n", __func__); \
		return MAXIN_ONEWIRE_CRC8_ERROR; \
	} \
} while (0)

/* Command response process */
#define MAXIM_COMMAND_FAIL_PROCESS(x) do { \
	if ((x) != MAXIM_ONEWIRE_COMMAND_SUCCESS) { \
		hwlog_err("maxim: Error command indicator %x in %s\n", \
			(x), __func__); \
		return MAXIM_ONEWIRE_COM_ERROR; \
	} \
} while (0)

/* dts read property error process*/
#define MAXIM_ONEWIRE_DTS_READ_ERROR_RETURN(x, y)  do { \
	if (x) { \
		hwlog_err("DTS do not have "y", needed in %s.\n",  \
			__func__); \
		return MAXIM_ONEWIRE_DTS_ERROR; \
	} \
} while (0)

/* NULL pointer process*/
#define MAXIM_ONEWIRE_NULL_POINT_RETURN(x) do { \
	if (!x) { \
		hwlog_err("NULL point: "#x", found in %s.", __func__); \
		return -1; \
	} \
} while (0)

/* onewire communication error process*/
#define MAXIM_ONEWIRE_COMMUNICATION_INFO(x, y) do { \
	if (x) { \
		hwlog_info(y" failed(%x) in %s.", x, __func__); \
	} \
} while (0)


unsigned char check_crc8(unsigned char *data, int length);
unsigned short check_crc16(unsigned char *check_data, int length);
void log_info_arry(char *msg, unsigned char *array, int arry_len);
int   snprintf_array(unsigned char *buf, int buf_len,
	unsigned char *array, int arry_len);
void set_sched_affinity_to_current(void);
void set_sched_affinity_to_all(void);

int  maxim_check_rom_id_format(struct maxim_onewire_drv_data *drv_data);
int  maxim_drv_data_init(struct maxim_onewire_drv_data *drv_data,
	struct platform_device *pdev, int cmds, const char **cmd_str);
int  maxim_dev_sys_node_init(struct maxim_onewire_drv_data *drv_data,
	struct platform_device *pdev, const struct attribute **attrs);
void maxim_destroy_drv_data(struct maxim_onewire_drv_data *ds_drv,
	struct platform_device *pdev);
void maxim_parise_printable_sn(unsigned char *page,
	unsigned int  sn_offset_bits, unsigned char *sn);

#endif

