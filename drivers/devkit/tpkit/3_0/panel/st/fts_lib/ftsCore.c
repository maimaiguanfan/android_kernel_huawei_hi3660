/*
  *
  **************************************************************************
  **                        STMicroelectronics				**
  **************************************************************************
  **                        marco.cali@st.com				**
  **************************************************************************
  *                                                                        *
  *		FTS Core functions					 *
  *                                                                        *
  **************************************************************************
  **************************************************************************
  *
  */

/*!
  * \file ftsCore.c
  * \brief Contains the implementation of the Core functions
  */

#include <linux/spinlock.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include "ftsCompensation.h"
#include "ftsCore.h"
#include "ftsError.h"
#include "ftsIO.h"
#include "ftsTest.h"
#include "ftsTime.h"
#include "ftsTool.h"
#include "../fts.h"
/** @addtogroup system_info
  * @{
  */
SysInfo systemInfo;	/* /< Global System Info variable, accessible in all the
			 * driver */
/** @}*/

static int reset_gpio = GPIO_NOT_DEFINED;	/* /< gpio number of the rest
						 * pin, the value is
						 *  GPIO_NOT_DEFINED if the
						 * reset pin is not connected */
static int st_irq_gpio = GPIO_NOT_DEFINED;
static int system_reseted_up = 0;
static int system_reseted_down =0;

#define FTS_POLLFORRVENT_TEMPSIZE 128

/**
  * Initialize core variables of the library.
  * Must be called during the probe before any other lib function
  * @param info pointer to fts_ts_info which contains info about the device and
  *its hw setup
  * @return OK if success or an error code which specify the type of error
  */
int initCore(struct fts_ts_info *info)
{
	int ret = OK;

	TS_LOG_INFO( "%s: Initialization of the Core...\n", __func__);
	ret |= resetErrorList();
	ret |= initTestToDo();

	if (ret < OK)
		TS_LOG_ERR("%s: Initialization Core ERROR %08X!\n",__func__, ret);
	else
		TS_LOG_ERR( "%s: Initialization Finished!\n",__func__);
	return ret;
}

/**
  * Set the reset_gpio variable with the actual gpio number of the board link to
  *the reset pin
  * @param gpio gpio number link to the reset pin of the IC
  */
void setResetGpio(int gpio)
{
	reset_gpio = gpio;
	TS_LOG_ERR( "%s %s: reset_gpio = %d\n", __func__,__func__, reset_gpio);
}
void setIrqGpio(int gpio)
{
	st_irq_gpio = gpio;
	TS_LOG_ERR( "%s %s: reset_gpio = %d\n", __func__,__func__, st_irq_gpio);
}

/**
  * Perform a system reset of the IC.
  * If the reset pin is associated to a gpio, the function execute an hw reset
  * (toggling of reset pin) otherwise send an hw command to the IC
  * @return OK if success or an error code which specify the type of error
  */
int fts_system_reset(void)
{
	u8 readData[FIFO_EVENT_SIZE] ={0};
	int event_to_search =(int)EVT_ID_CONTROLLER_READY;
	int res = -1;
	int i =0;
	u8 data[1] = { SYSTEM_RESET_VALUE };
	struct fts_ts_info *info = fts_get_info();

	TS_LOG_INFO("%s :starting ...\n", __func__);
	for (i = 0; i < RETRY_SYSTEM_RESET && res < 0; i++) {
		resetErrorList();

		disable_irq(info->chip_data->ts_platform_data->irq_id);
		if (reset_gpio == GPIO_NOT_DEFINED){
			res = fts_writeU8UX(FTS_CMD_HW_REG_W, ADDR_SIZE_HW_REG,
					    ADDR_SYSTEM_RESET, data, ARRAY_SIZE( data));
		} else {
			gpio_set_value(reset_gpio, 0);
			msleep(10);
			gpio_set_value(reset_gpio, 1);
			res = OK;
		}

		if (res < OK){
			TS_LOG_ERR( "%s fts_system_reset: ERROR %08X\n", __func__,ERROR_BUS_W);
		} else {
			fts_disableInterrupt();
			res = pollForEvent(&event_to_search, 1, readData,
					   GENERAL_TIMEOUT);
			if (res < OK)
				TS_LOG_ERR( "%s fts_system_reset: ERROR %08X\n", __func__, res);
		}

		enable_irq(info->chip_data->ts_platform_data->irq_id);
	}

	if (res < OK) {
		TS_LOG_ERR("%s fts_system_reset...failed after 3 attempts: ERROR %08X\n",
			 __func__, (res | ERROR_SYSTEM_RESET_FAIL));
		return res | ERROR_SYSTEM_RESET_FAIL;
	} else {
		TS_LOG_INFO( "%s System reset DONE!\n", __func__);
		system_reseted_down = 1;
		system_reseted_up = 1;
		return OK;
	}
}

/**
  * Return the value of system_resetted_down.
  * @return the flag value: 0 if not set, 1 if set
  */
int isSystemResettedDown(void)
{
	return system_reseted_down;
}

/**
  * Return the value of system_resetted_up.
  * @return the flag value: 0 if not set, 1 if set
  */
int isSystemResettedUp(void)
{
	return system_reseted_up;
}


/**
  * Set the value of system_reseted_down flag
  * @param val value to write in the flag
  */
void setSystemResetedDown(int val)
{
	system_reseted_down = val;
}

/**
  * Set the value of system_reseted_up flag
  * @param val value to write in the flag
  */
void setSystemResetedUp(int val)
{
	system_reseted_up = val;
}


/** @addtogroup events_group
  * @{
  */

/**
  * Poll the FIFO looking for a specified event within a timeout. Support a
  *retry mechanism.
  * @param event_to_search pointer to an array of int where each element
  * correspond to a byte of the event to find.
  * If the element of the array has value -1, the byte of the event,
  * in the same position of the element is ignored.
  * @param event_bytes size of event_to_search
  * @param readData pointer to an array of byte which will contain the event
  *found
  * @param time_to_wait time to wait before going in timeout
  * @return OK if success or an error code which specify the type of error
  */
int pollForEvent(int *event_to_search, int event_bytes, u8 *readData, int
		 time_to_wait)
{
	int i =0, find= 0, retry =0 , count_err = 0;
	int time_to_count = 0;
	int err_handling = OK;
	StopWatch clock;
	char * temp1 = NULL;

	u8 cmd[1] = { FIFO_CMD_READONE };
	char temp[FTS_POLLFORRVENT_TEMPSIZE] = { 0 };

	time_to_count = time_to_wait / TIMEOUT_RESOLUTION;

	startStopWatch(&clock);
	while (find != 1 && retry < time_to_count &&
		fts_writeReadU8UX(cmd[0], 0, 0, readData, FIFO_EVENT_SIZE,  DUMMY_FIFO) >= OK) {

		if (readData[0] == EVT_ID_ERROR) {
			temp1 = printHex("ERROR EVENT = ",readData,FIFO_EVENT_SIZE,temp);
			if(temp1){
				TS_LOG_ERR( "%s %s\n", __func__, temp1);
			}
			memset(temp, 0, FTS_POLLFORRVENT_TEMPSIZE);
			count_err++;
			err_handling = errorHandler(readData, FIFO_EVENT_SIZE);
			if ((err_handling & 0xF0FF0000) == ERROR_HANDLER_STOP_PROC) {
				TS_LOG_ERR( "%s pollForEvent: forced to be stopped! ERROR %08X\n",
					 __func__, err_handling);
				return err_handling;
			}
		} else {
			if (readData[0] != EVT_ID_NOEVENT) {
				temp1 = printHex( "READ EVENT = ", readData,FIFO_EVENT_SIZE,temp);
				if(temp1){
					TS_LOG_ERR( "%s %s\n", __func__, temp1);
				}
				memset(temp, 0, FTS_POLLFORRVENT_TEMPSIZE);
			}

			if ((readData[0] == EVT_ID_CONTROLLER_READY) &&
					(event_to_search[0] != EVT_ID_CONTROLLER_READY)) {
				TS_LOG_ERR("%s pollForEvent: Unmanned Controller Ready Event! Setting reset flags...\n",__func__ );
				setSystemResetedUp(1);
				setSystemResetedDown(1);
			}
		}

		find = 1;
		for (i = 0; i < event_bytes; i++) {
			if ((event_to_search[i] != -1) && ((int)readData[i] !=event_to_search[i])) {
				find = 0;
				break;
			}
		}

		retry++;
		msleep(TIMEOUT_RESOLUTION);
	}
	stopStopWatch(&clock);
	if ((retry >= time_to_count) && find != 1) {
		TS_LOG_ERR( "%s pollForEvent: ERROR %08X\n", __func__, ERROR_TIMEOUT);
		return ERROR_TIMEOUT;
	} else if (find == 1) {
		temp1 = printHex("FOUND EVENT = ", readData,FIFO_EVENT_SIZE, temp);
		if(temp1){
			TS_LOG_ERR( "%s %s\n", __func__, temp1);
		}
		memset(temp, 0, FTS_POLLFORRVENT_TEMPSIZE);
		TS_LOG_INFO("%s Event found in %d ms (%d iterations)! Number of errors found = %d\n",
			 __func__, elapsedMillisecond(&clock), retry, count_err);
		return count_err;
	} else {
		TS_LOG_ERR( "%s pollForEvent: ERROR %08X\n", __func__, ERROR_BUS_R);
		return ERROR_BUS_R;
	}
}

/** @}*/

/**
  * Check that the FW sent the echo even after a command was sent
  * @param cmd pointer to an array of byte which contain the command previously
  *sent
  * @param size size of cmd
  * @return OK if success or an error code which specify the type of error
  */
int checkEcho(u8 *cmd, int size)
{
	int ret =0, i =0;
	int event_to_search[FIFO_EVENT_SIZE] = {0};
	u8 readData[FIFO_EVENT_SIZE] = {0};

	if (size < 1 || !cmd) {
		TS_LOG_ERR("%s checkEcho: Invalid Size = %d not valid! ERROR %08X\n",
			 __func__,size, ERROR_OP_NOT_ALLOW);
		return ERROR_OP_NOT_ALLOW;
	} else {
		if ((size + 3) > FIFO_EVENT_SIZE)
			size = FIFO_EVENT_SIZE - 3;
		/* Echo event 0x43 0x01 xx xx xx xx xx fifo_status
		  * therefore command with more than 5 bytes will be trunked */

		event_to_search[0] = EVT_ID_STATUS_UPDATE;
		event_to_search[1] = EVT_TYPE_STATUS_ECHO;
		for (i = 2; i < size + 2; i++)
			event_to_search[i] = cmd[i - 2];
		ret = pollForEvent(event_to_search, size + 2, readData, TIEMOUT_ECHO);
		if (ret < OK) {
			TS_LOG_ERR( "%s checkEcho: Echo Event not found! ERROR %08X\n",
				 __func__, ret);
			return ret | ERROR_CHECK_ECHO_FAIL;
		} else if (ret > OK) {
			TS_LOG_ERR( "%s checkEcho: Echo Event found but with some error events before! num_error = %d\n",
				 __func__, ret);
			return ERROR_CHECK_ECHO_FAIL;
		}

		TS_LOG_INFO( "%s ECHO OK!\n", __func__);
		return ret;
	}
}


/** @addtogroup scan_mode
  * @{
  */
/**
  * Set a scan mode in the IC
  * @param mode scan mode to set; possible values @link scan_opt Scan Mode
  * Option @endlink
  * @param settings option for the selected scan mode
  * (for example @link active_bitmask Active Mode Bitmask @endlink)
  * @return OK if success or an error code which specify the type of error
  */
int setScanMode(u8 mode, u8 settings)
{
	u8 cmd[3] = { FTS_CMD_SCAN_MODE, mode, settings };
	int ret = 0;
	int size = 3;

	TS_LOG_INFO( "%s: Setting scan mode: mode = %02X settings = %02X !\n",
		 __func__, mode, settings);
	if (mode == SCAN_MODE_LOW_POWER)
		size = 2;
	/* use write instead of writeFw because can be called while the interrupt are  enabled */
	ret = fts_write(cmd, size);
	if (ret < OK) {
		TS_LOG_ERR( "%s: write failed...ERROR %08X !\n",__func__, ret);
		return ret | ERROR_SET_SCAN_MODE_FAIL;
	}
	TS_LOG_INFO( "%s: Setting scan mode OK!\n", __func__);
	return OK;
}
/** @}*/


/** @addtogroup feat_sel
  * @{
  */
/**
  * Set a feature and its option in the IC
  * @param feat feature to set; possible values @link feat_opt Feature Selection
  *Option @endlink
  * @param settings pointer to an array of byte which store the options for
  * the selected feature (for example the gesture mask to activate
  * @link gesture_opt Gesture IDs @endlink)
  * @param size in bytes of settings
  * @return OK if success or an error code which specify the type of error
  */
int setFeatures(u8 feat, u8 *settings, int size)
{
	u8 cmd[2 + size];
	int i = 0;
	int ret =0;

	TS_LOG_INFO( "%s: Setting feature: feat = %02X !\n", __func__,feat);
	cmd[0] = FTS_CMD_FEATURE;
	cmd[1] = feat;
	TS_LOG_INFO( "%s: Settings = ",  __func__);
	for (i = 0; i < size; i++) {
		cmd[2 + i] = settings[i];
		TS_LOG_INFO( "%02X ", settings[i]);
	}
	/* use write instead of writeFw because can be called while the interrupts are enabled */
	ret = fts_write(cmd, 2 + size);
	if (ret < OK) {
		TS_LOG_ERR( "%s: write failed...ERROR %08X !\n", __func__, ret);
		return ret | ERROR_SET_FEATURE_FAIL;
	}
	TS_LOG_INFO( "%s: Setting feature OK!\n",  __func__);
	return OK;
}
/** @}*/

/** @addtogroup sys_cmd
  * @{
  */
/**
  * Write a system command to the IC
  * @param sys_cmd System Command to execute; possible values
  * @link sys_opt System Command Option @endlink
  * @param sett settings option for the selected system command
  * (@link sys_special_opt Special Command Option @endlink, @link ito_opt
  * ITO Test Option @endlink, @link load_opt Load Host Data Option @endlink)
  * @param size in bytes of settings
  * @return OK if success or an error code which specify the type of error
  */
int writeSysCmd(u8 sys_cmd, u8 *sett, int size)
{
	u8 cmd[2 + size];
	int ret = 0;

	cmd[0] = FTS_CMD_SYSTEM;
	cmd[1] = sys_cmd;

	TS_LOG_INFO( "%s: Command = %02X %02X \n",__func__, cmd[0], cmd[1]);
	for (ret = 0; ret < size; ret++) {
		cmd[2 + ret] = sett[ret];
		TS_LOG_INFO( "%s:%02X \n", __func__,cmd[2 + ret]);
	}

	TS_LOG_INFO( "%s: Writing Sys command...\n", __func__);
	if (sys_cmd != SYS_CMD_LOAD_DATA)
		ret = fts_writeFwCmd(cmd, 2 + size);
	else {
		if (size >= 1)
			ret = requestSyncFrame(sett[0]);
		else {
			TS_LOG_ERR( "%s: No setting argument! ERROR %08X\n",
				 __func__, ERROR_OP_NOT_ALLOW);
			return ERROR_OP_NOT_ALLOW;
		}
	}
	if (ret < OK)
		TS_LOG_ERR( "%s: ERROR %08X\n",__func__, ret);

	else
		TS_LOG_INFO( "%s: FINISHED!\n",  __func__);

	return ret;
}
/** @}*/

/** @addtogroup system_info
  * @{
  */
/**
  * Initialize the System Info Struct with default values according to the error
  * found during the reading
  * @param i2cError 1 if there was an I2C error while reading the System Info
  * data from memory, other value if another error occurred
  * @return OK if success or an error code which specify the type of error
  */
void defaultSysInfo(int i2cError)
{
	int i = 0;

	TS_LOG_ERR("%s Setting default System Info...\n", __func__);

	if (i2cError == 1) {
		systemInfo.u16_fwVer = 0xFFFF;
		systemInfo.u16_cfgProjectId = 0xFFFF;
		for (i = 0; i < RELEASE_INFO_SIZE; i++){
			systemInfo.u8_releaseInfo[i] = 0xFF;
		}
		systemInfo.u16_cxVer = 0xFFFF;
	} else {
		systemInfo.u16_fwVer = 0x0000;
		systemInfo.u16_cfgProjectId = 0x0000;
		for (i = 0; i < RELEASE_INFO_SIZE; i++){
			systemInfo.u8_releaseInfo[i] = 0x00;
		}
		systemInfo.u16_cxVer = 0x0000;
	}

	systemInfo.u8_scrRxLen = 0;
	systemInfo.u8_scrTxLen = 0;

	TS_LOG_ERR("%s default System Info DONE!\n", __func__);
	return ;
}

/**
  * Read the System Info data from memory. System Info is loaded automatically
  * after every system reset.
  * @param request if 1, will be asked to the FW to reload the data, otherwise
  * attempt to read it directly from memory
  * @return OK if success or an error code which specify the type of error
  */
int readSysInfo(int request)
{
	int ret = 0, i = 0, index = 0;
	u8 sett = LOAD_SYS_INFO;
	u8 data[SYS_INFO_SIZE] = { 0 };
	char temp[256] = { 0 };
	char * temp1 = NULL;

	if (request == 1) {
		TS_LOG_ERR("%s: Requesting System Info...\n", __func__);

		ret = writeSysCmd(SYS_CMD_LOAD_DATA, &sett, 1);
		if (ret < OK) {
			TS_LOG_ERR("%s: error while writing the sys cmd ERROR %08X\n",__func__, ret);
			goto FAIL;
		}
	}

	TS_LOG_INFO("%s: Reading System Info...\n", __func__);
	ret = fts_writeReadU8UX(FTS_CMD_FRAMEBUFFER_R, BITS_16,
				ADDR_FRAMEBUFFER, data, SYS_INFO_SIZE,
				DUMMY_FRAMEBUFFER);
	if (ret < OK) {
		TS_LOG_ERR("%s: error while reading the system data ERROR %08X\n", __func__, ret);
		goto FAIL;
	}

	TS_LOG_INFO( "%s: Parsing System Info...\n",__func__);

	if (data[0] != HEADER_SIGNATURE) {
		TS_LOG_ERR("%s : The Header Signature is wrong!  sign: %02X != %02X ERROR %08X\n",
			  __func__, data[0], HEADER_SIGNATURE, ERROR_WRONG_DATA_SIGN);
		ret = ERROR_WRONG_DATA_SIGN;
		goto FAIL;
	}


	if (data[1] != LOAD_SYS_INFO) {
		TS_LOG_ERR("%s: The Data ID is wrong!  ids: %02X != %02X ERROR %08X\n",
			__func__, data[3], LOAD_SYS_INFO,ERROR_DIFF_DATA_TYPE);
		ret = ERROR_DIFF_DATA_TYPE;
		goto FAIL;
	}

	index += 4;
	u8ToU16(&data[index], &systemInfo.u16_apiVer_rev);
	index += 2;
	systemInfo.u8_apiVer_minor = data[index++];
	systemInfo.u8_apiVer_major = data[index++];
	u8ToU16(&data[index], &systemInfo.u16_chip0Ver);
	index += 2;
	u8ToU16(&data[index], &systemInfo.u16_chip0Id);
	index += 2;
	u8ToU16(&data[index], &systemInfo.u16_chip1Ver);
	index += 2;
	u8ToU16(&data[index], &systemInfo.u16_chip1Id);
	index += 2;
	u8ToU16(&data[index], &systemInfo.u16_fwVer);
	index += 2;
	TS_LOG_ERR("%s FW VER = %04X\n", __func__, systemInfo.u16_fwVer);

	u8ToU16(&data[index], &systemInfo.u16_svnRev);
	index += 2;
	TS_LOG_ERR( "%s SVN REV = %04X\n", __func__, systemInfo.u16_svnRev);

	u8ToU16(&data[index], &systemInfo.u16_cfgVer);
	index += 2;
	TS_LOG_ERR( "%s CONFIG VER = %04X\n", __func__, systemInfo.u16_cfgVer);

	u8ToU16(&data[index], &systemInfo.u16_cfgProjectId);
	index += 2;
	TS_LOG_ERR("%s CONFIG PROJECT ID = %04X\n", __func__,systemInfo.u16_cfgProjectId);

	u8ToU16(&data[index], &systemInfo.u16_cxVer);
	index += 2;
	TS_LOG_ERR("%s CX VER = %04X\n", __func__, systemInfo.u16_cxVer);

	u8ToU16(&data[index], &systemInfo.u16_cxProjectId);
	index += 2;
	TS_LOG_ERR("%s CX PROJECT ID = %04X\n", __func__, systemInfo.u16_cxProjectId);

	systemInfo.u8_cfgAfeVer = data[index++];
	systemInfo.u8_cxAfeVer =  data[index++];
	systemInfo.u8_panelCfgAfeVer = data[index++];
	TS_LOG_ERR("%s AFE VER: CFG = %02X - CX = %02X - PANEL = %02X\n", __func__,
		 systemInfo.u8_cfgAfeVer, systemInfo.u8_cxAfeVer,systemInfo.u8_panelCfgAfeVer);

	systemInfo.u8_protocol = data[index++];
	TS_LOG_ERR( "%s Protocol = %02X\n", __func__, systemInfo.u8_protocol);

	for (i = 0; i < DIE_INFO_SIZE; i++){
		systemInfo.u8_dieInfo[i] = data[index++];
	}
	temp1  = printHex("Die Info =  ",systemInfo.u8_dieInfo,DIE_INFO_SIZE, temp);
	if(temp1){
		TS_LOG_ERR( "%s %s\n", __func__, temp1);
	}
	memset(temp, 0, sizeof(temp));

	for (i = 0; i < RELEASE_INFO_SIZE; i++)
		systemInfo.u8_releaseInfo[i] = data[index++];
	temp1  = printHex("Release Info =  ",systemInfo.u8_releaseInfo,RELEASE_INFO_SIZE, temp);
	if(temp1){
		TS_LOG_ERR( "%s %s\n", __func__, temp1);
	}
	memset(temp, 0,  sizeof(temp));

	u8ToU32(&data[index], &systemInfo.u32_fwCrc);
	index += 4;
	u8ToU32(&data[index], &systemInfo.u32_cfgCrc);
	index += 4;

	index += 4;	/* skip reserved area */

	systemInfo.u8_mpFlag = data[index++];
	TS_LOG_ERR( "%s MP FLAG = %02X\n", __func__,
		 systemInfo.u8_mpFlag);

	index += 3 + 4; /* +3 remaining from mp flag address */
	systemInfo.u8_ssDetScanSet = data[index];
	TS_LOG_ERR("%s SS Detect Scan Select = %d \n", __func__, systemInfo.u8_ssDetScanSet);

	index += 4;
	u8ToU16(&data[index], &systemInfo.u16_scrResX);
	index += 2;
	u8ToU16(&data[index], &systemInfo.u16_scrResY);
	index += 2;
	TS_LOG_ERR("%s Screen Resolution = %d x %d\n", __func__,systemInfo.u16_scrResX, systemInfo.u16_scrResY);

	systemInfo.u8_scrTxLen = data[index++];
	TS_LOG_ERR("%s TX Len = %d\n", __func__, systemInfo.u8_scrTxLen);
	systemInfo.u8_scrRxLen = data[index++];
	TS_LOG_ERR("%s RX Len = %d\n", __func__, systemInfo.u8_scrRxLen);
	systemInfo.u8_keyLen = data[index++];
	TS_LOG_ERR("%s Key Len = %d\n", __func__, systemInfo.u8_keyLen);
	systemInfo.u8_forceLen = data[index++];
	TS_LOG_ERR( "%s Force Len = %d\n", __func__, systemInfo.u8_forceLen);

	index += 40;	/* skip reserved area */

	u8ToU16(&data[index], &systemInfo.u16_dbgInfoAddr);
	index += 2;

	index += 6;	/* skip reserved area */

	u8ToU16(&data[index], &systemInfo.u16_msTchRawAddr);
	index += 2;
	u8ToU16(&data[index], &systemInfo.u16_msTchFilterAddr);
	index += 2;
	u8ToU16(&data[index], &systemInfo.u16_msTchStrenAddr);
	index += 2;
	u8ToU16(&data[index], &systemInfo.u16_msTchBaselineAddr);
	index += 2;

	u8ToU16(&data[index], &systemInfo.u16_ssTchTxRawAddr);
	index += 2;
	u8ToU16(&data[index], &systemInfo.u16_ssTchTxFilterAddr);
	index += 2;
	u8ToU16(&data[index], &systemInfo.u16_ssTchTxStrenAddr);
	index += 2;
	u8ToU16(&data[index], &systemInfo.u16_ssTchTxBaselineAddr);
	index += 2;

	u8ToU16(&data[index], &systemInfo.u16_ssTchRxRawAddr);
	index += 2;
	u8ToU16(&data[index], &systemInfo.u16_ssTchRxFilterAddr);
	index += 2;
	u8ToU16(&data[index], &systemInfo.u16_ssTchRxStrenAddr);
	index += 2;
	u8ToU16(&data[index], &systemInfo.u16_ssTchRxBaselineAddr);
	index += 2;

	u8ToU16(&data[index], &systemInfo.u16_keyRawAddr);
	index += 2;
	u8ToU16(&data[index], &systemInfo.u16_keyFilterAddr);
	index += 2;
	u8ToU16(&data[index], &systemInfo.u16_keyStrenAddr);
	index += 2;
	u8ToU16(&data[index], &systemInfo.u16_keyBaselineAddr);
	index += 2;

	u8ToU16(&data[index], &systemInfo.u16_frcRawAddr);
	index += 2;
	u8ToU16(&data[index], &systemInfo.u16_frcFilterAddr);
	index += 2;
	u8ToU16(&data[index], &systemInfo.u16_frcStrenAddr);
	index += 2;
	u8ToU16(&data[index], &systemInfo.u16_frcBaselineAddr);
	index += 2;

	u8ToU16(&data[index], &systemInfo.u16_ssHvrTxRawAddr);
	index += 2;
	u8ToU16(&data[index], &systemInfo.u16_ssHvrTxFilterAddr);
	index += 2;
	u8ToU16(&data[index], &systemInfo.u16_ssHvrTxStrenAddr);
	index += 2;
	u8ToU16(&data[index], &systemInfo.u16_ssHvrTxBaselineAddr);
	index += 2;

	u8ToU16(&data[index], &systemInfo.u16_ssHvrRxRawAddr);
	index += 2;
	u8ToU16(&data[index], &systemInfo.u16_ssHvrRxFilterAddr);
	index += 2;
	u8ToU16(&data[index], &systemInfo.u16_ssHvrRxStrenAddr);
	index += 2;
	u8ToU16(&data[index], &systemInfo.u16_ssHvrRxBaselineAddr);
	index += 2;

	u8ToU16(&data[index], &systemInfo.u16_ssPrxTxRawAddr);
	index += 2;
	u8ToU16(&data[index], &systemInfo.u16_ssPrxTxFilterAddr);
	index += 2;
	u8ToU16(&data[index], &systemInfo.u16_ssPrxTxStrenAddr);
	index += 2;
	u8ToU16(&data[index], &systemInfo.u16_ssPrxTxBaselineAddr);
	index += 2;

	u8ToU16(&data[index], &systemInfo.u16_ssPrxRxRawAddr);
	index += 2;
	u8ToU16(&data[index], &systemInfo.u16_ssPrxRxFilterAddr);
	index += 2;
	u8ToU16(&data[index], &systemInfo.u16_ssPrxRxStrenAddr);
	index += 2;
	u8ToU16(&data[index], &systemInfo.u16_ssPrxRxBaselineAddr);
	index += 2;

	u8ToU16(&data[index], &systemInfo.u16_ssDetRawAddr);
	index += 2;
	u8ToU16(&data[index], &systemInfo.u16_ssDetFilterAddr);
	index += 2;
	u8ToU16(&data[index], &systemInfo.u16_ssDetStrenAddr);
	index += 2;
	u8ToU16(&data[index], &systemInfo.u16_ssDetBaselineAddr);
	index += 2;

	TS_LOG_ERR("%s Parsed %d bytes!\n", __func__, index);


	if (index != SYS_INFO_SIZE) {
		TS_LOG_ERR("%s: index = %d different from %d ERROR %08X\n",
			 __func__,index, SYS_INFO_SIZE,ERROR_OP_NOT_ALLOW);
		return ERROR_OP_NOT_ALLOW;
	}

	TS_LOG_ERR( "%s System Info Read DONE!\n", __func__);
	return OK;

FAIL:
	if(isI2cError(ret))	
		defaultSysInfo(1);
	else
		defaultSysInfo(0);
	return ret;
}
/** @}*/


/**
  * Read data from the Config Memory
  * @param offset Starting address in the Config Memory of data to read
  * @param outBuf pointer of a byte array which contain the bytes to read
  * @param len number of bytes to read
  * @return OK if success or an error code which specify the type of error
  */
int readConfig(u16 offset, u8 *outBuf, int len)
{
	int ret = 0;
	u64 final_address = offset + ADDR_CONFIG_OFFSET;

	TS_LOG_INFO("%s: Starting to read config memory at %08llX ...\n",__func__, final_address);
	ret = fts_writeReadU8UX(FTS_CMD_CONFIG_R, BITS_16, final_address,
				outBuf, len, DUMMY_CONFIG);
	if (ret < OK) {
		TS_LOG_ERR(" %s: Impossible to read Config Memory... ERROR %08X!\n",__func__,ret);
		return ret;
	}

	TS_LOG_INFO("%s: Read config memory FINISHED!\n", __func__);
	return OK;
}

/**
  * Write data into the Config Memory
  * @param offset Starting address in the Config Memory where write the data
  * @param data pointer of a byte array which contain the data to write
  * @param len number of bytes to write
  * @return OK if success or an error code which specify the type of error
  */
int writeConfig(u16 offset, u8 *data, int len)
{
	int ret = 0;
	u64 final_address = offset + ADDR_CONFIG_OFFSET;

	TS_LOG_INFO( "%s: Starting to write config memory at %08llX ...\n",
		 __func__, final_address);
	ret = fts_writeU8UX(FTS_CMD_CONFIG_W, BITS_16, final_address, data, len);
	if (ret < OK) {
		TS_LOG_ERR("%s: Impossible to write Config Memory... ERROR %08X!\n",__func__, ret);
		return ret;
	}

	TS_LOG_INFO( "%s: Write config memory FINISHED!\n", __func__);
	return OK;
}

/**
  * Disable the interrupt so the ISR of the driver can not be called
  * @return OK if success or an error code which specify the type of error
  */
int fts_disableInterrupt(void)
{
	int ret = 0;
	u8 cmd[3] = {0xA4,0x01,0x00};
	ret = fts_write(cmd, sizeof(cmd));
	if (ret < OK) {
		TS_LOG_ERR( "%s: write failed...ERROR %08X !\n", __func__, ret);
		return ret | ERROR_SET_SCAN_MODE_FAIL;
	}
	return ret;
}




/**
  * Enable the interrupt so the ISR of the driver can be called
  * @return OK if success or an error code which specify the type of error
  */
int fts_enableInterrupt(void)
{
	int ret =0;
	u8 cmd[3] = {0xA4,0x01,0x01};
	ret = fts_write(cmd, sizeof(cmd));
	if (ret < OK) {
		TS_LOG_ERR( "%s: write failed...ERROR %08X !\n", __func__, ret);
		return ret | ERROR_SET_SCAN_MODE_FAIL;
	}
	return ret;
}

/**
  *	Check if there is a crc error in the IC which prevent the fw to run.
  *	@return  OK if no CRC error, or a number >OK according the CRC error
  * found
  */
int fts_crc_check(void)
{
	u8 val = 0;
	u8 crc_status = 0;
	int res = 0;
	u8 error_to_search[6] = { EVT_TYPE_ERROR_CRC_CFG_HEAD,
				  EVT_TYPE_ERROR_CRC_CFG,
				  EVT_TYPE_ERROR_CRC_CX,
				  EVT_TYPE_ERROR_CRC_CX_HEAD,
				  EVT_TYPE_ERROR_CRC_CX_SUB,
				  EVT_TYPE_ERROR_CRC_CX_SUB_HEAD };


	res = fts_writeReadU8UX(FTS_CMD_HW_REG_R, ADDR_SIZE_HW_REG, ADDR_CRC,
				&val, 1, DUMMY_HW_REG);
	/* read 2 bytes because the first one is a dummy byte! */
	if (res < OK) {
		TS_LOG_ERR("%s: Cannot read crc status ERROR %08X\n", __func__, res);
		return res;
	}

	crc_status = val & CRC_MASK;
	if (crc_status != OK) {	/* CRC error if crc_status!=0 */
		TS_LOG_ERR("%s:CRC ERROR = %02X\n",__func__,crc_status);
		return CRC_CODE;
	}

	TS_LOG_INFO("%s: Verifying if Config CRC Error...\n",__func__);
	res = fts_system_reset();
	if (res >= OK) {
		res = pollForErrorType(error_to_search, 2);
		if (res < OK) {
			TS_LOG_ERR( "%s : No Config CRC Error Found!\n", __func__);
			TS_LOG_ERR( "%s : Verifying if Cx CRC Error...\n",__func__);
			res = pollForErrorType(&error_to_search[2], 4);
			if (res < OK) {
				TS_LOG_ERR( "%s: No Cx CRC Error Found!\n", __func__);
				return OK;
			} else {
				TS_LOG_ERR("%s: Cx CRC Error found! CRC ERROR = %02X\n", __func__, res);
				return CRC_CX;
			}
		} else {
			TS_LOG_ERR( "%s: Config CRC Error found! CRC ERROR = %02X\n",__func__, res);
			return CRC_CONFIG;
		}
	} else {
		TS_LOG_ERR("%s: Error while executing system reset! ERROR %08X\n",__func__, res);
		return res;
	}

	return OK;
}

/**
  * Request a host data and use the sync method to understand when the FW load
  * it
  * @param type the type ID of host data to load (@link load_opt Load Host Data
  * Option  @endlink)
  * @return OK if success or an error code which specify the type of error
  */
int requestSyncFrame(u8 type)
{
	u8 request[3] = { FTS_CMD_SYSTEM, SYS_CMD_LOAD_DATA, type };
	u8 readData[DATA_HEADER] = { 0 };
	int ret, retry = 0, retry2 = 0, time_to_count = 0;
	int count =0 , new_count = 0;

	TS_LOG_INFO( "%s: Starting to get a sync frame...\n",  __func__);

	while (retry2 < RETRY_MAX_REQU_DATA) {
		TS_LOG_INFO( "%s: Reading count...\n", __func__);

		ret = fts_writeReadU8UX(FTS_CMD_FRAMEBUFFER_R, BITS_16,
					ADDR_FRAMEBUFFER, readData, DATA_HEADER,
					DUMMY_FRAMEBUFFER);
		if (ret < OK) {
			TS_LOG_INFO("%s: Error while reading count! ERROR %08X\n",
				 __func__, ret | ERROR_REQU_DATA);
			ret |= ERROR_REQU_DATA;
			retry2++;
			continue;
		}

		if (readData[0] != HEADER_SIGNATURE){
			TS_LOG_ERR("%s: Invalid Signature while reading count! ERROR %08X\n", __func__, ret | ERROR_REQU_DATA);
		}

		count = (readData[3] << 8) | readData[2];
		new_count = count;
		TS_LOG_INFO( "%s: Base count = %d\n", __func__, count);

		TS_LOG_INFO( "%s: Requesting frame %02X  attempt = %d\n",__func__,  type, retry2 + 1);

		ret = fts_write(request, sizeof(request));
		if (ret >= OK) {
			TS_LOG_INFO( "%s: Polling for new count...\n", __func__);
			time_to_count = TIMEOUT_REQU_DATA / TIMEOUT_RESOLUTION;
			while (count == new_count && retry < time_to_count) {
				ret = fts_writeReadU8UX(FTS_CMD_FRAMEBUFFER_R,
							BITS_16,
							ADDR_FRAMEBUFFER,
							readData,
							DATA_HEADER,
							DUMMY_FRAMEBUFFER);
				if ((ret >= OK) && (readData[0] == HEADER_SIGNATURE) && (readData[1] == type))
					new_count = ((readData[3] << 8) | readData[2]);
				else
					TS_LOG_INFO("%s: invalid Signature or can not read count... ERROR %08X\n",__func__, ret);
				retry++;
				msleep(TIMEOUT_RESOLUTION);
			}

			if (count == new_count) {
				TS_LOG_ERR("%s: New count not received! ERROR %08X\n",
					  __func__, ERROR_TIMEOUT | ERROR_REQU_DATA);
				ret = ERROR_TIMEOUT | ERROR_REQU_DATA;
			} else {
				TS_LOG_INFO("%s: New count found! count = %d! Frame ready!\n",__func__, new_count);
				return OK;
			}
		}
		retry2++;
	}
	TS_LOG_ERR( "%s: Request Data failed! ERROR %08X\n", __func__, ret);
	return ret;
}


/**
  * Save MP flag value into the flash
  * @param mpflag Value to write in the MP Flag field
  * @return OK if success or an error code which specify the type of error
  */
int saveMpFlag(u8 mpflag)
{
	int ret = 0;

	TS_LOG_ERR( "%s: Saving MP Flag = %02X\n",__func__, mpflag);
	ret = writeSysCmd(SYS_CMD_MP_FLAG, &mpflag, 1);
	if (ret < OK) {
		TS_LOG_ERR( "%s: Error while writing MP flag on ram... ERROR %08X\n",__func__, ret);
		return ret;
	}

	mpflag =  SAVE_PANEL_CONF;
	ret = writeSysCmd(SYS_CMD_SAVE_FLASH, &mpflag, 1);
	if (ret < OK) {
		TS_LOG_ERR( "%s: Error while saving MP flag on flash... ERROR %08X\n",__func__, ret);
		return ret;
	}

	ret = readSysInfo(1);
	if (ret < OK) {
		TS_LOG_ERR( "%s: Error while refreshing SysInfo... ERROR %08X\n", __func__, ret);
		return ret;
	}

	TS_LOG_ERR( "%s: Saving MP Flag OK!\n",  __func__);
	return OK;
}



int calculateCRC8(u8* u8_srcBuff, int size, u8 *crc) {
	u8 u8_remainder =0;
	u8 bit = 0;
	int i=0;
	u8_remainder = 0x00;

	TS_LOG_INFO( "%s: Start CRC computing...\n", __func__);
	if(size!=0 && u8_srcBuff!=NULL){
		// Perform modulo-2 division, a byte at a time.
		for ( i = 0; i < size; i++) {//Bring the next byte into the remainder.
			u8_remainder ^= u8_srcBuff[i]; //Perform modulo-2 division, a bit at a time.
			for (bit = 8; bit > 0; --bit) {		//Try to divide the current data bit.
				if (u8_remainder & (0x1 << 7)){
					u8_remainder = (u8_remainder << 1) ^ 0x9B;
				} else {
					u8_remainder = (u8_remainder << 1);
				}
			}
		} // The final remainder is the CRC result.
		*crc = u8_remainder;
		TS_LOG_INFO( "%s: CRC value = %02X\n", __func__, *crc);
		return OK;
	}else{
		TS_LOG_ERR( "%s: Arguments passed not valid! Data pointer = NULL or size = 0 (%d) ERROR %08X\n",
				__func__,size, ERROR_OP_NOT_ALLOW);
		return ERROR_OP_NOT_ALLOW;
	}
}


int writeLockDownInfo(u8 *data, int size,u8 lock_id)
{
	int ret = 0,i = 0;
	u8 crc_data=0;
	u8 crc_head=0;
	u8 cmd_lockdown_prepare[8]={LOCKDOWN_SIGNATURE,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
	u8 cmd_lockdown_crc[4]={0x00};
	u8 lockdown_save[3]={0xA4,0x00,0x04};
	char temp_buf[100] = { 0 };
	u8 error_to_search[4] = {EVT_TYPE_ERROR_LOCKDOWN_FLASH,EVT_TYPE_ERROR_LOCKDOWN_CRC,
							EVT_TYPE_ERROR_LOCKDOWN_NO_DATA,EVT_TYPE_ERROR_LOCKDOWN_WRITE_FULL};
	char * temp1 = NULL;

	TS_LOG_INFO("%s:enter",__func__);
	if(lock_id< 0x70 || lock_id > 0x77 || size <=0 || size > LOCKDOWN_LENGTH-20){
		TS_LOG_ERR("%s the lock id type is:%02X size:%d not support\n",__func__,lock_id,size);
		return ERROR_LOCKDOWN_CODE;
	}
	temp1 = printHex("Lockdown Code = ",data,size,temp_buf);
	if(temp1){
		TS_LOG_ERR("%s %s\n", __func__, temp1);
	}

	TS_LOG_INFO("%s: Writing Lockdown code into the IC ...\n", __func__);
	fts_disableInterrupt();

	for(i=0;i<3;i++){
		cmd_lockdown_prepare[1]= lock_id;
		ret = calculateCRC8(data,size,&crc_data);
		if (ret < OK) {
			TS_LOG_ERR( "%s : Unable to compute data CRC.. ERROR %08X\n", __func__, ret);
			ret = (ret | ERROR_LOCKDOWN_CODE);
			continue;
		}
		TS_LOG_INFO( "%s: Get the data CRC value:%02X\n",  __func__,crc_data);
		ret = fts_writeU8UX(LOCKDOWN_WRITEREAD_CMD, BITS_16, ADDR_LOCKDOWN,cmd_lockdown_prepare,ARRAY_SIZE(cmd_lockdown_prepare));
		if (ret < OK) {
			TS_LOG_ERR("%s  Unable to write Lockdown data prepare at %d iteration.. ERROR %08X\n", __func__,i, ret);
			ret = (ret | ERROR_LOCKDOWN_CODE);
			continue;
		}
		TS_LOG_INFO( "%s: Compute 8bit header CRC...\n",  __func__);

		cmd_lockdown_crc[0]= (u8)size;
		cmd_lockdown_crc[1] = crc_data;
		cmd_lockdown_crc[2] = lock_id;
		ret = calculateCRC8(cmd_lockdown_crc,3,&crc_head);
		if (ret < OK) {
			TS_LOG_ERR( "%s: Unable to compute head CRC.. ERROR %08X\n",__func__, ret);
			ret = (ret | ERROR_LOCKDOWN_CODE);
			continue;
		}
		cmd_lockdown_crc[3] = crc_head;
		TS_LOG_INFO( "%s: Get the header CRC value:%02X\n",  __func__,crc_head);

		ret = fts_writeU8UX(LOCKDOWN_WRITEREAD_CMD, BITS_16, ADDR_LOCKDOWN+LOCKDOWN_DATA_OFFSET-LOCKDOWN_HEAD_LENGTH,
			cmd_lockdown_crc,ARRAY_SIZE(cmd_lockdown_crc));
		if (ret < OK) {
			TS_LOG_ERR("%sUnable to write Lockdown  head at %d iteration.. ERROR %08X\n", __func__,i, ret);
			ret = (ret | ERROR_LOCKDOWN_CODE);
			continue;
		}
		mdelay(10);
		ret = fts_writeU8UX(LOCKDOWN_WRITEREAD_CMD, BITS_16, ADDR_LOCKDOWN+LOCKDOWN_DATA_OFFSET,data,size);
		if (ret < OK) {
			TS_LOG_ERR("%s: Unable to write Lockdown  head at %d iteration.. ERROR %08X\n", __func__,i, ret);
			ret = (ret | ERROR_LOCKDOWN_CODE);
			continue;
		}
		mdelay(10);
		ret = fts_write(lockdown_save, sizeof(lockdown_save));
		if (ret < OK) {
			TS_LOG_ERR( "%s: fts_write ERROR %08X\n",__func__, ret);
		}
		mdelay(5);
		ret = checkEcho(lockdown_save, sizeof(lockdown_save));
		if(ret<OK){
			TS_LOG_ERR( "%s No Echo received.. ERROR %08X !\n", __func__, ret);
			continue;
		}else{
			TS_LOG_ERR( "%s Echo FOUND... OK!\n", __func__);
			ret = pollForErrorType(error_to_search, sizeof(error_to_search));
			if(ret<OK){
				TS_LOG_ERR( "%s : No Error Found! \n",  __func__);
				ret=OK;
			}else{
				TS_LOG_ERR( "%s: have error when write lockdown ERROR = %02X\n",  __func__, ret);
				ret = ERROR_LOCKDOWN_CODE;
			}
			break;
		}
	}
	if(ret<OK)
		TS_LOG_ERR( "%s end, write lockdown failed\n", __func__);
	else
		TS_LOG_ERR( "%s end, write lockdown success\n", __func__);

	fts_enableInterrupt();
	return ret;
}


int readLockDownInfo(u8 *lockData,u8 lock_id,int size)
{
	int ret =0,i = 0;
	int loaded_cnt=0;
	int loaded_cnt_after=0;
	u8 *temp =NULL;
	u8 cmd_lockdown[3]={0xA4,0x06,0x00};
	char temp_buf[100] = { 0 };
	char * temp1 = NULL;
	TS_LOG_INFO("%s :enter",__func__);
	if(lock_id< 0x70 || lock_id > 0x77 || size <=0 || size > LOCKDOWN_LENGTH-20){
		TS_LOG_ERR("%s the lock id type is:%02X not support\n",__func__,lock_id);
		return ERROR_LOCKDOWN_CODE;
	}

	temp = (u8*)kmalloc(LOCKDOWN_LENGTH*sizeof(u8), GFP_KERNEL);
	if(temp==NULL){
		TS_LOG_ERR("FTS temp alloc  memory failed \n");
		return -ENOMEM;
	}
	memset(temp,0,LOCKDOWN_LENGTH*sizeof(u8));

	fts_disableInterrupt();
	for(i=0;i<3;i++){
		ret = fts_writeReadU8UX(LOCKDOWN_WRITEREAD_CMD, BITS_16, ADDR_LOCKDOWN, temp,LOCKDOWN_HEAD_LENGTH, DUMMY_CONFIG);
		if ( ret < OK) {
			TS_LOG_ERR("%s: error while reading data ERROR %08X \n", __func__, ret);
			goto END;
		}
		loaded_cnt = (int)((temp[3] & 0xFF) << 8) + (temp[2] & 0xFF);
		cmd_lockdown[2] = lock_id;
		ret = fts_write(cmd_lockdown,3);
		if (ret < OK) {
			TS_LOG_ERR( "%s: fts_write ERROR %08X\n",__func__, ret);
		}
		mdelay(10);
		ret = checkEcho(cmd_lockdown,3);
		if(ret<OK){
			TS_LOG_ERR( "%s No Echo received.. ERROR %08X !\n", __func__, ret);
			continue;
		}else{
			TS_LOG_ERR( "%s Echo FOUND... OK!\n", __func__);
		}
		ret = fts_writeReadU8UX(LOCKDOWN_WRITEREAD_CMD, BITS_16, ADDR_LOCKDOWN, temp, size+LOCKDOWN_DATA_OFFSET, DUMMY_CONFIG);
		if ( ret < OK) {
			TS_LOG_ERR("%s: error while reading data ERROR %08X \n",  __func__, ret);
			goto END;
		}

		loaded_cnt_after = (int)((temp[3] & 0xFF) << 8) + (temp[2] & 0xFF);
		if(temp[4] == EVT_TYPE_ERROR_LOCKDOWN_FLASH || temp[4] == EVT_TYPE_ERROR_LOCKDOWN_NO_DATA){
			TS_LOG_ERR("%s: can not read the lockdown code ERROR type:%02X\n",  __func__, temp[4]);
			ret = ERROR_LOCKDOWN_CODE;
			goto END;
		}

		TS_LOG_ERR("%s signature:%02X id:%02X %02X beforecnt:%d,aftercnt:%d\n", __func__,temp[0],temp[1],lock_id,loaded_cnt,loaded_cnt_after);
		if( loaded_cnt_after == loaded_cnt + 1){
			ret = OK;
			memcpy(lockData, &temp[LOCKDOWN_DATA_OFFSET], size);
			break;
		}

	}
	temp1 =  printHex("Lockdown Code = ",lockData,size,temp_buf);
	if(temp1){
		TS_LOG_ERR("%s %s\n", __func__,temp1);
	}

END:
	fts_enableInterrupt();
	if(temp){
		kfree(temp);
	}
	return ret;
}
