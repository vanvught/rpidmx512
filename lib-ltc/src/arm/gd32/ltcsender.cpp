/**
 * @file ltcsender.cpp
 *
 */
/* Copyright (C) 2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#pragma GCC push_options
#pragma GCC optimize ("O2")
#pragma GCC optimize ("no-tree-loop-distribute-patterns")

#include <cstdint>
#include <cstring>
#include <cassert>

#include "ltcsender.h"
#include "ltc.h"
#include "ltcencoder.h"
#include "timecodeconst.h"

#include "debug.h"

#if defined (GD32F4XX)
// DMA
# define DMA_PARAMETER_STRUCT				dma_single_data_parameter_struct
# define DMA_CHMADDR						DMA_CHM0ADDR
# define DMA_MEMORY_TO_PERIPHERAL			DMA_MEMORY_TO_PERIPH
# define dma_init							dma_single_data_mode_init
# define dma_struct_para_init				dma_single_data_para_struct_init
# define dma_memory_to_memory_disable(x,y)
// GPIO
# define GPIOx_BOP_OFFSET					0x18U;
#else
// DMA
# define DMA_PARAMETER_STRUCT				dma_parameter_struct
# define dma_interrupt_flag_clear(x,y,z)
// GPIO
# define GPIOx_BOP_OFFSET					0x10U;
#endif

#define LTC_OUTPUT_RCU_GPIOx		RCU_GPIOA
#define LTC_OUTPUT_GPIOx			GPIOA
#define LTC_OUTPUT_GPIO_PINx		GPIO_PIN_4
#define LTC_OUTPUT_GPIO_PIN_OFFSET	4U

static uint32_t buffer1[ltc::encoder::BUFFER_SIZE];
static uint32_t buffer2[ltc::encoder::BUFFER_SIZE];

static void gpio_config(){
	rcu_periph_clock_enable(LTC_OUTPUT_RCU_GPIOx);
#if !defined (GD32F4XX)
	gpio_init(GPIOx, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PINx);
#else
    gpio_mode_set(LTC_OUTPUT_GPIOx, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLDOWN, LTC_OUTPUT_GPIO_PINx);
    gpio_output_options_set(LTC_OUTPUT_GPIOx, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, LTC_OUTPUT_GPIO_PINx);
#endif

	GPIO_BC(LTC_OUTPUT_GPIOx) = LTC_OUTPUT_GPIO_PINx;
}

static void dma_config(){
	DMA_PARAMETER_STRUCT dma_init_struct;
	dma_struct_para_init(&dma_init_struct);

    rcu_periph_clock_enable(RCU_DMA0);

    dma_deinit(DMA0, DMA_CH6);
    dma_init_struct.direction = DMA_MEMORY_TO_PERIPH;
#if !defined (GD32F4XX)
	dma_init_struct.memory_width = DMA_MEMORY_WIDTH_32BIT;
#endif
#if !defined (GD32F4XX)
	dma_init_struct.memory_addr = reinterpret_cast<uint32_t>(&buffer1);
#else
	dma_init_struct.memory0_addr = reinterpret_cast<uint32_t>(&buffer1);
#endif
    dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    dma_init_struct.periph_addr = LTC_OUTPUT_GPIOx + GPIOx_BOP_OFFSET;
#if !defined (GD32F4XX)
	dma_init_struct.periph_width = DMA_PERIPHERAL_WIDTH_32BIT;
#else
	dma_init_struct.periph_memory_width = DMA_PERIPH_WIDTH_32BIT;
#endif
    dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
    dma_init_struct.priority = DMA_PRIORITY_ULTRA_HIGH;
    dma_init_struct.circular_mode = DMA_CIRCULAR_MODE_ENABLE;
    dma_init_struct.number = ltc::encoder::FORMAT_SIZE_BITS * 2U;
    dma_init(DMA0, DMA_CH6, &dma_init_struct);

    dma_channel_subperipheral_select(DMA0, DMA_CH6, DMA_SUBPERI2);

    dma_switch_buffer_mode_config(DMA0, DMA_CH6, reinterpret_cast<uint32_t>(&buffer2), DMA_MEMORY_1);
    dma_switch_buffer_mode_enable(DMA0, DMA_CH6, ENABLE);

    dma_channel_enable(DMA0, DMA_CH6);
}

LtcSender *LtcSender::s_pThis;

LtcSender::LtcSender([[maybe_unused]] uint32_t nVolume) {
	DEBUG_ENTRY

	assert(s_pThis == nullptr);
	s_pThis = this;

	gpio_config();
	dma_config();

	platform::ltc::timer3_config();

	DEBUG_EXIT
}

void LtcSender::Start() {
	DEBUG_ENTRY
	//TODO
	DEBUG_EXIT
}

void LtcSender::SetTimeCode(const struct ltc::TimeCode *pLtcSenderTimeCode, bool nExternalClock) {
	LtcEncoder::SetTimeCode(pLtcSenderTimeCode, nExternalClock);
	uint32_t *pDst;

	if ((DMA_CHCTL(DMA0, DMA_CH6)) & DMA_CHXCTL_MBS) {
		pDst = buffer1;
	} else {
		pDst = buffer2;
	}

	LtcEncoder::Encode(pDst);

	if (__builtin_expect((m_nTypePrevious != pLtcSenderTimeCode->nType), 0)) {
		m_nTypePrevious = pLtcSenderTimeCode->nType;
		TIMER_CAR(TIMER3) = ((TimeCodeConst::TMR_INTV[pLtcSenderTimeCode->nType] + 1) / (ltc::encoder::FORMAT_SIZE_BITS * 2U)) - 1U; //TODO Define as constant
		TIMER_CNT(TIMER3) = 0;
		TIMER_CTL0(TIMER3) |= TIMER_CTL0_CEN;
	}
}
