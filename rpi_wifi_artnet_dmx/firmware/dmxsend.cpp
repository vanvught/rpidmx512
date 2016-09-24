/**
 * @file dmxsend.cpp
 *
 */
/* Copyright (C) 2016 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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


#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "arm/synchronize.h"
#include "bcm2835.h"
#include "bcm2835_gpio.h"
#include "bcm2835_vc.h"
#include "arm/pl011.h"
#include "dmx.h"
#ifdef DEBUG
#include "monitor.h"
#endif
#include "dmxsend.h"
#include "util.h"

#ifdef __cplusplus
extern "C" {
#endif
void __attribute__((interrupt("IRQ"))) c_irq_handler(void);
void __attribute__((interrupt("FIQ"))) c_fiq_handler(void);
#ifdef __cplusplus
}
#endif

static uint32_t m_OutputBreakTime = DMX_TRANSMIT_BREAK_TIME_MIN;
static uint32_t m_OutputMabTime = DMX_TRANSMIT_MAB_TIME_MIN;
static uint32_t m_OutputPeriod = DMX_TRANSMIT_PERIOD_DEFAULT;
static uint32_t m_OutputPeriodRequested = DMX_TRANSMIT_PERIOD_DEFAULT;
static uint16_t m_OutputDataLength = DMX_UNIVERSE_SIZE + 1;

static uint8_t m_OutputBuffer[DMX_DATA_BUFFER_SIZE] ALIGNED;	///< SC + UNIVERSE SIZE

static volatile TDMXSendState m_State = DMXSendIdle;
static volatile uint32_t m_SendBreakMicros;
static volatile unsigned m_CurrentSlot;

/**
 * Timer interrupt
 */
void __attribute__((interrupt("IRQ"))) c_irq_handler(void) {
	dmb();
#if DEBUG
	bcm2835_gpio_set(21);
#endif
	const uint32_t m_TimerIRQMicros = BCM2835_ST->CLO;

	BCM2835_ST->CS = BCM2835_ST_CS_M1;

	switch (m_State) {
	case DMXSendIdle:
	case DMXSendInterPacket:
		BCM2835_ST->C1 = m_TimerIRQMicros + m_OutputBreakTime;
		BCM2835_PL011->LCRH = PL011_LCRH_WLEN8 | PL011_LCRH_STP2 | PL011_LCRH_FEN | PL011_LCRH_BRK;
		m_SendBreakMicros = m_TimerIRQMicros;
		dmb();
		m_State = DMXSendBreak;
		break;
	case DMXSendBreak:
		BCM2835_ST->C1 = m_TimerIRQMicros + m_OutputMabTime;
		BCM2835_PL011->LCRH = PL011_LCRH_WLEN8 | PL011_LCRH_STP2 | PL011_LCRH_FEN;
		dmb();
		m_State = DMXSendMAB;
		break;
	case DMXSendMAB:
		BCM2835_ST->C1 = m_SendBreakMicros + m_OutputPeriod;

		for (m_CurrentSlot = 0; !(BCM2835_PL011->FR & PL011_FR_TXFF); m_CurrentSlot++) {
			if (m_CurrentSlot >= m_OutputDataLength) {
				break;
			}

			BCM2835_PL011->DR = m_OutputBuffer[m_CurrentSlot];
		}

		if (m_CurrentSlot < m_OutputDataLength) {
			m_State = DMXSendData;
			BCM2835_PL011->IMSC = BCM2835_PL011->IMSC | PL011_IMSC_TXIM;
		} else {
			m_State = DMXSendInterPacket;
		}

		break;
	case DMXSendData:
		printf("Output period too short (brk %d, mab %d, period %d, dlen %d, slot %d)\n",
				(int)m_OutputBreakTime, (int)m_OutputMabTime, (int)m_OutputPeriod, (int)m_OutputDataLength, (int)m_CurrentSlot);
		assert (0);
		break;
	default:
		assert (0);
		break;
	}

#if DEBUG
	bcm2835_gpio_clr(21);
#endif
	dmb();
}

/**
 * PL011 TX interrupt
 */
void __attribute__((interrupt("FIQ"))) c_fiq_handler(void) {
	dmb();
#if DEBUG
	bcm2835_gpio_set(20);
#endif
	assert(m_State == DMXSendData);

	if (BCM2835_PL011->MIS == PL011_MIS_TXMIS) {

		for (; !(BCM2835_PL011->FR & PL011_FR_TXFF); m_CurrentSlot++) {
			if (m_CurrentSlot >= m_OutputDataLength) {
				break;
			}

			BCM2835_PL011->DR = m_OutputBuffer[m_CurrentSlot];
		}

		if (m_CurrentSlot >= m_OutputDataLength) {
			BCM2835_PL011->IMSC = BCM2835_PL011->IMSC & ~ PL011_IMSC_TXIM;
			m_State = DMXSendInterPacket;
		}

		BCM2835_PL011->ICR = PL011_ICR_TXIC;
	}
#if DEBUG
	bcm2835_gpio_clr(20);
#endif
	dmb();
}

/**
 *
 */
DMXSend::DMXSend(void) {
	uint32_t value;

	(void) bcm2835_vc_set_clock_rate(BCM2835_VC_CLOCK_ID_UART, 4000000);// Set UART clock rate to 4000000 (4MHz)

	dmb();

	BCM2835_PL011->CR = 0;												// Disable everything
	value = BCM2835_GPIO->GPFSEL1;
	value &= ~(7 << 12);
	value |= BCM2835_GPIO_FSEL_ALT0 << 12;								// Pin 14 PL011_TXD
	value &= ~(7 << 15);
	value |= BCM2835_GPIO_FSEL_ALT0 << 15;								// Pin 15 PL011_RXD
	BCM2835_GPIO->GPFSEL1 = value;

	bcm2835_gpio_set_pud(RPI_V2_GPIO_P1_08, BCM2835_GPIO_PUD_OFF);		// Disable pull-up/down
	bcm2835_gpio_set_pud(RPI_V2_GPIO_P1_10, BCM2835_GPIO_PUD_OFF);		// Disable pull-up/down

	dmb();

	BCM2835_PL011->IMSC = (uint32_t) 0;
	BCM2835_PL011->CR = (uint32_t) 0;

	// Poll the "flags register" to wait for the UART to stop transmitting or receiving
	while ((BCM2835_PL011->FR & PL011_FR_BUSY) != 0);

	// Flush the transmit FIFO by marking FIFOs as disabled in the "line control register"
	BCM2835_PL011->LCRH &= ~PL011_LCRH_FEN;

	// Clear all interrupt status
	BCM2835_PL011->ICR = 0x7FF;

	// UART Clock 4000000 (4MHz) , 250000 Bps
	BCM2835_PL011->IBRD = 1;
	BCM2835_PL011->FBRD = 0;

	// Set 8, N, 2, FIFO enabled
	BCM2835_PL011->LCRH = PL011_LCRH_WLEN8 | PL011_LCRH_STP2 | PL011_LCRH_FEN;

	// TX interrupt triggers when TX FIFO becomes 1/4 full
	BCM2835_PL011->IFLS = PL011_IFLS_TXIFLSEL_1_4;

	// Enable UART, TX only
	BCM2835_PL011->CR = PL011_CR_TXE | PL011_CR_UARTEN;

	dmb();

	BCM2835_ST->C1 = BCM2835_ST->CLO - 1;
	BCM2835_ST->CS = BCM2835_ST_CS_M1;

	dmb();

	BCM2835_IRQ->IRQ_ENABLE1 = BCM2835_TIMER1_IRQn;
	BCM2835_IRQ->FIQ_CONTROL = (uint32_t) BCM2835_FIQ_ENABLE | (uint32_t) INTERRUPT_VC_UART;

	dmb();
#if DEBUG
	value = BCM2835_GPIO->GPFSEL1;
	value &= ~(7 << 27);
	value |= BCM2835_GPIO_FSEL_OUTP << 27;
	BCM2835_GPIO->GPFSEL1 = value;

	value = BCM2835_GPIO->GPFSEL2;
	value &= ~(7 << 0);
	value |= BCM2835_GPIO_FSEL_OUTP << 0;
	BCM2835_GPIO->GPFSEL2 = value;

	value = BCM2835_GPIO->GPFSEL2;
	value &= ~(7 << 3);
	value |= BCM2835_GPIO_FSEL_OUTP << 3;
	BCM2835_GPIO->GPFSEL2 = value;

	bcm2835_gpio_clr(21);

	dmb();
#endif
}

/**
 *
 */
DMXSend::~DMXSend(void) {
	__disable_irq();
	__disable_fiq();

	dmb();
}

/**
 *
 */
void DMXSend::Start(void) {
	assert(m_State == DMXSendIdle);

	__enable_irq();
	__enable_fiq();

	dmb();

	BCM2835_ST->CS = BCM2835_ST_CS_M1;

	const uint32_t clo = BCM2835_ST->CLO;

	if (clo - m_SendBreakMicros > m_OutputPeriod) {
		BCM2835_ST->C1 = clo + 4;
	} else {
		BCM2835_ST->C1 = m_OutputPeriod + m_SendBreakMicros + 4;
	}

	dmb();
}

/**
 *
 */
void DMXSend::Stop(void) {

	while (m_State != DMXSendIdle) {
		if (m_State == DMXSendInterPacket) {
			dmb();

			BCM2835_ST->C1 =  BCM2835_ST->CLO - 1;
			BCM2835_ST->CS = BCM2835_ST_CS_M1;

			dmb();

			m_State = DMXSendIdle;
		}
	}

	__disable_irq();
	__disable_fiq();

	dmb();
}

/**
 *
 * @return
 */
uint16_t DMXSend::GetDataLength(void) {
	return m_OutputDataLength;
}

/**
 *
 * @param length
 */
void DMXSend::SetDataLength(uint16_t length) {
	assert(length <= DMX_DATA_BUFFER_SIZE);

	m_OutputDataLength = length;

	SetPeriodTime(m_OutputPeriodRequested);	///< Recalculate output period for new data length
}

/**
 *
 * @return
 */
uint32_t DMXSend::GetBreakTime(void) {
	return m_OutputBreakTime;
}

/**
 *
 * @param break_time
 */
void DMXSend::SetBreakTime(const uint32_t break_time) {
	m_OutputBreakTime = MAX((uint32_t) DMX_TRANSMIT_BREAK_TIME_MIN, break_time);

	SetPeriodTime(m_OutputPeriodRequested);	///< Recalculate output period for new BREAK time
}

/**
 *
 * @return
 */
uint32_t DMXSend::GetMabTime(void) {
	return m_OutputMabTime;
}

/**
 *
 * @param mab_time
 */
void DMXSend::SetMabTime(uint32_t mab_time) {
	m_OutputMabTime = MAX((uint32_t) DMX_TRANSMIT_MAB_TIME_MIN, mab_time);

	SetPeriodTime(m_OutputPeriodRequested);	///< Recalculate output period for new MAB time
}

/**
 *
 * @return
 */
uint32_t DMXSend::GetPeriodTimeRequested(void) {
	return m_OutputPeriodRequested;
}

/**
 *
 * @return
 */
uint32_t DMXSend::GetPeriodTime(void) {
	return m_OutputPeriod;
}

/**
 *
 * @param period
 */
void DMXSend::SetPeriodTime(const uint32_t period) {
	const uint32_t package_length_us = m_OutputBreakTime + m_OutputMabTime + (m_OutputDataLength * 44);

	m_OutputPeriodRequested = period;

	if (period != 0) {
		if (period < package_length_us) {
			m_OutputPeriod = (uint32_t) MAX(DMX_TRANSMIT_BREAK_TO_BREAK_TIME_MIN, package_length_us + 44);
		} else {
			m_OutputPeriod = period;
		}
	} else {
		m_OutputPeriod = (uint32_t) MAX(DMX_TRANSMIT_BREAK_TO_BREAK_TIME_MIN, package_length_us + 44);
	}
}

/**
 *
 */
void DMXSend::ClearOutputData(void) {
	for (unsigned i = 0; i < DMX_DATA_BUFFER_SIZE; i++) {
		m_OutputBuffer[i] = 0;
	}
}

/**
 *
 * @param data
 * @param length
 * @return
 */
bool DMXSend::SetDataTry(const uint8_t *data, const uint16_t length) {
	assert(length <= DMX_UNIVERSE_SIZE);

	if (m_State != DMXSendIdle && m_State != DMXSendInterPacket) {
		return false;
	}

	(void *)memcpy(&m_OutputBuffer[1], data, (size_t)length);

	SetDataLength(length+1);

	return true;
}

/**
 *
 * @param nPortId
 * @param data
 * @param length
 */
void DMXSend::SetData(const uint8_t nPortId, const uint8_t *data, const uint16_t length) {
	while (!SetDataTry (data, length)) {
		// just wait
	}
#if DEBUG
	monitor_line(MONITOR_LINE_STATS, "%d-%x:%x:%x-%d", nPortId, data[0], data[1], data[2], length);
#endif
}

