/**
 * @file macronix.cpp
 *
 */
/*
 * Copyright 2009(C) Marvell International Ltd. and its affiliates
 * Prafulla Wadaskar <prafulla@marvell.com>
 *
 * Based on drivers/mtd/spi/stmicro.c
 *
 * Copyright 2008, Network Appliance Inc.
 * Jason McMullan <mcmullan@netapp.com>
 *
 * Copyright (C) 2004-2007 Freescale Semiconductor, Inc.
 * TsiChung Liew (Tsi-Chung.Liew@freescale.com)
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
/*
 * Original code : https://github.com/martinezjavier/u-boot/blob/master/drivers/mtd/spi/macronix.c
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

struct MacronixSpiFlashParams
{
    const uint16_t kIdcode;
    const uint16_t kNrBlocks;
    const char* const kName;
};

static constexpr struct MacronixSpiFlashParams kMacronixSpiFlashTable[] = {
	{
		0x2013,
		8,
		"MX25L4005",
	},
	{
		0x2014,
		16,
		"MX25L8005",
	},
	{
		0x2015,
		32,
		"MX25L1605D",
	},
	{
		0x2016,
		64,
		"MX25L3205D",
	},
	{
		0x2017,
		128,
		"MX25L6405D",
	},
	{
		0x2018,
		256,
		"MX25L12805D",
	},
	{
		0x2618,
		256,
		"MX25L12855E",
	},
};

bool SpiFlashProbeMacronix(struct SpiFlashInfo *flash, uint8_t *idcode) {
	const struct MacronixSpiFlashParams *params;
	unsigned int i;
	uint32_t id = idcode[2] | static_cast<uint32_t>(idcode[1] << 8);

	for (i = 0; i < ARRAY_SIZE(kMacronixSpiFlashTable); i++) {
		params = &kMacronixSpiFlashTable[i];

		if (params->kIdcode == id) {
			break;
		}
	}

	if (i == ARRAY_SIZE(kMacronixSpiFlashTable)) {
		DEBUG_PRINTF("Unsupported Macronix ID %04x\n", id);
		return false;
	}

	flash->name = params->kName;
	flash->size = 16U * spi::flash::SECTOR_SIZE * params->kNrBlocks;

	/* Clear BP# bits for read-only flash */
	spi_flash_cmd_write_status(0);

	return true;
}
