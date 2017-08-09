#if 0
#if !defined (__linux__) || !defined (__CYGWIN__)
#define __linux__
#endif
#endif
/**
 * @file dmxmonitor.cpp
 *
 */
/* Copyright (C) 2016, 2017 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#include <stdbool.h>
#include <stdio.h>

#include "dmxmonitor.h"

#if defined (__linux__) || defined (__CYGWIN__)
#include <time.h>
#include <sys/time.h>
#else
#include "console.h"
#define TOP_ROW		3
#endif

DMXMonitor::DMXMonitor(void) : m_bIsStarted(false)
#if defined (__linux__) || defined (__CYGWIN__)
	, m_nMaxChannels(32)
#endif
{

}

DMXMonitor::~DMXMonitor(void) {
	this->Stop();
}

#if defined (__linux__) || defined (__CYGWIN__)
void DMXMonitor::DisplayDateTime(const char *pString) {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	struct tm tm = *localtime(&tv.tv_sec);

	printf("%.2d-%.2d-%.4d %.2d:%.2d:%.2d.%.6ld %s\n", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900, tm.tm_hour, tm.tm_min, tm.tm_sec, tv.tv_usec, pString);
}

void DMXMonitor::SetMaxDmxChannels(const uint16_t nMaxChannels) {
	m_nMaxChannels = nMaxChannels;
}

void DMXMonitor::Start(void) {
	if(m_bIsStarted) {
		return;
	}

	m_bIsStarted = true;
	DisplayDateTime("Start");

}

void DMXMonitor::Stop(void) {
	if(!m_bIsStarted) {
		return;
	}

	m_bIsStarted = false;
	DisplayDateTime("Stop");
}

void DMXMonitor::SetData(const uint8_t nPort, const uint8_t *pData, const uint16_t nLength) {
	struct timeval tv;
	uint8_t *p = (uint8_t *)pData;
	int j;

	gettimeofday(&tv, NULL);
	struct tm tm = *localtime(&tv.tv_sec);

	printf("%.2d-%.2d-%.4d %.2d:%.2d:%.2d.%.6ld DMX %d:%d ", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900, tm.tm_hour, tm.tm_min, tm.tm_sec, tv.tv_usec, nLength, m_nMaxChannels);

	for (j = 0; (j < m_nMaxChannels) && (j < nLength); j++) {
		const uint8_t d = *p++;
		printf("%2x ", d);
	}

	for (; j < m_nMaxChannels; j++) {
		printf("-- ");
	}
	printf("\n");
}
#else
void DMXMonitor::Start(void) {
	if(m_bIsStarted) {
		return;
	}

	m_bIsStarted = true;

	for (int i = TOP_ROW; i < (TOP_ROW + 17); i++) {
		console_clear_line(i);
	}

	console_set_cursor(0, TOP_ROW);
	console_puts("    00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31\n");

	for (int i = 1; i < (int) (16 * 32); i = i + (int) 32) {
		printf("%3d ", i);
		console_puts("-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --\n");
	}
}

void DMXMonitor::Stop(void) {
	if(!m_bIsStarted) {
		return;
	}

	m_bIsStarted = false;

	for (int i = (TOP_ROW + 1); i < (TOP_ROW + 17); i++) {
		console_set_cursor(4, i);
		console_puts("-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --");
	}
}

void DMXMonitor::SetData(const uint8_t nPort, const uint8_t *pData, const uint16_t nLength) {
	uint8_t row = TOP_ROW;
	uint8_t *p = (uint8_t *)pData;
	uint16_t slot = 0;
	uint8_t i, j;

	for (i = 0; (i < 16) && (slot < nLength); i++) {

		console_set_cursor(4, ++row);

		for (j = 0; (j < 32) && (slot < nLength); j++) {
			const uint8_t d = *p++;
			if (d == (uint8_t) 0) {
				console_puts(" 0");
			} else {
				console_puthex_fg_bg(d, (uint16_t)(d > 92 ? CONSOLE_BLACK : CONSOLE_WHITE), (uint16_t)RGB(d,d,d));
			}
			(void) console_putc((int) ' ');
			slot++;
		}

		for (; j < 32; j++) {
			console_puts("-- ");
		}
	}

	for (; i < 16; i++) {
		console_set_cursor(4, ++row);
		console_puts("-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --");
	}

}
#endif
