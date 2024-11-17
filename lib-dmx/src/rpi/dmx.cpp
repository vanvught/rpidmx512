/**
 * @file dmx.cpp
 *
 */
/* Copyright (C) 2020-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#if __GNUC__ > 8
# pragma GCC target ("general-regs-only")
#endif

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <algorithm>
#include <cassert>

#include "dmx.h"
#include "dmxconst.h"
#include "rdm.h"
#include "rdm_e120.h"

#include "arm/arm.h"
#include "arm/synchronize.h"
#include "arm/pl011.h"
#include "irq_timer.h"

#include "bcm2835.h"
#include "bcm2835_st.h"
#include "bcm2835_gpio.h"
#include "bcm2835_vc.h"

#include "debug.h"

#if (GPIO_DMX_DATA_DIRECTION != RPI_V2_GPIO_P1_12)
# error
#endif

#ifndef ALIGNED
# define ALIGNED __attribute__ ((aligned (4)))
#endif

typedef enum {
	IDLE = 0,
	PRE_BREAK,
	BREAK,
	MAB,
	DMXDATA,
	RDMDATA,
	CHECKSUMH,
	CHECKSUML,
	RDMDISCFE,
	RDMDISCEUID,
	RDMDISCECS,
	DMXINTER
} _dmx_state;

using namespace dmx;

static PortDirection s_nPortDirection = dmx::PortDirection::INP;

// DMX

static volatile uint32_t sv_nDmxDataBufferIndexHead;
static volatile uint32_t sv_nDmxDataBufferIndexTail;
static struct Data s_DmxData[buffer::INDEX_ENTRIES] ALIGNED;
static uint8_t s_DmxDataPrevious[buffer::SIZE] ALIGNED;
static volatile _dmx_state sv_DmxReceiveState = IDLE;
static volatile uint32_t sv_nDmxDataIndex;

static uint32_t s_nDmxTransmitBreakTime { dmx::transmit::BREAK_TIME_MIN };
static uint32_t s_nDmxTransmitMabTime { dmx::transmit::MAB_TIME_MIN };
static uint32_t s_nDmxTransmitPeriod { dmx::transmit::PERIOD_DEFAULT };

static uint32_t s_nDmxSendDataLength = (dmx::max::CHANNELS + 1);		///< SC + UNIVERSE SIZE
static volatile uint32_t sv_nFiqMicrosCurrent;
static volatile uint32_t sv_nFiqMicrosPrevious;
static volatile bool sv_isDmxPreviousBreak = false;
static volatile uint32_t sv_DmxBreakToBreakLatest;
static volatile uint32_t sv_DmxBreakToBreakPrevious;
static volatile uint32_t sv_DmxSlotsInPacketPrevious;
static volatile _dmx_state sv_DmxTransmitState = IDLE;
static volatile bool sv_doDmxTransmitAlways = false;
static volatile uint32_t sv_DmxTransmitBreakMicros;
static volatile uint32_t sv_DmxTransmitCurrentSlot;
static bool s_IsStopped = true;

static volatile uint32_t sv_nDmxUpdatesPerSecond;
static volatile uint32_t sv_nDmxPacketsPrevious;
static volatile struct TotalStatistics sv_TotalStatistics ALIGNED;

// RDM

static volatile uint32_t sv_nRdmDataBufferIndexHead;
static volatile uint32_t sv_nRdmDataBufferIndexTail;
static uint8_t s_RdmData[RDM_DATA_BUFFER_INDEX_ENTRIES][RDM_DATA_BUFFER_SIZE] ALIGNED;
static volatile uint16_t sv_nRdmChecksum;	///< This must be uint16_t
volatile uint32_t gsv_RdmDataReceiveEnd;
static volatile uint32_t sv_RdmDiscIndex;

/**
 * Timer 1 interrupt DMX Receiver
 * Slot time-out
 */
static void irq_timer1_dmx_receive(uint32_t clo) {
	dmb();
	if (sv_DmxReceiveState == DMXDATA) {
		if (clo - sv_nFiqMicrosCurrent > s_DmxData[0].Statistics.nSlotToSlot) {
			dmb();
			sv_DmxReceiveState = IDLE;
			s_DmxData[sv_nDmxDataBufferIndexHead].Statistics.nSlotsInPacket = sv_nDmxDataIndex - 1;
			sv_nDmxDataBufferIndexHead = (sv_nDmxDataBufferIndexHead + 1) & buffer::INDEX_MASK;
#ifdef LOGIC_ANALYZER
			bcm2835_gpio_clr(GPIO_ANALYZER_CH3);	// DMX DATA
			bcm2835_gpio_set(GPIO_ANALYZER_CH4);	// IDLE
#endif
		} else {
			BCM2835_ST->C1 = clo + s_DmxData[sv_nDmxDataBufferIndexHead].Statistics.nSlotToSlot;
		}
	}
}

/**
 * Timer 1 interrupt DMX Sender
 */
static void irq_timer1_dmx_sender(uint32_t clo) {
	switch (sv_DmxTransmitState) {
	case IDLE:
	case DMXINTER:
		BCM2835_ST->C1 = clo + s_nDmxTransmitBreakTime;
		BCM2835_PL011->LCRH = PL011_LCRH_WLEN8 | PL011_LCRH_STP2 | PL011_LCRH_FEN | PL011_LCRH_BRK;
		sv_DmxTransmitBreakMicros = clo;
		dmb();
		sv_DmxTransmitState = BREAK;
		break;
	case BREAK:
		BCM2835_ST->C1 = clo + s_nDmxTransmitMabTime;
		BCM2835_PL011->LCRH = PL011_LCRH_WLEN8 | PL011_LCRH_STP2 | PL011_LCRH_FEN;
		dmb();
		sv_DmxTransmitState = MAB;
		break;
	case MAB: {
		BCM2835_ST->C1 = sv_DmxTransmitBreakMicros + s_nDmxTransmitPeriod;

		for (sv_DmxTransmitCurrentSlot = 0; !(BCM2835_PL011->FR & PL011_FR_TXFF); sv_DmxTransmitCurrentSlot++) {
			if (sv_DmxTransmitCurrentSlot >= s_nDmxSendDataLength) {
				break;
			}

			BCM2835_PL011->DR = s_DmxData[0].Data[sv_DmxTransmitCurrentSlot];
		}

		if (sv_DmxTransmitCurrentSlot < s_nDmxSendDataLength) {
			dmb();
			sv_DmxTransmitState = DMXDATA;
			BCM2835_PL011->IMSC = PL011_IMSC_TXIM;
		} else {
			dmb();
			sv_DmxTransmitState = DMXINTER;
		}
	}
		break;
	case DMXDATA:
		printf("Output period too short (dlen %d, slot %d)\n", s_nDmxSendDataLength, sv_DmxTransmitCurrentSlot);
		assert(0);
		break;
	default:
		assert(0);
		break;
	}
}

/**
 * Timer 3 interrupt DMX Receiver
 * Statistics
 */
static void irq_timer3_dmx_receive(uint32_t clo) {
	BCM2835_ST->C3 = clo + (uint32_t) 1000000;
	dmb();
	sv_nDmxUpdatesPerSecond = sv_TotalStatistics.nDmxPackets - sv_nDmxPacketsPrevious;
	sv_nDmxPacketsPrevious = sv_TotalStatistics.nDmxPackets;
}

/**
 * Interrupt handler for continues receiving DMX512 data.
 */
static void fiq_dmx_in_handler(void) {
	sv_nFiqMicrosCurrent = BCM2835_ST->CLO;

	const auto dr = BCM2835_PL011->DR;

	if (dr & PL011_DR_BE) {
		sv_DmxReceiveState = BREAK;
		sv_DmxBreakToBreakLatest = sv_nFiqMicrosCurrent;
	} else {
		const auto data = static_cast<uint8_t>(dr & 0xFF);

		switch (sv_DmxReceiveState) {
		case IDLE:
			if (data == 0xFE) {
				sv_DmxReceiveState = RDMDISCFE;
				s_RdmData[sv_nRdmDataBufferIndexHead][0] = 0xFE;
				sv_nDmxDataIndex = 1;
			}
			break;
		case BREAK:
			switch (data) {
			case START_CODE:
				sv_DmxReceiveState = DMXDATA;
				s_DmxData[sv_nDmxDataBufferIndexHead].Data[0] = START_CODE;
				sv_nDmxDataIndex = 1;
				sv_TotalStatistics.nDmxPackets = sv_TotalStatistics.nDmxPackets + 1;

				if (sv_isDmxPreviousBreak) {
					s_DmxData[sv_nDmxDataBufferIndexHead].Statistics.nBreakToBreak = sv_DmxBreakToBreakLatest - sv_DmxBreakToBreakPrevious;
					sv_DmxBreakToBreakPrevious = sv_DmxBreakToBreakLatest;

				} else {
					sv_isDmxPreviousBreak = true;
					sv_DmxBreakToBreakPrevious = sv_DmxBreakToBreakLatest;
				}
				break;
			case E120_SC_RDM:
				sv_DmxReceiveState = RDMDATA;
				s_RdmData[sv_nRdmDataBufferIndexHead][0] = E120_SC_RDM;
				sv_nRdmChecksum = E120_SC_RDM;
				sv_nDmxDataIndex = 1;
				sv_TotalStatistics.nRdmPackets = sv_TotalStatistics.nRdmPackets + 1;
				sv_isDmxPreviousBreak = false;
				break;
			default:
				sv_DmxReceiveState = IDLE;
				sv_isDmxPreviousBreak = false;
				break;
			}
			break;
		case DMXDATA:
			s_DmxData[sv_nDmxDataBufferIndexHead].Statistics.nSlotToSlot = sv_nFiqMicrosCurrent - sv_nFiqMicrosPrevious;

			if (s_DmxData[sv_nDmxDataBufferIndexHead].Statistics.nSlotToSlot < 44) { // Broadcom BUG ? FIQ is late
				s_DmxData[sv_nDmxDataBufferIndexHead].Statistics.nSlotToSlot = (uint32_t)44;
			}
			s_DmxData[sv_nDmxDataBufferIndexHead].Data[sv_nDmxDataIndex++] = data;
		    BCM2835_ST->C1 = sv_nFiqMicrosCurrent + s_DmxData[0].Statistics.nSlotToSlot + (uint32_t)12;

			if (sv_nDmxDataIndex > dmx::max::CHANNELS) {
				sv_DmxReceiveState = IDLE;
				s_DmxData[sv_nDmxDataBufferIndexHead].Statistics.nSlotsInPacket = dmx::max::CHANNELS;
				sv_nDmxDataBufferIndexHead = (sv_nDmxDataBufferIndexHead + 1) & buffer::INDEX_MASK;
				dmb();
			}
			break;
		case RDMDATA:
			if (sv_nDmxDataIndex > RDM_DATA_BUFFER_SIZE) {
				sv_DmxReceiveState = IDLE;
			} else {
				s_RdmData[sv_nRdmDataBufferIndexHead][sv_nDmxDataIndex++] = data;
				sv_nRdmChecksum += data;

				const auto *p = reinterpret_cast<struct TRdmMessage *>(&s_RdmData[sv_nRdmDataBufferIndexHead][0]);
				if (sv_nDmxDataIndex == p->message_length) {
					sv_DmxReceiveState = CHECKSUMH;
				}
			}
			break;
		case CHECKSUMH:
			s_RdmData[sv_nRdmDataBufferIndexHead][sv_nDmxDataIndex++] =	data;
			sv_nRdmChecksum -= static_cast<uint16_t>(data << 8);
			sv_DmxReceiveState = CHECKSUML;
			break;
		case CHECKSUML: {
			s_RdmData[sv_nRdmDataBufferIndexHead][sv_nDmxDataIndex++] = data;
			sv_nRdmChecksum -= data;
			const auto *p = reinterpret_cast<struct TRdmMessage *>(&s_RdmData[sv_nRdmDataBufferIndexHead][0]);

			if ((sv_nRdmChecksum == 0) && (p->sub_start_code == E120_SC_SUB_MESSAGE)) {
				sv_nRdmDataBufferIndexHead = (sv_nRdmDataBufferIndexHead + 1) & RDM_DATA_BUFFER_INDEX_MASK;
				gsv_RdmDataReceiveEnd = BCM2835_ST->CLO;
				dmb();
			}
			sv_DmxReceiveState = IDLE;
		}
			break;
		case RDMDISCFE:
			s_RdmData[sv_nRdmDataBufferIndexHead][sv_nDmxDataIndex++] = data;

			if ((data == 0xAA) || (sv_nDmxDataIndex == 9 )) {
				sv_DmxReceiveState = RDMDISCEUID;
				sv_RdmDiscIndex = 0;
			}
			break;
		case RDMDISCEUID:
			s_RdmData[sv_nRdmDataBufferIndexHead][sv_nDmxDataIndex++] = data;
			sv_RdmDiscIndex++;

			if (sv_RdmDiscIndex == 2 * RDM_UID_SIZE) {
				sv_DmxReceiveState = RDMDISCECS;
				sv_RdmDiscIndex = 0;
			}
			break;
		case RDMDISCECS:
			s_RdmData[sv_nRdmDataBufferIndexHead][sv_nDmxDataIndex++] = data;
			sv_RdmDiscIndex++;

			if (sv_RdmDiscIndex == 4) {
				sv_nRdmDataBufferIndexHead = (sv_nRdmDataBufferIndexHead + 1) & RDM_DATA_BUFFER_INDEX_MASK;
				sv_DmxReceiveState = IDLE;
				gsv_RdmDataReceiveEnd = BCM2835_ST->CLO;
				dmb();
			}
			break;
		default:
			sv_DmxReceiveState = IDLE;
			sv_isDmxPreviousBreak = false;
			break;
		}
	}

	sv_nFiqMicrosPrevious = sv_nFiqMicrosCurrent;
}

/**
 * PL011 TX interrupt
 */
void fiq_dmx_out_handler(void) {
	if (BCM2835_PL011->MIS == PL011_MIS_TXMIS) {

		for (; !(BCM2835_PL011->FR & PL011_FR_TXFF); sv_DmxTransmitCurrentSlot++) {
			if (sv_DmxTransmitCurrentSlot >= s_nDmxSendDataLength) {
				break;
			}

			BCM2835_PL011->DR = s_DmxData[0].Data[sv_DmxTransmitCurrentSlot];
		}

		if (sv_DmxTransmitCurrentSlot >= s_nDmxSendDataLength) {
			BCM2835_PL011->IMSC &= ~PL011_IMSC_TXIM;
			dmb();
			sv_DmxTransmitState = DMXINTER;
		}

		BCM2835_PL011->ICR = PL011_ICR_TXIC;
	}
}

static void __attribute__((interrupt("FIQ"))) fiq_dmx(void) {
	dmb();

#ifdef LOGIC_ANALYZER
	bcm2835_gpio_set(GPIO_ANALYZER_CH1);
#endif

	if (s_nPortDirection == PortDirection::INP) {
		fiq_dmx_in_handler();
	} else {
		fiq_dmx_out_handler();
	}

#ifdef LOGIC_ANALYZER
	bcm2835_gpio_clr(GPIO_ANALYZER_CH1);
#endif

	dmb();
}

Dmx *Dmx::s_pThis = nullptr;

Dmx::Dmx() {
	assert(s_pThis == nullptr);
	s_pThis = this;

	bcm2835_gpio_fsel(m_nDataDirectionGpio, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_clr(m_nDataDirectionGpio);	// 0 = input, 1 = output

#ifdef LOGIC_ANALYZER
	bcm2835_gpio_fsel(GPIO_ANALYZER_CH1, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_clr(GPIO_ANALYZER_CH1);
	bcm2835_gpio_fsel(GPIO_ANALYZER_CH2, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_clr(GPIO_ANALYZER_CH2);
	bcm2835_gpio_fsel(GPIO_ANALYZER_CH3, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_clr(GPIO_ANALYZER_CH3);
	bcm2835_gpio_fsel(GPIO_ANALYZER_CH4, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_set(GPIO_ANALYZER_CH4);
	bcm2835_gpio_fsel(GPIO_ANALYZER_CH5, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_clr(GPIO_ANALYZER_CH5);
	bcm2835_gpio_fsel(GPIO_ANALYZER_CH6, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_clr(GPIO_ANALYZER_CH6);
	bcm2835_gpio_fsel(GPIO_ANALYZER_CH7, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_clr(GPIO_ANALYZER_CH7);
#endif

	ClearData(0);

	sv_nDmxDataBufferIndexHead = 0;
	sv_nDmxDataBufferIndexTail = 0;

	sv_nRdmDataBufferIndexHead = 0;
	sv_nRdmDataBufferIndexTail = 0;

	sv_DmxReceiveState = IDLE;

	sv_DmxTransmitState = IDLE;
	sv_doDmxTransmitAlways = false;

	irq_handler_init();

	irq_timer_set(IRQ_TIMER_3, irq_timer3_dmx_receive);
	BCM2835_ST->C3 = BCM2835_ST->CLO + (uint32_t) 1000000;
	dmb();

	UartInit();

	__disable_fiq();
	arm_install_handler((unsigned) fiq_dmx, ARM_VECTOR(ARM_VECTOR_FIQ));
}

void Dmx::UartInit() {
	uint32_t ibrd = 12;													// Default UART CLOCK 48Mhz

	// Work around BROADCOM firmware bug
	if (bcm2835_vc_get_clock_rate(BCM2835_VC_CLOCK_ID_UART) != 48000000) {
		(void) bcm2835_vc_set_clock_rate(BCM2835_VC_CLOCK_ID_UART, 4000000);// Set UART clock rate to 4000000 (4MHz)
		ibrd = 1;
	}

	BCM2835_PL011->CR = 0;												// Disable everything

	dmb();

    // Set the GPI0 pins to the Alt 0 function to enable PL011 access on them
    bcm2835_gpio_fsel(RPI_V2_GPIO_P1_08, BCM2835_GPIO_FSEL_ALT0);		// PL011_TXD
    bcm2835_gpio_fsel(RPI_V2_GPIO_P1_10, BCM2835_GPIO_FSEL_ALT0);		// PL011_RXD

    // Disable pull-up/down
    bcm2835_gpio_set_pud(RPI_V2_GPIO_P1_08, BCM2835_GPIO_PUD_OFF);
    bcm2835_gpio_set_pud(RPI_V2_GPIO_P1_10, BCM2835_GPIO_PUD_OFF);

    dmb();

	while ((BCM2835_PL011->FR & PL011_FR_BUSY) != 0)
		;																// Poll the "flags register" to wait for the UART to stop transmitting or receiving

	BCM2835_PL011->LCRH &= ~PL011_LCRH_FEN;								// Flush the transmit FIFO by marking FIFOs as disabled in the "line control register"
	BCM2835_PL011->ICR = 0x7FF;											// Clear all interrupt status
	BCM2835_PL011->IBRD = ibrd;											//
	BCM2835_PL011->FBRD = 0;											//
	BCM2835_PL011->LCRH = PL011_LCRH_WLEN8 | PL011_LCRH_STP2;			// Set 8, N, 2, FIFO disabled
	BCM2835_PL011->CR = PL011_CR_TXE | PL011_CR_RXE | PL011_CR_UARTEN;	// Enable UART

	BCM2835_PL011->IMSC = PL011_IMSC_RXIM;
	BCM2835_IRQ->FIQ_CONTROL = (uint32_t) BCM2835_FIQ_ENABLE | (uint32_t) INTERRUPT_VC_UART;

	isb();
}

void Dmx::UartEnableFifo() {	// DMX Output
	BCM2835_PL011->CR = 0;
	BCM2835_PL011->ICR = 0x7FF;
	BCM2835_PL011->LCRH = PL011_LCRH_WLEN8 | PL011_LCRH_STP2 | PL011_LCRH_FEN;
	BCM2835_PL011->IFLS = PL011_IFLS_TXIFLSEL_1_4;
	BCM2835_PL011->IMSC = 0;
	BCM2835_PL011->CR = PL011_CR_TXE | PL011_CR_RXE | PL011_CR_UARTEN;
	isb();
}
void Dmx::UartDisableFifo() {	// DMX Input
	BCM2835_PL011->CR = 0;
	BCM2835_PL011->LCRH &= ~PL011_LCRH_FEN;
	BCM2835_PL011->ICR = 0x7FF;
	BCM2835_PL011->LCRH = PL011_LCRH_WLEN8 | PL011_LCRH_STP2;
	BCM2835_PL011->IMSC = PL011_IMSC_RXIM;
	BCM2835_PL011->CR = PL011_CR_TXE | PL011_CR_RXE | PL011_CR_UARTEN;
	isb();
}

void Dmx::StartOutput([[maybe_unused]] uint32_t nPortIndex) {

}

void Dmx::SetOutput([[maybe_unused]] const bool doForce) {
	if (sv_DmxTransmitState != IDLE) {
		return;
	}

	UartEnableFifo();
	__enable_fiq();

	irq_timer_set(IRQ_TIMER_1, irq_timer1_dmx_sender);

	const uint32_t clo = BCM2835_ST->CLO;

	if (clo - sv_DmxTransmitBreakMicros > s_nDmxTransmitPeriod) {
		BCM2835_ST->C1 = clo + 4;
	} else {
		BCM2835_ST->C1 = s_nDmxTransmitPeriod + sv_DmxTransmitBreakMicros + 4;
	}

	isb();
}

void Dmx::StartData() {
	switch (s_nPortDirection) {
	case PortDirection::OUTP: {
		sv_doDmxTransmitAlways = true;
		sv_DmxTransmitState = IDLE;

		UartEnableFifo();
		__enable_fiq();

		irq_timer_set(IRQ_TIMER_1, irq_timer1_dmx_sender);

		const uint32_t clo = BCM2835_ST->CLO;

		if (clo - sv_DmxTransmitBreakMicros > s_nDmxTransmitPeriod) {
			BCM2835_ST->C1 = clo + 4;
		} else {
			BCM2835_ST->C1 = s_nDmxTransmitPeriod + sv_DmxTransmitBreakMicros + 4;
		}

		isb();
	}
		break;
	case PortDirection::INP:
		sv_DmxReceiveState = IDLE;

		irq_timer_set(IRQ_TIMER_1, irq_timer1_dmx_receive);

		while ((BCM2835_PL011->FR & PL011_FR_BUSY) != 0) {
			(void) BCM2835_PL011->DR;
		}

		UartDisableFifo();
		__enable_fiq();

		isb();

		break;
	default:
		break;
	}

	s_IsStopped = false;
}

void Dmx::StopData() {
	if (s_IsStopped) {
		return;
	}

	if (sv_doDmxTransmitAlways) {
		do {
			dmb();
			if (sv_DmxTransmitState == DMXINTER) {
				while ((BCM2835_PL011->FR & PL011_FR_BUSY) == PL011_FR_BUSY)
					;
				dmb();
				BCM2835_ST->C1 = BCM2835_ST->CLO - 1;
				BCM2835_ST->CS = BCM2835_ST_CS_M1;

				dmb();
				sv_DmxTransmitState = IDLE;
			}
			dmb();
		} while (sv_DmxTransmitState != IDLE);
	}

	irq_timer_set(IRQ_TIMER_1, nullptr);

	__disable_fiq();
	isb();

	sv_doDmxTransmitAlways = false;
	sv_DmxReceiveState = IDLE;

	for (uint32_t i = 0; i < buffer::INDEX_ENTRIES; i++) {
		s_DmxData[i].Statistics.nSlotsInPacket = 0;
	}

	s_IsStopped = true;
}

void Dmx::SetPortDirection([[maybe_unused]] uint32_t nPortIndex, PortDirection tPortDirection, bool bEnableData) {
	assert(nPort == 0);

	if (tPortDirection != s_nPortDirection) {
		StopData();

		switch (tPortDirection) {
		case PortDirection::OUTP:
			bcm2835_gpio_set(m_nDataDirectionGpio); // 0 = input, 1 = output
			s_nPortDirection = PortDirection::OUTP;
			break;
		case PortDirection::INP:
		default:
			bcm2835_gpio_clr(m_nDataDirectionGpio); // 0 = input, 1 = output
			s_nPortDirection = PortDirection::INP;
			break;
		}
	} else if (!bEnableData) {
		StopData();
	}

	if (bEnableData) {
		StartData();
	}
}

PortDirection Dmx::GetPortDirection([[maybe_unused]] uint32_t nPortIndex) {
	return s_nPortDirection;
}

// DMX

void Dmx::SetDmxBreakTime(uint32_t nBreakTime) {
	s_nDmxTransmitBreakTime = std::max(transmit::BREAK_TIME_MIN, nBreakTime);

	SetDmxPeriodTime(m_nDmxTransmitPeriodRequested);
}

uint32_t Dmx::GetDmxBreakTime() {
	return s_nDmxTransmitBreakTime;
}

void Dmx::SetDmxMabTime(uint32_t nMabTime) {
	s_nDmxTransmitMabTime =std::max(transmit::MAB_TIME_MIN, nMabTime);

	SetDmxPeriodTime(m_nDmxTransmitPeriodRequested);
}

uint32_t Dmx::GetDmxMabTime() {
	return s_nDmxTransmitMabTime;
}

void Dmx::SetDmxPeriodTime(uint32_t nPeriodTime) {
	const auto package_length_us = s_nDmxTransmitBreakTime + s_nDmxTransmitMabTime + (s_nDmxSendDataLength * 44);

	m_nDmxTransmitPeriodRequested = nPeriodTime;

	if (nPeriodTime != 0) {
		if (nPeriodTime < package_length_us) {
			s_nDmxTransmitPeriod = std::max(transmit::BREAK_TO_BREAK_TIME_MIN, package_length_us + 44);
		} else {
			s_nDmxTransmitPeriod = nPeriodTime;
		}
	} else {
		s_nDmxTransmitPeriod = std::max(transmit::BREAK_TO_BREAK_TIME_MIN, package_length_us + 44);
	}
}

uint32_t Dmx::GetDmxPeriodTime() {
	return s_nDmxTransmitPeriod;
}

uint32_t Dmx::GetSendDataLength() {
	return s_nDmxSendDataLength;
}

const volatile struct TotalStatistics *Dmx::GetTotalStatistics() {
	return &sv_TotalStatistics;
}

const uint8_t* Dmx::GetDmxCurrentData([[maybe_unused]]uint32_t nPortIndex) {
	return s_DmxData[sv_nDmxDataBufferIndexTail].Data;
}

const uint8_t* Dmx::GetDmxAvailable([[maybe_unused]]uint32_t nPortIndex) {
	dmb();
	if (sv_nDmxDataBufferIndexHead == sv_nDmxDataBufferIndexTail) {
		return nullptr;
	} else {
		const auto *p = s_DmxData[sv_nDmxDataBufferIndexTail].Data;
		sv_nDmxDataBufferIndexTail = (sv_nDmxDataBufferIndexTail + 1) & buffer::INDEX_MASK;
		return p;
	}
}

const uint8_t* Dmx::GetDmxChanged([[maybe_unused]]uint32_t nPortIndex) {
	const auto *p = GetDmxAvailable(0);
	auto *src = reinterpret_cast<const uint32_t *>(p);

	if (src == nullptr) {
		return nullptr;
	}

	auto *dst = reinterpret_cast<uint32_t *>(s_DmxDataPrevious);
	const auto *dmx_statistics = reinterpret_cast<const struct Data *>(p);

	if (dmx_statistics->Statistics.nSlotsInPacket != sv_DmxSlotsInPacketPrevious) {
		sv_DmxSlotsInPacketPrevious = dmx_statistics->Statistics.nSlotsInPacket;
		for (uint32_t i = 0; i < buffer::SIZE / 4; i++) {
			*dst= *src;
			dst++;
			src++;
		}
		return p;
	}

	auto is_changed = false;

	for (uint32_t i = 0; i < buffer::SIZE / 4; i++) {
		if (*dst != *src) {
			*dst = *src;
			is_changed = true;
		}
		dst++;
		src++;
	}

	return (is_changed ? p : nullptr);
}

void Dmx::SetSendDataLength(uint32_t nLength) {
	s_nDmxSendDataLength = nLength;
	SetDmxPeriodTime(m_nDmxTransmitPeriodRequested);
}

void Dmx::SetDmxSlots(uint16_t nSlots) {
	SetSendDataLength(nSlots + 1U);
}

uint16_t Dmx::GetDmxSlots() {
	return s_nDmxSendDataLength - 1U;
}

void Dmx::SetSendData([[maybe_unused]]uint32_t nPortIndex, const uint8_t *pData, uint32_t nLength) {
	do {
		dmb();
	} while (sv_DmxTransmitState != IDLE && sv_DmxTransmitState != DMXINTER);

	__builtin_prefetch(pData);
	memcpy(s_DmxData[0].Data, pData, nLength);

	SetSendDataLength(nLength);
}

void Dmx::SetSendDataWithoutSC([[maybe_unused]]uint32_t nPortIndex, const uint8_t *pData, uint32_t nLength) {
	do {
		dmb();
	} while (sv_DmxTransmitState != IDLE && sv_DmxTransmitState != DMXINTER);

	s_DmxData[0].Data[0] = START_CODE;

	__builtin_prefetch(pData);
	memcpy(&s_DmxData[0].Data[1], pData, nLength);

	SetSendDataLength(nLength + 1);
}

uint32_t Dmx::GetUpdatesPerSecond([[maybe_unused]]uint32_t nPortIndex) {
	dmb();
	return sv_nDmxUpdatesPerSecond;
}

void Dmx::ClearData([[maybe_unused]]uint32_t nPortIndex) {
	for (uint32_t i = 0; i < buffer::INDEX_ENTRIES; i++) {
		memset(s_DmxData[i].Data, 0, dmx::buffer::SIZE);
		memset(&s_DmxData[i].Statistics, 0, sizeof(struct Statistics));
	}
}

// RDM

const uint8_t *Dmx::RdmReceive([[maybe_unused]] uint32_t nPortIndex) {
	assert(nPort == 0);

	dmb();
	if (sv_nRdmDataBufferIndexHead == sv_nRdmDataBufferIndexTail) {
		return nullptr;
	} else {
		const auto *p = &s_RdmData[sv_nRdmDataBufferIndexTail][0];
		sv_nRdmDataBufferIndexTail = (sv_nRdmDataBufferIndexTail + 1) & RDM_DATA_BUFFER_INDEX_MASK;
		return p;
	}
}

const uint8_t* Dmx::RdmReceiveTimeOut(uint32_t nPortIndex, uint32_t nTimeOut) {
	assert(nPort == 0);

	uint8_t *p = nullptr;
	const auto nMicros = BCM2835_ST->CLO;

	do {
		if ((p = const_cast<uint8_t*>(RdmReceive(nPortIndex))) != nullptr) {
			return reinterpret_cast<const uint8_t*>(p);
		}
	} while ((BCM2835_ST->CLO - nMicros) < nTimeOut);

	return p;
}

void Dmx::RdmSendRaw([[maybe_unused]] uint32_t nPortIndex, const uint8_t *pRdmData, uint32_t nLength) {
	assert(nPort == 0);

	BCM2835_PL011->LCRH &= ~PL011_LCRH_FEN;
	BCM2835_PL011->LCRH = PL011_LCRH_WLEN8 | PL011_LCRH_STP2 | PL011_LCRH_BRK;
	udelay(RDM_TRANSMIT_BREAK_TIME);

	BCM2835_PL011->LCRH = PL011_LCRH_WLEN8 | PL011_LCRH_STP2;
	udelay(RDM_TRANSMIT_MAB_TIME);

	for (uint32_t i = 0; i < nLength; i++) {
		while ((BCM2835_PL011->FR & PL011_FR_TXFF) == PL011_FR_TXFF)
			;
		BCM2835_PL011->DR = pRdmData[i];
	}

	while ((BCM2835_PL011->FR & PL011_FR_BUSY) != 0)
		;
}
