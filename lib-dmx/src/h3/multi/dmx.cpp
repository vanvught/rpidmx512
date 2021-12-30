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

#include <cstdint>
#include <cstring>
#include <algorithm>
#include <cassert>

#include "dmx.h"
#include "h3/dmx_config.h"
#include "./../dmx_internal.h"

#include "arm/arm.h"
#include "arm/synchronize.h"
#include "arm/gic.h"

#include "h3.h"
#include "h3_uart.h"
#include "h3_hs_timer.h"
#include "h3_dma.h"
#include "h3_ccu.h"
#include "h3_gpio.h"
#include "h3_timer.h"

#include "irq_timer.h"

#include "rdm.h"
#include "rdm_e120.h"

#include "debug.h"

using namespace dmx;

extern "C" {
int console_error(const char*);
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
	uint8_t data[buffer::SIZE]; // multiple of uint32_t
	uint32_t nLength;
};

struct TCoherentRegion {
	struct sunxi_dma_lli lli[dmxmulti::max::OUT];
	struct TDmxMultiData dmx_data[dmxmulti::max::OUT][DMX_DATA_OUT_INDEX] ALIGNED;
};

struct TRdmMultiData {
	uint8_t data[RDM_DATA_BUFFER_SIZE];
	uint16_t nChecksum;	// This must be uint16_t
	uint16_t _padding;
	uint32_t nIndex;
	uint32_t nDiscIndex;
};

static volatile TxRxState s_tReceiveState[dmxmulti::max::OUT] ALIGNED;

#if defined(ORANGE_PI)
static constexpr uint8_t s_nDmxDataDirectionGpioPin[dmxmulti::max::OUT] = {
		0,
		GPIO_DMX_DATA_DIRECTION_OUT_C,
		GPIO_DMX_DATA_DIRECTION_OUT_B,
		0 };
#else
static constexpr uint8_t s_nDmxDataDirectionGpioPin[dmxmulti::max::OUT] = {
		GPIO_DMX_DATA_DIRECTION_OUT_D,
		GPIO_DMX_DATA_DIRECTION_OUT_A,
		GPIO_DMX_DATA_DIRECTION_OUT_B,
		GPIO_DMX_DATA_DIRECTION_OUT_C };
#endif

// DMX TX

static uint32_t s_nDmxTransmistBreakTimeINTV;
static uint32_t s_nDmxTransmitMabTimeINTV;
static uint32_t s_nDmxTransmitPeriodINTV;

static struct TCoherentRegion *s_pCoherentRegion;

static volatile uint32_t s_nDmxDataWriteIndex[dmxmulti::max::OUT];
static volatile uint32_t s_nDmxDataReadIndex[dmxmulti::max::OUT];

static volatile TxRxState s_tDmxSendState ALIGNED;

// DMX RX

static volatile struct Data s_aDmxData[dmxmulti::max::IN][buffer::INDEX_ENTRIES] ALIGNED;
static volatile uint32_t s_nDmxDataBufferIndexHead[dmxmulti::max::IN];
static volatile uint32_t s_nDmxDataBufferIndexTail[dmxmulti::max::IN];
static volatile uint32_t s_nDmxDataIndex[dmxmulti::max::IN];

static volatile uint32_t s_nDmxUpdatesPerSecond[dmxmulti::max::IN];
static volatile uint32_t s_nDmxPackets[dmxmulti::max::IN];
static volatile uint32_t s_nDmxPacketsPrevious[dmxmulti::max::IN];

// RDM

static struct TRdmMultiData s_aRdmData[dmxmulti::max::OUT][RDM_DATA_BUFFER_INDEX_ENTRIES] ALIGNED;
static struct TRdmMultiData *s_pRdmDataCurrent[dmxmulti::max::OUT] ALIGNED;

static volatile uint32_t s_nRdmDataWriteIndex[dmxmulti::max::OUT];
static volatile uint32_t s_nRdmDataReadIndex[dmxmulti::max::OUT];

static volatile UartState s_UartState[dmxmulti::max::OUT] ALIGNED;

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
# ifndef DO_NOT_USE_UART0
		if (s_UartState[0] == UartState::TX) {
			H3_UART0->LCR = UART_LCR_8_N_2;
		}
# endif
#endif
		s_tDmxSendState = TxRxState::MAB;
		break;
	case TxRxState::MAB:
		H3_TIMER->TMR0_INTV = s_nDmxTransmitPeriodINTV;
		H3_TIMER->TMR0_CTRL |= (TIMER_CTRL_EN_START | TIMER_CTRL_RELOAD); // 0x3;

		if (s_UartState[1] == UartState::TX) {
			H3_DMA_CHL1->DESC_ADDR = reinterpret_cast<uint32_t>(&s_pCoherentRegion->lli[1]);
			H3_DMA_CHL1->EN = DMA_CHAN_ENABLE_START;
		}

		if (s_UartState[2] == UartState::TX) {
			H3_DMA_CHL2->DESC_ADDR = reinterpret_cast<uint32_t>(&s_pCoherentRegion->lli[2]);
			H3_DMA_CHL2->EN = DMA_CHAN_ENABLE_START;
		}
#if defined (ORANGE_PI_ONE)
		if (s_UartState[3] == UartState::TX) {
			H3_DMA_CHL3->DESC_ADDR = reinterpret_cast<uint32_t>(&s_pCoherentRegion->lli[3]);
			H3_DMA_CHL3->EN = DMA_CHAN_ENABLE_START;
		}
# ifndef DO_NOT_USE_UART0
		if (s_UartState[0] == UartState::TX) {
			H3_DMA_CHL0->DESC_ADDR = reinterpret_cast<uint32_t>(&s_pCoherentRegion->lli[0]);
			H3_DMA_CHL0->EN = DMA_CHAN_ENABLE_START;
		}
# endif
#endif
		isb();

		s_tDmxSendState = TxRxState::DMXINTER;
		break;
	default:
		assert(0);
		__builtin_unreachable();
		break;
	}
#ifdef LOGIC_ANALYZER
	h3_gpio_clr(6);
#endif
}

#include <cstdio>

static void fiq_in_handler(const uint32_t nUart, const H3_UART_TypeDef *pUart, const uint32_t nIIR) {
#ifdef LOGIC_ANALYZER
	h3_gpio_set(3);
#endif
	uint32_t nIndex;

	isb();

	if (pUart->LSR & (UART_LSR_BI | UART_LSR_FE | UART_LSR_FIFOERR)) {
		s_tReceiveState[nUart] = TxRxState::PRE_BREAK;
	}

	auto nRFL = pUart->RFL;

	while(nRFL--) {
		while ((pUart->LSR & UART_LSR_DR) != UART_LSR_DR)
			;
		const auto nData = static_cast<uint8_t>(pUart->O00.RBR);
		dmb();

		switch (s_tReceiveState[nUart]) {
		case TxRxState::IDLE:
			s_pRdmDataCurrent[nUart]->data[0] = nData;
			s_pRdmDataCurrent[nUart]->nIndex = 1;

			s_tReceiveState[nUart] = TxRxState::RDMDISCFE;
			break;
		case TxRxState::PRE_BREAK:
			s_tReceiveState[nUart] = TxRxState::BREAK;
			break;
		case TxRxState::BREAK:
			switch (nData) {
			case START_CODE:
				s_tReceiveState[nUart] = TxRxState::DMXDATA;
				s_aDmxData[nUart][s_nDmxDataBufferIndexHead[nUart]].Data[0] = START_CODE;
				s_nDmxDataIndex[nUart] = 1;
				s_nDmxPackets[nUart]++;
				break;
			case E120_SC_RDM:
				s_pRdmDataCurrent[nUart]->data[0] = E120_SC_RDM;
				s_pRdmDataCurrent[nUart]->nChecksum = E120_SC_RDM;
				s_pRdmDataCurrent[nUart]->nIndex = 1;

				s_tReceiveState[nUart] = TxRxState::RDMDATA;
				break;
			default:
				s_tReceiveState[nUart] = TxRxState::IDLE;
				break;
			}
			break;
		case TxRxState::DMXDATA:
			s_aDmxData[nUart][s_nDmxDataBufferIndexHead[nUart]].Data[s_nDmxDataIndex[nUart]] = nData;
			s_nDmxDataIndex[nUart]++;

			if (s_nDmxDataIndex[nUart] > max::CHANNELS) {
				s_tReceiveState[nUart] = TxRxState::IDLE;
				s_aDmxData[nUart][s_nDmxDataBufferIndexHead[nUart]].Statistics.nSlotsInPacket = max::CHANNELS;
				s_nDmxDataBufferIndexHead[nUart] = (s_nDmxDataBufferIndexHead[nUart] + 1) & buffer::INDEX_MASK;
				return;
			}
			break;
		case TxRxState::RDMDATA:
			if (s_pRdmDataCurrent[nUart]->nIndex > RDM_DATA_BUFFER_SIZE) {
				s_tReceiveState[nUart] = TxRxState::IDLE;
			} else {
				nIndex = s_pRdmDataCurrent[nUart]->nIndex;
				s_pRdmDataCurrent[nUart]->data[nIndex] = nData;
				s_pRdmDataCurrent[nUart]->nIndex++;

				s_pRdmDataCurrent[nUart]->nChecksum = static_cast<uint16_t>(s_pRdmDataCurrent[nUart]->nChecksum + nData);

				const auto *p = reinterpret_cast<struct TRdmMessage *>(&s_pRdmDataCurrent[nUart]->data[0]);

				if (s_pRdmDataCurrent[nUart]->nIndex == p->message_length) {
					s_tReceiveState[nUart] = TxRxState::CHECKSUMH;
				}
			}
			break;
		case TxRxState::CHECKSUMH:
			nIndex = s_pRdmDataCurrent[nUart]->nIndex;
			s_pRdmDataCurrent[nUart]->data[nIndex] = nData;
			s_pRdmDataCurrent[nUart]->nIndex++;
			s_pRdmDataCurrent[nUart]->nChecksum = static_cast<uint16_t>(s_pRdmDataCurrent[nUart]->nChecksum - static_cast<uint16_t>(nData << 8));

			s_tReceiveState[nUart] = TxRxState::CHECKSUML;
			break;
		case TxRxState::CHECKSUML: {
			nIndex = s_pRdmDataCurrent[nUart]->nIndex;
			s_pRdmDataCurrent[nUart]->data[nIndex] = nData;
			s_pRdmDataCurrent[nUart]->nIndex++;

			s_pRdmDataCurrent[nUart]->nChecksum = static_cast<uint16_t>(s_pRdmDataCurrent[nUart]->nChecksum - nData);

			const auto *p = reinterpret_cast<struct TRdmMessage *>(&s_aRdmData[nUart][s_nRdmDataWriteIndex[nUart]].data[0]);

			if ((s_aRdmData[nUart][s_nRdmDataWriteIndex[nUart]].nChecksum == 0) && (p->sub_start_code == E120_SC_SUB_MESSAGE)) {
				s_nRdmDataWriteIndex[nUart] = (s_nRdmDataWriteIndex[nUart] + 1) & RDM_DATA_BUFFER_INDEX_MASK;
				s_pRdmDataCurrent[nUart] = &s_aRdmData[nUart][s_nRdmDataWriteIndex[nUart]];
			}

			s_tReceiveState[nUart] = TxRxState::IDLE;
		}
			break;
		case TxRxState::RDMDISCFE:
			nIndex = s_pRdmDataCurrent[nUart]->nIndex;
			s_pRdmDataCurrent[nUart]->data[nIndex] = nData;
			s_pRdmDataCurrent[nUart]->nIndex++;

			if ((nData == 0xAA) || (s_pRdmDataCurrent[nUart]->nIndex == 9)) {
				s_pRdmDataCurrent[nUart]->nDiscIndex = 0;

				s_tReceiveState[nUart] = TxRxState::RDMDISCEUID;
			}
			break;
		case TxRxState::RDMDISCEUID:
			nIndex = s_pRdmDataCurrent[nUart]->nIndex;
			s_pRdmDataCurrent[nUart]->data[nIndex] = nData;
			s_pRdmDataCurrent[nUart]->nIndex++;

			s_pRdmDataCurrent[nUart]->nDiscIndex++;

			if (s_pRdmDataCurrent[nUart]->nDiscIndex == 2 * RDM_UID_SIZE) {
				s_pRdmDataCurrent[nUart]->nDiscIndex = 0;

				s_tReceiveState[nUart] = TxRxState::RDMDISCECS;
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

				s_tReceiveState[nUart] = TxRxState::IDLE;
			}

			break;
		default:
			s_tReceiveState[nUart] = TxRxState::IDLE;
			break;
		}
	}

	if (((pUart->USR & UART_USR_BUSY) == 0) && ((nIIR & UART_IIR_IID_TIME_OUT) == UART_IIR_IID_TIME_OUT)) {
		if (s_tReceiveState[nUart] == TxRxState::DMXDATA) {
			s_tReceiveState[nUart] = TxRxState::IDLE;
			s_aDmxData[nUart][s_nDmxDataBufferIndexHead[nUart]].Statistics.nSlotsInPacket = s_nDmxDataIndex[nUart] - 1;
			s_nDmxDataBufferIndexHead[nUart] = (s_nDmxDataBufferIndexHead[nUart] + 1) & buffer::INDEX_MASK;
		}

		if ((s_tReceiveState[nUart] == TxRxState::RDMDISCEUID) || (s_tReceiveState[nUart] == TxRxState::RDMDISCECS)) {
			s_nRdmDataWriteIndex[nUart] = (s_nRdmDataWriteIndex[nUart] + 1) & RDM_DATA_BUFFER_INDEX_MASK;
			s_pRdmDataCurrent[nUart] = &s_aRdmData[nUart][s_nRdmDataWriteIndex[nUart]];
			s_tReceiveState[nUart] = TxRxState::IDLE;
			h3_gpio_clr(10);
		}
	}
#ifdef LOGIC_ANALYZER
	h3_gpio_clr(3);
#endif
}

static void __attribute__((interrupt("FIQ"))) fiq_dmx_multi(void) {
	dmb();
#ifdef LOGIC_ANALYZER
	h3_gpio_set(3);
#endif
	auto nIIR = H3_UART1->O08.IIR;
	if (nIIR & UART_IIR_IID_RD) {
		fiq_in_handler(1, reinterpret_cast<H3_UART_TypeDef *>(H3_UART1_BASE), nIIR);
		H3_GIC_CPUIF->EOI = H3_UART1_IRQn;
		gic_unpend(H3_UART1_IRQn);
	}

	nIIR = H3_UART2->O08.IIR;
	if (nIIR & UART_IIR_IID_RD) {
		fiq_in_handler(2, reinterpret_cast<H3_UART_TypeDef *>(H3_UART2_BASE), nIIR);
		H3_GIC_CPUIF->EOI = H3_UART2_IRQn;
		gic_unpend(H3_UART2_IRQn);
	}
#if defined (ORANGE_PI_ONE)
	nIIR = H3_UART3->O08.IIR;
	if (nIIR & UART_IIR_IID_RD) {
		fiq_in_handler(3, reinterpret_cast<H3_UART_TypeDef *>(H3_UART3_BASE), nIIR);
		H3_GIC_CPUIF->EOI = H3_UART3_IRQn;
		gic_unpend(H3_UART3_IRQn);
	}

# ifndef DO_NOT_USE_UART0
	nIIR = H3_UART0->O08.IIR;
	if (nIIR & UART_IIR_IID_RD) {
		fiq_in_handler(0, reinterpret_cast<H3_UART_TypeDef *>(H3_UART0_BASE), nIIR);
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

static void irq_timer1_dmx_receive(__attribute__((unused)) uint32_t clo) {
	for (uint32_t i = 0; i < dmxmulti::max::IN; i++) {
		s_nDmxUpdatesPerSecond[i] = s_nDmxPackets[i] - s_nDmxPacketsPrevious[i];
		s_nDmxPacketsPrevious[i] = s_nDmxPackets[i];
	}
}

static void uart_config(uint32_t nUart) {
	H3_UART_TypeDef *p = nullptr;

	if (nUart == 1) {
		p = reinterpret_cast<H3_UART_TypeDef *>(H3_UART1_BASE);

		uint32_t value = H3_PIO_PORTG->CFG0;
		// PG6, TX
		value &= static_cast<uint32_t>(~(GPIO_SELECT_MASK << PG6_SELECT_CFG0_SHIFT));
		value |= H3_PG6_SELECT_UART1_TX << PG6_SELECT_CFG0_SHIFT;
		// PG7, RX
		value &= static_cast<uint32_t>(~(GPIO_SELECT_MASK << PG7_SELECT_CFG0_SHIFT));
		value |= H3_PG7_SELECT_UART1_RX << PG7_SELECT_CFG0_SHIFT;
		H3_PIO_PORTG->CFG0 = value;

		H3_CCU->BUS_SOFT_RESET4 |= CCU_BUS_SOFT_RESET4_UART1;
		H3_CCU->BUS_CLK_GATING3 |= CCU_BUS_CLK_GATING3_UART1;
	} else if (nUart == 2) {
		p = reinterpret_cast<H3_UART_TypeDef *>(H3_UART2_BASE);

		uint32_t value = H3_PIO_PORTA->CFG0;
		// PA0, TX
		value &= static_cast<uint32_t>(~(GPIO_SELECT_MASK << PA0_SELECT_CFG0_SHIFT));
		value |= H3_PA0_SELECT_UART2_TX << PA0_SELECT_CFG0_SHIFT;
		// PA1, RX
		value &= static_cast<uint32_t>(~(GPIO_SELECT_MASK << PA1_SELECT_CFG0_SHIFT));
		value |= H3_PA1_SELECT_UART2_RX << PA1_SELECT_CFG0_SHIFT;
		H3_PIO_PORTA->CFG0 = value;

		H3_CCU->BUS_SOFT_RESET4 |= CCU_BUS_SOFT_RESET4_UART2;
		H3_CCU->BUS_CLK_GATING3 |= CCU_BUS_CLK_GATING3_UART2;
	}
#if defined (ORANGE_PI_ONE)
	else if (nUart == 3) {
		p = reinterpret_cast<H3_UART_TypeDef *>(H3_UART3_BASE);

		uint32_t value = H3_PIO_PORTA->CFG1;
		// PA13, TX
		value &= static_cast<uint32_t>(~(GPIO_SELECT_MASK << PA13_SELECT_CFG1_SHIFT));
		value |= H3_PA13_SELECT_UART3_TX << PA13_SELECT_CFG1_SHIFT;
		// PA14, RX
		value &= static_cast<uint32_t>(~(GPIO_SELECT_MASK << PA14_SELECT_CFG1_SHIFT));
		value |= H3_PA14_SELECT_UART3_RX << PA14_SELECT_CFG1_SHIFT;
		H3_PIO_PORTA->CFG1 = value;

		H3_CCU->BUS_SOFT_RESET4 |= CCU_BUS_SOFT_RESET4_UART3;
		H3_CCU->BUS_CLK_GATING3 |= CCU_BUS_CLK_GATING3_UART3;
	}
# ifndef DO_NOT_USE_UART0
	else if (nUart == 0) {
		p = reinterpret_cast<H3_UART_TypeDef *>(H3_UART0_BASE);

		uint32_t value = H3_PIO_PORTA->CFG0;
		// PA4, TX
		value &= static_cast<uint32_t>(~(GPIO_SELECT_MASK << PA4_SELECT_CFG0_SHIFT));
		value |= H3_PA4_SELECT_UART0_TX << PA4_SELECT_CFG0_SHIFT;
		// PA5, RX
		value &= static_cast<uint32_t>(~(GPIO_SELECT_MASK << PA5_SELECT_CFG0_SHIFT));
		value |= H3_PA5_SELECT_UART0_RX << PA5_SELECT_CFG0_SHIFT;
		H3_PIO_PORTA->CFG0 = value;

		H3_CCU->BUS_SOFT_RESET4 |= CCU_BUS_SOFT_RESET4_UART0;
		H3_CCU->BUS_CLK_GATING3 |= CCU_BUS_CLK_GATING3_UART0;
	}
# endif
#endif

	assert(p != nullptr);

	if (p != nullptr) {
		p->O08.FCR = 0;
		p->LCR = UART_LCR_DLAB;
		p->O00.DLL = BAUD_250000_L;
		p->O04.DLH = BAUD_250000_H;
		p->O04.IER = 0;
		p->LCR = UART_LCR_8_N_2;
	}

	isb();
}

static void UartEnableFifoTx(uint32_t nUart) {	// DMX TX
	auto *pUart = _get_uart(nUart);
	assert(pUart != nullptr);

	if (pUart != nullptr) {
		pUart->O08.FCR = UART_FCR_EFIFO | UART_FCR_TRESET;
		pUart->O04.IER = 0;
		isb();
	}
}

static void UartEnableFifoRx(uint32_t nUart) {	// RDM RX
	auto *pUart = _get_uart(nUart);
	assert(pUart != nullptr);

	if (pUart != nullptr) {
		pUart->O08.FCR = UART_FCR_EFIFO | UART_FCR_RRESET | UART_FCR_TRIG1;
		pUart->O04.IER = UART_IER_ERBFI;
		isb();
	}
}

Dmx *Dmx::s_pThis = nullptr;

Dmx::Dmx() {
	DEBUG_ENTRY

	assert(s_pThis == nullptr);
	s_pThis = this;

	// DMX TX

	s_pCoherentRegion = reinterpret_cast<struct TCoherentRegion *>(H3_MEM_COHERENT_REGION + MEGABYTE/2);

	s_nDmxTransmistBreakTimeINTV = transmit::BREAK_TIME_MIN * 12 ;
	s_nDmxTransmitMabTimeINTV = transmit::MAB_TIME_MIN * 12 ;
	s_nDmxTransmitPeriodINTV = (transmit::PERIOD_DEFAULT * 12) - (transmit::MAB_TIME_MIN * 12) - (transmit::BREAK_TIME_MIN * 12);

	s_tDmxSendState = TxRxState::IDLE;

	for (uint32_t i = 0; i < dmxmulti::max::OUT; i++) {
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
		m_tDmxPortDirection[i] = PortDirection::INP;
		//
		s_UartState[i] = UartState::IDLE;
		// RDM RX
		s_nRdmDataWriteIndex[i] = 0;
		s_nRdmDataReadIndex[i] = 0;
		s_pRdmDataCurrent[i] = &s_aRdmData[i][0];
		s_tReceiveState[i] = TxRxState::IDLE;
		// DMX RX
		s_nDmxDataBufferIndexHead[i] = 0;
		s_nDmxDataBufferIndexTail[i] = 0;
		s_nDmxDataIndex[i] = 0;
		s_nDmxUpdatesPerSecond[i] = 0;
		s_nDmxPackets[i] = 0;
		s_nDmxPacketsPrevious[i] = 0;
	}

#ifdef LOGIC_ANALYZER
	h3_gpio_fsel(3, GPIO_FSEL_OUTPUT);
	h3_gpio_clr(3);
	h3_gpio_fsel(6, GPIO_FSEL_OUTPUT);
	h3_gpio_clr(6);
	h3_gpio_fsel(10, GPIO_FSEL_OUTPUT);
	h3_gpio_clr(10);
#endif

		/*
		 * OPIZERO	OPIONE	OUT	PORT
		 * -		UART1	1	0
		 * UART2	UART2	2	1
		 * UART1	UART3	3	2
		 * -		UART0	4	3
		 */

	h3_gpio_fsel(s_nDmxDataDirectionGpioPin[1], GPIO_FSEL_OUTPUT);
	h3_gpio_clr(s_nDmxDataDirectionGpioPin[1]);	// 0 = input, 1 = output
	h3_gpio_fsel(s_nDmxDataDirectionGpioPin[2], GPIO_FSEL_OUTPUT);
	h3_gpio_clr(s_nDmxDataDirectionGpioPin[2]);	// 0 = input, 1 = output
#if defined (ORANGE_PI_ONE)
	h3_gpio_fsel(s_nDmxDataDirectionGpioPin[3], GPIO_FSEL_OUTPUT);
	h3_gpio_clr(s_nDmxDataDirectionGpioPin[3]);	// 0 = input, 1 = output
	h3_gpio_fsel(s_nDmxDataDirectionGpioPin[0], GPIO_FSEL_OUTPUT);
	h3_gpio_clr(s_nDmxDataDirectionGpioPin[0]);	// 0 = input, 1 = output
#endif

	uart_config(1);
	uart_config(2);
#if defined (ORANGE_PI_ONE)
	uart_config(3);
# ifndef DO_NOT_USE_UART0
	uart_config(0);
# endif
#endif

	__disable_fiq();

	arm_install_handler(reinterpret_cast<unsigned>(fiq_dmx_multi), ARM_VECTOR(ARM_VECTOR_FIQ));

	gic_fiq_config(H3_UART1_IRQn, GIC_CORE0);
	gic_fiq_config(H3_UART2_IRQn, GIC_CORE0);
#if defined (ORANGE_PI_ONE)
	gic_fiq_config(H3_UART3_IRQn, GIC_CORE0);
# ifndef DO_NOT_USE_UART0
	gic_fiq_config(H3_UART0_IRQn, GIC_CORE0);
# endif
#endif

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
	irq_timer_set(IRQ_TIMER_1, irq_timer1_dmx_receive);

	H3_TIMER->TMR0_CTRL |= TIMER_CTRL_SINGLE_MODE;
	H3_TIMER->TMR0_INTV = 12000; // Wait 1ms
	H3_TIMER->TMR0_CTRL |= (TIMER_CTRL_EN_START | TIMER_CTRL_RELOAD); // 0x3;

	H3_TIMER->TMR1_INTV = 0xB71B00; // 1 second
	H3_TIMER->TMR1_CTRL &= ~(TIMER_CTRL_SINGLE_MODE);
	H3_TIMER->TMR1_CTRL |= (TIMER_CTRL_EN_START | TIMER_CTRL_RELOAD);

	H3_CCU->BUS_SOFT_RESET0 |= CCU_BUS_SOFT_RESET0_DMA;
	H3_CCU->BUS_CLK_GATING0 |= CCU_BUS_CLK_GATING0_DMA;

#if 0
	H3_DMA->IRQ_PEND0 |= H3_DMA->IRQ_PEND0;
	H3_DMA->IRQ_PEND1 |= H3_DMA->IRQ_PEND1;

	H3_DMA->IRQ_EN0 = DMA_IRQ_EN0_DMA0_PKG_IRQ_EN | DMA_IRQ_EN0_DMA1_PKG_IRQ_EN
			| DMA_IRQ_EN0_DMA2_PKG_IRQ_EN | DMA_IRQ_EN0_DMA3_PKG_IRQ_EN;
#endif
	isb();

	__enable_fiq();

	DEBUG_EXIT
}

void Dmx::SetPortDirection(uint32_t nPortIndex, PortDirection tPortDirection, bool bEnableData) {
	assert(nPortIndex < dmxmulti::max::OUT);

	DEBUG_PRINTF("nPort=%d, tPortDirection=%d, bEnableData=%d", nPortIndex, tPortDirection, bEnableData);

	const auto nUart = _port_to_uart(nPortIndex);

	if (tPortDirection != m_tDmxPortDirection[nUart]) {
		StopData(nUart, nPortIndex);
		switch (tPortDirection) {
		case PortDirection::OUTP:
			h3_gpio_set(s_nDmxDataDirectionGpioPin[nUart]);	// 0 = input, 1 = output
			m_tDmxPortDirection[nUart] = PortDirection::OUTP;
			break;
		case PortDirection::INP:
			h3_gpio_clr(s_nDmxDataDirectionGpioPin[nUart]);	// 0 = input, 1 = output
			m_tDmxPortDirection[nUart] = PortDirection::INP;
			break;
		default:
			assert(0);
			__builtin_unreachable();
			break;
		}
	} else if (!bEnableData) {
		StopData(nUart, nPortIndex);
	}

	if (bEnableData) {
		StartData(nUart, nPortIndex);
	}
}

void Dmx::ClearData(uint32_t nUart) {
	for (uint32_t j = 0; j < DMX_DATA_OUT_INDEX; j++) {
		auto *p = &s_pCoherentRegion->dmx_data[nUart][j];
		auto *p32 = reinterpret_cast<uint32_t *>(p->data);

		for (uint32_t i = 0; i < buffer::SIZE / 4; i++) {
			*p32++ = 0;
		}

		p->nLength = 513; // Including START Code
	}
}

void Dmx::StartData(uint32_t nUart, __attribute__((unused)) uint32_t nPortIndex) {
	assert(s_UartState[nUart] == UartState::IDLE);

	switch (m_tDmxPortDirection[nUart]) {
	case PortDirection::OUTP:
		UartEnableFifoTx(nUart);
		s_UartState[nUart] = UartState::TX;
		dmb();
		break;
	case PortDirection::INP: {
		auto *p = _get_uart(nUart);
		assert(p != nullptr);

		if (p != nullptr) {
			while (!(p->USR & UART_USR_TFE))
				;
		}

		UartEnableFifoRx(nUart);
		s_tReceiveState[nUart] = TxRxState::IDLE;
		s_UartState[nUart] = UartState::RX;
		dmb();
		break;
	}
	default:
		assert(0);
		__builtin_unreachable();
		break;
	}
}

void Dmx::StopData(uint32_t nUart, __attribute__((unused)) uint32_t nPortIndex) {
	assert(nUart < dmxmulti::max::OUT);

	dmb();
	if (s_UartState[nUart] == UartState::IDLE) {
		return;
	}

	auto *pUart = _get_uart(nUart);
	assert(pUart != nullptr);

	if (m_tDmxPortDirection[nUart] == PortDirection::OUTP) {
		auto IsIdle = false;

		do {
			dmb();
			if (s_tDmxSendState == TxRxState::DMXINTER) {
				while (!(pUart->USR & UART_USR_TFE))
					;
				IsIdle = true;
			}
		} while (!IsIdle);
	} else if (m_tDmxPortDirection[nUart] == PortDirection::INP) {
		pUart->O08.FCR = 0;
		pUart->O04.IER = 0;

		s_tReceiveState[nUart] = TxRxState::IDLE;
	}

	s_UartState[nUart] = UartState::IDLE;
	dmb();
}

// DMX Send

void Dmx::SetDmxBreakTime(uint32_t nBreakTime) {
	DEBUG_PRINTF("nBreakTime=%u", nBreakTime);

	m_nDmxTransmitBreakTime = std::max(transmit::BREAK_TIME_MIN, nBreakTime);
	s_nDmxTransmistBreakTimeINTV = m_nDmxTransmitBreakTime * 12;
	//
	SetDmxPeriodTime(m_nDmxTransmitPeriodRequested);
}

void Dmx::SetDmxMabTime(uint32_t nMabTime) {
	DEBUG_PRINTF("nMabTime=%u", nMabTime);

	m_nDmxTransmitMabTime = std::min(std::max(transmit::MAB_TIME_MIN, nMabTime), transmit::MAB_TIME_MAX);
	s_nDmxTransmitMabTimeINTV = m_nDmxTransmitMabTime * 12;
	//
	SetDmxPeriodTime(m_nDmxTransmitPeriodRequested);
}

void Dmx::SetDmxPeriodTime(uint32_t nPeriod) {
	DEBUG_ENTRY
	DEBUG_PRINTF("nPeriod=%u", nPeriod);

	m_nDmxTransmitPeriodRequested = nPeriod;

	auto nLengthMax = m_nDmxTransmissionLength[0];

	DEBUG_PRINTF("nLengthMax=%u", nLengthMax);

	for (uint32_t i = 1; i < dmxmulti::max::OUT; i++) {
		if (m_nDmxTransmissionLength[i] > nLengthMax) {
			nLengthMax = m_nDmxTransmissionLength[i];
		}
	}

	const auto nPackageLengthMicroSeconds = m_nDmxTransmitBreakTime + m_nDmxTransmitMabTime + (nLengthMax * 44) + 44;

	if (nPeriod != 0) {
		if (nPeriod < nPackageLengthMicroSeconds) {
			m_nDmxTransmitPeriod = std::max(transmit::BREAK_TO_BREAK_TIME_MIN, nPackageLengthMicroSeconds + 44);
		} else {
			m_nDmxTransmitPeriod = nPeriod;
		}
	} else {
		m_nDmxTransmitPeriod =  std::max(transmit::BREAK_TO_BREAK_TIME_MIN, nPackageLengthMicroSeconds + 44);
	}

	s_nDmxTransmitPeriodINTV = (m_nDmxTransmitPeriod * 12) - s_nDmxTransmistBreakTimeINTV - s_nDmxTransmitMabTimeINTV;

	DEBUG_PRINTF("nPeriod=%u, nLengthMax=%u, m_nDmxTransmitPeriod=%u", nPeriod, nLengthMax, m_nDmxTransmitPeriod);
	DEBUG_ENTRY
}

void Dmx::SetDmxSlots(uint16_t nSlots) {
	DEBUG_ENTRY
	DEBUG_PRINTF("nSlots=%u", nSlots);

	if ((nSlots >= 2) && (nSlots <= dmx::max::CHANNELS)) {
		m_nDmxTransmitSlots = nSlots;

		for (uint32_t i = 0; i < dmxmulti::max::OUT; i++) {
			if (m_nDmxTransmissionLength[i] != 0) {
				m_nDmxTransmissionLength[i] = std::min(m_nDmxTransmissionLength[i], static_cast<uint32_t>(nSlots));
				DEBUG_PRINTF("m_nDmxTransmissionLength[%u]=%u", i, m_nDmxTransmissionLength[i]);
			}
		}

		SetDmxPeriodTime(m_nDmxTransmitPeriodRequested);
	}

	DEBUG_EXIT
}

void Dmx::SetPortSendDataWithoutSC(uint32_t nPortIndex, const uint8_t *pData, uint32_t nLength) {
	assert(pData != 0);
	assert(nLength != 0);

	const auto nUart = _port_to_uart(nPortIndex);
	assert(nUart < dmxmulti::max::OUT);

	const auto nNext = (s_nDmxDataWriteIndex[nUart] + 1) & (DMX_DATA_OUT_INDEX - 1);
	auto *p = &s_pCoherentRegion->dmx_data[nUart][nNext];

	auto *pDst = p->data;
	nLength = std::min(nLength, static_cast<uint32_t>(m_nDmxTransmitSlots));
	p->nLength = nLength + 1U;

	__builtin_prefetch(pData);
	memcpy(&pDst[1], pData,  nLength);

//	DEBUG_PRINTF("nLength=%u, m_nDmxTransmissionLength[%u]=%u", nLength, nUart, m_nDmxTransmissionLength[nUart]);

	if (nLength != m_nDmxTransmissionLength[nUart]) {
		m_nDmxTransmissionLength[nUart] = nLength;
		SetDmxPeriodTime(m_nDmxTransmitPeriodRequested);
	}

	s_nDmxDataWriteIndex[nUart] = nNext;
}

// DMX Receive

const uint8_t *Dmx::GetDmxAvailable(uint32_t nPortIndex)  {
	const auto nUart = _port_to_uart(nPortIndex);

	dmb();
	if (s_nDmxDataBufferIndexHead[nUart] == s_nDmxDataBufferIndexTail[nUart]) {
		return nullptr;
	} else {
		const auto *p = const_cast<const uint8_t *>(s_aDmxData[nUart][s_nDmxDataBufferIndexTail[nUart]].Data);
		s_nDmxDataBufferIndexTail[nUart] = (s_nDmxDataBufferIndexTail[nUart] + 1) & buffer::INDEX_MASK;
		return p;
	}
}

uint32_t Dmx::GetUpdatesPerSecond(uint32_t nPortIndex) {
	const auto uart = _port_to_uart(nPortIndex);

	dmb();
	return s_nDmxUpdatesPerSecond[uart];
}

// RDM Send

void Dmx::RdmSendRaw(uint32_t nPortIndex, const uint8_t* pRdmData, uint32_t nLength) {
	assert(nPortIndex < dmxmulti::max::OUT);
	assert(pRdmData != nullptr);
	assert(nLength != 0);

	auto *p = _get_uart(_port_to_uart(nPortIndex));
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

// RDM Receive

const uint8_t *Dmx::RdmReceive(uint32_t nPortIndex) {
	assert(nPortIndex < dmxmulti::max::OUT);

	const auto nUart = _port_to_uart(nPortIndex);

	dmb();
	if (s_nRdmDataWriteIndex[nUart] == s_nRdmDataReadIndex[nUart]) {
		return nullptr;
	} else {
		const auto *p = &s_aRdmData[nUart][s_nRdmDataReadIndex[nUart]].data[0];
		s_nRdmDataReadIndex[nUart] = (s_nRdmDataReadIndex[nUart] + 1) & RDM_DATA_BUFFER_INDEX_MASK;
		return p;
	}
}

const uint8_t *Dmx::RdmReceiveTimeOut(uint32_t nPortIndex, uint16_t nTimeOut) {
	assert(nPortIndex < dmxmulti::max::OUT);

	uint8_t *p = nullptr;
	const auto nMicros = H3_TIMER->AVS_CNT1;

	do {
		if ((p = const_cast<uint8_t*>(RdmReceive(nPortIndex))) != nullptr) {
			return p;
		}
	} while ((H3_TIMER->AVS_CNT1 - nMicros) < nTimeOut);

	return p;
}
