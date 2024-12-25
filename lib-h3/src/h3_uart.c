/**
 * @file h3_uart.c
 *
 */
/* Copyright (C) 2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <assert.h>

#include "h3_uart.h"

#include "h3.h"
#include "h3_gpio.h"
#include "h3_ccu.h"

#include "arm/synchronize.h"

#define DEFAUlT_BAUDRATE	115200U

void __attribute__((cold)) h3_uart_begin(uint32_t uart_base, uint32_t baudrate, uint32_t bits, uint32_t parity, uint32_t stop_bits) {
	assert((uart_base >= H3_UART0_BASE) && (uart_base <= H3_UART3_BASE));
	assert(baudrate != 0);

	if ((((24000000U / 16U) / baudrate) > (uint16_t) (~0)) || (((24000000U / 16U) / baudrate) == 0)) {
		baudrate = DEFAUlT_BAUDRATE;
	}

	const uint32_t divisor = (24000000U / 16U) / baudrate;
	H3_UART_TypeDef *p = (H3_UART_TypeDef*) (uart_base);
	uint32_t lcr;

	if (uart_base == H3_UART0_BASE) {
		uint32_t value = H3_PIO_PORTA->CFG0;
		// PA4, TX
		value &= (uint32_t) (~(GPIO_SELECT_MASK << PA4_SELECT_CFG0_SHIFT));
		value |= H3_PA4_SELECT_UART0_TX << PA4_SELECT_CFG0_SHIFT;
		// PA5, RX
		value &= (uint32_t) (~(GPIO_SELECT_MASK << PA5_SELECT_CFG0_SHIFT));
		value |= H3_PA5_SELECT_UART0_RX << PA5_SELECT_CFG0_SHIFT;
		H3_PIO_PORTA->CFG0 = value;

		H3_CCU->BUS_SOFT_RESET4 |= CCU_BUS_SOFT_RESET4_UART0;
		H3_CCU->BUS_CLK_GATING3 |= CCU_BUS_CLK_GATING3_UART0;
	} else if (uart_base == H3_UART1_BASE) {
		uint32_t value = H3_PIO_PORTG->CFG0;
		// PG6, TX
		value &= (uint32_t) (~(GPIO_SELECT_MASK << PG6_SELECT_CFG0_SHIFT));
		value |= H3_PG6_SELECT_UART1_TX << PG6_SELECT_CFG0_SHIFT;
		// PG7, RX
		value &= (uint32_t) (~(GPIO_SELECT_MASK << PG7_SELECT_CFG0_SHIFT));
		value |= H3_PG7_SELECT_UART1_RX << PG7_SELECT_CFG0_SHIFT;
		H3_PIO_PORTG->CFG0 = value;

		H3_CCU->BUS_SOFT_RESET4 |= CCU_BUS_SOFT_RESET4_UART1;
		H3_CCU->BUS_CLK_GATING3 |= CCU_BUS_CLK_GATING3_UART1;
	} else if (uart_base == H3_UART2_BASE) {
		uint32_t value = H3_PIO_PORTA->CFG0;
		// PA0, TX
		value &= (uint32_t) (~(GPIO_SELECT_MASK << PA0_SELECT_CFG0_SHIFT));
		value |= H3_PA0_SELECT_UART2_TX << PA0_SELECT_CFG0_SHIFT;
		// PA1, RX
		value &= (uint32_t) (~(GPIO_SELECT_MASK << PA1_SELECT_CFG0_SHIFT));
		value |= H3_PA1_SELECT_UART2_RX << PA1_SELECT_CFG0_SHIFT;
		H3_PIO_PORTA->CFG0 = value;

		H3_CCU->BUS_SOFT_RESET4 |= CCU_BUS_SOFT_RESET4_UART2;
		H3_CCU->BUS_CLK_GATING3 |= CCU_BUS_CLK_GATING3_UART2;
	} else if (uart_base == H3_UART3_BASE) {
		uint32_t value = H3_PIO_PORTA->CFG1;
		// PA13, TX
		value &= (uint32_t) (~(GPIO_SELECT_MASK << PA13_SELECT_CFG1_SHIFT));
		value |= H3_PA13_SELECT_UART3_TX << PA13_SELECT_CFG1_SHIFT;
		// PA14, RX
		value &= (uint32_t) (~(GPIO_SELECT_MASK << PA14_SELECT_CFG1_SHIFT));
		value |= H3_PA14_SELECT_UART3_RX << PA14_SELECT_CFG1_SHIFT;
		H3_PIO_PORTA->CFG1 = value;

		H3_CCU->BUS_SOFT_RESET4 |= CCU_BUS_SOFT_RESET4_UART3;
		H3_CCU->BUS_CLK_GATING3 |= CCU_BUS_CLK_GATING3_UART3;
	} else {
		assert(0);
	}

	switch (bits) {
	case H3_UART_BITS_5:
		lcr = UART_LCR_DLS_5BITS;
		break;
	case H3_UART_BITS_6:
		lcr = UART_LCR_DLS_6BITS;
		break;
	case H3_UART_BITS_7:
		lcr = UART_LCR_DLS_7BITS;
		break;
	case H3_UART_BITS_8:
	default:
		lcr = UART_LCR_DLS_8BITS;
	}

	if (parity != H3_UART_PARITY_NONE) {
		lcr |= UART_LCR_PEN;

		if (parity == H3_UART_PARITY_ODD) {
			lcr |= UART_LCR_EPS_ODD;
		} else if (parity == H3_UART_PARITY_EVEN) {
			lcr |= UART_LCR_EPS_EVEN;
		}
	}

	if (stop_bits == H3_UART_STOP_2BITS) {
		lcr |= UART_LCR_STOP_2BITS;
	} else {
		lcr |= UART_LCR_STOP_1BIT;
	}

	dmb();
	p->O08.FCR = 0;
	p->LCR = UART_LCR_DLAB;
	p->O00.DLL = divisor & 0xFF;
	p->O04.DLH = (divisor >> 8);
	p->LCR = lcr;
	p->O08.FCR = UART_FCR_EFIFO | UART_FCR_TRESET | UART_FCR_RRESET;
	p->O04.IER = 0;
	isb();
}

void h3_uart_set_baudrate(const uint32_t uart_base, uint32_t baudrate) {
	assert((uart_base >= H3_UART0_BASE) && (uart_base <= H3_UART3_BASE));
	assert(baudrate != 0);

	H3_UART_TypeDef *p = (H3_UART_TypeDef*) (uart_base);
	const uint32_t lcr = p->LCR;

	if ((((24000000 / 16) / baudrate) > (uint16_t) (~0)) || (((24000000 / 16) / baudrate) == 0)) {
		baudrate = DEFAUlT_BAUDRATE;
	}

	const uint32_t divisor = (24000000 / 16) / baudrate;

	dmb();
	p->LCR = UART_LCR_DLAB;
	p->O00.DLL = divisor & 0xFF;
	p->O04.DLH = (divisor >> 8);
	p->LCR = lcr;
	isb();
}

void h3_uart_transmit(const uint32_t uart_base, const uint8_t *data, uint32_t length) {
	assert((uart_base >= H3_UART0_BASE) && (uart_base <= H3_UART3_BASE));
	assert(data != NULL);

	H3_UART_TypeDef *p_uart = (H3_UART_TypeDef*) (uart_base);

	while (length > 0) {
		uint32_t available = 64U - p_uart->TFL;

		while ((length > 0) && (available > 0)) {
			p_uart->O00.THR = (uint32_t) (*data);
			length--;
			available--;
			data++;
		}
	}
}

void h3_uart_transmit_string(const uint32_t uart_base, const char *data) {
	assert((uart_base >= H3_UART0_BASE) && (uart_base <= H3_UART3_BASE));
	assert(data != NULL);

	H3_UART_TypeDef *p_uart = (H3_UART_TypeDef *)(uart_base);

	while (*data != '\0') {
		uint32_t available = 64U - p_uart->TFL;

		while ((*data != '\0') && (available > 0)) {
			p_uart->O00.THR = (uint32_t) (*data);
			available--;
			data++;
		}
	}
}
