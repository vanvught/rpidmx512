/**
 * @file winbond.cpp
 *
 */
/*
 * Copyright 2008, Network Appliance Inc.
 * Author: Jason McMullan <mcmullan <at> netapp.com>
 * Licensed under the GPL-2 or later.
 */
/*
 * Original code : https://github.com/martinezjavier/u-boot/blob/master/drivers/mtd/spi/winbond.c
 */
/* Copyright (C) 2018-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <stdint.h>

#include "debug.h"
#include "spi_flash_internal.h"

struct winbond_spi_flash_params {
	uint16_t	id;
	uint16_t	nr_blocks;
	const char	*name;
};

static const struct winbond_spi_flash_params winbond_spi_flash_table[] = {
	{
		0x2014,
		16,
		"W25P80",
	},
	{
		0x2015,
		32,
		"W25P16",
	},
	{
		0x2016,
		64,
		"W25P32",
	},
	{
		0x3013,
		8,
		"W25X40",
	},
	{
		0x3015,
		32,
		"W25X16",
	},
	{
		0x3016,
		64,
		"W25X32",
	},
	{
		0x3017,
		128,
		"W25X64",
	},
	{
		0x4014,
		16,
		"W25Q80BL/W25Q80BV",
	},
	{
		0x4015,
		32,
		"W25Q16CL/W25Q16DV",
	},
	{
		0x4016,
		64,
		"W25Q32BV/W25Q32FV_SPI",
	},
	{
		0x4017,
		128,
		"W25Q64CV/W25Q64FV_SPI",
	},
	{
		0x4018,
		256,
		"W25Q128BV/W25Q128FV_SPI",
	},
	{
		0x4019,
		512,
		"W25Q256",
	},
	{
		0x5014,
		16,
		"W25Q80BW",
	},
	{
		0x6015,
		32,
		"W25Q16DW",
	},
	{
		0x6016,
		64,
		"W25Q32DW/W25Q32FV_QPI",
	},
	{
		0x6017,
		128,
		"W25Q64DW/W25Q64FV_QPI",
	},
	{
		0x6018,
		256,
		"W25Q128FW/W25Q128FV_QPI",
	},
};

int spi_flash_probe_winbond(struct spi_flash *flash, uint8_t *idcode) {
	const struct winbond_spi_flash_params *params;
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(winbond_spi_flash_table); i++) {
		params = &winbond_spi_flash_table[i];
		if (params->id == ((idcode[1] << 8) | idcode[2])) {
			break;
		}
	}

	if (i == ARRAY_SIZE(winbond_spi_flash_table)) {
		DEBUG_PRINTF("Unsupported Winbond ID %02x%02x", idcode[1], idcode[2]);
		return -1;
	}

	flash->name = params->name;
	flash->page_size = 256;
	flash->sector_size = (idcode[1] == 0x20) ? 65536 : 4096;
	flash->size = 4096U * 16U * params->nr_blocks;

	return 0;
}
