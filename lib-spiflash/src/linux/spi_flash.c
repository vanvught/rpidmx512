/**
 * @file spi_flash.h
 *
 */
/* Copyright (C) 2018-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#if defined (RASPPI)
#include <stdio.h>
#include <stdint.h>

#include "bcm2835.h"

#include "../spi_flash_internal.h"

int spi_init(void) {
	bcm2835_spi_begin();
	bcm2835_spi_chipSelect(BCM2835_SPI_CS_NONE);
	bcm2835_spi_setClockDivider((uint16_t) ((uint32_t) BCM2835_CORE_CLK_HZ / (uint32_t) SPI_XFER_SPEED_HZ));
	bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);
	bcm2835_gpio_fsel(RPI_V2_GPIO_P1_24, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_set(RPI_V2_GPIO_P1_24);
	return 0;
}

int spi_xfer(unsigned len, const void *dout, void *din, unsigned long flags) {
	if (flags & SPI_XFER_BEGIN) {
		bcm2835_gpio_clr(RPI_V2_GPIO_P1_24);
	}

	if (din == 0) {
		bcm2835_spi_writenb((char *) dout, len);
	} else if (dout == 0) {
		bcm2835_spi_transfern(din, len);
	} else {
		bcm2835_spi_transfernb((char *) dout, (char *) din, len);
	}

	if (flags & SPI_XFER_END) {
		bcm2835_gpio_set(RPI_V2_GPIO_P1_24);
	}

	return 0;
}
#else
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

#include "debug.h"

static FILE *file = NULL;

#define FLASH_SECTOR_SIZE	4096
#define FLASH_SIZE			(512 * FLASH_SECTOR_SIZE)

#define FLASH_FILE_NAME		"spiflash.bin"

int spi_flash_probe(__attribute__((unused)) unsigned int cs, __attribute__((unused)) unsigned int max_hz, __attribute__((unused)) unsigned int spi_mode) {
	DEBUG_ENTRY

	if ((file = fopen(FLASH_FILE_NAME, "r+")) == NULL) {
		perror("fopen r+");
		file = fopen(FLASH_FILE_NAME, "w+");
		int i;
		for (i = 0; i < FLASH_SIZE; i++) {
			if (fputc(0xFF, file) == EOF) {
				perror("fputc(0xFF, file)");
				DEBUG_EXIT
				return -1;
			}
		}

		if (fflush(file) != 0) {
			perror("fflush");
		}
	}

	DEBUG_EXIT
	return 0;
}

uint32_t spi_flash_get_sector_size(void) {
	return FLASH_SECTOR_SIZE;
}

const char *spi_flash_get_name(void) {
	return "SPI Flash None Driver";
}

uint32_t spi_flash_get_size(void) {
	return FLASH_SIZE;
}

int spi_flash_cmd_erase(uint32_t offset, size_t len) {
	DEBUG_ENTRY

	DEBUG_PRINTF("offset=%d, len=%d", offset, (int) len);

	if (offset % FLASH_SECTOR_SIZE || len % FLASH_SECTOR_SIZE) {
		DEBUG_PUTS("Erase offset/length not multiple of erase size");
		DEBUG_EXIT
		return -1;
	}

	if (fseek(file, offset, SEEK_SET) != 0) {
		perror("fseek");
		DEBUG_EXIT
		return -1;
	}

	size_t i;
	for (i = 0; i < len; i++) {
		if (fputc(0xFF, file) == EOF) {
			perror("fputc(0xFF, file)");
			DEBUG_EXIT
			return -1;
		}
	}

	if (fflush(file) != 0) {
		perror("fflush");
	}

	sync();

	DEBUG_EXIT
	return 0;
}

int spi_flash_cmd_write_multi(uint32_t offset, size_t len, const void *buf) {
	DEBUG_ENTRY

	assert(file != NULL);

	DEBUG_PRINTF("offset=%d, len=%d", (int) offset, (int) len);

	if (fseek(file, offset, SEEK_SET) != 0) {
		perror("fseek");
		DEBUG_EXIT
		return -1;
	}

	if (fwrite(buf, 1, len, file) != len) {
		perror("fwrite");
		DEBUG_EXIT
		return -1;
	}

	if (fflush(file) != 0) {
		perror("fflush");
	}

	sync();

	DEBUG_EXIT
	return 0;
}

int spi_flash_cmd_read_fast(uint32_t offset, size_t len, void *data) {
	DEBUG_ENTRY

	assert(file != NULL);

	DEBUG_PRINTF("offset=%d, len=%d", (int) offset, (int) len);

	if (fseek(file, offset, SEEK_SET) != 0) {
		perror("fseek");
		DEBUG_EXIT
		return -1;
	}

	if (fread(data, 1, len, file) != len) {
		perror("fread");
		DEBUG_EXIT
		return -1;
	}

	DEBUG_EXIT
	return 0;
}
#endif
