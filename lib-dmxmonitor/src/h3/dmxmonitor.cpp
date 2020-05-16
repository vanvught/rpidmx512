/**
 * @file dmxmonitor.cpp
 *
 */
/* Copyright (C) 2016-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <stdio.h>

#include "dmxmonitor.h"
#include "console.h"

#define TOP_ROW			2

#define HEX_COLUMNS		32
#define HEX_ROWS		16

#define DEC_COLUMNS		24
#define DEC_ROWS		22

enum {
	DMX_FOOTPRINT = 512,
	DMX_START_ADDRESS = 1
};

DMXMonitor::DMXMonitor(void) {
	memset(m_Data, 0, sizeof(m_Data) / sizeof(m_Data[0]));
}

DMXMonitor::~DMXMonitor(void) {
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

void DMXMonitor::Start(__attribute__((unused)) uint8_t nPort) {
	if(m_bIsStarted) {
		return;
	}

	m_bIsStarted = true;

	uint8_t row = TOP_ROW;

	console_clear_line(TOP_ROW);

	switch (m_tFormat) {
		case DMXMonitorFormat::DMX_MONITOR_FORMAT_PCT:
			console_putc('%');
			break;
		case DMXMonitorFormat::DMX_MONITOR_FORMAT_DEC:
			console_putc('D');
			break;
		default:
			console_putc('H');
			break;
	}

	if (m_tFormat != DMXMonitorFormat::DMX_MONITOR_FORMAT_DEC) {
		console_puts("   01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32");

		for (uint32_t i = 1; i < (HEX_ROWS * HEX_COLUMNS); i = i + HEX_COLUMNS) {
			console_set_cursor(0, ++row);
			printf("%3d", i);
		}
	} else {
		console_puts("     1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18  19  20  21  22  23  24");

		for (uint32_t i = 1; i < (DEC_ROWS * DEC_COLUMNS); i = i + DEC_COLUMNS) {
			console_set_cursor(0, ++row);
			printf("%3d", i);
		}
	}

	Update();
}

void DMXMonitor::Stop(__attribute__((unused)) uint8_t nPort) {
	if(!m_bIsStarted) {
		return;
	}

	m_bIsStarted = false;

	if (m_tFormat != DMXMonitorFormat::DMX_MONITOR_FORMAT_DEC) {
		for (uint32_t i = (TOP_ROW + 1); i < (TOP_ROW + HEX_ROWS + 1); i++) {
			console_set_cursor(4, i);
			console_puts("-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --");
		}
	} else {
		uint32_t i;
		for (i = (TOP_ROW + 1); i < (TOP_ROW + DEC_ROWS); i++) {
			console_set_cursor(4, i);
			console_puts("--- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---");
		}
		console_set_cursor(4, i);
		console_puts("--- --- --- --- --- --- --- ---");
	}
}

void DMXMonitor::Cls(void) {
	uint32_t i;

	for (i = TOP_ROW; i < (TOP_ROW + HEX_ROWS + 1); i++) {
		console_clear_line(i);
	}

	if (m_tFormat == DMXMonitorFormat::DMX_MONITOR_FORMAT_DEC) {
		for (; i < (TOP_ROW + DEC_ROWS + 1); i++) {
			console_clear_line(i);
		}
	}
}

void DMXMonitor::SetData(__attribute__((unused)) uint8_t nPort, const uint8_t *pData, uint16_t nLength) {
	m_nSlots = nLength;

	memcpy(m_Data, pData, nLength);

	Update();
}

void DMXMonitor::Update(void) {
	uint32_t row = TOP_ROW;
	uint32_t i, j;
	uint8_t *p = m_Data;
	uint16_t slot = 0;

	if (m_tFormat != DMXMonitorFormat::DMX_MONITOR_FORMAT_DEC) {
		for (i = 0; (i < HEX_ROWS) && (slot < m_nSlots); i++) {

			console_set_cursor(4, ++row);

			for (j = 0; (j < HEX_COLUMNS) && (slot < m_nSlots); j++) {
				const uint8_t d = *p++;

				if (d == 0) {
					console_puts(" 0");
				} else {
					if (m_tFormat == DMXMonitorFormat::DMX_MONITOR_FORMAT_HEX) {
						console_puthex_fg_bg(d, (d > 92 ? CONSOLE_BLACK : CONSOLE_WHITE), RGB(d, d, d));
					} else {
						console_putpct_fg_bg(
								(static_cast<uint32_t>(d) * 100) / 255,
								(d > 92 ? CONSOLE_BLACK : CONSOLE_WHITE),
								RGB(d, d, d));
					}
				}
				console_putc(' ');
				slot++;
			}

			for (; j < HEX_COLUMNS; j++) {
				console_puts("   ");
			}
		}

		for (; i < HEX_ROWS; i++) {
			console_set_cursor(4, ++row);
			console_puts("                                                                                               ");
		}
	} else {
		for (i = 0; (i < DEC_ROWS) && (slot < m_nSlots); i++) {

			console_set_cursor(4, ++row);

			for (j = 0; (j < DEC_COLUMNS) && (slot < m_nSlots); j++) {
				const uint8_t d = *p++;

				if (d == 0) {
					console_puts("  0");
				} else {
					console_put3dec_fg_bg(d, (d > 92 ? CONSOLE_BLACK : CONSOLE_WHITE), RGB(d, d, d));
				}
				console_putc(' ');
				slot++;
			}

			for (; j < DEC_COLUMNS; j++) {
				console_puts("    ");
			}
		}

		for (; i < DEC_ROWS; i++) {
			console_set_cursor(4, ++row);
			console_puts("                                                                                               ");
		}
	}
}
