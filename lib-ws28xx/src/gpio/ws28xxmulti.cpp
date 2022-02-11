/**
 * @file ws28xxmulti.cpp
 *
 */
/* Copyright (C) 2021-2022 by Arjan van Vught mailto:info@gd32-dmx.org
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
#include <cstdio>
#include <cassert>

#include "ws28xxmulti.h"
#include "pixelconfiguration.h"
#include "pixeltype.h"
#include "gpio/pixelmulti_config.h"

#include "gd32.h"

#include "debug.h"

#if defined (GD32F4XX)
# define DMA_PARAMETER_STRUCT				dma_multi_data_parameter_struct
# define DMA_CHMADDR						DMA_CHM0ADDR
# define DMA_MEMORY_TO_PERIPHERAL			DMA_MEMORY_TO_PERIPH
# define DMA_PERIPHERAL_WIDTH_32BIT			DMA_PERIPH_WIDTH_32BIT
# define dma_init							dma_multi_data_mode_init
# define dma_memory_to_memory_disable(x,y)
#else
# define DMA_PARAMETER_STRUCT				dma_parameter_struct
#endif

static const uint32_t s_GPIO_PINs[] = { GPIO_PINx };
static uint16_t s_T0H[(640 * 24)]__attribute__ ((aligned (4)));		//FIXME Use define's

static constexpr uint32_t MASTER_TIMER_PERIOD = (0.00000125f * MASTER_TIMER_CLOCK) - 1U;

static volatile bool sv_isRunning;

extern "C" {
void TIMER4_IRQHandler() { // Slave
	__DMB();
	const auto nIntFlag = TIMER_INTF(TIMER4);

	if ((nIntFlag & TIMER_INT_FLAG_CH0) == TIMER_INT_FLAG_CH0) {
		TIMER_CTL0(TIMER7) &= ~(uint32_t) TIMER_CTL0_CEN;

		TIMER_DMAINTEN(TIMER7) &= (~(uint32_t) (TIMER_DMA_CH0D));
		TIMER_DMAINTEN(TIMER7) &= (~(uint32_t) (TIMER_DMA_CH1D));
		TIMER_DMAINTEN(TIMER7) &= (~(uint32_t) (TIMER_DMA_CH2D));

		GPIO_BC(GPIOx) = GPIO_PINx;

		sv_isRunning = false;
	}

	TIMER_INTF(TIMER4) = (~(uint32_t) nIntFlag);
	__DMB();
}
}

using namespace pixel;

WS28xxMulti *WS28xxMulti::s_pThis = nullptr;

WS28xxMulti::WS28xxMulti(PixelConfiguration& pixelConfiguration) {
	DEBUG_ENTRY

	assert(s_pThis == nullptr);
	s_pThis = this;

	uint32_t nLedsPerPixel;
	pixelConfiguration.Validate(nLedsPerPixel);
	pixelConfiguration.Dump();

	m_Type = pixelConfiguration.GetType();
	m_nCount = pixelConfiguration.GetCount();
	m_Map = pixelConfiguration.GetMap();
	m_nBufSize = m_nCount * nLedsPerPixel;

	m_nBufSize *= 8;

	DEBUG_PRINTF("m_nBufSize=%u [%u]", m_nBufSize, (m_nBufSize + 1024)/1024);

	const auto nT0H = (__builtin_popcount(pixelConfiguration.GetLowCode()) * (MASTER_TIMER_PERIOD + 1)) / 8;
	const auto nT1H = (__builtin_popcount(pixelConfiguration.GetHighCode()) * (MASTER_TIMER_PERIOD + 1)) / 8;

	DEBUG_PRINTF("nT0H=%u, nT1H=%u", nT0H, nT1H);

	/**
	 * GPIO Initialization
	 */

	rcu_periph_clock_enable(RCU_GPIOx);
#if !defined (GD32F4XX)
	gpio_init(GPIOx, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PINx);
#else
    gpio_mode_set(GPIOx, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLDOWN, GPIO_PINx);
    gpio_output_options_set(GPIOx, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PINx);
#endif

	GPIO_BC(GPIOx) = GPIO_PINx;

	/**
	 * BEGIN Timer's
	 *
	 * Timer 7 is Master -> TIMER7_TRGO
	 * Timer 4 is Slave -> ITI3
	 *
	 */

	DEBUG_PRINTF("MASTER_TIMER_PERIOD=%u", MASTER_TIMER_PERIOD);

	timer_parameter_struct timer_initpara;

	// Timer 7 Master

	rcu_periph_clock_enable(RCU_TIMER7);

	timer_deinit(TIMER7);
	TIMER_CNT(TIMER7) = 0;

	timer_initpara.prescaler = 0;
	timer_initpara.alignedmode = TIMER_COUNTER_EDGE;
	timer_initpara.counterdirection = TIMER_COUNTER_UP;
	timer_initpara.period = MASTER_TIMER_PERIOD;	// 1.25 us
	timer_initpara.clockdivision = TIMER_CKDIV_DIV1;

	timer_init(TIMER7, &timer_initpara);

	timer_master_slave_mode_config(TIMER7, TIMER_MASTER_SLAVE_MODE_DISABLE);
	timer_master_output_trigger_source_select(TIMER7, TIMER_TRI_OUT_SRC_CH0);

	timer_channel_output_mode_config(TIMER7, TIMER_CH_0, TIMER_OC_MODE_ACTIVE);
	timer_channel_output_mode_config(TIMER7, TIMER_CH_1, TIMER_OC_MODE_ACTIVE);
	timer_channel_output_mode_config(TIMER7, TIMER_CH_2, TIMER_OC_MODE_ACTIVE);

	timer_channel_output_pulse_value_config(TIMER7, TIMER_CH_0, 1);		// High
	timer_channel_output_pulse_value_config(TIMER7, TIMER_CH_1, nT0H);
	timer_channel_output_pulse_value_config(TIMER7, TIMER_CH_2, nT1H);

	// Timer 4 Slave

	rcu_periph_clock_enable(RCU_TIMER4);

	timer_deinit(TIMER4);
	TIMER_CNT(TIMER4) = 0;

	timer_initpara.prescaler = 0;
	timer_initpara.alignedmode = TIMER_COUNTER_EDGE;
	timer_initpara.counterdirection = TIMER_COUNTER_UP;
	timer_initpara.period = (uint32_t) ~0;
	timer_initpara.clockdivision = TIMER_CKDIV_DIV1;

	timer_init(TIMER4, &timer_initpara);

	timer_master_slave_mode_config(TIMER4, TIMER_MASTER_SLAVE_MODE_DISABLE);
	timer_slave_mode_select(TIMER4, TIMER_SLAVE_MODE_EXTERNAL0);
	timer_input_trigger_source_select(TIMER4, TIMER_SMCFG_TRGSEL_ITI3);

	timer_channel_output_mode_config(TIMER4, TIMER_CH_0, TIMER_OC_MODE_ACTIVE);
	timer_channel_output_pulse_value_config(TIMER4, TIMER_CH_0, m_nBufSize);

	timer_interrupt_enable(TIMER4, TIMER_INT_CH0);

	NVIC_SetPriority(TIMER4_IRQn, 0);
	NVIC_EnableIRQ(TIMER4_IRQn);

	/**
	 * END Timer's
	 */

	/**
	 * START DMA configuration
	 */

	DMA_PARAMETER_STRUCT dma_init_struct;
	rcu_periph_clock_enable(RCU_DMA1);

	// Timer 7 Channel 0, DMA Channel 2
	dma_deinit(DMA1, DMA_CH2);
	dma_init_struct.direction = DMA_MEMORY_TO_PERIPHERAL;
#if !defined (GD32F4XX)
	dma_init_struct.memory_addr = (uint32_t) s_GPIO_PINs;
#else
	dma_init_struct.memory0_addr = (uint32_t) s_GPIO_PINs;
#endif
	dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_DISABLE;
	dma_init_struct.memory_width = DMA_MEMORY_WIDTH_32BIT;
	dma_init_struct.periph_addr = GPIOx + 0x10U;
	dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
	dma_init_struct.periph_width = DMA_PERIPHERAL_WIDTH_32BIT;
	dma_init_struct.priority = DMA_PRIORITY_HIGH;
	dma_init(DMA1, DMA_CH2, &dma_init_struct);
	/* configure DMA mode */
	dma_circulation_disable(DMA1, DMA_CH2);
	dma_memory_to_memory_disable(DMA1, DMA_CH2);

	// Timer 7 Channel 1, DMA Channel 4
	dma_deinit(DMA1, DMA_CH4);
	dma_init_struct.direction = DMA_MEMORY_TO_PERIPHERAL;
#if !defined (GD32F4XX)
	dma_init_struct.memory_addr = (uint32_t) s_T0H;
#else
	dma_init_struct.memory0_addr = (uint32_t) s_T0H;
#endif
	dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
	dma_init_struct.memory_width = DMA_MEMORY_WIDTH_16BIT;
	dma_init_struct.periph_addr = GPIOx + 0x14U;
	dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
	dma_init_struct.periph_width = DMA_PERIPHERAL_WIDTH_32BIT;
	dma_init(DMA1, DMA_CH4, &dma_init_struct);
	/* configure DMA mode */
	dma_circulation_disable(DMA1, DMA_CH4);
	dma_memory_to_memory_disable(DMA1, DMA_CH4);

	// Timer 7 Channel 2, DMA Channel 0
	dma_deinit(DMA1, DMA_CH0);
	dma_init_struct.direction = DMA_MEMORY_TO_PERIPHERAL;
#if !defined (GD32F4XX)
	dma_init_struct.memory_addr = (uint32_t) s_GPIO_PINs;
#else
	dma_init_struct.memory0_addr = (uint32_t) s_GPIO_PINs;
#endif
	dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_DISABLE;
	dma_init_struct.memory_width = DMA_MEMORY_WIDTH_32BIT;
	dma_init_struct.periph_addr = GPIOx + 0x14U;
	dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
	dma_init_struct.periph_width = DMA_PERIPHERAL_WIDTH_32BIT;
	dma_init_struct.priority = DMA_PRIORITY_HIGH;
	dma_init(DMA1, DMA_CH0, &dma_init_struct);
	/* configure DMA mode */
	dma_circulation_disable(DMA1, DMA_CH0);
	dma_memory_to_memory_disable(DMA1, DMA_CH0);

	/**
	 * END DMA configuration
	 */

	DEBUG_EXIT
}

WS28xxMulti::~WS28xxMulti() {
	s_pThis = nullptr;
}

void WS28xxMulti::Print() {
	printf("Pixel parameters\n");
	printf(" Type    : %s [%d] - %s [%d]\n", PixelType::GetType(m_Type), static_cast<int>(m_Type), PixelType::GetMap(m_Map), static_cast<int>(m_Map));
	printf(" Count   : %d\n", m_nCount);
}

void WS28xxMulti::Update() {
	assert(!sv_isRunning);

	sv_isRunning = true;

	timer_disable(TIMER4);
	TIMER_CNT(TIMER4) = 0;

	timer_disable(TIMER7);
	TIMER_CNT(TIMER7) = 0;

	DMA_CHCTL(DMA1, DMA_CH2) &= ~DMA_CHXCTL_CHEN;
	DMA_CHMADDR(DMA1, DMA_CH2) = (uint32_t) s_GPIO_PINs;
	DMA_CHCNT(DMA1, DMA_CH2) = (m_nBufSize & DMA_CHXCNT_CNT);
	DMA_CHCTL(DMA1, DMA_CH2) |= DMA_CHXCTL_CHEN;

	DMA_CHCTL(DMA1, DMA_CH4) &= ~DMA_CHXCTL_CHEN;
	DMA_CHMADDR(DMA1, DMA_CH4) = (uint32_t) s_T0H;
	DMA_CHCNT(DMA1, DMA_CH4) = ((m_nBufSize) & DMA_CHXCNT_CNT);
	DMA_CHCTL(DMA1, DMA_CH4) |= DMA_CHXCTL_CHEN;

	DMA_CHCTL(DMA1, DMA_CH0) &= ~DMA_CHXCTL_CHEN;
	DMA_CHMADDR(DMA1, DMA_CH0) = (uint32_t) s_GPIO_PINs;
	DMA_CHCNT(DMA1, DMA_CH0) = ((m_nBufSize) & DMA_CHXCNT_CNT);
	DMA_CHCTL(DMA1, DMA_CH0) |= DMA_CHXCTL_CHEN;

	timer_dma_enable(TIMER7, TIMER_DMA_CH0D);
	timer_dma_enable(TIMER7, TIMER_DMA_CH1D);
	timer_dma_enable(TIMER7, TIMER_DMA_CH2D);

	timer_enable(TIMER4);
	timer_enable(TIMER7);
}

void WS28xxMulti::Blackout() {
	DEBUG_ENTRY
	do {
		__DMB();
	}	while (sv_isRunning);

	for (uint32_t i = 0;  i < sizeof(s_T0H) / sizeof(s_T0H[0]); i++) {
		s_T0H[i] = GPIO_PINx;
	}

	Update();

	do {
		__DMB();
	}	while (sv_isRunning);
	DEBUG_EXIT
}

bool  WS28xxMulti::IsUpdating() {
	__DMB();
	return sv_isRunning;
}

#pragma GCC push_options
#pragma GCC optimize ("O3")

#define BIT_SET(Addr, Bit) {																						\
	*(volatile uint32_t *) (BITBAND_SRAM_BASE + (((uint32_t)&Addr) - SRAM_BASE) * 32U + (Bit & 0xFF) * 4U) = 0x1;	\
}

#define BIT_CLEAR(Addr, Bit) {																						\
	*(volatile uint32_t *) (BITBAND_SRAM_BASE + (((uint32_t)&(Addr)) - SRAM_BASE) * 32U + (Bit & 0xFF) * 4U) = 0x0;	\
}

void WS28xxMulti::SetColour(uint32_t nPortIndex, uint32_t nPixelIndex, uint8_t nColour1, uint8_t nColour2, uint8_t nColour3) {
	assert(nPortIndex < 8);
	assert(nPixelIndex < m_nBufSize / 8);

	uint32_t j = 0;
	const auto k = nPixelIndex * pixel::single::RGB;
	const auto nBit = nPortIndex + GPIO_PIN_OFFSET;

	assert(nBit < 8);

	auto *p = &s_T0H[k];

	for (uint8_t mask = 0x80; mask != 0; mask = static_cast<uint8_t>(mask >> 1)) {
		if (!(mask & nColour1)) {
			BIT_SET(p[j], nBit);
		} else {
			BIT_CLEAR(p[j], nBit);
		}
		if (!(mask & nColour2)) {
			BIT_SET(p[8 + j], nBit);
		} else {
			BIT_CLEAR(p[8 + j], nBit);
		}
		if (!(mask & nColour3)) {
			BIT_SET(p[16 + j], nBit);
		} else {
			BIT_CLEAR(p[16 + j], nBit);
		}

		j++;
	}
}

void WS28xxMulti::SetPixel(uint32_t nPortIndex, uint32_t nPixelIndex, uint8_t nRed, uint8_t nGreen, uint8_t nBlue) {
	assert(nPortIndex < 8);
	assert(nPixelIndex < m_nBufSize / 8);

	switch (m_Map) {
	case Map::RGB:
		SetColour(nPortIndex, nPixelIndex, nRed, nGreen, nBlue);
		break;
	case Map::RBG:
		SetColour(nPortIndex, nPixelIndex, nRed, nBlue, nGreen);
		break;
	case Map::GRB:
		SetColour(nPortIndex, nPixelIndex, nGreen, nRed, nBlue);
		break;
	case Map::GBR:
		SetColour(nPortIndex, nPixelIndex, nGreen, nBlue, nRed);
		break;
	case Map::BRG:
		SetColour(nPortIndex, nPixelIndex, nBlue, nRed, nGreen);
		break;
	case Map::BGR:
		SetColour(nPortIndex, nPixelIndex, nBlue, nGreen, nRed);
		break;
	default:
		assert(0);
		__builtin_unreachable();
		break;
	}

	return;
}

void WS28xxMulti::SetPixel(uint32_t nPortIndex, uint32_t nPixelIndex, uint8_t nRed, uint8_t nGreen, uint8_t nBlue, uint8_t nWhite) {
	assert(nPortIndex < 8);
	assert(nPixelIndex < m_nBufSize / 8);

	uint32_t j = 0;
	const auto k = nPixelIndex * pixel::single::RGBW;
	const auto nBit = nPortIndex + GPIO_PIN_OFFSET;

	assert(nBit < 8);

	auto *p = &s_T0H[k];

	for (uint8_t mask = 0x80; mask != 0; mask = static_cast<uint8_t>(mask >> 1)) {
		// GRBW
		if (!(mask & nGreen)) {
			BIT_SET(p[j], nBit);
		} else {
			BIT_CLEAR(p[j], nBit);
		}

		if (!(mask & nRed)) {
			BIT_SET(p[8 + j], nBit);
		} else {
			BIT_CLEAR(p[8 + j], nBit);
		}

		if (!(mask & nBlue)) {
			BIT_SET(p[16 + j], nBit);
		} else {
			BIT_CLEAR(p[16 + j], nBit);
		}

		if (!(mask & nWhite)) {
			BIT_SET(p[24 + j], nBit);
		} else {
			BIT_CLEAR(p[24 + j], nBit);
		}

		j++;
	}
}
