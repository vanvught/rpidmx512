/**
 * @file paint.cpp
 *
 */
/* Copyright (C) 2022-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <cassert>

#include "spi/paint.h"
#include "spi/spi_lcd.h"

#include "debug.h"

#if !defined(SPI_LCD_FRAME_BUFFER_ROWS)
 static constexpr uint32_t FRAME_BUFFER_ROWS = 5;
#else
 static constexpr uint32_t FRAME_BUFFER_ROWS = SPI_LCD_FRAME_BUFFER_ROWS;
#endif

static uint16_t s_FrameBuffer[config::WIDTH * FRAME_BUFFER_ROWS];

using namespace spi::lcd;

static void fill_framebuffer(uint16_t nColour) {
	nColour = __builtin_bswap16(nColour);

	for (size_t i = 0; i < sizeof(s_FrameBuffer) / sizeof(s_FrameBuffer[0]); i++) {
		s_FrameBuffer[i] = nColour;
	}
}

Paint::Paint() {
	DEBUG_ENTRY

	DEBUG_EXIT
}

Paint::~Paint() {
	DEBUG_ENTRY

	DEBUG_EXIT
}

void Paint::FillColour(uint16_t nColour) {
	SetAddressWindow(0, 0, m_nWidth - 1, m_nHeight - 1);

	fill_framebuffer(nColour);

	for (uint32_t i = 0; i < config::HEIGHT / FRAME_BUFFER_ROWS; i++) {
		WriteData(reinterpret_cast<uint8_t *>(s_FrameBuffer), sizeof(s_FrameBuffer));
	}
}

void Paint::Fill(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t nColour) {
	if (!(x0 < m_nWidth && (y0 < m_nHeight) && (x1 < m_nWidth) && (y1 < m_nHeight))) {
		DEBUG_PRINTF("[%u:%u] %u:%u-%u:%u", m_nWidth, m_nHeight, x0, y0, x1, y1);
		return;
	}

	assert(x1 > x0);
	assert(y1 > y0);

	SetAddressWindow(x0, y0, x1, y1);

	fill_framebuffer(nColour);

	auto nPixels = static_cast<size_t>((1 + (y1 - y0)) * (1 + (x1 - x0)));
	const auto nBufferSize = sizeof(s_FrameBuffer) / sizeof(s_FrameBuffer[0]);

	if (nPixels > nBufferSize) {
		WriteDataStart(reinterpret_cast<uint8_t *>(s_FrameBuffer), sizeof(s_FrameBuffer));

		nPixels = nPixels - nBufferSize;

		while (nPixels > nBufferSize) {
			WriteDataContinue(reinterpret_cast<uint8_t *>(s_FrameBuffer), sizeof(s_FrameBuffer));
			nPixels = nPixels - nBufferSize;
		}

		if (nPixels > 0) {
			WriteDataEnd(reinterpret_cast<uint8_t *>(s_FrameBuffer), nPixels * 2);
		} else {
			CS_Set();
		}
	} else {
		WriteData(reinterpret_cast<uint8_t *>(s_FrameBuffer), nPixels * 2);
	}
}

void Paint::DrawChar(uint16_t x0, uint16_t y0, const char nChar, sFONT *pFont, uint16_t nColourBackground, uint16_t nColourForeGround) {
	const auto x1 = static_cast<uint16_t>(x0 + pFont->Width - 1);
	const auto y1 = static_cast<uint16_t>(y0 + pFont->Height - 1);

	SetAddressWindow(x0, y0, x1, y1);

    nColourForeGround = __builtin_bswap16(nColourForeGround);
    nColourBackground = __builtin_bswap16(nColourBackground);

    const auto nCharOffset = (nChar - ' ') * pFont->Height;
    const auto *ptr = &pFont->table[nCharOffset];

    DEBUG_PRINTF("w=%u, h=%u, nCharOffset=%u", pFont->Width , pFont->Height, nCharOffset);

    uint32_t nIndex = 0;

    if (pFont->Width == 8) {
    	for (auto nPage = 0; nPage < pFont->Height; nPage++) {
    		auto line = *ptr++;

    		for (auto nColumn = 0; nColumn < pFont->Width; nColumn++) {
    			if ((line & 0x80) != 0) {
    				s_FrameBuffer[nIndex++] = nColourForeGround;
    			} else {
    				s_FrameBuffer[nIndex++] = nColourBackground;
    			}

    			line = line << 1;
    		}
    	}
    } else if (pFont->Width < 16) {
    	for (auto nPage = 0; nPage < pFont->Height; nPage++) {
    		auto line = *ptr++;

    		for (auto nColumn = 0; nColumn < pFont->Width; nColumn++) {
    			if ((line & 0x8000) != 0) {
    				s_FrameBuffer[nIndex++] = nColourForeGround;
    			} else {
    				s_FrameBuffer[nIndex++] = nColourBackground;
    			}

    			line = line << 1;
    		}
    	}
    } else {
    	for (auto nPage = 0; nPage < pFont->Height; nPage++) {
    		auto line = *ptr++;

    		for (auto nColumn = 0; nColumn < pFont->Width; nColumn++) {
    			if ((line & 0x1) != 0) {
    				s_FrameBuffer[nIndex++] = nColourForeGround;
    			} else {
    				s_FrameBuffer[nIndex++] = nColourBackground;
    			}

    			line = line >> 1;
    		}
    	}
    }

 	WriteData(reinterpret_cast<uint8_t *>(s_FrameBuffer), nIndex * 2);
}

void Paint::DrawPixel(uint16_t x, uint16_t y, uint16_t nColour) {
	SetAddressWindow(x, y, x, y);
	WriteData_Word(nColour);
}

/**
 * Bresenham
 */
//TODO Can be optimized for horizontal and vertical lines
void Paint::DrawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t nColour) {
	const auto dx = abs(x1 - x0);
	const auto dy = -abs(y1 - y0);

	const auto sx = x0 < x1 ? 1 : -1;
	const auto sy = y0 < y1 ? 1 : -1;

	auto error = dx + dy;

	while (true) {
		DrawPixel(x0, y0, nColour);

		if ((x0 == x1) && (y0 == y1)) {
			break;
		}

		auto e2 = 2 * error;

		if (e2 >= dy) {
			if (x0 == x1) {
				break;
			}
			error = error + dy;
			x0 = x0 + sx;
		}

		if (e2 <= dx) {
			if (y0 == y1) {
				break;
			}
			error = error + dx;
			y0 = y0 + sy;
		}
	}
}
