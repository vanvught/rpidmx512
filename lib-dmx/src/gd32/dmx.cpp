/**
 * @file dmx.cpp
 *
 */
/* Copyright (C) 2021-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#if defined (CONFIG_TIMER6_HAVE_NO_IRQ_HANDLER)
# error
#endif

#pragma GCC push_options
#pragma GCC optimize ("O3")
#pragma GCC optimize ("-fprefetch-loop-arrays")

#include <cstdint>
#include <cstring>
#include <algorithm>
#include <cassert>

#include "dmx.h"
#include "dmxconst.h"

#include "rdm.h"
#include "rdm_e120.h"

#include "gd32.h"
#include "gd32_dma.h"
#include "gd32_uart.h"
#include "gd32/dmx_config.h"
#include "dmx_internal.h"

#include "logic_analyzer.h"

#include "debug.h"

#ifdef ALIGNED
#undef ALIGNED
#endif
#define ALIGNED __attribute__ ((aligned (4)))

/**
 * Needed for older GD32F firmware
 */
#if !defined(USART_TRANSMIT_DMA_ENABLE)
# define USART_TRANSMIT_DMA_ENABLE	USART_DENT_ENABLE
#endif

/**
 * https://www.gd32-dmx.org/memory.html
 */
#if defined (GD32F20X) || defined (GD32F4XX) || defined (GD32H7XX)
# define SECTION_DMA_BUFFER					__attribute__ ((section (".dmx")))
#else
# define SECTION_DMA_BUFFER
#endif

extern struct HwTimersSeconds g_Seconds;

namespace dmx {
enum class TxRxState {
	IDLE, BREAK, MAB, DMXDATA, DMXINTER, RDMDATA, CHECKSUMH, CHECKSUML, RDMDISC
};

enum class PortState {
	IDLE, TX, RX
};

struct TxDmxPacket {
	uint8_t data[dmx::buffer::SIZE];	// multiple of uint16_t
	uint16_t nLength;
	bool bDataPending;
};

struct TxData {
	struct TxDmxPacket dmx;
	OutputStyle outputStyle ALIGNED;
	volatile TxRxState State;
};

struct RxDmxPackets {
	uint32_t nPerSecond;
	uint32_t nCount;
	uint32_t nCountPrevious;
	uint16_t nTimerCounterPrevious;
};

struct RxData {
	struct {
		uint8_t data[dmx::buffer::SIZE] ALIGNED;	// multiple of uint16_t
		uint32_t nSlotsInPacket;
	} Dmx;
	struct {
		uint8_t data[sizeof(struct TRdmMessage)] ALIGNED;
		uint32_t nIndex;
	} Rdm;
	TxRxState State;
} ALIGNED;

struct DirGpio {
	uint32_t nPort;
	uint32_t nPin;
};

}  // namespace dmx

using namespace dmx;

static constexpr DirGpio s_DirGpio[DMX_MAX_PORTS] = {
		{ config::DIR_PORT_0_GPIO_PORT, config::DIR_PORT_0_GPIO_PIN },
#if DMX_MAX_PORTS >= 2
		{ config::DIR_PORT_1_GPIO_PORT, config::DIR_PORT_1_GPIO_PIN },
#endif
#if DMX_MAX_PORTS >= 3
		{ config::DIR_PORT_2_GPIO_PORT, config::DIR_PORT_2_GPIO_PIN },
#endif
#if DMX_MAX_PORTS >= 4
		{ config::DIR_PORT_3_GPIO_PORT, config::DIR_PORT_3_GPIO_PIN },
#endif
#if DMX_MAX_PORTS >= 5
		{ config::DIR_PORT_4_GPIO_PORT, config::DIR_PORT_4_GPIO_PIN },
#endif
#if DMX_MAX_PORTS >= 6
		{ config::DIR_PORT_5_GPIO_PORT, config::DIR_PORT_5_GPIO_PIN },
#endif
#if DMX_MAX_PORTS >= 7
		{ config::DIR_PORT_6_GPIO_PORT, config::DIR_PORT_6_GPIO_PIN },
#endif
#if DMX_MAX_PORTS == 8
		{ config::DIR_PORT_7_GPIO_PORT, config::DIR_PORT_7_GPIO_PIN },
#endif
};

static volatile PortState sv_PortState[dmx::config::max::PORTS] ALIGNED;

#if !defined (CONFIG_DMX_DISABLE_STATISTICS)
static volatile dmx::TotalStatistics sv_TotalStatistics[dmx::config::max::PORTS] ALIGNED;
#endif

// DMX RX

[[maybe_unused]] static uint8_t sv_RxDmxPrevious[dmx::config::max::PORTS][dmx::buffer::SIZE] ALIGNED;
static volatile RxDmxPackets sv_nRxDmxPackets[dmx::config::max::PORTS] ALIGNED;

// RDM RX
volatile uint32_t gsv_RdmDataReceiveEnd;

// DMX RDM RX

static volatile RxData sv_RxBuffer[dmx::config::max::PORTS] ALIGNED;

// DMX TX

static TxData s_TxBuffer[dmx::config::max::PORTS] ALIGNED SECTION_DMA_BUFFER;

static uint32_t s_nDmxTransmitBreakTime;
static uint32_t s_nDmxTransmitMabTime;
static uint32_t s_nDmxTransmitInterTime;

static void irq_handler_dmx_rdm_input(const uint32_t uart, const uint32_t nPortIndex) {
	uint32_t nIndex;
	uint16_t nCounter;

	if (RESET != (USART_REG_VAL(uart, USART_FLAG_FERR) & BIT(USART_BIT_POS(USART_FLAG_FERR)))) {
		USART_REG_VAL(uart, USART_FLAG_FERR) &= ~BIT(USART_BIT_POS(USART_FLAG_FERR));
		static_cast<void>(GET_BITS(USART_RDATA(uart), 0U, 8U));
		if (sv_RxBuffer[nPortIndex].State == TxRxState::IDLE) {
			sv_RxBuffer[nPortIndex].State = TxRxState::BREAK;
		}
		return;
	}

	const auto data = static_cast<uint8_t>(GET_BITS(USART_RDATA(uart), 0U, 8U));

	switch (sv_RxBuffer[nPortIndex].State) {
	case TxRxState::IDLE:
		sv_RxBuffer[nPortIndex].State = TxRxState::RDMDISC;
		sv_RxBuffer[nPortIndex].Rdm.data[0] = data;
		sv_RxBuffer[nPortIndex].Rdm.nIndex = 1;
#if DMX_MAX_PORTS >= 5
		if (nPortIndex < 4) {
#endif
			sv_nRxDmxPackets[nPortIndex].nTimerCounterPrevious = static_cast<uint16_t>(TIMER_CNT(TIMER2));
#if DMX_MAX_PORTS >= 5
		} else {
			sv_nRxDmxPackets[nPortIndex].nTimerCounterPrevious = static_cast<uint16_t>(TIMER_CNT(TIMER3));
		}
#endif
		break;
	case TxRxState::BREAK:
		switch (data) {
		case START_CODE:
			sv_RxBuffer[nPortIndex].Dmx.data[0] = START_CODE;
			sv_RxBuffer[nPortIndex].Dmx.nSlotsInPacket = 1;
			sv_nRxDmxPackets[nPortIndex].nCount++;
			sv_RxBuffer[nPortIndex].State = TxRxState::DMXDATA;
#if DMX_MAX_PORTS >= 5
			if (nPortIndex < 4) {
#endif
				sv_nRxDmxPackets[nPortIndex].nTimerCounterPrevious = static_cast<uint16_t>(TIMER_CNT(TIMER2));
#if DMX_MAX_PORTS >= 5
			} else {
				sv_nRxDmxPackets[nPortIndex].nTimerCounterPrevious = static_cast<uint16_t>(TIMER_CNT(TIMER3));
			}
#endif
			break;
		case E120_SC_RDM:
			sv_RxBuffer[nPortIndex].Rdm.data[0] = E120_SC_RDM;
			sv_RxBuffer[nPortIndex].Rdm.nIndex = 1;
			sv_RxBuffer[nPortIndex].State = TxRxState::RDMDATA;
			break;
		default:
			sv_RxBuffer[nPortIndex].Dmx.nSlotsInPacket = 0;
			sv_RxBuffer[nPortIndex].Rdm.nIndex = 0;
			sv_RxBuffer[nPortIndex].State = TxRxState::IDLE;
			break;
		}
		break;
		case TxRxState::DMXDATA:
			nIndex = sv_RxBuffer[nPortIndex].Dmx.nSlotsInPacket;
			sv_RxBuffer[nPortIndex].Dmx.data[nIndex] = data;
			sv_RxBuffer[nPortIndex].Dmx.nSlotsInPacket++;

			if (sv_RxBuffer[nPortIndex].Dmx.nSlotsInPacket > dmx::max::CHANNELS) {
				sv_RxBuffer[nPortIndex].Dmx.nSlotsInPacket |= 0x8000;
				sv_RxBuffer[nPortIndex].State = TxRxState::IDLE;
				break;
			}

#if DMX_MAX_PORTS >= 5
			if (nPortIndex < 4) {
#endif
				nCounter = static_cast<uint16_t>(TIMER_CNT(TIMER2));
#if DMX_MAX_PORTS >= 5
			} else {
				nCounter = static_cast<uint16_t>(TIMER_CNT(TIMER3));
			}
#endif
			{
				const auto nDelta = nCounter - sv_nRxDmxPackets[nPortIndex].nTimerCounterPrevious;
				sv_nRxDmxPackets[nPortIndex].nTimerCounterPrevious = nCounter;
				const auto nPulse = static_cast<uint16_t>(nCounter + nDelta + 4);

				switch(nPortIndex){
				case 0:
					TIMER_CH0CV(TIMER2) = nPulse;
					break;
#if DMX_MAX_PORTS >= 2
				case 1:
					TIMER_CH1CV(TIMER2) = nPulse;
					break;
#endif
#if DMX_MAX_PORTS >= 3
				case 2:
					TIMER_CH2CV(TIMER2) = nPulse;
					break;
#endif
#if DMX_MAX_PORTS >= 4
				case 3:
					TIMER_CH3CV(TIMER2) = nPulse;
					break;
#endif
#if DMX_MAX_PORTS >= 5
				case 4:
					TIMER_CH0CV(TIMER3) = nPulse;
					break;
#endif
#if DMX_MAX_PORTS >= 6
				case 5:
					TIMER_CH1CV(TIMER3) = nPulse;
					break;
#endif
#if DMX_MAX_PORTS >= 7
				case 6:
					TIMER_CH2CV(TIMER3) = nPulse;
					break;
#endif
#if DMX_MAX_PORTS == 8
				case 7:
					TIMER_CH3CV(TIMER3) = nPulse;
					break;
#endif
				default:
					assert(0);
					__builtin_unreachable();
					break;
				}
			}
			break;
		case TxRxState::RDMDATA: {
			nIndex = sv_RxBuffer[nPortIndex].Rdm.nIndex;
			sv_RxBuffer[nPortIndex].Rdm.data[nIndex] = data;
			sv_RxBuffer[nPortIndex].Rdm.nIndex++;

			const auto *p = reinterpret_cast<volatile struct TRdmMessage*>(&sv_RxBuffer[nPortIndex].Rdm.data[0]);

			nIndex = sv_RxBuffer[nPortIndex].Rdm.nIndex;

			if ((nIndex >= 24) && (nIndex <= sizeof(struct TRdmMessage)) && (nIndex == p->message_length)) {
				sv_RxBuffer[nPortIndex].State = TxRxState::CHECKSUMH;
			} else if (nIndex > sizeof(struct TRdmMessage)) {
				sv_RxBuffer[nPortIndex].State = TxRxState::IDLE;
			}
		}
		break;
		case TxRxState::CHECKSUMH:
			nIndex = sv_RxBuffer[nPortIndex].Rdm.nIndex;
			sv_RxBuffer[nPortIndex].Rdm.data[nIndex] = data;
			sv_RxBuffer[nPortIndex].Rdm.nIndex++;
			sv_RxBuffer[nPortIndex].State = TxRxState::CHECKSUML;
			break;
		case TxRxState::CHECKSUML:
			nIndex = sv_RxBuffer[nPortIndex].Rdm.nIndex;
			sv_RxBuffer[nPortIndex].Rdm.data[nIndex] = data;
			sv_RxBuffer[nPortIndex].Rdm.nIndex |= 0x4000;
			sv_RxBuffer[nPortIndex].State = TxRxState::IDLE;
			gsv_RdmDataReceiveEnd = DWT->CYCCNT;
			break;
		case TxRxState::RDMDISC:
			nIndex = sv_RxBuffer[nPortIndex].Rdm.nIndex;

			if (nIndex < 24) {
				sv_RxBuffer[nPortIndex].Rdm.data[nIndex] = data;
				sv_RxBuffer[nPortIndex].Rdm.nIndex++;
			}

#if DMX_MAX_PORTS >= 5
			if (nPortIndex < 4) {
#endif
				nCounter = static_cast<uint16_t>(TIMER_CNT(TIMER2));
#if DMX_MAX_PORTS >= 5
			} else {
				nCounter = static_cast<uint16_t>(TIMER_CNT(TIMER3));
			}
#endif
			{
				const auto nDelta = nCounter - sv_nRxDmxPackets[nPortIndex].nTimerCounterPrevious;
				sv_nRxDmxPackets[nPortIndex].nTimerCounterPrevious = nCounter;
				const auto nPulse = static_cast<uint16_t>(nCounter + nDelta + 4);

				switch(nPortIndex){
				case 0:
					TIMER_CH0CV(TIMER2) = nPulse;
					break;
#if DMX_MAX_PORTS >= 2
				case 1:
					TIMER_CH1CV(TIMER2) = nPulse;
					break;
#endif
#if DMX_MAX_PORTS >= 3
				case 2:
					TIMER_CH2CV(TIMER2) = nPulse;
					break;
#endif
#if DMX_MAX_PORTS >= 4
				case 3:
					TIMER_CH3CV(TIMER2) = nPulse;
					break;
#endif
#if DMX_MAX_PORTS >= 5
				case 4:
					TIMER_CH0CV(TIMER3) = nPulse;
					break;
#endif
#if DMX_MAX_PORTS >= 6
				case 5:
					TIMER_CH1CV(TIMER3) = nPulse;
					break;
#endif
#if DMX_MAX_PORTS >= 7
				case 6:
					TIMER_CH2CV(TIMER3) = nPulse;
					break;
#endif
#if DMX_MAX_PORTS == 8
				case 7:
					TIMER_CH3CV(TIMER3) = nPulse;
					break;
#endif
				default:
					assert(0);
					__builtin_unreachable();
					break;
				}
			}
			break;
		default:
			sv_RxBuffer[nPortIndex].Dmx.nSlotsInPacket = 0;
			sv_RxBuffer[nPortIndex].Rdm.nIndex = 0;
			sv_RxBuffer[nPortIndex].State = TxRxState::IDLE;
			break;
	}
}

extern "C" {
#if !defined(CONFIG_DMX_TRANSMIT_ONLY)
#if defined (DMX_USE_USART0)
void USART0_IRQHandler(void) {
	irq_handler_dmx_rdm_input(USART0, config::USART0_PORT);
}
#endif
#if defined (DMX_USE_USART1)
void USART1_IRQHandler(void) {
	irq_handler_dmx_rdm_input(USART1, config::USART1_PORT);
}
#endif
#if defined (DMX_USE_USART2)
void USART2_IRQHandler(void) {
	irq_handler_dmx_rdm_input(USART2, config::USART2_PORT);
}
#endif
#if defined (DMX_USE_UART3)
void UART3_IRQHandler(void) {
	irq_handler_dmx_rdm_input(UART3, config::UART3_PORT);
}
#endif
#if defined (DMX_USE_UART4)
void UART4_IRQHandler(void) {
	irq_handler_dmx_rdm_input(UART4, config::UART4_PORT);
}
#endif
#if defined (DMX_USE_USART5)
void USART5_IRQHandler(void) {
	irq_handler_dmx_rdm_input(USART5, config::USART5_PORT);
}
#endif
#if defined (DMX_USE_UART6)
void UART6_IRQHandler(void) {
	irq_handler_dmx_rdm_input(UART6, config::UART6_PORT);
}
#endif
#if defined (DMX_USE_UART7)
void UART7_IRQHandler(void) {
	irq_handler_dmx_rdm_input(UART7, config::UART7_PORT);
}
#endif
#endif
}

static void timer1_config() {
	rcu_periph_clock_enable(RCU_TIMER1);
	timer_deinit(TIMER1);

	timer_parameter_struct timer_initpara;
	timer_struct_para_init(&timer_initpara);

	timer_initpara.prescaler = TIMER_PSC_1MHZ;
	timer_initpara.alignedmode = TIMER_COUNTER_EDGE;
	timer_initpara.counterdirection = TIMER_COUNTER_UP;
	timer_initpara.period = ~0;
	timer_initpara.clockdivision = TIMER_CKDIV_DIV1;
	timer_initpara.repetitioncounter = 0;
	timer_init(TIMER1, &timer_initpara);

	timer_flag_clear(TIMER1, ~0);
	timer_interrupt_flag_clear(TIMER1, ~0);

#if defined (DMX_USE_USART0)
	timer_channel_output_mode_config(TIMER1, TIMER_CH_0, TIMER_OC_MODE_ACTIVE);
	timer_channel_output_pulse_value_config(TIMER1, TIMER_CH_0, ~0);
	timer_interrupt_enable(TIMER1, TIMER_INT_CH0);
#endif

#if defined (DMX_USE_USART1)
	timer_channel_output_mode_config(TIMER1, TIMER_CH_1, TIMER_OC_MODE_ACTIVE);
	timer_channel_output_pulse_value_config(TIMER1, TIMER_CH_1, ~0);
	timer_interrupt_enable(TIMER1, TIMER_INT_CH1);
#endif

#if defined (DMX_USE_USART2)
	timer_channel_output_mode_config(TIMER1, TIMER_CH_2, TIMER_OC_MODE_ACTIVE);
	timer_channel_output_pulse_value_config(TIMER1, TIMER_CH_2, ~0);
	timer_interrupt_enable(TIMER1, TIMER_INT_CH2);
#endif

#if defined (DMX_USE_UART3)
	timer_channel_output_mode_config(TIMER1, TIMER_CH_3, TIMER_OC_MODE_ACTIVE);
	timer_channel_output_pulse_value_config(TIMER1, TIMER_CH_3, ~0);
	timer_interrupt_enable(TIMER1, TIMER_INT_CH3);
#endif

	NVIC_SetPriority(TIMER1_IRQn, 0);
	NVIC_EnableIRQ(TIMER1_IRQn);

	timer_enable(TIMER1);
}

static void timer4_config() {
	rcu_periph_clock_enable(RCU_TIMER4);
	timer_deinit(TIMER4);

	timer_parameter_struct timer_initpara;
	timer_struct_para_init(&timer_initpara);

	timer_initpara.prescaler = TIMER_PSC_1MHZ;
	timer_initpara.alignedmode = TIMER_COUNTER_EDGE;
	timer_initpara.counterdirection = TIMER_COUNTER_UP;
	timer_initpara.period = ~0;
	timer_initpara.clockdivision = TIMER_CKDIV_DIV1;
	timer_initpara.repetitioncounter = 0;
	timer_init(TIMER4, &timer_initpara);

	timer_flag_clear(TIMER4, ~0);
	timer_interrupt_flag_clear(TIMER4, ~0);

#if defined (DMX_USE_UART4)
	timer_channel_output_mode_config(TIMER4, TIMER_CH_0, TIMER_OC_MODE_ACTIVE);
	timer_channel_output_pulse_value_config(TIMER4, TIMER_CH_0, ~0);
	timer_interrupt_enable(TIMER4, TIMER_INT_CH0);
#endif

#if defined (DMX_USE_USART5)
	timer_channel_output_mode_config(TIMER4, TIMER_CH_1, TIMER_OC_MODE_ACTIVE);
	timer_channel_output_pulse_value_config(TIMER4, TIMER_CH_1, ~0);
	timer_interrupt_enable(TIMER4, TIMER_INT_CH1);
#endif

#if defined (DMX_USE_UART6)
	timer_channel_output_mode_config(TIMER4, TIMER_CH_2, TIMER_OC_MODE_ACTIVE);
	timer_channel_output_pulse_value_config(TIMER4, TIMER_CH_2, ~0);
	timer_interrupt_enable(TIMER4, TIMER_INT_CH2);
#endif

#if defined (DMX_USE_UART7)
	timer_channel_output_mode_config(TIMER4, TIMER_CH_3, TIMER_OC_MODE_ACTIVE);
	timer_channel_output_pulse_value_config(TIMER4, TIMER_CH_3, ~0);
	timer_interrupt_enable(TIMER4, TIMER_INT_CH3);
#endif

	NVIC_SetPriority(TIMER4_IRQn, 0);
	NVIC_EnableIRQ(TIMER4_IRQn);

	timer_enable(TIMER4);
}

#if !defined(CONFIG_DMX_TRANSMIT_ONLY)
static void timer2_config() {
	rcu_periph_clock_enable(RCU_TIMER2);
	timer_deinit(TIMER2);

	timer_parameter_struct timer_initpara;
	timer_struct_para_init(&timer_initpara);

	timer_initpara.prescaler = TIMER_PSC_1MHZ;
	timer_initpara.alignedmode = TIMER_COUNTER_EDGE;
	timer_initpara.counterdirection = TIMER_COUNTER_UP;
	timer_initpara.period = static_cast<uint32_t>(~0);
	timer_initpara.clockdivision = TIMER_CKDIV_DIV1;
	timer_initpara.repetitioncounter = 0;
	timer_init(TIMER2, &timer_initpara);

	timer_flag_clear(TIMER2, ~0);
	timer_interrupt_flag_clear(TIMER2, ~0);

	timer_channel_output_mode_config(TIMER2, TIMER_CH_0, TIMER_OC_MODE_ACTIVE);
	timer_channel_output_mode_config(TIMER2, TIMER_CH_1, TIMER_OC_MODE_ACTIVE);
	timer_channel_output_mode_config(TIMER2, TIMER_CH_2, TIMER_OC_MODE_ACTIVE);
	timer_channel_output_mode_config(TIMER2, TIMER_CH_3, TIMER_OC_MODE_ACTIVE);

	NVIC_SetPriority(TIMER2_IRQn, 2);
	NVIC_EnableIRQ(TIMER2_IRQn);

	timer_enable(TIMER2);
}

static void timer3_config() {
#if DMX_MAX_PORTS >= 5
	rcu_periph_clock_enable(RCU_TIMER3);
	timer_deinit(TIMER3);

	timer_parameter_struct timer_initpara;
	timer_struct_para_init(&timer_initpara);

	timer_initpara.prescaler = TIMER_PSC_1MHZ;
	timer_initpara.alignedmode = TIMER_COUNTER_EDGE;
	timer_initpara.counterdirection = TIMER_COUNTER_UP;
	timer_initpara.period = static_cast<uint32_t>(~0);
	timer_initpara.clockdivision = TIMER_CKDIV_DIV1;
	timer_initpara.repetitioncounter = 0;
	timer_init(TIMER3, &timer_initpara);

	timer_flag_clear(TIMER3, ~0);
	timer_interrupt_flag_clear(TIMER3, ~0);

	timer_channel_output_mode_config(TIMER3, TIMER_CH_0, TIMER_OC_MODE_ACTIVE);
	timer_channel_output_mode_config(TIMER3, TIMER_CH_1, TIMER_OC_MODE_ACTIVE);
	timer_channel_output_mode_config(TIMER3, TIMER_CH_2, TIMER_OC_MODE_ACTIVE);
	timer_channel_output_mode_config(TIMER3, TIMER_CH_3, TIMER_OC_MODE_ACTIVE);

	NVIC_SetPriority(TIMER3_IRQn, 2);
	NVIC_EnableIRQ(TIMER3_IRQn);

	timer_enable(TIMER3);
#endif
}
#endif

static void usart_dma_config(void) {
	DMA_PARAMETER_STRUCT dma_init_struct;
	rcu_periph_clock_enable(RCU_DMA0);
	rcu_periph_clock_enable(RCU_DMA1);
#if defined (GD32H7XX)
	rcu_periph_clock_enable(RCU_DMAMUX);
#endif
	/*
	 * USART 0
	 */
#if defined (DMX_USE_USART0)
	dma_deinit(USART0_DMAx, USART0_TX_DMA_CHx);
# if defined (GD32H7XX)
	dma_init_struct.request = DMA_REQUEST_USART0_TX;
# endif
	dma_init_struct.direction = DMA_MEMORY_TO_PERIPHERAL;
	dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
# if defined (GD32F4XX) || defined (GD32H7XX)
# else
	dma_init_struct.memory_width = DMA_MEMORY_WIDTH_8BIT;
# endif
# if defined (GD32H7XX)
	dma_init_struct.periph_addr = (uint32_t) &USART_TDATA(USART0>();
# else
	dma_init_struct.periph_addr = USART0 + 0x04U;
# endif
	dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
# if defined (GD32F4XX) || defined (GD32H7XX)
	dma_init_struct.periph_memory_width = DMA_PERIPHERAL_WIDTH_8BIT;
# else
	dma_init_struct.periph_width = DMA_PERIPHERAL_WIDTH_8BIT;
# endif
	dma_init_struct.priority = DMA_PRIORITY_HIGH;
	dma_init(USART0_DMAx, USART0_TX_DMA_CHx, &dma_init_struct);
	/* configure DMA mode */
	dma_circulation_disable(USART0_DMAx, USART0_TX_DMA_CHx);
	dma_memory_to_memory_disable(USART0_DMAx, USART0_TX_DMA_CHx);
# if defined (GD32F4XX)
	dma_channel_subperipheral_select(USART0_DMAx, USART0_TX_DMA_CHx, USART0_TX_DMA_SUBPERIx);
# endif
	gd32_dma_interrupt_disable<USART0_DMAx, USART0_TX_DMA_CHx, DMA_INTERRUPT_DISABLE>();
# if !defined (GD32F4XX)
	NVIC_SetPriority(DMA0_Channel3_IRQn, 1);
	NVIC_EnableIRQ(DMA0_Channel3_IRQn);
# else
	NVIC_SetPriority(DMA1_Channel7_IRQn, 1);
	NVIC_EnableIRQ(DMA1_Channel7_IRQn);
# endif
#endif
	/*
	 * USART 1
	 */
#if defined (DMX_USE_USART1)
	dma_deinit(USART1_DMAx, USART1_TX_DMA_CHx);
# if defined (GD32H7XX)
	dma_init_struct.request = DMA_REQUEST_USART1_TX;
# endif
	dma_init_struct.direction = DMA_MEMORY_TO_PERIPHERAL;
	dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
# if defined (GD32F4XX) || defined (GD32H7XX)
# else
	dma_init_struct.memory_width = DMA_MEMORY_WIDTH_8BIT;
# endif
# if defined (GD32H7XX)
	dma_init_struct.periph_addr = (uint32_t) &USART_TDATA(USART1>();
# else
	dma_init_struct.periph_addr = USART1 + 0x04U;
# endif
	dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
# if defined (GD32F4XX) || defined (GD32H7XX)
	dma_init_struct.periph_memory_width = DMA_PERIPHERAL_WIDTH_8BIT;
# else
	dma_init_struct.periph_width = DMA_PERIPHERAL_WIDTH_8BIT;
# endif
	dma_init_struct.priority = DMA_PRIORITY_HIGH;
	dma_init(USART1_DMAx, USART1_TX_DMA_CHx, &dma_init_struct);
	/* configure DMA mode */
	dma_circulation_disable(USART1_DMAx, USART1_TX_DMA_CHx);
	dma_memory_to_memory_disable(USART1_DMAx, USART1_TX_DMA_CHx);
# if defined (GD32F4XX)
	dma_channel_subperipheral_select(USART1_DMAx, USART1_TX_DMA_CHx, USART1_TX_DMA_SUBPERIx);
# endif
	gd32_dma_interrupt_disable<USART1_DMAx, USART1_TX_DMA_CHx, DMA_INTERRUPT_DISABLE>();
	NVIC_SetPriority(DMA0_Channel6_IRQn, 1);
	NVIC_EnableIRQ(DMA0_Channel6_IRQn);
#endif
	/*
	 * USART 2
	 */
#if defined (DMX_USE_USART2)
	dma_deinit(USART2_DMAx, USART2_TX_DMA_CHx);
# if defined (GD32H7XX)
	dma_init_struct.request = DMA_REQUEST_USART2_TX;
# endif
	dma_init_struct.direction = DMA_MEMORY_TO_PERIPHERAL;
	dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
# if defined (GD32F4XX) || defined (GD32H7XX)
# else
	dma_init_struct.memory_width = DMA_MEMORY_WIDTH_8BIT;
# endif
# if defined (GD32H7XX)
	dma_init_struct.periph_addr = (uint32_t) &USART_TDATA(USART2>();
# else
	dma_init_struct.periph_addr = USART2 + 0x04U;
# endif
	dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
# if defined (GD32F4XX) || defined (GD32H7XX)
	dma_init_struct.periph_memory_width = DMA_PERIPHERAL_WIDTH_8BIT;
# else
	dma_init_struct.periph_width = DMA_PERIPHERAL_WIDTH_8BIT;
# endif
	dma_init_struct.priority = DMA_PRIORITY_HIGH;
	dma_init(USART2_DMAx, USART2_TX_DMA_CHx, &dma_init_struct);
	/* configure DMA mode */
	dma_circulation_disable(USART2_DMAx, USART2_TX_DMA_CHx);
	dma_memory_to_memory_disable(USART2_DMAx, USART2_TX_DMA_CHx);
# if defined (GD32F4XX)
	dma_channel_subperipheral_select(USART2_DMAx, USART2_TX_DMA_CHx, USART2_TX_DMA_SUBPERIx);
# endif
	gd32_dma_interrupt_disable<USART2_DMAx, USART2_TX_DMA_CHx, DMA_INTERRUPT_DISABLE>();
#if defined (GD32F4XX) || defined (GD32H7XX)
	NVIC_SetPriority(DMA0_Channel3_IRQn, 1);
	NVIC_EnableIRQ(DMA0_Channel3_IRQn);
# else
	NVIC_SetPriority(DMA0_Channel1_IRQn, 1);
	NVIC_EnableIRQ(DMA0_Channel1_IRQn);
# endif
#endif
	/*
	 * UART 3
	 */
#if defined (DMX_USE_UART3)
	dma_deinit(UART3_DMAx, UART3_TX_DMA_CHx);
# if defined (GD32H7XX)
	dma_init_struct.request = DMA_REQUEST_UART3_TX;
# endif
	dma_init_struct.direction = DMA_MEMORY_TO_PERIPHERAL;
	dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
# if defined (GD32F4XX) || defined (GD32H7XX)
# else
	dma_init_struct.memory_width = DMA_MEMORY_WIDTH_8BIT;
# endif
# if defined (GD32H7XX)
	dma_init_struct.periph_addr = (uint32_t) &USART_TDATA(UART3>();
# else
	dma_init_struct.periph_addr = UART3 + 0x04U;
# endif
	dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
# if defined (GD32F4XX) || defined (GD32H7XX)
	dma_init_struct.periph_memory_width = DMA_PERIPHERAL_WIDTH_8BIT;
# else
	dma_init_struct.periph_width = DMA_PERIPHERAL_WIDTH_8BIT;
# endif
	dma_init_struct.priority = DMA_PRIORITY_HIGH;
	dma_init(UART3_DMAx, UART3_TX_DMA_CHx, &dma_init_struct);
	/* configure DMA mode */
	dma_circulation_disable(UART3_DMAx, UART3_TX_DMA_CHx);
	dma_memory_to_memory_disable(UART3_DMAx, UART3_TX_DMA_CHx);
# if defined (GD32F4XX)
	dma_channel_subperipheral_select(UART3_DMAx, UART3_TX_DMA_CHx, UART3_TX_DMA_SUBPERIx);
# endif
	gd32_dma_interrupt_disable<UART3_DMAx, UART3_TX_DMA_CHx, DMA_INTERRUPT_DISABLE>();
# if !defined (GD32F4XX)
	NVIC_SetPriority(DMA1_Channel4_IRQn, 1);
	NVIC_EnableIRQ(DMA1_Channel4_IRQn);
# else
	NVIC_SetPriority(DMA0_Channel4_IRQn, 1);
	NVIC_EnableIRQ(DMA0_Channel4_IRQn);
# endif
#endif
	/*
	 * UART 4
	 */
#if defined (DMX_USE_UART4)
	dma_deinit(UART4_DMAx, UART4_TX_DMA_CHx);
# if defined (GD32H7XX)
	dma_init_struct.request = DMA_REQUEST_UART4_TX;
# endif
	dma_init_struct.direction = DMA_MEMORY_TO_PERIPHERAL;
	dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
# if defined (GD32F4XX) || defined (GD32H7XX)
# else
	dma_init_struct.memory_width = DMA_MEMORY_WIDTH_8BIT;
# endif
# if defined (GD32H7XX)
	dma_init_struct.periph_addr = (uint32_t) &USART_TDATA(UART4>();
# else
	dma_init_struct.periph_addr = UART4 + 0x04U;
# endif
	dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
#if defined (GD32F4XX) || defined (GD32H7XX)
	dma_init_struct.periph_memory_width = DMA_PERIPHERAL_WIDTH_8BIT;
#else
	dma_init_struct.periph_width = DMA_PERIPHERAL_WIDTH_8BIT;
#endif
	dma_init_struct.priority = DMA_PRIORITY_HIGH;
	dma_init(UART4_DMAx, UART4_TX_DMA_CHx, &dma_init_struct);
	/* configure DMA mode */
	dma_circulation_disable(UART4_DMAx, UART4_TX_DMA_CHx);
	dma_memory_to_memory_disable(UART4_DMAx, UART4_TX_DMA_CHx);
# if defined (GD32F4XX)
	dma_channel_subperipheral_select(UART4_DMAx, UART4_TX_DMA_CHx, UART4_TX_DMA_SUBPERIx);
# endif
	gd32_dma_interrupt_disable<UART4_DMAx, UART4_TX_DMA_CHx, DMA_INTERRUPT_DISABLE>();
# if !defined (GD32F4XX)
	NVIC_SetPriority(DMA1_Channel3_IRQn, 1);
	NVIC_EnableIRQ(DMA1_Channel3_IRQn);
# else
	NVIC_SetPriority(DMA0_Channel7_IRQn, 1);
	NVIC_EnableIRQ(DMA0_Channel7_IRQn);
# endif
#endif
	/*
	 * USART 5
	 */
#if defined (DMX_USE_USART5)
	dma_deinit(USART5_DMAx, USART5_TX_DMA_CHx);
# if defined (GD32H7XX)
	dma_init_struct.request = DMA_REQUEST_USART5_TX;
# endif
	dma_init_struct.direction = DMA_MEMORY_TO_PERIPHERAL;
	dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
# if defined (GD32F20X)
	dma_init_struct.memory_width = DMA_MEMORY_WIDTH_8BIT;
# endif
# if defined (GD32H7XX)
	dma_init_struct.periph_addr = (uint32_t) &USART_TDATA(USART5>();
# else
	dma_init_struct.periph_addr = USART5 + 0x04U;
# endif
	dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
#if defined (GD32F20X)
	dma_init_struct.periph_width = DMA_PERIPHERAL_WIDTH_8BIT;
# else
	dma_init_struct.periph_memory_width = DMA_PERIPHERAL_WIDTH_8BIT;
#endif
	dma_init_struct.priority = DMA_PRIORITY_HIGH;
	dma_init(USART5_DMAx, USART5_TX_DMA_CHx, &dma_init_struct);
	/* configure DMA mode */
	dma_circulation_disable(USART5_DMAx, USART5_TX_DMA_CHx);
	dma_memory_to_memory_disable(USART5_DMAx, USART5_TX_DMA_CHx);
# if defined (GD32F4XX)
	dma_channel_subperipheral_select(USART5_DMAx, USART5_TX_DMA_CHx, USART5_TX_DMA_SUBPERIx);
# endif
	gd32_dma_interrupt_disable<USART5_DMAx, USART5_TX_DMA_CHx, DMA_INTERRUPT_DISABLE>();
	NVIC_SetPriority(DMA1_Channel6_IRQn, 1);
	NVIC_EnableIRQ(DMA1_Channel6_IRQn);
#endif
	/*
	 * UART 6
	 */
#if defined (DMX_USE_UART6)
	dma_deinit(UART6_DMAx, UART6_TX_DMA_CHx);
# if defined (GD32H7XX)
	dma_init_struct.request = DMA_REQUEST_UART6_TX;
# endif
	dma_init_struct.direction = DMA_MEMORY_TO_PERIPHERAL;
	dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
# if defined (GD32F20X)
	dma_init_struct.memory_width = DMA_MEMORY_WIDTH_8BIT;
# endif
# if defined (GD32H7XX)
	dma_init_struct.periph_addr = (uint32_t) &USART_TDATA(UART6>();
# else
	dma_init_struct.periph_addr = UART6 + 0x04U;
# endif
	dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
# if defined (GD32F20X)
	dma_init_struct.periph_width = DMA_PERIPHERAL_WIDTH_8BIT;
# else
	dma_init_struct.periph_memory_width = DMA_PERIPHERAL_WIDTH_8BIT;
# endif
	dma_init_struct.priority = DMA_PRIORITY_HIGH;
	dma_init(UART6_DMAx, UART6_TX_DMA_CHx, &dma_init_struct);
	/* configure DMA mode */
	dma_circulation_disable(UART6_DMAx, UART6_TX_DMA_CHx);
	dma_memory_to_memory_disable(UART6_DMAx, UART6_TX_DMA_CHx);
# if defined (GD32F4XX)
	dma_channel_subperipheral_select(UART6_DMAx, UART6_TX_DMA_CHx, UART6_TX_DMA_SUBPERIx);
# endif
	gd32_dma_interrupt_disable<UART6_DMAx, UART4_TX_DMA_CHx, DMA_INTERRUPT_DISABLE>();
# if defined (GD32F20X)
	NVIC_SetPriority(DMA1_Channel4_IRQn, 1);
	NVIC_EnableIRQ(DMA1_Channel4_IRQn);
# else
	NVIC_SetPriority(DMA0_Channel1_IRQn, 1);
	NVIC_EnableIRQ(DMA0_Channel1_IRQn);
# endif
#endif
	/*
	 * UART 7
	 */
#if defined (DMX_USE_UART7)
	dma_deinit(UART7_DMAx, UART7_TX_DMA_CHx);
# if defined (GD32H7XX)
	dma_init_struct.request = DMA_REQUEST_UART7_TX;
# endif
	dma_init_struct.direction = DMA_MEMORY_TO_PERIPHERAL;
	dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
# if defined (GD32F20X)
	dma_init_struct.memory_width = DMA_MEMORY_WIDTH_8BIT;
# endif
# if defined (GD32H7XX)
	dma_init_struct.periph_addr = (uint32_t) &USART_TDATA(UART7>();
# else
	dma_init_struct.periph_addr = UART7 + 0x04U;
# endif
	dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
# if defined (GD32F20X)
	dma_init_struct.periph_width = DMA_PERIPHERAL_WIDTH_8BIT;
# else
	dma_init_struct.periph_memory_width = DMA_PERIPHERAL_WIDTH_8BIT;
# endif
	dma_init_struct.priority = DMA_PRIORITY_HIGH;
	dma_init(UART7_DMAx, UART7_TX_DMA_CHx, &dma_init_struct);
	/* configure DMA mode */
	dma_circulation_disable(UART7_DMAx, UART7_TX_DMA_CHx);
	dma_memory_to_memory_disable(UART7_DMAx, UART7_TX_DMA_CHx);
# if defined (GD32F4XX)
	dma_channel_subperipheral_select(UART7_DMAx, UART7_TX_DMA_CHx, UART7_TX_DMA_SUBPERIx);
# endif
#if defined (GD32F20X)
	NVIC_SetPriority(DMA1_Channel3_IRQn, 1);
	NVIC_EnableIRQ(DMA1_Channel3_IRQn);
# else
	NVIC_SetPriority(DMA0_Channel0_IRQn, 1);
	NVIC_EnableIRQ(DMA0_Channel0_IRQn);
# endif
#endif
}

extern "C" {
void TIMER1_IRQHandler() {
	/*
	 * USART 0
	 */
#if defined (DMX_USE_USART0)
	if ((TIMER_INTF(TIMER1) & TIMER_INT_FLAG_CH0) == TIMER_INT_FLAG_CH0) {
		switch (s_TxBuffer[dmx::config::USART0_PORT].State) {
		case TxRxState::DMXINTER:
			gd32_gpio_mode_output<USART0_GPIOx, USART0_TX_GPIO_PINx>();
			GPIO_BC(USART0_GPIOx) = USART0_TX_GPIO_PINx;
			s_TxBuffer[dmx::config::USART0_PORT].State = TxRxState::BREAK;
			TIMER_CH0CV(TIMER1) =  TIMER_CNT(TIMER1) + s_nDmxTransmitBreakTime;
			break;
		case TxRxState::BREAK:
			gd32_gpio_mode_af<USART0_GPIOx, USART0_TX_GPIO_PINx, USART0>();
			s_TxBuffer[dmx::config::USART0_PORT].State = TxRxState::MAB;
			TIMER_CH0CV(TIMER1) =  TIMER_CNT(TIMER1) + s_nDmxTransmitMabTime;
			break;
		case TxRxState::MAB: {
			uint32_t dmaCHCTL = DMA_CHCTL(USART0_DMAx, USART0_TX_DMA_CHx);
			dmaCHCTL &= ~DMA_CHXCTL_CHEN;
			DMA_CHCTL(USART0_DMAx, USART0_TX_DMA_CHx) = dmaCHCTL;
			gd32_dma_interrupt_flag_clear<USART0_DMAx, USART0_TX_DMA_CHx, DMA_INTF_FTFIF>();
			const auto *p = &s_TxBuffer[dmx::config::USART0_PORT].dmx;
			DMA_CHMADDR(USART0_DMAx, USART0_TX_DMA_CHx) = (uint32_t) p->data;
			DMA_CHCNT(USART0_DMAx, USART0_TX_DMA_CHx) = (p->nLength & DMA_CHXCNT_CNT);
			dmaCHCTL |= DMA_CHXCTL_CHEN;
			dmaCHCTL |= DMA_INTERRUPT_ENABLE;
			DMA_CHCTL(USART0_DMAx, USART0_TX_DMA_CHx) = dmaCHCTL;
			USART_CTL2(USART0) |= USART_TRANSMIT_DMA_ENABLE;
		}
			break;
		default:
			break;
		}

		TIMER_INTF(TIMER1) = static_cast<uint32_t>(~TIMER_INT_FLAG_CH0);
	}
#endif
	/*
	 * USART 1
	 */
#if defined (DMX_USE_USART1)
	if ((TIMER_INTF(TIMER1) & TIMER_INT_FLAG_CH1) == TIMER_INT_FLAG_CH1) {
		switch (s_TxBuffer[dmx::config::USART1_PORT].State) {
		case TxRxState::DMXINTER:
			gd32_gpio_mode_output<USART1_GPIOx, USART1_TX_GPIO_PINx>();
			GPIO_BC(USART1_GPIOx) = USART1_TX_GPIO_PINx;
			s_TxBuffer[dmx::config::USART1_PORT].State = TxRxState::BREAK;
			TIMER_CH1CV(TIMER1) = TIMER_CNT(TIMER1) + s_nDmxTransmitBreakTime;
			break;
		case TxRxState::BREAK:
			gd32_gpio_mode_af<USART1_GPIOx, USART1_TX_GPIO_PINx, USART1>();
			s_TxBuffer[dmx::config::USART1_PORT].State = TxRxState::MAB;
			TIMER_CH1CV(TIMER1) =  TIMER_CNT(TIMER1) + s_nDmxTransmitMabTime;
			break;
		case TxRxState::MAB: {
			uint32_t dmaCHCTL = DMA_CHCTL(USART1_DMAx, USART1_TX_DMA_CHx);
			dmaCHCTL &= ~DMA_CHXCTL_CHEN;
			DMA_CHCTL(USART1_DMAx, USART1_TX_DMA_CHx) = dmaCHCTL;
			gd32_dma_interrupt_flag_clear<USART1_DMAx, USART1_TX_DMA_CHx, DMA_INTF_FTFIF);
			const auto *p = &s_TxBuffer[dmx::config::USART1_PORT].dmx;
			DMA_CHMADDR(USART1_DMAx, USART1_TX_DMA_CHx) = (uint32_t) p->data;
			DMA_CHCNT(USART1_DMAx, USART1_TX_DMA_CHx) = (p->nLength & DMA_CHXCNT_CNT);
			dmaCHCTL |= DMA_CHXCTL_CHEN;
			dmaCHCTL |= DMA_INTERRUPT_ENABLE;
			DMA_CHCTL(USART1_DMAx, USART1_TX_DMA_CHx) = dmaCHCTL;
			USART_CTL2(USART1) |= USART_TRANSMIT_DMA_ENABLE;
		}
			break;
		default:
			break;
		}

		TIMER_INTF(TIMER1) = static_cast<uint32_t>(~TIMER_INT_FLAG_CH1);
	}
#endif
	/*
	 * USART 2
	 */
#if defined (DMX_USE_USART2)
	if ((TIMER_INTF(TIMER1) & TIMER_INT_FLAG_CH2) == TIMER_INT_FLAG_CH2) {
		switch (s_TxBuffer[dmx::config::USART2_PORT].State) {
		case TxRxState::DMXINTER:
			gd32_gpio_mode_output<USART2_GPIOx, USART2_TX_GPIO_PINx>();
			GPIO_BC(USART2_GPIOx) = USART2_TX_GPIO_PINx;
			s_TxBuffer[dmx::config::USART2_PORT].State = TxRxState::BREAK;
			TIMER_CH2CV(TIMER1) = TIMER_CNT(TIMER1) + s_nDmxTransmitBreakTime;
			break;
		case TxRxState::BREAK:
			gd32_gpio_mode_af<USART2_GPIOx, USART2_TX_GPIO_PINx, USART2>();
			s_TxBuffer[dmx::config::USART2_PORT].State = TxRxState::MAB;
			TIMER_CH2CV(TIMER1) = TIMER_CNT(TIMER1) + s_nDmxTransmitMabTime;
			break;
		case TxRxState::MAB: {
			uint32_t dmaCHCTL = DMA_CHCTL(USART2_DMAx, USART2_TX_DMA_CHx);
			dmaCHCTL &= ~DMA_CHXCTL_CHEN;
			DMA_CHCTL(USART2_DMAx, USART2_TX_DMA_CHx) = dmaCHCTL;
			gd32_dma_interrupt_flag_clear<USART2_DMAx, USART2_TX_DMA_CHx, DMA_INTF_FTFIF>();
			const auto *p = &s_TxBuffer[dmx::config::USART2_PORT].dmx;
			DMA_CHMADDR(USART2_DMAx, USART2_TX_DMA_CHx) = (uint32_t) p->data;
			DMA_CHCNT(USART2_DMAx, USART2_TX_DMA_CHx) = (p->nLength & DMA_CHXCNT_CNT);
			dmaCHCTL |= DMA_CHXCTL_CHEN;
			dmaCHCTL |= DMA_INTERRUPT_ENABLE;
			DMA_CHCTL(USART2_DMAx, USART2_TX_DMA_CHx) = dmaCHCTL;
			USART_CTL2(USART2) |= USART_TRANSMIT_DMA_ENABLE;
		}
			break;
		default:
			break;
		}

		TIMER_INTF(TIMER1) = static_cast<uint32_t>(~TIMER_INT_FLAG_CH2);
	}
#endif
	/*
	 * UART 3
	 */
#if defined (DMX_USE_UART3)
	if ((TIMER_INTF(TIMER1) & TIMER_INT_FLAG_CH3) == TIMER_INT_FLAG_CH3) {
		switch (s_TxBuffer[dmx::config::UART3_PORT].State) {
		case TxRxState::DMXINTER:
			gd32_gpio_mode_output<UART3_GPIOx, UART3_TX_GPIO_PINx>();
			GPIO_BC(UART3_GPIOx) = UART3_TX_GPIO_PINx;
			s_TxBuffer[dmx::config::UART3_PORT].State = TxRxState::BREAK;
			TIMER_CH3CV(TIMER1) = TIMER_CNT(TIMER1) + s_nDmxTransmitBreakTime;
			break;
		case TxRxState::BREAK:
			gd32_gpio_mode_af<UART3_GPIOx, UART3_TX_GPIO_PINx, UART3>();
			s_TxBuffer[dmx::config::UART3_PORT].State = TxRxState::MAB;
			TIMER_CH3CV(TIMER1) = TIMER_CNT(TIMER1) + s_nDmxTransmitMabTime;
			break;
		case TxRxState::MAB: {
			uint32_t dmaCHCTL = DMA_CHCTL(UART3_DMAx, UART3_TX_DMA_CHx);
			dmaCHCTL &= ~DMA_CHXCTL_CHEN;
			DMA_CHCTL(UART3_DMAx, UART3_TX_DMA_CHx) = dmaCHCTL;
			gd32_dma_interrupt_flag_clear<UART3_DMAx, UART3_TX_DMA_CHx, DMA_INTF_FTFIF>();
			const auto *p = &s_TxBuffer[dmx::config::UART3_PORT].dmx;
			DMA_CHMADDR(UART3_DMAx, UART3_TX_DMA_CHx) = (uint32_t) p->data;
			DMA_CHCNT(UART3_DMAx, UART3_TX_DMA_CHx) = (p->nLength & DMA_CHXCNT_CNT);
			dmaCHCTL |= DMA_CHXCTL_CHEN;
			dmaCHCTL |= DMA_INTERRUPT_ENABLE;
			DMA_CHCTL(UART3_DMAx, UART3_TX_DMA_CHx) = dmaCHCTL;
			USART_CTL2(UART3) |= USART_TRANSMIT_DMA_ENABLE;
		}
			break;
		default:
			break;
		}

		TIMER_INTF(TIMER1) = static_cast<uint32_t>(~TIMER_INT_FLAG_CH3);
	}
#endif
	// Clear all remaining interrupt flags (safety measure)
	TIMER_INTF(TIMER1) = static_cast<uint32_t>(~0);
}

void TIMER4_IRQHandler() {
	/*
	 * UART 4
	 */
#if defined (DMX_USE_UART4)
	if ((TIMER_INTF(TIMER4) & TIMER_INT_FLAG_CH0) == TIMER_INT_FLAG_CH0) {
		switch (s_TxBuffer[dmx::config::UART4_PORT].State) {
		case TxRxState::DMXINTER:
			gd32_gpio_mode_output<UART4_TX_GPIOx, UART4_TX_GPIO_PINx>();
			GPIO_BC(UART4_TX_GPIOx) = UART4_TX_GPIO_PINx;
			s_TxBuffer[dmx::config::UART4_PORT].State = TxRxState::BREAK;
			TIMER_CH0CV(TIMER4) = TIMER_CNT(TIMER4) + s_nDmxTransmitBreakTime;
			break;
		case TxRxState::BREAK:
			gd32_gpio_mode_af<UART4_TX_GPIOx, UART4_TX_GPIO_PINx, UART4>();
			s_TxBuffer[dmx::config::UART4_PORT].State = TxRxState::MAB;
			TIMER_CH0CV(TIMER4) = TIMER_CNT(TIMER4) + s_nDmxTransmitMabTime;
			break;
		case TxRxState::MAB: {
			uint32_t dmaCHCTL = DMA_CHCTL(UART4_DMAx, UART4_TX_DMA_CHx);
			dmaCHCTL &= ~DMA_CHXCTL_CHEN;
			DMA_CHCTL(UART4_DMAx, UART4_TX_DMA_CHx) = dmaCHCTL;
			gd32_dma_interrupt_flag_clear<UART4_DMAx, UART4_TX_DMA_CHx, DMA_INTF_FTFIF>();
			const auto *p = &s_TxBuffer[dmx::config::UART4_PORT].dmx;
			DMA_CHMADDR(UART4_DMAx, UART4_TX_DMA_CHx) = (uint32_t) p->data;
			DMA_CHCNT(UART4_DMAx, UART4_TX_DMA_CHx) = (p->nLength & DMA_CHXCNT_CNT);
			dmaCHCTL |= DMA_CHXCTL_CHEN;
			dmaCHCTL |= DMA_INTERRUPT_ENABLE;
			DMA_CHCTL(UART4_DMAx, UART4_TX_DMA_CHx) = dmaCHCTL;
			USART_CTL2(UART4) |= USART_TRANSMIT_DMA_ENABLE;
		}
			break;
		default:
			break;
		}

		TIMER_INTF(TIMER4) = static_cast<uint32_t>(~TIMER_INT_FLAG_CH0);
	}
#endif
	/*
	 * USART 5
	 */
#if defined (DMX_USE_USART5)
	if ((TIMER_INTF(TIMER4) & TIMER_INT_FLAG_CH1) == TIMER_INT_FLAG_CH1) {
		switch (s_TxBuffer[dmx::config::USART5_PORT].State) {
		case TxRxState::DMXINTER:
			gd32_gpio_mode_output<USART5_GPIOx, USART5_TX_GPIO_PINx>();
			GPIO_BC(USART5_GPIOx) = USART5_TX_GPIO_PINx;
			s_TxBuffer[dmx::config::USART5_PORT].State = TxRxState::BREAK;
			TIMER_CH1CV(TIMER4) = TIMER_CNT(TIMER4) + s_nDmxTransmitBreakTime;
			break;
		case TxRxState::BREAK:
			gd32_gpio_mode_af<USART5_GPIOx, USART5_TX_GPIO_PINx, USART5>();
			s_TxBuffer[dmx::config::USART5_PORT].State = TxRxState::MAB;
			TIMER_CH1CV(TIMER4) = TIMER_CNT(TIMER4) + s_nDmxTransmitMabTime;
			break;
		case TxRxState::MAB: {
			uint32_t dmaCHCTL = DMA_CHCTL(USART5_DMAx, USART5_TX_DMA_CHx);
			dmaCHCTL &= ~DMA_CHXCTL_CHEN;
			DMA_CHCTL(USART5_DMAx, USART5_TX_DMA_CHx) = dmaCHCTL;
			gd32_dma_interrupt_flag_clear<USART5_DMAx, USART5_TX_DMA_CHx, DMA_INTF_FTFIF>();
			const auto *p = &s_TxBuffer[dmx::config::USART5_PORT].dmx;
			DMA_CHMADDR(USART5_DMAx, USART5_TX_DMA_CHx) = (uint32_t) p->data;
			DMA_CHCNT(USART5_DMAx, USART5_TX_DMA_CHx) = (p->nLength & DMA_CHXCNT_CNT);
			dmaCHCTL |= DMA_CHXCTL_CHEN;
			dmaCHCTL |= DMA_INTERRUPT_ENABLE;
			DMA_CHCTL(USART5_DMAx, USART5_TX_DMA_CHx) = dmaCHCTL;
			USART_CTL2(USART5) |= USART_TRANSMIT_DMA_ENABLE;
		}
			break;
		default:
			break;
		}

		TIMER_INTF(TIMER4) = static_cast<uint32_t>(~TIMER_INT_FLAG_CH1);
	}
#endif
	/*
	 * UART 6
	 */
#if defined (DMX_USE_UART6)
	if ((TIMER_INTF(TIMER4) & TIMER_INT_FLAG_CH2) == TIMER_INT_FLAG_CH2) {
		switch (s_TxBuffer[dmx::config::UART6_PORT].State) {
		case TxRxState::DMXINTER:
			gd32_gpio_mode_output<UART6_GPIOx, UART6_TX_GPIO_PINx>();
			GPIO_BC(UART6_GPIOx) = UART6_TX_GPIO_PINx;
			s_TxBuffer[dmx::config::UART6_PORT].State = TxRxState::BREAK;
			TIMER_CH2CV(TIMER4) = TIMER_CNT(TIMER4) + s_nDmxTransmitBreakTime;
			break;
		case TxRxState::BREAK:
			gd32_gpio_mode_af<UART6_GPIOx, UART6_TX_GPIO_PINx, UART6>();
			s_TxBuffer[dmx::config::UART6_PORT].State = TxRxState::MAB;
			TIMER_CH2CV(TIMER4) = TIMER_CNT(TIMER4) + s_nDmxTransmitMabTime;
			break;
		case TxRxState::MAB: {
			uint32_t dmaCHCTL = DMA_CHCTL(UART6_DMAx, UART6_TX_DMA_CHx);
			dmaCHCTL &= ~DMA_CHXCTL_CHEN;
			DMA_CHCTL(UART6_DMAx, UART6_TX_DMA_CHx)= dmaCHCTL;
			gd32_dma_interrupt_flag_clear<UART6_DMAx, UART6_TX_DMA_CHx, DMA_INTF_FTFIF>();
			const auto *p = &s_TxBuffer[dmx::config::UART6_PORT].dmx;
			DMA_CHMADDR(UART6_DMAx, UART6_TX_DMA_CHx) = (uint32_t) p->data;
			DMA_CHCNT(UART6_DMAx, UART6_TX_DMA_CHx) = (p->nLength & DMA_CHXCNT_CNT);
			dmaCHCTL |= DMA_CHXCTL_CHEN;
			dmaCHCTL |= DMA_INTERRUPT_ENABLE;
			DMA_CHCTL(UART6_DMAx, UART6_TX_DMA_CHx)= dmaCHCTL;
			USART_CTL2(UART6) |= USART_TRANSMIT_DMA_ENABLE;
		}
			break;
		default:
			break;
		}

		TIMER_INTF(TIMER4) = static_cast<uint32_t>(~TIMER_INT_FLAG_CH2);
	}
#endif
	/*
	 * UART 7
	 */
#if defined (DMX_USE_UART7)
	if ((TIMER_INTF(TIMER4) & TIMER_INT_FLAG_CH3) == TIMER_INT_FLAG_CH3) {
		switch (s_TxBuffer[dmx::config::UART7_PORT].State) {
		case TxRxState::DMXINTER:
			gd32_gpio_mode_output<UART7_GPIOx, UART7_TX_GPIO_PINx>();
			GPIO_BC(UART7_GPIOx) = UART7_TX_GPIO_PINx;
			s_TxBuffer[dmx::config::UART7_PORT].State = TxRxState::BREAK;
			TIMER_CH3CV(TIMER4) = TIMER_CNT(TIMER4) + s_nDmxTransmitBreakTime;
			break;
		case TxRxState::BREAK:
			gd32_gpio_mode_af<UART7_GPIOx, UART7_TX_GPIO_PINx, UART7>();
			s_TxBuffer[dmx::config::UART7_PORT].State = TxRxState::MAB;
			TIMER_CH3CV(TIMER4) = TIMER_CNT(TIMER4) + s_nDmxTransmitMabTime;
			break;
		case TxRxState::MAB: {
			uint32_t dmaCHCTL = DMA_CHCTL(UART7_DMAx, UART7_TX_DMA_CHx);
			dmaCHCTL &= ~DMA_CHXCTL_CHEN;
			DMA_CHCTL(UART7_DMAx, UART7_TX_DMA_CHx) = dmaCHCTL;
			gd32_dma_interrupt_flag_clear<UART7_DMAx, UART7_TX_DMA_CHx, DMA_INTF_FTFIF>();
			const auto *p = &s_TxBuffer[dmx::config::UART7_PORT].dmx;
			DMA_CHMADDR(UART7_DMAx, UART7_TX_DMA_CHx) = (uint32_t) p->data;
			DMA_CHCNT(UART7_DMAx, UART7_TX_DMA_CHx) = (p->nLength & DMA_CHXCNT_CNT);
			dmaCHCTL |= DMA_CHXCTL_CHEN;
			dmaCHCTL |= DMA_INTERRUPT_ENABLE;
			DMA_CHCTL(UART7_DMAx, UART7_TX_DMA_CHx)= dmaCHCTL;
			USART_CTL2(UART7) |= USART_TRANSMIT_DMA_ENABLE;
		}
			break;
		default:
			break;
		}

		TIMER_INTF(TIMER4) = static_cast<uint32_t>(~TIMER_INT_FLAG_CH3);
	}
#endif
	// Clear all remaining interrupt flags (safety measure)
	TIMER_INTF(TIMER4) = static_cast<uint32_t>(~0);
}

void TIMER6_IRQHandler() {
	const auto nIntFlag = TIMER_INTF(TIMER6);

	if ((nIntFlag & TIMER_INT_FLAG_UP) == TIMER_INT_FLAG_UP) {
#if !defined(CONFIG_DMX_TRANSMIT_ONLY)
		for (size_t i = 0; i < DMX_MAX_PORTS; i++) {
			auto &packet = sv_nRxDmxPackets[i];
			packet.nPerSecond = sv_nRxDmxPackets[i].nCount - packet.nCountPrevious;
			packet.nCountPrevious = packet.nCount;
		}
#endif
		++g_Seconds.nUptime;
	}

	// Clear all remaining interrupt flags (safety measure)
	TIMER_INTF(TIMER6) = static_cast<uint32_t>(~nIntFlag);
}

#if !defined(CONFIG_DMX_TRANSMIT_ONLY)
void TIMER2_IRQHandler() {
	const auto nIntFlag = TIMER_INTF(TIMER2);

	if ((nIntFlag & TIMER_INT_FLAG_CH0) == TIMER_INT_FLAG_CH0) {
		if (sv_RxBuffer[0].State == TxRxState::DMXDATA) {
			sv_RxBuffer[0].State = TxRxState::IDLE;
			sv_RxBuffer[0].Dmx.nSlotsInPacket |= 0x8000;
		} else if (sv_RxBuffer[0].State == TxRxState::RDMDISC) {
			sv_RxBuffer[0].State = TxRxState::IDLE;
			sv_RxBuffer[0].Rdm.nIndex |= 0x4000;
		}
	}
#if DMX_MAX_PORTS >= 2
	else if ((nIntFlag & TIMER_INT_FLAG_CH1) == TIMER_INT_FLAG_CH1) {
		if (sv_RxBuffer[1].State == TxRxState::DMXDATA) {
			sv_RxBuffer[1].State = TxRxState::IDLE;
			sv_RxBuffer[1].Dmx.nSlotsInPacket |= 0x8000;
		} else if (sv_RxBuffer[1].State == TxRxState::RDMDISC) {
			sv_RxBuffer[1].State = TxRxState::IDLE;
			sv_RxBuffer[1].Rdm.nIndex |= 0x4000;
		}
	}
#endif
#if DMX_MAX_PORTS >= 3
	else if ((nIntFlag & TIMER_INT_FLAG_CH2) == TIMER_INT_FLAG_CH2) {
		if (sv_RxBuffer[2].State == TxRxState::DMXDATA) {
			sv_RxBuffer[2].State = TxRxState::IDLE;
			sv_RxBuffer[2].Dmx.nSlotsInPacket |= 0x8000;
		} else if (sv_RxBuffer[2].State == TxRxState::RDMDISC) {
			sv_RxBuffer[2].State = TxRxState::IDLE;
			sv_RxBuffer[2].Rdm.nIndex |= 0x4000;
		}
	}
#endif
#if DMX_MAX_PORTS >= 4
	else if ((nIntFlag & TIMER_INT_FLAG_CH3) == TIMER_INT_FLAG_CH3) {
		if (sv_RxBuffer[3].State == TxRxState::DMXDATA) {
			sv_RxBuffer[3].State = TxRxState::IDLE;
			sv_RxBuffer[3].Dmx.nSlotsInPacket |= 0x8000;
		} else if (sv_RxBuffer[3].State == TxRxState::RDMDISC) {
			sv_RxBuffer[3].State = TxRxState::IDLE;
			sv_RxBuffer[3].Rdm.nIndex |= 0x4000;
		}
	}
#endif
	timer_interrupt_flag_clear(TIMER2, nIntFlag);
}

void TIMER3_IRQHandler() {
	const auto nIntFlag = TIMER_INTF(TIMER3);
#if DMX_MAX_PORTS >= 5
	if ((nIntFlag & TIMER_INT_FLAG_CH0) == TIMER_INT_FLAG_CH0) {
		if (sv_RxBuffer[4].State == TxRxState::DMXDATA) {
			sv_RxBuffer[4].State = TxRxState::IDLE;
			sv_RxBuffer[4].Dmx.nSlotsInPacket |= 0x8000;
		} else if (sv_RxBuffer[4].State == TxRxState::RDMDISC) {
			sv_RxBuffer[4].State = TxRxState::IDLE;
			sv_RxBuffer[4].Rdm.nIndex |= 0x4000;
		}
	}
# if DMX_MAX_PORTS >= 6
	else if ((nIntFlag & TIMER_INT_FLAG_CH1) == TIMER_INT_FLAG_CH1) {
		if (sv_RxBuffer[5].State == TxRxState::DMXDATA) {
			sv_RxBuffer[5].State = TxRxState::IDLE;
			sv_RxBuffer[5].Dmx.nSlotsInPacket |= 0x8000;
		} else if (sv_RxBuffer[5].State == TxRxState::RDMDISC) {
			sv_RxBuffer[5].State = TxRxState::IDLE;
			sv_RxBuffer[5].Rdm.nIndex |= 0x4000;
		}
	}
# endif
# if DMX_MAX_PORTS >= 7
	else if ((nIntFlag & TIMER_INT_FLAG_CH2) == TIMER_INT_FLAG_CH2) {
		if (sv_RxBuffer[6].State == TxRxState::DMXDATA) {
			sv_RxBuffer[6].State = TxRxState::IDLE;
			sv_RxBuffer[6].Dmx.nSlotsInPacket |= 0x8000;
		} else if (sv_RxBuffer[6].State == TxRxState::RDMDISC) {
			sv_RxBuffer[6].State = TxRxState::IDLE;
			sv_RxBuffer[6].Rdm.nIndex |= 0x4000;
		}
	}
# endif
# if DMX_MAX_PORTS == 8
	else if ((nIntFlag & TIMER_INT_FLAG_CH3) == TIMER_INT_FLAG_CH3) {
		if (sv_RxBuffer[7].State == TxRxState::DMXDATA) {
			sv_RxBuffer[7].State = TxRxState::IDLE;
			sv_RxBuffer[7].Dmx.nSlotsInPacket |= 0x8000;
		} else if (sv_RxBuffer[7].State == TxRxState::RDMDISC) {
			sv_RxBuffer[7].State = TxRxState::IDLE;
			sv_RxBuffer[7].Rdm.nIndex |= 0x4000;
		}
	}
# endif
#endif
	timer_interrupt_flag_clear(TIMER3, nIntFlag);
}
#endif
}

extern "C" {
/*
 * USART 0
 */
#if defined (DMX_USE_USART0)
# if defined (GD32F4XX) || defined (GD32H7XX)
void DMA1_Channel7_IRQHandler() {
	if (gd32_dma_interrupt_flag_get<DMA1, DMA_CH7, DMA_INTERRUPT_FLAG_GET>()) {
		gd32_dma_interrupt_disable<DMA1, DMA_CH7, DMA_INTERRUPT_DISABLE>();

		if (s_TxBuffer[dmx::config::USART0_PORT].outputStyle == dmx::OutputStyle::DELTA) {
			s_TxBuffer[dmx::config::USART0_PORT].State = TxRxState::IDLE;
		} else {
			timer_channel_output_pulse_value_config(TIMER1, TIMER_CH_0 , TIMER_CNT(TIMER1) + s_nDmxTransmitInterTime);
			s_TxBuffer[dmx::config::USART0_PORT].State = TxRxState::DMXINTER;
		}

#if !defined (CONFIG_DMX_DISABLE_STATISTICS)
		sv_TotalStatistics[dmx::config::USART0_PORT].Dmx.Sent++;
#endif
	}

	gd32_dma_interrupt_flag_clear<DMA1, DMA_CH7, DMA_INTERRUPT_FLAG_CLEAR>();
}
# else
void DMA0_Channel3_IRQHandler() {
	if (gd32_dma_interrupt_flag_get<DMA0, DMA_CH3, DMA_INTERRUPT_FLAG_GET>()) {
		gd32_dma_interrupt_disable<DMA0, DMA_CH3, DMA_INTERRUPT_DISABLE>();

		if (s_TxBuffer[dmx::config::USART0_PORT].outputStyle == dmx::OutputStyle::DELTA) {
			s_TxBuffer[dmx::config::USART0_PORT].State = TxRxState::IDLE;
		} else {
			timer_channel_output_pulse_value_config(TIMER1, TIMER_CH_0 , TIMER_CNT(TIMER1) + s_nDmxTransmitInterTime);
			s_TxBuffer[dmx::config::USART0_PORT].State = TxRxState::DMXINTER;
		}

#if !defined (CONFIG_DMX_DISABLE_STATISTICS)
		sv_TotalStatistics[dmx::config::USART0_PORT].Dmx.Sent++;
#endif
	}

	gd32_dma_interrupt_flag_clear<DMA0, DMA_CH3, DMA_INTERRUPT_FLAG_CLEAR>();
}
# endif
#endif
/*
 * USART 1
 */
#if defined (DMX_USE_USART1)
void DMA0_Channel6_IRQHandler() {
	if (gd32_dma_interrupt_flag_get<DMA0, DMA_CH6, DMA_INTERRUPT_FLAG_GET>()) {
		gd32_dma_interrupt_disable<DMA0, DMA_CH6, DMA_INTERRUPT_DISABLE>();

		if (s_TxBuffer[dmx::config::USART1_PORT].outputStyle == dmx::OutputStyle::DELTA) {
			s_TxBuffer[dmx::config::USART1_PORT].State = TxRxState::IDLE;
		} else {
			timer_channel_output_pulse_value_config(TIMER1, TIMER_CH_1 , TIMER_CNT(TIMER1) + s_nDmxTransmitInterTime);
			s_TxBuffer[dmx::config::USART1_PORT].State = TxRxState::DMXINTER;
		}

#if !defined (CONFIG_DMX_DISABLE_STATISTICS)
		sv_TotalStatistics[dmx::config::USART1_PORT].Dmx.Sent++;
#endif
	}

	gd32_dma_interrupt_flag_clear<DMA0, DMA_CH6, DMA_INTERRUPT_FLAG_CLEAR>();
}
#endif
/*
 * USART 2
 */
#if defined (DMX_USE_USART2)
# if defined (GD32F4XX) || defined (GD32H7XX)
void DMA0_Channel3_IRQHandler() {
	if (gd32_dma_interrupt_flag_get<DMA0, DMA_CH3, DMA_INTERRUPT_FLAG_GET>()) {
		gd32_dma_interrupt_disable<DMA0, DMA_CH3, DMA_INTERRUPT_DISABLE>();

		if (s_TxBuffer[dmx::config::USART2_PORT].outputStyle == dmx::OutputStyle::DELTA) {
			s_TxBuffer[dmx::config::USART2_PORT].State = TxRxState::IDLE;
		} else {
			timer_channel_output_pulse_value_config(TIMER1, TIMER_CH_2 , TIMER_CNT(TIMER1) + s_nDmxTransmitInterTime);
			s_TxBuffer[dmx::config::USART2_PORT].State = TxRxState::DMXINTER;
		}

#if !defined (CONFIG_DMX_DISABLE_STATISTICS)
		sv_TotalStatistics[dmx::config::USART2_PORT].Dmx.Sent++;
#endif
	}

	gd32_dma_interrupt_flag_clear<DMA0, DMA_CH3, DMA_INTERRUPT_FLAG_CLEAR>();
}
# else
void DMA0_Channel1_IRQHandler() {
	if (gd32_dma_interrupt_flag_get<DMA0, DMA_CH1, DMA_INTERRUPT_FLAG_GET>()) {
		gd32_dma_interrupt_disable<DMA0, DMA_CH1, DMA_INTERRUPT_DISABLE>();

		if (s_TxBuffer[dmx::config::USART2_PORT].outputStyle == dmx::OutputStyle::DELTA) {
			s_TxBuffer[dmx::config::USART2_PORT].State = TxRxState::IDLE;
		} else {
			timer_channel_output_pulse_value_config(TIMER1, TIMER_CH_2 , TIMER_CNT(TIMER1) + s_nDmxTransmitInterTime);
			s_TxBuffer[dmx::config::USART2_PORT].State = TxRxState::DMXINTER;
		}

#if !defined (CONFIG_DMX_DISABLE_STATISTICS)
		sv_TotalStatistics[dmx::config::USART2_PORT].Dmx.Sent++;
#endif
	}

	gd32_dma_interrupt_flag_clear<DMA0, DMA_CH1, DMA_INTERRUPT_FLAG_CLEAR>();
}
# endif
#endif
/*
 * UART 3
 */
#if defined (DMX_USE_UART3)
# if defined (GD32F4XX) || defined (GD32H7XX)
void DMA0_Channel4_IRQHandler() {
	if (gd32_dma_interrupt_flag_get<DMA0, DMA_CH4, DMA_INTERRUPT_FLAG_GET>()) {
		gd32_dma_interrupt_disable<DMA0, DMA_CH4, DMA_INTERRUPT_DISABLE>();

		if (s_TxBuffer[dmx::config::UART3_PORT].outputStyle == dmx::OutputStyle::DELTA) {
			s_TxBuffer[dmx::config::UART3_PORT].State = TxRxState::IDLE;
		} else {
			timer_channel_output_pulse_value_config(TIMER4, TIMER_CH_3 , TIMER_CNT(TIMER4) + s_nDmxTransmitInterTime);
			s_TxBuffer[dmx::config::UART3_PORT].State = TxRxState::DMXINTER;
		}

#if !defined (CONFIG_DMX_DISABLE_STATISTICS)
		sv_TotalStatistics[dmx::config::UART3_PORT].Dmx.Sent++;
#endif
	}

	gd32_dma_interrupt_flag_clear<DMA0, DMA_CH4, DMA_INTERRUPT_FLAG_CLEAR>();
}
# else
void DMA1_Channel4_IRQHandler() {
	if (gd32_dma_interrupt_flag_get<DMA1, DMA_CH4, DMA_INTERRUPT_FLAG_GET>()) {
		gd32_dma_interrupt_disable<DMA1, DMA_CH4, DMA_INTERRUPT_DISABLE>();

		if (s_TxBuffer[dmx::config::UART3_PORT].outputStyle == dmx::OutputStyle::DELTA) {
			s_TxBuffer[dmx::config::UART3_PORT].State = TxRxState::IDLE;
		} else {
			timer_channel_output_pulse_value_config(TIMER1, TIMER_CH_3 , TIMER_CNT(TIMER1) + s_nDmxTransmitInterTime);
			s_TxBuffer[dmx::config::UART3_PORT].State = TxRxState::DMXINTER;
		}

#if !defined (CONFIG_DMX_DISABLE_STATISTICS)
		sv_TotalStatistics[dmx::config::UART3_PORT].Dmx.Sent++;
#endif
	}

	gd32_dma_interrupt_flag_clear<DMA1, DMA_CH4, DMA_INTERRUPT_FLAG_CLEAR>();
}
# endif
#endif
/*
 * UART 4
 */
#if defined (DMX_USE_UART4)
# if defined (GD32F20X)
void DMA1_Channel3_IRQHandler() {
	if (gd32_dma_interrupt_flag_get<DMA1, DMA_CH3, DMA_INTERRUPT_FLAG_GET>()) {
		gd32_dma_interrupt_disable<DMA1, DMA_CH3, DMA_INTERRUPT_DISABLE>();

		if (s_TxBuffer[dmx::config::UART4_PORT].outputStyle == dmx::OutputStyle::DELTA) {
			s_TxBuffer[dmx::config::UART4_PORT].State = TxRxState::IDLE;
		} else {
			timer_channel_output_pulse_value_config(TIMER4, TIMER_CH_0 , TIMER_CNT(TIMER4) + s_nDmxTransmitInterTime);
			s_TxBuffer[dmx::config::UART4_PORT].State = TxRxState::DMXINTER;
		}

#if !defined (CONFIG_DMX_DISABLE_STATISTICS)
		sv_TotalStatistics[dmx::config::UART4_PORT].Dmx.Sent++;
#endif
	}

	gd32_dma_interrupt_flag_clear<DMA1, DMA_CH3, DMA_INTERRUPT_FLAG_CLEAR>();
}
# endif
# if defined (GD32F4XX) || defined (GD32H7XX)
void DMA0_Channel7_IRQHandler() {
	if (gd32_dma_interrupt_flag_get<DMA0, DMA_CH7, DMA_INTERRUPT_FLAG_GET>()) {
		gd32_dma_interrupt_disable<DMA0, DMA_CH7, DMA_INTERRUPT_DISABLE>();

		if (s_TxBuffer[dmx::config::UART4_PORT].outputStyle == dmx::OutputStyle::DELTA) {
			s_TxBuffer[dmx::config::UART4_PORT].State = TxRxState::IDLE;
		} else {
			timer_channel_output_pulse_value_config(TIMER4, TIMER_CH_0 , TIMER_CNT(TIMER4) + s_nDmxTransmitInterTime);
			s_TxBuffer[dmx::config::UART4_PORT].State = TxRxState::DMXINTER;
		}
	}

#if !defined (CONFIG_DMX_DISABLE_STATISTICS)
		sv_TotalStatistics[dmx::config::UART4_PORT].Dmx.Sent++;
#endif

	gd32_dma_interrupt_flag_clear<DMA0, DMA_CH7, DMA_INTERRUPT_FLAG_CLEAR>();
}
# endif
#endif
/*
 * USART 5
 */
#if defined (DMX_USE_USART5)
void DMA1_Channel6_IRQHandler() {
	if (gd32_dma_interrupt_flag_get<DMA1, DMA_CH6, DMA_INTERRUPT_FLAG_GET>()) {
		gd32_dma_interrupt_disable<DMA1, DMA_CH6, DMA_INTERRUPT_DISABLE>();

		if (s_TxBuffer[dmx::config::USART5_PORT].outputStyle == dmx::OutputStyle::DELTA) {
			s_TxBuffer[dmx::config::USART5_PORT].State = TxRxState::IDLE;
		} else {
			timer_channel_output_pulse_value_config(TIMER4, TIMER_CH_1 , TIMER_CNT(TIMER4) + s_nDmxTransmitInterTime);
			s_TxBuffer[dmx::config::USART5_PORT].State = TxRxState::DMXINTER;
		}

#if !defined (CONFIG_DMX_DISABLE_STATISTICS)
		sv_TotalStatistics[dmx::config::USART5_PORT].Dmx.Sent++;
#endif
	}

	gd32_dma_interrupt_flag_clear<DMA1, DMA_CH6, DMA_INTERRUPT_FLAG_CLEAR>();
}
#endif
/*
 * UART 6
 */
#if defined (DMX_USE_UART6)
# if defined (GD32F20X)
void DMA1_Channel4_IRQHandler() {
	if (gd32_dma_interrupt_flag_get<DMA1, DMA_CH4, DMA_INTERRUPT_FLAG_GET>()) {
		gd32_dma_interrupt_disable<DMA1, DMA_CH4, DMA_INTERRUPT_DISABLE>();

		if (s_TxBuffer[dmx::config::UART6_PORT].outputStyle == dmx::OutputStyle::DELTA) {
			s_TxBuffer[dmx::config::UART6_PORT].State = TxRxState::IDLE;
		} else {
			timer_channel_output_pulse_value_config(TIMER4, TIMER_CH_2 , TIMER_CNT(TIMER4) + s_nDmxTransmitInterTime);
			s_TxBuffer[dmx::config::UART6_PORT].State = TxRxState::DMXINTER;
		}

#if !defined (CONFIG_DMX_DISABLE_STATISTICS)
		sv_TotalStatistics[dmx::config::UART6_PORT].Dmx.Sent++;
#endif
	}

	gd32_dma_interrupt_flag_clear<DMA1, DMA_CH4, DMA_INTERRUPT_FLAG_CLEAR>();
}
# endif
# if defined (GD32F4XX) || defined (GD32H7XX)
void DMA0_Channel1_IRQHandler() {
	if (gd32_dma_interrupt_flag_get<DMA0, DMA_CH1, DMA_INTERRUPT_FLAG_GET>()) {
		gd32_dma_interrupt_disable<DMA0, DMA_CH1, DMA_INTERRUPT_DISABLE>();

		if (s_TxBuffer[dmx::config::UART6_PORT].outputStyle == dmx::OutputStyle::DELTA) {
			s_TxBuffer[dmx::config::UART6_PORT].State = TxRxState::IDLE;
		} else {
			timer_channel_output_pulse_value_config(TIMER4, TIMER_CH_2 , TIMER_CNT(TIMER4) + s_nDmxTransmitInterTime);
			s_TxBuffer[dmx::config::UART6_PORT].State = TxRxState::DMXINTER;
		}

#if !defined (CONFIG_DMX_DISABLE_STATISTICS)
		sv_TotalStatistics[dmx::config::UART6_PORT].Dmx.Sent++;
#endif
	}

	gd32_dma_interrupt_flag_clear<DMA0, DMA_CH1, DMA_INTERRUPT_FLAG_CLEAR>();
}
# endif
#endif
/*
 * UART 7
 */
#if defined (DMX_USE_UART7)
# if defined (GD32F20X)
void DMA1_Channel3_IRQHandler() {
	if (gd32_dma_interrupt_flag_get<DMA1, DMA_CH3, DMA_INTERRUPT_FLAG_GET>()) {
		gd32_dma_interrupt_disable<DMA1, DMA_CH3, DMA_INTERRUPT_DISABLE>();

		if (s_TxBuffer[dmx::config::UART7_PORT].outputStyle == dmx::OutputStyle::DELTA) {
			s_TxBuffer[dmx::config::UART7_PORT].State = TxRxState::IDLE;
		} else {
			timer_channel_output_pulse_value_config(TIMER4, TIMER_CH_3 , TIMER_CNT(TIMER4) + s_nDmxTransmitInterTime);
			s_TxBuffer[dmx::config::UART7_PORT].State = TxRxState::DMXINTER;
		}

#if !defined (CONFIG_DMX_DISABLE_STATISTICS)
		sv_TotalStatistics[dmx::config::UART7_PORT].Dmx.Sent++;
#endif
	}

	gd32_dma_interrupt_flag_clear<DMA1, DMA_CH3, DMA_INTERRUPT_FLAG_CLEAR>();
}
# endif
# if defined (GD32F4XX) || defined (GD32H7XX)
void DMA0_Channel0_IRQHandler() {
	if (gd32_dma_interrupt_flag_get<DMA0, DMA_CH0, DMA_INTERRUPT_FLAG_GET>()) {
		gd32_dma_interrupt_disable<DMA0, DMA_CH0, DMA_INTERRUPT_DISABLE>();

		if (s_TxBuffer[dmx::config::UART7_PORT].outputStyle == dmx::OutputStyle::DELTA) {
			s_TxBuffer[dmx::config::UART7_PORT].State = TxRxState::IDLE;
		} else {
			timer_channel_output_pulse_value_config(TIMER4, TIMER_CH_3 , TIMER_CNT(TIMER4) + s_nDmxTransmitInterTime);
			s_TxBuffer[dmx::config::UART7_PORT].State = TxRxState::DMXINTER;
		}

#if !defined (CONFIG_DMX_DISABLE_STATISTICS)
		sv_TotalStatistics[dmx::config::UART7_PORT].Dmx.Sent++;
#endif
	}

	gd32_dma_interrupt_flag_clear<DMA0, DMA_CH0, DMA_INTERRUPT_FLAG_CLEAR>();
}
# endif
#endif
}

static void uart_dmx_config(const uint32_t usart_periph) {
	gd32_uart_begin(usart_periph, 250000U, GD32_UART_BITS_8, GD32_UART_PARITY_NONE, GD32_UART_STOP_2BITS);
}

Dmx *Dmx::s_pThis;

Dmx::Dmx() {
	DEBUG_ENTRY

	assert(s_pThis == nullptr);
	s_pThis = this;

	s_nDmxTransmitBreakTime = m_nDmxTransmitBreakTime;
	s_nDmxTransmitMabTime = m_nDmxTransmitMabTime;
	s_nDmxTransmitInterTime = dmx::transmit::PERIOD_DEFAULT - s_nDmxTransmitBreakTime - s_nDmxTransmitMabTime - (dmx::max::CHANNELS * 44) - 44;

	for (auto i = 0; i < DMX_MAX_PORTS; i++) {
#if defined (GPIO_INIT)
		gpio_init(s_DirGpio[i].nPort, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, s_DirGpio[i].nPin);
#else
		gpio_mode_set(s_DirGpio[i].nPort, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLDOWN, s_DirGpio[i].nPin);
		gpio_output_options_set(s_DirGpio[i].nPort, GPIO_OTYPE_PP, GPIO_OSPEED, s_DirGpio[i].nPin);
		gpio_af_set(s_DirGpio[i].nPort, GPIO_AF_0, s_DirGpio[i].nPin);
#endif
		m_nDmxTransmissionLength[i] = dmx::max::CHANNELS;
		SetPortDirection(i, PortDirection::INP, false);
		sv_RxBuffer[i].State = TxRxState::IDLE;
		s_TxBuffer[i].State = TxRxState::IDLE;
		s_TxBuffer[i].outputStyle = dmx::OutputStyle::DELTA;
		ClearData(i);
	}

	usart_dma_config();	// DMX Transmit
#if defined (DMX_USE_USART0) || defined (DMX_USE_USART1) || defined (DMX_USE_USART2) || defined (DMX_USE_UART3)
	timer1_config();	// DMX Transmit -> USART0, USART1, USART2, UART3
#endif
#if defined (DMX_USE_UART4) || defined (DMX_USE_USART5) ||  defined (DMX_USE_UART6) || defined (DMX_USE_UART7)
	timer4_config();	// DMX Transmit -> UART4, USART5, UART6, UART7
#endif
#if !defined(CONFIG_DMX_TRANSMIT_ONLY)
	timer2_config();	// DMX Receive	-> Slot time-out Port 0,1,2,3
	timer3_config();	// DMX Receive	-> Slot time-out Port 4,5,6,7
#endif

#if defined (DMX_USE_USART0)
	uart_dmx_config(USART0);
	NVIC_SetPriority(USART0_IRQn, 0);
	NVIC_EnableIRQ(USART0_IRQn);
#endif
#if defined (DMX_USE_USART1)
	uart_dmx_config(USART1);
	NVIC_SetPriority(USART1_IRQn, 0);
	NVIC_EnableIRQ(USART1_IRQn);
#endif
#if defined (DMX_USE_USART2)
	uart_dmx_config(USART2);
	NVIC_SetPriority(USART2_IRQn, 0);
	NVIC_EnableIRQ(USART2_IRQn);
#endif
#if defined (DMX_USE_UART3)
	uart_dmx_config(UART3);
	NVIC_SetPriority(UART3_IRQn, 0);
	NVIC_EnableIRQ(UART3_IRQn);
#endif
#if defined (DMX_USE_UART4)
	uart_dmx_config(UART4);
	NVIC_SetPriority(UART4_IRQn, 0);
	NVIC_EnableIRQ(UART4_IRQn);
#endif
#if defined (DMX_USE_USART5)
	uart_dmx_config(USART5);
	NVIC_SetPriority(USART5_IRQn, 0);
	NVIC_EnableIRQ(USART5_IRQn);
#endif
#if defined (DMX_USE_UART6)
	uart_dmx_config(UART6);
	NVIC_SetPriority(UART6_IRQn, 0);
	NVIC_EnableIRQ(UART6_IRQn);
#endif
#if defined (DMX_USE_UART7)
	uart_dmx_config(UART7);
	NVIC_SetPriority(UART7_IRQn, 0);
	NVIC_EnableIRQ(UART7_IRQn);
#endif

	DEBUG_EXIT
}

void Dmx::SetPortDirection(const uint32_t nPortIndex, PortDirection portDirection, bool bEnableData) {
	assert(nPortIndex < dmx::config::max::PORTS);

	const auto nUart = dmx_port_to_uart(nPortIndex);

	if (m_dmxPortDirection[nPortIndex] != portDirection) {
		m_dmxPortDirection[nPortIndex] = portDirection;

		StopData(nUart, nPortIndex);

		if (portDirection == PortDirection::OUTP) {
			GPIO_BOP(s_DirGpio[nPortIndex].nPort) = s_DirGpio[nPortIndex].nPin;
		} else if (portDirection == PortDirection::INP) {
			GPIO_BC(s_DirGpio[nPortIndex].nPort) = s_DirGpio[nPortIndex].nPin;
		} else {
			assert(0);
		}
	} else if (!bEnableData) {
		StopData(nUart, nPortIndex);
	}

	if (bEnableData) {
		StartData(nUart, nPortIndex);
	}
}

void Dmx::ClearData(const uint32_t nPortIndex) {
	assert(nPortIndex < dmx::config::max::PORTS);

	auto *p = &s_TxBuffer[nPortIndex];
	p->dmx.nLength = 513; // Including START Code
	memset(p->dmx.data, 0, dmx::buffer::SIZE);
}

#if !defined (CONFIG_DMX_DISABLE_STATISTICS)
volatile dmx::TotalStatistics& Dmx::GetTotalStatistics(const uint32_t nPortIndex) {
	sv_TotalStatistics[nPortIndex].Dmx.Received = sv_nRxDmxPackets[nPortIndex].nCount;
	return sv_TotalStatistics[nPortIndex];
}
#endif

void Dmx::StartDmxOutput(const uint32_t nPortIndex) {
	assert(nPortIndex < dmx::config::max::PORTS);
	const auto nUart = dmx_port_to_uart(nPortIndex);

	/*
	 * USART_FLAG_TC is set after power on.
	 * The flag is cleared by DMA interrupt when maximum slots - 1 are transmitted.
	 */

	//TODO Do we need a timeout just to be safe?
	while (SET != usart_flag_get(nUart, USART_FLAG_TC))
		;

	switch (nUart) {
	/* TIMER 1 */
#if defined (DMX_USE_USART0)
	case USART0:
		gd32_gpio_mode_output<USART0_GPIOx, USART0_TX_GPIO_PINx>();
		GPIO_BC(USART0_GPIOx) = USART0_TX_GPIO_PINx;
		TIMER_CH0CV(TIMER1) = TIMER_CNT(TIMER1) + s_nDmxTransmitBreakTime;
		s_TxBuffer[dmx::config::USART0_PORT].State = TxRxState::BREAK;
		return;
		break;
#endif
#if defined (DMX_USE_USART1)
	case USART1:
		gd32_gpio_mode_output<USART1_GPIOx, USART1_TX_GPIO_PINx>();
		GPIO_BC(USART1_GPIOx) = USART1_TX_GPIO_PINx;
		TIMER_CH1CV(TIMER1) = TIMER_CNT(TIMER1) + s_nDmxTransmitBreakTime;
		s_TxBuffer[dmx::config::USART1_PORT].State = TxRxState::BREAK;
		return;
		break;
#endif
#if defined (DMX_USE_USART2)
	case USART2:
		gd32_gpio_mode_output<USART2_GPIOx, USART2_TX_GPIO_PINx>();
		GPIO_BC(USART2_GPIOx) = USART2_TX_GPIO_PINx;
		TIMER_CH2CV(TIMER1) = TIMER_CNT(TIMER1) + s_nDmxTransmitBreakTime;
		s_TxBuffer[dmx::config::USART2_PORT].State = TxRxState::BREAK;
		return;
		break;
#endif
#if defined (DMX_USE_UART3)
	case UART3:
		gd32_gpio_mode_output<UART3_GPIOx, UART3_TX_GPIO_PINx>();
		GPIO_BC(UART3_GPIOx) = UART3_TX_GPIO_PINx;
		TIMER_CH3CV(TIMER1) = TIMER_CNT(TIMER1) + s_nDmxTransmitBreakTime;
		s_TxBuffer[dmx::config::UART3_PORT].State = TxRxState::BREAK;
		return;
		break;
#endif
	/* TIMER 4 */
#if defined (DMX_USE_UART4)
	case UART4:
		gd32_gpio_mode_output<UART4_TX_GPIOx, UART4_TX_GPIO_PINx>();
		GPIO_BC(UART4_TX_GPIOx) = UART4_TX_GPIO_PINx;
		TIMER_CH0CV(TIMER4) = TIMER_CNT(TIMER4) + s_nDmxTransmitBreakTime;
		s_TxBuffer[dmx::config::UART4_PORT].State = TxRxState::BREAK;
		return;
		break;
#endif
#if defined (DMX_USE_USART5)
	case USART5:
		gd32_gpio_mode_output<USART5_GPIOx, USART5_TX_GPIO_PINx>();
		GPIO_BC(USART5_GPIOx) = USART5_TX_GPIO_PINx;
		TIMER_CH1CV(TIMER4) = TIMER_CNT(TIMER4) + s_nDmxTransmitBreakTime;
		s_TxBuffer[dmx::config::USART5_PORT].State = TxRxState::BREAK;
		return;
		break;
#endif
#if defined (DMX_USE_UART6)
	case UART6:
		gd32_gpio_mode_output<UART6_GPIOx, UART6_TX_GPIO_PINx>();
		GPIO_BC(UART6_GPIOx) = UART6_TX_GPIO_PINx;
		TIMER_CH2CV(TIMER4) = TIMER_CNT(TIMER4) + s_nDmxTransmitBreakTime;
		s_TxBuffer[dmx::config::UART6_PORT].State = TxRxState::BREAK;
		return;
		break;
#endif
#if defined (DMX_USE_UART7)
	case UART7:
		gd32_gpio_mode_output<UART7_GPIOx, UART7_TX_GPIO_PINx>();
		GPIO_BC(UART7_GPIOx) = UART7_TX_GPIO_PINx;
		TIMER_CH3CV(TIMER4) = TIMER_CNT(TIMER4) + s_nDmxTransmitBreakTime;
		s_TxBuffer[dmx::config::UART7_PORT].State = TxRxState::BREAK;
		return;
		break;
#endif
	default:
		assert(0);
		__builtin_unreachable();
		break;
	}

	assert(0);
	__builtin_unreachable();
}


void Dmx::StopData(const uint32_t nUart, const uint32_t nPortIndex) {
	assert(nPortIndex < dmx::config::max::PORTS);

	if (sv_PortState[nPortIndex] == PortState::IDLE) {
		return;
	}

	sv_PortState[nPortIndex] = PortState::IDLE;

	if (m_dmxPortDirection[nPortIndex] == PortDirection::OUTP) {
		do {
			if (s_TxBuffer[nPortIndex].State == dmx::TxRxState::DMXINTER) {
				gd32_usart_flag_clear<USART_FLAG_TC>(nUart);
				do {
					__DMB();
				} while (!gd32_usart_flag_get<USART_FLAG_TC>(nUart));

				s_TxBuffer[nPortIndex].State = dmx::TxRxState::IDLE;
			}
		} while (s_TxBuffer[nPortIndex].State != dmx::TxRxState::IDLE);

		return;
	}

	if (m_dmxPortDirection[nPortIndex] == PortDirection::INP) {
		usart_interrupt_disable(nUart, USART_INT_RBNE);
		sv_RxBuffer[nPortIndex].State = TxRxState::IDLE;

#if !defined(CONFIG_DMX_TRANSMIT_ONLY)
		switch (nPortIndex) {
		case 0:
			TIMER_DMAINTEN(TIMER2) &= (~TIMER_INT_CH0);
			break;
#if DMX_MAX_PORTS >= 2
		case 1:
			TIMER_DMAINTEN(TIMER2) &= (~TIMER_INT_CH1);
			break;
#endif
#if DMX_MAX_PORTS >= 3
		case 2:
			TIMER_DMAINTEN(TIMER2) &= (~TIMER_INT_CH2);
			break;
#endif
#if DMX_MAX_PORTS >= 4
		case 3:
			TIMER_DMAINTEN(TIMER2) &= (~TIMER_INT_CH3);
			break;
#endif
#if DMX_MAX_PORTS >= 5
		case 4:
			TIMER_DMAINTEN(TIMER3) &= (~TIMER_INT_CH0);
			break;
#endif
#if DMX_MAX_PORTS >= 6
		case 5:
			TIMER_DMAINTEN(TIMER3) &= (~TIMER_INT_CH1);
			break;
#endif
#if DMX_MAX_PORTS >= 7
		case 6:
			TIMER_DMAINTEN(TIMER3) &= (~TIMER_INT_CH2);
			break;
#endif
#if DMX_MAX_PORTS == 8
		case 7:
			TIMER_DMAINTEN(TIMER3) &= (~TIMER_INT_CH3);
			break;
#endif
		default:
			assert(0);
			__builtin_unreachable();
			break;
		}
#endif
		return;
	}

	assert(0);
	__builtin_unreachable();
}

// DMX Send

void Dmx::SetDmxBreakTime(uint32_t nBreakTime) {
	s_nDmxTransmitBreakTime = std::max(transmit::BREAK_TIME_MIN, nBreakTime);
	SetDmxPeriodTime(m_nDmxTransmitPeriodRequested);
}

void Dmx::SetDmxMabTime(uint32_t nMabTime) {
	s_nDmxTransmitMabTime = std::max(transmit::MAB_TIME_MIN, nMabTime);
	SetDmxPeriodTime(m_nDmxTransmitPeriodRequested);
}

void Dmx::SetDmxPeriodTime(uint32_t nPeriod) {
	m_nDmxTransmitPeriodRequested = nPeriod;

	auto nLengthMax = s_TxBuffer[0].dmx.nLength;

	for (uint32_t nPortIndex = 1; nPortIndex < dmx::config::max::PORTS; nPortIndex++) {
		const auto nLength = s_TxBuffer[nPortIndex].dmx.nLength;
		if (nLength > nLengthMax) {
			nLengthMax = nLength;
		}
	}

	auto nPackageLengthMicroSeconds = s_nDmxTransmitBreakTime + s_nDmxTransmitMabTime + (nLengthMax * 44U);

	// The GD32F4xx/GD32H7XX Timer 1 has a 32-bit counter
#if  defined(GD32F4XX) || defined (GD32H7XX)
#else
	if (nPackageLengthMicroSeconds > (static_cast<uint16_t>(~0) - 44U)) {
		s_nDmxTransmitBreakTime = std::min(transmit::BREAK_TIME_TYPICAL, s_nDmxTransmitBreakTime);
		s_nDmxTransmitMabTime = transmit::MAB_TIME_MIN;
		nPackageLengthMicroSeconds = s_nDmxTransmitBreakTime + s_nDmxTransmitMabTime + (nLengthMax * 44U);
	}
#endif

	if (nPeriod != 0) {
		if (nPeriod < nPackageLengthMicroSeconds) {
			m_nDmxTransmitPeriod = std::max(transmit::BREAK_TO_BREAK_TIME_MIN, nPackageLengthMicroSeconds + 44U);
		} else {
			m_nDmxTransmitPeriod = nPeriod;
		}
	} else {
		m_nDmxTransmitPeriod = std::max(transmit::BREAK_TO_BREAK_TIME_MIN, nPackageLengthMicroSeconds + 44U);
	}

	s_nDmxTransmitInterTime = m_nDmxTransmitPeriod - nPackageLengthMicroSeconds;

	m_nDmxTransmitBreakTime = s_nDmxTransmitBreakTime;
	m_nDmxTransmitMabTime = s_nDmxTransmitMabTime;

	DEBUG_PRINTF("nPeriod=%u, nLengthMax=%u, m_nDmxTransmitPeriod=%u, nPackageLengthMicroSeconds=%u -> s_nDmxTransmitInterTime=%u", nPeriod, nLengthMax, m_nDmxTransmitPeriod, nPackageLengthMicroSeconds, s_nDmxTransmitInterTime);
}

void Dmx::SetDmxSlots(uint16_t nSlots) {
	if ((nSlots >= 2) && (nSlots <= dmx::max::CHANNELS)) {
		m_nDmxTransmitSlots = nSlots;

		for (uint32_t i = 0; i < dmx::config::max::PORTS; i++) {
			m_nDmxTransmissionLength[i] = std::min(m_nDmxTransmissionLength[i], static_cast<uint32_t>(nSlots));
		}

		SetDmxPeriodTime(m_nDmxTransmitPeriodRequested);
	}
}

#include <cstdio>

void Dmx::SetOutputStyle(const uint32_t nPortIndex, const dmx::OutputStyle outputStyle) {
	assert(nPortIndex < dmx::config::max::PORTS);
	s_TxBuffer[nPortIndex].outputStyle = outputStyle;

	if (outputStyle == dmx::OutputStyle::CONTINOUS) {
		if (!m_bHasContinuosOutput) {
			m_bHasContinuosOutput = true;
			if (m_dmxPortDirection[nPortIndex] == dmx::PortDirection::OUTP) {
				StartDmxOutput(nPortIndex);
			}
			return;
		}

		for (size_t nIndex = 0; nIndex < dmx::config::max::PORTS; nIndex++) {
			if ((s_TxBuffer[nIndex].outputStyle == dmx::OutputStyle::CONTINOUS) && (m_dmxPortDirection[nIndex] == dmx::PortDirection::OUTP)) {
				const auto nUart = dmx_port_to_uart(nIndex);
				StopData(nUart, nIndex);
			}
		}

		for (size_t nIndex = 0; nIndex < dmx::config::max::PORTS; nIndex++) {
			if ((s_TxBuffer[nIndex].outputStyle == dmx::OutputStyle::CONTINOUS) && (m_dmxPortDirection[nIndex] == dmx::PortDirection::OUTP)) {
				StartDmxOutput(nIndex);
			}
		}
	} else {
		m_bHasContinuosOutput = false;
		for (size_t nIndex = 0; nIndex < dmx::config::max::PORTS; nIndex++) {
			if (s_TxBuffer[nIndex].outputStyle == dmx::OutputStyle::CONTINOUS) {
				m_bHasContinuosOutput = true;
				return;
			}
		}
	}
}

dmx::OutputStyle Dmx::GetOutputStyle(const uint32_t nPortIndex) const {
	assert(nPortIndex < dmx::config::max::PORTS);
	return s_TxBuffer[nPortIndex].outputStyle;
}

void Dmx::SetSendData(const uint32_t nPortIndex, const uint8_t *pData, uint32_t nLength) {
	assert(nPortIndex < dmx::config::max::PORTS);

	auto &p = s_TxBuffer[nPortIndex];
	auto *pDst = p.dmx.data;

	nLength = std::min(nLength, static_cast<uint32_t>(m_nDmxTransmitSlots));
	p.dmx.nLength = static_cast<uint16_t>(nLength + 1);

	memcpy(pDst, pData, nLength);

	if (nLength != m_nDmxTransmissionLength[nPortIndex]) {
		m_nDmxTransmissionLength[nPortIndex] = nLength;
		SetDmxPeriodTime(m_nDmxTransmitPeriodRequested);
	}
}

void Dmx::SetSendDataWithoutSC(const uint32_t nPortIndex, const uint8_t *pData, uint32_t nLength) {
	assert(nPortIndex < dmx::config::max::PORTS);

	auto &p = s_TxBuffer[nPortIndex];
	auto *pDst = p.dmx.data;

	nLength = std::min(nLength, static_cast<uint32_t>(m_nDmxTransmitSlots));
	p.dmx.nLength = static_cast<uint16_t>(nLength + 1);
	p.dmx.bDataPending = true;

	pDst[0] = START_CODE;
	memcpy(&pDst[1], pData, nLength);

	if (nLength != m_nDmxTransmissionLength[nPortIndex]) {
		m_nDmxTransmissionLength[nPortIndex] = nLength;
		SetDmxPeriodTime(m_nDmxTransmitPeriodRequested);
	}
}

void Dmx::Blackout() {
	DEBUG_ENTRY

	for (uint32_t nPortIndex = 0; nPortIndex < DMX_MAX_PORTS; nPortIndex++) {
		if (m_dmxPortDirection[nPortIndex] == dmx::PortDirection::OUTP) {
			const auto nUart = dmx_port_to_uart(nPortIndex);

			StopData(nUart, nPortIndex);
			ClearData(nPortIndex);
			StartData(nUart, nPortIndex);
		}
	}

	DEBUG_EXIT
}

void Dmx::FullOn() {
	DEBUG_ENTRY

	for (uint32_t nPortIndex = 0; nPortIndex < DMX_MAX_PORTS; nPortIndex++) {
		if (m_dmxPortDirection[nPortIndex] == dmx::PortDirection::OUTP) {
			const auto nUart = dmx_port_to_uart(nPortIndex);

			StopData(nUart, nPortIndex);

			auto *p = &s_TxBuffer[nPortIndex];
			auto *p16 = reinterpret_cast<uint16_t *>(p->dmx.data);

			for (auto i = 0; i < dmx::buffer::SIZE / 2; i++) {
				*p16++ = UINT16_MAX;
			}

			p->dmx.data[0] = dmx::START_CODE;
			p->dmx.nLength = 513;

			StartData(nUart, nPortIndex);
		}
	}

	DEBUG_EXIT
}

void Dmx::StartOutput(const uint32_t nPortIndex) {
	if ((sv_PortState[nPortIndex] == dmx::PortState::TX)
			&& (s_TxBuffer[nPortIndex].outputStyle == dmx::OutputStyle::DELTA)
			&& (s_TxBuffer[nPortIndex].State == dmx::TxRxState::IDLE)) {
		StartDmxOutput(nPortIndex);
	}
}

void Dmx::Sync() {
	logic_analyzer::ch0_set();

	for (uint32_t nPortIndex = 0; nPortIndex < dmx::config::max::PORTS; nPortIndex++) {
		auto &txBuffer = s_TxBuffer[nPortIndex];

		if (!txBuffer.dmx.bDataPending) {
			continue;
		}

		txBuffer.dmx.bDataPending = false;

		if ((sv_PortState[nPortIndex] == dmx::PortState::TX)) {
			if ((txBuffer.outputStyle == dmx::OutputStyle::DELTA) && (txBuffer.State == dmx::TxRxState::IDLE)) {
				logic_analyzer::ch1_set();
				StartDmxOutput(nPortIndex);
				logic_analyzer::ch1_clear();
			}
		}
	}

	logic_analyzer::ch0_clear();
}

void Dmx::StartData(const uint32_t nUart, const uint32_t nPortIndex) {
	assert(nPortIndex < dmx::config::max::PORTS);
	assert(sv_PortState[nPortIndex] == PortState::IDLE);

	if (m_dmxPortDirection[nPortIndex] == dmx::PortDirection::OUTP) {
		sv_PortState[nPortIndex] = PortState::TX;
		return;
	}

	if (m_dmxPortDirection[nPortIndex] == dmx::PortDirection::INP) {
		sv_RxBuffer[nPortIndex].State = TxRxState::IDLE;

		while (RESET == usart_flag_get(nUart, USART_FLAG_TBE))
			;

		usart_interrupt_flag_clear(nUart, USART_INT_FLAG_RBNE);
		usart_interrupt_enable(nUart, USART_INT_RBNE);

		sv_PortState[nPortIndex] = PortState::RX;

#if !defined(CONFIG_DMX_TRANSMIT_ONLY)
		switch (nPortIndex) {
		case 0:
			TIMER_DMAINTEN(TIMER2) |= TIMER_INT_CH0;
			break;
#if DMX_MAX_PORTS >= 2
		case 1:
			TIMER_DMAINTEN(TIMER2) |= TIMER_INT_CH1;
			break;
#endif
#if DMX_MAX_PORTS >= 3
		case 2:
			TIMER_DMAINTEN(TIMER2) |= TIMER_INT_CH2;
			break;
#endif
#if DMX_MAX_PORTS >= 4
		case 3:
			TIMER_DMAINTEN(TIMER2) |= TIMER_INT_CH3;
			break;
#endif
#if DMX_MAX_PORTS >= 5
		case 4:
			TIMER_DMAINTEN(TIMER3) |= TIMER_INT_CH0;
			break;
#endif
#if DMX_MAX_PORTS >= 6
		case 5:
			TIMER_DMAINTEN(TIMER3) |= TIMER_INT_CH1;
			break;
#endif
#if DMX_MAX_PORTS >= 7
		case 6:
			TIMER_DMAINTEN(TIMER3) |= TIMER_INT_CH2;
			break;
#endif
#if DMX_MAX_PORTS == 8
		case 7:
			TIMER_DMAINTEN(TIMER3) |= TIMER_INT_CH3;
			break;
#endif
		default:
			assert(0);
			__builtin_unreachable();
			break;
		}
#endif
		return;
	}

	assert(0);
	__builtin_unreachable();
}


// DMX Receive

const uint8_t *Dmx::GetDmxChanged([[maybe_unused]] uint32_t nPortIndex) {
#if !defined(CONFIG_DMX_TRANSMIT_ONLY)
	const auto *p = GetDmxAvailable(nPortIndex);

	if (p == nullptr) {
		return nullptr;
	}

	const auto *pSrc16 = reinterpret_cast<const uint16_t*>(p);
	auto *pDst16 = reinterpret_cast<uint16_t *>(&sv_RxDmxPrevious[nPortIndex][0]);

	auto isChanged = false;

	for (auto i = 0; i < buffer::SIZE / 2; i++) {
		if (*pDst16 != *pSrc16) {
			*pDst16 = *pSrc16;
			isChanged = true;
		}
		pDst16++;
		pSrc16++;
	}

	return (isChanged ? p : nullptr);
#else
	return nullptr;
#endif
}

const uint8_t *Dmx::GetDmxAvailable([[maybe_unused]] uint32_t nPortIndex)  {
	assert(nPortIndex < dmx::config::max::PORTS);
#if !defined(CONFIG_DMX_TRANSMIT_ONLY)
	if ((sv_RxBuffer[nPortIndex].Dmx.nSlotsInPacket & 0x8000) != 0x8000) {
		return nullptr;
	}

	sv_RxBuffer[nPortIndex].Dmx.nSlotsInPacket &= ~0x8000;
	sv_RxBuffer[nPortIndex].Dmx.nSlotsInPacket--;	// Remove SC from length

#if !defined (CONFIG_DMX_DISABLE_STATISTICS)
	sv_TotalStatistics[nPortIndex].Dmx.Received++;
#endif
	return const_cast<const uint8_t *>(sv_RxBuffer[nPortIndex].Dmx.data);
#else
	return nullptr;
#endif
}

const uint8_t *Dmx::GetDmxCurrentData(const uint32_t nPortIndex) {
	return const_cast<const uint8_t *>(sv_RxBuffer[nPortIndex].Dmx.data);
}

uint32_t Dmx::GetDmxUpdatesPerSecond([[maybe_unused]] uint32_t nPortIndex) {
	assert(nPortIndex < dmx::config::max::PORTS);
#if !defined(CONFIG_DMX_TRANSMIT_ONLY)
	return sv_nRxDmxPackets[nPortIndex].nPerSecond;
#else
	return 0;
#endif
}

// RDM Send

void Dmx::RdmSendRaw(const uint32_t nPortIndex, const uint8_t* pRdmData, uint32_t nLength) {
	assert(nPortIndex < dmx::config::max::PORTS);
	assert(pRdmData != nullptr);
	assert(nLength != 0);

	const auto nUart = dmx_port_to_uart(nPortIndex);

	while (SET != usart_flag_get(nUart, USART_FLAG_TC))
		;

	switch (nUart) {
#if defined (DMX_USE_USART0)
	case USART0:
		gd32_gpio_mode_output<USART0_GPIOx, USART0_TX_GPIO_PINx>();
		GPIO_BC(USART0_GPIOx) = USART0_TX_GPIO_PINx;
		break;
#endif
#if defined (DMX_USE_USART1)
	case USART1:
		gd32_gpio_mode_output<USART1_GPIOx, USART1_TX_GPIO_PINx>();
		GPIO_BC(USART1_GPIOx) = USART1_TX_GPIO_PINx;
		break;
#endif
#if defined (DMX_USE_USART2)
	case USART2:
		gd32_gpio_mode_output<USART2_GPIOx, USART2_TX_GPIO_PINx>();
		GPIO_BC(USART2_GPIOx) = USART2_TX_GPIO_PINx;
		break;
#endif
#if defined (DMX_USE_UART3)
	case UART3:
		gd32_gpio_mode_output<UART3_GPIOx, UART3_TX_GPIO_PINx>();
		GPIO_BC(UART3_GPIOx) = UART3_TX_GPIO_PINx;
		break;
#endif
#if defined (DMX_USE_UART4)
	case UART4:
		gd32_gpio_mode_output<UART4_TX_GPIOx, UART4_TX_GPIO_PINx>();
		GPIO_BC(UART4_TX_GPIOx) = UART4_TX_GPIO_PINx;
		break;
#endif
#if defined (DMX_USE_USART5)
	case USART5:
		gd32_gpio_mode_output<USART5_GPIOx, USART5_TX_GPIO_PINx>();
		GPIO_BC(USART5_GPIOx) = USART5_TX_GPIO_PINx;
		break;
#endif
#if defined (DMX_USE_UART6)
	case UART6:
		gd32_gpio_mode_output<UART6_GPIOx, UART6_TX_GPIO_PINx>();
		GPIO_BC(UART6_GPIOx) = UART6_TX_GPIO_PINx;
		break;
#endif
#if defined (DMX_USE_UART7)
	case UART7:
		gd32_gpio_mode_output<UART7_GPIOx, UART7_TX_GPIO_PINx>();
		GPIO_BC(UART7_GPIOx) = UART7_TX_GPIO_PINx;
		break;
#endif
	default:
		assert(0);
		__builtin_unreachable();
		break;
	}

	TIMER_CNT(TIMER5) = 0;
	do {
		__DMB();
	} while (TIMER_CNT(TIMER5) < RDM_TRANSMIT_BREAK_TIME);

	switch (nUart) {
#if defined (DMX_USE_USART0)
	case USART0:
		gd32_gpio_mode_af<USART0_GPIOx, USART0_TX_GPIO_PINx, USART0>();
		break;
#endif
#if defined (DMX_USE_USART1)
	case USART1:
		gd32_gpio_mode_af<USART1_GPIOx, USART1_TX_GPIO_PINx, USART1>();
		break;
#endif
#if defined (DMX_USE_USART2)
	case USART2:
		gd32_gpio_mode_af<USART2_GPIOx, USART2_TX_GPIO_PINx, USART2>();
		break;
#endif
#if defined (DMX_USE_UART3)
	case UART3:
		gd32_gpio_mode_af<UART3_GPIOx, UART3_TX_GPIO_PINx, UART3>();
		break;
#endif
#if defined (DMX_USE_UART4)
	case UART4:
		gd32_gpio_mode_af<UART4_TX_GPIOx, UART4_TX_GPIO_PINx, UART4>();
		break;
#endif
#if defined (DMX_USE_USART5)
	case USART5:
		gd32_gpio_mode_af<USART5_GPIOx, USART5_TX_GPIO_PINx, USART5>();
		break;
#endif
#if defined (DMX_USE_UART6)
	case UART6:
		gd32_gpio_mode_af<UART6_GPIOx, UART6_TX_GPIO_PINx, UART6>();
		break;
#endif
#if defined (DMX_USE_UART7)
	case UART7:
		gd32_gpio_mode_af<UART7_GPIOx, UART7_TX_GPIO_PINx, UART7>();
		break;
#endif
	default:
		assert(0);
		__builtin_unreachable();
		break;
	}

	TIMER_CNT(TIMER5) = 0;
	do {
		__DMB();
	} while (TIMER_CNT(TIMER5) < RDM_TRANSMIT_MAB_TIME);

	for (uint32_t i = 0; i < nLength; i++) {
		while (RESET == usart_flag_get(nUart, USART_FLAG_TBE))
			;
		USART_TDATA(nUart) = USART_TDATA_TDATA & pRdmData[i];
	}

	while (SET != usart_flag_get(nUart, USART_FLAG_TC)) {
		static_cast<void>(GET_BITS(USART_RDATA(nUart), 0U, 8U));
	}

#if !defined (CONFIG_DMX_DISABLE_STATISTICS)
	sv_TotalStatistics[nPortIndex].Rdm.Sent.Class++;
#endif
}

void Dmx::RdmSendDiscoveryRespondMessage(uint32_t nPortIndex, const uint8_t *pRdmData, uint32_t nLength) {
	assert(nPortIndex < dmx::config::max::PORTS);
	assert(pRdmData != nullptr);
	assert(nLength != 0);

	// 3.2.2 Responder Packet spacing
	udelay(RDM_RESPONDER_PACKET_SPACING, gsv_RdmDataReceiveEnd);

	SetPortDirection(nPortIndex, dmx::PortDirection::OUTP, false);

	const auto nUart = dmx_port_to_uart(nPortIndex);

	for (uint32_t i = 0; i < nLength; i++) {
		while (RESET == usart_flag_get(nUart, USART_FLAG_TBE))
			;
		USART_TDATA(nUart) = USART_TDATA_TDATA & pRdmData[i];
	}

	while (SET != usart_flag_get(nUart, USART_FLAG_TC)) {
		static_cast<void>(GET_BITS(USART_RDATA(nUart), 0U, 8U));
	}

	TIMER_CNT(TIMER5) = 0;
	do {
		__DMB();
	} while (TIMER_CNT(TIMER5) < RDM_RESPONDER_DATA_DIRECTION_DELAY);


	SetPortDirection(nPortIndex, dmx::PortDirection::INP, true);

#if !defined (CONFIG_DMX_DISABLE_STATISTICS)
	sv_TotalStatistics[nPortIndex].Rdm.Sent.DiscoveryResponse++;
#endif
}

// RDM Receive

const uint8_t *Dmx::RdmReceive(const uint32_t nPortIndex) {
	assert(nPortIndex < dmx::config::max::PORTS);

	if ((sv_RxBuffer[nPortIndex].Rdm.nIndex & 0x4000) != 0x4000) {
		return nullptr;
	}

	sv_RxBuffer[nPortIndex].Rdm.nIndex = 0;

	const auto *p = const_cast<const uint8_t *>(sv_RxBuffer[nPortIndex].Rdm.data);

	if (p[0] == E120_SC_RDM) {
		const auto *pRdmCommand = reinterpret_cast<const struct TRdmMessage *>(p);

		uint32_t i;
		uint16_t nChecksum = 0;

		for (i = 0; i < 24; i++) {
			nChecksum = static_cast<uint16_t>(nChecksum + p[i]);
		}

		for (; i < pRdmCommand->message_length; i++) {
			nChecksum = static_cast<uint16_t>(nChecksum + p[i]);
		}

		if (p[i++] == static_cast<uint8_t>(nChecksum >> 8)) {
			if (p[i] == static_cast<uint8_t>(nChecksum)) {
#if !defined (CONFIG_DMX_DISABLE_STATISTICS)
				sv_TotalStatistics[nPortIndex].Rdm.Received.Good++;
#endif
				return p;
			}
		}
#if !defined (CONFIG_DMX_DISABLE_STATISTICS)
		sv_TotalStatistics[nPortIndex].Rdm.Received.Bad++;
#endif
		return nullptr;
	} else {
#if !defined (CONFIG_DMX_DISABLE_STATISTICS)
		sv_TotalStatistics[nPortIndex].Rdm.Received.DiscoveryResponse++;
#endif
	}

	return p;
}

const uint8_t *Dmx::RdmReceiveTimeOut(const uint32_t nPortIndex, uint16_t nTimeOut) {
	assert(nPortIndex < dmx::config::max::PORTS);

	uint8_t *p = nullptr;
	TIMER_CNT(TIMER5) = 0;

	do {
		if ((p = const_cast<uint8_t *>(RdmReceive(nPortIndex))) != nullptr) {
			return p;
		}
	} while (TIMER_CNT(TIMER5) < nTimeOut);

	return nullptr;
}
