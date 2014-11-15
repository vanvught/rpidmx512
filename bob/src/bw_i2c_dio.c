/**
 * @file bw_i2c_dio.c
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
#ifdef BARE_METAL
#include <bcm2835_i2c.h>
#endif
#include <device_info.h>
#include <bw.h>
#include <bw_i2c_dio.h>

#ifndef BARE_METAL
#define udelay bcm2835_delayMicroseconds
#endif

extern int printf(const char *format, ...);

/**
 *
 * @param device_info
 */
inline static void dio_i2c_setup(device_info_t *device_info) {
	bcm2835_i2c_setSlaveAddress(device_info->slave_address >> 1);
	bcm2835_i2c_setClockDivider(BCM2835_I2C_CLOCK_DIVIDER_2500);
}

/**
 *
 * @param device_info
 * @return
 */
int bw_i2c_dio_start(device_info_t *device_info) {

	if (bcm2835_init() != 1)
		return 1;

	bcm2835_i2c_begin();

	if (device_info->slave_address <= 0)
		device_info->slave_address = BW_DIO_DEFAULT_SLAVE_ADDRESS;

	return 0;
}

/**
 *
 */
void bw_i2c_dio_end(void) {
	bcm2835_i2c_end();
	bcm2835_close();
}

/**
 *
 * @param device_info
 * @param mask
 */
void bw_i2c_dio_fsel_mask(device_info_t *device_info, const uint8_t mask) {
	char cmd[2];

	cmd[0] = BW_PORT_WRITE_IO_DIRECTION;
	cmd[1] = mask;

	dio_i2c_setup(device_info);
	bcm2835_i2c_write(cmd, sizeof(cmd) / sizeof(char));
	udelay(BW_DIO_I2C_BYTE_WAIT_US);
}

/**
 *
 * @param device_info
 * @param pins
 */
void bw_i2c_dio_output(device_info_t *device_info, const uint8_t pins) {
	char cmd[2];

	cmd[0] = BW_PORT_WRITE_SET_ALL_OUTPUTS;
	cmd[1] = pins;

	dio_i2c_setup(device_info);
	bcm2835_i2c_write(cmd, sizeof(cmd) / sizeof(char));
	udelay(BW_DIO_I2C_BYTE_WAIT_US);
}

/**
 *
 * @param device_info
 */
void bw_i2c_dio_read_id(device_info_t *device_info) {
	static char cmd[] = { BW_PORT_READ_ID_STRING };
	char buf[BW_ID_STRING_LENGTH];
	dio_i2c_setup(device_info);
	bcm2835_i2c_write(cmd, sizeof(cmd) / sizeof(char));
	udelay(BW_DIO_I2C_BYTE_WAIT_US);
	bcm2835_i2c_read(buf, BW_ID_STRING_LENGTH);
	printf("[%s]\n", buf);
}
