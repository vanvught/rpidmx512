/**
 * @file dmx_multiinput.cpp
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

#include "h3/dmxmultiinput.h"
#include "h3/dmxmulti.h"

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

extern "C" {
 int console_error(const char *);
}

enum class DmxState {
	IDLE,
	PRE_BREAK,
	BREAK,
	DATA
} ;

#ifndef ALIGNED
# define ALIGNED __attribute__ ((aligned (4)))
#endif

static volatile DmxState s_DmxReceiveState[DMX_MAX_IN] ALIGNED;
//
static volatile struct _dmx_data s_aDmxData[DMX_MAX_IN][DMX_DATA_BUFFER_INDEX_ENTRIES] ALIGNED;
static volatile uint32_t s_nDmxDataBufferIndexHead[DMX_MAX_IN] ALIGNED;
static volatile uint32_t s_nDmxDataBufferIndexTail[DMX_MAX_IN] ALIGNED;
static volatile uint32_t s_nDmxDataIndex[DMX_MAX_IN];
//
static volatile uint32_t s_nDmxUpdatesPerSecond[DMX_MAX_IN] ALIGNED;
static volatile uint32_t s_nDmxPackets[DMX_MAX_IN] ALIGNED;
static volatile uint32_t s_nDmxPacketsPrevious[DMX_MAX_IN] ALIGNED;

const uint8_t *DmxMultiInput::GetDmxAvailable(uint32_t nPort)  {
	const auto nUart = _port_to_uart(nPort);

	dmb();
	if (s_nDmxDataBufferIndexHead[nUart] == s_nDmxDataBufferIndexTail[nUart]) {
		return nullptr;
	} else {
		const auto *p = const_cast<const uint8_t *>(s_aDmxData[nUart][s_nDmxDataBufferIndexTail[nUart]].data);
		s_nDmxDataBufferIndexTail[nUart] = (s_nDmxDataBufferIndexTail[nUart] + 1) & DMX_DATA_BUFFER_INDEX_MASK;
		return p;
	}
}

uint32_t DmxMultiInput::GetUpdatesPerSeconde(uint32_t nPort) {
	const auto uart = _port_to_uart(nPort);

	dmb();
	return s_nDmxUpdatesPerSecond[uart];
}

void fiq_dmx_in_handler(uint32_t uart, const H3_UART_TypeDef *u, uint32_t iir) {
	isb();

	if ((u->LSR & UART_LSR_BI) == UART_LSR_BI) {
		s_DmxReceiveState[uart] = DmxState::PRE_BREAK;
#ifdef LOGIC_ANALYZER
		h3_gpio_set(GPIO_ANALYZER_CH2);	// BREAK
		h3_gpio_clr(GPIO_ANALYZER_CH3); // DATA
#endif
	}

	auto rfl = u->RFL;

	while(rfl--) {
#ifdef LOGIC_ANALYZER
		h3_gpio_set(GPIO_ANALYZER_CH6); // CHL6
#endif
		while ((u->LSR & UART_LSR_DR) != UART_LSR_DR)
			;
		uint8_t data = u->O00.RBR;

		switch (s_DmxReceiveState[uart]) {
		case DmxState::IDLE:
			return;
			break;
		case DmxState::PRE_BREAK:
			s_DmxReceiveState[uart] = DmxState::BREAK;
#ifdef LOGIC_ANALYZER
			h3_gpio_clr(GPIO_ANALYZER_CH2);	// BREAK
#endif
			break;
		case DmxState::BREAK:
			switch (data) {
			case DMX512_START_CODE:
				s_DmxReceiveState[uart] = DmxState::DATA;
				s_aDmxData[uart][s_nDmxDataBufferIndexHead[uart]].data[0] = DMX512_START_CODE;
				s_nDmxDataIndex[uart] = 1;
				s_nDmxPackets[uart]++;
#ifdef LOGIC_ANALYZER
				h3_gpio_set(GPIO_ANALYZER_CH3);	// DATA
#endif
				break;
			default:
				s_DmxReceiveState[uart] = DmxState::IDLE;
				return;
				break;
			}
			break;
		case DmxState::DATA:
			s_aDmxData[uart][s_nDmxDataBufferIndexHead[uart]].data[s_nDmxDataIndex[uart]] = data;
			s_nDmxDataIndex[uart]++;

			if (s_nDmxDataIndex[uart] > DMX_MAX_CHANNELS) {
				s_DmxReceiveState[uart] = DmxState::IDLE;
				s_aDmxData[uart][s_nDmxDataBufferIndexHead[uart]].statistics.slots_in_packet = DMX_MAX_CHANNELS;
				s_nDmxDataBufferIndexHead[uart] = (s_nDmxDataBufferIndexHead[uart] + 1) & DMX_DATA_BUFFER_INDEX_MASK;
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
		s_DmxReceiveState[uart] = DmxState::IDLE;
		s_aDmxData[uart][s_nDmxDataBufferIndexHead[uart]].statistics.slots_in_packet = s_nDmxDataIndex[uart] - 1;
		s_nDmxDataBufferIndexHead[uart] = (s_nDmxDataBufferIndexHead[uart] + 1) & DMX_DATA_BUFFER_INDEX_MASK;
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
		fiq_dmx_in_handler(1, reinterpret_cast<H3_UART_TypeDef *>(H3_UART1_BASE), iir);
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
		fiq_dmx_in_handler(2, reinterpret_cast<H3_UART_TypeDef *>(H3_UART2_BASE), iir);
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
		fiq_dmx_in_handler(3, reinterpret_cast<H3_UART_TypeDef *>(H3_UART3_BASE), iir);
		H3_GIC_CPUIF->EOI = H3_UART3_IRQn;
		gic_unpend(H3_UART3_IRQn);
		isb();
	}

	iir = H3_UART0->O08.IIR;
	if (iir & UART_IIR_IID_RD) {
		fiq_dmx_in_handler(0, reinterpret_cast<H3_UART_TypeDef *>(H3_UART0_BASE), iir);
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
	dmb();
	for (uint32_t i = 0; i < DMX_MAX_IN; i++) {
		s_nDmxUpdatesPerSecond[i] = s_nDmxPackets[i] - s_nDmxPacketsPrevious[i];
		s_nDmxPacketsPrevious[i] = s_nDmxPackets[i];
	}
}

void DmxMultiInput::StartData(uint32_t nPort) {
	const auto nUart = _port_to_uart(nPort);

	if (m_StateUart[nUart] == UartState::RX) {
		return;
	}

	auto *p = _get_uart(nUart);
	assert(p != nullptr);

	while ((p->USR & UART_USR_BUSY) == UART_USR_BUSY) {
		(void) p->O00.RBR;
	}

	p->O08.FCR = UART_FCR_EFIFO | UART_FCR_RRESET | UART_FCR_TRIG1;
	p->O04.IER = UART_IER_ERBFI;

	m_StateUart[nUart] = UartState::RX;
	s_DmxReceiveState[nUart] = DmxState::IDLE;
}

void DmxMultiInput::StopData(uint32_t nPort) {
	const auto nUart = _port_to_uart(nPort);

	if (m_StateUart[nUart] == UartState::IDLE) {
		return;
	}

	auto *p = _get_uart(nUart);
	assert(p != 0);

	p->O08.FCR = 0;
	p->O04.IER = 0;

	m_StateUart[nUart] = UartState::IDLE;
	s_DmxReceiveState[nUart] = DmxState::IDLE;
}

DmxMultiInput::DmxMultiInput() {
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

	for (i = 0; i < DMX_MAX_IN; i++) {
		s_nDmxDataBufferIndexHead[i] = 0;
		s_nDmxDataBufferIndexTail[i] = 0;

		auto *p = reinterpret_cast<volatile uint8_t *>(&s_aDmxData[i]->statistics);
		for (uint32_t j = 0; j < sizeof(struct _dmx_statistics); j++) {
			*p++ = 0;
		}

		s_nDmxDataIndex[i] = 0;
		s_DmxReceiveState[i] = DmxState::IDLE;
		//
		s_nDmxUpdatesPerSecond[i] = 0;
		s_nDmxPackets[i] = 0;
		s_nDmxPacketsPrevious[i] = 0;
	}

	DmxMulti::UartInit(1);
	DmxMulti::UartInit(2);
#if defined (ORANGE_PI_ONE)
	DmxMulti::UartInit(3);
# ifndef DO_NOT_USE_UART0
	DmxMulti::UartInit(0);
# endif
#endif

	__disable_fiq();

	arm_install_handler(reinterpret_cast<unsigned>(fiq_dmx_multi_input), ARM_VECTOR(ARM_VECTOR_FIQ));

	gic_fiq_config(H3_UART1_IRQn, GIC_CORE0);
	gic_fiq_config(H3_UART2_IRQn, GIC_CORE0);
#if defined (ORANGE_PI_ONE)
	gic_fiq_config(H3_UART3_IRQn, GIC_CORE0);
# ifndef DO_NOT_USE_UART0
	gic_fiq_config(H3_UART0_IRQn, GIC_CORE0);
# endif
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
