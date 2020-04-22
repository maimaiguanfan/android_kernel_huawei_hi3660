/*
  *
  **************************************************************************
  **                        STMicroelectronics                            **
  **************************************************************************
  **                        marco.cali@st.com                             **
  **************************************************************************
  *                                                                        *
  *                      FTS API for Flashing the IC                       *
  *                                                                        *
  **************************************************************************
  **************************************************************************
  *
  */


/*!
  * \file ftsFlash.c
  * \brief Contains all the functions to handle the FW update process
  */

#include "ftsCore.h"
#include "ftsCompensation.h"
#include "ftsError.h"
#include "ftsFlash.h"
#include "ftsFrame.h"
#include "ftsIO.h"
#include "ftsSoftware.h"
#include "ftsTest.h"
#include "ftsTime.h"
#include "ftsTool.h"
#include "../fts.h"	/* needed for including the define FW_H_FILE */


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
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/firmware.h>


#define ST_FLASH_DMA_LEN 12
extern SysInfo systemInfo;	/* /< forward declaration of the global variable
				  *  of containing System Info Data */


/**
  * Retrieve the actual FW data from the system (bin file or header file)
  * @param pathToFile name of FW file to load or "NULL" if the FW data should be
  *loaded by a .h file
  * @param data pointer to the pointer which will contains the FW data
  * @param size pointer to a variable which will contain the size of the loaded
  *data
  * @return OK if success or an error code which specify the type of error
  */
int getFWdata(const char *pathToFile, u8 **data, int *size)
{
	const struct firmware *fw = NULL;
//	struct device *dev = NULL;
	int res = 0, from = 0;
	char *path = (char *)pathToFile;
	struct fts_ts_info *fts_info;

	TS_LOG_ERR( "%s getFWdata starting ...\n", __func__);

	if(!pathToFile || !size){
		TS_LOG_ERR( "%s:input error!\n", __func__);
		return ERROR_OP_NOT_ALLOW;
	}

	/* keep the switch case because if the argument passed is null but
	  * the option from .h is not set we still try to load from bin */
	switch (from) {
	default:
		TS_LOG_ERR( "%s Read FW from BIN file %s !\n", __func__, path);
		fts_info = fts_get_info();

		if (fts_info != NULL && fts_info->i2c_cmd_dev != NULL) {
			res = request_firmware(&fw, path, fts_info->i2c_cmd_dev);
			if (res == 0) {
				*size = fw->size;
				*data = (u8 *)kmalloc((*size) * sizeof(u8), GFP_KERNEL);
				if (*data == NULL) {
					TS_LOG_ERR("%s getFWdata: Impossible to allocate memory! ERROR %08X\n",
										 __func__, ERROR_ALLOC);
					release_firmware(fw);
					return ERROR_ALLOC;
				}
				memcpy(*data, (u8 *)fw->data, (*size));
				release_firmware(fw);
			} else {
				TS_LOG_ERR( "%s getFWdata: No File found! ERROR %08X\n",
								 __func__, ERROR_FILE_NOT_FOUND);
				return ERROR_FILE_NOT_FOUND;
			}
		} else {
			TS_LOG_ERR( "%s getFWdata: No device found! ERROR %08X\n",
				 			__func__,ERROR_OP_NOT_ALLOW);
			return ERROR_OP_NOT_ALLOW;
		}
	}

	TS_LOG_ERR( "%s getFWdata Finished!\n", __func__);
	return OK;
}


/**
  * Perform all the steps to read the FW that should be burnt in the IC from
  * the system and parse it in order to fill a Firmware struct with the relevant
  * info
  * @param path name of FW file to load or "NULL" if the FW data should be
  *loaded by a .h file
  * @param fw pointer to a Firmware variable which will contains the FW data and
  *info
  * @param keep_cx if 1, the CX area will be loaded otherwise will be skipped
  * @return OK if success or an error code which specify the type of error
  */
int readFwFile(const char *path, Firmware *fw, int keep_cx)
{
	int res = 0;
	int orig_size = 0;
	u8 *orig_data = NULL;

	res = getFWdata(path, &orig_data, &orig_size);
	if (res < OK) {
		TS_LOG_ERR(
			 "%s readFwFile: impossible retrieve FW... ERROR %08X\n",
			 __func__,
			 ERROR_MEMH_READ);
		return res | ERROR_MEMH_READ;
	}
	res = parseBinFile(orig_data, orig_size, fw, keep_cx);
	if (res < OK) {
		TS_LOG_ERR( "%s readFwFile: impossible parse ERROR %08X\n", __func__,
			 ERROR_MEMH_READ);
		return res | ERROR_MEMH_READ;
	}

	return OK;
}

/**
  * Perform all the steps necessary to burn the FW into the IC
  * @param path name of FW file to load or "NULL" if the FW data should be
  *loaded by a .h file
  * @param force if 1, the flashing procedure will be forced and executed
  *regardless the additional info, otherwise the FW in the file will be burnt
  *only if it is newer than the one running in the IC
  * @param keep_cx if 1, the CX area will be loaded and burnt otherwise will be
  *skipped and the area will be untouched
  * @return OK if success or an error code which specify the type of error
  */
int flashProcedure(const char *path, int force, int keep_cx)
{
	Firmware fw;
	int res = 0;

	fw.data = NULL;
	TS_LOG_DEBUG("%s Reading Fw file...\n", __func__);
	res = readFwFile(path, &fw, keep_cx);
	if (res < OK) {
		TS_LOG_ERR("%s flashProcedure: ERROR %08X\n", __func__,
						(res | ERROR_FLASH_PROCEDURE));
		kfree(fw.data);
		return res | ERROR_FLASH_PROCEDURE;
	}
	TS_LOG_DEBUG("%s Fw file read COMPLETED!\n", __func__);

	TS_LOG_INFO("%s Starting flashing procedure...\n", __func__);
	res = flash_burn(fw, force, keep_cx);
	if (res < OK && res != (ERROR_FW_NO_UPDATE | ERROR_FLASH_BURN_FAILED)) {
		TS_LOG_ERR("%s flashProcedure: ERROR %08X\n", __func__,
			 ERROR_FLASH_PROCEDURE);
		kfree(fw.data);
		return res | ERROR_FLASH_PROCEDURE;
	}
	TS_LOG_INFO("%s flashing procedure Finished!\n", __func__);
	kfree(fw.data);

	return OK;
}

/**
  * Poll the Flash Status Registers after the execution of a command to check
  * if the Flash becomes ready within a timeout
  * @param type register to check according to the previous command sent
  * @return OK if success or an error code which specify the type of error
  */
int wait_for_flash_ready(u8 type)
{
	u8 cmd[5] = { FTS_CMD_HW_REG_R, 0x20, 0x00, 0x00, type };
	u8 readData[2] = { 0 };
	int i = 0, res = -1;

	TS_LOG_INFO( "%s Waiting for flash ready ...\n", __func__);
	for (i = 0; i < FLASH_RETRY_COUNT && res != 0; i++) {
		res = fts_writeRead(cmd, ARRAY_SIZE(cmd), readData, 2);
		if (res < OK)
			TS_LOG_ERR( "%s wait_for_flash_ready: ERROR %08X\n",
				 __func__, ERROR_BUS_W);
		else {
			res = readData[0] & 0x80;
			TS_LOG_INFO( "%s flash status = %d\n", __func__, res);
		}
		msleep(FLASH_WAIT_BEFORE_RETRY);
	}

	if (i == FLASH_RETRY_COUNT && res != 0) {
		TS_LOG_ERR( "%s Wait for flash TIMEOUT! ERROR %08X\n", __func__,ERROR_TIMEOUT);
		return ERROR_TIMEOUT;
	}

	TS_LOG_INFO( "%s Flash READY!\n", __func__);
	return OK;
}


/**
  * Put the M3 in hold
  * @return OK if success or an error code which specify the type of error
  */
int hold_m3(void)
{
	int ret = 0;
	u8 cmd[1] = { 0x01 };

	TS_LOG_INFO( "%s Command m3 hold...\n", __func__);
	ret = fts_writeU8UX(FTS_CMD_HW_REG_W, ADDR_SIZE_HW_REG,
			    ADDR_SYSTEM_RESET, cmd, 1);
	if (ret < OK) {
		TS_LOG_ERR( "%s hold_m3: ERROR %08X\n", __func__, ret);
		return ret;
	}
	TS_LOG_INFO( "%s Hold M3 DONE!\n", __func__);

	return OK;
}



/**
  * Parse the raw data read from a FW file in order to fill properly the fields
  * of a Firmware variable
  * @param fw_data raw FW data loaded from system
  * @param fw_size size of fw_data
  * @param fwData pointer to a Firmware variable which will contain the
  *processed data
  * @param keep_cx if 1, the CX area will be loaded and burnt otherwise will be
  * skipped and the area will be untouched
  * @return OK if success or an error code which specify the type of error
  */
int parseBinFile(u8 *fw_data, int fw_size, Firmware *fwData, int keep_cx)
{
	int dimension = 0, index = 0;
	u32 temp = 0;
	int res = 0, i = 0;

	/* the file should contain at least the header plus the content_crc */
	if ((fw_size < FW_HEADER_SIZE + FW_BYTES_ALLIGN) || !fw_data || !fwData) {
		TS_LOG_ERR("%s parseBinFile: Read only %d instead of %d... ERROR %08X\n",
			 __func__, fw_size, FW_HEADER_SIZE + FW_BYTES_ALLIGN, ERROR_FILE_PARSE);
		res = ERROR_FILE_PARSE;
		goto END;
	} else {
		/* start parsing of bytes */
		u8ToU32(&fw_data[index], &temp);
		if (temp != FW_HEADER_SIGNATURE) {
			TS_LOG_ERR("%s : Wrong Signature %08X ... ERROR %08X\n",
				 __func__, temp, ERROR_FILE_PARSE);
			res = ERROR_FILE_PARSE;
			goto END;
		}
		TS_LOG_INFO( "%s : Fw Signature OK!\n", __func__);
		index += FW_BYTES_ALLIGN;
		u8ToU32(&fw_data[index], &temp);
		if (temp != FW_FTB_VER) {
			TS_LOG_ERR("%s : Wrong ftb_version %08X ... ERROR %08X\n",
				 __func__, temp, ERROR_FILE_PARSE);
			res = ERROR_FILE_PARSE;
			goto END;
		}
		TS_LOG_INFO( "%s : ftb_version OK!\n", __func__);
		index += FW_BYTES_ALLIGN;
		if (fw_data[index] != DCHIP_ID_0 || fw_data[index + 1] !=
		    DCHIP_ID_1) {
			TS_LOG_ERR( "%s : Wrong target %02X != %02X  %02X != %02X ... ERROR %08X\n",
				 __func__, fw_data[index], DCHIP_ID_0, fw_data[index + 1],DCHIP_ID_1, ERROR_FILE_PARSE);
			res = ERROR_FILE_PARSE;
			goto END;
		}
		index += FW_BYTES_ALLIGN;
		u8ToU32(&fw_data[index], &temp);
		TS_LOG_INFO( "%s : FILE SVN REV = %08X\n", __func__, temp);

		index += FW_BYTES_ALLIGN;
		u8ToU32(&fw_data[index], &temp);
		fwData->fw_ver = temp;
		TS_LOG_INFO( "%s : FILE Fw Version = %04X\n", __func__, fwData->fw_ver);

		index += FW_BYTES_ALLIGN;
		u8ToU32(&fw_data[index], &temp);
		fwData->config_id = temp;
		TS_LOG_INFO( "%s : FILE Config Project ID = %08X\n", __func__, temp);

		index += FW_BYTES_ALLIGN;
		u8ToU32(&fw_data[index], &temp);
		TS_LOG_INFO( "%s : FILE Config Version = %08X\n",__func__, temp);

		index += FW_BYTES_ALLIGN * 2;	/* skip reserved data */

		index += FW_BYTES_ALLIGN;
		TS_LOG_INFO( "%s : File External Release =  ", __func__);
		for (i = 0; i < EXTERNAL_RELEASE_INFO_SIZE; i++) {
			fwData->externalRelease[i] = fw_data[index++];
			TS_LOG_ERR( "%02X ", fwData->externalRelease[i]);
		}

		/* index+=FW_BYTES_ALLIGN; */
		u8ToU32(&fw_data[index], &temp);
		fwData->sec0_size = temp;
		TS_LOG_INFO( "%s :  sec0_size = %08X (%d bytes)\n",
			 __func__, fwData->sec0_size, fwData->sec0_size);

		index += FW_BYTES_ALLIGN;
		u8ToU32(&fw_data[index], &temp);
		fwData->sec1_size = temp;
		TS_LOG_INFO( "%s :  sec1_size = %08X (%d bytes)\n",
			 __func__, fwData->sec1_size, fwData->sec1_size);

		index += FW_BYTES_ALLIGN;
		u8ToU32(&fw_data[index], &temp);
		fwData->sec2_size = temp;
		TS_LOG_INFO( "%s :  sec2_size = %08X (%d bytes)\n",
			 __func__, fwData->sec2_size, fwData->sec2_size);

		index += FW_BYTES_ALLIGN;
		u8ToU32(&fw_data[index], &temp);
		fwData->sec3_size = temp;
		TS_LOG_INFO( "%s :  sec3_size = %08X (%d bytes)\n",
			 __func__, fwData->sec3_size, fwData->sec3_size);

		index += FW_BYTES_ALLIGN;/* skip header crc */

		/* if (!keep_cx) */
		/* { */
		dimension = fwData->sec0_size + fwData->sec1_size +
			    fwData->sec2_size + fwData->sec3_size;
		temp = fw_size;

		if (dimension + FW_HEADER_SIZE + FW_BYTES_ALLIGN != temp) {
			TS_LOG_ERR("%s : Read only %d instead of %d... ERROR %08X\n",
				 __func__, fw_size, dimension + FW_HEADER_SIZE + FW_BYTES_ALLIGN, ERROR_FILE_PARSE);
			res = ERROR_FILE_PARSE;
			goto END;
		}

		fwData->data = (u8 *)kmalloc(dimension * sizeof(u8),GFP_KERNEL);
		if (fwData->data == NULL) {
			TS_LOG_ERR( "%s : ERROR %08X\n", __func__,ERROR_ALLOC);
			res = ERROR_ALLOC;
			goto END;
		}

		index += FW_BYTES_ALLIGN;
		memcpy(fwData->data, &fw_data[index], dimension);
		if (fwData->sec2_size != 0)
			u8ToU16(&fwData->data[fwData->sec0_size +
					fwData->sec1_size + FW_CX_VERSION],
					&fwData->cx_ver);

		else {
			TS_LOG_ERR("%s : Initialize cx_ver to default value!\n",__func__);
			fwData->cx_ver = systemInfo.u16_cxVer;
		}

		TS_LOG_INFO( "%s : CX Version = %04X\n", __func__, fwData->cx_ver);
		fwData->data_size = dimension;
		TS_LOG_INFO( "%s READ FW DONE %d bytes!\n", __func__,fwData->data_size);
		res = OK;
		goto END;
	}

END:
	if(fw_data){
		kfree(fw_data);
		fw_data = NULL;
	}
	return res;
}



/**
  * Enable UVLO and Auto Power Down Mode
  * @return OK if success or an error code which specify the type of error
  */
int flash_enable_uvlo_autopowerdown(void)
{
	u8 cmd[6] = { FTS_CMD_HW_REG_W, 0x20, 0x00, 0x00, FLASH_UVLO_ENABLE_CODE0,
			FLASH_UVLO_ENABLE_CODE1 };
	u8 cmd1[6] = { FTS_CMD_HW_REG_W, 0x20, 0x00, 0x00, FLASH_AUTOPOWERDOWN_ENABLE_CODE0,
			FLASH_AUTOPOWERDOWN_ENABLE_CODE1 };

	TS_LOG_INFO( "%s Command enable uvlo ...\n", __func__);
	if (fts_write(cmd, ARRAY_SIZE(cmd)) < OK) {
		TS_LOG_ERR( "%s flash_enable_uvlo_autopowerdown: ERROR %08X\n", __func__, ERROR_BUS_W);
		return ERROR_BUS_W;
	}
	if (fts_write(cmd1, ARRAY_SIZE(cmd1)) < OK) {
		TS_LOG_ERR( "%s flash_enable_uvlo_autopowerdown: ERROR %08X\n", __func__, ERROR_BUS_W);
		return ERROR_BUS_W;
	}

	TS_LOG_INFO( "%s Enable uvlo and flash auto power down  DONE!\n", __func__);

	return OK;
}


/**
  * Unlock the flash to be programmed
  * @return OK if success or an error code which specify the type of error
  */
int flash_unlock(void)
{
	u8 cmd[6] = { FTS_CMD_HW_REG_W, 0x20, 0x00, 0x00, FLASH_UNLOCK_CODE0,
		      FLASH_UNLOCK_CODE1 };

	u8 cmd1[6] = { FTS_CMD_HW_REG_W, 0x20, 0x00, 0x00, FLASH_UNLOCK_CODE2,
		      FLASH_UNLOCK_CODE3 };

	TS_LOG_INFO( "%s Command unlock ...\n", __func__);
	if (fts_write(cmd, ARRAY_SIZE(cmd)) < OK) {
		TS_LOG_ERR( "%s flash_unlock: ERROR %08X\n", __func__, ERROR_BUS_W);
		return ERROR_BUS_W;
	}

	if (fts_write(cmd1, ARRAY_SIZE(cmd1)) < OK) {
		TS_LOG_ERR( "%s Command unlock: ERROR %08X\n", __func__, ERROR_BUS_W);
		return ERROR_BUS_W;
	}

	TS_LOG_INFO( "%s Unlock flash DONE!\n", __func__);

	return OK;
}

/**
  * Unlock the flash to be erased
  * @return OK if success or an error code which specify the type of error
  */
int flash_erase_unlock(void)
{
	u8 cmd[6] = { FTS_CMD_HW_REG_W,		0x20,	      0x00,
		      0x00,  FLASH_ERASE_UNLOCK_CODE0, FLASH_ERASE_UNLOCK_CODE1 };

	TS_LOG_INFO( "%s Try to erase unlock flash...\n", __func__);

	TS_LOG_INFO( "%s Command erase unlock ...\n", __func__);
	if (fts_write(cmd, ARRAY_SIZE(cmd)) < 0) {
		TS_LOG_ERR( "%s flash_erase_unlock: ERROR %08X\n", __func__,
			 ERROR_BUS_W);
		return ERROR_BUS_W;
	}

	TS_LOG_INFO( "%s Erase Unlock flash DONE!\n", __func__);

	return OK;
}

/**
  * Erase the full flash
  * @return OK if success or an error code which specify the type of error
  */
int flash_full_erase(void)
{
	int status = 0;
	u8 cmd1[6] = { FTS_CMD_HW_REG_W,      0x20,	 0x00,	    0x00,
		       FLASH_ERASE_CODE0 + 1, 0x00 };
	u8 cmd[6] = { FTS_CMD_HW_REG_W,	 0x20,	0x00,  0x00,
		      FLASH_ERASE_CODE0, FLASH_ERASE_CODE1 };

	if (fts_write(cmd1, ARRAY_SIZE(cmd1)) < OK) {
		TS_LOG_ERR( "%s flash_erase_page_by_page: ERROR %08X\n", __func__,
			 ERROR_BUS_W);
		return ERROR_BUS_W;
	}


	TS_LOG_INFO( "%s Command full erase sent ...\n", __func__);
	if (fts_write(cmd, ARRAY_SIZE(cmd)) < OK) {
		TS_LOG_ERR( "%s flash_full_erase: ERROR %08X\n", __func__,
			 ERROR_BUS_W);
		return ERROR_BUS_W;
	}

	status = wait_for_flash_ready(FLASH_ERASE_CODE0);

	if (status != OK) {
		TS_LOG_ERR( "%s flash_full_erase: ERROR %08X\n", __func__,
			 ERROR_FLASH_NOT_READY);
		return status | ERROR_FLASH_NOT_READY;
		/* Flash not ready within the chosen time, better exit! */
	}

	TS_LOG_INFO( "%s Full Erase flash DONE!\n", __func__);

	return OK;
}

/**
  * Erase the flash page by page, giving the possibility to skip the CX area and
  *  maintain therefore its value
  * @param keep_cx if SKIP_PANEL_INIT the Panel Init pages will be skipped,
  * if > SKIP_PANEL_CX_INIT Cx and Panel Init pages otherwise all the pages will
  * be deleted
  * @return OK if success or an error code which specify the type of error
  */
int flash_erase_page_by_page(ErasePage keep_cx)
{
	int status = 0;
	u8 i = 0;
	u8 cmd1[6] = { FTS_CMD_HW_REG_W, 0x20,0x00, 0x00,FLASH_ERASE_CODE0 + 1, 0x00 };
	u8 cmd[6] = { FTS_CMD_HW_REG_W, 0x20, 0x00, 0x00, FLASH_ERASE_CODE0, 0xA0 };
	u8 cmd2[9] = { FTS_CMD_HW_REG_W, 0x20, 0x00, 0x01, 0x28, 0xFF, 0xFF, 0xFF, 0xFF };
	u8 mask[4] = { 0 };

	for (i = FLASH_CX_PAGE_START; i <= FLASH_CX_PAGE_END && keep_cx >=
	     SKIP_PANEL_CX_INIT; i++) {
		TS_LOG_INFO( "%s Skipping erase CX page %d!\n", __func__, i);
		fromIDtoMask(i, mask, 4);
	}


	for (i = FLASH_PANEL_PAGE_START; i <= FLASH_PANEL_PAGE_END && keep_cx >=
	     SKIP_PANEL_INIT; i++) {
		TS_LOG_INFO( "%s Skipping erase Panel Init page %d!\n", __func__, i);
		fromIDtoMask(i, mask, 4);
	}


	TS_LOG_INFO( "%s Setting the page mask = ", __func__);
	for (i = 0; i < 4; i++) {
		cmd2[5 + i] = cmd2[5 + i] & (~mask[i]);
		TS_LOG_INFO( "%02X ", cmd2[5 + i]);
	}

	TS_LOG_INFO( "\n%s Writing page mask...\n", __func__);
	if (fts_write(cmd2, ARRAY_SIZE(cmd2)) < OK) {
		TS_LOG_ERR( "%s flash_erase_page_by_page: Page mask ERROR %08X\n",
			 __func__, ERROR_BUS_W);
		return ERROR_BUS_W;
	}

	if (fts_write(cmd1, ARRAY_SIZE(cmd1)) < OK) {
		TS_LOG_ERR( "%s flash_erase_page_by_page: Disable info ERROR %08X\n",
			 __func__,ERROR_BUS_W);
		return ERROR_BUS_W;
	}

	TS_LOG_INFO( "%s Command erase pages sent ...\n", __func__);
	if (fts_write(cmd, ARRAY_SIZE(cmd)) < OK) {
		TS_LOG_ERR( "%s flash_erase_page_by_page: Erase ERROR %08X\n",
			 __func__, ERROR_BUS_W);
		return ERROR_BUS_W;
	}

	status = wait_for_flash_ready(FLASH_ERASE_CODE0);

	if (status != OK) {
		TS_LOG_ERR( "%s flash_erase_page_by_page: ERROR %08X\n", __func__,
			 ERROR_FLASH_NOT_READY);
		return status | ERROR_FLASH_NOT_READY;
		/* Flash not ready within the chosen time, better exit! */
	}

	TS_LOG_INFO( "%s Erase flash page by page DONE!\n", __func__);

	return OK;
}


/**
  * Start the DMA procedure which actually transfer and burn the data loaded
  * from memory into the Flash
  * @return OK if success or an error code which specify the type of error
  */
int start_flash_dma(void)
{
	int status;
	/* write the command to erase the flash , cmd[0-12] */
	u8 cmd[ST_FLASH_DMA_LEN] = { FLASH_CMD_WRITE_REGISTER, 0x20, 0x00,
		0x00, 0x6B, 0x00, 0x40, 0x42, 0x0F, 0x00, 0x00,
		FLASH_DMA_CODE1 };

	TS_LOG_INFO("%s Command flash DMA\n", __func__);
	if (fts_write(cmd, ARRAY_SIZE(cmd)) < OK) {
		TS_LOG_ERR("%s start_flash_dma: ERROR %08X\n", __func__,
			 ERROR_BUS_W);
		return ERROR_BUS_W;
	}

	status = wait_for_flash_ready(FLASH_DMA_CODE0);

	if (status != OK) {
		TS_LOG_ERR("%s start_flash_dma: ERROR %08X\n", __func__,
			 ERROR_FLASH_NOT_READY);
		return status | ERROR_FLASH_NOT_READY;
	}
	TS_LOG_INFO("%s flash DMA DONE\n", __func__);
	return OK;
}

/**
  * Copy the FW data that should be burnt in the Flash into the memory and then
  * the DMA will take care about burning it into the Flash
  * @param address address in memory where to copy the data, possible values
  * are FLASH_ADDR_CODE, FLASH_ADDR_CONFIG, FLASH_ADDR_CX
  * @param data pointer to an array of byte which contain the data that should
  * be copied into the memory
  * @param size size of data
  * @return OK if success or an error code which specify the type of error
  */
int fillFlash(u32 address, u8 *data, int size)
{
	int remaining = size;
	int index = 0;
	int toWrite = 0;
	int byteBlock = 0;
	int wheel = 0;
	u32 addr = 0;
	int res = 0;
	int delta = 0;
	u8 *buff = NULL;
	u8 buff2[12] = { 0 };

	buff = (u8 *)kmalloc((DMA_CHUNK + 5) * sizeof(u8), GFP_KERNEL);
	if (buff == NULL) {
		TS_LOG_ERR( "%s fillFlash: ERROR %08X\n", __func__, ERROR_ALLOC);
		return ERROR_ALLOC;
	}

	while (remaining > 0) {
		byteBlock = 0;
		addr = FLASH_CMD_ADDR_HW;
		while (byteBlock < FLASH_CHUNK && remaining > 0) {
			index = 0;
			if (remaining >= DMA_CHUNK) {
				if ((byteBlock + DMA_CHUNK) <= FLASH_CHUNK) {
					toWrite = DMA_CHUNK;
					remaining -= DMA_CHUNK;
					byteBlock += DMA_CHUNK;
				} else {
					delta = FLASH_CHUNK - byteBlock;
					toWrite = delta;
					remaining -= delta;
					byteBlock += delta;
				}
			} else {
				if ((byteBlock + remaining) <= FLASH_CHUNK) {
					toWrite = remaining;
					byteBlock += remaining;
					remaining = 0;
				} else {
					delta = FLASH_CHUNK - byteBlock;
					toWrite = delta;
					remaining -= delta;
					byteBlock += delta;
				}
			}


			buff[index++] = FTS_CMD_HW_REG_W;
			buff[index++] = (u8)((addr &
						FLASH_ADDR_1BYTE_8BIT) >> 24);
			buff[index++] = (u8)((addr &
						FLASH_ADDR_2BYTE_8BIT) >> 16);
			buff[index++] = (u8)((addr &
						FLASH_ADDR_3BYTE_8BIT) >> 8);
			buff[index++] = (u8)(addr &
						FLASH_ADDR_4BYTE_8BIT);

			memcpy(&buff[index], data, toWrite);
			if (fts_write(buff, index + toWrite) < OK) {
				TS_LOG_ERR( "%s fillFlash: ERROR %08X\n", __func__, ERROR_BUS_W);
				kfree(buff);
				return ERROR_BUS_W;
			}

			/* msleep(10); */
			addr += toWrite;
			data += toWrite;
		}


		/* configuring the DMA */
		byteBlock = byteBlock / 4 - 1;
		index = 0;

		buff2[index++] = FLASH_CMD_WRITE_REGISTER;
		buff2[index++] = FLASH_DMA_CODE_CMD2;
		buff2[index++] = FLASH_DMA_CODE_CMD0;
		buff2[index++] = FLASH_DMA_CODE_CMD0;
		buff2[index++] = FLASH_DMA_CONFIG;
		buff2[index++] = FLASH_DMA_CODE_CMD0;
		buff2[index++] = FLASH_DMA_CODE_CMD0;

		addr = address + ((wheel * FLASH_CHUNK) / 4);
		buff2[index++] = (u8)((addr & FLASH_ADDR_4BYTE_8BIT));
		buff2[index++] = (u8)((addr & FLASH_ADDR_3BYTE_8BIT) >> 8);
		buff2[index++] = (u8)(byteBlock & FLASH_ADDR_4BYTE_8BIT);
		buff2[index++] = (u8)((byteBlock & FLASH_ADDR_3BYTE_8BIT) >> 8);
		buff2[index++] = 0x00;

		TS_LOG_INFO("%s DMA Command = %02X , address = %02X %02X, words =  %02X %02X\n",
			 __func__, buff2[0], buff2[8], buff2[7], buff2[10], buff2[9]);

		if (fts_write(buff2, index) < OK) {
			TS_LOG_ERR( "%s   Error during filling Flash! ERROR %08X\n",__func__,ERROR_BUS_W);
			kfree(buff);
			return ERROR_BUS_W;
		}

		res = start_flash_dma();
		if (res < OK) {
			TS_LOG_ERR("%s   Error during flashing DMA! ERROR %08X\n",__func__, res);
			kfree(buff);
			return res;
		}
		wheel++;
	}
	kfree(buff);
	return OK;
}


/**
  * Execute the procedure to burn a FW in FTM4/FTI IC
  * @param fw structure which contain the FW to be burnt
  * @param force_burn if >0, the flashing procedure will be forced and executed
  * regardless the additional info, otherwise the FW in the file will be burnt
  * only if it is newer than the one running in the IC
  * @param keep_cx if 1, the function preserve the CX/Panel Init area otherwise
  * will be cleared
  * @return OK if success or an error code which specify the type of error
  */
int flash_burn(Firmware fw, int force_burn, int keep_cx)
{
	int res = 0;
	if (!force_burn) {
			if (fw.fw_ver!=  systemInfo.u16_fwVer || fw.config_id != systemInfo.u16_cfgProjectId)
				goto start;
		TS_LOG_ERR("%s flash_burn: Firmware in the chip matches(or later) the firmware to flash! NO UPDATE ERROR %08X\n",
			 __func__, ERROR_FW_NO_UPDATE);
		return ERROR_FW_NO_UPDATE | ERROR_FLASH_BURN_FAILED;
	} else {
		/* burn procedure to update the CX memory, if not present just
		 * skip it if there isn't a new fw release. */
		if (force_burn == CRC_CX && fw.sec2_size == 0) {
			if (fw.fw_ver!=  systemInfo.u16_fwVer || fw.config_id != systemInfo.u16_cfgProjectId){
				/* Avoid loading the CX because it is missing  in the bin file, it just need to update to last fw+cfg because a new release */
				force_burn = 0;
				goto start;
			}
		TS_LOG_ERR("%s flash_burn: CRC in CX but fw does not contain CX data! NO UPDATE ERROR %08X\n",
							 __func__, ERROR_FW_NO_UPDATE);
		return ERROR_FW_NO_UPDATE | ERROR_FLASH_BURN_FAILED;
		}
	}

	/* programming procedure start */
start:
	TS_LOG_INFO( "%s Programming Procedure for flashing started:\n\n", __func__);

	TS_LOG_INFO( "%s 1) SYSTEM RESET:\n", __func__);
	res = fts_system_reset();
	if (res < 0) {
		TS_LOG_ERR( "%s    system reset FAILED!\n", __func__);
		/* If there is no firmware, there is no controller ready event
		  * and there will be a timeout, we can keep going. But if
		  * there is an I2C error, we must exit.
		  */
		if (res != (ERROR_SYSTEM_RESET_FAIL | ERROR_TIMEOUT))
			return res | ERROR_FLASH_BURN_FAILED;
	} else
		TS_LOG_INFO( "%s   system reset COMPLETED!\n\n", __func__);

	msleep(100); /* required by hw during flash procedure */

	TS_LOG_INFO( "%s 2) HOLD M3 :\n", __func__);
	res = hold_m3();
	if (res < OK) {
		TS_LOG_ERR( "%s    hold_m3 FAILED!\n", __func__);
		return res | ERROR_FLASH_BURN_FAILED;
	} else
		TS_LOG_INFO( "%s    hold_m3 COMPLETED!\n\n", __func__);


	TS_LOG_INFO( "%s 3) ENABLE UVLO AND AUTO POWER DOWN MODE :\n", __func__);
	res = flash_enable_uvlo_autopowerdown();
	if (res < OK) {
		TS_LOG_ERR( "%s    flash_enable_uvlo_autopowerdown FAILED!\n", __func__);
		return res | ERROR_FLASH_BURN_FAILED;
	} else
		TS_LOG_INFO( "%s    flash_enable_uvlo_autopowerdown COMPLETED!\n\n", __func__);


	TS_LOG_INFO( "%s 4) FLASH UNLOCK:\n", __func__);
	res = flash_unlock();
	if (res < OK) {
		TS_LOG_ERR( "%s   flash unlock FAILED! ERROR %08X\n", __func__,
			 ERROR_FLASH_BURN_FAILED);
		return res | ERROR_FLASH_BURN_FAILED;
	} else
		TS_LOG_INFO( "%s   flash unlock COMPLETED!\n\n", __func__);


	TS_LOG_INFO( "%s 5) FLASH ERASE UNLOCK:\n", __func__);
	res = flash_erase_unlock();
	if (res < 0) {
		TS_LOG_ERR( "%s   flash unlock FAILED! ERROR %08X\n", __func__,
			 ERROR_FLASH_BURN_FAILED);
		return res | ERROR_FLASH_BURN_FAILED;
	} else
		TS_LOG_INFO( "%s   flash unlock COMPLETED!\n\n", __func__);

	TS_LOG_INFO( "%s 6) FLASH ERASE:\n", __func__);
	if (keep_cx > 0) {
		if (fw.sec2_size != 0 && force_burn == CRC_CX)
			res = flash_erase_page_by_page(SKIP_PANEL_INIT);
		else
			res = flash_erase_page_by_page(SKIP_PANEL_CX_INIT);
	} else {
		/* res = flash_full_erase(); */
		res = flash_erase_page_by_page(SKIP_PANEL_INIT);
		if (fw.sec2_size == 0)
			TS_LOG_ERR(
				 "%s WARNING!!! Erasing CX memory but no CX in fw file! touch will not work right after fw update!\n",
				 __func__);
	}

	if (res < OK) {
		TS_LOG_ERR( "%s   flash erase FAILED! ERROR %08X\n", __func__,
			 ERROR_FLASH_BURN_FAILED);
		return res | ERROR_FLASH_BURN_FAILED;
	} else
		TS_LOG_INFO( "%s   flash erase COMPLETED!\n\n", __func__);

	TS_LOG_INFO( "%s 7) LOAD PROGRAM:\n", __func__);
	res = fillFlash(FLASH_ADDR_CODE, &fw.data[0], fw.sec0_size);
	if (res < OK) {
		TS_LOG_ERR( "%s   load program ERROR %08X\n", __func__,
			 ERROR_FLASH_BURN_FAILED);
		return res | ERROR_FLASH_BURN_FAILED;
	}
	TS_LOG_ERR( "%s   load program DONE!\n", __func__);

	TS_LOG_INFO( "%s 8) LOAD CONFIG:\n", __func__);
	res = fillFlash(FLASH_ADDR_CONFIG, &(fw.data[fw.sec0_size]), fw.sec1_size);
	if (res < OK) {
		TS_LOG_ERR( "%s   load config ERROR %08X\n", __func__,
			 ERROR_FLASH_BURN_FAILED);
		return res | ERROR_FLASH_BURN_FAILED;
	}
	TS_LOG_ERR( "%s   load config DONE!\n", __func__);

	if (fw.sec2_size != 0 && (force_burn == CRC_CX || keep_cx <= 0)) {
		TS_LOG_INFO( "%s 8.1) LOAD CX:\n", __func__);
		res = fillFlash(FLASH_ADDR_CX, &(fw.data[fw.sec0_size + fw.sec1_size]),fw.sec2_size);
		if (res < OK) {
			TS_LOG_ERR( "%s   load cx ERROR %08X\n", __func__,
				 ERROR_FLASH_BURN_FAILED);
			return res | ERROR_FLASH_BURN_FAILED;
		}
		TS_LOG_ERR( "%s   load cx DONE!\n", __func__);
	}

	TS_LOG_INFO( "%s   Flash burn COMPLETED!\n\n", __func__);

	TS_LOG_INFO( "%s 9) SYSTEM RESET:\n", __func__);
	res = fts_system_reset();
	if (res < 0) {
		TS_LOG_ERR( "%s    system reset FAILED! ERROR %08X\n", __func__,
			 ERROR_FLASH_BURN_FAILED);
		return res | ERROR_FLASH_BURN_FAILED;
	}
	TS_LOG_INFO( "%s   system reset COMPLETED!\n\n", __func__);


	TS_LOG_INFO( "%s 10) FINAL CHECK:\n", __func__);
	res = readSysInfo(0);
	if (res < 0) {
		TS_LOG_ERR("%s flash_burn: Unable to retrieve Chip INFO! ERROR %08X\n",
			 __func__, ERROR_FLASH_BURN_FAILED);
		return res | ERROR_FLASH_BURN_FAILED;
	}


	for (res = 0; res < EXTERNAL_RELEASE_INFO_SIZE; res++) {
		if (fw.externalRelease[res] != systemInfo.u8_releaseInfo[res]) {
			/* External release is printed during readSysInfo */
			TS_LOG_ERR( "%s  Firmware in the chip different from the one that was burn!\n",
				 __func__);
			return ERROR_FLASH_BURN_FAILED;
		}
	}


	TS_LOG_INFO( "%s   Final check OK!\n", __func__);

	return OK;
}
