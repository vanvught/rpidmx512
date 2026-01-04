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
/* Copyright (C) 2018-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include <cstdint>

#include "spi/spi_flash.h"
#include "spi_flash_internal.h"
 #include "firmware/debug/debug_debug.h"

struct WinbondSpiFlashParams
{
    const uint16_t kId;
    const uint16_t kNrBlocks;
    const char* const kName;
};

static constexpr struct WinbondSpiFlashParams kWinbondSpiFlashTable[] = {
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
		"W25Q80BL",
	},
	{
		0x4015,
		32,
		"W25Q16CL",
	},
	{
		0x4016,
		64,
		"W25Q32BV",
	},
	{
		0x4017,
		128,
		"W25Q64CV",
	},
	{
		0x4018,
		256,
		"W25Q128BV",
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
	}
};

bool SpiFlashProbeWinbond(struct SpiFlashInfo *flash, uint8_t *idcode) {
	const struct WinbondSpiFlashParams *params;
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(kWinbondSpiFlashTable); i++) {
		params = &kWinbondSpiFlashTable[i];
		if (params->kId == ((idcode[1] << 8) | idcode[2])) {
			break;
		}
	}

	if (i == ARRAY_SIZE(kWinbondSpiFlashTable)) {
		DEBUG_PRINTF("Unsupported Winbond ID %02x%02x", idcode[1], idcode[2]);
		return false;
	}

	flash->name = params->kName;
	flash->size = 16U * spi::flash::SECTOR_SIZE * params->kNrBlocks;

	return true;
}
