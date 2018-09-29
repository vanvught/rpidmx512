/**
 * @file macronix.c
 *
 */
/*
 * Original code : https://github.com/martinezjavier/u-boot/blob/master/drivers/mtd/spi/macronix.c
 */
/* Copyright (C) 2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

extern int spi_flash_cmd_write_status(uint8_t sr);

struct macronix_spi_flash_params {
	uint16_t idcode;
	uint16_t nr_blocks;
	const char *name;
};

static const struct macronix_spi_flash_params macronix_spi_flash_table[] = {
	{
		.idcode = 0x2013,
		.nr_blocks = 8,
		.name = "MX25L4005",
	},
	{
		.idcode = 0x2014,
		.nr_blocks = 16,
		.name = "MX25L8005",
	},
	{
		.idcode = 0x2015,
		.nr_blocks = 32,
		.name = "MX25L1605D",
	},
	{
		.idcode = 0x2016,
		.nr_blocks = 64,
		.name = "MX25L3205D",
	},
	{
		.idcode = 0x2017,
		.nr_blocks = 128,
		.name = "MX25L6405D",
	},
	{
		.idcode = 0x2018,
		.nr_blocks = 256,
		.name = "MX25L12805D",
	},
	{
		.idcode = 0x2618,
		.nr_blocks = 256,
		.name = "MX25L12855E",
	},
};

int spi_flash_probe_macronix(struct spi_flash *flash, uint8_t *idcode) {
	const struct macronix_spi_flash_params *params;
	unsigned int i;
	uint32_t id = idcode[2] | idcode[1] << 8;

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
