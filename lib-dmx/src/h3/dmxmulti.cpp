/**
 * @file dmxmulti.cpp
 *
 */
/* Copyright (C) 2018-2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <algorithm>
#include <cassert>

#include "h3/dmxmulti.h"

#include "arm/arm.h"
#include "arm/synchronize.h"
#include "arm/gic.h"

#include "h3.h"
#include "h3_hs_timer.h"
#include "h3_dma.h"
#include "h3_ccu.h"
#include "h3_gpio.h"
#include "h3_timer.h"

#include "gpio.h"

#include "irq_timer.h"

#include "uart.h"

#include "dmxgpioparams.h"

#include "dmx_multi_internal.h"

#include "rdm.h"
#include "rdm_e120.h"

#include "uart.h"

#include "debug.h"

extern "C" {
 int console_error(const char *);
}

#ifndef ALIGNED
# define ALIGNED __attribute__ ((aligned (4)))
#endif

static constexpr auto DMX_DATA_OUT_INDEX = (1U << 2);

enum class TxRxState {
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
};

enum class UartState {
	IDLE = 0, TX, RX
};

struct TDmxMultiData {
	uint8_t data[DMX_DATA_BUFFER_SIZE]; // multiple of uint32_t
	uint32_t nLength;
};

struct TCoherentRegion {
	struct sunxi_dma_lli lli[DMX_MAX_OUT];
	struct TDmxMultiData dmx_data[DMX_MAX_OUT][DMX_DATA_OUT_INDEX] ALIGNED;
};

struct TRdmMultiData {
	uint8_t data[RDM_DATA_BUFFER_SIZE];
	uint16_t nChecksum;	// This must be uint16_t
	uint16_t _padding;
	uint32_t nIndex;
	uint32_t nDiscIndex;
};

static uint32_t s_nDmxTransmistBreakTimeINTV;
static uint32_t s_nDmxTransmitMabTimeINTV;
static uint32_t s_nDmxTransmitPeriodINTV;

static struct TCoherentRegion *s_pCoherentRegion;

static volatile uint32_t s_nDmxDataWriteIndex[DMX_MAX_OUT] ALIGNED;
static volatile uint32_t s_nDmxDataReadIndex[DMX_MAX_OUT] ALIGNED;

static volatile TxRxState s_tDmxSendState;

static struct TRdmMultiData s_aRdmData[DMX_MAX_OUT][RDM_DATA_BUFFER_INDEX_ENTRIES] ALIGNED;
static struct TRdmMultiData *s_pRdmDataCurrent[DMX_MAX_OUT] ALIGNED;

static volatile TxRxState s_tRdmReceiveState[DMX_MAX_OUT] ALIGNED;
static volatile uint32_t s_nRdmDataWriteIndex[DMX_MAX_OUT] ALIGNED;
static volatile uint32_t s_nRdmDataReadIndex[DMX_MAX_OUT] ALIGNED;

static volatile UartState s_UartState[DMX_MAX_OUT] ALIGNED;
static volatile uint32_t s_nUartsSending;

static char CONSOLE_ERROR[] ALIGNED = "DMXDATA %\n";
static constexpr auto CONSOLE_ERROR_LENGTH = (sizeof(CONSOLE_ERROR) / sizeof(CONSOLE_ERROR[0]));

static void irq_timer0_dmx_multi_sender(__attribute__((unused))uint32_t clo) {
#ifdef LOGIC_ANALYZER
	h3_gpio_set(6);
#endif

	switch (s_tDmxSendState) {
	case TxRxState::IDLE:
	case TxRxState::DMXINTER:
		H3_TIMER->TMR0_INTV = s_nDmxTransmistBreakTimeINTV;
		H3_TIMER->TMR0_CTRL |= (TIMER_CTRL_EN_START | TIMER_CTRL_RELOAD); // 0x3;

		if (s_UartState[1] == UartState::TX) {
			H3_UART1->LCR = UART_LCR_8_N_2 | UART_LCR_BC;
		}

		if (s_UartState[2] == UartState::TX) {
			H3_UART2->LCR = UART_LCR_8_N_2 | UART_LCR_BC;
		}

#if defined (ORANGE_PI_ONE)
		if (s_UartState[3] == UartState::TX) {
			H3_UART3->LCR = UART_LCR_8_N_2 | UART_LCR_BC;
		}

# ifndef DO_NOT_USE_UART0
		if (s_UartState[0] == UartState::TX) {
			H3_UART0->LCR = UART_LCR_8_N_2 | UART_LCR_BC;
		}
# endif
#endif

		if (s_nDmxDataWriteIndex[1] != s_nDmxDataReadIndex[1]) {
			s_nDmxDataReadIndex[1] = (s_nDmxDataReadIndex[1] + 1) & (DMX_DATA_OUT_INDEX - 1);

			s_pCoherentRegion->lli[1].src = reinterpret_cast<uint32_t>(&s_pCoherentRegion->dmx_data[1][s_nDmxDataReadIndex[1]].data[0]);
			s_pCoherentRegion->lli[1].len = s_pCoherentRegion->dmx_data[1][s_nDmxDataReadIndex[1]].nLength;
		}

		if (s_nDmxDataWriteIndex[2] != s_nDmxDataReadIndex[2]) {
			s_nDmxDataReadIndex[2] = (s_nDmxDataReadIndex[2] + 1) & (DMX_DATA_OUT_INDEX - 1);

			s_pCoherentRegion->lli[2].src = reinterpret_cast<uint32_t>(&s_pCoherentRegion->dmx_data[2][s_nDmxDataReadIndex[2]].data[0]);
			s_pCoherentRegion->lli[2].len = s_pCoherentRegion->dmx_data[2][s_nDmxDataReadIndex[2]].nLength;
		}

#if defined (ORANGE_PI_ONE)
		if (s_nDmxDataWriteIndex[3] != s_nDmxDataReadIndex[3]) {
			s_nDmxDataReadIndex[3] = (s_nDmxDataReadIndex[3] + 1) & (DMX_DATA_OUT_INDEX - 1);

			s_pCoherentRegion->lli[3].src = reinterpret_cast<uint32_t>(&s_pCoherentRegion->dmx_data[3][s_nDmxDataReadIndex[3]].data[0]);
			s_pCoherentRegion->lli[3].len = s_pCoherentRegion->dmx_data[3][s_nDmxDataReadIndex[3]].nLength;
		}

# ifndef DO_NOT_USE_UART0
		if (s_nDmxDataWriteIndex[0] != s_nDmxDataReadIndex[0]) {
			s_nDmxDataReadIndex[0] = (s_nDmxDataReadIndex[0] + 1) & (DMX_DATA_OUT_INDEX - 1);

			s_pCoherentRegion->lli[0].src = reinterpret_cast<uint32_t>(&s_pCoherentRegion->dmx_data[0][s_nDmxDataReadIndex[0]].data[0]);
			s_pCoherentRegion->lli[0].len = s_pCoherentRegion->dmx_data[0][s_nDmxDataReadIndex[0]].nLength;
		}
# endif
#endif

		dmb();
		s_tDmxSendState = TxRxState::BREAK;
		break;
	case TxRxState::BREAK:
		H3_TIMER->TMR0_INTV = s_nDmxTransmitMabTimeINTV;
		H3_TIMER->TMR0_CTRL |= (TIMER_CTRL_EN_START | TIMER_CTRL_RELOAD); // 0x3;

		if (s_UartState[1] == UartState::TX) {
			H3_UART1->LCR = UART_LCR_8_N_2;
		}

		if (s_UartState[2] == UartState::TX) {
			H3_UART2->LCR = UART_LCR_8_N_2;
		}
#if defined (ORANGE_PI_ONE)
		if (s_UartState[3] == UartState::TX) {
			H3_UART3->LCR = UART_LCR_8_N_2;
		}
 #ifndef DO_NOT_USE_UART0
		if (s_UartState[0] == UartState::TX) {
			H3_UART0->LCR = UART_LCR_8_N_2;
		}
 #endif
#endif

		dmb();
		s_tDmxSendState = TxRxState::MAB;
		break;
	case TxRxState::MAB:
		H3_TIMER->TMR0_INTV = s_nDmxTransmitPeriodINTV;
		H3_TIMER->TMR0_CTRL |= (TIMER_CTRL_EN_START | TIMER_CTRL_RELOAD); // 0x3;

		if (s_UartState[1] == UartState::TX) {
			H3_DMA_CHL1->DESC_ADDR = reinterpret_cast<uint32_t>(&s_pCoherentRegion->lli[1]);
			H3_DMA_CHL1->EN = DMA_CHAN_ENABLE_START;
			s_nUartsSending |= (1 << 1);
		}

		if (s_UartState[2] == UartState::TX) {
			H3_DMA_CHL2->DESC_ADDR = reinterpret_cast<uint32_t>(&s_pCoherentRegion->lli[2]);
			H3_DMA_CHL2->EN = DMA_CHAN_ENABLE_START;
			s_nUartsSending |= (1 << 2);
		}
#if defined (ORANGE_PI_ONE)
		if (s_UartState[3] == UartState::TX) {
			H3_DMA_CHL3->DESC_ADDR = reinterpret_cast<uint32_t>(&s_pCoherentRegion->lli[3]);
			H3_DMA_CHL3->EN = DMA_CHAN_ENABLE_START;
			s_nUartsSending |= (1 << 3);
		}
# ifndef DO_NOT_USE_UART0
		if (s_UartState[0] == UartState::TX) {
			H3_DMA_CHL0->DESC_ADDR = reinterpret_cast<uint32_t>(&s_pCoherentRegion->lli[0]);
			H3_DMA_CHL0->EN = DMA_CHAN_ENABLE_START;
			s_nUartsSending |= (1 << 0);
		}
# endif
#endif
		isb();

		if (s_nUartsSending == 0) {
			dmb();
			s_tDmxSendState = TxRxState::DMXINTER;
		} else {
			dmb();
			s_tDmxSendState = TxRxState::DMXDATA;
		}
		break;
	case TxRxState::DMXDATA:
		assert(0);
#ifdef LOGIC_ANALYZER
		h3_gpio_set(20);
#endif
		CONSOLE_ERROR[CONSOLE_ERROR_LENGTH - 3] = '0' + s_nUartsSending;
		console_error(CONSOLE_ERROR);
#ifdef LOGIC_ANALYZER
		h3_gpio_clr(20);
#endif
		// Recover from this internal error.
		s_nUartsSending = 0;
		dmb();
		s_tDmxSendState = TxRxState::DMXINTER;

		H3_TIMER->TMR0_INTV = 12;
		H3_TIMER->TMR0_CTRL |= (TIMER_CTRL_EN_START | TIMER_CTRL_RELOAD); // 0x3;
		break;
	default:
		assert(0);
		break;
	}

#ifdef LOGIC_ANALYZER
	h3_gpio_clr(6);
#endif
}

static void fiq_rdm_in_handler(const uint32_t nUart, const H3_UART_TypeDef *pUart, __attribute__((unused))  const uint32_t nIIR) {
	uint16_t nIndex;

	isb();

	if (pUart->LSR & UART_LSR_BI) {
		s_tRdmReceiveState[nUart] = TxRxState::PRE_BREAK;
	}

	auto nRFL = pUart->RFL;

	while(nRFL--) {
		while ((pUart->LSR & UART_LSR_DR) != UART_LSR_DR)
			;
		const uint8_t nData = pUart->O00.RBR;

		switch (s_tRdmReceiveState[nUart]) {
		case TxRxState::IDLE:
			if (nData == 0xFE) {
				s_pRdmDataCurrent[nUart]->data[0] = 0xFE;
				s_pRdmDataCurrent[nUart]->nIndex = 1;

				s_tRdmReceiveState[nUart] = TxRxState::RDMDISCFE;
			}
			break;
		case TxRxState::PRE_BREAK:
			s_tRdmReceiveState[nUart] = TxRxState::BREAK;
			break;
		case TxRxState::BREAK:
			switch (nData) {
			case E120_SC_RDM:
				s_pRdmDataCurrent[nUart]->data[0] = E120_SC_RDM;
				s_pRdmDataCurrent[nUart]->nChecksum = E120_SC_RDM;
				s_pRdmDataCurrent[nUart]->nIndex = 1;

				s_tRdmReceiveState[nUart] = TxRxState::RDMDATA;
				break;
			default:
				s_tRdmReceiveState[nUart] = TxRxState::IDLE;
				break;
			}
			break;
		case TxRxState::RDMDATA:
			if (s_pRdmDataCurrent[nUart]->nIndex > RDM_DATA_BUFFER_SIZE) {
				s_tRdmReceiveState[nUart] = TxRxState::IDLE;
			} else {
				nIndex = s_pRdmDataCurrent[nUart]->nIndex;
				s_pRdmDataCurrent[nUart]->data[nIndex] = nData;
				s_pRdmDataCurrent[nUart]->nIndex++;

				s_pRdmDataCurrent[nUart]->nChecksum += nData;

				const auto *p = reinterpret_cast<struct _rdm_command *>(&s_pRdmDataCurrent[nUart]->data[0]);

				if (s_pRdmDataCurrent[nUart]->nIndex == p->message_length) {
					s_tRdmReceiveState[nUart] = TxRxState::CHECKSUMH;
				}
			}
			break;
		case TxRxState::CHECKSUMH:
			nIndex = s_pRdmDataCurrent[nUart]->nIndex;
			s_pRdmDataCurrent[nUart]->data[nIndex] = nData;
			s_pRdmDataCurrent[nUart]->nIndex++;

			s_pRdmDataCurrent[nUart]->nChecksum -= nData << 8;

			s_tRdmReceiveState[nUart] = TxRxState::CHECKSUML;
			break;
		case TxRxState::CHECKSUML: {
			nIndex = s_pRdmDataCurrent[nUart]->nIndex;
			s_pRdmDataCurrent[nUart]->data[nIndex] = nData;
			s_pRdmDataCurrent[nUart]->nIndex++;

			s_pRdmDataCurrent[nUart]->nChecksum -= nData;

			const auto *p = reinterpret_cast<struct _rdm_command *>(&s_aRdmData[nUart][s_nRdmDataWriteIndex[nUart]].data[0]);

			if ((s_aRdmData[nUart][s_nRdmDataWriteIndex[nUart]].nChecksum == 0) && (p->sub_start_code == E120_SC_SUB_MESSAGE)) {
				s_nRdmDataWriteIndex[nUart] = (s_nRdmDataWriteIndex[nUart] + 1) & RDM_DATA_BUFFER_INDEX_MASK;
				s_pRdmDataCurrent[nUart] = &s_aRdmData[nUart][s_nRdmDataWriteIndex[nUart]];
			}

			s_tRdmReceiveState[nUart] = TxRxState::IDLE;
		}
			break;
		case TxRxState::RDMDISCFE:
			nIndex = s_pRdmDataCurrent[nUart]->nIndex;
			s_pRdmDataCurrent[nUart]->data[nIndex] = nData;
			s_pRdmDataCurrent[nUart]->nIndex++;

			if ((nData == 0xAA) || (s_pRdmDataCurrent[nUart]->nIndex == 9)) {
				s_pRdmDataCurrent[nUart]->nDiscIndex = 0;

				s_tRdmReceiveState[nUart] = TxRxState::RDMDISCEUID;
			}
			break;
		case TxRxState::RDMDISCEUID:
			nIndex = s_pRdmDataCurrent[nUart]->nIndex;
			s_pRdmDataCurrent[nUart]->data[nIndex] = nData;
			s_pRdmDataCurrent[nUart]->nIndex++;

			s_pRdmDataCurrent[nUart]->nDiscIndex++;

			if (s_pRdmDataCurrent[nUart]->nDiscIndex == 2 * RDM_UID_SIZE) {
				s_pRdmDataCurrent[nUart]->nDiscIndex = 0;

				s_tRdmReceiveState[nUart] = TxRxState::RDMDISCECS;
			}
			break;
		case TxRxState::RDMDISCECS:
			nIndex = s_pRdmDataCurrent[nUart]->nIndex;
			s_pRdmDataCurrent[nUart]->data[nIndex] = nData;
			s_pRdmDataCurrent[nUart]->nIndex++;

			s_pRdmDataCurrent[nUart]->nDiscIndex++;

			if (s_pRdmDataCurrent[nUart]->nDiscIndex == 4) {
				s_nRdmDataWriteIndex[nUart] = (s_nRdmDataWriteIndex[nUart] + 1) & RDM_DATA_BUFFER_INDEX_MASK;
				s_pRdmDataCurrent[nUart] = &s_aRdmData[nUart][s_nRdmDataWriteIndex[nUart]];

				s_tRdmReceiveState[nUart] = TxRxState::IDLE;
			}

			break;
		default:
			s_tRdmReceiveState[nUart] = TxRxState::IDLE;
			break;
		}
	}

	dmb();
}

static void __attribute__((interrupt("FIQ"))) fiq_dmx_multi(void) {
	dmb();

#ifdef LOGIC_ANALYZER
	h3_gpio_set(3);
#endif

	// UART1
	if (H3_DMA->IRQ_PEND0 & (DMA_IRQ_PEND0_DMA1_HALF_IRQ_EN | DMA_IRQ_PEND0_DMA1_PKG_IRQ_EN)) {
		s_nUartsSending &= ~(1U << 1);
	}
	// UART2
	if (H3_DMA->IRQ_PEND0 & (DMA_IRQ_PEND0_DMA2_HALF_IRQ_EN | DMA_IRQ_PEND0_DMA2_PKG_IRQ_EN)) {
		s_nUartsSending &= ~(1U << 2);
	}
#if defined (ORANGE_PI_ONE)
	// UART3
	if (H3_DMA->IRQ_PEND0 & (DMA_IRQ_PEND0_DMA3_HALF_IRQ_EN | DMA_IRQ_PEND0_DMA3_PKG_IRQ_EN)) {
		s_nUartsSending &= ~(1U << 3);
	}
# ifndef DO_NOT_USE_UART0
	// UART0
	if (H3_DMA->IRQ_PEND0 & (DMA_IRQ_PEND0_DMA0_HALF_IRQ_EN | DMA_IRQ_PEND0_DMA0_PKG_IRQ_EN)) {
		s_nUartsSending &= ~(1U << 0);
	}
# endif
#endif

	if (gic_get_active_fiq() == H3_DMA_IRQn) {
		H3_DMA->IRQ_PEND0 |= H3_DMA->IRQ_PEND0;

		H3_GIC_CPUIF->EOI = H3_DMA_IRQn;
		gic_unpend(H3_DMA_IRQn);
		isb();

		if (s_nUartsSending == 0) {
			dmb();
			s_tDmxSendState = TxRxState::DMXINTER;
		}
	}

	auto nIIR = H3_UART1->O08.IIR;
	if (nIIR & UART_IIR_IID_RD) {
		fiq_rdm_in_handler(1, reinterpret_cast<H3_UART_TypeDef *>(H3_UART1_BASE), nIIR);
		H3_GIC_CPUIF->EOI = H3_UART1_IRQn;
		gic_unpend(H3_UART1_IRQn);
	}

	nIIR = H3_UART2->O08.IIR;
	if (nIIR & UART_IIR_IID_RD) {
		fiq_rdm_in_handler(2, reinterpret_cast<H3_UART_TypeDef *>(H3_UART2_BASE), nIIR);
		H3_GIC_CPUIF->EOI = H3_UART2_IRQn;
		gic_unpend(H3_UART2_IRQn);
	}

#if defined (ORANGE_PI_ONE)
	nIIR = H3_UART3->O08.IIR;
	if (nIIR & UART_IIR_IID_RD) {
		fiq_rdm_in_handler(3, reinterpret_cast<H3_UART_TypeDef *>(H3_UART3_BASE), nIIR);
		H3_GIC_CPUIF->EOI = H3_UART3_IRQn;
		gic_unpend(H3_UART3_IRQn);
	}

# ifndef DO_NOT_USE_UART0
	nIIR = H3_UART0->O08.IIR;
	if (nIIR & UART_IIR_IID_RD) {
		fiq_rdm_in_handler(0, reinterpret_cast<H3_UART_TypeDef *>(H3_UART0_BASE), nIIR);
		H3_GIC_CPUIF->EOI = H3_UART0_IRQn;
		gic_unpend(H3_UART0_IRQn);
	}
# endif
#endif

#ifdef LOGIC_ANALYZER
	h3_gpio_clr(3);
#endif
	dmb();
}

DmxMulti::DmxMulti() {
	DEBUG_ENTRY

	s_nDmxTransmistBreakTimeINTV = DMX_TRANSMIT_BREAK_TIME_MIN * 12 ;
	s_nDmxTransmitMabTimeINTV = DMX_TRANSMIT_MAB_TIME_MIN * 12 ;
	s_nDmxTransmitPeriodINTV = (DMX_TRANSMIT_PERIOD_DEFAULT * 12) - (DMX_TRANSMIT_MAB_TIME_MIN * 12) - (DMX_TRANSMIT_BREAK_TIME_MIN * 12);

	s_pCoherentRegion = reinterpret_cast<struct TCoherentRegion *>(H3_MEM_COHERENT_REGION + MEGABYTE/2);

	s_tDmxSendState = TxRxState::IDLE;
	s_nUartsSending = 0;

	for (uint32_t i = 0; i < DMX_MAX_OUT; i++) {
		// DMX TX
		ClearData(i);
		s_nDmxDataWriteIndex[i] = 0;
		s_nDmxDataReadIndex[i] = 0;
		m_nDmxTransmissionLength[i] = 0;
		// DMA UART TX
		auto *lli = &s_pCoherentRegion->lli[i];
		H3_UART_TypeDef *p = _get_uart(i);

		lli->cfg = DMA_CHAN_CFG_DST_IO_MODE | DMA_CHAN_CFG_SRC_LINEAR_MODE
				| DMA_CHAN_CFG_SRC_DRQ(DRQSRC_SDRAM)
				| DMA_CHAN_CFG_DST_DRQ(i + DRQDST_UART0TX);
		lli->src = reinterpret_cast<uint32_t>(&s_pCoherentRegion->dmx_data[i][s_nDmxDataReadIndex[i]].data[0]);
		lli->dst = reinterpret_cast<uint32_t>(&p->O00.THR);
		lli->len = s_pCoherentRegion->dmx_data[i][s_nDmxDataReadIndex[i]].nLength;
		lli->para = DMA_NORMAL_WAIT;
		lli->p_lli_next = DMA_LLI_LAST_ITEM;
		//
		m_tDmxPortDirection[i] = DMXRDM_PORT_DIRECTION_INP;
		//
		s_UartState[i] = UartState::IDLE;
		// RDM RX
		s_nRdmDataWriteIndex[i] = 0;
		s_nRdmDataReadIndex[i] = 0;
		s_pRdmDataCurrent[i] = &s_aRdmData[i][0];
		s_tRdmReceiveState[i] = TxRxState::IDLE;
	}

#ifdef LOGIC_ANALYZER
	h3_gpio_fsel(3, GPIO_FSEL_OUTPUT);
	h3_gpio_clr(3);
	h3_gpio_fsel(6, GPIO_FSEL_OUTPUT);
	h3_gpio_clr(6);
	h3_gpio_fsel(20, GPIO_FSEL_OUTPUT);
	h3_gpio_clr(20);
#endif

		/*
		 * OPIZERO	OPIONE	OUT	PORT
		 * -		UART1	1	0
		 * UART2	UART2	2	1
		 * UART1	UART3	3	2
		 * -		UART0	4	3
		 */

#if defined(ORANGE_PI) || defined(NANO_PI)
	m_nDmxDataDirectionGpioPin[2] = GPIO_DMX_DATA_DIRECTION_OUT_B;
	m_nDmxDataDirectionGpioPin[1] = GPIO_DMX_DATA_DIRECTION_OUT_C;
#elif defined (ORANGE_PI_ONE)
	m_nDmxDataDirectionGpioPin[0] = GPIO_DMX_DATA_DIRECTION_OUT_D;
	m_nDmxDataDirectionGpioPin[1] = GPIO_DMX_DATA_DIRECTION_OUT_A;
	m_nDmxDataDirectionGpioPin[2] = GPIO_DMX_DATA_DIRECTION_OUT_B;
	m_nDmxDataDirectionGpioPin[3] = GPIO_DMX_DATA_DIRECTION_OUT_C;
#endif

	h3_gpio_fsel(m_nDmxDataDirectionGpioPin[1], GPIO_FSEL_OUTPUT);
	h3_gpio_clr (m_nDmxDataDirectionGpioPin[1]);	// 0 = input, 1 = output
	h3_gpio_fsel(m_nDmxDataDirectionGpioPin[2], GPIO_FSEL_OUTPUT);
	h3_gpio_clr(m_nDmxDataDirectionGpioPin[2]);	// 0 = input, 1 = output
#if defined (ORANGE_PI_ONE)
	h3_gpio_fsel(m_nDmxDataDirectionGpioPin[3], GPIO_FSEL_OUTPUT);
	h3_gpio_clr(m_nDmxDataDirectionGpioPin[3]);	// 0 = input, 1 = output
	h3_gpio_fsel(m_nDmxDataDirectionGpioPin[0], GPIO_FSEL_OUTPUT);
	h3_gpio_clr(m_nDmxDataDirectionGpioPin[0]);	// 0 = input, 1 = output
#endif

	s_nUartsSending = 0;

	UartInit(1);
	UartInit(2);
#if defined (ORANGE_PI_ONE)
	UartInit(3);
# ifndef DO_NOT_USE_UART0
	UartInit(0);
# endif
#endif

	__disable_fiq();

	arm_install_handler(reinterpret_cast<unsigned>(fiq_dmx_multi), ARM_VECTOR(ARM_VECTOR_FIQ));

	gic_fiq_config(H3_DMA_IRQn, GIC_CORE0);

	gic_fiq_config(H3_UART1_IRQn, GIC_CORE0);
	gic_fiq_config(H3_UART2_IRQn, GIC_CORE0);
#if defined (ORANGE_PI_ONE)
	gic_fiq_config(H3_UART3_IRQn, GIC_CORE0);
# ifndef DO_NOT_USE_UART0
	gic_fiq_config(H3_UART0_IRQn, GIC_CORE0);
# endif
#endif

	s_tDmxSendState = TxRxState::IDLE;

	UartEnableFifoTx(1);
	UartEnableFifoTx(2);
#if defined (ORANGE_PI_ONE)
	UartEnableFifoTx(3);
# ifndef DO_NOT_USE_UART0
	UartEnableFifoTx(0);
# endif
#endif

	irq_timer_init();
	irq_timer_set(IRQ_TIMER_0, irq_timer0_dmx_multi_sender);

	H3_TIMER->TMR0_CTRL |= TIMER_CTRL_SINGLE_MODE;
	H3_TIMER->TMR0_INTV = 12000; // Wait 1ms
	H3_TIMER->TMR0_CTRL |= (TIMER_CTRL_EN_START | TIMER_CTRL_RELOAD); // 0x3;

	H3_CCU->BUS_SOFT_RESET0 |= CCU_BUS_SOFT_RESET0_DMA;
	H3_CCU->BUS_CLK_GATING0 |= CCU_BUS_CLK_GATING0_DMA;

	H3_DMA->IRQ_PEND0 |= H3_DMA->IRQ_PEND0;
	H3_DMA->IRQ_PEND1 |= H3_DMA->IRQ_PEND1;

	H3_DMA->IRQ_EN0 = DMA_IRQ_EN0_DMA0_PKG_IRQ_EN | DMA_IRQ_EN0_DMA1_PKG_IRQ_EN
			| DMA_IRQ_EN0_DMA2_PKG_IRQ_EN | DMA_IRQ_EN0_DMA3_PKG_IRQ_EN;

	isb();

	__enable_fiq();

	DEBUG_EXIT
}

void DmxMulti::SetDmxBreakTime(uint32_t nBreakTime) {
	m_nDmxTransmitBreakTime = std::max(DMX_TRANSMIT_BREAK_TIME_MIN, nBreakTime);
	s_nDmxTransmistBreakTimeINTV = m_nDmxTransmitBreakTime * 12;
	//
	SetDmxPeriodTime(m_nDmxTransmitPeriodRequested);
}

void DmxMulti::SetDmxMabTime(uint32_t nMabTime) {
	m_nDmxTransmitMabTime = std::max(DMX_TRANSMIT_MAB_TIME_MIN, nMabTime);
	s_nDmxTransmitMabTimeINTV = m_nDmxTransmitMabTime * 12;
	//
	SetDmxPeriodTime(m_nDmxTransmitPeriodRequested);
}

void DmxMulti::SetDmxPeriodTime(uint32_t nPeriod) {
	m_nDmxTransmitPeriodRequested = nPeriod;

	auto nLengthMax = m_nDmxTransmissionLength[0];

	for (uint32_t i = 1; i < DMX_MAX_OUT; i++) {
		if (m_nDmxTransmissionLength[i] > nLengthMax) {
			nLengthMax = m_nDmxTransmissionLength[i];
		}
	}

	const auto nPackageLengthMicroSeconds = m_nDmxTransmitBreakTime + m_nDmxTransmitMabTime + (nLengthMax * 44) + 44;

	if (nPeriod != 0) {
		if (nPeriod < nPackageLengthMicroSeconds) {
			m_nDmxTransmitPeriod = std::max(DMX_TRANSMIT_BREAK_TO_BREAK_TIME_MIN, nPackageLengthMicroSeconds + 44);
		} else {
			m_nDmxTransmitPeriod = nPeriod;
		}
	} else {
		m_nDmxTransmitPeriod =  std::max(DMX_TRANSMIT_BREAK_TO_BREAK_TIME_MIN, nPackageLengthMicroSeconds + 44);
	}

	s_nDmxTransmitPeriodINTV = (m_nDmxTransmitPeriod * 12) - s_nDmxTransmistBreakTimeINTV - s_nDmxTransmitMabTimeINTV;

	DEBUG_PRINTF("nPeriod=%u, nLengthMax=%u, m_nDmxTransmitPeriod=%u", nPeriod, nLengthMax, m_nDmxTransmitPeriod);
}

void DmxMulti::SetPortSendDataWithoutSC(uint8_t nPort, const uint8_t *pData, uint16_t nLength) {
	assert(pData != 0);
	assert(nLength != 0);

	const auto nUart = _port_to_uart(nPort);
	assert(nUart < DMX_MAX_OUT);

	const auto nNext = (s_nDmxDataWriteIndex[nUart] + 1) & (DMX_DATA_OUT_INDEX - 1);
	auto *p = &s_pCoherentRegion->dmx_data[nUart][nNext];

	auto *pDst = p->data;
	p->nLength = nLength + 1U;

	__builtin_prefetch(pData);
	memcpy(&pDst[1], pData,  nLength);

	DEBUG_PRINTF("nLength=%u, m_nDmxTransmissionLength[%u]=%u", nLength, nUart, m_nDmxTransmissionLength[nUart]);

	if (nLength != m_nDmxTransmissionLength[nUart]) {
		m_nDmxTransmissionLength[nUart] = nLength;
		SetDmxPeriodTime(m_nDmxTransmitPeriodRequested);
	}

	s_nDmxDataWriteIndex[nUart] = nNext;
}

void DmxMulti::SetPortDirection(uint8_t nPort, TDmxRdmPortDirection tPortDirection, bool bEnableData) {
	assert(nPort < DMX_MAX_OUT);

	DEBUG_PRINTF("nPort=%d, tPortDirection=%d, bEnableData=%d", nPort, tPortDirection, bEnableData);

	const auto nUart = _port_to_uart(nPort);

	if (tPortDirection != m_tDmxPortDirection[nUart]) {
		StopData(nUart);
		switch (tPortDirection) {
		case DMX_PORT_DIRECTION_OUTP:
			h3_gpio_set(m_nDmxDataDirectionGpioPin[nUart]);	// 0 = input, 1 = output
			m_tDmxPortDirection[nUart] = DMXRDM_PORT_DIRECTION_OUTP;
			break;
		case DMX_PORT_DIRECTION_INP:
			h3_gpio_clr(m_nDmxDataDirectionGpioPin[nUart]);	// 0 = input, 1 = output
			m_tDmxPortDirection[nUart] = DMXRDM_PORT_DIRECTION_INP;
			break;
		default:
			assert(0);
			break;
		}
	} else if (!bEnableData) {
		StopData(nUart);
	}

	if (bEnableData) {
		StartData(nUart);
	}
}

void DmxMulti::RdmSendRaw(uint8_t nPort, const uint8_t* pRdmData, uint16_t nLength) {
	assert(nPort < DMX_MAX_OUT);
	assert(pRdmData != nullptr);
	assert(nLength != 0);

	auto *p = _get_uart(_port_to_uart(nPort));
	assert(p != nullptr);

	while (!(p->LSR & UART_LSR_TEMT))
		;

	p->LCR = UART_LCR_8_N_2 | UART_LCR_BC;
	udelay(RDM_TRANSMIT_BREAK_TIME);

	p->LCR = UART_LCR_8_N_2;
	udelay(RDM_TRANSMIT_MAB_TIME);

	for (uint32_t i = 0; i < nLength; i++) {
		while (!(p->LSR & UART_LSR_THRE))
			;
		p->O00.THR = pRdmData[i];
	}

	while ((p->USR & UART_USR_BUSY) == UART_USR_BUSY) {
		static_cast<void>(EXT_UART->O00.RBR);
	}
}

const uint8_t *DmxMulti::RdmReceive(uint8_t nPort) {
	assert(nPort < DMX_MAX_OUT);

	const auto nUart = _port_to_uart(nPort);

	dmb();
	if (s_nRdmDataWriteIndex[nUart] == s_nRdmDataReadIndex[nUart]) {
		return nullptr;
	} else {
		const auto *p = &s_aRdmData[nUart][s_nRdmDataReadIndex[nUart]].data[0];
		s_nRdmDataReadIndex[nUart] = (s_nRdmDataReadIndex[nUart] + 1) & RDM_DATA_BUFFER_INDEX_MASK;
		return p;
	}
}

const uint8_t *DmxMulti::RdmReceiveTimeOut(uint8_t nPort, uint32_t nTimeOut) {
	assert(nPort < DMX_MAX_OUT);

	uint8_t *p = nullptr;
	const auto nMicros = H3_TIMER->AVS_CNT1;

	do {
		if ((p = const_cast<uint8_t*>(RdmReceive(nPort))) != nullptr) {
			return p;
		}
	} while ((H3_TIMER->AVS_CNT1 - nMicros) < nTimeOut);

	return p;
}

void DmxMulti::UartEnableFifoTx(uint32_t nUart) {	// DMX TX
	auto *pUart = _get_uart(nUart);
	assert(pUart != nullptr);

	if (pUart != nullptr) {
		pUart->O08.FCR = UART_FCR_EFIFO | UART_FCR_TRESET;
		pUart->O04.IER = 0;
		isb();
	}
}

void DmxMulti::UartEnableFifoRx(uint32_t nUart) {	// RDM RX
	auto *pUart = _get_uart(nUart);
	assert(pUart != nullptr);

	if (pUart != nullptr) {
		pUart->O08.FCR = UART_FCR_EFIFO | UART_FCR_RRESET | UART_FCR_TRIG1;
		pUart->O04.IER = UART_IER_ERBFI;
		isb();
	}
}

void DmxMulti::ClearData(uint32_t nUart) {
	for (uint32_t j = 0; j < DMX_DATA_OUT_INDEX; j++) {
		auto *p = &s_pCoherentRegion->dmx_data[nUart][j];
		auto *p32 = reinterpret_cast<uint32_t *>(p->data);

		for (uint32_t i = 0; i < DMX_DATA_BUFFER_SIZE / 4; i++) {
			*p32++ = 0;
		}

		p->nLength = 513; // Including START Code
	}
}

void DmxMulti::StartData(uint32_t nUart) {
	assert(s_UartState[nUart] == UartState::IDLE);

	switch (m_tDmxPortDirection[nUart]) {
	case DMX_PORT_DIRECTION_OUTP:
		UartEnableFifoTx(nUart);
		dmb();
		s_UartState[nUart] = UartState::TX;
		break;
	case DMX_PORT_DIRECTION_INP: {
		s_tRdmReceiveState[nUart] = TxRxState::IDLE;

		auto *p = _get_uart(nUart);
		assert(p != nullptr);

		if (p != nullptr) {
			while (!(p->USR & UART_USR_TFE))
				;
		}

		UartEnableFifoRx(nUart);
		dmb();
		s_UartState[nUart] = UartState::RX;
		break;
	}
	default:
		assert(0);
		break;
	}
}

void DmxMulti::StopData(uint32_t nUart) {
	assert(nUart < DMX_MAX_OUT);

	dmb();
	if (s_UartState[nUart] == UartState::IDLE) {
		return;
	}

	if (m_tDmxPortDirection[nUart] == DMXRDM_PORT_DIRECTION_OUTP) {
		auto *pUart = _get_uart(nUart);
		assert(pUart != nullptr);

		auto IsIdle = false;

		do {
			dmb();
			if (s_tDmxSendState == TxRxState::DMXINTER) {
				while (!(pUart->USR & UART_USR_TFE))
					;
				IsIdle = true;
			}
		} while (!IsIdle);
	}

	s_UartState[nUart] = UartState::IDLE;
	dmb();
}
