/*
 * Copyright (C) 2010 - 2017 Novatek, Inc.
 *
 * $Revision: 20544 $
 * $Date: 2017-12-20 11:08:15 +0800 (週三, 20 十二月 2017) $
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 */

struct nvt_ts_mem_map {
	uint32_t EVENT_BUF_ADDR;
	uint32_t RAW_PIPE0_ADDR;
	uint32_t RAW_PIPE0_Q_ADDR;
	uint32_t RAW_PIPE1_ADDR;
	uint32_t RAW_PIPE1_Q_ADDR;
	uint32_t BASELINE_ADDR;
	uint32_t BASELINE_Q_ADDR;
	uint32_t BASELINE_BTN_ADDR;
	uint32_t BASELINE_BTN_Q_ADDR;
	uint32_t DIFF_PIPE0_ADDR;
	uint32_t DIFF_PIPE0_Q_ADDR;
	uint32_t DIFF_PIPE1_ADDR;
	uint32_t DIFF_PIPE1_Q_ADDR;
	uint32_t RAW_BTN_PIPE0_ADDR;
	uint32_t RAW_BTN_PIPE0_Q_ADDR;
	uint32_t RAW_BTN_PIPE1_ADDR;
	uint32_t RAW_BTN_PIPE1_Q_ADDR;
	uint32_t DIFF_BTN_PIPE0_ADDR;
	uint32_t DIFF_BTN_PIPE0_Q_ADDR;
	uint32_t DIFF_BTN_PIPE1_ADDR;
	uint32_t DIFF_BTN_PIPE1_Q_ADDR;
	uint32_t READ_FLASH_CHECKSUM_ADDR;
	uint32_t RW_FLASH_DATA_ADDR;
	/* Phase 2 Host Download */
	uint32_t BOOT_RDY_ADDR;
	/* BLD CRC */
	uint32_t BLD_LENGTH_ADDR;
	uint32_t ILM_LENGTH_ADDR;
	uint32_t DLM_LENGTH_ADDR;
	uint32_t BLD_DES_ADDR;
	uint32_t ILM_DES_ADDR;
	uint32_t DLM_DES_ADDR;
	uint32_t G_ILM_CHECKSUM_ADDR;
	uint32_t G_DLM_CHECKSUM_ADDR;
	uint32_t R_ILM_CHECKSUM_ADDR;
	uint32_t R_DLM_CHECKSUM_ADDR;
	uint32_t BLD_CRC_EN_ADDR;
	uint32_t DMA_CRC_EN_ADDR;
	uint32_t BLD_ILM_DLM_CRC_ADDR;
	uint32_t DMA_CRC_FLAG_ADDR;
//#if NVT_QEEXO_EARSENSE
	uint32_t DIFF_QEEXO_DOWN_ADDR;
	uint32_t DIFF_QEEXO_UP_ADDR;
//#endif
};


static const struct nvt_ts_mem_map NT36672A_memory_map = {
	.EVENT_BUF_ADDR           = 0x21C00,
	.RAW_PIPE0_ADDR           = 0x20000,
	.RAW_PIPE0_Q_ADDR         = 0,
	.RAW_PIPE1_ADDR           = 0x23000,
	.RAW_PIPE1_Q_ADDR         = 0,
	.BASELINE_ADDR            = 0x20BFC,
	.BASELINE_Q_ADDR          = 0,
	.BASELINE_BTN_ADDR        = 0x23BFC,
	.BASELINE_BTN_Q_ADDR      = 0,
	.DIFF_PIPE0_ADDR          = 0x206DC,
	.DIFF_PIPE0_Q_ADDR        = 0,
	.DIFF_PIPE1_ADDR          = 0x236DC,
	.DIFF_PIPE1_Q_ADDR        = 0,
	.RAW_BTN_PIPE0_ADDR       = 0x20510,
	.RAW_BTN_PIPE0_Q_ADDR     = 0,
	.RAW_BTN_PIPE1_ADDR       = 0x23510,
	.RAW_BTN_PIPE1_Q_ADDR     = 0,
	.DIFF_BTN_PIPE0_ADDR      = 0x20BF0,
	.DIFF_BTN_PIPE0_Q_ADDR    = 0,
	.DIFF_BTN_PIPE1_ADDR      = 0x23BF0,
	.DIFF_BTN_PIPE1_Q_ADDR    = 0,
	.READ_FLASH_CHECKSUM_ADDR = 0x24000,
	.RW_FLASH_DATA_ADDR       = 0x24002,
	/* Phase 2 Host Download */
	.BOOT_RDY_ADDR            = 0x3F10D,
	/* BLD CRC */
	.BLD_LENGTH_ADDR          = 0x3F10E,	//0x3F10E ~ 0x3F10F	(2 bytes)
	.ILM_LENGTH_ADDR          = 0x3F118,	//0x3F118 ~ 0x3F119	(2 bytes)
	.DLM_LENGTH_ADDR          = 0x3F130,	//0x3F130 ~ 0x3F131	(2 bytes)
	.BLD_DES_ADDR             = 0x3F114,	//0x3F114 ~ 0x3F116	(3 bytes)
	.ILM_DES_ADDR             = 0x3F128,	//0x3F128 ~ 0x3F12A	(3 bytes)
	.DLM_DES_ADDR             = 0x3F12C,	//0x3F12C ~ 0x3F12E	(3 bytes)
	.G_ILM_CHECKSUM_ADDR      = 0x3F100,	//0x3F100 ~ 0x3F103	(4 bytes)
	.G_DLM_CHECKSUM_ADDR      = 0x3F104,	//0x3F104 ~ 0x3F107	(4 bytes)
	.R_ILM_CHECKSUM_ADDR      = 0x3F120,	//0x3F120 ~ 0x3F123 (4 bytes)
	.R_DLM_CHECKSUM_ADDR      = 0x3F124,	//0x3F124 ~ 0x3F127 (4 bytes)
	.BLD_CRC_EN_ADDR          = 0x3F30E,
	.DMA_CRC_EN_ADDR          = 0x3F132,
	.BLD_ILM_DLM_CRC_ADDR     = 0x3F133,
	.DMA_CRC_FLAG_ADDR        = 0x3F134,
//#if NVT_QEEXO_EARSENSE
    .DIFF_QEEXO_DOWN_ADDR     = 0x22550,
    .DIFF_QEEXO_UP_ADDR       = 0x227D8,
//#endif
};

static const struct nvt_ts_mem_map NT36772_memory_map = {
	.EVENT_BUF_ADDR           = 0x11E00,
	.RAW_PIPE0_ADDR           = 0x10000,
	.RAW_PIPE0_Q_ADDR         = 0,
	.RAW_PIPE1_ADDR           = 0x12000,
	.RAW_PIPE1_Q_ADDR         = 0,
	.BASELINE_ADDR            = 0x10E70,
	.BASELINE_Q_ADDR          = 0,
	.BASELINE_BTN_ADDR        = 0x12E70,
	.BASELINE_BTN_Q_ADDR      = 0,
	.DIFF_PIPE0_ADDR          = 0x10830,
	.DIFF_PIPE0_Q_ADDR        = 0,
	.DIFF_PIPE1_ADDR          = 0x12830,
	.DIFF_PIPE1_Q_ADDR        = 0,
	.RAW_BTN_PIPE0_ADDR       = 0x10E60,
	.RAW_BTN_PIPE0_Q_ADDR     = 0,
	.RAW_BTN_PIPE1_ADDR       = 0x12E60,
	.RAW_BTN_PIPE1_Q_ADDR     = 0,
	.DIFF_BTN_PIPE0_ADDR      = 0x10E68,
	.DIFF_BTN_PIPE0_Q_ADDR    = 0,
	.DIFF_BTN_PIPE1_ADDR      = 0x12E68,
	.DIFF_BTN_PIPE1_Q_ADDR    = 0,
	.READ_FLASH_CHECKSUM_ADDR = 0x14000,
	.RW_FLASH_DATA_ADDR       = 0x14002,
};

static const struct nvt_ts_mem_map NT36525_memory_map = {
	.EVENT_BUF_ADDR           = 0x11A00,
	.RAW_PIPE0_ADDR           = 0x10000,
	.RAW_PIPE0_Q_ADDR         = 0,
	.RAW_PIPE1_ADDR           = 0x12000,
	.RAW_PIPE1_Q_ADDR         = 0,
	.BASELINE_ADDR            = 0x10B08,
	.BASELINE_Q_ADDR          = 0,
	.BASELINE_BTN_ADDR        = 0x12B08,
	.BASELINE_BTN_Q_ADDR      = 0,
	.DIFF_PIPE0_ADDR          = 0x1064C,
	.DIFF_PIPE0_Q_ADDR        = 0,
	.DIFF_PIPE1_ADDR          = 0x1264C,
	.DIFF_PIPE1_Q_ADDR        = 0,
	.RAW_BTN_PIPE0_ADDR       = 0x10634,
	.RAW_BTN_PIPE0_Q_ADDR     = 0,
	.RAW_BTN_PIPE1_ADDR       = 0x12634,
	.RAW_BTN_PIPE1_Q_ADDR     = 0,
	.DIFF_BTN_PIPE0_ADDR      = 0x10AFC,
	.DIFF_BTN_PIPE0_Q_ADDR    = 0,
	.DIFF_BTN_PIPE1_ADDR      = 0x12AFC,
	.DIFF_BTN_PIPE1_Q_ADDR    = 0,
	.READ_FLASH_CHECKSUM_ADDR = 0x14000,
	.RW_FLASH_DATA_ADDR       = 0x14002,
};

static const struct nvt_ts_mem_map NT36870_memory_map = {
	.EVENT_BUF_ADDR           = 0x25000,
	.RAW_PIPE0_ADDR           = 0x20000,
	.RAW_PIPE0_Q_ADDR         = 0x204C8,
	.RAW_PIPE1_ADDR           = 0x23000,
	.RAW_PIPE1_Q_ADDR         = 0x234C8,
	.BASELINE_ADDR            = 0x21350,
	.BASELINE_Q_ADDR          = 0x21818,
	.BASELINE_BTN_ADDR        = 0x24350,
	.BASELINE_BTN_Q_ADDR      = 0x24358,
	.DIFF_PIPE0_ADDR          = 0x209B0,
	.DIFF_PIPE0_Q_ADDR        = 0x20E78,
	.DIFF_PIPE1_ADDR          = 0x239B0,
	.DIFF_PIPE1_Q_ADDR        = 0x23E78,
	.RAW_BTN_PIPE0_ADDR       = 0x20990,
	.RAW_BTN_PIPE0_Q_ADDR     = 0x20998,
	.RAW_BTN_PIPE1_ADDR       = 0x23990,
	.RAW_BTN_PIPE1_Q_ADDR     = 0x23998,
	.DIFF_BTN_PIPE0_ADDR      = 0x21340,
	.DIFF_BTN_PIPE0_Q_ADDR    = 0x21348,
	.DIFF_BTN_PIPE1_ADDR      = 0x24340,
	.DIFF_BTN_PIPE1_Q_ADDR    = 0x24348,
	.READ_FLASH_CHECKSUM_ADDR = 0x24000,
	.RW_FLASH_DATA_ADDR       = 0x24002,
};

static const struct nvt_ts_mem_map NT36676F_memory_map = {
	.EVENT_BUF_ADDR           = 0x11A00,
	.RAW_PIPE0_ADDR           = 0x10000,
	.RAW_PIPE0_Q_ADDR         = 0,
	.RAW_PIPE1_ADDR           = 0x12000,
	.RAW_PIPE1_Q_ADDR         = 0,
	.BASELINE_ADDR            = 0x10B08,
	.BASELINE_Q_ADDR          = 0,
	.BASELINE_BTN_ADDR        = 0x12B08,
	.BASELINE_BTN_Q_ADDR      = 0,
	.DIFF_PIPE0_ADDR          = 0x1064C,
	.DIFF_PIPE0_Q_ADDR        = 0,
	.DIFF_PIPE1_ADDR          = 0x1264C,
	.DIFF_PIPE1_Q_ADDR        = 0,
	.RAW_BTN_PIPE0_ADDR       = 0x10634,
	.RAW_BTN_PIPE0_Q_ADDR     = 0,
	.RAW_BTN_PIPE1_ADDR       = 0x12634,
	.RAW_BTN_PIPE1_Q_ADDR     = 0,
	.DIFF_BTN_PIPE0_ADDR      = 0x10AFC,
	.DIFF_BTN_PIPE0_Q_ADDR    = 0,
	.DIFF_BTN_PIPE1_ADDR      = 0x12AFC,
	.DIFF_BTN_PIPE1_Q_ADDR    = 0,
	.READ_FLASH_CHECKSUM_ADDR = 0x14000,
	.RW_FLASH_DATA_ADDR       = 0x14002,
};

#define NVT_ID_BYTE_MAX 6
struct nvt_ts_trim_id_table {
	uint8_t id[NVT_ID_BYTE_MAX];
	uint8_t mask[NVT_ID_BYTE_MAX];
	const struct nvt_ts_mem_map *mmap;
	uint8_t carrier_system;
};

static const struct nvt_ts_trim_id_table trim_id_table[] = {
	{.id = {0x0A, 0xFF, 0xFF, 0x72, 0x65, 0x03}, .mask = {1, 0, 0, 1, 1, 1},
		.mmap = &NT36672A_memory_map, .carrier_system = 0},
	{.id = {0x0A, 0xFF, 0xFF, 0x72, 0x66, 0x03}, .mask = {1, 0, 0, 1, 1, 1},
		.mmap = &NT36672A_memory_map, .carrier_system = 0},
	{.id = {0x0A, 0xFF, 0xFF, 0x82, 0x66, 0x03}, .mask = {1, 0, 0, 1, 1, 1},
		.mmap = &NT36672A_memory_map, .carrier_system = 0},
	{.id = {0x0A, 0xFF, 0xFF, 0x70, 0x66, 0x03}, .mask = {1, 0, 0, 1, 1, 1},
		.mmap = &NT36672A_memory_map, .carrier_system = 0},
	{.id = {0x0B, 0xFF, 0xFF, 0x70, 0x66, 0x03}, .mask = {1, 0, 0, 1, 1, 1},
		.mmap = &NT36672A_memory_map, .carrier_system = 0},
	{.id = {0x0A, 0xFF, 0xFF, 0x72, 0x67, 0x03}, .mask = {1, 0, 0, 1, 1, 1},
		.mmap = &NT36672A_memory_map, .carrier_system = 0},
	{.id = {0x55, 0x00, 0xFF, 0x00, 0x00, 0x00}, .mask = {1, 1, 0, 1, 1, 1},
		.mmap = &NT36772_memory_map, .carrier_system = 0},
	{.id = {0x55, 0x72, 0xFF, 0x00, 0x00, 0x00}, .mask = {1, 1, 0, 1, 1, 1},
		.mmap = &NT36772_memory_map, .carrier_system = 0},
	{.id = {0xAA, 0x00, 0xFF, 0x00, 0x00, 0x00}, .mask = {1, 1, 0, 1, 1, 1},
		.mmap = &NT36772_memory_map, .carrier_system = 0},
	{.id = {0xAA, 0x72, 0xFF, 0x00, 0x00, 0x00}, .mask = {1, 1, 0, 1, 1, 1},
		.mmap = &NT36772_memory_map, .carrier_system = 0},
	{.id = {0xFF, 0xFF, 0xFF, 0x72, 0x67, 0x03}, .mask = {0, 0, 0, 1, 1, 1},
		.mmap = &NT36772_memory_map, .carrier_system = 0},
	{.id = {0xFF, 0xFF, 0xFF, 0x70, 0x66, 0x03}, .mask = {0, 0, 0, 1, 1, 1},
		.mmap = &NT36772_memory_map, .carrier_system = 0},
	{.id = {0xFF, 0xFF, 0xFF, 0x70, 0x67, 0x03}, .mask = {0, 0, 0, 1, 1, 1},
		.mmap = &NT36772_memory_map, .carrier_system = 0},
	{.id = {0xFF, 0xFF, 0xFF, 0x72, 0x66, 0x03}, .mask = {0, 0, 0, 1, 1, 1},
		.mmap = &NT36772_memory_map, .carrier_system = 0},
	{.id = {0xFF, 0xFF, 0xFF, 0x25, 0x65, 0x03}, .mask = {0, 0, 0, 1, 1, 1},
		.mmap = &NT36525_memory_map, .carrier_system = 0},
	{.id = {0xFF, 0xFF, 0xFF, 0x70, 0x68, 0x03}, .mask = {0, 0, 0, 1, 1, 1},
		.mmap = &NT36870_memory_map, .carrier_system = 1},
	{.id = {0xFF, 0xFF, 0xFF, 0x76, 0x66, 0x03}, .mask = {0, 0, 0, 1, 1, 1},
		.mmap = &NT36676F_memory_map, .carrier_system = 0}
};
