/**
 * @file ft245rl.c
 *
 * @brief Interface for FT245RL
 *
 * Orange Pi
 * P1:H3 PIO_PA			FT245RL
 *
 *  3:GPIO12	<--->	D0
 *  5:GPIO11	<--->	D1
 *  7:GPIO06	<--->	D2
 * 26:GPIO10	<--->	D3
 * 24:GPIO13	<--->	D4
 * 21:GPIO16	<--->	D5
 * 19:GPIO15	<--->	D6
 * 23:GPIO14	<--->	D7
 *
 * 15:GPIO03	---->	WR
 * 16:GPIO19	---->	RD#
 *
 * 18:GPIO18	<----	TXE#
 * 22:GPIO02	<----	RXF#
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

#include <stdint.h>
#include <stdbool.h>

#include "h3_gpio.h"
#include "h3.h"

// PIO PA 6 10 11 12 13 14 15 16
#define D0		12	// CFG1
#define D1		11	// CFG1
#define D2		6	// CFG0
#define D3		10	// CFG1
#define D4		13	// CFG1
#define D5		16	// CFG2
#define D6		15	// CFG1
#define D7		14	// CFG1

#define WR		3	// CFG0
#define _RD		19	// CFG2

#define _TXE	18	// CFG2
#define _RXF	2	// CFG0

#define NOP_COUNT_READ 24
#define NOP_COUNT_WRITE 2

/**
 * Set the GPIOs for data to output
 */
static void data_gpio_fsel_output() {
	uint32_t value = H3_PIO_PORTA->CFG0;
	value &= ~(GPIO_SELECT_MASK << PA6_SELECT_CFG0_SHIFT);	// D2
	value |= (GPIO_FSEL_OUTPUT << PA6_SELECT_CFG0_SHIFT);
	H3_PIO_PORTA->CFG0 = value;

	value = H3_PIO_PORTA->CFG1;
	value &= ~(GPIO_SELECT_MASK << PA12_SELECT_CFG1_SHIFT);	// D0
	value |= (GPIO_FSEL_OUTPUT << PA12_SELECT_CFG1_SHIFT);
	value &= ~(GPIO_SELECT_MASK << PA11_SELECT_CFG1_SHIFT);	// D1
	value |= (GPIO_FSEL_OUTPUT << PA11_SELECT_CFG1_SHIFT);
	value &= ~(GPIO_SELECT_MASK << PA10_SELECT_CFG1_SHIFT);	// D3
	value |= (GPIO_FSEL_OUTPUT << PA10_SELECT_CFG1_SHIFT);
	value &= ~(GPIO_SELECT_MASK << PA13_SELECT_CFG1_SHIFT);	// D4
	value |= (GPIO_FSEL_OUTPUT << PA13_SELECT_CFG1_SHIFT);
	value &= ~(GPIO_SELECT_MASK << PA15_SELECT_CFG1_SHIFT);	// D6
	value |= (GPIO_FSEL_OUTPUT << PA15_SELECT_CFG1_SHIFT);
	value &= ~(GPIO_SELECT_MASK << PA14_SELECT_CFG1_SHIFT);	// D7
	value |= (GPIO_FSEL_OUTPUT << PA14_SELECT_CFG1_SHIFT);
	H3_PIO_PORTA->CFG1 = value;

	value = H3_PIO_PORTA->CFG2;
	value &= ~(GPIO_SELECT_MASK << PA16_SELECT_CFG2_SHIFT);	// D5
	value |= (GPIO_FSEL_OUTPUT << PA16_SELECT_CFG2_SHIFT);
	H3_PIO_PORTA->CFG2 = value;
}

/**
 * Set the GPIOs for data to input
 */
static void data_gpio_fsel_input() {
	uint32_t value = H3_PIO_PORTA->CFG0;
	value &= ~(GPIO_SELECT_MASK << PA6_SELECT_CFG0_SHIFT);	// D2
	value |= (GPIO_FSEL_INPUT << PA6_SELECT_CFG0_SHIFT);
	H3_PIO_PORTA->CFG0 = value;

	value = H3_PIO_PORTA->CFG1;
	value &= ~(GPIO_SELECT_MASK << PA12_SELECT_CFG1_SHIFT);	// D0
	value |= (GPIO_FSEL_INPUT << PA12_SELECT_CFG1_SHIFT);
	value &= ~(GPIO_SELECT_MASK << PA11_SELECT_CFG1_SHIFT);	// D1
	value |= (GPIO_FSEL_INPUT << PA11_SELECT_CFG1_SHIFT);
	value &= ~(GPIO_SELECT_MASK << PA10_SELECT_CFG1_SHIFT);	// D3
	value |= (GPIO_FSEL_INPUT << PA10_SELECT_CFG1_SHIFT);
	value &= ~(GPIO_SELECT_MASK << PA13_SELECT_CFG1_SHIFT);	// D4
	value |= (GPIO_FSEL_INPUT << PA13_SELECT_CFG1_SHIFT);
	value &= ~(GPIO_SELECT_MASK << PA15_SELECT_CFG1_SHIFT);	// D6
	value |= (GPIO_FSEL_INPUT << PA15_SELECT_CFG1_SHIFT);
	value &= ~(GPIO_SELECT_MASK << PA14_SELECT_CFG1_SHIFT);	// D7
	value |= (GPIO_FSEL_INPUT << PA14_SELECT_CFG1_SHIFT);
	H3_PIO_PORTA->CFG1 = value;

	value = H3_PIO_PORTA->CFG2;
	value &= ~(GPIO_SELECT_MASK << PA16_SELECT_CFG2_SHIFT);	// D5
	value |= (GPIO_FSEL_INPUT << PA16_SELECT_CFG2_SHIFT);
	H3_PIO_PORTA->CFG2 = value;
}

/**
 * Set RD#, WR to output, TXE#, RXF# to input.
 * Set RD# to high, set WR to low
 */
void FT245RL_init(void) {
	// RD#, WR output
	uint32_t value = H3_PIO_PORTA->CFG0;
	value &= ~(GPIO_SELECT_MASK << PA3_SELECT_CFG0_SHIFT);	// WR
	value |= (GPIO_FSEL_OUTPUT << PA3_SELECT_CFG0_SHIFT);
	H3_PIO_PORTA->CFG0 = value;

	value = H3_PIO_PORTA->CFG2;
	value &= ~(GPIO_SELECT_MASK << PA19_SELECT_CFG2_SHIFT);	// RD#
	value |= (GPIO_FSEL_OUTPUT << PA19_SELECT_CFG2_SHIFT);
	H3_PIO_PORTA->CFG2 = value;

	// TXE#, RXF# input
	value = H3_PIO_PORTA->CFG0;
	value &= ~(GPIO_SELECT_MASK << PA2_SELECT_CFG0_SHIFT);	// RXF#
	value |= (GPIO_FSEL_INPUT << PA2_SELECT_CFG0_SHIFT);
	H3_PIO_PORTA->CFG0 = value;

	value = H3_PIO_PORTA->CFG2;
	value &= ~(GPIO_SELECT_MASK << PA18_SELECT_CFG2_SHIFT);	// TXE#
	value |= (GPIO_FSEL_INPUT << PA18_SELECT_CFG2_SHIFT);
	H3_PIO_PORTA->CFG2 = value;

	// RD#	high
	h3_gpio_set(_RD);
	// WR	low
	h3_gpio_clr(WR);
}

/**
 * Write 8-bits to USB
 */
void FT245RL_write_data(uint8_t data) {
	uint8_t i;
	data_gpio_fsel_output();
	// Raise WR to start the write.
	h3_gpio_set(WR);
	i = NOP_COUNT_WRITE;
	for (; i > 0; i--) {
		asm volatile("nop"::);
	}
	// Put the data on the bus.
	uint32_t out_gpio = H3_PIO_PORTA->DAT & ~( (1 << D0) | (1 << D1) | (1 << D2) | (1 << D3) | (1 << D4) | (1 << D5) | (1 << D6) | (1 << D7));
	out_gpio |= (data & 1) ? (1 << D0) : 0;
	out_gpio |= (data & 2) ? (1 << D1) : 0;
	out_gpio |= (data & 4) ? (1 << D2) : 0;
	out_gpio |= (data & 8) ? (1 << D3) : 0;
	out_gpio |= (data & 16) ? (1 << D4) : 0;
	out_gpio |= (data & 32) ? (1 << D5) : 0;
	out_gpio |= (data & 64) ? (1 << D6) : 0;
	out_gpio |= (data & 128) ? (1 << D7) : 0;
	H3_PIO_PORTA->DAT = out_gpio;
	i = NOP_COUNT_WRITE;
	for (; i > 0; i--) {
		asm volatile("nop"::);
	}
	// Drop WR to tell the FT245 to read the data.
	h3_gpio_clr(WR);
}

/**
 * Read 8-bits from USB
 */
uint8_t FT245RL_read_data() {
	data_gpio_fsel_input();
	h3_gpio_clr(_RD);
	// Wait for the FT245 to respond with data.
	uint8_t i = NOP_COUNT_READ;
	for (; i > 0; i--) {
		asm volatile("nop"::);
	}
	// Read the data from the data port.
	const uint32_t in_gpio = H3_PIO_PORTA->DAT;
	uint8_t data = in_gpio & (1 << D0) ? 1 : 0;
	data |= in_gpio & (1 << D1) ? 2 : 0;
	data |= in_gpio & (1 << D2) ? 4 : 0;
	data |= in_gpio & (1 << D3) ? 8 : 0;
	data |= in_gpio & (1 << D4) ? 16 : 0;
	data |= in_gpio & (1 << D5) ? 32 : 0;
	data |= in_gpio & (1 << D6) ? 64 : 0;
	data |= in_gpio & (1 << D7) ? 128 : 0;
	// Bring RD# back up so the FT245 can let go of the data.
	h3_gpio_set(_RD);
	return data;
}

/**
 * Read RXF#
 */
bool FT245RL_data_available(void) {
	return (!(H3_PIO_PORTA->DAT & (1 << _RXF)));
}

/**
 * Read TXE#
 */
bool FT245RL_can_write(void) {
	return (!(H3_PIO_PORTA->DAT & (1 << _TXE)));
}
