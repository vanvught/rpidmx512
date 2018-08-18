/**
 * @file bw_spi.c
 *
 */
/* Copyright (C) 2017-2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include <stddef.h>
#ifndef NDEBUG
 #include <stdio.h>
#endif

#include "bob.h"

#include "bw.h"

void bw_spi_read_id(const device_info_t *device_info, char *id) {
	char buffer[BW_ID_STRING_LENGTH + 2];
	char *s1, *s2;
	int n = BW_ID_STRING_LENGTH;

	if (id == NULL) {
		return;
	}

	buffer[0] = (char) (device_info->slave_address | 1);
	buffer[1] = (char) BW_PORT_READ_ID_STRING;

	if (device_info->chip_select >= SPI_CS2) {
		bcm2835_aux_spi_setClockDivider(bcm2835_aux_spi_CalcClockDivider(32000));
		bcm2835_aux_spi_transfern(buffer, sizeof(buffer) / sizeof(buffer[0]));
	} else {
		bcm2835_spi_setClockDivider(5000);
		bcm2835_spi_setChipSelectPolarity(device_info->chip_select, LOW);
		bcm2835_spi_chipSelect(device_info->chip_select);
		bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);
		bcm2835_spi_transfern(buffer, sizeof(buffer) / sizeof(buffer[0]));
	}

	s1 = (char *) id;
	s2 = (char *) &buffer[2];

	while (n > 0 && *s2 != '\0') {
		*s1++ = *s2++;
		--n;
	}

	while (n > 0) {
		*s1++ = '\0';
		--n;
	}

#ifndef NDEBUG
	printf("%s:%s device_info->chip_select=%d, id=%s\n", __FILE__, __FUNCTION__, (int) device_info->chip_select, id);
#endif
}
