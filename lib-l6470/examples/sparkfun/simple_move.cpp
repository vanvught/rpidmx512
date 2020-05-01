/**
 * @file simple_move.cpp
 *
 */
/* Copyright (C) 2017-2019 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <unistd.h>
#include <sys/types.h>

#include "bcm2835.h"
#include "autodriver.h"

#define GPIO_BUSY_IN	RPI_V2_GPIO_P1_35
#define GPIO_RESET_OUT 	RPI_V2_GPIO_P1_38

int main(int argc, char **argv) {
	/***************************************************************************/
	/*                                                                         */
	if (getuid() != 0) {
		fprintf(stderr, "Error: Not started with 'root'\n");
		return -1;
	}

	if (bcm2835_init() != 1) {
		fprintf(stderr, "bcm2835_init() failed\n");
		return -2;
	}

	bcm2835_spi_begin();

	bcm2835_gpio_fsel(GPIO_RESET_OUT, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_set(GPIO_RESET_OUT);

	bcm2835_gpio_fsel(GPIO_BUSY_IN, BCM2835_GPIO_FSEL_INPT);

	bcm2835_gpio_clr(GPIO_RESET_OUT);
	bcm2835_delayMicroseconds(10000);
	bcm2835_gpio_set(GPIO_RESET_OUT);
	bcm2835_delayMicroseconds(10000);
	/*                                                                         */
	/***************************************************************************/

	AutoDriver board(0, BCM2835_SPI_CS0, GPIO_RESET_OUT);
	//AutoDriver board(0, BCM2835_SPI_CS0, GPIO_RESET_OUT, GPIO_BUSY_IN);

	const int stepmove = 300000;

	while (board.busyCheck())
		;

	board.move(stepmove);

	while (board.busyCheck())
		;

	board.move(-stepmove);

	while (board.busyCheck())
		;

	board.hardHiZ();

	puts("Done!");

	return 0;
}
