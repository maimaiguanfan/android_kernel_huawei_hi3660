/*
  *
  **************************************************************************
  **                        STMicroelectronics				 **
  **************************************************************************
  **                        marco.cali@st.com				 **
  **************************************************************************
  *                                                                        *
  *                     FTS Utility Functions				   *
  *                                                                        *
  **************************************************************************
  **************************************************************************
  *
  */

/*!
  * \file ftsTool.c
  * \brief Contains all the functions to support common operation inside the
  *driver
  */

#include "ftsCompensation.h"
#include "ftsCore.h"
#include "ftsError.h"
#include "ftsHardware.h"
#include "ftsIO.h"
#include "ftsSoftware.h"
#include "ftsTime.h"
#include "ftsTool.h"
#include "../fts.h"	/* needed for the tag define */


#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <stdarg.h>
#include <linux/serio.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/ctype.h>


/**
  * Print an array of byte in a HEX string and attach at the beginning a label.
  * The function allocate memory that should be free outside the function itself
  * @param label string to attach at the beginning
  * @param buff pointer to the byte array that should be printed as HEX string
  * @param count size of buff
  * @param result pointer to the array of characters that compose the HEX final
  * string
  * @return pointer to the array of characters that compose the HEX string,
  * (same address of result)
  * @warning result MUST be allocated outside the function and should be
  * big enough to contain the data converted as HEX!
  */
char *printHex(char *label, u8 *buff, int count, u8 *result)
{
	int i = 0;
	int offset = 0;

	if(!label  ||!buff  ||!result){
		TS_LOG_INFO( "%s:input is valid.\n", __func__);
		return NULL;
	}
	offset = strlen(label);
	strlcpy(result, label, offset+1); /* +1 for terminator char */

	for (i = 0; i < count; i++) {
		snprintf(&result[offset], 4, "%02X ", buff[i]);
		/* this append automatically a null terminator char */
		offset += 3;
	}
	return result;
}

/**
  * Clear the FIFO from any event
  * @return OK if success or an error code which specify the type of error
  */
int flushFIFO(void)
{
	int ret = 0;
	u8 sett = SPECIAL_FIFO_FLUSH;

	ret = writeSysCmd(SYS_CMD_SPECIAL, &sett, 1);	/* flush the FIFO */
	if (ret < OK) {
		TS_LOG_ERR("%s flushFIFO: ERROR %08X\n", __func__, ret);
		return ret;
	}
	TS_LOG_INFO("%s FIFO flushed!\n", __func__);
	return OK;
}

/**
  * Convert an array of 2 bytes to a u16, src has LSB first (little endian).
  * @param src pointer to the source byte array
  * @param dst pointer to the destination u16.
  * @return OK
  */
int u8ToU16(u8 *src, u16 *dst)
{
	if ((src == NULL) || (dst == NULL)) {
		TS_LOG_INFO("%s error!\n", __func__);
		return ERROR_OP_NOT_ALLOW;
	}
	*dst = (u16)(((src[1] & 0x00FF) << 8) + (src[0] & 0x00FF));
	return OK;
}

/**
  * Convert an array of 2 bytes to a u16, src has MSB first (big endian).
  * @param src pointer to the source byte array
  * @param dst pointer to the destination u16.
  * @return OK
  */
int u8ToU16_be(u8 *src, u16 *dst)
{
	if ((src == NULL) || (dst == NULL)) {
		TS_LOG_INFO("%s error!\n", __func__);
		return ERROR_OP_NOT_ALLOW;
	}
	*dst = (u16)(((src[0] & 0x00FF) << 8) + (src[1] & 0x00FF));
	return OK;
}



/**
  * Convert an array of bytes to a u32, src has LSB first (little endian).
  * @param src array of bytes to convert
  * @param dst pointer to the destination u32 variable.
  * @return OK
  */
int u8ToU32(u8 *src, u32 *dst)
{
	if ((src == NULL) || (dst == NULL)) {
		TS_LOG_INFO("%s error!\n", __func__);
		return ERROR_OP_NOT_ALLOW;
	}
	*dst = (u32)(((src[3] & 0xFF) << 24) + ((src[2] & 0xFF) << 16) +
		     ((src[1] & 0xFF) << 8) + (src[0] & 0xFF));
	return OK;
}



/**
  * Convert a u32 to an array of 4 bytes, dst has MSB first (big endian).
  * @param src u32 value to convert
  * @param dst pointer to the destination array of 4 bytes.
  * @return OK
  */
int u32ToU8_be(u32 src, u8 *dst)
{
	if (dst == NULL) {
		TS_LOG_INFO("%s error!\n", __func__);
		return ERROR_OP_NOT_ALLOW;
	}

	dst[0] = (u8)((src & 0xFF000000) >> 24);
	dst[1] = (u8)((src & 0x00FF0000) >> 16);
	dst[2] = (u8)((src & 0x0000FF00) >> 8);
	dst[3] = (u8)(src & 0x000000FF);
	return OK;
}

/**
  * Execute a function passed as argment and retry it defined number of times if
  *not successfull
  * @param code pointer to a function which return an int and doesn't have any
  *parameters
  * @param wait_before_retry interval of time in ms to wait between one trial
  *and another one
  * @param retry_count max number of retry to attemp
  * @return last return value obtained from the last execution of the code
  *function
  */
int attempt_function(int (*code)(void), unsigned long wait_before_retry, int  retry_count)
{
	int result = 0;
	int count = 0;

	do {
		result = code();
		count++;
		msleep(wait_before_retry);
	} while (count < retry_count && result < 0);


	if (count == retry_count)
		return result | ERROR_TIMEOUT;
	else
		return result;
}

/**
  * Enable all the possible sensing mode supported by the FW
  * @return OK if success or an error code which specify the type of error
  */
int senseOn(void)
{
	int ret = 0;

	ret = setScanMode(SCAN_MODE_ACTIVE, 0xFF);	/* enable all */
	if (ret < OK) {
		TS_LOG_INFO("%s senseOn: ERROR %08X\n", __func__, ret);
		return ret;
	}

	TS_LOG_INFO("%s senseOn: SENSE ON\n", __func__);
	return OK;
}

/**
  * Disable  all the sensing mode
  * @return  OK if success or an error code which specify the type of error
  */
int senseOff(void)
{
	int ret = 0;

	ret = setScanMode(SCAN_MODE_ACTIVE, 0x00);
	if (ret < OK) {
		TS_LOG_ERR("%s senseOff: ERROR %08X\n", __func__, ret);
		return ret;
	}

	TS_LOG_INFO("%s senseOff: SENSE OFF\n", __func__);
	return OK;
}



/**
  * Clean up the IC status executing a system reset and giving
  * the possibility to re-enabling the sensing
  * @param enableTouch if 1, re-enable the sensing and the interrupt of the IC
  * @return OK if success or an error code which specify the type of error
  */
int cleanUp(int enableTouch)
{
	int res =0;

	TS_LOG_INFO("%s cleanUp: system reset...\n", __func__);
	res = fts_system_reset();
	if (res < OK)
		return res;
	if (enableTouch) {
		TS_LOG_INFO("%s cleanUp: enabling touches...\n", __func__);
		res = senseOn();	/* already enable everything */
		if (res < OK)
			return res;
		TS_LOG_INFO( "%s cleanUp: enabling interrupts...\n", __func__);
		res = fts_enableInterrupt();
		if (res < OK)
			return res;
	}
	return OK;
}

/**
  * Transform an array of short in a matrix of short with a defined number of
  * columns and the resulting number of rows
  * @param data array of bytes to convert
  * @param size size of data
  * @param columns number of columns that the resulting matrix should have.
  * @return a reference to a matrix of short where for each row there are
  * columns elements
  * @warning If size = 0 it will be allocated a matrix 1*1 wich still should
  * be free
  */
short **array1dTo2d_short(short *data, int size, int columns)
{
	int i = 0;
	short **matrix = NULL;
	if (!data) {
		TS_LOG_ERR("%s data NULL.\n", __func__);
		return NULL;
	}
	if (size == 0) {
		matrix = (short **)kmalloc(1 * sizeof(short *), GFP_KERNEL);
		if (matrix == NULL) {
			TS_LOG_ERR("%s kmalloc  failed1.\n", __func__);
			return NULL;
		}
		matrix[0] = (short *)kmalloc(0 * sizeof(short), GFP_KERNEL);
		if (matrix[0] == NULL) {
			TS_LOG_ERR("%s kmalloc  failed2.\n", __func__);
			kfree(matrix);
			matrix = NULL;
			return NULL;
		}
	} else {
		if (!columns) {
			TS_LOG_ERR("%s columns = 0.\n", __func__);
			return NULL;
		}
		matrix = (short **)kmalloc(((int)(size / columns)) * sizeof(short *), GFP_KERNEL);
		if (matrix == NULL) {
			TS_LOG_ERR("%s kmalloc  failed3.\n", __func__);
			return NULL;
		}
		if (matrix != NULL) {
			for (i = 0; i < (int)(size / columns); i++) {
				matrix[i] = (short *)kmalloc(columns * sizeof(short), GFP_KERNEL);
				if(matrix[i] == NULL){
					goto error;
				}
			}
			for (i = 0; i < size; i++) {
				matrix[i / columns][i % columns] = data[i];
            }
		}
	}

	return matrix;
error:
	if (matrix != NULL) {
		for (i = 0; i < (int)(size / columns); i++) {
			if(matrix[i] != NULL) {
				kfree(matrix[i]);
				matrix[i] = NULL;
			}
		}
		kfree(matrix);
		matrix = NULL;
	}
	return NULL;
}

/**
  * Transform an array of u16 in a matrix of u16 with a defined number of
  * columns and the resulting number of rows
  * @param data array of bytes to convert
  * @param size size of data
  * @param columns number of columns that the resulting matrix should have.
  * @return a reference to a matrix of u16 where for each row there are columns
  * elements
  * @warning If size = 0 it will be allocated a matrix 1*1 wich still should
  * be free
  */
u16 **array1dTo2d_u16(u16 *data, int size, int columns)
{
	int i = 0;
	u16 **matrix = NULL;
	if(!data) {
		TS_LOG_ERR("%s data NULL.\n", __func__);
		return NULL;
	}

	if (size == 0) {
		matrix = (u16 **)kmalloc(1 *sizeof(u16 *), GFP_KERNEL);
		if(matrix == NULL){
			TS_LOG_ERR("%s kmalloc  failed1.\n", __func__);
			return NULL;
		}
		matrix[0] = (u16 *)kmalloc(0 *sizeof(u16), GFP_KERNEL);
		if(matrix[0] ==NULL){
			TS_LOG_ERR("%s kmalloc  failed2.\n", __func__);
			kfree(matrix);
			matrix = NULL;
			return NULL;
		}
	} else {
		if (!columns) {
			TS_LOG_ERR("%s columns = 0.\n", __func__);
			return NULL;
		}
		matrix = (u16 **)kmalloc(((int)(size / columns)) * sizeof(u16 *), GFP_KERNEL);
		if (matrix == NULL) {
			TS_LOG_ERR("%s kmalloc  failed3.\n", __func__);
			return NULL;
		}
		if (matrix != NULL) {
			for (i = 0; i < (int)(size / columns); i++){
				matrix[i] = (u16 *)kmalloc(columns *sizeof(u16), GFP_KERNEL);
				if(matrix[i] == NULL){
					goto error;
				}
			}
			for (i = 0; i < size; i++)
				matrix[i / columns][i % columns] = data[i];
		}
	}

	return matrix;
error:
	if (matrix != NULL) {
		for (i = 0; i < (int)(size / columns); i++){
			if(matrix[i] != NULL){
				kfree(matrix[i]);
				matrix[i] = NULL;
			}
		}
		kfree(matrix);
		matrix = NULL;
	}
	return NULL;
}

/**
  * Transform an array of u8 in a matrix of u8 with a defined number of
  * columns and the resulting number of rows
  * @param data array of bytes to convert
  * @param size size of data
  * @param columns number of columns that the resulting matrix should have.
  * @return a reference to a matrix of short where for each row there are
  * columns elements
  * @warning If size = 0 it will be allocated a matrix 1*1 wich still should
  * be free
  */
u8 **array1dTo2d_u8(u8 *data, int size, int columns)
{
	int i = 0;
	u8 **matrix = NULL;
	if(!data) {
		TS_LOG_ERR("%s data NULL.\n", __func__);
		return NULL;
	}

	if (size == 0) {
		matrix = (u8 **)kmalloc(1 * sizeof(u8 *), GFP_KERNEL);
		if(matrix == NULL){
			TS_LOG_ERR("%s kmalloc  failed1.\n", __func__);
			return NULL;
		}
		matrix[0] = (u8 *)kmalloc(0 * sizeof(u8), GFP_KERNEL);
		if(matrix[0] ==NULL){
			TS_LOG_ERR("%s kmalloc  failed2.\n", __func__);
			kfree(matrix);
			matrix = NULL;
			return NULL;
		}
	} else {
		if (!columns) {
			TS_LOG_ERR("%s columns = 0.\n", __func__);
			return NULL;
		}
		matrix = kmalloc(((int)(size / columns)) * sizeof(u8 *), GFP_KERNEL);
		if (matrix == NULL) {
			TS_LOG_ERR("%s kmalloc  failed3.\n", __func__);
			return NULL;
		}
		if (matrix != NULL) {
			for (i = 0; i < (int)(size / columns); i++){
				matrix[i] = (u8 *)kmalloc(columns * sizeof(u8),GFP_KERNEL);
				if(matrix[i] == NULL){
					goto error;
				}
			}

			for (i = 0; i < size; i++){
				matrix[i / columns][i % columns] = data[i];
			}
		}
	}

	return matrix;
error:
	if (matrix != NULL) {
		for (i = 0; i < (int)(size / columns); i++){
			if(matrix[i] != NULL){
				kfree(matrix[i]);
				matrix[i] = NULL;
			}
		}
		kfree(matrix);
		matrix = NULL;
	}
	return NULL;
}

/**
  * Transform an array of i8 in a matrix of i8 with a defined number of
  * columns and the resulting number of rows
  * @param data array of bytes to convert
  * @param size size of data
  * @param columns number of columns that the resulting matrix should have.
  * @return a reference to a matrix of short where for each row there are
  * columns elements
  * @warning If size = 0 it will be allocated a matrix 1*1 wich still should
  * be free
  */
i8 **array1dTo2d_i8(i8 *data, int size, int columns)
{
	int i = 0;
	i8 **matrix = NULL;
	if(!data) {
		TS_LOG_ERR("%s data NULL.\n", __func__);
		return NULL;
	}

	if (size == 0) {
		matrix = (i8 **)kmalloc(1 * sizeof(i8 *), GFP_KERNEL);
		if(matrix == NULL){
			TS_LOG_ERR("%s kmalloc  failed1.\n", __func__);
			return NULL;
		}
		matrix[0] = (i8 *)kmalloc(0 *  sizeof(i8), GFP_KERNEL);
		if (matrix[0] == NULL) {
			TS_LOG_ERR("%s kmalloc  failed2.\n", __func__);
			kfree(matrix);
			matrix = NULL;
			return NULL;
		}
	} else {
		if (!columns) {
			TS_LOG_ERR("%s columns = 0.\n", __func__);
			return NULL;
		}
		matrix = (i8 **)kmalloc(((int)(size / columns)) * sizeof(i8 *), GFP_KERNEL);
		if (matrix == NULL) {
			TS_LOG_ERR("%s kmalloc  failed3.\n", __func__);
			return NULL;
		}
		if (matrix != NULL) {
			for (i = 0; i < (int)(size / columns); i++){
				matrix[i] = (i8 *)kmalloc(columns * sizeof(i8),GFP_KERNEL);
				if(matrix[i] == NULL){
					goto error;
				}
			}
			for (i = 0; i < size; i++){
				matrix[i / columns][i % columns] = data[i];
			}
		}
	}
	return matrix;
error:
	if (matrix != NULL) {
		for (i = 0; i < (int)(size / columns); i++){
			if(matrix[i] != NULL){
				kfree(matrix[i]);
				matrix[i] = NULL;
			}
		}
		kfree(matrix);
		matrix = NULL;
	}
	return NULL;
}

/**
  * Print in the kernel log a label followed by a matrix of short row x columns
  *and free its memory
  * @param label pointer to the string to print before the actual matrix
  * @param matrix reference to the matrix of short which contain the actual data
  * @param row number of rows on which the matrix should be print
  * @param column number of columns for each row
  */
void print_frame_short(char *label, short **matrix, int row, int column)
{
	int i = 0;
	int j = 0;

	if(!label || (*matrix == NULL)) {
		return;
	}
	TS_LOG_INFO("%s %s\n", __func__, label);
	for (i = 0; i < row; i++) {
		TS_LOG_INFO("%s ", __func__);
		for (j = 0; j < column; j++) {
			printk("%d ", matrix[i][j]);
        }
		TS_LOG_INFO("\n");
		kfree(matrix[i]);
		matrix[i] = NULL;
	}
	kfree(matrix);
	matrix = NULL;
}

/**
  * Print in the kernel log a label followed by a matrix of u16 row x columns
  *and free its memory
  * @param label pointer to the string to print before the actual matrix
  * @param matrix reference to the matrix of u16 which contain the actual data
  * @param row number of rows on which the matrix should be print
  * @param column number of columns for each row
  */
void print_frame_u16(char *label, u16 **matrix, int row, int column)
{
	int i = 0;
	int j = 0;

	if(!label || (*matrix == NULL)) {
		return;
	}
	TS_LOG_INFO("%s %s\n", __func__, label);
	for (i = 0; i < row; i++) {
		TS_LOG_INFO("%s ", __func__);
		for (j = 0; j < column; j++)
			printk("%d ", matrix[i][j]);
		TS_LOG_INFO("\n");
		kfree(matrix[i]);
		matrix[i] = NULL;
	}
	kfree(matrix);
	matrix = NULL;
}

/**
  * Print in the kernel log a label followed by a matrix of u8 row x columns and
  *free its memory
  * @param label pointer to the string to print before the actual matrix
  * @param matrix reference to the matrix of u8 which contain the actual data
  * @param row number of rows on which the matrix should be print
  * @param column number of columns for each row
  */
void print_frame_u8(char *label, u8 **matrix, int row, int column)
{
	int i = 0;
	int j = 0;

	if(!label || (*matrix == NULL)) {
		return;
	}
	TS_LOG_INFO( "%s %s\n", __func__, label);
	for (i = 0; i < row; i++) {
		TS_LOG_INFO("%s ", __func__);
		for (j = 0; j < column; j++)
			printk("%d ", matrix[i][j]);
		TS_LOG_INFO( "\n");
		kfree(matrix[i]);
		matrix[i] = NULL;
	}
	kfree(matrix);
	matrix = NULL;
}

/**
  * Print in the kernel log a label followed by a matrix of i8 row x columns and
  * free its memory
  * @param label pointer to the string to print before the actual matrix
  * @param matrix reference to the matrix of u8 which contain the actual data
  * @param row number of rows on which the matrix should be print
  * @param column number of columns for each row
  */
void print_frame_i8(char *label, i8 **matrix, int row, int column)
{
	int i = 0;
	int j = 0;

	if(!label || (*matrix == NULL)) {
		return;
	}

	TS_LOG_INFO("%s %s\n", __func__, label);
	for (i = 0; i < row; i++) {
		TS_LOG_INFO("%s ", __func__);
		for (j = 0; j < column; j++)
			printk("%d ", matrix[i][j]);
		TS_LOG_INFO( "\n");
		kfree(matrix[i]);
		matrix[i] = NULL;
	}
	kfree(matrix);
	matrix = NULL;
}

/**
  * Print in the kernel log a label followed by a matrix of u32 row x columns
  *and free its memory
  * @param label pointer to the string to print before the actual matrix
  * @param matrix reference to the matrix of u32 which contain the actual data
  * @param row number of rows on which the matrix should be print
  * @param column number of columns for each row
  */
void print_frame_u32(char *label, u32 **matrix, int row, int column)
{
	int i = 0;
	int j = 0;

	if(!label || (*matrix == NULL)) {
		return;
	}
	TS_LOG_INFO("%s %s\n", __func__, label);
	for (i = 0; i < row; i++) {
		TS_LOG_INFO("%s ", __func__);
		for (j = 0; j < column; j++)
			printk("%d ", matrix[i][j]);
		TS_LOG_INFO("\n");
		kfree(matrix[i]);
		matrix[i] = NULL;
	}
	kfree(matrix);
	matrix = NULL;
}

/**
  * Print in the kernel log a label followed by a matrix of int row x columns
  * and free its memory
  * @param label pointer to the string to print before the actual matrix
  * @param matrix reference to the matrix of int which contain the actual data
  * @param row number of rows on which the matrix should be print
  * @param column number of columns for each row
  */
void print_frame_int(char *label, int **matrix, int row, int column)
{
	int i = 0;
	int j = 0;
	if(!label || (*matrix == NULL)) {
		return;
	}
	TS_LOG_INFO("%s %s\n", __func__, label);
	for (i = 0; i < row; i++) {
		TS_LOG_INFO("%s ", __func__);
		for (j = 0; j < column; j++)
			printk("%d ", matrix[i][j]);
		TS_LOG_INFO("\n");
		kfree(matrix[i]);
		matrix[i] = NULL;
	}
	kfree(matrix);
	matrix = NULL;
}


/**
  * Convert an array of bytes to an u64, src has MSB first (big endian).
  * @param src array of bytes
  * @param dest pointer to the destination u64.
  * @param size size of src (can be <= 8)
  * @return OK if success or ERROR_OP_NOT_ALLOW if size exceed 8
  */
int u8ToU64_be(u8 *src, u64 *dest, int size)
{
	int i = 0;

	/* u64 temp =0; */
	if (size > sizeof(u64) || (!dest)) {
		return ERROR_OP_NOT_ALLOW;
	} else {
		*dest = 0;
		for (i = 0; i < size; i++)
			*dest |= (u64)(src[i]) << ((size - 1 - i) * 8);

		return OK;
	}
}

/**
  * Convert an u64 to an array of bytes, dest has MSB first (big endian).
  * @param src value of u64
  * @param dest pointer to the destination array of bytes.
  * @param size size of src (can be <= 8)
  * @return OK if success or ERROR_OP_NOT_ALLOW if size exceed 8
  */
int u64ToU8_be(u64 src, u8 *dest, int size)
{
	int i = 0;

	if (size > sizeof(u64) || (!dest)) {
		return ERROR_OP_NOT_ALLOW;
	} else {
		for (i = 0; i < size; i++)
			dest[i] = (u8)((src >> ((size - 1 - i) * 8)) & 0xFF);
	}

	return OK;
}



/*********** NEW API *************/

/**
  * Convert a value of an id in a bitmask with a 1 in the position of the value
  *of the id
  * @param id Value of the ID to convert
  * @param mask pointer to the bitmask that will be updated with the value of id
  * @param size dimension in bytes of mask
  * @return OK if success or ERROR_OP_NOT_ALLOW if size of mask is not enough to
  *contain ID
  */
int fromIDtoMask(u8 id, u8 *mask, int size)
{
	if (((int)((id) / 8)) < size) {
		TS_LOG_INFO("%s: ID = %d Index = %d Position = %d !\n",
			 __func__, id, ((int)((id) / 8)), (id % 8));
		mask[((int)((id) / 8))] |= 0x01 << (id % 8);
		return OK;
	} else {
		TS_LOG_ERR("%s: Bitmask too small! Impossible contain ID = %d %d>=%d! ERROR %08X\n",
			__func__, id, ((int)((id) / 8)), size,
			 ERROR_OP_NOT_ALLOW);
		return ERROR_OP_NOT_ALLOW;
	}
}
