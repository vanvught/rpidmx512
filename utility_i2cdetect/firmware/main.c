/**
 * @file main.c
 *
 */
/* Copyright (C) 2017 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include "bcm2835_i2c.h"

#include "hardware.h"
#include "console.h"
#include "util.h"

#include "software_version.h"

void __attribute__((interrupt("FIQ"))) c_fiq_handler(void) {}
void __attribute__((interrupt("IRQ"))) c_irq_handler(void) {}

struct device_details {
	uint8_t slave_address;
	const char *name;
} static const devices[] ALIGNED = {
		{ 0x27, "PCF8574T" },
		{ 0x41, "BW:LCD" },
		{ 0x42, "BW:7 digital IO lines (DIO)" },
		{ 0x43, "BW:Servo" },
		{ 0x44, "BW:7 FETs" },
		{ 0x45, "BW:3 FETs" },
		{ 0x46, "BW:Temp" },
		{ 0x47, "BW:Relay" },
		{ 0x48, "PCF8591 | BW:Motor" },
		{ 0x4A, "BW:User Interface (UI)" },
		{ 0x4B, "BW:7 segment" },
		{ 0x4D, "BW:Pushbutton" },
		{ 0x4E, "BW:Bigrelay" },
		{ 0x4F, "BW:Dimmer" },
		{ 0x52, "BW:Raspberry juice" },
		{ 0x57, "MCP7941X {EEPROM}" },
		{ 0x6F, "MCP7941X {SRAM RTCC}" } };

/*@observer@*/static const char *lookup_device(const uint8_t slave_address) {
	int i = 0;
	int j = (int) (sizeof(devices) / sizeof(struct device_details));

	for (i = 0; i < j; i++) {
		if (devices[i].slave_address == slave_address) {
			return devices[i].name;
		}
	}

	return "Unknown device";
}

void notmain(void) {
	uint8_t first = 0x03, last = 0x77;
	uint8_t i, j;
	char buf;
	uint8_t address;
	uint8_t ret;

	hardware_init();

	printf("[V%s] %s Compiled on %s at %s\n", SOFTWARE_VERSION, hardware_board_get_model(), __DATE__, __TIME__);

	console_set_top_row(3);

	bcm2835_i2c_begin();
	bcm2835_i2c_setClockDivider(BCM2835_I2C_CLOCK_DIVIDER_2500); // 100kHz

	for (address = 0x00; address <= 0x7F; address++) {
		/* Skip unwanted addresses */
		if (address < first || address > last) {
			continue;
		}

		/* Set slave address */
		bcm2835_i2c_setSlaveAddress(address);

		/* Probe this address */
		if ((address >= 0x30 && address <= 0x37) || (address >= 0x50 && address <= 0x5F)) {
			ret = bcm2835_i2c_read(&buf, 1);
		} else {
			ret = bcm2835_i2c_write(NULL, 0);
		}

		if (ret == 0) {
			printf("0x%.2X : 0x%.2X : %s\n", (unsigned int) address, (unsigned int) (address << 1), lookup_device(address));
		}
	}

	(void) console_puts("\n");
	(void) console_puts("     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f\n");

	for (i = 0; i < 128; i += 16) {
		printf("%02x: ", (unsigned int) i);
		for (j = 0; j < 16; j++) {
			/* Skip unwanted addresses */
			if (i + j < first || i + j > last) {
				(void) console_puts("   ");
				continue;
			}

			/* Set slave address */
			bcm2835_i2c_setSlaveAddress(i + j);

			/* Probe this address */
			if ((i + j >= 0x30 && i + j <= 0x37) || (i + j >= 0x50 && i + j <= 0x5F)) {
				ret = bcm2835_i2c_read(&buf, 1);
			} else {
				/* This is known to corrupt the Atmel AT24RF08 EEPROM */
				ret = bcm2835_i2c_write(NULL, 0);
			}

			if (ret != 0) {
				(void) console_puts("-- ");
			} else {
				printf("%02x ", (unsigned int) (i + j));
			}
		}

		(void) console_puts("\n");
	}

	for (;;) {

	}

}
