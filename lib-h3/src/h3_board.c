/**
 * @file h3_board.c
 *
 */
/* Copyright (C) 2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#ifndef NDEBUG
 #include <stdio.h>
#endif

#include "h3_board.h"

void h3_board_dump(void) {
#ifndef NDEBUG
	#define PORT_LETTER(gpio) 	('A' + H3_GPIO_TO_PORT(gpio))
	#define OUT_L(g)			g, PORT_LETTER(g), H3_GPIO_TO_NUMBER(g)
	#define OUT_R(g)			PORT_LETTER(g), H3_GPIO_TO_NUMBER(g), g

	printf("%s\n", H3_BOARD_NAME);
	printf("              3V3 PWR   1 :  2 5V PWR\n");
	printf("I2C%d SDA  GPIO%-3d P%c%-2d  3 :  4 5V PWR\n", EXT_I2C_NUMBER, OUT_L(GPIO_EXT_3));
	printf("I2C%d SCL  GPIO%-3d P%c%-2d  5 :  6 GND\n",  EXT_I2C_NUMBER, OUT_L(GPIO_EXT_5));
	printf("          GPIO%-3d P%c%-2d  7 :  8 P%c%-2d GPIO%-3d UART%d TX\n",  OUT_L(GPIO_EXT_7), OUT_R(GPIO_EXT_8), EXT_UART_NUMBER);
	printf("                  GND   9 : 10 P%c%-2d GPIO%-3d UART%d RX\n",  OUT_R(GPIO_EXT_10), EXT_UART_NUMBER);
	printf("          GPIO%-3d P%c%-2d 11 : 12 P%c%-2d GPIO%-3d\n",  OUT_L(GPIO_EXT_11), OUT_R(GPIO_EXT_12));
	printf("          GPIO%-3d P%c%-2d 13 : 14 GND\n",  OUT_L(GPIO_EXT_13));
	printf("          GPIO%-3d P%c%-2d 15 : 16 P%c%-2d GPIO%-3d\n",  OUT_L(GPIO_EXT_15), OUT_R(GPIO_EXT_16));
	printf("              3V3 PWR  17 : 18 P%c%-2d GPIO%-3d\n",  OUT_R(GPIO_EXT_18));
	printf("SPI%d MOSI GPIO%-3d P%c%-2d 19 : 20 GND\n", EXT_SPI_NUMBER, OUT_L(GPIO_EXT_19));
	printf("SPI%d MISO GPIO%-3d P%c%-2d 21 : 22 P%c%-2d GPIO%-3d\n", EXT_SPI_NUMBER, OUT_L(GPIO_EXT_21), OUT_R(GPIO_EXT_22));
	printf("SPI%d CLK  GPIO%-3d P%c%-2d 23 : 24 P%c%-2d GPIO%-3d SPI%d CS0\n", EXT_SPI_NUMBER, OUT_L(GPIO_EXT_23), OUT_R(GPIO_EXT_24), EXT_SPI_NUMBER);
#if !defined(NANO_PI)
	printf("                  GND  24 : 26 P%c%-2d GPIO%-3d\n",  OUT_R(GPIO_EXT_26));
#endif
	printf("\nStatus led: P%c%-2d GPIO%-3d\n", PORT_LETTER(H3_BOARD_STATUS_LED), H3_BOARD_STATUS_LED, H3_GPIO_TO_NUMBER(H3_BOARD_STATUS_LED));
#endif
}

