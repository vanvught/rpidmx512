/* Copyright (C) 2015 by Arjan van Vught <pm @ http://www.raspberrypi.org/forum/>
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

/*
 * Based on
	    i2cdetect.c - a user-space program to scan for I2C devices
	    Copyright (C) 1999-2004  Frodo Looijaard <frodol@dds.nl>, and
	                             Mark D. Studebaker <mdsxyz123@yahoo.com>
	    Copyright (C) 2004-2012  Jean Delvare <khali@linux-fr.org>

 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 */

#include <stdio.h>
#include <stdlib.h>

#ifdef __AVR_ARCH__
#define FUNC_PREFIX(x) avr_##x
#include "avr_i2c.h"
#include "uart.h"
#else
#include "bcm2835.h"
#define FUNC_PREFIX(x) bcm2835_##x
#endif

#define VERSION "1.1.0"

struct device_details {
	char slave_address;
	const char *name;
} const devices[] = {
		{ 0x41, "BW:LCD" },
		{ 0x42, "BW:7 digital IO lines (DIO)" },
		{ 0x43, "BW:Servo" },
		{ 0x44, "BW:7 FETs" },
		{ 0x45, "BW:3 FETs" },
		{ 0x46, "BW:Temp" },
		{ 0x47, "BW:Relay" },
		{ 0x48, "BW:Motor" },
		{ 0x4A, "BW:User Interface (UI)" },
		{ 0x4B, "BW:7 segment" },
		{ 0x4D, "BW:Pushbutton" },
		{ 0x4E, "BW:Bigrelay" },
		{ 0x4F, "BW:Dimmer" },
		{ 0x52, "BW:Raspberry juice" },
		{ 0x57, "MCP7941X {EEPROM}" },
		{ 0x6F, "MCP7941X {SRAM RTCC}" }
};

static const char *lookup_device(char slave_address) {
	int i = 0;
	int j = sizeof(devices) / sizeof(struct device_details);

	for (i = 0; i < j; i++) {
		if (devices[i].slave_address == slave_address) {
			return devices[i].name;
		}
	}

	return "Unknown device";
}

int main(int argc, char **argv) {
	uint8_t first = 0x03, last = 0x77;
	uint8_t i, j;
	char buf;
	uint8_t address;
	uint8_t ret;

#ifdef __AVR_ARCH__
	UART_BEGIN();
#else
    int flags = 0;
    int version = 0;

	while (1 + flags < argc && argv[1 + flags][0] == '-') {
		switch (argv[1 + flags][1]) {
		case 'V': version = 1; break;
		case 'a':
			first = 0x00;
			last = 0x7F;
			break;
		default:
			fprintf(stderr, "Warning: Unsupported flag \"-%c\"!\n", argv[1 + flags][1]);
			//help(); // TODO
			exit(1);
		}
		flags++;
	}

    if (version) {
		fprintf(stderr, "i2cdetect version %s\n", VERSION);
		exit(0);
	}
#endif
#if !defined(BARE_METAL) && !defined(__AVR_ARCH__)
	if (bcm2835_init() != 1) {
		fprintf(stderr, "bcm2835_init() failed\n");
		exit(1);
	}
#endif

	FUNC_PREFIX(i2c_begin());

#ifdef __AVR_ARCH__
#else
	bcm2835_i2c_setClockDivider(BCM2835_I2C_CLOCK_DIVIDER_2500); // 100kHz
#endif

	for (address = 0x00; address <= 0x7F; address++) {
		/* Skip unwanted addresses */
		if (address < first || address > last) {
			continue;
		}

		/* Set slave address */
		FUNC_PREFIX(i2c_setSlaveAddress(address));

		/* Probe this address */
		if ((address >= 0x30 && address <= 0x37) || (address >= 0x50 && address <= 0x5F)) {
			ret = FUNC_PREFIX(i2c_read(&buf, 1));
		} else {
			ret = FUNC_PREFIX(i2c_write(NULL, 0));
		}

		if (ret == 0) {
			printf("0x%.2X : 0x%.2X : %s\n", address, address << 1, lookup_device(address));
		}
	}

	printf("\n");

	printf("     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f\n");

	for (i = 0; i < 128; i += 16) {
		printf("%02x: ", i);
		for (j = 0; j < 16; j++) {
			/* Skip unwanted addresses */
			if (i + j < first || i + j > last) {
				printf("   ");
				continue;
			}

			/* Set slave address */
			FUNC_PREFIX(i2c_setSlaveAddress(i + j));


			/* Probe this address */
			if ((i + j >= 0x30 && i + j <= 0x37) || (i + j >= 0x50 && i + j <= 0x5F)) {
				ret = FUNC_PREFIX(i2c_read(&buf, 1));
			} else {
				/* This is known to corrupt the Atmel AT24RF08 EEPROM */
				ret = FUNC_PREFIX(i2c_write(NULL, 0));
			}

			if (ret != 0)
				printf("-- ");
			else
				printf("%02x ", i + j);
		}
		printf("\n");
	}

	FUNC_PREFIX(i2c_end());

#if !defined(BARE_METAL) && !defined(__AVR_ARCH__)
	bcm2835_close();
#endif

	return 0;
}
