/**
 * @file spi_flash.h
 *
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

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>

#include "bcm2835.h"

#include "../spi_flash_internal.h"
#include "debug.h"

int spi_init(void) {

	if (getuid() != 0) {
		fprintf(stderr, "Error: Not started with 'root'\n");
		return -2;
	}

	if (bcm2835_init() == 0) {
		fprintf(stderr, "Error: Not able to init the bmc2835 library\n");
		return -1;
	}

	bcm2835_spi_begin();
	bcm2835_spi_chipSelect(BCM2835_SPI_CS_NONE);
	bcm2835_spi_setClockDivider((uint16_t) ((uint32_t) BCM2835_CORE_CLK_HZ / (uint32_t) SPI_XFER_SPEED_HZ));
	bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);
	bcm2835_gpio_fsel(RPI_V2_GPIO_P1_24, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_set(RPI_V2_GPIO_P1_24);

	return 0;
}

int spi_xfer(unsigned bitlen, const void *dout, void *din, unsigned long flags) {
	uint32_t len = bitlen / 8;

	if (bitlen % 8) {
		DEBUG_PUTS("non byte-aligned SPI transfer");
		return -3;
	}

	if (flags & SPI_XFER_BEGIN) {
		bcm2835_gpio_clr(RPI_V2_GPIO_P1_24);
	}

	if (din == 0) {
		//DEBUG_PRINTF("Write only:%d", len);
		bcm2835_spi_writenb((char *)dout, len);
	} else if (dout == 0) {
		//DEBUG_PRINTF("Read only:%d", len);
		bcm2835_spi_transfern(din, len);
	}
	else {
		//DEBUG_PUTS("Transfer");
		bcm2835_spi_transfernb((char *)dout, (char *)din, len);
	}

	if (flags & SPI_XFER_END) {
		bcm2835_gpio_set(RPI_V2_GPIO_P1_24);
	}

	return 0;
}
