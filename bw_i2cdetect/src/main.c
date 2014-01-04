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
 gcc -O3 -Wall -o i2cdetect main.c -l bcm2835
 sudo ./i2cdetect

Or you can test it before installing with:
 gcc -O3 -Wall -o i2cdetect -I ../../src ../../src/bcm2835.c main.c
 sudo ./i2cdetect
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <bcm2835.h>

#define VERSION "1.0.0"

#ifndef FALSE
#define FALSE	0
#define TRUE	!FALSE
#endif

struct device_details {
	char address;
	char name[32];
	char isBW;
};

struct device_details devices[] = {
		{ 0x41, "LCD",                  TRUE},
		{ 0x4A, "User Interface (UI)",  TRUE},
		{ 0x57, "MCP7941X {EEPROM}",    FALSE},
		{ 0x6F, "MCP7941X {SRAM RTCC}", FALSE}
};

static char *lookup_device(char address) {
	int i = 0;
	int j = sizeof(devices) / sizeof(struct device_details);
	static char string[48];

	for (i = 0; i < j; i++) {
		if (devices[i].address == address) {
			strcpy(string, devices[i].name);
			if (devices[i].isBW) {
				char bw_address[10];
				snprintf(bw_address, sizeof(bw_address), " BW:0x%.2X", address << 1);
				strcat(string, bw_address);
			}
			return string;
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
		fprintf(stderr, "bw_i2cdetect version %s\n", VERSION);
		exit(0);
	}

	if (bcm2835_init() != 1) {
		fprintf(stderr, "bcm2835_init() failed\n");
		return 1;
	}

	bcm2835_i2c_begin();

	bcm2835_i2c_setClockDivider(BCM2835_I2C_CLOCK_DIVIDER_2500); // 100kHz

	int address;

	for (address = 0x00; address <= 0x7F; address += 1) {
		if (address < first || address > last) {
			continue;
		}
		bcm2835_i2c_setSlaveAddress(address);
		if (bcm2835_i2c_write(NULL, 0) == 0) {
			printf("0x%.2X : %s\n", address, lookup_device(address));
		}
	}

	bcm2835_i2c_end();

	bcm2835_close();

	return 0;
}
