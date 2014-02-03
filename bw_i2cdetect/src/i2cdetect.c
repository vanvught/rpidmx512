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

/*
After installing bcm2835, you can build this with something like:
 gcc -O3 -Wall -o i2cdetect i2cdetect.c -l bcm2835
 sudo ./i2cdetect

Or you can test it before installing with:
 gcc -O3 -Wall -o i2cdetect -I ../../src ../../src/bcm2835.c i2cdetect.c
 sudo ./i2cdetect
*/

#include <stdio.h>
#include <stdlib.h>

#include <bcm2835.h>

#define VERSION "1.0.0"

struct device_details {
	char slave_address;
	const char *name;
} const devices[] = {
		{ 0x41, "BW:LCD" },
		{ 0x4A, "BW:User Interface (UI)" },
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
	int first = 0x03, last = 0x77;
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

	if (bcm2835_init() != 1) {
		fprintf(stderr, "bcm2835_init() failed\n");
		exit(1);
	}

	bcm2835_i2c_begin();

	bcm2835_i2c_setClockDivider(BCM2835_I2C_CLOCK_DIVIDER_2500); // 100kHz

	int address;

	for (address = 0x00; address <= 0x7F; address++) {
		if (address < first || address > last) {
			continue;
		}
		bcm2835_i2c_setSlaveAddress(address);
		if (bcm2835_i2c_write(NULL, 0) == 0) {
			printf("0x%.2X : 0x%.2X : %s\n", address, address << 1, lookup_device(address));
		}
	}

	bcm2835_i2c_end();

	bcm2835_close();

	return 0;
}
