/**
 * @file dmx.cpp
 *
 */
/* Copyright (C) 2018-2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include "rdm.h"
#include "rdm_e120.h"

#include "arm/arm.h"
#include "arm/synchronize.h"
#include "arm/gic.h"
#include "irq_timer.h"

#include "h3_gpio.h"
#include "h3_uart.h"
#include "h3_ccu.h"
#include "h3_timer.h"
#include "h3_hs_timer.h"
#include "h3_board.h"

#include "debug.h"

#define GPIO_ANALYZER_CH1 GPIO_EXT_26
#define GPIO_ANALYZER_CH2 GPIO_EXT_24
#define GPIO_ANALYZER_CH3 GPIO_EXT_22
#define GPIO_ANALYZER_CH4 GPIO_EXT_18
#define GPIO_ANALYZER_CH5 GPIO_EXT_16

void console_error(const char*);

#if (EXT_UART_NUMBER == 1)
# define UART_IRQN 		H3_UART1_IRQn
#elif (EXT_UART_NUMBER == 3)
# define UART_IRQN 		H3_UART3_IRQn
#else
# error Unsupported UART device configured
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
	RDMDISC,
	DMXINTER
} _dmx_state;

namespace dmx {
enum class PortState {
	IDLE, TX, RX
};
}  // namespace dmx

using namespace dmx;

static PortDirection s_nPortDirection = dmx::PortDirection::INP;
static volatile PortState sv_PortState;
static OutputStyle s_OutputStyle;

static volatile dmx::TotalStatistics sv_TotalStatistics[dmx::config::max::PORTS] ALIGNED;

// DMX

static volatile uint32_t sv_nDmxDataBufferIndexHead;
static volatile uint32_t sv_nDmxDataBufferIndexTail;
static struct Data s_DmxData[buffer::INDEX_ENTRIES] ALIGNED;
static uint8_t s_DmxDataPrevious[buffer::SIZE] ALIGNED;
static volatile _dmx_state sv_DmxReceiveState = IDLE;
static volatile uint32_t sv_nDmxDataIndex;

static uint32_t s_DmxTransmitBreakTimeIntv;
static uint32_t s_DmxTransmitMabTimeIntv;
static uint32_t s_DmxTransmitPeriodIntv;

static uint32_t s_nDmxSendDataLength = (dmx::max::CHANNELS + 1);		///< SC + UNIVERSE SIZE
static volatile uint32_t sv_nFiqMicrosCurrent;
static volatile uint32_t sv_nFiqMicrosPrevious;
static volatile bool sv_isDmxPreviousBreak = false;
static volatile uint32_t sv_DmxBreakToBreakLatest;
static volatile uint32_t sv_DmxBreakToBreakPrevious;
static volatile uint32_t sv_DmxSlotsInPacketPrevious;
static volatile _dmx_state sv_DmxTransmitState = IDLE;
static volatile uint32_t sv_DmxTransmitCurrentSlot;

static volatile uint32_t sv_nDmxUpdatesPerSecond;
static volatile uint32_t sv_nDmxPacketsPrevious;

// RDM

static volatile uint32_t sv_nRdmDataBufferIndexHead;
static volatile uint32_t sv_nRdmDataBufferIndexTail;
static uint8_t s_RdmData[RDM_DATA_BUFFER_INDEX_ENTRIES][RDM_DATA_BUFFER_SIZE] ALIGNED;
static volatile uint32_t sv_nRdmDiscSlotToSlot[RDM_DATA_BUFFER_INDEX_ENTRIES];
static volatile uint16_t sv_nRdmChecksum;	///< This must be uint16_t
volatile uint32_t gsv_RdmDataReceiveEnd;

/**
 * Timer 0 interrupt DMX Receiver
 * Slot time-out
 */
static void irq_timer0_dmx_receive(uint32_t clo) {
	dmb();
	if (sv_DmxReceiveState == DMXDATA) {
		if (clo - sv_nFiqMicrosCurrent > s_DmxData[0].Statistics.nSlotToSlot) {
			dmb();
			sv_DmxReceiveState = IDLE;
			s_DmxData[sv_nDmxDataBufferIndexHead].Statistics.nSlotsInPacket = sv_nDmxDataIndex - 1;
			sv_nDmxDataBufferIndexHead = (sv_nDmxDataBufferIndexHead + 1) & buffer::INDEX_MASK;
		} else {
			H3_TIMER->TMR0_INTV = s_DmxData[sv_nDmxDataBufferIndexHead].Statistics.nSlotToSlot * 12;
			H3_TIMER->TMR0_CTRL |= (TIMER_CTRL_EN_START | TIMER_CTRL_RELOAD); // 0x3;
		}
	} else if (sv_DmxReceiveState == RDMDISC) {
		if (clo - sv_nFiqMicrosCurrent > sv_nRdmDiscSlotToSlot[sv_nRdmDataBufferIndexHead]) {
			dmb();
			sv_nRdmDataBufferIndexHead = (sv_nRdmDataBufferIndexHead + 1) & RDM_DATA_BUFFER_INDEX_MASK;
			sv_DmxReceiveState = IDLE;
			gsv_RdmDataReceiveEnd = H3_HS_TIMER->CURNT_LO;
			h3_gpio_clr(GPIO_ANALYZER_CH3);
		} else {
			H3_TIMER->TMR0_INTV = sv_nRdmDiscSlotToSlot[sv_nRdmDataBufferIndexHead] * 12;
			H3_TIMER->TMR0_CTRL |= (TIMER_CTRL_EN_START | TIMER_CTRL_RELOAD); // 0x3;
		}
	}
}

/**
 * Timer 0 interrupt DMX Sender
 */
static void irq_timer0_dmx_sender([[maybe_unused]] uint32_t clo) {
	h3_gpio_set(GPIO_ANALYZER_CH1);

	switch (sv_DmxTransmitState) {
	case DMXINTER:
		h3_gpio_set(GPIO_ANALYZER_CH2);

		H3_TIMER->TMR0_INTV = s_DmxTransmitBreakTimeIntv;
		H3_TIMER->TMR0_CTRL |= (TIMER_CTRL_EN_START | TIMER_CTRL_RELOAD);

		EXT_UART->LCR = UART_LCR_8_N_2 | UART_LCR_BC;
		dmb();
		sv_DmxTransmitState = BREAK;

		h3_gpio_clr(GPIO_ANALYZER_CH2);
		break;
	case BREAK:
		h3_gpio_set(GPIO_ANALYZER_CH3);

		H3_TIMER->TMR0_INTV = s_DmxTransmitMabTimeIntv;
		H3_TIMER->TMR0_CTRL |= (TIMER_CTRL_EN_START | TIMER_CTRL_RELOAD);
		EXT_UART->LCR = UART_LCR_8_N_2;
		dmb();
		sv_DmxTransmitState = MAB;

		h3_gpio_clr(GPIO_ANALYZER_CH3);
		break;
	case MAB: {
		h3_gpio_set(GPIO_ANALYZER_CH4);

		uint32_t fifo_cnt = 16;

		for (sv_DmxTransmitCurrentSlot = 0; fifo_cnt-- > 0; sv_DmxTransmitCurrentSlot++) {
			if (sv_DmxTransmitCurrentSlot >= s_nDmxSendDataLength) {
				break;
			}

			EXT_UART->O00.THR = s_DmxData[0].Data[sv_DmxTransmitCurrentSlot];
		}

		if (sv_DmxTransmitCurrentSlot < s_nDmxSendDataLength) {
			dmb();
			sv_DmxTransmitState = DMXDATA;
			EXT_UART->O04.IER = UART_IER_ETBEI;
		} else {
			if (s_OutputStyle == dmx::OutputStyle::DELTA) {
				EXT_UART->O04.IER &= ~UART_IER_ETBEI;
				dmb();
				sv_DmxTransmitState = IDLE;
			} else {
				dmb();
				sv_DmxTransmitState = DMXINTER;
			}
		}

		h3_gpio_clr(GPIO_ANALYZER_CH4);
	}
	break;
	case DMXDATA:
		printf("Output period too short (%d, slot %d)\n", s_nDmxSendDataLength, sv_DmxTransmitCurrentSlot);
//		assert(0);
		break;
	default:
		break;
	}

	h3_gpio_clr(GPIO_ANALYZER_CH1);
}

/**
 * Timer 1 interrupt DMX Receiver
 * Statistics
 */
static void irq_timer1_dmx_receive([[maybe_unused]] uint32_t clo) {
	dmb();
	sv_nDmxUpdatesPerSecond = sv_TotalStatistics[0].Dmx.Received - sv_nDmxPacketsPrevious;
	sv_nDmxPacketsPrevious = sv_TotalStatistics[0].Dmx.Received;
}

/**
 * Interrupt handler for continues receiving DMX512 data.
 */
static void fiq_dmx_in_handler() {
	sv_nFiqMicrosCurrent = H3_HS_TIMER->CURNT_LO;

	if (EXT_UART->LSR & UART_LSR_BI) {
		sv_DmxReceiveState = PRE_BREAK;
		sv_DmxBreakToBreakLatest = sv_nFiqMicrosCurrent;
	} else if (EXT_UART->O08.IIR & UART_IIR_IID_RD) {
		const auto data = static_cast<uint8_t>(EXT_UART->O00.RBR);

		switch (sv_DmxReceiveState) {
		case IDLE:
			sv_DmxReceiveState = RDMDISC;
			s_RdmData[sv_nRdmDataBufferIndexHead][0] = data;
			sv_nDmxDataIndex = 1;
			h3_gpio_set(GPIO_ANALYZER_CH3);
			break;
		case PRE_BREAK:
			sv_DmxReceiveState = BREAK;
			break;
		case BREAK:
			switch (data) {
			case START_CODE:
				sv_DmxReceiveState = DMXDATA;
				s_DmxData[sv_nDmxDataBufferIndexHead].Data[0] = START_CODE;
				sv_nDmxDataIndex = 1;
				sv_TotalStatistics[0].Dmx.Received = sv_TotalStatistics[0].Dmx.Received + 1;

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
			s_DmxData[sv_nDmxDataBufferIndexHead].Data[sv_nDmxDataIndex++] = data;

			H3_TIMER->TMR0_INTV = (s_DmxData[0].Statistics.nSlotToSlot + 12) * 12;
			H3_TIMER->TMR0_CTRL |= (TIMER_CTRL_EN_START | TIMER_CTRL_RELOAD); // 0x3;

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
				sv_nRdmChecksum = static_cast<uint16_t>(sv_nRdmChecksum + data);

				const auto *p = reinterpret_cast<struct TRdmMessage *>(&s_RdmData[sv_nRdmDataBufferIndexHead][0]);
				if (sv_nDmxDataIndex == p->message_length) {
					sv_DmxReceiveState = CHECKSUMH;
				}
			}
			break;
		case CHECKSUMH:
			s_RdmData[sv_nRdmDataBufferIndexHead][sv_nDmxDataIndex++] =	data;
			sv_nRdmChecksum = static_cast<uint16_t>(sv_nRdmChecksum - static_cast<uint16_t>(data << 8));
			sv_DmxReceiveState = CHECKSUML;
			break;
		case CHECKSUML: {
			s_RdmData[sv_nRdmDataBufferIndexHead][sv_nDmxDataIndex++] = data;
			sv_nRdmChecksum = static_cast<uint16_t>(sv_nRdmChecksum - data);
			const auto *p = reinterpret_cast<struct TRdmMessage *>(&s_RdmData[sv_nRdmDataBufferIndexHead][0]);

			if ((sv_nRdmChecksum == 0) && (p->sub_start_code == E120_SC_SUB_MESSAGE)) {
				sv_nRdmDataBufferIndexHead = (sv_nRdmDataBufferIndexHead + 1) & RDM_DATA_BUFFER_INDEX_MASK;
				gsv_RdmDataReceiveEnd = H3_HS_TIMER->CURNT_LO;
				sv_TotalStatistics[0].Rdm.Received.Good = sv_TotalStatistics[0].Rdm.Received.Good + 1;
				dmb();
			} else {
				sv_TotalStatistics[0].Rdm.Received.Bad = sv_TotalStatistics[0].Rdm.Received.Bad + 1;
			}

			sv_DmxReceiveState = IDLE;
		}
			break;
		case RDMDISC:
			sv_nRdmDiscSlotToSlot[sv_nRdmDataBufferIndexHead] = sv_nFiqMicrosCurrent - sv_nFiqMicrosPrevious;
			s_RdmData[sv_nRdmDataBufferIndexHead][sv_nDmxDataIndex++] = data;

			H3_TIMER->TMR0_INTV = (sv_nRdmDiscSlotToSlot[sv_nRdmDataBufferIndexHead] + 12) * 12;
			H3_TIMER->TMR0_CTRL |= (TIMER_CTRL_EN_START | TIMER_CTRL_RELOAD); // 0x3

			if (sv_nDmxDataIndex == 24) {
				sv_nRdmDataBufferIndexHead = (sv_nRdmDataBufferIndexHead + 1) & RDM_DATA_BUFFER_INDEX_MASK;
				sv_DmxReceiveState = IDLE;
				gsv_RdmDataReceiveEnd = H3_HS_TIMER->CURNT_LO;
				dmb();
				h3_gpio_clr(GPIO_ANALYZER_CH3);
			}
			break;
		default:
			sv_DmxReceiveState = IDLE;
			sv_isDmxPreviousBreak = false;
			break;
		}
	} else {
	}

	sv_nFiqMicrosPrevious = sv_nFiqMicrosCurrent;
}

/**
 * EXT_UART TX interrupt
 */
static void fiq_dmx_out_handler() {
	h3_gpio_set(GPIO_ANALYZER_CH5);

	uint32_t fifo_cnt = 16;

	for (; fifo_cnt-- > 0; sv_DmxTransmitCurrentSlot++) {
		if (sv_DmxTransmitCurrentSlot >= s_nDmxSendDataLength) {
			break;
		}

		EXT_UART->O00.THR = s_DmxData[0].Data[sv_DmxTransmitCurrentSlot];
	}

	if (sv_DmxTransmitCurrentSlot >= s_nDmxSendDataLength) {
		EXT_UART->O04.IER &= ~UART_IER_ETBEI;
		dmb();
		if (s_OutputStyle == dmx::OutputStyle::DELTA) {
			sv_DmxTransmitState = IDLE;
		} else {
			H3_TIMER->TMR0_INTV = s_DmxTransmitPeriodIntv;
			H3_TIMER->TMR0_CTRL |= (TIMER_CTRL_EN_START | TIMER_CTRL_RELOAD); // 0x3;
			sv_DmxTransmitState = DMXINTER;
		}
	}

	h3_gpio_clr(GPIO_ANALYZER_CH5);
}

static void __attribute__((interrupt("FIQ"))) fiq_dmx() {
	dmb();

	if (gic_get_active_fiq() == UART_IRQN) {
		const uint32_t iir = EXT_UART->O08.IIR ;

		if (s_nPortDirection == PortDirection::INP) {
			fiq_dmx_in_handler();
		} else {
			if ((iir & 0xF) == UART_IIR_IID_THRE) {
				fiq_dmx_out_handler();
			} else {
				(void) EXT_UART->USR;
			}
		}

		H3_GIC_CPUIF->EOI = UART_IRQN;
		gic_unpend<UART_IRQN>();
	} else {
		console_error("spurious interrupt\n");
	}

	dmb();
}

static void uart_dmx_config() {
#if (EXT_UART_NUMBER == 1)
	uint32_t value = H3_PIO_PORTG->CFG0;
	// PG6, TX
	value &= static_cast<uint32_t> (~(GPIO_SELECT_MASK << PG6_SELECT_CFG0_SHIFT));
	value |= H3_PG6_SELECT_UART1_TX << PG6_SELECT_CFG0_SHIFT;
	// PG7, RX
	value &= static_cast<uint32_t> (~(GPIO_SELECT_MASK << PG7_SELECT_CFG0_SHIFT));
	value |= H3_PG7_SELECT_UART1_RX << PG7_SELECT_CFG0_SHIFT;
	H3_PIO_PORTG->CFG0 = value;

	H3_CCU->BUS_CLK_GATING3 |= CCU_BUS_CLK_GATING3_UART1;
	H3_CCU->BUS_SOFT_RESET4 |= CCU_BUS_SOFT_RESET4_UART1;
#elif (EXT_UART_NUMBER == 3)
	uint32_t value = H3_PIO_PORTA->CFG1;
	// PA13, TX
	value &= static_cast<uint32_t>(~(GPIO_SELECT_MASK << PA13_SELECT_CFG1_SHIFT));
	value |= H3_PA13_SELECT_UART3_TX << PA13_SELECT_CFG1_SHIFT;
	// PA14, RX
	value &= static_cast<uint32_t> (~(GPIO_SELECT_MASK << PA14_SELECT_CFG1_SHIFT));
	value |= H3_PA14_SELECT_UART3_RX << PA14_SELECT_CFG1_SHIFT;
	H3_PIO_PORTA->CFG1 = value;

	H3_CCU->BUS_CLK_GATING3 |= CCU_BUS_CLK_GATING3_UART3;
	H3_CCU->BUS_SOFT_RESET4 |= CCU_BUS_SOFT_RESET4_UART3;
#else
# error Unsupported UART device configured
#endif

	EXT_UART->O08.FCR = 0;

	EXT_UART->LCR = UART_LCR_DLAB;
	EXT_UART->O00.DLL = BAUD_250000_L;
	EXT_UART->O04.DLH = BAUD_250000_H;
	EXT_UART->LCR = UART_LCR_8_N_2;

	isb();
}

static void uart_enable_fifo() {	// DMX Output
	EXT_UART->O08.FCR = UART_FCR_EFIFO | UART_FCR_TRESET;
	EXT_UART->O04.IER = 0;
	isb();
}

static void uart_disable_fifo() {	// DMX Input
	EXT_UART->O08.FCR = 0;
	EXT_UART->O04.IER = UART_IER_ERBFI;
	isb();
}

Dmx *Dmx::s_pThis = nullptr;

Dmx::Dmx() {
	assert(s_pThis == nullptr);
	s_pThis = this;

	s_DmxTransmitBreakTimeIntv = m_nDmxTransmitBreakTime * 12;
	s_DmxTransmitMabTimeIntv = m_nDmxTransmitMabTime * 12;
	s_DmxTransmitPeriodIntv = (transmit::PERIOD_DEFAULT * 12) - s_DmxTransmitBreakTimeIntv - s_DmxTransmitMabTimeIntv;

	h3_gpio_fsel(GPIO_EXT_12, GPIO_FSEL_OUTPUT);
	h3_gpio_clr(GPIO_EXT_12);	// 0 = input, 1 = output


#define LOGIC_ANALYZER
#ifdef LOGIC_ANALYZER
# if defined GPIO_ANALYZER_CH1
	h3_gpio_fsel(GPIO_ANALYZER_CH1, GPIO_FSEL_OUTPUT);
	h3_gpio_clr(GPIO_ANALYZER_CH1);
# endif
# if defined GPIO_ANALYZER_CH2
	h3_gpio_fsel(GPIO_ANALYZER_CH2, GPIO_FSEL_OUTPUT);
	h3_gpio_clr(GPIO_ANALYZER_CH2);
# endif
# if defined GPIO_ANALYZER_CH3
	h3_gpio_fsel(GPIO_ANALYZER_CH3, GPIO_FSEL_OUTPUT);
	h3_gpio_clr(GPIO_ANALYZER_CH3);
# endif
# if defined GPIO_ANALYZER_CH4
	h3_gpio_fsel(GPIO_ANALYZER_CH4, GPIO_FSEL_OUTPUT);
	h3_gpio_clr(GPIO_ANALYZER_CH4);
# endif
# if defined GPIO_ANALYZER_CH5
	h3_gpio_fsel(GPIO_ANALYZER_CH5, GPIO_FSEL_OUTPUT);
	h3_gpio_clr(GPIO_ANALYZER_CH5);
# endif
# if defined GPIO_ANALYZER_CH6
	h3_gpio_fsel(GPIO_ANALYZER_CH6, GPIO_FSEL_OUTPUT);
	h3_gpio_clr(GPIO_ANALYZER_CH6);
# endif
# if defined GPIO_ANALYZER_CH7
	h3_gpio_fsel(GPIO_ANALYZER_CH7, GPIO_FSEL_OUTPUT);
	h3_gpio_clr(GPIO_ANALYZER_CH7);
# endif
#endif

	ClearData(0);

	sv_nDmxDataBufferIndexHead = 0;
	sv_nDmxDataBufferIndexTail = 0;

	sv_nRdmDataBufferIndexHead = 0;
	sv_nRdmDataBufferIndexTail = 0;

	sv_DmxReceiveState = IDLE;

	sv_DmxTransmitState = IDLE;

	irq_handler_init();

	irq_timer_set(IRQ_TIMER_1, irq_timer1_dmx_receive);
	H3_TIMER->TMR1_INTV = 0xB71B00; // 1 second
	H3_TIMER->TMR1_CTRL &= ~(TIMER_CTRL_SINGLE_MODE);
	H3_TIMER->TMR1_CTRL |= (TIMER_CTRL_EN_START | TIMER_CTRL_RELOAD); // 0x3;

	H3_TIMER->TMR0_CTRL |= (TIMER_CTRL_SINGLE_MODE);

	gic_fiq_config(UART_IRQN, GIC_CORE0);

	uart_dmx_config();

	__disable_fiq();
	arm_install_handler(reinterpret_cast<unsigned>(fiq_dmx), ARM_VECTOR(ARM_VECTOR_FIQ));
}


void Dmx::StartOutput(const uint32_t nPortIndex) {
	if ((sv_PortState == dmx::PortState::TX)
			&& (s_OutputStyle == dmx::OutputStyle::DELTA)
			&& (sv_DmxTransmitState == IDLE)) {

		StartDmxOutput(nPortIndex);
	}
}

void Dmx::Sync() {
	if (sv_PortState == dmx::PortState::TX) {
		if ((s_OutputStyle == dmx::OutputStyle::DELTA) && (sv_DmxTransmitState == IDLE)) {
			StartDmxOutput(0);
		}
	}
}

void Dmx::StartData([[maybe_unused]] const uint32_t nPortIndex) {
	assert(sv_PortState == PortState::IDLE);

	if (s_nPortDirection == PortDirection::OUTP) {
		sv_PortState = PortState::TX;
		return;
	}

	if (s_nPortDirection == PortDirection::INP) {
		sv_DmxReceiveState = IDLE;

		irq_timer_set(IRQ_TIMER_0, irq_timer0_dmx_receive);
		H3_TIMER->TMR0_CTRL |= TIMER_CTRL_SINGLE_MODE;

		while ((EXT_UART->USR & UART_USR_BUSY) == UART_USR_BUSY) {
			(void) EXT_UART->O00.RBR;
		}

		uart_disable_fifo();
		__enable_fiq();

		isb();
		return;
	}

	assert(0);
	__builtin_unreachable();
}

void Dmx::StopData([[maybe_unused]]  const uint32_t nPortIndex) {
	DEBUG_PRINTF("nPortIndex=%u, sv_PortState=%u", nPortIndex, sv_PortState);

	if (sv_PortState == PortState::IDLE) {
		return;
	}

	if (s_nPortDirection == PortDirection::OUTP) {
		do {
			dmb();
			if (sv_DmxTransmitState == DMXINTER) {
				while (!(EXT_UART->USR & UART_USR_TFE))
					;
				sv_DmxTransmitState = IDLE;
			}
			dmb();
		} while (sv_DmxTransmitState != IDLE);
	} else if (s_nPortDirection == PortDirection::INP) {
		sv_DmxReceiveState = IDLE;
	} else {
		assert(0);
		__builtin_unreachable();
	}

	irq_timer_set(IRQ_TIMER_0, nullptr);

	__disable_fiq();
	isb();

	for (uint32_t i = 0; i < buffer::INDEX_ENTRIES; i++) {
		s_DmxData[i].Statistics.nSlotsInPacket = 0;
	}

	sv_PortState = PortState::IDLE;
}

void Dmx::SetPortDirection(const uint32_t nPortIndex, const PortDirection portDirection, const bool bEnableData) {
	DEBUG_PRINTF("nPortIndex=%u %s %c", nPortIndex, portDirection == PortDirection::INP ? "Input" : "Output", bEnableData ? 'Y' : 'N');
	assert(nPortIndex == 0);

	if (s_nPortDirection != portDirection) {
		 s_nPortDirection = portDirection;

		StopData(nPortIndex);

		switch (portDirection) {
		case PortDirection::OUTP:
			h3_gpio_set(GPIO_EXT_12); // 0 = input, 1 = output
			break;
		case PortDirection::INP:
		default:
			h3_gpio_clr(GPIO_EXT_12); // 0 = input, 1 = output
			break;
		}
	} else if (!bEnableData) {
		StopData(nPortIndex);
	}

	if (bEnableData) {
		StartData(nPortIndex);
	}
}

PortDirection Dmx::GetPortDirection([[maybe_unused]] uint32_t nPortIndex) {
	assert(nPortIndex == 0);

	return s_nPortDirection;
}

// DMX

void Dmx::SetDmxBreakTime(uint32_t nBreakTime) {
	m_nDmxTransmitBreakTime = std::max(transmit::BREAK_TIME_MIN, nBreakTime);
	s_DmxTransmitBreakTimeIntv = m_nDmxTransmitBreakTime * 12;

	SetDmxPeriodTime(m_nDmxTransmitPeriodRequested);
}

void Dmx::SetDmxMabTime(uint32_t nMabTime) {
	m_nDmxTransmitMabTime = std::max(transmit::MAB_TIME_MIN, nMabTime);
	s_DmxTransmitMabTimeIntv = m_nDmxTransmitMabTime * 12;

	SetDmxPeriodTime(m_nDmxTransmitPeriodRequested);
}

void Dmx::SetDmxPeriodTime(uint32_t nPeriodTime) {
	const auto package_length_us = m_nDmxTransmitBreakTime + m_nDmxTransmitMabTime + (s_nDmxSendDataLength * 44);

	m_nDmxTransmitPeriodRequested = nPeriodTime;

	if (nPeriodTime != 0) {
		if (nPeriodTime < package_length_us) {
			m_nDmxTransmitPeriod =  std::max(transmit::BREAK_TO_BREAK_TIME_MIN, package_length_us + 44);
		} else {
			m_nDmxTransmitPeriod = nPeriodTime;
		}
	} else {
		m_nDmxTransmitPeriod = std::max(transmit::BREAK_TO_BREAK_TIME_MIN, package_length_us + 44);
	}

	s_DmxTransmitPeriodIntv = (m_nDmxTransmitPeriod * 12) - s_DmxTransmitBreakTimeIntv - s_DmxTransmitMabTimeIntv;
}

void Dmx::SetDmxSlots(uint16_t nSlots) {
	if ((nSlots >= 2) && (nSlots <= dmx::max::CHANNELS)) {
		m_nDmxTransmitSlots = nSlots;
		s_nDmxSendDataLength = 1U + m_nDmxTransmitSlots;
		SetDmxPeriodTime(m_nDmxTransmitPeriodRequested);
	}
}

const uint8_t* Dmx::GetDmxCurrentData([[maybe_unused]] uint32_t nPortIndex) {
	return s_DmxData[sv_nDmxDataBufferIndexTail].Data;
}

const uint8_t* Dmx::GetDmxAvailable([[maybe_unused]] uint32_t nPortIndex) {
	dmb();
	if (sv_nDmxDataBufferIndexHead == sv_nDmxDataBufferIndexTail) {
		return nullptr;
	} else {
		const auto *p = s_DmxData[sv_nDmxDataBufferIndexTail].Data;
		sv_nDmxDataBufferIndexTail = (sv_nDmxDataBufferIndexTail + 1) & buffer::INDEX_MASK;
		return p;
	}
}

const uint8_t* Dmx::GetDmxChanged([[maybe_unused]] uint32_t nPortIndex) {
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

void Dmx::SetOutputStyle(const uint32_t nPortIndex, const dmx::OutputStyle outputStyle) {
	if (s_OutputStyle == outputStyle) {
		return;
	}

	s_OutputStyle = outputStyle;

	if ((s_OutputStyle == dmx::OutputStyle::CONTINOUS)
			&& (s_nPortDirection == dmx::PortDirection::OUTP)) {
		StopData(nPortIndex);
		StartDmxOutput(nPortIndex);
	}
}

dmx::OutputStyle Dmx::GetOutputStyle([[maybe_unused]] const uint32_t nPortIndex) const {
	return s_OutputStyle;
}

void Dmx::SetSendData([[maybe_unused]] uint32_t nPortIndex, const uint8_t *pData, uint32_t nLength) {
	assert(nPortIndex == 0);

	do {
		dmb();
	} while (sv_DmxTransmitState != IDLE && sv_DmxTransmitState != DMXINTER);

	__builtin_prefetch(pData);
	memcpy(reinterpret_cast<void *>(s_DmxData[0].Data), pData, nLength);

	if (nLength != s_nDmxSendDataLength) {
		s_nDmxSendDataLength = nLength;
		SetDmxPeriodTime(m_nDmxTransmitPeriodRequested);
	}
}

void Dmx::SetSendDataWithoutSC([[maybe_unused]] uint32_t nPortIndex, const uint8_t *pData, uint32_t nLength) {
	do {
		dmb();
	} while (sv_DmxTransmitState != IDLE && sv_DmxTransmitState != DMXINTER);

	nLength = std::min(nLength, static_cast<uint32_t>(m_nDmxTransmitSlots));

	s_DmxData[0].Data[0] = START_CODE;

	__builtin_prefetch(pData);
	memcpy(&s_DmxData[0].Data[1], pData, nLength);

	if (nLength != s_nDmxSendDataLength) {
		s_nDmxSendDataLength = 1U + nLength;
		SetDmxPeriodTime(m_nDmxTransmitPeriodRequested);
	}
}

void Dmx::Blackout() {
	DEBUG_ENTRY

	do {
		dmb();
	} while (sv_DmxTransmitState != IDLE && sv_DmxTransmitState != DMXINTER);

	auto *p = reinterpret_cast<uint32_t *>(s_DmxData[0].Data);

	for (uint32_t i = 0; i < buffer::SIZE / 4; i++) {
		*p++ = 0;
	}

	s_DmxData[0].Data[0] = START_CODE;

	DEBUG_EXIT
}

void Dmx::FullOn() {
	DEBUG_ENTRY

	do {
		dmb();
	} while (sv_DmxTransmitState != IDLE && sv_DmxTransmitState != DMXINTER);

	auto *p = reinterpret_cast<uint32_t *>(s_DmxData[0].Data);

	for (uint32_t i = 0; i < buffer::SIZE / 4; i++) {
		*p++ = static_cast<uint32_t>(~0);
	}

	s_DmxData[0].Data[0] = START_CODE;

	DEBUG_EXIT
}

uint32_t Dmx::GetDmxUpdatesPerSecond([[maybe_unused]] uint32_t nPortIndex) {
	dmb();
	return sv_nDmxUpdatesPerSecond;
}

void Dmx::ClearData([[maybe_unused]] uint32_t nPortIndex) {
	assert(nPortIndex == 0);

	for (uint32_t j = 0; j < buffer::INDEX_ENTRIES; j++) {
		auto *p = reinterpret_cast<uint32_t *>(s_DmxData[j].Data);

		for (uint32_t i = 0; i < buffer::SIZE / 4; i++) {
			*p++ = 0;
		}
	}
}

volatile dmx::TotalStatistics& Dmx::GetTotalStatistics([[maybe_unused]] const uint32_t nPortIndex) {
	return sv_TotalStatistics[0];
}

void Dmx::StartDmxOutput([[maybe_unused]] const uint32_t nPortIndex) {
	assert(nPortIndex == 0);

	uart_enable_fifo();
	__enable_fiq();

	irq_timer_set(IRQ_TIMER_0, irq_timer0_dmx_sender);

	H3_TIMER->TMR0_INTV = s_DmxTransmitBreakTimeIntv;
	H3_TIMER->TMR0_CTRL |= (TIMER_CTRL_EN_START | TIMER_CTRL_RELOAD | TIMER_CTRL_SINGLE_MODE);

	EXT_UART->LCR = UART_LCR_8_N_2 | UART_LCR_BC;

	isb();

	sv_DmxTransmitState = BREAK;
}

const uint8_t *Dmx::RdmReceive([[maybe_unused]] uint32_t nPortIndex) {
	assert(nPortIndex == 0);

	dmb();
	if (sv_nRdmDataBufferIndexHead == sv_nRdmDataBufferIndexTail) {
		return nullptr;
	} else {
		const auto *p = &s_RdmData[sv_nRdmDataBufferIndexTail][0];
		sv_nRdmDataBufferIndexTail = (sv_nRdmDataBufferIndexTail + 1) & RDM_DATA_BUFFER_INDEX_MASK;
		return p;
	}
}

const uint8_t* Dmx::RdmReceiveTimeOut(const uint32_t nPortIndex, uint16_t nTimeOut) {
	assert(nPortIndex == 0);

	uint8_t *p = nullptr;
	const auto nMicros = H3_TIMER->AVS_CNT1;

	do {
		if ((p = const_cast<uint8_t*>(RdmReceive(nPortIndex))) != nullptr) {
			return reinterpret_cast<const uint8_t*>(p);
		}
	} while ((H3_TIMER->AVS_CNT1 - nMicros) < nTimeOut);

	return p;
}

void Dmx::RdmSendRaw([[maybe_unused]] uint32_t nPortIndex, const uint8_t *pRdmData, uint32_t nLength) {
	assert(nPortIndex == 0);

	while (!(EXT_UART->LSR & UART_LSR_TEMT))
		;

	EXT_UART->LCR = UART_LCR_8_N_2 | UART_LCR_BC;
	udelay(RDM_TRANSMIT_BREAK_TIME);

	EXT_UART->LCR = UART_LCR_8_N_2;
	udelay(RDM_TRANSMIT_MAB_TIME);

	for (uint32_t i = 0; i < nLength; i++) {
		while (!(EXT_UART->LSR & UART_LSR_THRE))
			;
		EXT_UART->O00.THR = pRdmData[i];
	}

	while ((EXT_UART->USR & UART_USR_BUSY) == UART_USR_BUSY) {
		(void) EXT_UART->O00.RBR;
	}
}

void Dmx::RdmSendDiscoveryRespondMessage([[maybe_unused]] const uint32_t nPortIndex, const uint8_t *pRdmData, uint32_t nLength) {
	DEBUG_PRINTF("nPortIndex=%u, pRdmData=%p, nLength=%u", nPortIndex, pRdmData, nLength);
	assert(nPortIndex < dmx::config::max::PORTS);
	assert(pRdmData != nullptr);
	assert(nLength != 0);

	// 3.2.2 Responder Packet spacing
	udelay(RDM_RESPONDER_PACKET_SPACING, gsv_RdmDataReceiveEnd);

	SetPortDirection(nPortIndex, dmx::PortDirection::OUTP, false);

	EXT_UART->LCR = UART_LCR_8_N_2;

	for (uint32_t i = 0; i < nLength; i++) {
		while (!(EXT_UART->LSR & UART_LSR_THRE))
			;
		EXT_UART->O00.THR = pRdmData[i];
	}

	while (!((EXT_UART->LSR & UART_LSR_TEMT) == UART_LSR_TEMT))
		;

	udelay(RDM_RESPONDER_DATA_DIRECTION_DELAY);

	SetPortDirection(nPortIndex, dmx::PortDirection::INP, true);

	DEBUG_EXIT
}
