/**
 * @file dmx_multi_uart.c
 *
 */
/* Copyright (C) 2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include "h3.h"
#include "h3_gpio.h"
#include "h3_ccu.h"

#include "uart.h"

#include "arm/synchronize.h"

#include "dmx.h"

void dmx_multi_uart_init(uint8_t uart) {
	assert(uart < DMX_MAX_OUT);

	H3_UART_TypeDef *p = 0;

	if (uart == 1) {
		p = (H3_UART_TypeDef *)H3_UART1_BASE;

		uint32_t value = H3_PIO_PORTG->CFG0;
		// PG6, TX
		value &= (uint32_t) ~(GPIO_SELECT_MASK << PG6_SELECT_CFG0_SHIFT);
		value |= H3_PG6_SELECT_UART1_TX << PG6_SELECT_CFG0_SHIFT;
		// PG7, RX
		value &= (uint32_t) ~(GPIO_SELECT_MASK << PG7_SELECT_CFG0_SHIFT);
		value |= H3_PG7_SELECT_UART1_RX << PG7_SELECT_CFG0_SHIFT;
		H3_PIO_PORTG->CFG0 = value;

		H3_CCU->BUS_SOFT_RESET4 |= CCU_BUS_SOFT_RESET4_UART1;
		H3_CCU->BUS_CLK_GATING3 |= CCU_BUS_CLK_GATING3_UART1;
	} else if (uart == 2) {
		p = (H3_UART_TypeDef *)H3_UART2_BASE;

		uint32_t value = H3_PIO_PORTA->CFG0;
		// PA0, TX
		value &= (uint32_t) ~(GPIO_SELECT_MASK << PA0_SELECT_CFG0_SHIFT);
		value |= H3_PA0_SELECT_UART2_TX << PA0_SELECT_CFG0_SHIFT;
		// PA1, RX
		value &= (uint32_t) ~(GPIO_SELECT_MASK << PA1_SELECT_CFG0_SHIFT);
		value |= H3_PA1_SELECT_UART2_RX << PA1_SELECT_CFG0_SHIFT;
		H3_PIO_PORTA->CFG0 = value;

		H3_CCU->BUS_SOFT_RESET4 |= CCU_BUS_SOFT_RESET4_UART2;
		H3_CCU->BUS_CLK_GATING3 |= CCU_BUS_CLK_GATING3_UART2;
	} else if (uart == 3) {
		p = (H3_UART_TypeDef *)H3_UART3_BASE;

		uint32_t value = H3_PIO_PORTA->CFG1;
		// PA13, TX
		value &= (uint32_t) ~(GPIO_SELECT_MASK << PA13_SELECT_CFG1_SHIFT);
		value |= H3_PA13_SELECT_UART3_TX << PA13_SELECT_CFG1_SHIFT;
		// PA14, RX
		value &= (uint32_t) ~(GPIO_SELECT_MASK << PA14_SELECT_CFG1_SHIFT);
		value |= H3_PA14_SELECT_UART3_RX << PA14_SELECT_CFG1_SHIFT;
		H3_PIO_PORTA->CFG1 = value;

		H3_CCU->BUS_SOFT_RESET4 |= CCU_BUS_SOFT_RESET4_UART3;
		H3_CCU->BUS_CLK_GATING3 |= CCU_BUS_CLK_GATING3_UART3;
	} else if (uart == 0) {
		p = (H3_UART_TypeDef *)H3_UART0_BASE;

		uint32_t value = H3_PIO_PORTA->CFG0;
		// PA4, TX
		value &= (uint32_t) ~(GPIO_SELECT_MASK << PA4_SELECT_CFG0_SHIFT);
		value |= H3_PA4_SELECT_UART0_TX << PA4_SELECT_CFG0_SHIFT;
		// PA5, RX
		value &= (uint32_t) ~(GPIO_SELECT_MASK << PA5_SELECT_CFG0_SHIFT);
		value |= H3_PA5_SELECT_UART0_RX << PA5_SELECT_CFG0_SHIFT;
		H3_PIO_PORTA->CFG0 = value;

		H3_CCU->BUS_SOFT_RESET4 |= CCU_BUS_SOFT_RESET4_UART0;
		H3_CCU->BUS_CLK_GATING3 |= CCU_BUS_CLK_GATING3_UART0;
	} else {
		assert(0);
	}

	assert(p != 0);

	p->O08.FCR = 0;
	p->LCR = UART_LCR_DLAB;
	p->O00.DLL = BAUD_250000_L;
	p->O04.DLH = BAUD_250000_H;
	p->O04.IER = 0;
	p->LCR = UART_LCR_8_N_2;

	isb();
}
