/**
 * @file dmx_multi_input.c
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
#include <string.h>
#include <assert.h>

//#if !defined (ORANGE_PI_ONE)
// #define LOGIC_ANALYZER
//#endif
//#include <stdio.h>
extern void console_error(const char *);

#include "h3_gpio.h"
#include "h3_timer.h"

#include "gpio.h"

#include "arm/arm.h"
#include "arm/synchronize.h"
#include "arm/gic.h"

#include "irq_timer.h"

#include "uart.h"

#include "dmx.h"
#include "dmx_uarts.h"
#include "dmx_multi_internal.h"

typedef enum {
	UART_STATE_IDLE,
	UART_STATE_RX
} _uart_state;

typedef enum {
	IDLE,
	PRE_BREAK,
	BREAK,
	DATA
} _dmx_state;

#ifndef ALIGNED
 #define ALIGNED __attribute__ ((aligned (4)))
#endif

extern void dmx_multi_uart_init(uint8_t uart);

static _uart_state uart_state[4] ALIGNED;

static volatile _dmx_state dmx_receive_state[4] ALIGNED;
//
static volatile struct _dmx_data dmx_data[4][DMX_DATA_BUFFER_INDEX_ENTRIES] ALIGNED;
static volatile uint32_t dmx_data_buffer_index_head[4] ALIGNED;
static volatile uint32_t dmx_data_buffer_index_tail[4] ALIGNED;
static volatile uint32_t dmx_data_index[4];
//
static volatile uint32_t dmx_updates_per_second[4] ALIGNED;
static volatile uint32_t dmx_packets[4] ALIGNED;
static volatile uint32_t dmx_packets_previous[4] ALIGNED;

const uint8_t *dmx_multi_get_available(uint8_t port)  {
	const uint32_t uart = _port_to_uart(port);

	dmb();
	if (dmx_data_buffer_index_head[uart] == dmx_data_buffer_index_tail[uart]) {
		return NULL;
	} else {
		const uint8_t *p = (const uint8_t *)dmx_data[uart][dmx_data_buffer_index_tail[uart]].data;
		dmx_data_buffer_index_tail[uart] = (dmx_data_buffer_index_tail[uart] + 1) & DMX_DATA_BUFFER_INDEX_MASK;
		return p;
	}
}

uint32_t dmx_multi_get_updates_per_seconde(uint8_t port) {
	const uint32_t uart = _port_to_uart(port);

	dmb();
	return dmx_updates_per_second[uart];
}

void fiq_dmx_in_handler(uint8_t uart, const H3_UART_TypeDef *u, uint32_t iir) {
	isb();

	if ((u->LSR & UART_LSR_BI) == UART_LSR_BI) {
		dmx_receive_state[uart] = PRE_BREAK;
#ifdef LOGIC_ANALYZER
		h3_gpio_set(GPIO_ANALYZER_CH2);	// BREAK
		h3_gpio_clr(GPIO_ANALYZER_CH3); // DATA
#endif
	}

	uint32_t rfl = u->RFL;

	while(rfl--) {
#ifdef LOGIC_ANALYZER
		h3_gpio_set(GPIO_ANALYZER_CH6); // CHL6
#endif
		while ((u->LSR & UART_LSR_DR) != UART_LSR_DR)
			;
		uint8_t data = u->O00.RBR;

		switch (dmx_receive_state[uart]) {
		case IDLE:
			return;
			break;
		case PRE_BREAK:
			dmx_receive_state[uart] = BREAK;
#ifdef LOGIC_ANALYZER
			h3_gpio_clr(GPIO_ANALYZER_CH2);	// BREAK
#endif
			break;
		case BREAK:
			switch (data) {
			case DMX512_START_CODE:
				dmx_receive_state[uart] = DATA;
				dmx_data[uart][dmx_data_buffer_index_head[uart]].data[0] = DMX512_START_CODE;
				dmx_data_index[uart] = 1;
				dmx_packets[uart]++;
#ifdef LOGIC_ANALYZER
				h3_gpio_set(GPIO_ANALYZER_CH3);	// DATA
#endif
				break;
			default:
				dmx_receive_state[uart] = IDLE;
				return;
				break;
			}
			break;
		case DATA:
			dmx_data[uart][dmx_data_buffer_index_head[uart]].data[dmx_data_index[uart]] = data;
			dmx_data_index[uart]++;

			if (dmx_data_index[uart] > DMX_MAX_CHANNELS) {
				dmx_receive_state[uart] = IDLE;
				dmx_data[uart][dmx_data_buffer_index_head[uart]].statistics.slots_in_packet = DMX_MAX_CHANNELS;
				dmx_data_buffer_index_head[uart] = (dmx_data_buffer_index_head[uart] + 1) & DMX_DATA_BUFFER_INDEX_MASK;
#ifdef LOGIC_ANALYZER
				h3_gpio_clr(GPIO_ANALYZER_CH3); // DATA
				h3_gpio_clr(GPIO_ANALYZER_CH6); // CHL6
#endif
				return;
			}
			break;
		default:
			console_error("default(state)\n");
			break;
		}
#ifdef LOGIC_ANALYZER
		h3_gpio_clr(GPIO_ANALYZER_CH6); // CHL6
#endif
	}

	if (((u->USR & UART_USR_BUSY) == 0) || ((iir & UART_IIR_IID_TIME_OUT) == UART_IIR_IID_TIME_OUT)) {
		dmx_receive_state[uart] = IDLE;
		dmx_data[uart][dmx_data_buffer_index_head[uart]].statistics.slots_in_packet = dmx_data_index[uart] - 1;
		dmx_data_buffer_index_head[uart] = (dmx_data_buffer_index_head[uart] + 1) & DMX_DATA_BUFFER_INDEX_MASK;
		dmb();
#ifdef LOGIC_ANALYZER
		h3_gpio_clr(GPIO_ANALYZER_CH3); // DATA
#endif
	}
}

static void __attribute__((interrupt("FIQ"))) fiq_dmx_multi_input(void) {
	dmb();
	uint32_t iir;

#ifdef LOGIC_ANALYZER
		h3_gpio_set(GPIO_ANALYZER_CH1);
#endif

	iir = H3_UART1->O08.IIR;
	if ((iir & UART_IIR_IID_RD) == UART_IIR_IID_RD) {
#ifdef LOGIC_ANALYZER
		h3_gpio_set(GPIO_ANALYZER_CH4);
#endif
		fiq_dmx_in_handler(1, (H3_UART_TypeDef *) H3_UART1_BASE, iir);
		H3_GIC_CPUIF->EOI = H3_UART1_IRQn;
		gic_unpend(H3_UART1_IRQn);
		isb();
#ifdef LOGIC_ANALYZER
		h3_gpio_clr(GPIO_ANALYZER_CH4);
#endif
	}

	iir = H3_UART2->O08.IIR;
	if ((iir & UART_IIR_IID_RD) == UART_IIR_IID_RD) {
#ifdef LOGIC_ANALYZER
		h3_gpio_set(GPIO_ANALYZER_CH5);
#endif
		fiq_dmx_in_handler(2, (H3_UART_TypeDef *) H3_UART2_BASE, iir);
		H3_GIC_CPUIF->EOI = H3_UART2_IRQn;
		gic_unpend(H3_UART2_IRQn);
		isb();
#ifdef LOGIC_ANALYZER
		h3_gpio_clr(GPIO_ANALYZER_CH5);
#endif
	}

#if defined (ORANGE_PI_ONE)
	iir = H3_UART3->O08.IIR;
	if (iir & UART_IIR_IID_RD) {
		fiq_dmx_in_handler(3, (H3_UART_TypeDef *) H3_UART3_BASE, iir);
		H3_GIC_CPUIF->EOI = H3_UART3_IRQn;
		gic_unpend(H3_UART3_IRQn);
		isb();
	}

	iir = H3_UART0->O08.IIR;
	if (iir & UART_IIR_IID_RD) {
		fiq_dmx_in_handler(0, (H3_UART_TypeDef *) H3_UART0_BASE, iir);
		H3_GIC_CPUIF->EOI = H3_UART0_IRQn;
		gic_unpend(H3_UART0_IRQn);
		isb();
	}
#endif

#ifdef LOGIC_ANALYZER
	h3_gpio_clr(GPIO_ANALYZER_CH1);
#endif
	dmb();
}

static void irq_timer1_dmx_receive(__attribute__((unused)) uint32_t clo) {
	uint32_t i;

	for (i = 0; i < 4; i++) {
		dmb();
		dmx_updates_per_second[i] = dmx_packets[i] - dmx_packets_previous[i];
		dmx_packets_previous[i] = dmx_packets[i];
	}
}

void dmx_multi_start_data(uint8_t port) {
	const uint32_t uart = _port_to_uart(port);

	if (uart_state[uart] == UART_STATE_RX) {
		return;
	}

	H3_UART_TypeDef *p = _get_uart(uart);
	assert(p != 0);

//#ifdef LOGIC_ANALYZER
//	h3_gpio_set(GPIO_ANALYZER_CH7);
//#endif

	while ((p->USR & UART_USR_BUSY) == UART_USR_BUSY) {
		(void) p->O00.RBR;
	}

//#ifdef LOGIC_ANALYZER
//	h3_gpio_clr(GPIO_ANALYZER_CH7);
//#endif

	p->O08.FCR = UART_FCR_EFIFO | UART_FCR_RRESET | UART_FCR_TRIG1;
	p->O04.IER = UART_IER_ERBFI;

	uart_state[uart] = UART_STATE_RX;
	dmx_receive_state[uart] = IDLE;
}

void dmx_multi_stop_data(uint8_t port) {
	const uint32_t uart = _port_to_uart(port);

	if (uart_state[uart] == UART_STATE_IDLE) {
		return;
	}

	H3_UART_TypeDef *p = _get_uart(uart);
	assert(p != 0);

	p->O08.FCR = 0;
	p->O04.IER = 0;

	uart_state[uart] = UART_STATE_IDLE;
	dmx_receive_state[uart] = IDLE;
}

void dmx_multi_input_init(void) {
	uint32_t i;

	// 0 = input, 1 = output
	h3_gpio_fsel(GPIO_DMX_DATA_DIRECTION_OUT_B, GPIO_FSEL_OUTPUT);
	h3_gpio_clr(GPIO_DMX_DATA_DIRECTION_OUT_B);
	h3_gpio_fsel(GPIO_DMX_DATA_DIRECTION_OUT_C, GPIO_FSEL_OUTPUT);
	h3_gpio_clr(GPIO_DMX_DATA_DIRECTION_OUT_C);
#if defined (ORANGE_PI_ONE)
	h3_gpio_fsel(GPIO_DMX_DATA_DIRECTION_OUT_A, GPIO_FSEL_OUTPUT);
	h3_gpio_clr(GPIO_DMX_DATA_DIRECTION_OUT_A);
	h3_gpio_fsel(GPIO_DMX_DATA_DIRECTION_OUT_D, GPIO_FSEL_OUTPUT);
	h3_gpio_clr(GPIO_DMX_DATA_DIRECTION_OUT_D);
#endif

#ifdef LOGIC_ANALYZER
	h3_gpio_fsel(GPIO_ANALYZER_CH1, GPIO_FSEL_OUTPUT);		///< FIQ
	h3_gpio_clr(GPIO_ANALYZER_CH1);
	h3_gpio_fsel(GPIO_ANALYZER_CH2, GPIO_FSEL_OUTPUT);		///< BREAK
	h3_gpio_clr(GPIO_ANALYZER_CH2);
	h3_gpio_fsel(GPIO_ANALYZER_CH3, GPIO_FSEL_OUTPUT);		///< DATA
	h3_gpio_clr(GPIO_ANALYZER_CH3);
	h3_gpio_fsel(GPIO_ANALYZER_CH4, GPIO_FSEL_OUTPUT);		///<
	h3_gpio_clr(GPIO_ANALYZER_CH4);
	h3_gpio_fsel(GPIO_ANALYZER_CH5, GPIO_FSEL_OUTPUT);		///< Interrupt UART1
	h3_gpio_clr(GPIO_ANALYZER_CH5);
	h3_gpio_fsel(GPIO_ANALYZER_CH6, GPIO_FSEL_OUTPUT);		///< Interrupt UART2
	h3_gpio_clr(GPIO_ANALYZER_CH6);
	h3_gpio_fsel(GPIO_ANALYZER_CH7, GPIO_FSEL_OUTPUT);		///< BUSY
	h3_gpio_clr(GPIO_ANALYZER_CH7);
#endif

	for (i = 0; i < DMX_MAX_UARTS; i++) {
		dmx_data_buffer_index_head[i] = 0;
		dmx_data_buffer_index_tail[i] = 0;
		memset((void *) &dmx_data[i]->statistics, 0, sizeof(struct _dmx_statistics));
		dmx_data_index[i] = 0;
		dmx_receive_state[i] = IDLE;
		//
		dmx_updates_per_second[i] = 0;
		dmx_packets[i] = 0;
		dmx_packets_previous[i] = 0;
	}

	dmx_multi_uart_init(1);
	dmx_multi_uart_init(2);
#if defined (ORANGE_PI_ONE)
	dmx_multi_uart_init(3);
 #ifndef DO_NOT_USE_UART0
	dmx_multi_uart_init(0);
 #endif
#endif

	__disable_fiq();

	arm_install_handler((unsigned) fiq_dmx_multi_input, ARM_VECTOR(ARM_VECTOR_FIQ));

	gic_fiq_config(H3_UART1_IRQn, 1);
	gic_fiq_config(H3_UART2_IRQn, 1);
#if defined (ORANGE_PI_ONE)
	gic_fiq_config(H3_UART3_IRQn, 1);
 #ifndef DO_NOT_USE_UART0
	gic_fiq_config(H3_UART0_IRQn, 1);
 #endif
#endif

	isb();
	__enable_fiq();

	irq_timer_init();

	irq_timer_set(IRQ_TIMER_1, irq_timer1_dmx_receive);
	H3_TIMER->TMR1_INTV = 0xB71B00; // 1 second
	H3_TIMER->TMR1_CTRL &= ~(TIMER_CTRL_SINGLE_MODE);
	H3_TIMER->TMR1_CTRL |= (TIMER_CTRL_EN_START | TIMER_CTRL_RELOAD);

	isb();
	__enable_irq();
}
