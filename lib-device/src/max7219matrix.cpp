/**
 * @file max7219matrix.cpp
 *
 */
/* Copyright (C) 2020-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <cassert>
#include <algorithm>

#include "max7219matrix.h"

#include "font_cp437.h"

#include "debug.h"

static uint8_t spi_data[64] __attribute__((aligned(4)));

Max7219Matrix::Max7219Matrix(): m_nFontSize(cp437_font_size()) {
	DEBUG_ENTRY

	m_pFont = new uint8_t[m_nFontSize * 8];
	assert(m_pFont != nullptr);

	auto pDst= m_pFont;

	for (uint32_t i = 0; i < m_nFontSize; i++) {
		for (uint32_t j = 0; j < 8; j++) {
			*pDst++ = Rotate(i, 7 - j);
		}
	}

	DEBUG_EXIT
}

Max7219Matrix::~Max7219Matrix() {
	DEBUG_ENTRY

	delete[] m_pFont;

	DEBUG_EXIT
}

void Max7219Matrix::Init(uint16_t nCount, uint8_t nIntensity) {
	DEBUG_ENTRY

	constexpr uint16_t sf = sizeof(spi_data) / 2;
	m_nCount = std::min(nCount, sf);

	DEBUG_PRINTF("m_nCount=%d", m_nCount);

	WriteAll(max7219::reg::SHUTDOWN, max7219::reg::shutdown::NORMAL_OP);
	WriteAll(max7219::reg::DISPLAY_TEST, 0);
	WriteAll(max7219::reg::DECODE_MODE, 0);
	WriteAll(max7219::reg::SCAN_LIMIT, 7);

	WriteAll(max7219::reg::INTENSITY, nIntensity & 0x0F);

	Max7219Matrix::Cls();

	DEBUG_EXIT
}

void Max7219Matrix::Cls() {
	DEBUG_ENTRY

	WriteAll(max7219::reg::DIGIT0, 0);
	WriteAll(max7219::reg::DIGIT1, 0);
	WriteAll(max7219::reg::DIGIT2, 0);
	WriteAll(max7219::reg::DIGIT3, 0);
	WriteAll(max7219::reg::DIGIT4, 0);
	WriteAll(max7219::reg::DIGIT5, 0);
	WriteAll(max7219::reg::DIGIT6, 0);
	WriteAll(max7219::reg::DIGIT7, 0);

	DEBUG_EXIT
}

void Max7219Matrix::Write(const char *pBuffer, uint16_t nCount) {
	DEBUG_PRINTF("nByte=%d", nCount);

	if (nCount > m_nCount) {
		nCount = m_nCount;
	}

	int32_t k;

	for (uint32_t i = 1; i < 9; i++) {
		k = static_cast<int32_t>(nCount);

		uint16_t j;

		for (j = 0; j < (m_nCount * 2U) - (nCount * 2U); j = static_cast<uint16_t>(j + 2)) {
			spi_data[j] = max7219::reg::NOOP;
			spi_data[j + 1] = 0;
		}

		while (--k >= 0) {
			auto c = static_cast<uint32_t>(pBuffer[k]);

			if (c >= m_nFontSize) {
				c = ' ';
			}

			const auto p = &m_pFont[c * 8];

			spi_data[j++] = static_cast<uint8_t>(i);
			spi_data[j++] = p[i - 1];
		}

		HAL_SPI::Write(reinterpret_cast<const char *>(spi_data), j);
	}
}

void Max7219Matrix::UpdateCharacter(uint32_t nChar, const uint8_t pBytes[8]) {
	if (nChar > m_nFontSize) {
		return;
	}

	auto pFont = &m_pFont[nChar * 8];

	for (uint32_t j = 0; j < 8; j++) {
		uint8_t b = 0;

		for (uint32_t y = 0; y < 8; y++) {
			const auto set = pBytes[y] & (1U << (7U - j));
			b |= static_cast<uint8_t>((set != 0) ? (1U << y) : 0);
		}

		pFont[j] = b;
	}
}

void Max7219Matrix::WriteAll(uint8_t nRegister, uint8_t nData) {
	DEBUG_ENTRY

	for (uint32_t i = 0; i < (m_nCount * 2); i = i + 2) {
		spi_data[i] = nRegister;
		spi_data[i+1] = nData;
	}

	HAL_SPI::Write(reinterpret_cast<const char *>(spi_data), static_cast<uint32_t>(m_nCount * 2));

	DEBUG_EXIT
}

uint8_t Max7219Matrix::Rotate(uint32_t r, uint32_t x) {
	uint8_t nByte = 0;

	for (uint32_t y = 0; y < 8; y++) {
		const auto set = cp437_font[r][y] & (1U << x);
		nByte |= static_cast<uint8_t>((set != 0) ? (1U << y) : 0);
	}

	return nByte;
}
