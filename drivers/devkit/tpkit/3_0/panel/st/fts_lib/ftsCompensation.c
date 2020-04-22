/*
  *
  **************************************************************************
  **                        STMicroelectronics				**
  **************************************************************************
  **                        marco.cali@st.com				**
  **************************************************************************
  *                                                                        *
  *               FTS functions for getting Initialization Data		 *
  *                                                                        *
  **************************************************************************
  **************************************************************************
  *
  */
/*!
  * \file ftsCompensation.c
  * \brief Contains all the function to work with Initialization Data
  */

#include "ftsCompensation.h"
#include "ftsCore.h"
#include "ftsError.h"
#include "ftsFrame.h"
#include "ftsHardware.h"
#include "ftsIO.h"
#include "ftsSoftware.h"
#include "ftsTime.h"
#include "ftsTool.h"


#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <stdarg.h>
#include <linux/serio.h>
#include <linux/time.h>
#include <linux/delay.h>
#include <linux/ctype.h>


/**
  * Request to the FW to load the specified Initialization Data
  * @param type type of Initialization data to load @link load_opt Load Host
  * Data Option @endlink
  * @return OK if success or an error code which specify the type of error
  */
int requestCompensationData(u8 type)
{
	int ret = ERROR_OP_NOT_ALLOW;
	int retry = 0;

	TS_LOG_INFO( "%s: Requesting compensation data... attemp = %d\n", __func__, retry + 1);
	while (retry < RETRY_COMP_DATA_READ) {
		ret = writeSysCmd(SYS_CMD_LOAD_DATA,  &type, 1);
		/* send request to load in memory the Compensation Data */
		if (ret < OK) {
			TS_LOG_ERR( "%s: failed at %d attemp!\n", __func__, retry + 1);
			retry += 1;
		} else {
			TS_LOG_INFO("%s: Request Compensation data FINISHED!\n", __func__);
			return OK;
		}
	}

	TS_LOG_ERR("%s: Requesting compensation data... ERROR %08X\n", __func__,
		 ret | ERROR_REQU_COMP_DATA);
	return ret | ERROR_REQU_COMP_DATA;
}


/**
  * Read Initialization Data Header and check that the type loaded match
  * with the one previously requested
  * @param type type of Initialization data requested @link load_opt Load Host
  * Data Option @endlink
  * @param header pointer to DataHeader variable which will contain the header
  * @param address pointer to a variable which will contain the updated address
  * to the next data
  * @return OK if success or an error code which specify the type of error
  */
int readCompensationDataHeader(u8 type, DataHeader *header, u64 *address)
{
	u64 offset = ADDR_FRAMEBUFFER;
	u8 data[COMP_DATA_HEADER] = {0};
	int ret = 0;

	ret = fts_writeReadU8UX(FTS_CMD_FRAMEBUFFER_R, BITS_16, offset, data,
				COMP_DATA_HEADER, DUMMY_FRAMEBUFFER);
	if (ret < OK) {	/* i2c function have already a retry mechanism */
		TS_LOG_ERR("%s: error while reading data header ERROR %08X\n",__func__, ret);
		return ret;
	}

	TS_LOG_INFO( "%s Read Data Header done!\n", __func__);

	if (data[0] != HEADER_SIGNATURE) {
		TS_LOG_ERR("%s The Header Signature was wrong! %02X != %02X ERROR %08X\n",
			  __func__, data[0], HEADER_SIGNATURE, ERROR_WRONG_DATA_SIGN);
		return ERROR_WRONG_DATA_SIGN;
	}


	if (data[1] != type) {
		TS_LOG_ERR("%s: Wrong type found! %02X!=%02X ERROR %08X\n",
			  __func__, data[1], type, ERROR_DIFF_DATA_TYPE);
		return ERROR_DIFF_DATA_TYPE;
	}

	TS_LOG_INFO( "%s Type = %02X of Compensation data OK!\n", __func__, type);

	header->type = type;

	*address = offset + COMP_DATA_HEADER;

	return OK;
}


/**
  * Read MS Global Initialization data from the buffer such as Cx1
  * @param address pointer to a variable which contain the address from where
  * to read the data and will contain the updated address to the next data
  * @param global pointer to MutualSenseData variable which will contain the MS
  * initialization data
  * @return OK if success or an error code which specify the type of error
  */
int readMutualSenseGlobalData(u64 *address, MutualSenseData *global)
{
	u8 data[COMP_DATA_GLOBAL] = {0};
	int ret = 0;

	TS_LOG_INFO( "%s Address for Global data= %08llX\n", __func__, *address);

	ret = fts_writeReadU8UX(FTS_CMD_FRAMEBUFFER_R, BITS_16, *address, data,
				COMP_DATA_GLOBAL, DUMMY_FRAMEBUFFER);
	if (ret < OK) {
		TS_LOG_ERR( "%s: error while reading info data ERROR %08X\n", __func__, ret);
		return ret;
	}
	TS_LOG_INFO( "%s Global data Read !\n", __func__);

	global->header.force_node = data[0];
	global->header.sense_node = data[1];
	global->cx1 = data[2];
	/* all other bytes are reserved atm */

	TS_LOG_INFO( "%s force_len = %d sense_len = %d CX1 = %d\n", __func__,
		 global->header.force_node, global->header.sense_node,
		 global->cx1);

	*address += COMP_DATA_GLOBAL;
	return OK;
}


/**
  * Read MS Initialization data for each node from the buffer
  * @param address a variable which contain the address from where to read the
  * data
  * @param node pointer to MutualSenseData variable which will contain the MS
  * initialization data
  * @return OK if success or an error code which specify the type of error
  */
int readMutualSenseNodeData(u64 address, MutualSenseData *node)
{
	int ret = 0;
	int size = node->header.force_node * node->header.sense_node;

	TS_LOG_INFO( "%s Address for Node data = %08llX\n", __func__, address);
	if(size <= 0){
		TS_LOG_ERR("%s:input error.",__func__);
		return ERROR_ALLOC;		
	}
	node->node_data = (i8 *)kmalloc(size * (sizeof(i8)), GFP_KERNEL);

	if (node->node_data == NULL) {
		TS_LOG_ERR("%s:can not allocate node_data... ERROR %08X",__func__, ERROR_ALLOC);
		return ERROR_ALLOC;
	}

	TS_LOG_INFO( "%s Node Data to read %d bytes\n", __func__, size);
	ret = fts_writeReadU8UX(FTS_CMD_FRAMEBUFFER_R, BITS_16, address,
				node->node_data, size, DUMMY_FRAMEBUFFER);
	if (ret < OK) {
		TS_LOG_ERR("%s: error while reading node data ERROR %08X\n",__func__, ret);
		kfree(node->node_data);
		node->node_data = NULL;
		return ret;
	}
	node->node_data_size = size;

	TS_LOG_INFO( "%s Read node data OK!\n", __func__);

	return size;
}

/**
  * Perform all the steps to read the necessary info for MS Initialization data
  * from the buffer and store it in a MutualSenseData variable
  * @param type type of MS Initialization data to read @link load_opt Load Host
  * Data Option @endlink
  * @param data pointer to MutualSenseData variable which will contain the MS
  * initialization data
  * @return OK if success or an error code which specify the type of error
  */
int readMutualSenseCompensationData(u8 type, MutualSenseData *data)
{
	int ret = 0;
	u64 address = 0;

	data->node_data = NULL;

	if (!(type == LOAD_CX_MS_TOUCH || type == LOAD_CX_MS_LOW_POWER ||
	      type == LOAD_CX_MS_KEY || type == LOAD_CX_MS_FORCE)) {
		TS_LOG_ERR("%s: Choose a MS type of compensation data ERROR %08X\n",
			  __func__, ERROR_OP_NOT_ALLOW);
		return ERROR_OP_NOT_ALLOW;
	}

	ret = requestCompensationData(type);
	if (ret < 0) {
		TS_LOG_ERR("%s: ERROR %08X\n",  __func__,ERROR_REQU_COMP_DATA);
		return ret | ERROR_REQU_COMP_DATA;
	}

	ret = readCompensationDataHeader(type, &(data->header), &address);
	if (ret < 0) {
		TS_LOG_ERR("%s: ERROR %08X\n", __func__,ERROR_COMP_DATA_HEADER);
		return ret | ERROR_COMP_DATA_HEADER;
	}

	ret = readMutualSenseGlobalData(&address, data);
	if (ret < 0) {
		TS_LOG_ERR("%s: ERROR %08X\n", __func__,ERROR_COMP_DATA_GLOBAL);
		return ret | ERROR_COMP_DATA_GLOBAL;
	}

	ret = readMutualSenseNodeData(address, data);
	if (ret < 0) {
		TS_LOG_ERR("%s: ERROR %08X\n", __func__,ERROR_COMP_DATA_NODE);
		return ret | ERROR_COMP_DATA_NODE;
	}

	return OK;
}

/**
  * Read SS Global Initialization data from the buffer such as Ix1/Cx1 for force
  * and sense
  * @param address pointer to a variable which contain the address from where
  * to read the data and will contain the updated address to the next data
  * @param global pointer to MutualSenseData variable which will contain the SS
  * initialization data
  * @return OK if success or an error code which specify the type of error
  */
int readSelfSenseGlobalData(u64 *address, SelfSenseData *global)
{
	int ret = 0;
	u8 data[COMP_DATA_GLOBAL] = {0};

	TS_LOG_INFO( "%s Address for Global data= %08llX\n", __func__, *address);
	ret = fts_writeReadU8UX(FTS_CMD_FRAMEBUFFER_R, BITS_16, *address, data,
				COMP_DATA_GLOBAL, DUMMY_FRAMEBUFFER);
	if (ret < OK) {
		TS_LOG_ERR("%s: error while reading the data... ERROR %08X\n",__func__, ret);
		return ret;
	}

	TS_LOG_INFO( "%s Global data Read !\n", __func__);


	global->header.force_node = data[0];
	global->header.sense_node = data[1];
	global->f_ix1 = data[2];
	global->s_ix1 = data[3];
	global->f_cx1 = (i8)data[4];
	global->s_cx1 = (i8)data[5];
	global->f_max_n = data[6];
	global->s_max_n = data[7];

	TS_LOG_INFO(
		 "%s force_len = %d sense_len = %d  f_ix1 = %d   s_ix1 = %d   f_cx1 = %d   s_cx1 = %d\n",
		 __func__, global->header.force_node, global->header.sense_node,
		 global->f_ix1, global->s_ix1, global->f_cx1, global->s_cx1);
	TS_LOG_INFO( "%s max_n = %d   s_max_n = %d\n", __func__, global->f_max_n,
		 global->s_max_n);


	*address += COMP_DATA_GLOBAL;

	return OK;
}

/**
  * Read SS Initialization data for each node of force and sense channels from
  * the buffer
  * @param address a variable which contain the address from where to read the
  * data
  * @param node pointer to SelfSenseData variable which will contain the SS
  * initialization data
  * @return OK if success or an error code which specify the type of error
  */
int readSelfSenseNodeData(u64 address, SelfSenseData *node)
{
	int size = node->header.force_node * 2 + node->header.sense_node * 2;
	u8 data[size];
	int ret = 0;

	node->ix2_fm = (u8 *)kmalloc(node->header.force_node * (sizeof(u8)), GFP_KERNEL);
	if (node->ix2_fm == NULL) {
		TS_LOG_ERR("%s: can not allocate memory for ix2_fm... ERROR %08X",
			 __func__, ERROR_ALLOC);
		return ERROR_ALLOC;
	}

	node->cx2_fm = (i8 *)kmalloc(node->header.force_node * (sizeof(i8)),GFP_KERNEL);
	if (node->cx2_fm == NULL) {
		TS_LOG_ERR("%s: can not allocate memory for cx2_fm ... ERROR %08X",
			 __func__, ERROR_ALLOC);
		ret = ERROR_ALLOC;
		goto ERR;
	}
	node->ix2_sn = (u8 *)kmalloc(node->header.sense_node * (sizeof(u8)), GFP_KERNEL);
	if (node->ix2_sn == NULL) {
		TS_LOG_ERR("%s: can not allocate memory for ix2_sn ERROR %08X",
			 __func__, ERROR_ALLOC);
		ret = ERROR_ALLOC;
		goto ERR;
	}
	node->cx2_sn = (i8 *)kmalloc(node->header.sense_node * (sizeof(i8)), GFP_KERNEL);
	if (node->cx2_sn == NULL) {
		TS_LOG_ERR("%s: can not allocate memory for cx2_sn ERROR %08X",
			 __func__, ERROR_ALLOC);
		ret = ERROR_ALLOC;
		goto ERR;
	}

	TS_LOG_INFO( "%s Address for Node data = %08llX\n", __func__, address);

	TS_LOG_INFO( "%s Node Data to read %d bytes\n", __func__, size);

	ret = fts_writeReadU8UX(FTS_CMD_FRAMEBUFFER_R, BITS_16, address, data,
				size, DUMMY_FRAMEBUFFER);
	if (ret < OK) {
		TS_LOG_ERR("%s: error while reading data... ERROR %08X\n",__func__,  ret);
		goto ERR;
	}

	TS_LOG_INFO( "%s Read node data ok!\n", __func__);

	memcpy(node->ix2_fm, data, node->header.force_node);
	memcpy(node->ix2_sn, &data[node->header.force_node],node->header.sense_node);
	memcpy(node->cx2_fm, &data[node->header.force_node +node->header.sense_node],
				node->header.force_node);
	memcpy(node->cx2_sn, &data[node->header.force_node * 2 + node->header.sense_node],
	       		node->header.sense_node);

	return OK;

ERR:
	if(node->ix2_fm){
		kfree(node->ix2_fm);
		node->ix2_fm = NULL;
	}
	if(node->cx2_fm){
		kfree(node->cx2_fm);
		node->cx2_fm = NULL;
	}
	if(node->ix2_sn){
		kfree(node->ix2_sn);
		node->ix2_sn = NULL;
	}
	if(node->cx2_sn){
		kfree(node->cx2_sn);
		node->cx2_sn = NULL;
	}
	return ret;
}

/**
  * Perform all the steps to read the necessary info for SS Initialization data
  * from the buffer and store it in a SelfSenseData variable
  * @param type type of SS Initialization data to read @link load_opt Load Host
  * Data Option @endlink
  * @param data pointer to SelfSenseData variable which will contain the SS
  * initialization data
  * @return OK if success or an error code which specify the type of error
  */
int readSelfSenseCompensationData(u8 type, SelfSenseData *data)
{
	int ret = 0;
	u64 address = 0;

	if(!data){
		TS_LOG_ERR( "%s:input error!\n", __func__);
		return ERROR_OP_NOT_ALLOW;
	}
	data->ix2_fm = NULL;
	data->cx2_fm = NULL;
	data->ix2_sn = NULL;
	data->cx2_sn = NULL;

	if (!(type == LOAD_CX_SS_TOUCH || type == LOAD_CX_SS_TOUCH_IDLE ||
	      type == LOAD_CX_SS_KEY || type == LOAD_CX_SS_FORCE)) {
		TS_LOG_ERR("%s: Choose a SS type of compensation data ERROR %08X\n",
			 __func__, ERROR_OP_NOT_ALLOW);
		return ERROR_OP_NOT_ALLOW;
	}

	ret = requestCompensationData(type);
	if (ret < 0) {
		TS_LOG_ERR("%s: error while requesting data... ERROR %08X\n",
			 __func__, ERROR_REQU_COMP_DATA);
		return ret | ERROR_REQU_COMP_DATA;
	}

	ret = readCompensationDataHeader(type, &(data->header), &address);
	if (ret < 0) {
		TS_LOG_ERR( "%s: error while reading data header... ERROR %08X\n",
			 __func__, ERROR_COMP_DATA_HEADER);
		return ret | ERROR_COMP_DATA_HEADER;
	}

	ret = readSelfSenseGlobalData(&address, data);
	if (ret < 0) {
		TS_LOG_ERR("%s: ERROR %08X\n", __func__,ERROR_COMP_DATA_GLOBAL);
		return ret | ERROR_COMP_DATA_GLOBAL;
	}

	ret = readSelfSenseNodeData(address, data);
	if (ret < 0) {
		TS_LOG_ERR("%s: ERROR %08X\n",__func__,ERROR_COMP_DATA_NODE);
		return ret | ERROR_COMP_DATA_NODE;
	}

	return OK;
}

/**
  * Read TOT MS Global Initialization data from the buffer such as number of
  * force and sense channels
  * @param address pointer to a variable which contain the address from where
  * to read the data and will contain the updated address to the next data
  * @param global pointer to a variable which will contain the TOT MS
  * initialization data
  * @return OK if success or an error code which specify the type of error
  */
int readTotMutualSenseGlobalData(u64 *address, TotMutualSenseData *global)
{
	int ret = 0;
	u8 data[COMP_DATA_GLOBAL] = {0};

	TS_LOG_INFO( "%s Address for Global data= %04llX\n", __func__, *address);
	if(!address || !global){
		TS_LOG_ERR( "%s:input error!\n", __func__);
		return ERROR_OP_NOT_ALLOW;
	}
	ret = fts_writeReadU8UX(FTS_CMD_FRAMEBUFFER_R, BITS_16, *address, data,
				COMP_DATA_GLOBAL, DUMMY_FRAMEBUFFER);
	if (ret < OK) {
		TS_LOG_ERR("%s: error while reading info data ERROR %08X\n",__func__, ret);
		return ret;
	}
	TS_LOG_INFO( "%s Global data Read !\n", __func__);

	global->header.force_node = data[0];
	global->header.sense_node = data[1];
	/* all other bytes are reserved atm */

	TS_LOG_INFO( "%s force_len = %d sense_len = %d\n", __func__,
		 global->header.force_node, global->header.sense_node);

	*address += COMP_DATA_GLOBAL;
	return OK;
}


/**
  * Read TOT MS Initialization data for each node from the buffer
  * @param address a variable which contain the address from where to read the
  * data
  * @param node pointer to MutualSenseData variable which will contain the TOT
  * MS initialization data
  * @return OK if success or an error code which specify the type of error
  */
int readTotMutualSenseNodeData(u64 address, TotMutualSenseData *node)
{
	int ret = 0, i = 0;
	int size = node->header.force_node * node->header.sense_node;
	int toRead = size * sizeof(u16);
	u8 data[toRead];

	memset(data, 0, toRead);
	TS_LOG_INFO( "%s Address for Node data = %04llX\n", __func__, address);

	node->node_data = (short *)kmalloc(size * (sizeof(short)), GFP_KERNEL);

	if (node->node_data == NULL) {
		TS_LOG_ERR("%s: can not allocate node_data... ERROR %08X",
			 __func__, ERROR_ALLOC);
		return ERROR_ALLOC;
	}

	TS_LOG_INFO( "%s Node Data to read %d bytes\n", __func__, size);

	ret = fts_writeReadU8UX(FTS_CMD_FRAMEBUFFER_R, BITS_16, address, data,
				toRead, DUMMY_FRAMEBUFFER);
	if (ret < OK) {
		TS_LOG_ERR("%s: error while reading node data ERROR %08X\n",__func__, ret);
		kfree(node->node_data);
		node->node_data = NULL;
		return ret;
	}
	node->node_data_size = size;

	for (i = 0; i < size; i++)
		node->node_data[i] = ((short)data[i * 2 + 1]) << 8 | data[i * 2];

	TS_LOG_INFO( "%s Read node data OK!\n", __func__);

	return size;
}

/**
  * Perform all the steps to read the necessary info for TOT MS Initialization
  * data from the buffer and store it in a TotMutualSenseData variable
  * @param type type of TOT MS Initialization data to read @link load_opt Load
  * Host Data Option @endlink
  * @param data pointer to a variable which will contain the TOT MS
  * initialization data
  * @return OK if success or an error code which specify the type of error
  */
int readTotMutualSenseCompensationData(u8 type, TotMutualSenseData *data)
{
	int ret = 0;
	u64 address = 0;

	if(!data){
		TS_LOG_ERR( "%s:input error!\n", __func__);
		return ERROR_OP_NOT_ALLOW;
	}
	data->node_data = NULL;

	if (!(type == LOAD_PANEL_CX_TOT_MS_TOUCH || type ==
	      LOAD_PANEL_CX_TOT_MS_LOW_POWER || type ==
	      LOAD_PANEL_CX_TOT_MS_KEY ||
	      type == LOAD_PANEL_CX_TOT_MS_FORCE)) {
		TS_LOG_ERR("%s: Choose a TOT MS type of compensation data ERROR %08X\n",
			  __func__, ERROR_OP_NOT_ALLOW);
		return ERROR_OP_NOT_ALLOW;
	}

	ret = requestCompensationData(type);
	if (ret < 0) {
		TS_LOG_ERR("%s: ERROR %08X\n",__func__, ERROR_REQU_COMP_DATA);
		return ret | ERROR_REQU_COMP_DATA;
	}

	ret = readCompensationDataHeader(type, &(data->header), &address);
	if (ret < 0) {
		TS_LOG_ERR( "%s : ERROR %08X\n",  __func__,ERROR_COMP_DATA_HEADER);
		return ret | ERROR_COMP_DATA_HEADER;
	}

	ret = readTotMutualSenseGlobalData(&address, data);
	if (ret < 0) {
		TS_LOG_ERR( "%s: ERROR %08X\n", __func__,ERROR_COMP_DATA_GLOBAL);
		return ret | ERROR_COMP_DATA_GLOBAL;
	}

	ret = readTotMutualSenseNodeData(address, data);
	if (ret < 0) {
		TS_LOG_ERR( "%s: ERROR %08X\n", __func__,ERROR_COMP_DATA_NODE);
		return ret | ERROR_COMP_DATA_NODE;
	}

	return OK;
}

/**
  * Read TOT SS Global Initialization data from the buffer such as number of
  * force and sense channels
  * @param address pointer to a variable which contain the address from where
  * to read the data and will contain the updated address to the next data
  * @param global pointer to a variable which will contain the TOT SS
  * initialization data
  * @return OK if success or an error code which specify the type of error
  */
int readTotSelfSenseGlobalData(u64 *address, TotSelfSenseData *global)
{
	int ret = 0;
	u8 data[COMP_DATA_GLOBAL] = {0};

	if(!address || !global){
		TS_LOG_ERR( "%s:input error!\n", __func__);
		return ERROR_OP_NOT_ALLOW;
	}
	TS_LOG_INFO( "%s Address for Global data= %04llX\n", __func__, *address);
	ret = fts_writeReadU8UX(FTS_CMD_FRAMEBUFFER_R, BITS_16, *address, data,
				COMP_DATA_GLOBAL, DUMMY_FRAMEBUFFER);
	if (ret < OK) {
		TS_LOG_ERR("%s: error while reading the data... ERROR %08X\n",__func__, ret);
		return ret;
	}

	TS_LOG_INFO( "%s Global data Read !\n", __func__);


	global->header.force_node = data[0];
	global->header.sense_node = data[1];


	TS_LOG_INFO( "%s force_len = %d sense_len = %d\n", __func__,
		 global->header.force_node, global->header.sense_node);


	*address += COMP_DATA_GLOBAL;

	return OK;
}

/**
  * Read TOT SS Global Initialization data from the buffer such as number of
  * force and sense channels
  * @param address pointer to a variable which contain the address from where
  * to read the data and will contain the updated address to the next data
  * @param node pointer to a variable which will contain the TOT SS
  * initialization data
  * @return OK if success or an error code which specify the type of error
  */
int readTotSelfSenseNodeData(u64 address, TotSelfSenseData *node)
{
	int size = node->header.force_node * 2 + node->header.sense_node * 2;
	int toRead = size * 2;	/* *2 2 bytes each node */
	u8 data[toRead];
	int ret = 0, i = 0, j = 0;

	memset(data, 0, toRead);
	node->ix_fm = (u16 *)kmalloc(node->header.force_node * (sizeof(u16)), GFP_KERNEL);
	if (node->ix_fm == NULL) {
		TS_LOG_ERR( "%s: can not allocate memory for ix2_fm... ERROR %08X",
			 __func__, ERROR_ALLOC);
		return ERROR_ALLOC;
	}

	node->cx_fm = (short *)kmalloc(node->header.force_node *
				       (sizeof(short)), GFP_KERNEL);
	if (node->cx_fm == NULL) {
		TS_LOG_ERR("%s: can not allocate memory for cx2_fm ... ERROR %08X",
			 __func__, ERROR_ALLOC);
		ret = ERROR_ALLOC;
		goto out;
	}
	node->ix_sn = (u16 *)kmalloc(node->header.sense_node * (sizeof(u16)),GFP_KERNEL);
	if (node->ix_sn == NULL) {
		TS_LOG_ERR( "%s: can not allocate memory for ix2_sn ERROR %08X",
			 __func__, ERROR_ALLOC);
		ret = ERROR_ALLOC;
		goto out;
	}
	node->cx_sn = (short *)kmalloc(node->header.sense_node *
				       (sizeof(short)), GFP_KERNEL);
	if (node->cx_sn == NULL) {
		TS_LOG_ERR("%s: can not allocate memory for cx2_sn ERROR %08X",
			 __func__, ERROR_ALLOC);
		ret = ERROR_ALLOC;
		goto out;
	}


	TS_LOG_INFO( "%s Address for Node data = %04llX\n", __func__, address);

	TS_LOG_INFO( "%s Node Data to read %d bytes\n", __func__, size);

	ret = fts_writeReadU8UX(FTS_CMD_FRAMEBUFFER_R, BITS_16, address, data,
				toRead, DUMMY_FRAMEBUFFER);
	if (ret < OK) {
		TS_LOG_ERR( "%s: error while reading data... ERROR %08X\n",__func__, ret);
		goto  out;
	}

	TS_LOG_INFO( "%s Read node data ok!\n", __func__);

	j = 0;
	for (i = 0; i < node->header.force_node; i++) {
		node->ix_fm[i] = ((u16)data[j + 1]) << 8 | data[j];
		j += 2;
	}

	for (i = 0; i < node->header.sense_node; i++) {
		node->ix_sn[i] = ((u16)data[j + 1]) << 8 | data[j];
		j += 2;
	}

	for (i = 0; i < node->header.force_node; i++) {
		node->cx_fm[i] = ((short)data[j + 1]) << 8 | data[j];
		j += 2;
	}

	for (i = 0; i < node->header.sense_node; i++) {
		node->cx_sn[i] = ((short)data[j + 1]) << 8 | data[j];
		j += 2;
	}

	if (j != toRead)
		TS_LOG_ERR( "%s: parsed a wrong number of bytes %d!=%d\n", __func__, j, toRead);

	return OK;

out:
	if(node->ix_fm){
		kfree(node->ix_fm);		
		node->ix_fm = NULL;
	}
	if(node->cx_fm){
		kfree(node->cx_fm);		
		node->cx_fm = NULL;
	}
	if(node->ix_sn){
		kfree(node->ix_sn);		
		node->ix_sn = NULL;
	}
	if(node->cx_sn){
		kfree(node->cx_sn);		
		node->cx_sn = NULL;
	}
	return ret ;
}

/**
  * Perform all the steps to read the necessary info for TOT SS Initialization
  * data from the buffer and store it in a TotSelfSenseData variable
  * @param type type of TOT MS Initialization data to read @link load_opt Load
  * Host Data Option @endlink
  * @param data pointer to a variable which will contain the TOT MS
  * initialization data
  * @return OK if success or an error code which specify the type of error
  */
int readTotSelfSenseCompensationData(u8 type, TotSelfSenseData *data)
{
	int ret = 0;
	u64 address = 0;
	if(!data){
		TS_LOG_ERR( "%s:input error!\n", __func__);
		return ERROR_OP_NOT_ALLOW;
	}
	data->ix_fm = NULL;
	data->cx_fm = NULL;
	data->ix_sn = NULL;
	data->cx_sn = NULL;

	if (!(type == LOAD_PANEL_CX_TOT_SS_TOUCH || type ==
	      LOAD_PANEL_CX_TOT_SS_TOUCH_IDLE || type ==
	      LOAD_PANEL_CX_TOT_SS_KEY ||
	      type == LOAD_PANEL_CX_TOT_SS_FORCE)) {
		TS_LOG_ERR("%s: Choose a TOT SS type of compensation data ERROR %08X\n",
			  __func__, ERROR_OP_NOT_ALLOW);
		return ERROR_OP_NOT_ALLOW;
	}

	ret = requestCompensationData(type);
	if (ret < 0) {
		TS_LOG_ERR("%s: error while requesting data... ERROR %08X\n",
			 __func__, ERROR_REQU_COMP_DATA);
		return ret | ERROR_REQU_COMP_DATA;
	}

	ret = readCompensationDataHeader(type, &(data->header), &address);
	if (ret < 0) {
		TS_LOG_ERR("%s: error while reading data header... ERROR %08X\n",
			 __func__, ERROR_COMP_DATA_HEADER);
		return ret | ERROR_COMP_DATA_HEADER;
	}

	ret = readTotSelfSenseGlobalData(&address, data);
	if (ret < 0) {
		TS_LOG_ERR( "%s: ERROR %08X\n", __func__,ERROR_COMP_DATA_GLOBAL);
		return ret | ERROR_COMP_DATA_GLOBAL;
	}

	ret = readTotSelfSenseNodeData(address, data);
	if (ret < 0) {
		TS_LOG_ERR( "%s ERROR %08X\n", __func__, ERROR_COMP_DATA_NODE);
		return ret | ERROR_COMP_DATA_NODE;
	}

	return OK;
}
