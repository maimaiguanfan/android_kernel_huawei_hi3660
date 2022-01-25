/*
  *
  **************************************************************************
  **                        STMicroelectronics				  **
  **************************************************************************
  **                        marco.cali@st.com				  **
  **************************************************************************
  *                                                                        *
  *			   FTS API for MP test				   *
  *                                                                        *
  **************************************************************************
  **************************************************************************
  *
  */

/*!
  * \file ftsTest.c
  * \brief Contains all the functions related to the Mass Production Test
  */

#include "ftsCompensation.h"
#include "ftsCore.h"
#include "ftsError.h"
#include "ftsFrame.h"
#include "ftsHardware.h"
#include "ftsIO.h"
#include "ftsSoftware.h"
#include "ftsTest.h"
#include "ftsTime.h"
#include "ftsTool.h"
#include "../fts.h"


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
#include <linux/firmware.h>


#ifdef LIMITS_H_FILE
#include "../fts_limits.h"
#endif


TestToDo tests;	/* /< global variable that specify the tests to perform during
		 * the Mass Production Test */
static LimitFile limit_file;	/* /< variable which contains the limit file
				 * during test */
extern SysInfo systemInfo;
extern struct fts_ts_info* fts_get_info(void);
/**
  * Initialize the testToDo variable with the default tests to perform during
  * the Mass Production Test
  * @return OK
  */
int initTestToDo(void)
{
	/*** Initialize Limit File ***/
	limit_file.size = 0;
	limit_file.data = NULL;
	strlcpy(limit_file.name, " ", MAX_LIMIT_FILE_NAME);

	tests.MutualRawAdjITO = 1;

	tests.MutualRaw = 1;  /* in case of YOCTA please use Map */
	tests.MutualRawMap = 0;
	tests.MutualRawGap = 1;
	tests.MutualRawAdj = 0;
	tests.MutualRawAdjGap = 0;
	tests.MutualRawAdjPeak = 0;

	tests.MutualRawLP = 0; /* in case of YOCTA please use Map */
	tests.MutualRawGapLP = 0;
	tests.MutualRawMapLP = 0;
	tests.MutualRawAdjLP = 0;

	tests.MutualCx1 = 0;
	tests.MutualCx2 = 0;
	tests.MutualCx2Adj = 0;
	tests.MutualCxTotal = 1;
	tests.MutualCxTotalAdj = 0;

	tests.MutualCx1LP = 0;
	tests.MutualCx2LP = 0;
	tests.MutualCx2AdjLP = 0;
	tests.MutualCxTotalLP = 0;
	tests.MutualCxTotalAdjLP = 0;


	tests.MutualKeyRaw = 0;

	tests.MutualKeyCx1 = 0;
	tests.MutualKeyCx2 = 0;

	tests.MutualKeyCxTotal = 0;

	tests.SelfForceRaw = 1; /* in case of YOCTA please use Map */
	tests.SelfForceRawGap = 0;
	tests.SelfForceRawMap = 0;

	tests.SelfForceRawLP = 0; /* in case of YOCTA please use Map */
	tests.SelfForceRawGapLP = 0;
	tests.SelfForceRawMapLP = 0;

	tests.SelfForceIx1 = 0;
	tests.SelfForceIx2 = 0;
	tests.SelfForceIx2Adj = 0;
	tests.SelfForceIxTotal = 0;
	tests.SelfForceIxTotalAdj = 0;
	tests.SelfForceCx1 = 0;
	tests.SelfForceCx2 = 0;
	tests.SelfForceCx2Adj = 0;
	tests.SelfForceCxTotal = 0;
	tests.SelfForceCxTotalAdj = 0;
	tests.SelfForceIx1LP = 0;
	tests.SelfForceIx2LP = 0;
	tests.SelfForceIx2AdjLP = 0;
	tests.SelfForceIxTotalLP = 0;
	tests.SelfForceIxTotalAdjLP = 0;
	tests.SelfForceCx1LP = 0;
	tests.SelfForceCx2LP = 0;
	tests.SelfForceCx2AdjLP = 0;
	tests.SelfForceCxTotalLP = 0;
	tests.SelfForceCxTotalAdjLP = 0;

	tests.SelfSenseRaw = 1;
	tests.SelfSenseRawGap = 0;
	tests.SelfSenseRawMap = 0;

	tests.SelfSenseRawLP = 0;
	tests.SelfSenseRawGapLP = 0;
	tests.SelfSenseRawMapLP = 0;

	tests.SelfSenseIx1 = 0;
	tests.SelfSenseIx2 = 0;
	tests.SelfSenseIx2Adj = 0;
	tests.SelfSenseIxTotal = 0;
	tests.SelfSenseIxTotalAdj = 0;
	tests.SelfSenseCx1 = 0;
	tests.SelfSenseCx2 = 0;
	tests.SelfSenseCx2Adj = 0;
	tests.SelfSenseCxTotal = 0;
	tests.SelfSenseCxTotalAdj = 0;
	tests.SelfSenseIx1LP = 0;
	tests.SelfSenseIx2LP = 0;
	tests.SelfSenseIx2AdjLP = 0;
	tests.SelfSenseIxTotalLP = 0;
	tests.SelfSenseIxTotalAdjLP = 0;
	tests.SelfSenseCx1LP = 0;
	tests.SelfSenseCx2LP = 0;
	tests.SelfSenseCx2AdjLP = 0;
	tests.SelfSenseCxTotalLP = 0;
	tests.SelfSenseCxTotalAdjLP = 0;

	return OK;
}

/**
  * Compute the Horizontal adjacent matrix doing the abs of the difference
  * between the column i with the i-1 one. \n
  * Both the original data matrix and the adj matrix are disposed as 1 dimension
  * array one row after the other \n
  * The resulting matrix has one column less than the starting original one \n
  * @param data pointer to the array of signed bytes containing the original
  * data
  * @param row number of rows of the original data
  * @param column number of columns of the original data
  * @param result pointer of a pointer to an array of unsigned bytes which will
  * contain the adj matrix
  * @return OK if success or an error code which specify the type of error
  */
int computeAdjHoriz(i8 *data, int row, int column, u8 **result)
{
	int i = 0;
	int j = 0;
	int size = 0;

	if (column < 2 || !data || !result) {
		TS_LOG_ERR("%s computeAdjHoriz: ERROR %08X\n", __func__,ERROR_OP_NOT_ALLOW);
		return ERROR_OP_NOT_ALLOW;
	}
	size = row * (column - 1);
	*result = (u8 *)kmalloc(size * sizeof(u8), GFP_KERNEL);
	if (*result == NULL) {
		TS_LOG_ERR("%s computeAdjHoriz: ERROR %08X\n", __func__,ERROR_ALLOC);
		return ERROR_ALLOC;
	}

	for (i = 0; i < row; i++){
		for (j = 1; j < column; j++){
			*(*result + (i * (column - 1) + (j - 1))) = abs(data[i * column + j] - data[i * column + (j - 1)]);
		}
	}

	return OK;
}

/**
  * Compute the Horizontal adjacent matrix of short values doing the abs of
  * the difference between the column i with the i-1 one.
  * Both the original data matrix and the adj matrix are disposed as 1 dimension
  *  array one row after the other \n
  * The resulting matrix has one column less than the starting original one \n
  * @param data pointer to the array of signed bytes containing the original
  * data
  * @param row number of rows of the original data
  * @param column number of columns of the original data
  * @param result pointer of a pointer to an array of unsigned bytes which
  * will contain the adj matrix
  * @return OK if success or an error code which specify the type of error
  */
int computeAdjHorizTotal(short *data, int row, int column, u16 **result)
{
	int i = 0;
	int j = 0;
	int size = row * (column - 1);

	if (column < 2 || !data || !result) {
		TS_LOG_ERR("%s computeAdjHorizTotal: ERROR %08X\n", __func__, ERROR_OP_NOT_ALLOW);
		return ERROR_OP_NOT_ALLOW;
	}

	*result = (u16 *)kmalloc(size * sizeof(u16), GFP_KERNEL);
	if (*result == NULL) {
		TS_LOG_ERR("%s computeAdjHorizTotal: ERROR %08X\n", __func__, ERROR_ALLOC);
		return ERROR_ALLOC;
	}

	for (i = 0; i < row; i++) {
		for (j = 1; j < column; j++) {
			*(*result + (i * (column - 1) + (j - 1))) =
				abs(data[i * column + j] - data[i * column + (j - 1)]);
        }
    }

	return OK;
}

/**
  * Compute the Vertical adjacent matrix doing the abs of the difference between
  * the row i with the i-1 one.
  * Both the original data matrix and the adj matrix are disposed as 1 dimension
  * array one row after the other. \n
  * The resulting matrix has one column less than the starting original one \n
  * @param data pointer to the array of signed bytes containing the original
  * data
  * @param row number of rows of the original data
  * @param column number of columns of the original data
  * @param result pointer of a pointer to an array of unsigned bytes which will
  * contain the adj matrix
  * @return OK if success or an error code which specify the type of error
  */
int computeAdjVert(i8 *data, int row, int column, u8 **result)
{
	int i = 0;
	int j = 0;
	int size = (row - 1) * (column);

	if (row < 2 || !data || !result) {
		TS_LOG_ERR("%s computeAdjVert: ERROR %08X\n", __func__,ERROR_OP_NOT_ALLOW);
		return ERROR_OP_NOT_ALLOW;
	}

	*result = (u8 *)kmalloc(size * sizeof(u8), GFP_KERNEL);
	if (*result == NULL) {
		TS_LOG_ERR("%s computeAdjVert: ERROR %08X\n", __func__,ERROR_ALLOC);
		return ERROR_ALLOC;
	}

	for (i = 1; i < row; i++) {
		for (j = 0; j < column; j++) {
			*(*result + ((i - 1) * column + j)) =
				abs(data[i * column + j] - data[(i - 1) * column + j]);
        }
    }

	return OK;
}

/**
  * Compute the Vertical adjacent matrix of short values doing the abs of
  * the difference between the row i with the i-1 one.
  * Both the original data matrix and the adj matrix are disposed as 1 dimension
  * array one row after the other. \n
  * The resulting matrix has one column less than the starting original one \n
  * @param data pointer to the array of signed bytes containing the original
  * data
  * @param row number of rows of the original data
  * @param column number of columns of the original data
  * @param result pointer of a pointer to an array of unsigned bytes which will
  * contain the adj matrix
  * @return OK if success or an error code which specify the type of error
  */
int computeAdjVertTotal(short *data, int row, int column, u16 **result)
{
	int i = 0;
	int j = 0;
	int size = (row - 1) * (column);

	if (row < 2 || !data || !result) {
		TS_LOG_ERR("%s computeAdjVertTotal: ERROR %08X\n", __func__, ERROR_OP_NOT_ALLOW);
		return ERROR_OP_NOT_ALLOW;
	}

	*result = (u16 *)kmalloc(size * sizeof(u16), GFP_KERNEL);
	if (*result == NULL) {
		TS_LOG_ERR("%s computeAdjVertTotal: ERROR %08X\n", __func__, ERROR_ALLOC);
		return ERROR_ALLOC;
	}

	for (i = 1; i < row; i++) {
		for (j = 0; j < column; j++) {
			*(*result + ((i - 1) * column + j)) =
				abs(data[i * column + j] - data[(i - 1) * column + j]);
        }
    }

	return OK;
}

/**
  * Compute the Horizontal adjacent matrix doing the abs of the difference
  * between the column i with the i-1 one. \n
  * Both the original data matrix and the adj matrix are disposed as 1 dimension
  * array one row after the other \n
  * The resulting matrix has one column less than the starting original one \n
  * @param data pointer to the array of unsigned bytes containing the original
  * data
  * @param row number of rows of the original data
  * @param column number of columns of the original data
  * @param result pointer of a pointer to an array of unsigned bytes which will
  *  contain the adj matrix
  * @return OK if success or an error code which specify the type of error
  */
int computeAdjHorizFromU(u8 *data, int row, int column, u8 **result)
{
	int i = 0;
	int j = 0;
	int size = row * (column - 1);

	if (column < 2 || !data || !result) {
		TS_LOG_ERR("%s computeAdjHoriz: ERROR %08X\n", __func__,ERROR_OP_NOT_ALLOW);
		return ERROR_OP_NOT_ALLOW;
	}

	*result = (u8 *)kmalloc(size * sizeof(u8), GFP_KERNEL);
	if (*result == NULL) {
		TS_LOG_ERR("%s computeAdjHoriz: ERROR %08X\n", __func__, ERROR_ALLOC);
		return ERROR_ALLOC;
	}

	for (i = 0; i < row; i++) {
		for (j = 1; j < column; j++) {
			*(*result + (i * (column - 1) + (j - 1))) =
				abs(data[i * column + j] - data[i * column + (j - 1)]);
        }
    }

	return OK;
}

/**
  * Compute the Horizontal adjacent matrix of u16 values doing the abs of
  * the difference between the column i with the i-1 one.
  * Both the original data matrix and the adj matrix are disposed as 1 dimension
  *  array one row after the other \n
  * The resulting matrix has one column less than the starting original one \n
  * @param data pointer to the array of unsigned bytes containing the original
  * data
  * @param row number of rows of the original data
  * @param column number of columns of the original data
  * @param result pointer of a pointer to an array of unsigned bytes which will
  * contain the adj matrix
  * @return OK if success or an error code which specify the type of error
  */
int computeAdjHorizTotalFromU(u16 *data, int row, int column, u16 **result)
{
	int i = 0;
	int j = 0;
	int size = row * (column - 1);

	if (column < 2 || !data || !result) {
		TS_LOG_ERR("%s computeAdjHorizTotal: ERROR %08X\n", __func__,ERROR_OP_NOT_ALLOW);
		return ERROR_OP_NOT_ALLOW;
	}

	*result = (u16 *)kmalloc(size * sizeof(u16), GFP_KERNEL);
	if (*result == NULL) {
		TS_LOG_ERR("%s computeAdjHorizTotal: ERROR %08X\n", __func__,ERROR_ALLOC);
		return ERROR_ALLOC;
	}

	for (i = 0; i < row; i++) {
		for (j = 1; j < column; j++) {
			*(*result + (i * (column - 1) + (j - 1))) =
				abs(data[i * column + j] - data[i * column + (j - 1)]);
        }
    }

	return OK;
}

/**
  * Compute the Vertical adjacent matrix doing the abs of the difference between
  *  the row i with the i-1 one.
  * Both the original data matrix and the adj matrix are disposed as 1 dimension
  * array one row after the other. \n
  * The resulting matrix has one column less than the starting original one \n
  * @param data pointer to the array of unsigned bytes containing the original
  * data
  * @param row number of rows of the original data
  * @param column number of columns of the original data
  * @param result pointer of a pointer to an array of unsigned bytes which will
  *  contain the adj matrix
  * @return OK if success or an error code which specify the type of error
  */
int computeAdjVertFromU(u8 *data, int row, int column, u8 **result)
{
	int i = 0;
	int j = 0;
	int size = (row - 1) * (column);

	if (row < 2 || !data || !result) {
		TS_LOG_ERR("%s computeAdjVert: ERROR %08X\n", __func__,ERROR_OP_NOT_ALLOW);
		return ERROR_OP_NOT_ALLOW;
	}

	*result = (u8 *)kmalloc(size * sizeof(u8), GFP_KERNEL);
	if (*result == NULL) {
		TS_LOG_ERR("%s computeAdjVert: ERROR %08X\n", __func__,ERROR_ALLOC);
		return ERROR_ALLOC;
	}

	for (i = 1; i < row; i++)
		for (j = 0; j < column; j++)
			*(*result + ((i - 1) * column + j)) =
				abs(data[i * column + j] - data[(i - 1) *
								column + j]);

	return OK;
}

/**
  * Compute the Vertical adjacent matrix of u16 values doing the abs of
  * the difference between the row i with the i-1 one.
  * Both the original data matrix and the adj matrix are disposed as 1 dimension
  * array one row after the other. \n
  * The resulting matrix has one column less than the starting original one \n
  * @param data pointer to the array of unsigned bytes containing the original
  * data
  * @param row number of rows of the original data
  * @param column number of columns of the original data
  * @param result pointer of a pointer to an array of unsigned bytes which will
  * contain the adj matrix
  * @return OK if success or an error code which specify the type of error
  */
int computeAdjVertTotalFromU(u16 *data, int row, int column, u16 **result)
{
	int i = 0;
	int j = 0;
	int size = (row - 1) * (column);

	if (row < 2 || !data || !result) {
		TS_LOG_ERR("%s computeAdjVertTotal: ERROR %08X\n", __func__,ERROR_OP_NOT_ALLOW);
		return ERROR_OP_NOT_ALLOW;
	}

	*result = (u16 *)kmalloc(size * sizeof(u16), GFP_KERNEL);
	if (*result == NULL) {
		TS_LOG_ERR("%s computeAdjVertTotal: ERROR %08X\n", __func__,ERROR_ALLOC);
		return ERROR_ALLOC;
	}

	for (i = 1; i < row; i++) {
		for (j = 0; j < column; j++) {
			*(*result + ((i - 1) * column + j)) =
				abs(data[i * column + j] - data[(i - 1) * column + j]);
        }
    }

	return OK;
}



/**
  * Check that each value of a matrix of short doesn't exceed a min and a Max
  * value (these values are included in the interval). \n
  * The matrix is stored as 1 dimension array one row after the other. \n
  * @param data pointer to the array of short containing the data to check
  * @param row number of rows of data
  * @param column number of columns of data
  * @param min minimum value allowed
  * @param max Maximum value allowed
  * @return the number of elements that overcome the specified interval (0 = OK)
  */
int checkLimitsMinMax(short *data, int row, int column, int min, int max)
{
	int i = 0;
	int j = 0;
	int count = 0;
	if(!data) {
		TS_LOG_ERR("%s checkLimitsMinMax: ERROR %08X\n", __func__,ERROR_OP_NOT_ALLOW);
		return ERROR_OP_NOT_ALLOW;
	}
	for (i = 0; i < row; i++) {
		for (j = 0; j < column; j++) {
			if (data[i * column + j] < min || data[i * column + j] > max) {
				TS_LOG_ERR("%s checkLimitsMinMax: Node[%d,%d] = %d exceed limit [%d, %d]\n",
						__func__, i, j, data[i * column + j], min, max);
				count++;
			}
		}
	}

	return count;	/* if count is 0 = OK, test completed successfully */
}

/**
  * Check that the difference between the max and min of a matrix of short is
  * less or equal to a threshold.\n
  * The matrix is stored as 1 dimension array one row after the other.
  * @param data pointer to the array of short containing the data to check
  * @param row number of rows of data
  * @param column number of columns of data
  * @param threshold threshold value allowed
  * @return OK if the difference is <= to threshold otherwise
  * ERROR_TEST_CHECK_FAIL
  */
int checkLimitsGap(short *data, int row, int column, int threshold)
{
	int i = 0;
	int j = 0;
	int min_node = 0;
	int max_node = 0;

	if (row == 0 || column == 0 || !data) {
		TS_LOG_ERR("%s checkLimitsGap: invalid number of rows = %d or columns = %d  ERROR %08X\n",
			 __func__, row, column, ERROR_OP_NOT_ALLOW);
		return ERROR_OP_NOT_ALLOW;
	}

	min_node = data[0];
	max_node = data[0];

	for (i = 0; i < row; i++) {
		for (j = 0; j < column; j++) {
			if (data[i * column + j] < min_node) {
				min_node = data[i * column + j];
            } else if (data[i * column + j] > max_node) {
				max_node = data[i * column + j];
            }
		}
	}

	if (max_node - min_node > threshold) {
		TS_LOG_ERR("%s checkLimitsGap: GAP = %d exceed limit  %d\n",
			 __func__, max_node - min_node, threshold);
		return ERROR_TEST_CHECK_FAIL;
	} else
		return OK;
}

/**
  * Check that the difference between the max and min of a matrix of short is
  * less or equal to a threshold.\n
  * The matrix is stored as 1 dimension array one row after the other.
  * @param data pointer to the array of short containing the data to check
  * @param row number of rows of data
  * @param column number of columns of data
  * @param threshold threshold value allowed
  * @param row_start index of the starting column which should be considered
  * @param column_start index of the starting column which should be considered
  * @param row_end number of index to subtract to row to identify last
  *		valid row to check
  * @param column_end number of index to subtract to column to identify last
  *		valid column to check
  * @return OK if the difference is <= to threshold otherwise
  * ERROR_TEST_CHECK_FAIL
  */
int checkLimitsGapOffsets(short *data, int row, int column, int threshold,
	int row_start, int column_start, int row_end, int column_end)
{
	int i = 0;
	int j = 0;
	int min_node = 0;
	int max_node = 0;

	if (row == 0 || column == 0 || !data) {
		TS_LOG_ERR("%s checkLimitsGap: invalid number of rows = %d or columns = %d  ERROR %08X\n",
			 __func__, row, column, ERROR_OP_NOT_ALLOW);
		return ERROR_OP_NOT_ALLOW;
	}

	min_node = data[row_start*column+column_start];
	max_node = data[row_start*column+column_start];

	for (i = row_start; i < row-row_end; i++) {
		for (j = column_start; j < column-column_end; j++) {
			if (data[i * column + j] < min_node)
				min_node = data[i * column + j];
			else if (data[i * column + j] > max_node)
				max_node = data[i * column + j];
		}
	}

	if (max_node - min_node > threshold) {
		TS_LOG_ERR("%s checkLimitsGap: GAP = %d exceed limit  %d\n",
			 __func__, max_node - min_node, threshold);
		return ERROR_TEST_CHECK_FAIL;
	} else
		return OK;
}

/**
  * Check that each value of a matrix of i8 doesn't exceed a specific min and
  *Max value  set for each node (these values are included in the interval). \n
  * The matrixes of data, min and max values are stored as 1 dimension arrays
  *one row after the other.
  * @param data pointer to the array of short containing the data to check
  * @param row number of rows of data
  * @param column number of columns of data
  * @param min pointer to a matrix which specify the minimum value allowed for
  *each node
  * @param max pointer to a matrix which specify the Maximum value allowed for
  *each node
  * @return the number of elements that overcome the specified interval (0 = OK)
  */
int checkLimitsMap(i8 *data, int row, int column, int *min, int *max)
{
	int i = 0;
	int j = 0;
	int count = 0;
	if(!data || !min || !max) {
		TS_LOG_ERR("%s checkLimitsMap: ERROR %08X\n",
					 __func__,  ERROR_OP_NOT_ALLOW);
		return ERROR_OP_NOT_ALLOW;
	}
	for (i = 0; i < row; i++) {
		for (j = 0; j < column; j++) {
			if (data[i * column + j] < min[i * column + j] ||
			    data[i * column + j] > max[i * column + j]) {
				TS_LOG_ERR("%s checkLimitsMap: Node[%d,%d] = %d exceed limit [%d, %d]\n",
					 __func__, i, j, data[i * column + j],
					 min[i * column + j], max[i * column + j]);
				count++;
			}
		}
	}

	return count;	/* if count is 0 = OK, test completed successfully */
}

/**
  * Check that each value of a matrix of short doesn't exceed a specific min and
  *  Max value  set for each node (these values are included in the interval).
  * The matrixes of data, min and max values are stored as 1 dimension arrays
  * one row after the other.
  * @param data pointer to the array of short containing the data to check
  * @param row number of rows of data
  * @param column number of columns of data
  * @param min pointer to a matrix which specify the minimum value allowed for
  * each node
  * @param max pointer to a matrix which specify the Maximum value allowed for
  * each node
  * @return the number of elements that overcome the specified interval (0 = OK)
  */
int checkLimitsMapTotal(short *data, int row, int column, int *min, int *max)
{
	int i = 0;
	int j = 0;
	int count = 0;
	if(!data || !min || !max) {
		TS_LOG_ERR("%s checkLimitsMapTotal: ERROR %08X\n",
					 __func__,  ERROR_OP_NOT_ALLOW);
		return ERROR_OP_NOT_ALLOW;
	}
	for (i = 0; i < row; i++) {
		for (j = 0; j < column; j++) {
			if (data[i * column + j] < min[i * column + j] ||
			    data[i * column + j] > max[i * column + j]) {
				TS_LOG_ERR("%s checkLimitsMapTotal: Node[%d,%d] = %d exceed limit [%d, %d]\n",
					 __func__, i, j, data[i * column + j],
					 min[i * column + j], max[i * column + j]);
				count++;
			}
		}
	}

	return count;	/* if count is 0 = OK, test completed successfully */
}

/**
  * Check that each value of a matrix of u8 doesn't exceed a specific min and
  * Max value  set for each node (these values are included in the interval). \n
  * The matrixes of data, min and max values are stored as 1 dimension arrays
  * one row after the other.
  * @param data pointer to the array of short containing the data to check
  * @param row number of rows of data
  * @param column number of columns of data
  * @param min pointer to a matrix which specify the minimum value allowed for
  * each node
  * @param max pointer to a matrix which specify the Maximum value allowed for
  * each node
  * @return the number of elements that overcome the specified interval (0 = OK)
  */
int checkLimitsMapFromU(u8 *data, int row, int column, int *min, int *max)
{
	int i = 0;
	int j = 0;
	int count = 0;
	if(!data || !min || !max) {
		TS_LOG_ERR("%s checkLimitsMapFromU: ERROR %08X\n",
						 __func__,	ERROR_OP_NOT_ALLOW);
		return ERROR_OP_NOT_ALLOW;
	}
	for (i = 0; i < row; i++) {
		for (j = 0; j < column; j++) {
			if (data[i * column + j] < min[i * column + j] ||
			    data[i * column + j] > max[i * column + j]) {
				TS_LOG_ERR("%s checkLimitsMap: Node[%d,%d] = %d exceed limit [%d, %d]\n",
					 __func__, i, j, data[i * column + j],
					 min[i * column + j], max[i * column + j]);
				count++;
			}
		}
	}

	return count;	/* if count is 0 = OK, test completed successfully */
}

/**
  * Check that each value of a matrix of u16 doesn't exceed a specific min and
  * Max value  set for each node (these values are included in the interval).
  * The matrixes of data, min and max values are stored as 1 dimension arrays
  * one row after the other.
  * @param data pointer to the array of short containing the data to check
  * @param row number of rows of data
  * @param column number of columns of data
  * @param min pointer to a matrix which specify the minimum value allowed for
  * each node
  * @param max pointer to a matrix which specify the Maximum value allowed for
  * each node
  * @return the number of elements that overcome the specified interval (0 = OK)
  */
int checkLimitsMapTotalFromU(u16 *data, int row, int column, int *min, int *max)
{
	int i = 0;
	int j = 0;
	int count = 0;
	if(!data || !min || !max) {
		TS_LOG_ERR("%s checkLimitsMapTotalFromU: ERROR %08X\n",
						 __func__,	ERROR_OP_NOT_ALLOW);
		return ERROR_OP_NOT_ALLOW;
	}

	for (i = 0; i < row; i++) {
		for (j = 0; j < column; j++) {
			if (data[i * column + j] < min[i * column + j] ||
			    data[i * column + j] > max[i * column + j]) {
				TS_LOG_ERR("%s checkLimitsMapTotal: Node[%d,%d] = %d exceed limit [%d, %d]\n",
					 __func__, i, j, data[i * column + j],
					 min[i * column + j], max[i * column + j]);
				count++;
			}
		}
	}

	return count;	/* if count is 0 = OK, test completed successfully */
}
static void print_data_s16(short *data, int rows, int columns)
{
	int i = 0;
	int j = 0;

	if(NULL == data) {
		TS_LOG_ERR("%s:data is NULL\n", __func__);
		return;
	}

	if((rows <= 0) || (columns <= 0)) {
		TS_LOG_ERR("%s:Invalid! row is %d, column is %d\n", __func__, rows, columns);
		return;
	}

	for (i = 0; i < rows; i++) {
		for (j = 0; j < columns; j++) {
			printk("%d ", data[i*columns + j]);
		}
		printk("\n");
	}
}
static void st_fill_rawdata_buf(struct ts_rawdata_info *info, short *source_data, int rows, int columns, u16 datatype)
{
	int i = 0;
	int j = 0;
	int k = 0;
	int *data_ptr = NULL;

	if(NULL == source_data || info == NULL) {
		TS_LOG_ERR("%s:source_data or info is NULL\n", __func__);
		return;
	}

	if((rows <= 0) || (columns <= 0)) {
		TS_LOG_ERR("%s:Invalid! row is %d, column is %d\n", __func__, rows, columns);
		return;
	}

	TS_LOG_DEBUG("%s:info used size=%d, hybird used size=%d data type=%d, rows=%d, columns=%d\n",
				__func__, info->used_size, info->hybrid_buff_used_size, datatype, rows, columns);

	switch(datatype) {
	case MUTUALCOMPENSATIONTYPE:
	case MUTUALRAWTYPE:
	case STRENGTHTYPE:
		if ( info->used_size + rows * columns > TS_RAWDATA_BUFF_MAX){
			TS_LOG_ERR("%s:out of buffer range\n", __func__);
			return;
		}
		data_ptr = info->buff + info->used_size;
		for (i = 0; i < rows; i++) {
			for (j = 0; j < columns; j++) {
					data_ptr[k++] = (int)source_data[i*columns + j];
					printk("%d(%d) ", data_ptr[k - 1], source_data[i*columns + j]);
			}
			printk("\n");
		}

		info->used_size += rows * columns;
		break;
	case SELFFORCERAWTYPE:
	case SELFSENSERAWTYPE:
	case SSFORCEPRXTYPE:
	case SSSENSEPRXTYPE:
	case SSSENSEDATATYPE:
		data_ptr = info->hybrid_buff + info->hybrid_buff_used_size;
		for (i = 0; i < rows; i++) {
			for (j = 0; j < columns; j++) {
					data_ptr[k++] = (int)source_data[i*columns + j];
			}
		}

		info->hybrid_buff_used_size += rows * columns;
		TS_LOG_INFO("[%s]info->hybrid_buff_used_size -> %d\n",__func__,info->hybrid_buff_used_size);
		break;
	default:
		break;
	}

	return;
}

static void get_average_max_min(short *data, int rows, int columns, char *result)
{
	int i = 0;
	short ms_raw_max = 0;
	short ms_raw_min = 0;
	short ms_raw_average = 0;
	int ms_raw_total = 0;

	if(NULL == data) {
		TS_LOG_ERR("%s:data is NULL\n", __func__);
		return;
	}

	if((rows <= 0) || (columns <= 0)) {
		TS_LOG_ERR("%s:Invalid! row is %d, column is %d\n", __func__, rows, columns);
		return;
	}

	ms_raw_max = data[0];
	ms_raw_min = data[0];
	ms_raw_average = data[0];
	for (i = 0; i < rows * columns; i++) {
		ms_raw_max = max(ms_raw_max, data[i]);
		ms_raw_min = min(ms_raw_min, data[i]);
		ms_raw_total = ms_raw_total + data[i];
	}

	ms_raw_average = ms_raw_total/(rows*columns);

	TS_LOG_INFO("ms_raw_max:%d, ms_raw_min:%d, ms_raw_average:%d\n", ms_raw_max, ms_raw_min, ms_raw_average);
	snprintf(result, ST_NP_TEST_RES_BUF_LEN - 1, "[%d,%d,%d]", ms_raw_average, ms_raw_max, ms_raw_min);
}

/**
  * Check that each value of a matrix of u8 doesn't exceed a specific Max value
  * set for each node (max value is included in the interval).
  * The matrixes of data and max values are stored as 1 dimension arrays one row
  * after the other.
  * @param data pointer to the array of short containing the data to check
  * @param row number of rows of data
  * @param column number of columns of data
  * @param max pointer to a matrix which specify the Maximum value allowed for
  *each node
  * @return the number of elements that overcome the specified interval (0 = OK)
  */
int checkLimitsMapAdj(u8 *data, int row, int column, int *max)
{
	int i = 0;
	int j = 0;
	int count = 0;
	if(!data || !max) {
		TS_LOG_ERR("%s checkLimitsMapAdj: ERROR %08X\n",
							 __func__,	ERROR_OP_NOT_ALLOW);
		return ERROR_OP_NOT_ALLOW;
	}
	for (i = 0; i < row; i++) {
		for (j = 0; j < column; j++) {
			if (data[i * column + j] > max[i * column + j]) {
				TS_LOG_ERR("%s checkLimitsMapAdj: Node[%d,%d] = %d exceed limit > %d\n",
					 __func__, i, j, data[i * column + j],
					 max[i * column + j]);
				count++;
			}
		}
	}

	return count;	/* if count is 0 = OK, test completed successfully */
}

/**
  * Check that each value of a matrix of u16 doesn't exceed a specific Max value
  * set for each node (max value is included in the interval).
  * The matrixes of data and max values are stored as 1 dimension arrays one row
  * after the other.
  * @param data pointer to the array of short containing the data to check
  * @param row number of rows of data
  * @param column number of columns of data
  * @param max pointer to a matrix which specify the Maximum value allowed for
  * each node
  * @return the number of elements that overcome the specified interval (0 = OK)
  */
int checkLimitsMapAdjTotal(u16 *data, int row, int column, int *max)
{
	int i = 0;
	int j = 0;
	int count = 0;
	if(!data || !max) {
		TS_LOG_ERR("%s checkLimitsMapAdjTotal: ERROR %08X\n",
							 __func__,	ERROR_OP_NOT_ALLOW);
		return ERROR_OP_NOT_ALLOW;
	}
	for (i = 0; i < row; i++) {
		for (j = 0; j < column; j++) {
			if (data[i * column + j] > max[i * column + j]) {
				TS_LOG_ERR("%s checkLimitsMapAdjTotal: Node[%d,%d] = %d exceed limit > %d\n",
					 __func__, i, j, data[i * column + j], max[i * column + j]);
				count++;
			}
		}
	}

	return count;	/* if count is 0 = OK, test completed successfully */
}

/**
  * Perform an ITO test setting all the possible options
  * (see @link ito_opt ITO Options @endlink) and checking MS Raw ADJ if enabled
  * @param path_limits name of Production Limit file to load or
  * "NULL" if the limits data should be loaded by a .h file
  * @param todo pointer to a TestToDo variable which select the test to do
  * @return OK if success or an error code which specify the type of error
  */
int production_test_ito(char *path_limits, TestToDo *todo, TestResult *result)
{
	int res = OK;
	u8 sett[2] = { 0x00, 0x00 };
	MutualSenseFrame msRawFrame;
	int *thresholds = NULL;
	u16 *adj = NULL;
	int trows = 0;
	int tcolumns = 0;
	short ** temp1 =NULL;

	TS_LOG_INFO( "%s ITO Production test is starting...\n", __func__);
	if(!path_limits || !todo || !result) {
		TS_LOG_ERR("%s: path_limits NULL ERROR %08X\n",	__func__,ERROR_PROD_TEST_ITO);
		return res | ERROR_PROD_TEST_ITO;
	}
	if(result != NULL){
		result->I2c_Check = false;
		result->ITO_Test_Res = false;
	}
	res = fts_system_reset();
	if (res < 0) {
		TS_LOG_ERR("%s: ERROR %08X\n",  __func__,ERROR_PROD_TEST_ITO);
		return res | ERROR_PROD_TEST_ITO;
	}

	sett[0] = SPECIAL_TUNING_IOFF;
	TS_LOG_INFO( "%s Trimming Ioff...\n", __func__);
	res = writeSysCmd(SYS_CMD_SPECIAL_TUNING, sett, 2);
	if (res < OK) {
		TS_LOG_ERR("%s production_test_ito: Trimm Ioff ERROR %08X\n",
			 __func__, (res | ERROR_PROD_TEST_ITO));
		return res | ERROR_PROD_TEST_ITO;
	}
	if(result != NULL) {
		result->I2c_Check =true;
    }
	sett[0] = 0xFF;
	sett[1] = 0xFF;
	TS_LOG_INFO( "%s ITO Check command sent...\n", __func__);
	res = writeSysCmd(SYS_CMD_ITO, sett, 2);
	if (res < OK) {
		TS_LOG_ERR("%s production_test_ito: ERROR %08X\n", __func__,
			 (res | ERROR_PROD_TEST_ITO));
		return res | ERROR_PROD_TEST_ITO;
	}
	TS_LOG_INFO( "%s ITO Command = OK!\n", __func__);

	TS_LOG_INFO( "%s MS RAW ITO ADJ TEST:\n", __func__);
	if (todo->MutualRawAdjITO == 1) {
		TS_LOG_INFO( "%s Collecting MS Raw data...\n", __func__);
		res |= getMSFrame3(MS_RAW, &msRawFrame);
		if (res < OK) {
			TS_LOG_ERR("%s: getMSFrame failed... ERROR %08X\n",__func__, ERROR_PROD_TEST_ITO);
			goto ERROR;
		}
		TS_LOG_INFO( "%s MS RAW ITO ADJ HORIZONTAL TEST:\n", __func__);
		res = computeAdjHorizTotal(msRawFrame.node_data,
					   msRawFrame.header.force_node,
					   msRawFrame.header.sense_node,
					   &adj);
		if (res < OK) {
			TS_LOG_ERR("%s: computeAdjHoriz failed... ERROR %08X\n",__func__, ERROR_PROD_TEST_ITO);
			goto ERROR;
		}

		res = parseProductionTestLimits(path_limits, &limit_file,
						MS_RAW_ITO_ADJH, &thresholds,
						&trows, &tcolumns);
		if (res < OK || (trows != msRawFrame.header.force_node ||
				 tcolumns != msRawFrame.header.sense_node - 1)) {
			TS_LOG_ERR("%s: parseProductionTestLimits MS_RAW_ITO_ADJH failed... ERROR %08X\n",
				 __func__, ERROR_PROD_TEST_DATA);
			goto ERROR;
		}

		res = checkLimitsMapAdjTotal(adj, msRawFrame.header.force_node,
					     msRawFrame.header.sense_node - 1,
					     thresholds);
		if (res != OK) {
			TS_LOG_ERR("%s production_test_data: checkLimitsAdj MS RAW ITO ADJH failed... ERROR COUNT = %d\n",
				 __func__, res);
			TS_LOG_INFO("%s MS RAW ITO ADJ HORIZONTAL TEST:.................FAIL\n\n",__func__);
			temp1 = array1dTo2d_short( msRawFrame.node_data, msRawFrame.node_data_size,
											msRawFrame.header.sense_node);
			if(temp1) {
				print_frame_short("MS Raw ITO frame =",temp1,
                    msRawFrame.header.force_node, msRawFrame.header.sense_node);
			}
			res = ERROR_PROD_TEST_ITO;
			goto ERROR;
		} else {
			TS_LOG_INFO("%s MS RAW ITO ADJ HORIZONTAL TEST:.................OK\n",__func__);
        }

		kfree(thresholds);
		thresholds = NULL;

		kfree(adj);
		adj = NULL;

		TS_LOG_INFO( "%s MS RAW ITO ADJ VERTICAL TEST:\n", __func__);
		res = computeAdjVertTotal(msRawFrame.node_data,
					  msRawFrame.header.force_node,
					  msRawFrame.header.sense_node,
					  &adj);
		if (res < OK) {
			TS_LOG_ERR("%s: computeAdjVert failed... ERROR %08X\n",
				 __func__, ERROR_PROD_TEST_ITO);
			goto ERROR;
		}

		res = parseProductionTestLimits(path_limits, &limit_file,
						MS_RAW_ITO_ADJV, &thresholds,
						&trows, &tcolumns);
		if (res < OK || (trows != msRawFrame.header.force_node - 1 ||
				 tcolumns != msRawFrame.header.sense_node)) {
			TS_LOG_ERR("%s: parseProductionTestLimits MS_RAW_ITO_ADJV failed... ERROR %08X\n",
				 __func__, ERROR_PROD_TEST_ITO);
			goto ERROR;
		}

		res = checkLimitsMapAdjTotal(adj, msRawFrame.header.force_node -1,
                                     msRawFrame.header.sense_node,
					                 thresholds);
		if (res != OK) {
			TS_LOG_ERR("%s: checkLimitsAdj MS RAW ITO ADJV failed... ERROR COUNT = %d\n",__func__, res);
			TS_LOG_INFO("%s MS RAW ITO ADJ VERTICAL TEST:.................FAIL\n\n",__func__);
			temp1 = array1dTo2d_short( msRawFrame.node_data,msRawFrame.node_data_size,
						 				 msRawFrame.header.sense_node);
			if(temp1) {
				print_frame_short("MS Raw ITO frame =",temp1,
					  msRawFrame.header.force_node,
					  msRawFrame.header.sense_node);
			}
			res = ERROR_PROD_TEST_ITO;
			goto ERROR;
		} else {
			TS_LOG_INFO("%s MS RAW ITO ADJ VERTICAL TEST:.................OK\n",
				 __func__);
        }
		if(result != NULL) {
			result->ITO_Test_Res =true;
        }
		kfree(thresholds);
		thresholds = NULL;

		kfree(adj);
		adj = NULL;
	} else {
		TS_LOG_INFO("%s MS RAW ITO ADJ TEST:.................SKIPPED\n",
			 __func__);
    }

ERROR:
	if (thresholds != NULL) {
        kfree(thresholds);
    }
	if (adj != NULL) {
        kfree(adj);
    }
	if (msRawFrame.node_data != NULL) {
		kfree(msRawFrame.node_data);
    }
	freeLimitsFile(&limit_file);
	res |= fts_system_reset();
	if (res < OK) {
		TS_LOG_ERR("%s production_test_ito: ERROR %08X\n", __func__, ERROR_PROD_TEST_ITO);
		res = (res | ERROR_PROD_TEST_ITO);
	}
	return res;
}

/**
  * Perform the Initialization of the IC
  * @param type type of initialization to do
  * (see @link sys_special_opt Initialization Options (Full or Panel) @endlink)
  * @return OK if success or an error code which specify the type of error
  */
int production_test_initialization(u8 type)
{
	int res = 0;

	TS_LOG_INFO( "%s INITIALIZATION Production test is starting...\n", __func__);
	if (type != SPECIAL_PANEL_INIT && type != SPECIAL_FULL_PANEL_INIT) {
		TS_LOG_ERR("%s production_test_initialization: Type incompatible! Type = %02X ERROR %08X\n",
			 __func__, type, ERROR_OP_NOT_ALLOW |
			 ERROR_PROD_TEST_INITIALIZATION);
		return ERROR_OP_NOT_ALLOW | ERROR_PROD_TEST_INITIALIZATION;
	}

	res = fts_system_reset();
	if (res < 0) {
		TS_LOG_ERR("%s production_test_initialization: ERROR %08X\n",
			 __func__, ERROR_PROD_TEST_INITIALIZATION);
		return res | ERROR_PROD_TEST_INITIALIZATION;
	}

	TS_LOG_INFO( "%s INITIALIZATION command sent... %02X\n", __func__, type);
	res = writeSysCmd(SYS_CMD_SPECIAL, &type, 1);
	if (res < OK) {
		TS_LOG_ERR("%s production_test_initialization: ERROR %08X\n",
			 __func__, (res | ERROR_PROD_TEST_INITIALIZATION));
		return res | ERROR_PROD_TEST_INITIALIZATION;
	}

	TS_LOG_INFO( "%s Refresh Sys Info...\n", __func__);
    /* need to update the chipInfo in order to refresh several versions */
    res |= readSysInfo(1);
	if (res < 0) {
		TS_LOG_ERR("%s production_test_initialization: read sys info ERROR %08X\n",
			 __func__, ERROR_PROD_TEST_INITIALIZATION);
		res = (res | ERROR_PROD_TEST_INITIALIZATION);
	}

	return res;
}


/**
  * Perform a FULL (ITO + INIT + DATA CHECK) Mass Production Test of the IC
  * @param pathThresholds name of Production Limit file to load or
  * "NULL" if the limits data should be loaded by a .h file
  * @param stop_on_fail if 1, the test flow stops at the first data check
  * failure
  * otherwise it keeps going performing all the selected test
  * @param saveInit if >0 (possible values: NO_INIT, SPECIAL_PANEL_INIT or
  * SPECIAL_FULL_PANEL_INIT),
  * the Initialization of the IC is executed otherwise it is skipped
  * @param todo pointer to a TestToDo variable which select the test to do
  * @param mpflag MP flag value to write in case of successful test
  * @return OK if success or an error code which specify the type of error
  */
int production_test_main(char *pathThresholds, int stop_on_fail, int saveInit, TestToDo *todo,
	 u8 mpflag, struct ts_rawdata_info *info, TestResult *result)
{
	int res = 0;
	int ret = 0;
	if(!pathThresholds || !todo || !info || !result) {
		TS_LOG_ERR("%s production_test_main ERROR %08X\n",
			__func__, ERROR_ALLOC);
		res |= ERROR_ALLOC;
		goto END;
	}
	TS_LOG_INFO( "%s MAIN Production test is starting...\n", __func__);
	res = production_test_ito(pathThresholds, todo,result);
	if (res < 0) {
		TS_LOG_INFO( "%s Error during ITO TEST! ERROR %08X\n", __func__, res);
		goto END;	/* in case of ITO TEST failure is no sense keep going* */
	} else {
		TS_LOG_INFO( "%s ITO TEST OK!\n", __func__);
	}

	TS_LOG_INFO( "%s INITIALIZATION TEST :\n", __func__);
	if (saveInit != NO_INIT) {
		res = production_test_initialization((u8)saveInit);  // Calibration
		if (res < 0) {
			TS_LOG_INFO("%s Error during  INITIALIZATION TEST! ERROR %08X\n",
				 __func__, res);
			if (stop_on_fail) {
				goto END;
            }
		} else {
			TS_LOG_INFO( "%s INITIALIZATION TEST OK!\n", __func__);
        }
	} else {
		TS_LOG_INFO("%s INITIALIZATION TEST :................. SKIPPED\n",__func__);
    }

	if (saveInit != NO_INIT) {
		TS_LOG_INFO( "%s Cleaning up...\n", __func__);
		ret = fts_system_reset(); // Recovery After Calibration
		if (ret < 0) {
			TS_LOG_ERR("%s production_test_main: system reset ERROR %08X\n", __func__, ret);
			res |= ret;
			if (stop_on_fail) {
				goto END;
            }
		}
	}

	TS_LOG_INFO( "%s PRODUCTION DATA TEST:\n", __func__);
	ret = production_test_data(pathThresholds, stop_on_fail, todo, info, result);
	if (ret < OK) {
		TS_LOG_INFO("%s Error during PRODUCTION DATA TEST! ERROR %08X\n",
			 __func__, ret);
	} else {
		TS_LOG_INFO( "%s PRODUCTION DATA TEST OK!\n", __func__);

		if (saveInit != NO_INIT) {
			/* save the mp flag to desired value
			 * because data check OK */
			ret = saveMpFlag(mpflag);
			if (ret < OK) {
				TS_LOG_INFO(
					 "%s Error while saving MP FLAG! ERROR %08X\n",
					 __func__, ret);
			} else {
				TS_LOG_INFO( "%s MP FLAG saving OK!\n", __func__);
            }
		}
	}

	res |= ret;
	/* the OR is important because if the data test is OK but
	  * the init test fail, the main production test result should = FAIL */

END:
	if (res < OK) {
		TS_LOG_INFO("%s MAIN Production test finished.................FAILED\n", __func__);
		return res;
	} else {
		TS_LOG_INFO("%s MAIN Production test finished.................OK\n", __func__);
		return OK;
	}
}

/**
  * Perform all the test selected in a TestTodo variable related to MS raw data
  * (touch, keys etc..)
  * @param path_limits name of Production Limit file to load or
  * "NULL" if the limits data should be loaded by a .h file
  * @param stop_on_fail if 1, the test flow stops at the first data check
  * failure
  * otherwise it keeps going performing all the selected test
  * @param todo pointer to a TestToDo variable which select the test to do
  * @return OK if success or an error code which specify the type of error
  */
int production_test_ms_raw(char *path_limits, int stop_on_fail, TestToDo *todo,
		struct ts_rawdata_info *info, TestResult *result)
{
	int ret = 0;
	int count_fail = 0;
	MutualSenseFrame msRawFrame;
	MutualSenseFrame msStrengthFrame;
	int *thresholds = NULL;
	int trows = 0;
	int tcolumns = 0;
	u16 *adj = NULL;
	short ** temp1 = NULL;
	struct fts_ts_info *st_info = fts_get_info();
	msRawFrame.node_data = NULL;
	if(!path_limits || !todo || !info || !result) {
		TS_LOG_ERR( "%s NULL data \n", __func__);
		return ret | ERROR_PROD_TEST_DATA;
	}
	/************** Mutual Sense Test *************/
	TS_LOG_INFO( "%s\n", __func__);
	TS_LOG_INFO( "%s MS RAW DATA TEST is starting...\n", __func__);
	if (todo->MutualRaw == 1 || todo->MutualRawGap == 1 ||
	    todo->MutualRawAdjGap == 1 || todo->MutualRawAdj == 1 ) {
		ret = setScanMode(SCAN_MODE_LOCKED, LOCKED_ACTIVE);
		msleep(WAIT_FOR_FRESH_FRAMES);
		ret |= setScanMode(SCAN_MODE_ACTIVE, 0x00);
		msleep(WAIT_AFTER_SENSEOFF);
		flushFIFO();
		ret |= getMSFrame3(MS_RAW, &msRawFrame);
		if (ret < OK) {
			TS_LOG_ERR(
				 "%s production_test_data: getMSFrame failed... ERROR %08X\n",
				 __func__, ERROR_PROD_TEST_DATA);
			return ret | ERROR_PROD_TEST_DATA;
		}
		ret |= getMSFrame3(MS_STRENGTH, &msStrengthFrame);
			if (ret < OK) {
				TS_LOG_ERR("%s production_test_data: getStrengthFrame failed... ERROR %08X\n",
					 __func__, ERROR_PROD_TEST_DATA);
				return ret | ERROR_PROD_TEST_DATA;
			}
		fts_mode_handler(st_info, 0);
		TS_LOG_INFO( "%s MS RAW MIN MAX TEST:\n", __func__);
		if (todo->MutualRaw == 1) {
			ret = parseProductionTestLimits(path_limits,
							&limit_file,
							MS_RAW_MIN_MAX,
							&thresholds, &trows,
							&tcolumns);
			if (ret < OK || (trows != 1 || tcolumns != 2)) {
				TS_LOG_ERR("%s production_test_data: parseProductionTestLimits MS_RAW_MIN_MAX failed... ERROR %08X\n",
					 __func__, ERROR_PROD_TEST_DATA);
				ret |= ERROR_PROD_TEST_DATA;
				goto ERROR_LIMITS;
			}

			ret = checkLimitsMinMax(msRawFrame.node_data,
						msRawFrame.header.force_node,
						msRawFrame.header.sense_node,
						thresholds[0],
						thresholds[1]);
			if (ret != OK) {
				TS_LOG_ERR("%s production_test_data: checkLimitsMinMax MS RAW failed... ERROR COUNT = %d\n",
					 __func__, ret);
				if(result != NULL)
					result->MutualRawRes = false;
				TS_LOG_INFO("%s MS RAW MIN MAX TEST:.................FAIL\n\n",__func__);
				count_fail += 1;
				if (stop_on_fail == 1)
					goto ERROR;
			} else{
				TS_LOG_INFO("%s MS RAW MIN MAX TEST:.................OK\n",__func__);
				if(result != NULL)
					result->MutualRawRes = true;
			}
			kfree(thresholds);
			thresholds = NULL;
		} else
			TS_LOG_INFO(
				 "%s MS RAW MIN MAX TEST:.................SKIPPED\n",
				 __func__);
		if(info && result){
			ret = parseProductionTestLimits(path_limits,
				&limit_file,
				MS_STRENGTH_MIN_MAX,
				&thresholds, &trows,
				&tcolumns);
			if (ret < OK || (trows != 1 || tcolumns != 2)) {
				TS_LOG_ERR("%s production_test_data: parseProductionTestLimits MS_STRENGTH_MIN_MAX failed... ERROR %08X\n",
					 __func__, ERROR_PROD_TEST_DATA);
				ret |= ERROR_PROD_TEST_DATA;
				goto ERROR_LIMITS;
			}
			ret = checkLimitsMinMax(msStrengthFrame.node_data,
				msStrengthFrame.header.force_node,
				msStrengthFrame.header.sense_node,
				thresholds[0],
				thresholds[1]);
			if (ret != OK) {
				TS_LOG_ERR("%s production_test_data: checkLimitsMinMax MS Strength failed... ERROR COUNT = %d\n",
					 __func__, ret);
				TS_LOG_INFO("%s MS Strength MIN MAX TEST:.................FAIL\n\n",
					 __func__);
				if(result != NULL)
					result->MutualStrengthRes = false;
				count_fail += 1;
				if (stop_on_fail == 1)
					goto ERROR;
			} else{
				TS_LOG_INFO("%s MS Strength MIN MAX TEST:.................OK\n",
					 __func__);
				if(result != NULL)
					result->MutualStrengthRes = true;
			}
			kfree(thresholds);
			thresholds = NULL;

		}

		TS_LOG_INFO( "%s MS RAW GAP TEST:\n", __func__);
		if (todo->MutualRawGap == 1) {
			ret = parseProductionTestLimits(path_limits,
							&limit_file, MS_RAW_GAP,
							&thresholds, &trows,
							&tcolumns);
			if (ret < OK || (trows != 1 || tcolumns != 1)) {
				TS_LOG_ERR("%s production_test_data: parseProductionTestLimits MS_RAW_GAP failed... ERROR %08X\n",
					 __func__, ERROR_PROD_TEST_DATA);
				ret |= ERROR_PROD_TEST_DATA;
				goto ERROR_LIMITS;
			}

			ret = checkLimitsGap(msRawFrame.node_data,
					     msRawFrame.header.force_node,
					     msRawFrame.header.sense_node,
					     thresholds[0]);
			if (ret != OK) {
				if(result != NULL)
					result->MutualRawResGap= false;
				TS_LOG_ERR("%s production_test_data: checkLimitsGap MS RAW failed... ERROR = %08X\n",
					 __func__, ret);
				count_fail += 1;
				if (stop_on_fail == 1)
					goto ERROR;
			} else{
				if(result != NULL)
					result->MutualRawResGap= true;
				TS_LOG_INFO("%s MS RAW GAP TEST:.................OK\n\n",__func__);
				}
			kfree(thresholds);
			thresholds = NULL;
		} else
			TS_LOG_INFO("%s MS RAW GAP TEST:.................SKIPPED\n",__func__);

		TS_LOG_INFO( "%s MS RAW ADJ TEST:\n", __func__);
		if ((todo->MutualRawAdj == 1) || (todo->MutualRawAdjGap == 1) ) {
			TS_LOG_INFO( "%s MS RAW ADJ HORIZONTAL TESTs:\n", __func__);
			ret = computeAdjHorizTotal(msRawFrame.node_data,
						   msRawFrame.header.force_node,
						   msRawFrame.header.sense_node,
						   &adj);
			if (ret < OK) {
				TS_LOG_ERR("%s production_test_data: computeAdjHoriz failed... ERROR %08X\n",
					 __func__, ERROR_PROD_TEST_DATA);
				ret |= ERROR_PROD_TEST_DATA;
				goto ERROR_LIMITS;
			}

			if (todo->MutualRawAdj) {
				TS_LOG_INFO( "%s MS RAW ADJ HORIZONTAL min/Max:\n",__func__);

				ret = parseProductionTestLimits(path_limits,
							&limit_file,
							MS_RAW_ADJH,
							&thresholds, &trows,
							&tcolumns);
				if (ret < OK || (trows !=
					 msRawFrame.header.force_node ||
					 tcolumns !=
					 msRawFrame.header.sense_node - 1)) {
					TS_LOG_ERR("%s production_test_data: parseProductionTestLimits MS_RAW_ADJH failed... ERROR %08X\n",
						__func__, ERROR_PROD_TEST_DATA);
					ret |= ERROR_PROD_TEST_DATA;
					goto ERROR_LIMITS;
				}

				ret = checkLimitsMapAdjTotal(adj,
						     msRawFrame.header.
						     force_node,
						     msRawFrame.header.
						     sense_node - 1,
						     thresholds);
				if (ret != OK) {
					TS_LOG_ERR("%s production_test_data: checkLimitsAdj MS RAW ADJH failed... ERROR COUNT = %d\n",__func__, ret);
					TS_LOG_INFO("%s MS RAW ADJ HORIZONTAL min/Max:.................FAIL\n\n", __func__);
					count_fail += 1;
					if (stop_on_fail == 1)
						goto ERROR;
				} else
					TS_LOG_INFO("%s MS RAW ADJ HORIZONTAL min/Max:.................OK\n",__func__);

				if (thresholds != NULL) {
					kfree(thresholds);
					thresholds = NULL;
				}
			}

			if (todo->MutualRawAdjGap) {
				TS_LOG_INFO( "%s MS RAW ADJ HORIZONTAL GAP:\n",__func__);

				ret = parseProductionTestLimits(path_limits,
							&limit_file,
							MS_RAW_ADJH_GAP,
							&thresholds, &trows,
							&tcolumns);
				if (ret < OK || (trows != 1 ||
					 tcolumns != 1)) {
					TS_LOG_ERR(
						"%s production_test_data: parseProductionTestLimits MS_RAW_ADJH failed... ERROR %08X\n",
						__func__, ERROR_PROD_TEST_DATA);
					ret |= ERROR_PROD_TEST_DATA;
					goto ERROR_LIMITS;
				}

				ret = checkLimitsGapOffsets(adj,
						     msRawFrame.header.
						     force_node,
						     msRawFrame.header.
						     sense_node - 1,
						     thresholds[0],
						     1,1,1,1);
				if (ret != OK) {
					TS_LOG_ERR("%s production_test_data: checkLimitsAdj MS RAW ADJH GAP failed...\n", __func__);
					TS_LOG_INFO("%s MS RAW ADJ HORIZONTAL GAP:.................FAIL\n\n",__func__);
					count_fail += 1;
					if (stop_on_fail == 1)
						goto ERROR;
				} else
					TS_LOG_INFO("%s MS RAW ADJ HORIZONTAL GAP:.................OK\n", __func__);

				if (thresholds != NULL) {
					kfree(thresholds);
					thresholds = NULL;
				}
			}
			if(adj != NULL){
				kfree(adj);
				adj = NULL;
			}
			TS_LOG_INFO( "%s MS RAW ADJ VERTICAL TESTs:\n", __func__);
			ret = computeAdjVertTotal(msRawFrame.node_data,
						  msRawFrame.header.force_node,
						  msRawFrame.header.sense_node,
						  &adj);
			if (ret < OK) {
				TS_LOG_ERR( "%s production_test_data: computeAdjVert failed... ERROR %08X\n",
					 __func__, ERROR_PROD_TEST_DATA);
				ret |= ERROR_PROD_TEST_DATA;
				goto ERROR_LIMITS;
			}

			if (todo->MutualRawAdj) {
				TS_LOG_INFO( "%s MS RAW ADJ VERTICAL min/Max:\n",__func__);
				ret = parseProductionTestLimits(path_limits,
							&limit_file,
							MS_RAW_ADJV,
							&thresholds, &trows,
							&tcolumns);
				if (ret < OK || (trows !=
					msRawFrame.header.force_node - 1 ||
					tcolumns != msRawFrame.header.sense_node)) {
					TS_LOG_ERR("%s production_test_data: parseProductionTestLimits MS_RAW_ADJV failed... ERROR %08X\n",
						 __func__, ERROR_PROD_TEST_DATA);
					ret |= ERROR_PROD_TEST_DATA;
					goto ERROR_LIMITS;
				}

				ret = checkLimitsMapAdjTotal(adj,
							     msRawFrame.header.
							     force_node - 1,
							     msRawFrame.header.
							     sense_node,
							     thresholds);
				if (ret != OK) {
					TS_LOG_ERR("%s production_test_data: checkLimitsAdj MS RAW ADJV failed... ERROR COUNT = %d\n",
						 __func__, ret);
					TS_LOG_INFO("%s MS RAW ADJ VERTICAL min/Max:.................FAIL\n\n",
						 __func__);
					count_fail += 1;
					if (stop_on_fail == 1)
						goto ERROR;
				} else
					TS_LOG_INFO( "%s MS RAW ADJ VERTICAL min/Max:.................OK\n", __func__);

				if (thresholds != NULL) {
					kfree(thresholds);
					thresholds = NULL;
				}
			}

			if (todo->MutualRawAdjGap) {
				TS_LOG_INFO( "%s MS RAW ADJ VERTICAL GAP:\n",__func__);
				ret = parseProductionTestLimits(path_limits,
							&limit_file,
							MS_RAW_ADJV_GAP,
							&thresholds, &trows,
							&tcolumns);
				if (ret < OK || (trows != 1 ||
					tcolumns != 1)) {
					TS_LOG_ERR("%s production_test_data: parseProductionTestLimits MS_RAW_ADJV_GAP failed... ERROR %08X\n",
						 __func__, ERROR_PROD_TEST_DATA);
					ret |= ERROR_PROD_TEST_DATA;
					goto ERROR_LIMITS;
				}

				ret = checkLimitsGapOffsets(adj,
							    msRawFrame.header.
							    force_node - 1,
							    msRawFrame.header.
							    sense_node,
							    thresholds[0],
							    1,1,1,1);
				if (ret != OK) {
					TS_LOG_ERR( "%s production_test_data: checkLimitsAdj MS RAW ADJV GAP failed... ERROR COUNT = %d\n",
						 __func__, ret);
					TS_LOG_INFO("%s MS RAW ADJ VERTICAL GAP:.................FAIL\n\n",__func__);
					count_fail += 1;
					if (stop_on_fail == 1)
						goto ERROR;
				} else
					TS_LOG_INFO("%s MS RAW ADJ VERTICAL GAP:.................OK\n", __func__);
					kfree(thresholds);
					thresholds = NULL;
			}
			if(adj!=NULL){
				kfree(adj);
				adj = NULL;
			}
		} else {
			TS_LOG_INFO("%s MS RAW ADJ TEST:.................SKIPPED\n", __func__);
        }
	} else {
		TS_LOG_INFO( "%s MS RAW DATA TEST:.................SKIPPED\n", __func__);
    }

	if(info  && result) {
		print_data_s16(msRawFrame.node_data, msRawFrame.header.force_node,
			msRawFrame.header.sense_node);
		print_data_s16(msStrengthFrame.node_data, msStrengthFrame.header.force_node,
			msStrengthFrame.header.sense_node);

		st_fill_rawdata_buf(info, msRawFrame.node_data, msRawFrame.header.force_node,
			msRawFrame.header.sense_node, MUTUALRAWTYPE);
		st_fill_rawdata_buf(info, msStrengthFrame.node_data, msStrengthFrame.header.force_node,
			msStrengthFrame.header.sense_node, STRENGTHTYPE);

		get_average_max_min(msRawFrame.node_data, msRawFrame.header.force_node,
			msRawFrame.header.sense_node, result->mutal_raw_res_buf);
		get_average_max_min(msStrengthFrame.node_data, msStrengthFrame.header.force_node,
			msStrengthFrame.header.sense_node, result->mutal_noise_res_buf);
	}
ERROR:
	TS_LOG_INFO( "%s\n", __func__);
	if (msStrengthFrame.node_data != NULL) {
		kfree(msStrengthFrame.node_data);
		msStrengthFrame.node_data = NULL;
	}
	if (count_fail == 0) {
		if (msRawFrame.node_data != NULL) {
			kfree(msRawFrame.node_data);
			msRawFrame.node_data = NULL;
		}
		TS_LOG_INFO("%s MS RAW DATA TEST finished!.................OK\n",__func__);
		return OK;
	} else {
		temp1 =  array1dTo2d_short(msRawFrame.node_data,msRawFrame.node_data_size,msRawFrame.header.sense_node);
		if(temp1){
			print_frame_short("MS Raw frame =",temp1,msRawFrame.header.force_node,msRawFrame.header.sense_node);
		}

		if (msRawFrame.node_data != NULL){
			kfree(msRawFrame.node_data);
			msRawFrame.node_data = NULL;
		}
		if (thresholds != NULL){
			kfree(thresholds);
			thresholds = NULL;
		}
		if (adj != NULL){
			kfree(adj);
			adj = NULL;
		}

		TS_LOG_INFO( "%s MS RAW DATA TEST:.................FAIL fails_count = %d\n\n",__func__, count_fail);
		return ERROR_PROD_TEST_DATA | ERROR_TEST_CHECK_FAIL;
	}


ERROR_LIMITS:
	if (msRawFrame.node_data != NULL){
		kfree(msRawFrame.node_data);
		msRawFrame.node_data = NULL;
	}
	if (thresholds != NULL) {
		kfree(thresholds);
		thresholds =  NULL;
	}
	return ret;
}

/**
  * Perform all the tests selected in a TestTodo variable related to MS Init
  * data (touch, keys etc..)
  * @param path_limits name of Production Limit file to load or
  * "NULL" if the limits data should be loaded by a .h file
  * @param stop_on_fail if 1, the test flow stops at the first data check
  * failure otherwise it keeps going performing all the selected test
  * @param todo pointer to a TestToDo variable which select the test to do
  * @return OK if success or an error code which specify the type of error
  */
int production_test_ms_cx(char *path_limits, int stop_on_fail, TestToDo *todo,
		struct ts_rawdata_info *info, TestResult *result)
{
	int ret = 0;
	int count_fail = 0;

	int *thresholds_min = NULL;
	int *thresholds_max = NULL;
	int trows = 0;
	int tcolumns = 0;

	MutualSenseData msCompData;
	TotMutualSenseData totCompData;
	int i = 0;
	u16 *total_adjhor = NULL;
	u16 *total_adjvert = NULL;
	int rows,columns = 0;
	u16 *temp_u16_array = 0;
	short ** temp_short = NULL;

	/* MS CX TEST */
	TS_LOG_INFO( "%s\n", __func__);
	TS_LOG_INFO( "%s MS CX Testes are starting...\n", __func__);
	if(!path_limits || !todo || !info || !result) {
			TS_LOG_ERR( "%s NULL data \n", __func__);
			return ret | ERROR_PROD_TEST_DATA;
	}

	ret = readMutualSenseCompensationData(LOAD_CX_MS_TOUCH, &msCompData);
	/* read MS compensation data */
	if (ret < 0) {
		TS_LOG_ERR( "%s production_test_data: readMutualSenseCompensationData failed... ERROR %08X\n",
			 __func__, ERROR_PROD_TEST_DATA);
		return ret | ERROR_PROD_TEST_DATA;
	}

	ret = readTotMutualSenseCompensationData(LOAD_PANEL_CX_TOT_MS_TOUCH,
						 &totCompData);
	/* read  TOT MS compensation data */
	if (ret < 0) {
		TS_LOG_ERR( "%s production_test_data: readTotMutualSenseCompensationData failed... ERROR %08X\n",
			 __func__, ERROR_PROD_TEST_DATA);
		kfree(msCompData.node_data);
		msCompData.node_data = NULL;
		return ret | ERROR_PROD_TEST_DATA;
	}

	TS_LOG_INFO( "%s MS CX2 MIN MAX TEST:\n", __func__);
	if (todo->MutualCx2 == 1) {
		ret = parseProductionTestLimits(path_limits, &limit_file,
						MS_CX2_MAP_MIN, &thresholds_min,
						&trows, &tcolumns);
						/* load min thresholds */
		if (ret < 0 || (trows != msCompData.header.force_node ||
				tcolumns != msCompData.header.sense_node)) {
			TS_LOG_ERR("%s production_test_data: parseProductionTestLimits MS_CX2_MAP_MIN failed... ERROR %08X\n",
				 __func__, ERROR_PROD_TEST_DATA);
			ret |= ERROR_PROD_TEST_DATA;
			goto ERROR_LIMITS;
		}

		ret = parseProductionTestLimits(path_limits, &limit_file,
						MS_CX2_MAP_MAX, &thresholds_max,
						&trows, &tcolumns);
						/* load max thresholds */
		if (ret < 0 || (trows != msCompData.header.force_node ||
				tcolumns != msCompData.header.sense_node)) {
			TS_LOG_ERR("%s production_test_data: parseProductionTestLimits MS_CX2_MAP_MAX failed... ERROR %08X\n",
				 __func__, ERROR_PROD_TEST_DATA);
			ret |= ERROR_PROD_TEST_DATA;
			goto ERROR_LIMITS;
		}

		ret = checkLimitsMap(msCompData.node_data,
				     msCompData.header.force_node,
				     msCompData.header.sense_node,
				     thresholds_min, thresholds_max);
					 /* check the limits */
		if (ret != OK) {
			TS_LOG_ERR("%s production_test_data: checkLimitsMap MS CX2 MIN MAX failed... ERROR COUNT = %d\n",
				 __func__, ret);
			TS_LOG_INFO( "%s MS CX2 MIN MAX TEST:.................FAIL\n\n", __func__);
			count_fail += 1;
			if (stop_on_fail)
				goto ERROR;
		} else
			TS_LOG_INFO("%s MS CX2 MIN MAX TEST:.................OK\n\n",__func__);

		kfree(thresholds_min);
		thresholds_min = NULL;
		kfree(thresholds_max);
		thresholds_max = NULL;
	} else
		TS_LOG_INFO("%s MS CX2 MIN MAX TEST:.................SKIPPED\n\n",__func__);

	/* START OF TOTAL CHECK */
	TS_LOG_INFO( "%s MS TOTAL CX TEST:\n", __func__);

	if (todo->MutualCxTotal == 1 || todo->MutualCxTotalAdj == 1) {
		TS_LOG_INFO( "%s MS TOTAL CX MIN MAX TEST:\n", __func__);
		if (todo->MutualCxTotal == 1) {
			ret = parseProductionTestLimits(path_limits,
							&limit_file,
							MS_TOTAL_CX_MAP_MIN,
							&thresholds_min,
							&trows, &tcolumns);
			/* load min thresholds */
			if (ret < 0 || (trows !=
					totCompData.header.force_node ||
					tcolumns !=
					totCompData.header.sense_node)) {
				TS_LOG_ERR("%s production_test_data: parseProductionTestLimits MS_TOTAL_CX_MAP_MIN failed... ERROR %08X\n",
					 __func__, ERROR_PROD_TEST_DATA);
				ret |= ERROR_PROD_TEST_DATA;
				goto ERROR_LIMITS;
			}

			ret = parseProductionTestLimits(path_limits,
							&limit_file,
							MS_TOTAL_CX_MAP_MAX,
							&thresholds_max,
							&trows, &tcolumns);
			/* load max thresholds */
			if (ret < 0 || (trows !=
					totCompData.header.force_node ||
					tcolumns !=
					totCompData.header.sense_node)) {
				TS_LOG_ERR("%s production_test_data: parseProductionTestLimits MS_TOTAL_CX_MAP_MAX failed... ERROR %08X\n",
					 __func__, ERROR_PROD_TEST_DATA);
				ret |= ERROR_PROD_TEST_DATA;
				goto ERROR_LIMITS;
			}

			ret = checkLimitsMapTotal(totCompData.node_data,
						  totCompData.header.force_node,
						  totCompData.header.sense_node,
						  thresholds_min,
						  thresholds_max);
			/* check the limits */
			if (ret != OK) {
				TS_LOG_ERR("%s production_test_data: checkLimitsMap  MS TOTAL CX TEST failed... ERROR COUNT = %d\n",
					 __func__, ret);
				TS_LOG_INFO("%s MS TOTAL CX MIN MAX TEST:.................FAIL\n\n",__func__);
				count_fail += 1;
				if (stop_on_fail)
					goto ERROR;
			} else
				TS_LOG_INFO("%s MS TOTAL CX MIN MAX TEST:.................OK\n\n", __func__);

			kfree(thresholds_min);
			thresholds_min = NULL;
			kfree(thresholds_max);
			thresholds_max = NULL;
		} else
			TS_LOG_INFO( "%s MS TOTAL CX MIN MAX TEST:.................SKIPPED\n\n",__func__);


		TS_LOG_INFO( "%s MS TOTAL CX ADJ TEST:\n", __func__);
		if (todo->MutualCxTotalAdj == 1) {
			/* MS TOTAL CX ADJ HORIZ */
			TS_LOG_INFO( "%s MS TOTAL CX ADJ HORIZ TEST:\n", __func__);

			ret = computeAdjHorizTotal(totCompData.node_data,
						   totCompData.header.force_node,
						   totCompData.header.sense_node,
						   &total_adjhor);
			if (ret < 0) {
				TS_LOG_ERR("%s production_test_data: computeAdjHoriz failed... ERROR %08X\n",
					 __func__, ERROR_PROD_TEST_DATA);
				ret |= ERROR_PROD_TEST_DATA;
				goto ERROR_LIMITS;
			}
			TS_LOG_INFO( "%s MS TOTAL CX ADJ HORIZ computed!\n", __func__);

			ret = parseProductionTestLimits(path_limits,
							&limit_file,
							MS_TOTAL_CX_ADJH_MAP_MAX,
							&thresholds_max,
							&trows, &tcolumns);
			if (ret < 0 || (trows !=
					totCompData.header.force_node ||
					tcolumns !=
					totCompData.header.sense_node - 1)) {
				TS_LOG_ERR( "%s production_test_data: parseProductionTestLimits MS_TOTAL_CX_ADJH_MAP_MAX failed... ERROR %08X\n",
					 __func__, ERROR_PROD_TEST_DATA);
				ret |= ERROR_PROD_TEST_DATA;
				goto ERROR_LIMITS;
			}

			ret = checkLimitsMapAdjTotal(total_adjhor,
						     totCompData.header.
						     force_node,
						     totCompData.header.
						     sense_node - 1,
						     thresholds_max);
			if (ret != OK) {
				TS_LOG_ERR( "%s production_test_data: checkLimitsMapAdj MS TOTAL CX ADJH failed... ERROR COUNT = %d\n",
					 __func__, ret);
				TS_LOG_INFO("%s MS TOTAL CX ADJ HORIZ TEST:.................FAIL\n\n", __func__);
				count_fail += 1;
				if (stop_on_fail)
					goto ERROR;
			} else
				TS_LOG_INFO( "%s MS TOTAL CX ADJ HORIZ TEST:.................OK\n\n", __func__);

			kfree(thresholds_max);
			thresholds_max = NULL;
			kfree(total_adjhor);
			total_adjhor = NULL;

			/* MS TOTAL CX ADJ VERT */
			TS_LOG_INFO( "%s MS TOTAL CX ADJ VERT TEST:\n", __func__);

			ret = computeAdjVertTotal(totCompData.node_data,
						  totCompData.header.force_node,
						  totCompData.header.sense_node,
						  &total_adjvert);
			if (ret < 0) {
				TS_LOG_ERR("%s production_test_data: computeAdjVert failed... ERROR %08X\n",
					 __func__, ERROR_PROD_TEST_DATA);
				ret |= ERROR_PROD_TEST_DATA;
				goto ERROR_LIMITS;
			}
			TS_LOG_INFO( "%s MS TOTAL CX ADJ VERT computed!\n", __func__);

			ret = parseProductionTestLimits(path_limits,
							&limit_file,
							MS_TOTAL_CX_ADJV_MAP_MAX,
							&thresholds_max,
							&trows, &tcolumns);
			if (ret < 0 || (trows != totCompData.header.force_node -
					1 || tcolumns !=
					totCompData.header.sense_node)) {
				TS_LOG_ERR("%s production_test_data: parseProductionTestLimits MS_TOTAL_CX_ADJV_MAP_MAX failed... ERROR %08X\n",
					 __func__, ERROR_PROD_TEST_DATA);
				ret |= ERROR_PROD_TEST_DATA;
				goto ERROR_LIMITS;
			}

			ret = checkLimitsMapAdjTotal(total_adjvert,
						     totCompData.header.
						     force_node - 1,
						     totCompData.header.
						     sense_node - 1,
						     thresholds_max);
			if (ret != OK) {
				TS_LOG_ERR("%s production_test_data: checkLimitsMapAdj MS TOTAL CX ADJV failed... ERROR COUNT = %d\n",
					 __func__, ret);
				TS_LOG_INFO("%s MS TOTAL CX ADJ HORIZ TEST:.................FAIL\n",
					 __func__);
				count_fail += 1;
				if (stop_on_fail)
					goto ERROR;
			} else
				TS_LOG_INFO( "%s MS TOTAL CX ADJ VERT TEST:.................OK\n",
					 __func__);

			kfree(thresholds_max);
			thresholds_max = NULL;
			kfree(total_adjvert);
			total_adjvert = NULL;
		} else
			TS_LOG_INFO("%s MS TOTAL CX ADJ TEST:.................SKIPPED\n",
				 __func__);

		kfree(totCompData.node_data);
		totCompData.node_data = NULL;
	} else
		TS_LOG_INFO( "%s MS TOTAL CX TEST:.................SKIPPED\n", __func__);



	if (todo->MutualCxTotalLP == 1) {
		ret = production_test_ms_cx_lp(path_limits, stop_on_fail, todo);
		if (ret < OK) {
			count_fail += 1;
			TS_LOG_ERR("%s production_test_data: production_test_cx_lp failed... ERROR = %08X\n",
				 __func__, ret);
			TS_LOG_INFO("%s MS CX testes finished!.................FAILED  fails_count = %d\n\n",
				 __func__, count_fail);
			return ret;
		}
	} else
		TS_LOG_INFO( "%s MS CX LP TEST:.................SKIPPED\n", __func__);

	if(info && result){
		columns= msCompData.header.sense_node;
		rows = msCompData.node_data_size/columns;

		temp_u16_array = (u16 *)kmalloc(sizeof(u16) * columns * rows, GFP_KERNEL);
		for (i = 0; i < columns * rows; i++)
			temp_u16_array[i] = (u16)msCompData.node_data[i];

		st_fill_rawdata_buf(info, temp_u16_array, rows, columns, MUTUALCOMPENSATIONTYPE);
		get_average_max_min(temp_u16_array, rows, columns, result->mutal_cal_res_buf);

		kfree(temp_u16_array);
	}


ERROR:
	TS_LOG_INFO( "%s\n", __func__);
	if (count_fail == 0) {
		TS_LOG_INFO( "%s MS CX testes finished!.................OK\n", __func__);
		kfree(msCompData.node_data);
		msCompData.node_data = NULL;
		return OK;
	} else {
		print_frame_i8("MS Init Data (Cx2) =", array1dTo2d_i8(
				       msCompData.node_data,
				       msCompData.node_data_size,
				       msCompData.header.sense_node),
			       msCompData.header.force_node,
			       msCompData.header.sense_node);
		temp_short =  array1dTo2d_short(totCompData.node_data,
					  totCompData.node_data_size,
					  totCompData.header.sense_node);
		if(temp_short){
			print_frame_short(" TOT MS Init Data (Cx) =", temp_short,
					  totCompData.header.force_node,
					  totCompData.header.sense_node);
		}
		TS_LOG_INFO( "%s MS CX testes finished!.................FAILED  fails_count = %d\n\n", __func__, count_fail);

		if (thresholds_min != NULL)
			kfree(thresholds_min);
		if (thresholds_max != NULL)
			kfree(thresholds_max);
		if (totCompData.node_data != NULL)
			kfree(totCompData.node_data);
		if (total_adjhor != NULL)
			kfree(total_adjhor);
		if (total_adjvert != NULL)
			kfree(total_adjvert);
		if (msCompData.node_data != NULL)
			kfree(msCompData.node_data);
		return ERROR_TEST_CHECK_FAIL | ERROR_PROD_TEST_DATA;
	}

ERROR_LIMITS:

	if (thresholds_min != NULL)
		kfree(thresholds_min);
	if (thresholds_max != NULL)
		kfree(thresholds_max);
	if (totCompData.node_data != NULL)
		kfree(totCompData.node_data);
	if (total_adjhor != NULL)
		kfree(total_adjhor);
	if (total_adjvert != NULL)
		kfree(total_adjvert);
	if (msCompData.node_data != NULL)
		kfree(msCompData.node_data);
	return ret;
}


/**
  * Perform all the tests selected in a TestTodo variable related to MS LowPower
  * Init data (touch, keys etc..)
  * @param path_limits name of Production Limit file to load or
  * "NULL" if the limits data should be loaded by a .h file
  * @param stop_on_fail if 1, the test flow stops at the first data check
  * failure otherwise it keeps going performing all the selected test
  * @param todo pointer to a TestToDo variable which select the test to do
  * @return OK if success or an error code which specify the type of error
  */
int production_test_ms_cx_lp(char *path_limits, int stop_on_fail, TestToDo *todo)
{
	int ret = 0;
	int count_fail = 0;

	//int *thresholds = NULL;
	int *thresholds_min = NULL;
	int *thresholds_max = NULL;
	int trows = 0;
	int tcolumns = 0;
	short ** temp_short = NULL;
	MutualSenseData msCompData;
	TotMutualSenseData totCompData;


	//u16 container;
	/* u16 *total_cx = NULL; */
	//u16 *total_adjhor = NULL;
	//u16 *total_adjvert = NULL;


	/* MS CX TEST */
	TS_LOG_INFO( "%s\n", __func__);
	TS_LOG_INFO( "%s MS LP CX Testes are starting...\n", __func__);
	if(!path_limits || !todo) {
			TS_LOG_ERR( "%s NULL data \n", __func__);
			return ret | ERROR_PROD_TEST_DATA;
	}

	ret = readMutualSenseCompensationData(LOAD_CX_MS_LOW_POWER, &msCompData);
	/* read MS compensation data */
	if (ret < 0) {
		TS_LOG_ERR("%s : readMutualSenseCompensationData failed... ERROR %08X\n",
			 __func__, ERROR_PROD_TEST_DATA);
		return ret | ERROR_PROD_TEST_DATA;
	}

	ret = readTotMutualSenseCompensationData(LOAD_PANEL_CX_TOT_MS_LOW_POWER,
						 &totCompData);
	/* read  TOT MS compensation data */
	if (ret < 0) {
		TS_LOG_ERR( "%s : readTotMutualSenseCompensationData failed... ERROR %08X\n",
			 __func__, ERROR_PROD_TEST_DATA);
		kfree(msCompData.node_data);
		msCompData.node_data = NULL;
		return ret | ERROR_PROD_TEST_DATA;
	}

	/* START OF TOTAL CHECK */
	TS_LOG_INFO( "%s MS TOTAL LP CX TEST:\n", __func__);

	if (todo->MutualCxTotalLP == 1 ) {
		TS_LOG_INFO( "%s MS TOTAL LP CX MIN MAX TEST:\n", __func__);
			ret = parseProductionTestLimits(path_limits,
							&limit_file,
							MS_TOTAL_CX_LP_MAP_MIN,
							&thresholds_min,
							&trows, &tcolumns);
			/* load min thresholds */
			if (ret < 0 || (trows !=
					totCompData.header.force_node ||
					tcolumns !=
					totCompData.header.sense_node)) {
				TS_LOG_ERR( "%s : parseProductionTestLimits MS_TOTAL_CX_LP_MAP_MIN failed... ERROR %08X\n",
					 __func__, ERROR_PROD_TEST_DATA);
				ret |= ERROR_PROD_TEST_DATA;
				goto ERROR_LIMITS;
			}

			ret = parseProductionTestLimits(path_limits,
							&limit_file,
							MS_TOTAL_CX_LP_MAP_MAX,
							&thresholds_max,
							&trows, &tcolumns);
			/* load max thresholds */
			if (ret < 0 || (trows !=
					totCompData.header.force_node ||
					tcolumns !=
					totCompData.header.sense_node)) {
				TS_LOG_ERR( "%s : parseProductionTestLimits MS_TOTAL_CX_LP_MAP_MAX failed... ERROR %08X\n",
					 __func__, ERROR_PROD_TEST_DATA);
				ret |= ERROR_PROD_TEST_DATA;
				goto ERROR_LIMITS;
			}

			ret = checkLimitsMapTotal(totCompData.node_data,
						  totCompData.header.force_node,
						  totCompData.header.sense_node,
						  thresholds_min,
						  thresholds_max);
			/* check the limits */
			if (ret != OK) {
				TS_LOG_ERR( "%s : checkLimitsMap  MS TOTAL CX LP TEST failed... ERROR COUNT = %d\n",
					 __func__, ret);
				TS_LOG_INFO("%s MS TOTAL CX LP MIN MAX TEST:..........FAIL\n\n",__func__);
				count_fail += 1;
				if (stop_on_fail)
					goto ERROR;
			} else
				TS_LOG_INFO( "%s MS TOTAL CX LP MIN MAX TEST:...........OK\n\n",__func__);

			kfree(thresholds_min);
			thresholds_min = NULL;
			kfree(thresholds_max);
			thresholds_max = NULL;

		kfree(totCompData.node_data);
		totCompData.node_data = NULL;
	} else
		TS_LOG_INFO( "%s MS TOTAL CX LP TEST:.................SKIPPED\n", __func__);



ERROR:
	TS_LOG_INFO( "%s\n", __func__);
	if (count_fail == 0) {
		TS_LOG_INFO( "%s MS LP CX testes finished!.................OK\n",__func__);
		kfree(msCompData.node_data);
		msCompData.node_data = NULL;
		return OK;
	} else {
		print_frame_i8("MS LP Init Data (Cx2) =", array1dTo2d_i8(
				       msCompData.node_data,
				       msCompData.node_data_size,
				       msCompData.header.sense_node),
			       msCompData.header.force_node,
			       msCompData.header.sense_node);
		temp_short = array1dTo2d_short( totCompData.node_data,
					  totCompData.node_data_size,
					  totCompData.header.sense_node);
		if(temp_short){
			print_frame_short(" TOT MS LP Init Data (Cx) =", temp_short,
							  totCompData.header.force_node,
							  totCompData.header.sense_node);
		}
		TS_LOG_INFO( "%s MS LP CX testes finished!.................FAILED  fails_count = %d\n\n",
			 __func__, count_fail);
		if (thresholds_min != NULL){
			kfree(thresholds_min);
			thresholds_min = NULL;
		}
		if (thresholds_max != NULL){
			kfree(thresholds_max);
			thresholds_max = NULL;
		}
		if (totCompData.node_data != NULL){
			kfree(totCompData.node_data);
			totCompData.node_data = NULL;
		}
		if (msCompData.node_data != NULL){
			kfree(msCompData.node_data);
			msCompData.node_data = NULL;
		}
		return ERROR_TEST_CHECK_FAIL | ERROR_PROD_TEST_DATA;
	}

ERROR_LIMITS:

	if (thresholds_min != NULL){
		kfree(thresholds_min);
		thresholds_min = NULL;
	}
	if (thresholds_max != NULL){
		kfree(thresholds_max);
		thresholds_max = NULL;
	}
	if (totCompData.node_data != NULL){
		kfree(totCompData.node_data);
		totCompData.node_data = NULL;
	}
	if (msCompData.node_data != NULL){
		kfree(msCompData.node_data);
		msCompData.node_data = NULL;
	}
	return ret;
}

/**
  * Perform all the test selected in a TestTodo variable related to SS raw data
  * (touch, keys etc..)
  * @param path_limits name of Production Limit file to load or
  * "NULL" if the limits data should be loaded by a .h file
  * @param stop_on_fail if 1, the test flow stops at the first data check
  * failure
  * otherwise it keeps going performing all the selected test
  * @param todo pointer to a TestToDo variable which select the test to do
  * @return OK if success or an error code which specify the type of error
  */
int production_test_ss_raw(char *path_limits, int stop_on_fail, TestToDo *todo,
		struct ts_rawdata_info *info,TestResult *result)
{
	int ret=0;
	int count_fail = 0;
	int rows = 0;
	int columns = 0;

	SelfSenseFrame ssRawFrame;
	SelfSenseFrame ssStrengthFrame;
	int *thresholds = NULL;
	int trows = 0;
	int tcolumns = 0;
	short ** temp_short = NULL;
	struct fts_ts_info *st_info = fts_get_info();
	/* SS TEST */

	TS_LOG_INFO( "%s SS RAW Testes are starting...\n", __func__);
	if(!path_limits || !todo || !info || !result) {
			TS_LOG_ERR( "%s NULL data \n", __func__);
			return ret | ERROR_PROD_TEST_DATA;
	}
	/************** Self Sense Test **************/

	TS_LOG_INFO( "%s Getting SS Frame...\n", __func__);
	ret = setScanMode(SCAN_MODE_LOCKED, LOCKED_ACTIVE);
	msleep(WAIT_FOR_FRESH_FRAMES);
	ret |= setScanMode(SCAN_MODE_ACTIVE, 0x00);
	msleep(WAIT_AFTER_SENSEOFF);
	flushFIFO();
	ret |= getSSFrame3(SS_RAW, &ssRawFrame);
	if (ret < 0) {
		TS_LOG_ERR("%s production_test_data: getSSFrame failed... ERROR %08X\n",
			 __func__, ERROR_PROD_TEST_DATA);
		return ret | ERROR_PROD_TEST_DATA;
	}
	ret |= getSSFrame3(SS_STRENGTH, &ssStrengthFrame);
	if (ret < 0) {
		TS_LOG_ERR("%s production_test_data: getSS Strength Frame failed... ERROR %08X\n",
			 __func__, ERROR_PROD_TEST_DATA);
        if(ssRawFrame.force_data != NULL){
	    	kfree(ssRawFrame.force_data);
	    	ssRawFrame.force_data = NULL;
	    }
	    if(ssRawFrame.sense_data != NULL){
	    	kfree(ssRawFrame.sense_data);
	    	ssRawFrame.sense_data = NULL;
	    }
		return ret | ERROR_PROD_TEST_DATA;
	}
	fts_mode_handler(st_info,0);
	/* SS RAW (PROXIMITY) FORCE TEST */
	TS_LOG_INFO( "%s SS RAW FORCE TEST:\n", __func__);



	if (todo->SelfForceRaw == 1 ) {
		columns = 1;	/* there are no data for the sense channels
				  * because is a force frame */
		rows = ssRawFrame.header.force_node;

		TS_LOG_INFO( "%s SS RAW FORCE MIN MAX TEST:\n", __func__);
		if (todo->SelfForceRaw == 1) {
			ret = parseProductionTestLimits(path_limits,
							&limit_file,
							SS_RAW_FORCE_MIN_MAX,
							&thresholds, &trows,
							&tcolumns);
			if (ret < 0 || (trows != 1 || tcolumns != 2)) {
				TS_LOG_ERR( "%s production_test_data: parseProductionTestLimits SS_RAW_FORCE_MIN_MAX failed... ERROR %08X\n",
					 __func__, ERROR_PROD_TEST_DATA);
				ret |= ERROR_PROD_TEST_DATA;
				goto ERROR_LIMITS;
			}

			ret = checkLimitsMinMax(ssRawFrame.force_data, rows,
						columns, thresholds[0],
						thresholds[1]);
			if (ret != OK) {
				TS_LOG_ERR( "%s production_test_data: checkLimitsMinMax SS RAW FORCE failed... ERROR COUNT = %d\n",
					 __func__, ret);
				TS_LOG_INFO("%s SS RAW (PROXIMITY) FORCE MIN MAX TEST:.................FAIL\n\n", __func__);
				count_fail += 1;
				if(result != NULL)
					result->SelfForceRawRes = false;
				temp_short =  array1dTo2d_short(ssRawFrame.force_data,
							  					rows * columns, columns);
				if(temp_short){
					print_frame_short("SS Raw force frame =",temp_short , rows,columns);
				}

				if (stop_on_fail) {
					ret = ERROR_PROD_TEST_DATA |
					      ERROR_TEST_CHECK_FAIL;
					goto ERROR_LIMITS;
				}
			} else{
				if(result != NULL)
					result->SelfForceRawRes = true;
				TS_LOG_INFO( "%s SS RAW FORCE MIN MAX TEST:.................OK\n\n", __func__);
			}
			kfree(thresholds);
			thresholds = NULL;
		} else
			TS_LOG_INFO("%s SS RAW FORCE MIN MAX TEST:.................SKIPPED\n\n", __func__);

	} else
		TS_LOG_INFO("%s SS RAW FORCE TEST:.................SKIPPED\n\n",__func__);

	TS_LOG_INFO( "%s\n", __func__);
	/* SS RAW (PROXIMITY) SENSE TEST */
	TS_LOG_INFO( "%s SS RAW SENSE TEST:\n", __func__);

	if (todo->SelfSenseRaw == 1) {
		columns = ssRawFrame.header.sense_node;
		rows = 1; /* there are no data for the force channels
			  *  because is a sense frame */

		TS_LOG_INFO( "%s SS RAW SENSE MIN MAX TEST:\n", __func__);
		if (todo->SelfSenseRaw == 1) {
			ret = parseProductionTestLimits(path_limits,
							&limit_file,
							SS_RAW_SENSE_MIN_MAX,
							&thresholds, &trows,
							&tcolumns);
			if (ret < 0 || (trows != 1 || tcolumns != 2)) {
				TS_LOG_ERR( "%s production_test_data: parseProductionTestLimits SS_RAW_SENSE_MIN_MAX failed... ERROR %08X\n",
					 __func__, ERROR_PROD_TEST_DATA);
				ret |= ERROR_PROD_TEST_DATA;
				goto ERROR_LIMITS;
			}

			ret = checkLimitsMinMax(ssRawFrame.sense_data, rows,
						columns, thresholds[0],
						thresholds[1]);
			if (ret != OK) {
				TS_LOG_ERR( "%s production_test_data: checkLimitsMinMax SS RAW SENSE failed... ERROR COUNT = %d\n",
					 __func__, ret);
				TS_LOG_INFO( "%s SS RAW SENSE MIN MAX TEST:.................FAIL\n", __func__);
				count_fail += 1;
				if(result != NULL)
					result->SelfSenseRawRes = false;
				temp_short =  array1dTo2d_short( ssRawFrame.sense_data, rows * columns, columns);
				if(temp_short){
					print_frame_short("SS Raw sense frame =",temp_short , rows,columns);
				}
				if (stop_on_fail) {
					ret = ERROR_PROD_TEST_DATA |
					      ERROR_TEST_CHECK_FAIL;
					goto ERROR_LIMITS;
				}
			} else{
				if(result != NULL)
					result->SelfSenseRawRes = true;
				TS_LOG_INFO( "%s SS RAW SENSE MIN MAX TEST:.................OK\n", __func__);
			}
			kfree(thresholds);
			thresholds = NULL;
		} else
			TS_LOG_INFO( "%s SS RAW SENSE MIN MAX TEST:.................SKIPPED\n", __func__);


		if(result  && info) {
			ret = parseProductionTestLimits(path_limits,
							&limit_file,
							SS_STRENGTH_FORCE_MIN_MAX,
							&thresholds, &trows,
							&tcolumns);
			if (ret < 0 || (trows != 1 || tcolumns != 2)) {
				TS_LOG_ERR("%s production_test_data: parseProductionTestLimits SS_STRENGTH_FORCE_MIN_MAX failed... ERROR %08X\n",
					 __func__, ERROR_PROD_TEST_DATA);
				ret |= ERROR_PROD_TEST_DATA;
				goto ERROR_LIMITS;
			}

			ret = checkLimitsMinMax(ssStrengthFrame.force_data, ssStrengthFrame.header.force_node,
						1, thresholds[0],
						thresholds[1]);
			if (ret != OK) {
				TS_LOG_ERR( "%s production_test_data:SS_STRENGTH_FORCE_MIN_MAX failed... ERROR COUNT = %d\n",
					 __func__, ret);
				TS_LOG_INFO("%s SS SS_STRENGTH_FORCE_MIN_MAX TEST:.................FAIL\n\n", __func__);
				if(result != NULL)
					result->SelfForceStrengthData = false;
			} else{
				if(result != NULL)
					result->SelfForceStrengthData = true;
				TS_LOG_INFO("%s SS_STRENGTH_FORCE_MIN_MAX TEST:.................OK\n\n",__func__);
			}
			if(thresholds == NULL){
				kfree(thresholds);
				thresholds = NULL;
			}

			ret = parseProductionTestLimits(path_limits,
				&limit_file,
				SS_STRENGTH_SENSE_MIN_MAX,
				&thresholds, &trows,
				&tcolumns);
			if (ret < 0 || (trows != 1 || tcolumns != 2)) {
				TS_LOG_ERR( "%s production_test_data: parseProductionTestLimits SS_STRENGTH_SENSE_MIN_MAX failed... ERROR %08X\n",
					 __func__, ERROR_PROD_TEST_DATA);
				ret |= ERROR_PROD_TEST_DATA;
				goto ERROR_LIMITS;
			}

			ret = checkLimitsMinMax(ssStrengthFrame.sense_data, 1,
						ssStrengthFrame.header.sense_node, thresholds[0],
						thresholds[1]);
			if (ret != OK) {
				TS_LOG_ERR( "%s production_test_data:SS_STRENGTH_SENSE_MIN_MAX failed... ERROR COUNT = %d\n",
					 __func__, ret);
				TS_LOG_INFO( "%s SS SS_STRENGTH_SENSE_MIN_MAX TEST:.................FAIL\n\n", __func__);
				if(result != NULL)
					result->SelfSenseStrengthData = false;

			} else{
				if(result != NULL)
					result->SelfSenseStrengthData = true;
				TS_LOG_INFO("%s SS_STRENGTH_SENSE_MIN_MAX TEST:.................OK\n\n",__func__);
			}
				kfree(thresholds);
				thresholds = NULL;
			TS_LOG_INFO("%s.\n\n",__func__);
			print_data_s16(ssStrengthFrame.sense_data, 1, ssStrengthFrame.header.sense_node);
			st_fill_rawdata_buf(info, ssStrengthFrame.sense_data, 1, ssStrengthFrame.header.sense_node, SSSENSEPRXTYPE);

			print_data_s16(ssStrengthFrame.force_data, ssStrengthFrame.header.force_node, 1);
			st_fill_rawdata_buf(info, ssStrengthFrame.force_data, ssStrengthFrame.header.force_node, 1, SSFORCEPRXTYPE);

			print_data_s16(ssRawFrame.sense_data, 1, ssRawFrame.header.sense_node);
			st_fill_rawdata_buf(info, ssRawFrame.sense_data, 1, ssRawFrame.header.sense_node, SELFSENSERAWTYPE);

			print_data_s16(ssRawFrame.force_data, ssRawFrame.header.force_node, 1);
			st_fill_rawdata_buf(info, ssRawFrame.force_data, ssRawFrame.header.force_node, 1, SELFFORCERAWTYPE);		

		}

	} else {
		TS_LOG_INFO("%s SS RAW SENSE TEST:.................SKIPPED\n\n", __func__);
	}
	if(ssRawFrame.force_data != NULL){
		kfree(ssRawFrame.force_data);
		ssRawFrame.force_data = NULL;
	}
	if(ssRawFrame.sense_data != NULL){
		kfree(ssRawFrame.sense_data);
		ssRawFrame.sense_data = NULL;
	}
	if(ssStrengthFrame.force_data!= NULL){
		kfree(ssStrengthFrame.force_data);
		ssStrengthFrame.force_data = NULL;
	}
	if(ssStrengthFrame.sense_data != NULL){
		kfree(ssStrengthFrame.sense_data);
		ssStrengthFrame.sense_data = NULL;
	}
	ret = production_test_ss_raw_lp(path_limits, stop_on_fail, todo);
	if (ret < OK) {
		TS_LOG_ERR( "%s production_test_data: production_test_ss_raw_lp failed... ERROR = %08X\n",__func__ ,ret);
	}

	TS_LOG_INFO( "%s\n", __func__);
	if (count_fail == 0) {
		TS_LOG_INFO( "%s SS RAW testes finished!.................OK\n\n",__func__);
		return OK;
	} else {
		TS_LOG_INFO("%s SS RAW testes finished!.................FAILED  fails_count = %d\n\n", __func__, count_fail);
		return ERROR_TEST_CHECK_FAIL | ERROR_PROD_TEST_DATA;
	}

ERROR_LIMITS:
	if (ssRawFrame.force_data != NULL)
		kfree(ssRawFrame.force_data);
	if (ssRawFrame.sense_data != NULL)
		kfree(ssRawFrame.sense_data);
	if (thresholds != NULL)
		kfree(thresholds);
    if(ssStrengthFrame.force_data!= NULL){
		kfree(ssStrengthFrame.force_data);
		ssStrengthFrame.force_data = NULL;
	}
	if(ssStrengthFrame.sense_data != NULL){
		kfree(ssStrengthFrame.sense_data);
		ssStrengthFrame.sense_data = NULL;
	}

	return ret;
}


/**
  * Perform all the test selected in a TestTodo variable related to SS raw data
  * low power
  * @param path_limits name of Production Limit file to load or
  * "NULL" if the limits data should be loaded by a .h file
  * @param stop_on_fail if 1, the test flow stops at the first data check
  * failure
  *  otherwise it keeps going performing all the selected test
  * @param todo pointer to a TestToDo variable which select the test to do
  * @return OK if success or an error code which specify the type of error
  */
int production_test_ss_raw_lp(char *path_limits, int stop_on_fail,
			      TestToDo *todo)
{
	int ret=0;
	int count_fail = 0;
	int rows = 0;
	int columns = 0;

	SelfSenseFrame ssRawFrame;

	int *thresholds = NULL;
	int trows = 0;
	int tcolumns = 0;
	short ** temp_short = NULL;

	/* SS TEST */
	TS_LOG_INFO( "%s\n", __func__);
	TS_LOG_INFO( "%s SS RAW LP Testes are starting...\n", __func__);
	if(!path_limits || !todo) {
			TS_LOG_ERR( "%s NULL data \n", __func__);
			return ret | ERROR_PROD_TEST_DATA;
	}
	/************** Self Sense Test **************/

	TS_LOG_INFO( "%s Getting SS LP Frame...\n", __func__);
	//ret = setScanMode(SCAN_MODE_LOCKED, LOCKED_LP_DETECT);
	//msleep(WAIT_FOR_FRESH_FRAMES);
	//ret |= setScanMode(SCAN_MODE_ACTIVE, 0x00);
	//msleep(WAIT_AFTER_SENSEOFF);
	ret |= getSSFrame3(SS_DETECT_RAW, &ssRawFrame);
	if (ret < 0) {
		TS_LOG_ERR( "%s production_test_data: getSSFrame failed... ERROR %08X\n",
			 __func__, ERROR_PROD_TEST_DATA);
		return ret | ERROR_PROD_TEST_DATA;
	}

	/* SS RAW (PROXIMITY) FORCE TEST */
	TS_LOG_INFO( "%s SS RAW LP FORCE TEST:\n", __func__);



	if ((todo->SelfForceRawLP == 1 ) &&
		ssRawFrame.header.force_node != 0) {
		columns = 1;	/* there are no data for the sense channels
				  *  because is a force frame */
		rows = ssRawFrame.header.force_node;

		TS_LOG_INFO( "%s SS RAW LP FORCE MIN MAX TEST:\n", __func__);
		if (todo->SelfForceRawLP == 1) {
			ret = parseProductionTestLimits(path_limits,
							&limit_file,
							SS_RAW_LP_FORCE_MIN_MAX,
							&thresholds,
							&trows, &tcolumns);
			if (ret < 0 || (trows != 1 || tcolumns != 2)) {
				TS_LOG_ERR( "%s production_test_data: parseProductionTestLimits SS_RAW_FORCE_MIN_MAX failed... ERROR %08X\n",
					 __func__, ERROR_PROD_TEST_DATA);
				ret |= ERROR_PROD_TEST_DATA;
				goto ERROR_LIMITS;
			}

			ret = checkLimitsMinMax(ssRawFrame.force_data, rows,
						columns, thresholds[0],
						thresholds[1]);
			if (ret != OK) {
				TS_LOG_ERR( "%s production_test_data: checkLimitsMinMax SS RAW FORCE failed... ERROR COUNT = %d\n",
					 __func__, ret);
				TS_LOG_INFO("%s SS RAW LP FORCE MIN MAX TEST:.................FAIL\n\n", __func__);
				count_fail += 1;
				temp_short =  array1dTo2d_short(ssRawFrame.force_data,
							 						 rows *columns,columns);
				if(temp_short){
					print_frame_short("SS Raw LP force frame =",temp_short , rows,columns);
				}

				if (stop_on_fail) {
					ret = ERROR_PROD_TEST_DATA |
					      ERROR_TEST_CHECK_FAIL;
					goto ERROR_LIMITS;
				}
			} else
				TS_LOG_INFO("%s SS RAW LP FORCE MIN MAX TEST:.................OK\n\n", __func__);

			kfree(thresholds);
			thresholds = NULL;
		} else {
			TS_LOG_INFO("%s SS RAW LP FORCE MIN MAX TEST:.................SKIPPED\n\n",__func__);
        }
	} else {
		TS_LOG_INFO("%s SS RAW LP FORCE TEST:.................SKIPPED\n\n",__func__);
    }

    if (ssRawFrame.force_data != NULL)
		kfree(ssRawFrame.force_data);
	if (ssRawFrame.sense_data != NULL)
		kfree(ssRawFrame.sense_data);

	if (count_fail == 0) {
		TS_LOG_INFO("%s SS RAW LP testes finished!.................OK\n\n",__func__);
		return OK;
	} else {
		TS_LOG_INFO("%s SS RAW LP testes finished!.................FAILED  fails_count = %d\n\n",__func__, count_fail);
		return ERROR_TEST_CHECK_FAIL | ERROR_PROD_TEST_DATA;
	}

ERROR_LIMITS:
	if (ssRawFrame.force_data != NULL)
		kfree(ssRawFrame.force_data);
	if (ssRawFrame.sense_data != NULL)
		kfree(ssRawFrame.sense_data);
	if (thresholds != NULL)
		kfree(thresholds);

	return ret;
}

/**
  * Perform all the tests selected in a TestTodo variable related to SS Init
  * data (touch, keys etc..)
  * @param path_limits name of Production Limit file to load or
  * "NULL" if the limits data should be loaded by a .h file
  * @param stop_on_fail if 1, the test flow stops at the first data check
  * failure
  * otherwise it keeps going performing all the selected test
  * @param todo pointer to a TestToDo variable which select the test to do
  * @return OK if success or an error code which specify the type of error
  */
int production_test_ss_ix_cx(char *path_limits, int stop_on_fail,
			     TestToDo *todo)
{
	int ret = 0;
	int count_fail = 0;

	int trows = 0;
	int tcolumns = 0;
	int *thresholds_min = NULL;
	int *thresholds_max = NULL;
	u8** temp_u8 = NULL;
	i8** temp_i8 = NULL;
	u16 ** temp_u16 = NULL;
	short ** temp_short = NULL;

	SelfSenseData ssCompData;
	TotSelfSenseData totCompData;

	TS_LOG_INFO( "%s\n", __func__);
	TS_LOG_INFO( "%s SS IX CX testes are starting...\n", __func__);
	if(!path_limits || !todo) {
		TS_LOG_ERR( "%s NULL data \n", __func__);
		return ret | ERROR_PROD_TEST_DATA;
	}
	ret = readSelfSenseCompensationData(LOAD_CX_SS_TOUCH, &ssCompData);
	/* read the SS compensation data */
	if (ret < 0) {
		TS_LOG_ERR( "%s production_test_data: readSelfSenseCompensationData failed... ERROR %08X\n",
			 __func__, ERROR_PROD_TEST_DATA);
		return ret | ERROR_PROD_TEST_DATA;
	}

	ret = readTotSelfSenseCompensationData(LOAD_PANEL_CX_TOT_SS_TOUCH,
					       &totCompData);
	/* read the TOT SS compensation data */
	if (ret < 0) {
		TS_LOG_ERR( "%s production_test_data: readTotSelfSenseCompensationData failed... ERROR %08X\n",
			 __func__, ERROR_PROD_TEST_DATA);
		kfree(ssCompData.ix2_fm);
		ssCompData.ix2_fm = NULL;
		kfree(ssCompData.ix2_sn);
		ssCompData.ix2_sn = NULL;
		kfree(ssCompData.cx2_fm);
		ssCompData.cx2_fm = NULL;
		kfree(ssCompData.cx2_sn);
		ssCompData.cx2_sn = NULL;
		return ret | ERROR_PROD_TEST_DATA;
	}


	/* SS TOTAL FORCE IX */
	TS_LOG_INFO( "%s SS TOTAL IX FORCE TEST:\n", __func__);
	if (todo->SelfForceIxTotal == 1) {
		TS_LOG_INFO( "%s SS TOTAL IX FORCE MIN MAX TEST:\n", __func__);
		if (todo->SelfForceIxTotal == 1) {
			ret = parseProductionTestLimits(path_limits,
							&limit_file,
							SS_TOTAL_IX_FORCE_MAP_MIN,
							&thresholds_min,
							&trows, &tcolumns);
						/* load the min thresholds */
			if (ret < 0 || (trows !=
					totCompData.header.force_node ||
					tcolumns != 1)) {
				TS_LOG_ERR( "%s production_test_data: parseProductionTestLimits SS_TOTAL_IX_FORCE_MAP_MIN failed... ERROR %08X\n",
					 __func__, ERROR_PROD_TEST_DATA);
				ret |= ERROR_PROD_TEST_DATA;
				goto ERROR_LIMITS;
			}

			ret = parseProductionTestLimits(path_limits,
							&limit_file,
							SS_TOTAL_IX_FORCE_MAP_MAX,
							&thresholds_max,
							&trows, &tcolumns);
						/* load the max thresholds */
			if (ret < 0 || (trows !=
					totCompData.header.force_node ||
					tcolumns != 1)) {
				TS_LOG_ERR("%s production_test_data: parseProductionTestLimits SS_TOTAL_IX_FORCE_MAP_MAX failed... ERROR %08X\n",
					 __func__, ERROR_PROD_TEST_DATA);
				ret |= ERROR_PROD_TEST_DATA;
				goto ERROR_LIMITS;
			}

			ret = checkLimitsMapTotalFromU(totCompData.ix_fm,
						       totCompData.header.force_node, 1,
						       thresholds_min,
						       thresholds_max);
			/* check the values with thresholds */
			if (ret != OK) {
				TS_LOG_ERR("%s production_test_data: checkLimitsMap  SS TOTAL IX FORCE failed... ERROR COUNT = %d\n",
					 __func__, ret);
				TS_LOG_INFO( "%s SS TOTAL IX FORCE MIN MAX TEST:.................FAIL\n\n", __func__);
				count_fail += 1;
				if (stop_on_fail)
					goto ERROR;
			} else
				TS_LOG_INFO( "%s SS TOTAL IX FORCE MIN MAX TEST:.................OK\n\n", __func__);

			kfree(thresholds_min);
			thresholds_min = NULL;
			kfree(thresholds_max);
			thresholds_max = NULL;
		} else
			TS_LOG_INFO("%s SS TOTAL IX FORCE MIN MAX TEST:.................SKIPPED\n",__func__);

	} else
		TS_LOG_INFO("%s SS TOTAL IX FORCE TEST:.................SKIPPED\n\n",__func__);

	/* SS TOTAL IX SENSE */
	TS_LOG_INFO( "%s SS TOTAL IX SENSE TEST:\n", __func__);
	if (todo->SelfSenseIxTotal == 1 ) {
		TS_LOG_INFO( "%s SS TOTAL IX SENSE MIN MAX TEST:\n", __func__);
		if (todo->SelfSenseIxTotal == 1) {
			ret = parseProductionTestLimits(path_limits,
							&limit_file,
							SS_TOTAL_IX_SENSE_MAP_MIN,
							&thresholds_min,
							&trows, &tcolumns);
			/* load the min thresholds */
			if (ret < 0 || (trows != 1 || tcolumns !=
					totCompData.header.sense_node)) {
				TS_LOG_ERR( "%s production_test_data: parseProductionTestLimits SS_TOTAL_IX_SENSE_MAP_MIN failed... ERROR %08X\n",
					 __func__, ERROR_PROD_TEST_DATA);
				ret |= ERROR_PROD_TEST_DATA;
				goto ERROR_LIMITS;
			}

			ret = parseProductionTestLimits(path_limits,
							&limit_file,
							SS_TOTAL_IX_SENSE_MAP_MAX,
							&thresholds_max,
							&trows, &tcolumns);
			/* load the max thresholds */
			if (ret < 0 || (trows != 1 || tcolumns !=
					totCompData.header.sense_node)) {
				TS_LOG_ERR( "%s production_test_data: parseProductionTestLimits SS_TOTAL_IX_SENSE_MAP_MAX failed... ERROR %08X\n",
					 __func__, ERROR_PROD_TEST_DATA);
				ret |= ERROR_PROD_TEST_DATA;
				goto ERROR_LIMITS;
			}

			ret = checkLimitsMapTotalFromU(totCompData.ix_sn, 1,
						       totCompData.header.sense_node,
						       thresholds_min,
						       thresholds_max);
			/* check the values with thresholds */
			if (ret != OK) {
				TS_LOG_ERR( "%s production_test_data: checkLimitsMap SS TOTAL IX SENSE failed... ERROR COUNT = %d\n",
					 __func__, ret);
				TS_LOG_INFO("%s SS TOTAL IX SENSE MIN MAX TEST:.................FAIL\n\n",__func__);
				count_fail += 1;
				if (stop_on_fail)
					goto ERROR;
			} else
				TS_LOG_INFO("%s SS TOTAL IX SENSE MIN MAX TEST:.................OK\n\n", __func__);

			kfree(thresholds_min);
			thresholds_min = NULL;
			kfree(thresholds_max);
			thresholds_max = NULL;
		} else {
			TS_LOG_INFO("%s SS TOTAL IX SENSE MIN MAX TEST:.................SKIPPED\n", __func__);
		}
	} else {
		TS_LOG_INFO("%s SS TOTAL IX SENSE TEST:.................SKIPPED\n", __func__);
	}


	if (todo->SelfSenseIxTotalLP == 1) {
		ret = production_test_ss_ix_cx_lp(path_limits, stop_on_fail,
			todo);
		if (ret < OK) {
			count_fail += 1;
			TS_LOG_ERR("%s production_test_data: production_test_ss_ix_cx_lp failed... ERROR = %08X\n", __func__, ret);
			goto ERROR;
		}
	} else
		TS_LOG_INFO( "%s SS IX CX LP TEST:.................SKIPPED\n",__func__);

ERROR:
	TS_LOG_INFO( "%s\n", __func__);
	if (count_fail == 0) {
		kfree(ssCompData.ix2_fm);
		ssCompData.ix2_fm = NULL;
		kfree(ssCompData.ix2_sn);
		ssCompData.ix2_sn = NULL;
		kfree(ssCompData.cx2_fm);
		ssCompData.cx2_fm = NULL;
		kfree(ssCompData.cx2_sn);
		ssCompData.cx2_sn = NULL;
		kfree(totCompData.ix_fm);
		totCompData.ix_fm = NULL;
		kfree(totCompData.ix_sn);
		totCompData.ix_sn = NULL;
		kfree(totCompData.cx_fm);
		totCompData.cx_fm = NULL;
		kfree(totCompData.cx_sn);
		totCompData.cx_sn = NULL;
		TS_LOG_INFO("%s SS IX CX testes finished!.................OK\n\n", __func__);
		return OK;
	} else {
	/* print all kind of data in just one row for readability reason */
		temp_u8 = array1dTo2d_u8(ssCompData.ix2_fm, ssCompData.header.force_node, 1);
		if(temp_u8){
			print_frame_u8("SS Init Data Ix2_fm = ",temp_u8 ,
			       ssCompData.header.force_node, 1);
		}
		temp_i8 =  array1dTo2d_i8(ssCompData.cx2_fm,ssCompData.header.force_node, 1);
		if(temp_i8){
			print_frame_i8("SS Init Data Cx2_fm = ",temp_i8, ssCompData.header.force_node, 1);
		}
		temp_u8 = array1dTo2d_u8(ssCompData.ix2_sn,ssCompData.header.sense_node,
				      					 ssCompData.header.sense_node);
		if(temp_u8){
			print_frame_u8("SS Init Data Ix2_sn = ",temp_u8 , 1,ssCompData.header.sense_node);
		}
		temp_i8 = array1dTo2d_i8(ssCompData.cx2_sn,ssCompData.header.sense_node,
				       				ssCompData.header.sense_node);
		if(temp_i8){
			print_frame_i8("SS Init Data Cx2_sn = ", temp_i8, 1,
			       ssCompData.header.sense_node);
		}
		temp_u16 =  array1dTo2d_u16(totCompData.ix_fm, totCompData.header.force_node, 1);
		if(temp_u16){
			print_frame_u16("TOT SS Init Data Ix_fm = ", temp_u16,
				totCompData.header.force_node, 1);
		}
		temp_short =  array1dTo2d_short(totCompData.cx_fm,
						    totCompData.header.force_node, 1);
		if(temp_short){
			print_frame_short("TOT SS Init Data Cx_fm = ",temp_short,
				totCompData.header.force_node, 1);
		}
		temp_u16 = array1dTo2d_u16(totCompData.ix_sn,totCompData.header.sense_node,
						totCompData.header.sense_node);
		if(temp_u16){
			print_frame_u16("TOT SS Init Data Ix_sn = ",temp_u16 , 1,
				totCompData.header.sense_node);
		}
		temp_short = array1dTo2d_short(totCompData.cx_sn,
						    totCompData.header.sense_node,
						    totCompData.header.sense_node);
		if(temp_short){
			print_frame_short("TOT SS Init Data Cx_sn = ",temp_short,1,
				totCompData.header.sense_node);
		}
		TS_LOG_INFO("%s SS IX CX testes finished!.................FAILED  fails_count = %d\n\n",
			 __func__, count_fail);

		ret =  ERROR_TEST_CHECK_FAIL | ERROR_PROD_TEST_DATA;
	}

ERROR_LIMITS:

	if (thresholds_min != NULL){
		kfree(thresholds_min);
		thresholds_min = NULL;
	}
	if (thresholds_max != NULL){
		kfree(thresholds_max);
		thresholds_max = NULL;
	}
	if (ssCompData.ix2_fm != NULL){
		kfree(ssCompData.ix2_fm);
		ssCompData.ix2_fm = NULL;
	}
	if (ssCompData.ix2_sn != NULL){
		kfree(ssCompData.ix2_sn);
		ssCompData.ix2_sn = NULL;
	}
	if (ssCompData.cx2_fm != NULL){
		kfree(ssCompData.cx2_fm);
		ssCompData.cx2_fm = NULL;
	}
	if (ssCompData.cx2_sn != NULL){
		kfree(ssCompData.cx2_sn);
		ssCompData.cx2_sn = NULL;
	}
	if (totCompData.ix_fm != NULL){
		kfree(totCompData.ix_fm);
		totCompData.ix_fm = NULL;
	}
	if (totCompData.ix_sn != NULL){
		kfree(totCompData.ix_sn);
		totCompData.ix_sn = NULL;
	}
	if (totCompData.cx_fm != NULL){
		kfree(totCompData.cx_fm);
		totCompData.cx_fm = NULL;
	}
	if (totCompData.cx_sn != NULL){
		kfree(totCompData.cx_sn);
		totCompData.cx_sn = NULL;
	}

	return ret;
}

/**
  * Perform all the tests selected in a TestTodo variable related to SS Init
  * data for LP mode (touch, keys etc..)
  * @param path_limits name of Production Limit file to load or
  * "NULL" if the limits data should be loaded by a .h file
  * @param stop_on_fail if 1, the test flow stops at the first data check
  * failure
  * otherwise it keeps going performing all the selected test
  * @param todo pointer to a TestToDo variable which select the test to do
  * @return OK if success or an error code which specify the type of error
  */
int production_test_ss_ix_cx_lp(char *path_limits, int stop_on_fail,
			     TestToDo *todo)
{
	int ret = 0;
	int count_fail = 0;

	int trows, tcolumns;
	int *thresholds_min = NULL;
	int *thresholds_max = NULL;

	SelfSenseData ssCompData;
	TotSelfSenseData totCompData;
	u8 ** temp_u8 = NULL;
	i8 ** temp_i8 = NULL;
	u16 ** temp_u16 = NULL;
	short ** temp_short  = NULL;

	TS_LOG_INFO( "%s SS LP IX CX testes are starting...\n", __func__);
	if(!path_limits || !todo) {
		TS_LOG_ERR( "%s NULL data \n", __func__);
		return ret | ERROR_PROD_TEST_DATA;
	}
	ret = readSelfSenseCompensationData(LOAD_CX_SS_TOUCH_IDLE, &ssCompData);
	/* read the SS LP compensation data */
	if (ret < 0) {
		TS_LOG_ERR("%s production_test_data: readSelfSenseCompensationData failed... ERROR %08X\n",
			 __func__, ERROR_PROD_TEST_DATA);
		return ret | ERROR_PROD_TEST_DATA;
	}

	ret = readTotSelfSenseCompensationData(LOAD_PANEL_CX_TOT_SS_TOUCH_IDLE,
					       &totCompData);
	/* read the TOT SS LP compensation data */
	if (ret < 0) {
		TS_LOG_ERR("%s production_test_data: readTotSelfSenseCompensationData failed... ERROR %08X\n",
			 __func__, ERROR_PROD_TEST_DATA);
		kfree(ssCompData.ix2_fm);
		ssCompData.ix2_fm = NULL;
		kfree(ssCompData.ix2_sn);
		ssCompData.ix2_sn = NULL;
		kfree(ssCompData.cx2_fm);
		ssCompData.cx2_fm = NULL;
		kfree(ssCompData.cx2_sn);
		ssCompData.cx2_sn = NULL;
		return ret | ERROR_PROD_TEST_DATA;
	}


	/* SS TOTAL FORCE IX */
	TS_LOG_INFO( "%s SS TOTAL IX LP FORCE TEST:\n", __func__);
	if (todo->SelfForceIxTotalLP == 1) {
		TS_LOG_INFO( "%s SS TOTAL IX LP FORCE MIN MAX TEST:\n", __func__);
		if (todo->SelfForceIxTotalLP == 1) {
			ret = parseProductionTestLimits(path_limits,
							&limit_file,
							SS_TOTAL_IX_LP_FORCE_MAP_MIN,
							&thresholds_min,
							&trows, &tcolumns);
						/* load the min thresholds */
			if (ret < 0 || (trows !=
					totCompData.header.force_node ||
					tcolumns != 1)) {
				TS_LOG_ERR( "%s production_test_data: parseProductionTestLimits SS_TOTAL_IX_LP_FORCE_MAP_MIN failed... ERROR %08X\n",
					 __func__, ERROR_PROD_TEST_DATA);
				ret |= ERROR_PROD_TEST_DATA;
				goto ERROR_LIMITS;
			}

			ret = parseProductionTestLimits(path_limits,
							&limit_file,
							SS_TOTAL_IX_LP_FORCE_MAP_MAX,
							&thresholds_max,
							&trows, &tcolumns);
						/* load the max thresholds */
			if (ret < 0 || (trows !=
					totCompData.header.force_node ||
					tcolumns != 1)) {
				TS_LOG_ERR( "%s production_test_data: parseProductionTestLimits SS_TOTAL_IX_LP_FORCE_MAP_MAX failed... ERROR %08X\n",
					 __func__, ERROR_PROD_TEST_DATA);
				ret |= ERROR_PROD_TEST_DATA;
				goto ERROR_LIMITS;
			}

			ret = checkLimitsMapTotalFromU(totCompData.ix_fm,
						       totCompData.header.
						       force_node, 1,
						       thresholds_min,
						       thresholds_max);
			/* check the values with thresholds */
			if (ret != OK) {
				TS_LOG_ERR("%s production_test_data: checkLimitsMap  SS TOTAL IX LP FORCE failed... ERROR COUNT = %d\n",
					 __func__, ret);
				TS_LOG_INFO("%s SS TOTAL IX LP FORCE MIN MAX TEST:.................FAIL\n\n",
					 __func__);
				count_fail += 1;
				if (stop_on_fail)
					goto ERROR;
			} else
				TS_LOG_INFO("%s SS TOTAL IX LP FORCE MIN MAX TEST:.................OK\n\n", __func__);

			kfree(thresholds_min);
			thresholds_min = NULL;
			kfree(thresholds_max);
			thresholds_max = NULL;
		} else
			TS_LOG_INFO("%s SS TOTAL IX LP FORCE MIN MAX TEST:.................SKIPPED\n",__func__);
	} else
		TS_LOG_INFO("%s SS TOTAL IX LP FORCE TEST:.................SKIPPED\n\n",__func__);

	/* SS TOTAL IX SENSE */
	TS_LOG_INFO( "%s SS TOTAL IX LP SENSE TEST:\n", __func__);
	if (todo->SelfSenseIxTotalLP == 1 ) {
		TS_LOG_INFO( "%s SS TOTAL IX LP SENSE MIN MAX TEST:\n", __func__);
		if (todo->SelfSenseIxTotalLP == 1) {
			ret = parseProductionTestLimits(path_limits,
							&limit_file,
							SS_TOTAL_IX_LP_SENSE_MAP_MIN,
							&thresholds_min,
							&trows, &tcolumns);
			/* load the min thresholds */
			if (ret < 0 || (trows != 1 || tcolumns !=
					totCompData.header.sense_node)) {
				TS_LOG_ERR("%s production_test_data: parseProductionTestLimits SS_TOTAL_IX_LP_SENSE_MAP_MIN failed... ERROR %08X\n",
					 __func__, ERROR_PROD_TEST_DATA);
				ret |= ERROR_PROD_TEST_DATA;
				goto ERROR_LIMITS;
			}

			ret = parseProductionTestLimits(path_limits,
							&limit_file,
							SS_TOTAL_IX_LP_SENSE_MAP_MAX,
							&thresholds_max,
							&trows, &tcolumns);
			/* load the max thresholds */
			if (ret < 0 || (trows != 1 || tcolumns !=
					totCompData.header.sense_node)) {
				TS_LOG_ERR( "%s production_test_data: parseProductionTestLimits SS_TOTAL_IX_LP_SENSE_MAP_MAX failed... ERROR %08X\n",
					 __func__, ERROR_PROD_TEST_DATA);
				ret |= ERROR_PROD_TEST_DATA;
				goto ERROR_LIMITS;
			}

			ret = checkLimitsMapTotalFromU(totCompData.ix_sn, 1,
						       totCompData.header.
						       sense_node,
						       thresholds_min,
						       thresholds_max);
			/* check the values with thresholds */
			if (ret != OK) {
				TS_LOG_ERR( "%s production_test_data: checkLimitsMap SS TOTAL IX LP SENSE failed... ERROR COUNT = %d\n", __func__, ret);
				TS_LOG_INFO("%s SS TOTAL IX LP SENSE MIN MAX TEST:.................FAIL\n\n", __func__);
				count_fail += 1;
				if (stop_on_fail)
					goto ERROR;
			} else
				TS_LOG_INFO( "%s SS TOTAL IX LP SENSE MIN MAX TEST:.................OK\n\n",__func__);

			kfree(thresholds_min);
			thresholds_min = NULL;
			kfree(thresholds_max);
			thresholds_max = NULL;
		} else
			TS_LOG_INFO("%s SS TOTAL IX LP SENSE MIN MAX TEST:.................SKIPPED\n",__func__);

	} else
		TS_LOG_INFO("%s SS TOTAL IX LP SENSE TEST:.................SKIPPED\n",__func__);

ERROR:
	TS_LOG_INFO( "%s\n", __func__);
	if (count_fail == 0) {
		kfree(ssCompData.ix2_fm);
		ssCompData.ix2_fm = NULL;
		kfree(ssCompData.ix2_sn);
		ssCompData.ix2_sn = NULL;
		kfree(ssCompData.cx2_fm);
		ssCompData.cx2_fm = NULL;
		kfree(ssCompData.cx2_sn);
		ssCompData.cx2_sn = NULL;
		kfree(totCompData.ix_fm);
		totCompData.ix_fm = NULL;
		kfree(totCompData.ix_sn);
		totCompData.ix_sn = NULL;
		kfree(totCompData.cx_fm);
		totCompData.cx_fm = NULL;
		kfree(totCompData.cx_sn);
		totCompData.cx_sn = NULL;
		TS_LOG_INFO("%s SS LP IX CX  testes finished!.................OK\n\n",__func__);
		return OK;
	} else {
	/* print all kind of data in just one row for readability reason */
		temp_u8 =  array1dTo2d_u8( ssCompData.ix2_fm,
				       ssCompData.header.force_node, 1);
		if(temp_u8){
			print_frame_u8("SS LP Init Data Ix2_fm = ",temp_u8,
			       ssCompData.header.force_node, 1);
		}
		temp_i8 =  array1dTo2d_i8( ssCompData.cx2_fm,
				       ssCompData.header.force_node, 1);
		if(temp_i8){
			print_frame_i8("SS LP Init Data Cx2_fm = ",temp_i8,
			       ssCompData.header.force_node, 1);
		}
		temp_u8 =  array1dTo2d_u8( ssCompData.ix2_sn,
				       ssCompData.header.sense_node,
				       ssCompData.header.sense_node);
		if(temp_u8){
			print_frame_u8("SS LP Init Data Ix2_sn = ",temp_u8, 1,
			       ssCompData.header.sense_node);
		}
		temp_i8 = array1dTo2d_i8( ssCompData.cx2_sn,
				       ssCompData.header.sense_node,
				       ssCompData.header.sense_node);
		if(temp_i8){
			print_frame_i8("SS LP Init Data Cx2_sn = ",temp_i8 , 1,
			       ssCompData.header.sense_node);
		}
		temp_u16 =  array1dTo2d_u16(
					totCompData.ix_fm,
					totCompData.header.force_node, 1);
		if(temp_u16){
			print_frame_u16("TOT SS LP Init Data Ix_fm = ",temp_u16 ,
				totCompData.header.force_node, 1);
		}
		temp_short =  array1dTo2d_short(totCompData.cx_fm,
						    totCompData.header.force_node, 1);
		if(temp_short){
			print_frame_short("TOT SS LP Init Data Cx_fm = ",temp_short ,
				  totCompData.header.force_node, 1);
		}
		temp_u16 = array1dTo2d_u16(totCompData.ix_sn,totCompData.header.sense_node,
						totCompData.header.sense_node);
		if(temp_u16){
			print_frame_u16("TOT SS LP Init Data Ix_sn = ", temp_u16, 1,
				totCompData.header.sense_node);
		}
		temp_short =  array1dTo2d_short(totCompData.cx_sn,
						    totCompData.header.sense_node,
						    totCompData.header.sense_node);
		if(temp_short){
			print_frame_short("TOT SS LP Init Data Cx_sn = ",temp_short,1,
					totCompData.header.sense_node);
		}
		TS_LOG_INFO("%s SS LP IX CX testes finished!.................FAILED  fails_count = %d\n\n", __func__, count_fail);

		ret =  ERROR_TEST_CHECK_FAIL | ERROR_PROD_TEST_DATA;
	}

ERROR_LIMITS:

	if (thresholds_min != NULL){
		kfree(thresholds_min);
		thresholds_min = NULL;
	}
	if (thresholds_max != NULL){
		kfree(thresholds_max);
		thresholds_max = NULL;
	}
	if (ssCompData.ix2_fm != NULL){
		kfree(ssCompData.ix2_fm);
		ssCompData.ix2_fm = NULL;
	}
	if (ssCompData.ix2_sn != NULL){
		kfree(ssCompData.ix2_sn);
		ssCompData.ix2_sn = NULL;
	}
	if (ssCompData.cx2_fm != NULL){
		kfree(ssCompData.cx2_fm);
		ssCompData.ix2_sn = NULL;
	}
	if (ssCompData.cx2_sn != NULL){
		kfree(ssCompData.cx2_sn);
		ssCompData.cx2_sn = NULL;
	}
	if (totCompData.ix_fm != NULL){
		kfree(totCompData.ix_fm);
		totCompData.ix_fm = NULL;
	}
	if (totCompData.ix_sn != NULL){
		kfree(totCompData.ix_sn);
		totCompData.ix_sn = NULL;
	}
	if (totCompData.cx_fm != NULL){
		kfree(totCompData.cx_fm);
		totCompData.cx_fm = NULL;
	}
	if (totCompData.cx_sn != NULL){
		kfree(totCompData.cx_sn);
		totCompData.cx_sn = NULL;
	}
	return ret;
}

/**
  * Perform a complete Data Test check of the IC
  * @param path_limits name of Production Limit file to load or
  * "NULL" if the limits data should be loaded by a .h file
  * @param stop_on_fail if 1, the test flow stops at the first data check
  * failure otherwise it keeps going performing all the selected test
  * @param todo pointer to a TestToDo variable which select the test to do
  * @return OK if success or an error code which specify the type of error
  */
int production_test_data(char *path_limits, int stop_on_fail, TestToDo *todo,
		struct ts_rawdata_info *info, TestResult *result)
{
	int res = OK;
	int ret = 0;

	if (todo == NULL || !path_limits || !info || !result) {
		TS_LOG_ERR("%s production_test_data: No TestToDo specified!! ERROR = %08X\n",
			 __func__, (ERROR_OP_NOT_ALLOW | ERROR_PROD_TEST_DATA));
		return ERROR_OP_NOT_ALLOW | ERROR_PROD_TEST_DATA;
	}

	TS_LOG_INFO( "%s DATA Production test is starting...\n", __func__);

	ret = production_test_ms_raw(path_limits, stop_on_fail, todo, info, result);
	res |= ret;
	if (ret < 0) {
		TS_LOG_ERR("%s production_test_data: production_test_ms_raw failed... ERROR = %08X\n", __func__, ret);
		if (stop_on_fail == 1)
			goto END;
	}

	ret = production_test_ms_cx(path_limits, stop_on_fail, todo, info, result);
	res |= ret;
	if (ret < 0) {
		TS_LOG_ERR("%s production_test_data: production_test_ms_cx failed... ERROR = %08X\n",__func__, ret);
		if (stop_on_fail == 1)
			goto END;
	}

	ret = production_test_ss_raw(path_limits, stop_on_fail, todo, info, result);
	res |= ret;
	if (ret < 0) {
		TS_LOG_ERR("%s production_test_data: production_test_ss_raw failed... ERROR = %08X\n",__func__, ret);
		if (stop_on_fail == 1)
			goto END;
	}

	ret = production_test_ss_ix_cx(path_limits, stop_on_fail, todo);
	res |= ret;
	if (ret < 0) {
		TS_LOG_ERR("%s production_test_data: production_test_ss_ix_cx failed... ERROR = %08X\n", __func__, ret);
		if (stop_on_fail == 1)
			goto END;
	}

END:
	freeLimitsFile(&limit_file);	/* /< release the limit file loaded
					 * during the test */
	if (res < OK)
		TS_LOG_INFO( "%s DATA Production test failed!\n", __func__);
	else
		TS_LOG_INFO( "%s DATA Production test finished!\n", __func__);
	return res;
}


/**
  * Retrieve the actual Test Limit data from the system (bin file or header
  *file)
  * @param path name of Production Test Limit file to load or "NULL" if the
  *limits data should be loaded by a .h file
  * @param file pointer to the LimitFile struct which will contains the limits
  *data
  * @return OK if success or an error code which specify the type of error
  *encountered
  */
int getLimitsFile(char *path, LimitFile *file)
{
	const struct firmware *fw = NULL;
//	struct device *dev = NULL;
	int fd = -1;
	struct fts_ts_info *fts_info;

	TS_LOG_INFO( "%s Get Limits File starting... %s\n", __func__, path);

	if (file->data != NULL) {/* to avoid memory leak on consecutive call of
				 * the function with the same pointer */
		TS_LOG_INFO("%s Pointer to Limits Data already contains something... freeing its content!\n", __func__);
		kfree(file->data);
		file->data = NULL;
		file->size = 0;
	}

	strlcpy(file->name, path, MAX_LIMIT_FILE_NAME);
	if (strncmp(path, "NULL", 4) == 0) {
		TS_LOG_ERR("%s limit file path NULL... ERROR %08X\n", __func__,
			 ERROR_FILE_NOT_FOUND);
		return ERROR_FILE_NOT_FOUND;
	} else {
		fts_info = fts_get_info();
		if (fts_info != NULL && fts_info->i2c_cmd_dev != NULL) {
			TS_LOG_INFO( "%s Loading Limits File from .csv!\n", __func__);
			fd = request_firmware(&fw, path, fts_info->i2c_cmd_dev);
			if (fd == 0) {
				TS_LOG_INFO( "%s Start to copy %s...\n", __func__, path);
				file->size = fw->size;
				file->data = (char *)kmalloc((file->size) * sizeof(char),GFP_KERNEL);
				if (file->data != NULL) {
					memcpy(file->data, (char *)fw->data,file->size);
					TS_LOG_INFO("%s Limit file Size = %d\n",__func__,file->size);
					release_firmware(fw);
					return OK;
				} else {
					TS_LOG_ERR("%s Error while allocating data... ERROR %08X\n",
						 __func__, ERROR_ALLOC);
					release_firmware(fw);
					return ERROR_ALLOC;
				}
			} else {
				TS_LOG_ERR("%s Request the file %s failed... ERROR %08X\n",
					 __func__, path, ERROR_FILE_NOT_FOUND);
				return ERROR_FILE_NOT_FOUND;
			}
		} else {
			TS_LOG_ERR("%s Error while getting the device ERROR %08X\n",
				 __func__,ERROR_FILE_READ);
			return ERROR_FILE_READ;
		}
	}
}

/**
  * Reset and release the memory which store a Production Limit File previously
  *loaded
  * @param file pointer to the LimitFile struct to free
  * @return OK if success or an error code which specify the type of error
  *encountered
  */

int freeLimitsFile(LimitFile *file)
{
	TS_LOG_INFO( "%s Freeing Limit File ...\n", __func__);
	if (file != NULL) {
		if (file->data != NULL) {
			kfree(file->data);
			file->data = NULL;
		} else {
			TS_LOG_INFO( "%s Limit File was already freed!\n", __func__);
        }
		file->size = 0;
		strlcpy(file->name, " ", MAX_LIMIT_FILE_NAME);
		return OK;
	} else {
		TS_LOG_ERR("%s Passed a NULL argument! ERROR %08X\n", __func__,ERROR_OP_NOT_ALLOW);
		return ERROR_OP_NOT_ALLOW;
	}
}

/**
  * Reset and release the memory which store the current Limit File previously
  *loaded
  * @return OK if success or an error code which specify the type of error
  *encountered
  */

int freeCurrentLimitsFile(void)
{
	return freeLimitsFile(&limit_file);
}


/**
  * Parse the raw data read from a Production test limit file in order to find
  *the specified information
  * If no limits file data are passed, the function loads and stores the limit
  *file from the system
  * @param path name of Production Test Limit file to load or "NULL" if the
  *limits data should be loaded by a .h file
  * @param file pointer to LimitFile struct that should be parsed or NULL if the
  *limit file in the system should be loaded and then parsed
  * @param label string which identify a particular set of data in the file that
  *want to be loaded
  * @param data pointer to the pointer which will contains the specified limits
  *data as 1 dimension matrix with data arranged row after row
  * @param row pointer to a int variable which will contain the number of row of
  *data
  * @param column pointer to a int variable which will contain the number of
  *column of data
  * @return OK if success or an error code which specify the type of error
  */
int parseProductionTestLimits(char *path, LimitFile *file, char *label,
			      int **data, int *row, int *column)
{
	int find = 0;
	int i = 0;
	int j = 0;
	int z = 0;
	int n = 0;
	int size = 0;
	int pointer = 0;
	int ret = OK;
	char *token = NULL;
	char *line2 = NULL;
	char line[800] = {0};
	char *buf = NULL;
	char *data_file = NULL;

	if (file == NULL || strcmp(path, file->name) != 0 || file->size == 0) {
		TS_LOG_INFO("%s No limit File data passed... try to get them from the system!\n", __func__);
		ret = getLimitsFile(path/*LIMITS_FILE*/, &limit_file);
		if (ret < OK) {
			TS_LOG_ERR("%s parseProductionTestLimits: ERROR %08X\n",
				 __func__, ERROR_FILE_NOT_FOUND);
			return ERROR_FILE_NOT_FOUND;
		}
		size = limit_file.size;
		data_file = limit_file.data;
	} else {
		TS_LOG_INFO( "%s Limit File data passed as arguments!\n", __func__);
		size = file->size;
		data_file = file->data;
	}

	TS_LOG_INFO( "%s The size of the limits file is %d bytes...\n", __func__,size);

	while (find == 0) {
		/* start to look for the wanted label */
		if (readLine(&data_file[pointer], line, size - pointer, &n) < 0) {
			find = -1;
			break;
		}
		pointer += n;
		/* each header row start with *  ex.label,n_row,n_colum */
		if (line[0] == '*') {
			line2 = kstrdup(line, GFP_KERNEL);
			if (line2 == NULL) {
				TS_LOG_ERR("%s parseProductionTestLimits: kstrdup ERROR %08X\n",
					 __func__, ERROR_ALLOC);
				ret = ERROR_ALLOC;
				goto END;
			}
			buf = line2;
			line2 += 1;
			token = strsep(&line2, ",");
			/* if the row is the wanted one i retrieve rows and columns info  */
			if (strcmp(token, label) == 0) {
				find = 1;
				token = strsep(&line2, ",");
				if (token != NULL) {
					if (sscanf(token, "%d", row) == 1) {
						TS_LOG_INFO( "%s Row = %d\n",__func__, *row);
					} else {
						TS_LOG_INFO( "%s ERROR while reading the row value! ERROR %08X\n",
							__func__, ERROR_FILE_PARSE);
						ret = ERROR_FILE_PARSE;
						goto END;
					}
				} else {
					TS_LOG_ERR("%s parseProductionTestLimits 1: ERROR %08X\n",
						 __func__, ERROR_FILE_PARSE);
					ret = ERROR_FILE_PARSE;
					goto END;
				}
				token = strsep(&line2, ",");
				if (token != NULL) {
					if (sscanf(token, "%d", column) == 1) {
						TS_LOG_INFO( "%s Column = %d\n",__func__, *column);
					} else {
						TS_LOG_INFO( "%s ERROR while reading the column value! ERROR %08X\n",
							__func__, ERROR_FILE_PARSE);
						ret = ERROR_FILE_PARSE;
						goto END;
					}
				} else {
					TS_LOG_ERR("%s parseProductionTestLimits 2: ERROR %08X\n",
						 __func__, ERROR_FILE_PARSE);
					ret = ERROR_FILE_PARSE;
					goto END;
				}

				kfree(buf);
				buf = NULL;
				*data = (int *)kmalloc(((*row) * (*column)) * sizeof(int), GFP_KERNEL);
			    /* allocate the memory for containing the data */
				j = 0;
				if (*data == NULL) {
					TS_LOG_ERR("%s parseProductionTestLimits: ERROR %08X\n",
						 __func__, ERROR_ALLOC);
					ret = ERROR_ALLOC;
					goto END;
				}
				/* start to read the data */
				for (i = 0; i < *row; i++) {
					if (readLine(&data_file[pointer], line,size - pointer, &n) < 0) {
						TS_LOG_ERR("%s parseProductionTestLimits : ERROR %08X\n",
							 __func__, ERROR_FILE_READ);
						ret = ERROR_FILE_READ;
						goto END;
					}
					pointer += n;
					line2 = kstrdup(line, GFP_KERNEL);
					if (line2 == NULL) {
						TS_LOG_ERR("%s parseProductionTestLimits: kstrdup ERROR %08X\n",
							 __func__, ERROR_ALLOC);
						ret = ERROR_ALLOC;
						goto END;
					}
					buf = line2;
					token = strsep(&line2, ",");
					for (z = 0; (z < *column) && (token != NULL);z++) {
						if (sscanf(token, "%d",((*data) + j)) == 1) {
							j++;
							token = strsep(&line2, ",");
						}
					}
					kfree(buf);
					buf = NULL;
				}
			/* check that all the data are read */
				if (j == ((*row) * (*column))) {
					TS_LOG_INFO( "%s READ DONE!\n", __func__);
					ret = OK;
					goto END;
				}
				TS_LOG_ERR("%s parseProductionTestLimits 3: ERROR %08X\n",
					 __func__, ERROR_FILE_PARSE);
				ret = ERROR_FILE_PARSE;
				goto END;
			}
			kfree(buf);
			buf = NULL;
		}
	}
	TS_LOG_ERR("%s parseProductionTestLimits: ERROR %08X\n", __func__, ERROR_LABEL_NOT_FOUND);
	ret = ERROR_LABEL_NOT_FOUND;
END:
	if (buf != NULL)
		kfree(buf);
	return ret;
}


/**
  * Read one line of a text file passed as array of byte and terminate it with a
  *termination character '\0'
  * @param data text file as array of bytes
  * @param line pointer to an array of char that will contain the line read
  * @param size size of data
  * @param n pointer to a int variable which will contain the number of
  *characters of the line
  * @return OK if success or an error code which specify the type of error
  */
int readLine(char *data, char *line, int size, int *n)
{
	int i = 0;

	if (size < 1 || !data || !line) {
		TS_LOG_ERR("%s data null\n", __func__);
		return ERROR_OP_NOT_ALLOW;
	}

	while (data[i] != '\n' && i < size) {
		line[i] = data[i];
		i++;
	}
	*n = i + 1;
	line[i] = '\0';

	return OK;
}

int st_get_rawdata_aftertest(struct ts_rawdata_info *info,u8 signature)
{
	int ret = 0;
	TestResult *TestRes = NULL;
	char file_path[FTS_LIMIT_FILE_NAME_MAX_LEN] = {0};
	char fw_version_info[FTS_LIMIT_FILE_NAME_MAX_LEN] = {0};
	int sensor_len = 0;
	int force_len = 0;
	struct fts_ts_info *fts_info = fts_get_info();

	TS_LOG_INFO("sensor_len is %d, force_len is %d\n", sensor_len, force_len);
	if(!info) {
		TS_LOG_INFO("info NULL \n");
		return -ENOMEM;
	}
	sensor_len = getSenseLen();
	if(sensor_len <= 0) {
		//sensor_len = 32; //default value
		TS_LOG_INFO("can not get sensor len\n");
		return -ENOMEM;
	}
	force_len = getForceLen();
	if(force_len <= 0) {
		//force_len = 16; //default value
		TS_LOG_INFO("can not get force len\n");
		return -ENOMEM;
	}

	info->buff[0] = sensor_len;
	info->buff[1] = force_len;
	info->used_size = 2;

	info->hybrid_buff[0] = sensor_len;
	info->hybrid_buff[1] = force_len;
	info->hybrid_buff_used_size = 2;

	TS_LOG_INFO("%s:sensor_len is %d, force_len is %d\n", __func__, sensor_len, force_len);
	TestRes = (TestResult *)kzalloc(sizeof(TestResult), GFP_KERNEL);
	if (!TestRes) {
		TS_LOG_ERR("%s: out of mem\n", __func__);
		return -ENOMEM;
	}

	snprintf(file_path, sizeof(file_path), "ts/%s_st_%s_limit.csv",
			fts_info->chip_data->ts_platform_data->product_name,
			fts_info->project_id);
	TS_LOG_INFO("%s:limit file name=%s\n", __func__, file_path);

	ret = production_test_main(file_path, 0, NO_INIT, &tests,MP_FLAG_BOOT,info,TestRes);

	if (TestRes->I2c_Check) {
		strncat(info->result, "0P", strlen("0P"));
	} else {
		strncat(info->result, "0F", strlen("0F"));
	}

	if (TestRes->MutualRawRes){
		strncat(info->result, "-1P", strlen("-1P"));
	} else {
		strncat(info->result, "-1F", strlen("-1F"));
	}

	if (TestRes->MutualRawResGap){
		strncat(info->result, "-2P", strlen("-2P"));
	} else {
		strncat(info->result, "-2F", strlen("-2F"));
	}

	if (TestRes->MutualStrengthRes){
		strncat(info->result, "-3P", strlen("-3P"));
	} else {
		strncat(info->result, "-3F", strlen("-3F"));
	}

	if (TestRes->ITO_Test_Res) {
		strncat(info->result, "-5P", strlen("-5P"));
	} else {
		strncat(info->result, "-5F", strlen("-5F"));
	}
	if (TestRes->SelfSenseRawRes && TestRes->SelfForceRawRes) {
		strncat(info->result, "-6P", strlen("-6P"));
	} else {
		strncat(info->result, "-6F", strlen("-6F"));
	}
	if (TestRes->SelfSenseStrengthData && TestRes->SelfForceStrengthData) {
		strncat(info->result, "-9P", strlen("-9P"));
	} else {
		strncat(info->result, "-9F", strlen("-9F"));
	}
//	if (TestRes->SelfSenseData) {
//		strncat(info->result, "-10P", strlen("-10P"));
//	} else {
//		strncat(info->result, "-10F", strlen("-10F"));
//	}
	strncat(info->result, TestRes->mutal_raw_res_buf, ST_NP_TEST_RES_BUF_LEN);
	strncat(info->result, TestRes->mutal_noise_res_buf, ST_NP_TEST_RES_BUF_LEN);
	strncat(info->result, TestRes->mutal_cal_res_buf, ST_NP_TEST_RES_BUF_LEN);
	strncat(info->result, ";", strlen(";"));
	if (0 == strlen(info->result) || strstr(info->result, "F")) {
		strncat(info->result, "panel_reason-", strlen("panel_reason-"));
	}
	strncat(info->result, "st-", strlen("st-"));
	strncat(info->result, fts_info->project_id, ST_PROJECT_ID_LEN);
	snprintf(fw_version_info, sizeof(fw_version_info) - 1,
		"-%x.%x", systemInfo.u16_fwVer, systemInfo.u16_cfgVer);
	strncat(info->result, fw_version_info, strlen(fw_version_info));

	kfree(TestRes);
	TestRes =NULL;
	TS_LOG_INFO("%s:end\n",__func__);
	return ret;
}
