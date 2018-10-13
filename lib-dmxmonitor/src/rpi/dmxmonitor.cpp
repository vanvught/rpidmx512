/**
 * @file dmxmonitor.cpp
 *
 */
/* Copyright (C) 2016-2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#include "console.h"

#define TOP_ROW			3

enum {
	DMX_FOOTPRINT = 512,
	DMX_START_ADDRESS = 1
};

DMXMonitor::DMXMonitor(void) : m_nSlots(0), m_bIsStarted(false) {
	for (int i = 0; i < (int) (sizeof(m_Data) / sizeof(m_Data[0])); i++) {
		m_Data[i] = 0;
	}
}

DMXMonitor::~DMXMonitor(void) {
	this->Stop();
}

bool DMXMonitor::SetDmxStartAddress(uint16_t nDmxStartAddress) {
	if (nDmxStartAddress != DMX_START_ADDRESS) {
		return false;
	}

	return true;
}

uint16_t DMXMonitor::GetDmxStartAddress(void) {
	return DMX_START_ADDRESS;
}

uint16_t DMXMonitor::GetDmxFootprint(void) {
	return DMX_FOOTPRINT;
}

void DMXMonitor::Start(uint8_t nPort) {
	if(m_bIsStarted) {
		return;
	}

	m_bIsStarted = true;

	uint8_t row = TOP_ROW;

	console_clear_line(TOP_ROW);
	console_puts("    01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32");

	for (int i = 1; i < (int) (16 * 32); i = i + (int) 32) {
		console_set_cursor(0, ++row);
		printf("%3d", i);
	}

	Update();
}

void DMXMonitor::Stop(uint8_t nPort) {
	if(!m_bIsStarted) {
		return;
	}

	m_bIsStarted = false;

	for (unsigned i = (TOP_ROW + 1); i < (TOP_ROW + 17); i++) {
		console_set_cursor(4, i);
		console_puts("-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --");
	}
}

void DMXMonitor::Cls(void) {
	for (int i = TOP_ROW; i < (TOP_ROW + 17); i++) {
		console_clear_line(i);
	}
}

void DMXMonitor::SetData(uint8_t nPort, const uint8_t *pData, uint16_t nLength) {
	m_nSlots = nLength;

	for (unsigned i = 0 ; i < nLength; i++) {
		m_Data[i] = pData[i];
	}

	Update();
}

void DMXMonitor::Update(void) {
	unsigned row = TOP_ROW;
	unsigned i, j;
	uint8_t *p = (uint8_t *)m_Data;
	uint16_t slot = 0;

	for (i = 0; (i < 16) && (slot < m_nSlots); i++) {

		console_set_cursor(4, ++row);

		for (j = 0; (j < 32) && (slot < m_nSlots); j++) {
			const uint8_t d = *p++;
			if (d == (uint8_t) 0) {
				console_puts(" 0");
			} else {
				console_puthex_fg_bg(d, (uint16_t) (d > 92 ? CONSOLE_BLACK : CONSOLE_WHITE), (uint16_t) RGB(d, d, d));
			}
			console_putc((int) ' ');
			slot++;
		}

		for (; j < 32; j++) {
			console_puts("   ");
		}
	}

	for (; i < 16; i++) {
		console_set_cursor(4, ++row);
		console_puts("                                                                                               ");
	}
}

