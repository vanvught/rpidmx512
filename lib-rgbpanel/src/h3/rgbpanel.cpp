/**
 * @file rgbpanel.cpp
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

#if !defined(ORANGE_PI)
# error  Orange Pi Zero only
#endif

#include <cassert>
#include <algorithm>
#include <stdint.h>
#include <stdio.h>

#include "rgbpanel.h"

#include "h3_spi.h"
#include "h3_gpio.h"
#include "board/h3_opi_zero.h"
#include "h3_cpu.h"
#include "h3_smp.h"

#include "arm/synchronize.h"

#include "debug.h"

extern "C" {
void core1_task();
}

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

static uint32_t s_nColumns __attribute__ ((aligned (64)));
static uint32_t s_nRows ;
static uint32_t s_nBufferSize ;
static uint32_t s_nShowCounter ;
//
static volatile bool s_bDoSwap;
static volatile uint32_t s_nUpdatesCounter;
//
static uint32_t *s_pFramebuffer1 ;
static uint32_t *s_pFramebuffer2 ;
static uint8_t *s_pTablePWM ;
//
static bool s_bIsCoreRunning;

using namespace rgbpanel;

void RgbPanel::PlatformInit() {
	h3_cpu_off(H3_CPU2);
	h3_cpu_off(H3_CPU3);

	s_nColumns = m_nColumns;
	s_nRows = m_nRows;
	s_nShowCounter = 0;
	s_bDoSwap = false;
	s_nUpdatesCounter = 0;
	s_bIsCoreRunning = false;

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

	s_nBufferSize = m_nColumns * m_nRows * PWM_WIDTH;
	DEBUG_PRINTF("nBufferSize=%u", s_nBufferSize);

	s_pFramebuffer1 = new uint32_t[s_nBufferSize];
	assert(s_pFramebuffer1 != nullptr);

	s_pFramebuffer2 = new uint32_t[s_nBufferSize];
	assert(s_pFramebuffer2 != nullptr);

	DEBUG_PRINTF("%p %p", s_pFramebuffer1, s_pFramebuffer2);

	for (uint32_t i = 0; i < s_nBufferSize; i++) {
		s_pFramebuffer1[i] = 0;
		s_pFramebuffer2[i] = 0;
	}

	s_pTablePWM = new uint8_t[256];
	assert(s_pTablePWM != nullptr);

	for (uint32_t i = 0; i < 256; i++) {
		s_pTablePWM[i] = (i * PWM_WIDTH) / 255;
	}
}

void RgbPanel::PlatformCleanUp() {
	delete[] s_pFramebuffer1;
	delete[] s_pFramebuffer2;
	delete[] s_pTablePWM;
}

void RgbPanel::Start() {
	if (m_bIsStarted) {
		return;
	}

	m_bIsStarted = true;

	/**
	 * Currently it is not possible stop/starting the additionals core(s)
	 * We need to keep the additional core(s) running.
	 * Starting an already running core can crash the system.
	 */

	if (s_bIsCoreRunning) {
		return;
	}

	puts("smp_start_core(1, core1_task)");
	smp_start_core(1, core1_task);
	puts("Running");
	s_bIsCoreRunning = true;
}

void RgbPanel::Dump() {
	for (uint32_t nRow = 0; nRow < (m_nRows / 2); nRow++) {
		printf("[");
		for (uint32_t i = 0; i < m_nColumns; i++) {
			const uint32_t nIndex = (nRow * m_nColumns) + i;
			printf("%x ", s_pFramebuffer1[nIndex]);
		}
		puts("]");
	}
}

void RgbPanel::Cls() {
	auto lp = reinterpret_cast<uint64_t*>(s_pFramebuffer1);
	uint32_t n = s_nBufferSize * 4;

	while ((n / 8) > 0) {
		*(lp++) = 0;
		n -= 8;
	}

	m_nPosition = 0;
	m_nLine = 0;
}

#pragma GCC push_options
#pragma GCC optimize ("O3")

uint32_t RgbPanel::GetShowCounter() {
	dmb();
	return s_nShowCounter;
}

uint32_t RgbPanel::GetUpdatesCounter() {
	dmb();
	return s_nUpdatesCounter;
}

void RgbPanel::SetPixel(uint32_t nColumn, uint32_t nRow, uint8_t nRed, uint8_t nGreen, uint8_t nBlue) {
	if (__builtin_expect(((nColumn >= m_nColumns) || (nRow >= m_nRows)), 0)) {
		return;
	}

	if (nRow < (m_nRows / 2)) {
		const uint32_t nBaseIndex = (nRow * m_nColumns * PWM_WIDTH) + nColumn;

		for (uint32_t nPWM = 0; nPWM < PWM_WIDTH; nPWM++) {

			const uint32_t nIndex = nBaseIndex + (nPWM * m_nColumns);

			uint32_t nValue = s_pFramebuffer1[nIndex];

			nValue &= ~((1U << HUB75B_R1) | (1U << HUB75B_G1) | (1U << HUB75B_B1));

			if (s_pTablePWM[nRed] > nPWM) {
				nValue |= (1U << HUB75B_R1);
			}

			if (s_pTablePWM[nGreen] > nPWM) {
				nValue |= (1U << HUB75B_G1);
			}

			if (s_pTablePWM[nBlue] > nPWM) {
				nValue |= (1U << HUB75B_B1);
			}

			s_pFramebuffer1[nIndex] = nValue;
		}
	} else {
		const uint32_t nBaseIndex = ((nRow - (m_nRows / 2)) * m_nColumns * PWM_WIDTH) + nColumn;

		for (uint32_t nPWM = 0; nPWM < PWM_WIDTH; nPWM++) {

			const uint32_t nIndex = nBaseIndex + (nPWM * m_nColumns);

			uint32_t nValue = s_pFramebuffer1[nIndex];
			nValue &= ~((1U << HUB75B_R2) | (1U << HUB75B_G2) | (1U << HUB75B_B2));

			if (s_pTablePWM[nRed] > nPWM) {
				nValue |= (1U << HUB75B_R2);
			}

			if (s_pTablePWM[nGreen] > nPWM) {
				nValue |= (1U << HUB75B_G2);
			}

			if (s_pTablePWM[nBlue] > nPWM) {
				nValue |= (1U << HUB75B_B2);
			}

			s_pFramebuffer1[nIndex] = nValue;
		}
	}
}

void RgbPanel::Show() {
	do {
		dmb();
	} while (s_bDoSwap);

	s_bDoSwap = true;
	s_nShowCounter++;
}

void core1_task() {
	const uint32_t nMultiplier = s_nColumns * PWM_WIDTH;

	uint32_t nGPIO = H3_PIO_PORTA->DAT & ~((1U << HUB75B_R1) | (1U << HUB75B_G1) | (1U << HUB75B_B1) | (1U << HUB75B_R2) | (1U << HUB75B_G2) | (1U << HUB75B_B2));

	for (;;) {
		for (uint32_t nRow = 0; nRow < (s_nRows / 2); nRow++) {

			const uint32_t nBaseIndex = nRow * nMultiplier;

			for (uint32_t nPWM = 0; nPWM < PWM_WIDTH; nPWM++) {

				uint32_t nIndex = nBaseIndex + (nPWM * s_nColumns);

				/* Shift in next data */
				for (uint32_t i = 0; i < s_nColumns; i++) {
					const uint32_t nValue = s_pFramebuffer2[nIndex++];
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

		s_nUpdatesCounter++;

		if (s_bDoSwap) {
			auto pTmp = s_pFramebuffer1;
			s_pFramebuffer1 = s_pFramebuffer2;
			s_pFramebuffer2 = pTmp;
			dmb();
			s_bDoSwap = false;
		}
	}
}
