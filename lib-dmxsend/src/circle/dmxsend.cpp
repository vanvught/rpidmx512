/**
 * @file dmxsend.cpp
 *
 */
/*
 * Circle - A C++ bare metal environment for Raspberry Pi
 * Copyright (C) 2016  R. Stange <rsta2@o2online.de>
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

#include <stdint.h>
#include <assert.h>

#include <circle/bcm2835.h>
#include <circle/memio.h>
#include <circle/logger.h>
#include <circle/bcmpropertytags.h>
#include <circle/synchronize.h>
#include <circle/util.h>
#include <circle/debug.h>

#include "circle/dmxsend.h"

#define DMX512_START_CODE						0		///< The start code for DMX512 data. This is often referred to as NSC for "Null Start Code".

#define DMX_TRANSMIT_BREAK_TIME_MIN				92		///< 92 us
#define DMX_TRANSMIT_BREAK_TIME_TYPICAL			176		///< 176 us
#define DMX_TRANSMIT_MAB_TIME_MIN				12		///< 12 us
#define DMX_TRANSMIT_MAB_TIME_MAX				1E6		///< 1s
#define DMX_TRANSMIT_REFRESH_RATE_DEFAULT		40		///< 40 Hz
#define DMX_TRANSMIT_PERIOD_DEFAULT				(uint32_t)(1E6 / DMX_TRANSMIT_REFRESH_RATE_DEFAULT)	///< 25000 us
#define DMX_TRANSMIT_BREAK_TO_BREAK_TIME_MIN	1204	///< us

#define DMX_DATA_BUFFER_SIZE	513						///< including SC
#define MAX(a,b)				(((a) > (b)) ? (a) : (b))
#define GPIO_DMX_DATA_DIRECTION	18						///<  RPI_V2_GPIO_P1_12

// UART0
#define DR_OE_MASK		(1 << 11)
#define DR_BE_MASK		(1 << 10)
#define DR_PE_MASK		(1 << 9)
#define DR_FE_MASK		(1 << 8)

#define FR_TXFE_MASK		(1 << 7)
#define FR_RXFF_MASK		(1 << 6)
#define FR_TXFF_MASK		(1 << 5)
#define FR_RXFE_MASK		(1 << 4)
#define FR_BUSY_MASK		(1 << 3)

#define LCRH_SPS_MASK		(1 << 7)
#define LCRH_WLEN8_MASK		(3 << 5)
#define LCRH_WLEN7_MASK		(2 << 5)
#define LCRH_WLEN6_MASK		(1 << 5)
#define LCRH_WLEN5_MASK		(0 << 5)
#define LCRH_FEN_MASK		(1 << 4)
#define LCRH_STP2_MASK		(1 << 3)
#define LCRH_EPS_MASK		(1 << 2)
#define LCRH_PEN_MASK		(1 << 1)
#define LCRH_BRK_MASK		(1 << 0)

#define CR_CTSEN_MASK		(1 << 15)
#define CR_RTSEN_MASK		(1 << 14)
#define CR_OUT2_MASK		(1 << 13)
#define CR_OUT1_MASK		(1 << 12)
#define CR_RTS_MASK		(1 << 11)
#define CR_DTR_MASK		(1 << 10)
#define CR_RXE_MASK		(1 << 9)
#define CR_TXE_MASK		(1 << 8)
#define CR_LBE_MASK		(1 << 7)
#define CR_UART_EN_MASK		(1 << 0)

#define IFLS_RXIFSEL_SHIFT	3
#define IFLS_RXIFSEL_MASK	(7 << IFLS_RXIFSEL_SHIFT)
#define IFLS_TXIFSEL_SHIFT	0
#define IFLS_TXIFSEL_MASK	(7 << IFLS_TXIFSEL_SHIFT)
	#define IFLS_IFSEL_1_8		0
	#define IFLS_IFSEL_1_4		1
	#define IFLS_IFSEL_1_2		2
	#define IFLS_IFSEL_3_4		3
	#define IFLS_IFSEL_7_8		4

#define INT_OE			(1 << 10)
#define INT_BE			(1 << 9)
#define INT_PE			(1 << 8)
#define INT_FE			(1 << 7)
#define INT_RT			(1 << 6)
#define INT_TX			(1 << 5)
#define INT_RX			(1 << 4)
#define INT_DSRM		(1 << 3)
#define INT_DCDM		(1 << 2)
#define INT_CTSM		(1 << 1)

//#define MAX_IRQ_LATENCY		4		// not for release version

static const char FromDMXSend[] = "dmxsend";

/**
 *
 * @param pInterruptSystem
 */
DMXSend::DMXSend(CInterruptSystem *pInterruptSystem) :
		m_pInterruptSystem (pInterruptSystem),
		m_bIRQsConnected (FALSE),
#if RASPPI == 3
		// to be sure there is no collision with the Bluetooth controller
		m_GPIO32 (32, GPIOModeInput),
		m_GPIO33 (33, GPIOModeInput),
#endif
		m_TxDPin (14, GPIOModeAlternateFunction0),
		m_RxDPin (15, GPIOModeAlternateFunction0),
		m_DMXDataDirection (GPIO_DMX_DATA_DIRECTION, GPIOModeOutput),
		m_OutputBreakTime(DMX_TRANSMIT_BREAK_TIME_MIN),
		m_OutputMabTime(DMX_TRANSMIT_MAB_TIME_MIN),
		m_OutputPeriod(DMX_TRANSMIT_PERIOD_DEFAULT),
		m_OutputPeriodRequested(DMX_TRANSMIT_PERIOD_DEFAULT),
		m_OutputDataLength(DMX_UNIVERSE_SIZE + 1),
		m_State (DMXSendIdle)
{
	ClearOutputData();
}

DMXSend::~DMXSend(void) {
	assert(m_State == DMXSendIdle);

	if (m_bIRQsConnected) {
		assert(m_pInterruptSystem != 0);
		m_pInterruptSystem->DisconnectIRQ(ARM_IRQ_TIMER1);
		m_pInterruptSystem->DisconnectIRQ(ARM_IRQ_UART);
	}

	m_pInterruptSystem = 0;
}

boolean DMXSend::Initialize (void)
{
	CBcmPropertyTags Tags;
	TPropertyTagSetClockRate TagSetClockRate;
	TagSetClockRate.nClockId = CLOCK_ID_UART;
	TagSetClockRate.nRate = 4000000;
	TagSetClockRate.nSkipSettingTurbo = 0;
	if (!Tags.GetTag (PROPTAG_SET_CLOCK_RATE, &TagSetClockRate, sizeof TagSetClockRate, 12))
	{
		CLogger::Get ()->Write (FromDMXSend, LogError, "Cannot set UART clock rate");

		return TRUE;
	}

	// initialize UART
	assert (m_pInterruptSystem != 0);
	m_pInterruptSystem->ConnectIRQ (ARM_IRQ_UART, SerialIRQHandler, this);

	DataMemBarrier ();

	write32 (ARM_UART0_IMSC, 0);
	write32 (ARM_UART0_CR, 0);

	// Flush the transmit FIFO by marking FIFOs as disabled in the "line control register"
	write32 (ARM_UART0_LCRH, read32 (ARM_UART0_LCRH) & ~LCRH_FEN_MASK);

	// Clear all interrupt status
	write32 (ARM_UART0_ICR, 0x7FF);

	// set 250000 Bps
	write32 (ARM_UART0_IBRD, 1);
	write32 (ARM_UART0_FBRD, 0);

	// set 8, N, 2, FIFO enabled
	write32 (ARM_UART0_LCRH, LCRH_WLEN8_MASK | LCRH_STP2_MASK | LCRH_FEN_MASK);

	// TX interrupt triggers when TX FIFO becomes 1/4 full
	write32 (ARM_UART0_IFLS, IFLS_IFSEL_1_4 << IFLS_TXIFSEL_SHIFT);

	// Enable UART, TX only
	write32 (ARM_UART0_CR, CR_TXE_MASK | CR_UART_EN_MASK);

	DataMemBarrier ();

	// initialize timer 1
	write32 (ARM_SYSTIMER_C1, read32 (ARM_SYSTIMER_CLO)-1);
	write32 (ARM_SYSTIMER_CS, 1 << 1);
	
	DataMemBarrier ();

	m_pInterruptSystem->ConnectIRQ (ARM_IRQ_TIMER1, TimerIRQHandler, this);
	m_bIRQsConnected = TRUE;

	m_DMXDataDirection.Write (HIGH);

	return TRUE;
}

void DMXSend::Start(uint8_t nPort) {
	assert (m_State == DMXSendIdle);

	DataMemBarrier ();

	write32 (ARM_SYSTIMER_CS, 1 << 1);

	const uint32_t clo = read32 (ARM_SYSTIMER_CLO);
	if (clo - m_SendBreakMicros > m_OutputPeriod) {
		write32 (ARM_SYSTIMER_C1, clo + 4);
	} else {
		write32 (ARM_SYSTIMER_C1, m_OutputPeriod + m_SendBreakMicros + 4);
	}

	DataMemBarrier ();
}

void DMXSend::Stop(uint8_t nPort) {
	while (m_State != DMXSendIdle)
	{
		m_SpinLock.Acquire ();

		if (m_State == DMXSendInterPacket)
		{
			DataMemBarrier ();

			write32 (ARM_SYSTIMER_C1, read32 (ARM_SYSTIMER_CLO)-1);
			write32 (ARM_SYSTIMER_CS, 1 << 1);

			DataMemBarrier ();

			m_State = DMXSendIdle;
		}

		m_SpinLock.Release ();
	}
}

void DMXSend::SetData(/* unused */uint8_t nPortId, const uint8_t *data, uint16_t length) {
	while (!SetDataTry (data, length)) {
		// just wait
	}
}

boolean DMXSend::SetDataTry(const uint8_t *data, uint16_t length) {
	assert(length <= DMX_UNIVERSE_SIZE);

	m_SpinLock.Acquire ();

	if (m_State != DMXSendIdle && m_State != DMXSendInterPacket) {
		m_SpinLock.Release ();

		return FALSE;
	}

	m_OutputBuffer[0] = DMX512_START_CODE;
	(void *)memcpy(&m_OutputBuffer[1], data, (size_t)length);

	SetDataLength(length+1);

	m_SpinLock.Release ();

	return TRUE;
}

uint16_t DMXSend::GetDataLength(void) const {
	return m_OutputDataLength;
}

void DMXSend::SetDataLength(uint16_t length) {
	assert(length <= DMX_DATA_BUFFER_SIZE);

	m_OutputDataLength = length;

	SetDmxPeriodTime(m_OutputPeriodRequested);	///< Recalculate output period for new data length
}

uint32_t DMXSend::GetDmxBreakTime(void) const {
	return m_OutputBreakTime;
}

void DMXSend::SetDmxBreakTime(uint32_t break_time) {
	m_OutputBreakTime = MAX((uint32_t)DMX_TRANSMIT_BREAK_TIME_MIN, break_time);

	SetDmxPeriodTime(m_OutputPeriodRequested);	///< Recalculate output period for new BREAK time
}

uint32_t DMXSend::GetDmxMabTime(void) const {
	return m_OutputMabTime;
}

void DMXSend::SetDmxMabTime(uint32_t mab_time) {
	m_OutputMabTime = MAX((uint32_t)DMX_TRANSMIT_MAB_TIME_MIN, mab_time);

	SetDmxPeriodTime(m_OutputPeriodRequested);	///< Recalculate output period for new MAB time
}

uint32_t DMXSend::GetDmxPeriodTimeRequested(void) const {
	return m_OutputPeriodRequested;
}


uint32_t DMXSend::GetDmxPeriodTime(void) const {
	return m_OutputPeriod;
}

void DMXSend::SetDmxPeriodTime(uint32_t period) {
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

void DMXSend::ClearOutputData(void) {
	m_OutputBuffer[0] = DMX512_START_CODE;
	for (unsigned i = 1; i < (sizeof m_OutputBuffer / sizeof m_OutputBuffer[0]); i++) {
		m_OutputBuffer[i] = 0;
	}
}

void DMXSend::SerialIRQHandler(void) {
#ifdef DEBUG_CLICK
	debug_click (DEBUG_CLICK_LEFT);
#endif

	DataMemBarrier ();

	assert(m_State == DMXSendData);
	uint32_t Mis = read32(ARM_UART0_MIS);
	if (Mis != INT_TX) {
		CLogger::Get()->Write(FromDMXSend, LogPanic, "Unexpected interrupt (mis 0x%X)", (unsigned) Mis);
	}

	m_SpinLock.Acquire();

	for (; !(read32(ARM_UART0_FR) & FR_TXFF_MASK); m_CurrentSlot++) {
		if (m_CurrentSlot >= m_OutputDataLength) {
			break;
		}

		write32(ARM_UART0_DR, m_OutputBuffer[m_CurrentSlot]);
	}

	if (m_CurrentSlot >= m_OutputDataLength) {
		write32(ARM_UART0_IMSC, read32(ARM_UART0_IMSC) & ~INT_TX);

		m_State = DMXSendInterPacket;
	}

	write32(ARM_UART0_ICR, INT_TX);

	DataMemBarrier ();

	m_SpinLock.Release();
}

void DMXSend::SerialIRQHandler(void *pParam) {
	DMXSend *pThis = (DMXSend *) pParam;
	assert(pThis != 0);

	pThis->SerialIRQHandler();
}

void DMXSend::TimerIRQHandler(void) {
#ifdef DEBUG_CLICK
	debug_click (DEBUG_CLICK_RIGHT);
#endif

	m_SpinLock.Acquire();

	DataMemBarrier ();

	m_TimerIRQMicros = read32(ARM_SYSTIMER_CLO);

#ifdef MAX_IRQ_LATENCY
	unsigned Latency = m_TimerIRQMicros - read32 (ARM_SYSTIMER_C1);
	if (Latency > MAX_IRQ_LATENCY)
	{
		CLogger::Get ()->Write (FromDMXSend, LogPanic,
								"Allowed IRQ latency exceeded (lat %u, state %u)", Latency, m_State);
	}
#endif

	switch (m_State) {
	case DMXSendIdle:
	case DMXSendInterPacket:
		write32 (ARM_UART0_LCRH, read32 (ARM_UART0_LCRH) | LCRH_BRK_MASK);
		write32 (ARM_SYSTIMER_C1, m_TimerIRQMicros + m_OutputBreakTime);
		m_SendBreakMicros = m_TimerIRQMicros;
		m_State = DMXSendBreak;
		break;
	case DMXSendBreak:
		write32 (ARM_UART0_LCRH, read32 (ARM_UART0_LCRH) & ~LCRH_BRK_MASK);
		write32 (ARM_SYSTIMER_C1, m_TimerIRQMicros + m_OutputMabTime);
		m_State = DMXSendMAB;
		break;
	case DMXSendMAB:
		write32(ARM_SYSTIMER_C1, m_SendBreakMicros + m_OutputPeriod);
		for (m_CurrentSlot = 0; !(read32(ARM_UART0_FR) & FR_TXFF_MASK); m_CurrentSlot++) {
			if (m_CurrentSlot >= m_OutputDataLength) {
				break;
			}

			write32(ARM_UART0_DR, m_OutputBuffer[m_CurrentSlot]);
		}
		if (m_CurrentSlot < m_OutputDataLength) {
			m_State = DMXSendData;
			write32(ARM_UART0_IMSC, read32(ARM_UART0_IMSC) | INT_TX);
		} else {
			m_State = DMXSendInterPacket;
		}
		break;
	case DMXSendData:
		CLogger::Get ()->Write (FromDMXSend, LogPanic,
								"Output period too short (brk %u, mab %u, period %u, dlen %u, slot %u)",
								m_OutputBreakTime, m_OutputMabTime, m_OutputPeriod,
								(unsigned) m_OutputDataLength, m_CurrentSlot);
		break;
	default:
		assert (0);
		break;
	}

	write32 (ARM_SYSTIMER_CS, 1 << 1);

	DataMemBarrier ();

	m_SpinLock.Release ();
}

void DMXSend::TimerIRQHandler(void *pParam) {
	DMXSend *pThis = (DMXSend *) pParam;
	assert(pThis != 0);

	pThis->TimerIRQHandler();
}
