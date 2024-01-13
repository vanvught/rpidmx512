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

#include <cstdint>

#include "debug.h"
#include "spi_flash_internal.h"

extern int spi_flash_cmd_write_status(uint8_t sr);

struct macronix_spi_flash_params {
	uint16_t idcode;
	uint16_t nr_blocks;
	const char *name;
};

static constexpr struct macronix_spi_flash_params macronix_spi_flash_table[] = {
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

int spi_flash_probe_macronix(struct spi_flash *flash, uint8_t *idcode) {
	const struct macronix_spi_flash_params *params;
	unsigned int i;
	uint32_t id = idcode[2] | static_cast<uint32_t>(idcode[1] << 8);

	for (i = 0; i < ARRAY_SIZE(macronix_spi_flash_table); i++) {
		params = &macronix_spi_flash_table[i];

		if (params->idcode == id) {
			break;
		}
	}

	if (i == ARRAY_SIZE(macronix_spi_flash_table)) {
		DEBUG_PRINTF("Unsupported Macronix ID %04x\n", id);
		return -1;
	}

	flash->name = params->name;
	flash->page_size = 256;
	flash->sector_size = 4096;
	flash->size = 16 * flash->sector_size * params->nr_blocks;

	/* Clear BP# bits for read-only flash */
	spi_flash_cmd_write_status(0);

	return 0;
}
