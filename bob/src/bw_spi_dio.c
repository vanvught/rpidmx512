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
//
#include <bw.h>
#include <bw_dio.h>

#ifndef BARE_METAL
static void uwait(int us) { bcm2835_delayMicroseconds(us); }
#else
extern void uwait(int);
#endif

extern int printf(const char *format, ...);

inline void static dio_spi_setup(device_info_t *device_info) {
	bcm2835_spi_setClockDivider(2500); // 100kHz
	bcm2835_spi_setChipSelectPolarity(device_info->chip_select, LOW);
	bcm2835_spi_chipSelect(device_info->chip_select);
}

int bw_spi_dio_start (device_info_t *device_info) {

	if (bcm2835_init() != 1)
		return 1;

	bcm2835_spi_begin();
	// Just once. Assuming all devices do have the same
	bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);

	if (device_info->slave_address <= 0)
		device_info->slave_address = BW_DIO_DEFAULT_SLAVE_ADDRESS;

	return 0;
}

void bw_spi_dio_end(void) {
	bcm2835_spi_end();
	bcm2835_close();
}

void bw_spi_dio_fsel_mask(device_info_t device_info, unsigned char mask) {
	char cmd[3];

	cmd[0] = device_info.slave_address;
	cmd[1] = BW_PORT_WRITE_IO_DIRECTION;
	cmd[2] = mask;
	dio_spi_setup(&device_info);

	bcm2835_spi_writenb(cmd, sizeof(cmd) / sizeof(char));
	uwait(BW_DIO_SPI_BYTE_WAIT_US);
}

void bw_spi_dio_output(device_info_t device_info, unsigned char pins) {
	char cmd[3];

	cmd[0] = device_info.slave_address;
	cmd[1] = BW_PORT_WRITE_SET_ALL_OUTPUTS;
	cmd[2] = pins;

	dio_spi_setup(&device_info);
	bcm2835_spi_writenb(cmd, sizeof(cmd) / sizeof(char));
	uwait(BW_DIO_SPI_BYTE_WAIT_US);
}

void bw_spi_dio_read_id(device_info_t device_info) {
	char buf[BW_DIO_ID_STRING_LENGTH + 1];
	int i = 0;
	for (i = 0; i <= BW_DIO_ID_STRING_LENGTH; i++) {
		buf[i] = '\0';
	}

	buf[0] = device_info.slave_address | 1;
	buf[1] = BW_PORT_READ_ID_STRING;

	dio_spi_setup(&device_info);
	bcm2835_spi_transfern(buf, BW_DIO_ID_STRING_LENGTH);
	uwait(BW_DIO_SPI_BYTE_WAIT_US);
	printf("[%s]\n", &buf[1]);
}
