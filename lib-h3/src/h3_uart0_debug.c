/**
 * @file h3_uart0_debug.c
 *
 */
/* Copyright (C) 2018-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <stdio.h>

#include "h3.h"
#include "h3_ccu.h"
#include "h3_gpio.h"

#include "arm/synchronize.h"

#include "uart.h"

#define BUS_CLK_GATING3_UART0	(1U << 16)
#define BUS_SOFT_RESET4_UART0	(1U << 16)

static void uart_gpio_init(void) {
	uint32_t value = H3_PIO_PORTA->CFG0;
	// PA4, TX
	value &= (uint32_t)~(GPIO_SELECT_MASK << PA4_SELECT_CFG0_SHIFT);
	value |= H3_PA4_SELECT_UART0_TX << PA4_SELECT_CFG0_SHIFT;
	// PA5, RX
	value &= (uint32_t)~(GPIO_SELECT_MASK << PA5_SELECT_CFG0_SHIFT);
	value |= H3_PA5_SELECT_UART0_RX << PA5_SELECT_CFG0_SHIFT;
	H3_PIO_PORTA->CFG0 = value;
	// Pin 5, RX, Pull-up
	value = H3_PIO_PORTA->PUL0;
	value &= (uint32_t)~(GPIO_PULL_MASK << H3_PA5_PULL0_SHIFT);
	value |= GPIO_PULL_UP << H3_PA5_PULL0_SHIFT;
	H3_PIO_PORTA->PUL0 = value;
}

static void uart_clock_init(void) {
	H3_CCU->BUS_SOFT_RESET4 |= CCU_BUS_SOFT_RESET4_UART0;
	udelay(1000); // 1ms
	H3_CCU->BUS_CLK_GATING3 &= ~CCU_BUS_CLK_GATING3_UART0;
	udelay(1000); // 1ms
	H3_CCU->BUS_CLK_GATING3 |= CCU_BUS_CLK_GATING3_UART0;
}

void __attribute__((cold)) uart0_init(void) {
	uart_gpio_init();
	uart_clock_init();

	H3_UART0->LCR = UART_LCR_DLAB;
	H3_UART0->O00.DLL = BAUD_115200_L;
	H3_UART0->O04.DLH = BAUD_115200_H;
	H3_UART0->LCR = UART_LCR_8_N_1;
	H3_UART0->O08.FCR = UART_FCR_EFIFO | UART_FCR_TRESET;
	H3_UART0->O04.IER = 0;

	dmb();

	while ((H3_UART0->USR & UART_USR_BUSY) == UART_USR_BUSY) {
		(void) H3_UART0->O00.RBR;
	}
}

void uart0_putc(char c) {
	while (!(H3_UART0->LSR & UART_LSR_THRE))
		;
	H3_UART0->O00.THR = c;
}

void uart0_puts(char *s) {
	while (*s != '\0') {
		if (*s == '\n') {
			uart0_putc('\r');
		}
		uart0_putc(*s++);
	}
}

int uart0_getc(void) {
	if (__builtin_expect(((H3_UART0->LSR & UART_LSR_DR) != UART_LSR_DR), 1)) {
		return EOF;
	}

	const char c = H3_UART0->O00.RBR;

#if defined (UART0_ECHO)
	uart0_putc(c);
#endif

	return (int) c;
}

