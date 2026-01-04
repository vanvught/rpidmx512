/**
 * @file dmxmonitor.cpp
 *
 */
/* Copyright (C) 2016-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include "dmxmonitor.h"
#include "console.h"

using namespace dmxmonitor;

static constexpr uint32_t kTopRow = 2;
static constexpr uint32_t kHexColumns = 32;
static constexpr uint32_t kHexRows = 16;
static constexpr uint32_t kDecColumns = 24;
static constexpr uint32_t kDecRows = 22;

DmxMonitor::DmxMonitor() {
	assert(s_this == nullptr);
	s_this = this;
	
	memset(data_, 0, sizeof(data_) / sizeof(data_[0]));
}

bool DmxMonitor::SetDmxStartAddress(uint16_t nDmxStartAddress)  {
	if (nDmxStartAddress != dmxnode::kStartAddressDefault) {
		return false;
	}

	dmx_start_address_ = nDmxStartAddress;
	return true;
}

void DmxMonitor::Start([[maybe_unused]] uint32_t nPortIndex) {
	if(started_) {
		return;
	}

	started_ = true;

	auto row = kTopRow;

	ClearLine(kTopRow);

	switch (format_) {
		case Format::kPct:
			Putc('%');
			break;
		case Format::kDec:
			Putc('D');
			break;
		default:
			Putc('H');
			break;
	}

	if (format_ != Format::kDec) {
		Puts("   01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32");

		for (uint32_t i = 1; i < (kHexRows * kHexColumns); i = i + kHexColumns) {
			console::SetCursor(0, ++row);
			printf("%3d", i);
		}
	} else {
		Puts("     1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18  19  20  21  22  23  24");

		for (uint32_t i = 1; i < (kDecRows * kDecColumns); i = i + kDecColumns) {
			console::SetCursor(0, ++row);
			printf("%3d", i);
		}
	}

	Update();
}

void DmxMonitor::Stop([[maybe_unused]] uint32_t nPortIndex) {
	if(!started_) {
		return;
	}

	started_ = false;

	if (format_ != Format::kDec) {
		for (uint32_t i = (kTopRow + 1); i < (kTopRow + kHexRows + 1); i++) {
			console::SetCursor(4, i);
			Puts("-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --");
		}
	} else {
		uint32_t i;
		for (i = (kTopRow + 1); i < (kTopRow + kDecRows); i++) {
			console::SetCursor(4, i);
			Puts("--- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---");
		}
		console::SetCursor(4, i);
		Puts("--- --- --- --- --- --- --- ---");
	}
}

void DmxMonitor::Cls() {
	uint32_t i;

	for (i = kTopRow; i < (kTopRow + kHexRows + 2); i++) {
		ClearLine(i);
	}

	if (format_ == Format::kDec) {
		for (; i < (kTopRow + kDecRows + 2); i++) {
			ClearLine(i);
		}
	}
}

template<bool doUpdate>
void DmxMonitor::SetData([[maybe_unused]] uint32_t nPortIndex, const uint8_t *pData, uint32_t nLength) {
	slots_ = static_cast<uint16_t>(nLength);

	memcpy(data_, pData, nLength);

	if constexpr (doUpdate) {
		Update();
	}
}

void DmxMonitor::Update() {
	auto row = kTopRow;
	uint32_t i, j;
	auto *p = data_;
	uint16_t nSlot = 0;

	if (format_ != Format::kDec) {
		for (i = 0; (i < kHexRows) && (nSlot < slots_); i++) {
			console::SetCursor(4, ++row);

			for (j = 0; (j < kHexColumns) && (nSlot < slots_); j++) {
				const auto d = *p++;

				if (d == 0) {
					Puts(" 0");
				} else {
					if (format_ == Format::kHex) {
						PuthexFgBg(d, (d > 92 ? CONSOLE_BLACK : console::Colours::kConsoleWhite), RGB(d, d, d));
					} else {
						PutpctFgBg(
								static_cast<uint8_t>((d * 100) / 255),
								(d > 92 ? CONSOLE_BLACK : console::Colours::kConsoleWhite),
								RGB(d, d, d));
					}
				}
				Putc(' ');
				nSlot++;
			}

			for (; j < kHexColumns; j++) {
				Puts("   ");
			}
		}

		for (; i < kHexRows; i++) {
			console::SetCursor(4, ++row);
			Puts("                                                                                               ");
		}
	} else {
		for (i = 0; (i < kDecRows) && (nSlot < slots_); i++) {

			console::SetCursor(4, ++row);

			for (j = 0; (j < kDecColumns) && (nSlot < slots_); j++) {
				const uint8_t d = *p++;

				if (d == 0) {
					Puts("  0");
				} else {
					Put3decFgBg(d, (d > 92 ? CONSOLE_BLACK : console::Colours::kConsoleWhite), RGB(d, d, d));
				}
				Putc(' ');
				nSlot++;
			}

			for (; j < kDecColumns; j++) {
				Puts("    ");
			}
		}

		for (; i < kDecRows; i++) {
			console::SetCursor(4, ++row);
			Puts("                                                                                               ");
		}
	}
}

// Explicit template instantiations
template void DmxMonitor::SetData<true>(const uint32_t, const uint8_t *, uint32_t);
template void DmxMonitor::SetData<false>(const uint32_t, const uint8_t *, uint32_t);
