/**
 * @file bw_spi_relay.c
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

#include <bcm2835.h>
#include <device_info.h>
#include <bw.h>
#include <bw_spi_relay.h>

#ifndef BARE_METAL
#define uwait bcm2835_delayMicroseconds
#else
extern void uwait(int);
#endif

extern int printf(const char *format, ...);

static void bw_spi_relay_fsel_mask(device_info_t *, const uint8_t);

/**
 *
 * @param device_info
 */
inline void static fets_spi_setup(device_info_t *device_info) {
	bcm2835_spi_setClockDivider(2500); // 100kHz
	bcm2835_spi_setChipSelectPolarity(device_info->chip_select, LOW);
	bcm2835_spi_chipSelect(device_info->chip_select);
}

int bw_spi_relay_start(device_info_t *device_info) {

	if (bcm2835_init() != 1)
		return 1;

	bcm2835_spi_begin();
	// Just once. Assuming all devices do have the same
	bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);

	if (device_info->slave_address <= 0)
		device_info->slave_address = BW_RELAY_DEFAULT_SLAVE_ADDRESS;

	bw_spi_relay_fsel_mask(device_info, 0x7F);

	return 0;
}

void bw_spi_relay_end(void) {
	bcm2835_spi_end();
	bcm2835_close();
}

inline static void bw_spi_relay_fsel_mask(device_info_t *device_info, const uint8_t mask) {
	char cmd[3];

	cmd[0] = device_info->slave_address;
	cmd[1] = BW_PORT_WRITE_IO_DIRECTION;
	cmd[2] = mask;
	fets_spi_setup(device_info);

	bcm2835_spi_writenb(cmd, sizeof(cmd) / sizeof(char));
	uwait(BW_RELAY_SPI_BYTE_WAIT_US);
}

void bw_spi_relay_output(device_info_t *device_info, const uint8_t pins) {
	char cmd[3];

	cmd[0] = device_info->slave_address;
	cmd[1] = BW_PORT_WRITE_SET_ALL_OUTPUTS;
	cmd[2] = pins;

	fets_spi_setup(device_info);
	bcm2835_spi_writenb(cmd, sizeof(cmd) / sizeof(char));
	uwait(BW_RELAY_SPI_BYTE_WAIT_US);
}

/**
 *
 * @param device_info
 */
void bw_spi_relay_read_id(device_info_t *device_info) {
	char buf[BW_ID_STRING_LENGTH];
	int i = 0;
	for (i = 0; i < BW_ID_STRING_LENGTH; i++) {
		buf[i] = '\0';
	}

	buf[0] = device_info->slave_address | 1;
	buf[1] = BW_PORT_READ_ID_STRING;

	fets_spi_setup(device_info);
	bcm2835_spi_setClockDivider(5000); // 50 kHz
	bcm2835_spi_transfern(buf, BW_ID_STRING_LENGTH);
	uwait(BW_RELAY_SPI_BYTE_WAIT_US);
	printf("[%.20s]\n", &buf[2]);
}
