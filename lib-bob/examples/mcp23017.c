/**
 * @file mcp23017.c
 *
 */
/* Copyright (C) 2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#include <time.h>
#include <unistd.h>

#include "mcp23017.h"
#include "mcp23x17.h"	/* Needed for direct register access */

#define LED1	MCP23017_PIN_GPA0
#define LED2	MCP23017_PIN_GPB7
#define SWITCH	MCP23017_PIN_GPB0

static void sleep_ms(int milliseconds) {
	struct timespec ts;
	ts.tv_sec = milliseconds / 1000;
	ts.tv_nsec = (milliseconds % 1000) * 1000000;
	nanosleep(&ts, NULL);
}

int main(int argc, char **argv) {
	device_info_t device_mcp23017;
	int i;
	unsigned d;

	if (getuid() != 0) {
		fprintf(stderr, "Program is not started as \'root\' (sudo)\n");
		return -1;
	}

	device_mcp23017.slave_address = 0; /* Use default I2C address */

	if (!mcp23017_start(&device_mcp23017)) {
		fprintf(stderr, "Communication failure!\n");
		return -2;
	}

	puts("Test output 1\n");

	mcp23017_gpio_fsel(&device_mcp23017, LED1, MCP23017_FSEL_OUTP);
	mcp23017_gpio_fsel(&device_mcp23017, LED2, MCP23017_FSEL_OUTP);

	for (i = 0; i < 5; i++) {
		mcp23017_gpio_set(&device_mcp23017, LED1);
		mcp23017_gpio_clr(&device_mcp23017, LED2);
		printf("LED1 on, LED2 off\n");
		sleep_ms(500);
		mcp23017_gpio_clr(&device_mcp23017, LED1);
		mcp23017_gpio_set(&device_mcp23017, LED2);
		printf("LED1 off, LED2 on\n");
		sleep_ms(500);
	}

	puts("\nTest output 2\n");

	/*
	 * Set all ports to OUTPUT and set all to 0
	 */
	mcp23017_reg_write(&device_mcp23017, MCP23X17_IODIRA, 0x0000);
	mcp23017_reg_write(&device_mcp23017, MCP23X17_GPIOA, 0x0000);

	d = 1;

	for (i = 0; i < 16; i++) {
		mcp23017_reg_write(&device_mcp23017, MCP23X17_GPIOA, d);
		printf("%.4x\n", d);
		d = d << 1;
		sleep_ms(500);
	}

	puts("\nTest output 3\n");

	mcp23017_reg_write(&device_mcp23017, MCP23X17_GPIOA, 0xFFFF);

	printf("All on\n");

	sleep(1);

	mcp23017_reg_write(&device_mcp23017, MCP23X17_GPIOA, 0x0000);

	printf("All off\n");

	mcp23017_gpio_fsel(&device_mcp23017, SWITCH, MCP23017_FSEL_INPT);

	puts("\nTest input/output loop\n");

	for (;;) {
		if (mcp23017_gpio_lev(&device_mcp23017, SWITCH)) {
			mcp23017_gpio_set(&device_mcp23017, LED1);
		} else {
			mcp23017_gpio_clr(&device_mcp23017, LED1);
		}
	}

	return 0;
}
