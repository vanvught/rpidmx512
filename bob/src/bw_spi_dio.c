/**
 * @file bw_spi_dio.c
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
#include <util/delay.h>
#include <avr_spi.h>
#else
#include <bcm2835.h>
#ifdef BARE_METAL
#include <bcm2835_spi.h>
#endif
#endif
#include <device_info.h>
#include <bw.h>
#include <bw_spi_dio.h>

#ifdef __AVR_ARCH__
#define udelay _delay_us
#define FUNC_PREFIX(x) avr_##x
#else
#define FUNC_PREFIX(x) bcm2835_##x
#ifndef BARE_METAL
#define udelay bcm2835_delayMicroseconds
#endif
#endif

/**
 *
 * @param device_info
 */
inline static void dio_spi_setup(const device_info_t *device_info) {
#ifdef __AVR_ARCH__
#else
	bcm2835_spi_setClockDivider(2500); // 100kHz
	bcm2835_spi_chipSelect(device_info->chip_select);
#endif
}

/**
 *
 * @param device_info
 * @return
 */
uint8_t bw_spi_dio_start(device_info_t *device_info) {
#if !defined(BARE_METAL) && !defined(__AVR_ARCH__)
	if (bcm2835_init() != 1)
		return 1;
#endif
	FUNC_PREFIX(spi_begin());

	if (device_info->slave_address <= 0)
		device_info->slave_address = BW_DIO_DEFAULT_SLAVE_ADDRESS;

	return 0;
}

/**
 *
 */
void bw_spi_dio_end(void) {
	FUNC_PREFIX(spi_end());
}

/**
 *
 * @param device_info
 * @param mask
 */
void bw_spi_dio_fsel_mask(const device_info_t *device_info, const uint8_t mask) {
	char cmd[3];
	cmd[0] = device_info->slave_address;
	cmd[1] = BW_PORT_WRITE_IO_DIRECTION;
	cmd[2] = mask;
	dio_spi_setup(device_info);
	FUNC_PREFIX(spi_writenb(cmd, sizeof(cmd) / sizeof(char)));
	udelay(BW_DIO_SPI_BYTE_WAIT_US);
}

/**
 *
 * @param device_info
 * @param pins
 */
void bw_spi_dio_output(const device_info_t *device_info, const uint8_t pins) {
	char cmd[3];
	cmd[0] = device_info->slave_address;
	cmd[1] = BW_PORT_WRITE_SET_ALL_OUTPUTS;
	cmd[2] = pins;
	dio_spi_setup(device_info);
	FUNC_PREFIX(spi_writenb(cmd, sizeof(cmd) / sizeof(char)));
	udelay(BW_DIO_SPI_BYTE_WAIT_US);
}

