/**
 * @file bw_spi.c
 *
 */
/* Copyright (C) 2014 by Arjan van Vught <pm @ http://www.raspberrypi.org/forum/>
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

#ifdef __AVR_ARCH__
#include <avr_spi.h>
#else
#include <bcm2835.h>
#ifdef BARE_METAL
#include <bcm2835_spi.h>
#endif
#endif
#include <device_info.h>
#include <bw.h>

extern int printf(const char *format, ...);

/**
 * @ingroup SPI
 * Prints the identification string.
 * @param device_info
 */
void bw_spi_read_id(const device_info_t *device_info) {
	char buf[BW_ID_STRING_LENGTH];
	buf[0] = device_info->slave_address | 1;
	buf[1] = BW_PORT_READ_ID_STRING;
#ifdef __AVR_ARCH__
#else
	bcm2835_spi_chipSelect(device_info->chip_select);
	bcm2835_spi_setClockDivider(5000); // 50 kHz
#endif
	FUNC_PREFIX(spi_transfern(buf, BW_ID_STRING_LENGTH));
	printf("[%.20s]\n", &buf[2]);
}
