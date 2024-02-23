/**
 * @file dmxmonitor.cpp
 *
 */
/* Copyright (C) 2016-2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include "dmxmonitor.h"
#include "console.h"

using namespace dmxmonitor;
using namespace lightset;

static constexpr uint32_t TOP_ROW = 2;
static constexpr uint32_t HEX_COLUMNS = 32;
static constexpr uint32_t HEX_ROWS = 16;
static constexpr uint32_t DEC_COLUMNS = 24;
static constexpr uint32_t DEC_ROWS = 22;

DMXMonitor::DMXMonitor(void) {
	memset(m_Data, 0, sizeof(m_Data) / sizeof(m_Data[0]));
}

bool DMXMonitor::SetDmxStartAddress(uint16_t nDmxStartAddress)  {
	if (nDmxStartAddress != lightset::dmx::START_ADDRESS_DEFAULT) {
		return false;
	}

	m_nDmxStartAddress = nDmxStartAddress;
	return true;
}

void DMXMonitor::Start([[maybe_unused]] uint32_t nPortIndex) {
	if(m_bIsStarted) {
		return;
	}

	m_bIsStarted = true;

	auto row = TOP_ROW;

	console_clear_line(TOP_ROW);

	switch (m_Format) {
		case Format::PCT:
			console_putc('%');
			break;
		case Format::DEC:
			console_putc('D');
			break;
		default:
			console_putc('H');
			break;
	}

	if (m_Format != Format::DEC) {
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

void DMXMonitor::Stop([[maybe_unused]] uint32_t nPortIndex) {
	if(!m_bIsStarted) {
		return;
	}

	m_bIsStarted = false;

	if (m_Format != Format::DEC) {
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

void DMXMonitor::Cls() {
	uint32_t i;

	for (i = TOP_ROW; i < (TOP_ROW + HEX_ROWS + 2); i++) {
		console_clear_line(i);
	}

	if (m_Format == Format::DEC) {
		for (; i < (TOP_ROW + DEC_ROWS + 2); i++) {
			console_clear_line(i);
		}
	}
}

void DMXMonitor::SetData([[maybe_unused]] uint32_t nPortIndex, const uint8_t *pData, uint32_t nLength, const bool doUpdate) {
	m_nSlots = static_cast<uint16_t>(nLength);

	memcpy(m_Data, pData, nLength);

	if (doUpdate) {
		Update();
	}
}

void DMXMonitor::Update() {
	auto row = TOP_ROW;
	uint32_t i, j;
	auto *p = m_Data;
	uint16_t nSlot = 0;

	if (m_Format != Format::DEC) {
		for (i = 0; (i < HEX_ROWS) && (nSlot < m_nSlots); i++) {
			console_set_cursor(4, ++row);

			for (j = 0; (j < HEX_COLUMNS) && (nSlot < m_nSlots); j++) {
				const auto d = *p++;

				if (d == 0) {
					console_puts(" 0");
				} else {
					if (m_Format == Format::HEX) {
						console_puthex_fg_bg(d, (d > 92 ? CONSOLE_BLACK : CONSOLE_WHITE), RGB(d, d, d));
					} else {
						console_putpct_fg_bg(
								static_cast<uint8_t>((d * 100) / 255),
								(d > 92 ? CONSOLE_BLACK : CONSOLE_WHITE),
								RGB(d, d, d));
					}
				}
				console_putc(' ');
				nSlot++;
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
		for (i = 0; (i < DEC_ROWS) && (nSlot < m_nSlots); i++) {

			console_set_cursor(4, ++row);

			for (j = 0; (j < DEC_COLUMNS) && (nSlot < m_nSlots); j++) {
				const uint8_t d = *p++;

				if (d == 0) {
					console_puts("  0");
				} else {
					console_put3dec_fg_bg(d, (d > 92 ? CONSOLE_BLACK : CONSOLE_WHITE), RGB(d, d, d));
				}
				console_putc(' ');
				nSlot++;
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
