/**
 * @file hub75bdisplay.cpp
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
/**
 * PoC
 */

#include <cassert>
#include <algorithm>
#include <stdint.h>
#include <stdio.h>

#include "hub75bdisplay.h"

#include "h3_spi.h"
#include "h3_gpio.h"
#include "board/h3_opi_zero.h"

/*
 * FPS
 */
#include "irq_timer.h"
#include "h3_timer.h"
#include "arm/arm.h"
#include "arm/synchronize.h"

#include "debug.h"

// Orange Pi Zero only

#define HUB75B_A		GPIO_EXT_13		// PA0
#define HUB75B_B		GPIO_EXT_11		// PA1
#define HUB75B_C		GPIO_EXT_22		// PA2
#define HUB75B_D		GPIO_EXT_15		// PA3

#define HUB75B_CK		GPIO_EXT_26		// PA10
#define HUB75B_LA		GPIO_EXT_7		// PA6
#define HUB75B_OE		GPIO_EXT_12		// PA7

#define HUB75B_R1		GPIO_EXT_24		// PA13
#define HUB75B_G1		GPIO_EXT_23		// PA14
#define HUB75B_B1		GPIO_EXT_19		// PA15

#define HUB75B_R2		GPIO_EXT_21		// PA16
#define HUB75B_G2		GPIO_EXT_18		// PA18
#define HUB75B_B2		GPIO_EXT_16		// PA19

namespace hub75b {
static constexpr auto PWM_WIDTH = 16;
}  // namespace hub75b

/**
 * Timer 1
 * FPS
 */

static volatile uint32_t updates_per_seconde;
static uint32_t updates_per_seconde_previous;
static uint32_t updates_counter;

static void irq_timer1(__attribute__((unused)) uint32_t clo) {
	dmb();

	updates_per_seconde = updates_counter - updates_per_seconde_previous;
	updates_per_seconde_previous = updates_counter;

	dmb();
}

using namespace hub75b;

Hub75bDisplay::Hub75bDisplay(uint32_t nColumns, uint32_t nRows): m_nColumns(nColumns), m_nRows(nRows) {
	h3_spi_end();

	h3_gpio_fsel(HUB75B_CK, GPIO_FSEL_OUTPUT);
	h3_gpio_fsel(HUB75B_LA, GPIO_FSEL_OUTPUT);
	h3_gpio_fsel(HUB75B_OE, GPIO_FSEL_OUTPUT);

	h3_gpio_clr(HUB75B_CK);
	h3_gpio_clr(HUB75B_LA);
	h3_gpio_set(HUB75B_OE);

	h3_gpio_fsel(HUB75B_A, GPIO_FSEL_OUTPUT);
	h3_gpio_fsel(HUB75B_B, GPIO_FSEL_OUTPUT);
	h3_gpio_fsel(HUB75B_C, GPIO_FSEL_OUTPUT);
	h3_gpio_fsel(HUB75B_D, GPIO_FSEL_OUTPUT);

	h3_gpio_clr(HUB75B_A);
	h3_gpio_clr(HUB75B_B);
	h3_gpio_clr(HUB75B_C);
	h3_gpio_clr(HUB75B_D);

	h3_gpio_fsel(HUB75B_R1, GPIO_FSEL_OUTPUT);
	h3_gpio_fsel(HUB75B_G1, GPIO_FSEL_OUTPUT);
	h3_gpio_fsel(HUB75B_B1, GPIO_FSEL_OUTPUT);

	h3_gpio_clr(HUB75B_R1);
	h3_gpio_clr(HUB75B_G1);
	h3_gpio_clr(HUB75B_B1);

	h3_gpio_fsel(HUB75B_R2, GPIO_FSEL_OUTPUT);
	h3_gpio_fsel(HUB75B_G2, GPIO_FSEL_OUTPUT);
	h3_gpio_fsel(HUB75B_B2, GPIO_FSEL_OUTPUT);

	h3_gpio_clr(HUB75B_R2);
	h3_gpio_clr(HUB75B_G2);
	h3_gpio_clr(HUB75B_B2);

	__disable_irq();
	irq_timer_init();
	irq_timer_set(IRQ_TIMER_1, irq_timer1);

	const uint32_t nBufferSize = m_nColumns * m_nRows * PWM_WIDTH;
	DEBUG_PRINTF("nBufferSize=%u", nBufferSize);

	m_pFramebuffer = new uint32_t[nBufferSize];
	assert(m_pFramebuffer != nullptr);

	for (uint32_t i = 0; i < nBufferSize; i++) {
		m_pFramebuffer[i] = 0;
	}

	for (uint32_t i = 0; i < sizeof(m_TablePWM); i++) {
		m_TablePWM[i] = (i * PWM_WIDTH) / sizeof(m_TablePWM);
	}

	// LightSet
	m_nLastPortId = (m_nColumns * m_nRows) / 170;
	m_nLastPortDataLength = 3 * ((m_nColumns * m_nRows) - (m_nLastPortId * 170));
}

void Hub75bDisplay::Start() {
	H3_TIMER->TMR1_INTV = 0xB71B00; // 1 second
	H3_TIMER->TMR1_CTRL &= ~(TIMER_CTRL_SINGLE_MODE);
	H3_TIMER->TMR1_CTRL |= (TIMER_CTRL_EN_START | TIMER_CTRL_RELOAD);

	__enable_irq();
	isb();
}

void Hub75bDisplay::Dump() {
	//debug_dump(m_pFramebuffer, (m_nColumns * m_nRows / 2));

	for (uint32_t nRow = 0; nRow < (m_nRows / 2); nRow++) {
		printf("[");
		for (uint32_t i = 0; i < m_nColumns; i++) {
			const uint32_t nIndex = (nRow * m_nColumns) + i;
			printf("%x ", m_pFramebuffer[nIndex]);
		}
		puts("]");
	}
}

uint32_t Hub75bDisplay::GetFps() {
	dmb();
	return updates_per_seconde;
}

void Hub75bDisplay::SetPixel(uint32_t nColumn, uint32_t nRow, uint8_t nRed, uint8_t nGreen, uint8_t nBlue) {
	if (__builtin_expect(((nColumn >= m_nColumns) || (nRow >= m_nRows)), 0)) {
		return;
	}

	if (nRow < (m_nRows / 2)) {
		const uint32_t nBaseIndex = (nRow * m_nColumns * PWM_WIDTH) + nColumn;

		for (uint32_t nPWM = 0; nPWM < PWM_WIDTH; nPWM++) {

			const uint32_t nIndex = nBaseIndex + (nPWM * m_nColumns);

			uint32_t nValue = m_pFramebuffer[nIndex];

			nValue &= ~((1U << HUB75B_R1) | (1U << HUB75B_G1) | (1U << HUB75B_B1));

			if (m_TablePWM[nRed] > nPWM) {
				nValue |= (1U << HUB75B_R1);
			}

			if (m_TablePWM[nGreen] > nPWM) {
				nValue |= (1U << HUB75B_G1);
			}

			if (m_TablePWM[nBlue] > nPWM) {
				nValue |= (1U << HUB75B_B1);
			}

			m_pFramebuffer[nIndex] = nValue;
		}
	} else {
		const uint32_t nBaseIndex = ((nRow - (m_nRows / 2)) * m_nColumns * PWM_WIDTH) + nColumn;

		for (uint32_t nPWM = 0; nPWM < PWM_WIDTH; nPWM++) {

			const uint32_t nIndex = nBaseIndex + (nPWM * m_nColumns);

			uint32_t nValue = m_pFramebuffer[nIndex];
			nValue &= ~((1U << HUB75B_R2) | (1U << HUB75B_G2) | (1U << HUB75B_B2));

			if (m_TablePWM[nRed] > nPWM) {
				nValue |= (1U << HUB75B_R2);
			}

			if (m_TablePWM[nGreen] > nPWM) {
				nValue |= (1U << HUB75B_G2);
			}

			if (m_TablePWM[nBlue] > nPWM) {
				nValue |= (1U << HUB75B_B2);
			}

			m_pFramebuffer[nIndex] = nValue;
		}
	}
}

void Hub75bDisplay::Run() {
	uint32_t nGPIO = H3_PIO_PORTA->DAT & ~((1U << HUB75B_R1) | (1U << HUB75B_G1) | (1U << HUB75B_B1) | (1U << HUB75B_R2) | (1U << HUB75B_G2) | (1U << HUB75B_B2));

	for (uint32_t nRow = 0; nRow < (m_nRows / 2); nRow++) {

		const uint32_t nBaseIndex = (nRow * m_nColumns * PWM_WIDTH);

		for (uint32_t nPWM = 0; nPWM < PWM_WIDTH; nPWM++) {

			uint32_t nIndex = nBaseIndex + (nPWM * m_nColumns);

			/* Shift in next data */
			for (uint32_t i = 0; i < m_nColumns; i++) {
				const uint32_t nValue = m_pFramebuffer[nIndex++];
				// Clock high with data
				H3_PIO_PORTA->DAT = nGPIO | (1U << HUB75B_CK) | nValue;
				// Clock low
				H3_PIO_PORTA->DAT = nGPIO | nValue;
			}

			/* Blank the display */
			H3_PIO_PORTA->DAT = nGPIO | (1U << HUB75B_OE);

			/* Latch the previous data */
			H3_PIO_PORTA->DAT = nGPIO | (1U << HUB75B_LA) | (1U << HUB75B_OE);
			nGPIO |= (1U << HUB75B_OE);
			H3_PIO_PORTA->DAT = nGPIO;

			/* Update the row select */
			nGPIO &= ~(0xFU);
			nGPIO |= nRow;
			H3_PIO_PORTA->DAT = nGPIO;

			/* Enable the display */
			nGPIO &= ~(1U << HUB75B_OE);
			H3_PIO_PORTA->DAT = nGPIO;
		}
	}

	updates_counter++;
}

/**
 * DMX LightSet
 */
void Hub75bDisplay::Print() {
	printf("Universes : 1 to %u-%u\n", 1 + m_nLastPortId, m_nLastPortDataLength);
}

void Hub75bDisplay::SetData(uint8_t nPort, const uint8_t *pData, uint16_t nLength) {
	if (nPort < m_nLastPortId) {
		nLength = std::min(nLength, static_cast<uint16_t>(510));
	} else {
		nLength = std::min(nLength, static_cast<uint16_t>(m_nLastPortDataLength));
	}

	uint32_t nIndex = 0;

	for (uint32_t i = 0; i < nLength; i = i + 3) {

		const uint32_t nPixelIndex = (nPort * 170U) + nIndex++;

		const uint32_t nRow = nPixelIndex / m_nColumns;
		const uint32_t nColumn = nPixelIndex - (nRow * m_nColumns);

		SetPixel(nColumn, nRow, pData[i], pData[i + 1], pData[i + 2]);
	}

	if (nPort == m_nLastPortId) {
		// Swap buffers
	}
}
