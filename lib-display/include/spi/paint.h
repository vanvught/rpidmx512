/**
 * @file paint.h
 *
 */
/* Copyright (C) 2022-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef SPI_PAINT_H
#define SPI_PAINT_H

#include <cstdint>
#include <cstdlib>

#include "spi/lcd_font.h"
#include "spi/spilcd.h"

#include "spi/config.h"

#include "debug.h"

class Paint : public SpiLcd {
public:
	Paint(uint32_t nCS) : SpiLcd(nCS) {
		DEBUG_ENTRY
		DEBUG_EXIT
	}
	virtual ~Paint() {};

	uint32_t GetWidth() const {
		return m_nWidth;
	}

	uint32_t GetHeight() const {
		return m_nHeight;
	}

	void FillColour(const uint16_t nColour) {
		DEBUG_PRINTF("nColour=0x%x", nColour);

		SetAddressWindow(0, 0, m_nWidth - 1, m_nHeight - 1);

		FillFramebuffer(nColour);

		ClearCS();
		SetDC();

		for (uint32_t i = 0; i < config::HEIGHT / FRAME_BUFFER_ROWS; i++) {
			WriteDataContinue(reinterpret_cast<uint8_t *>(s_FrameBuffer), sizeof(s_FrameBuffer));
		}

		SetCS();
	}

	void Fill(uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1, uint16_t nColour) {
		if (!(x0 < m_nWidth && (y0 < m_nHeight) && (x1 < m_nWidth) && (y1 < m_nHeight))) {
//			DEBUG_PRINTF("[%u:%u] %u:%u-%u:%u", m_nWidth, m_nHeight, x0, y0, x1, y1);
			return;
		}

		assert(x1 > x0);
		assert(y1 > y0);

		SetAddressWindow(x0, y0, x1, y1);

		FillFramebuffer(nColour);

		auto nPixels = (1 + (y1 - y0)) * (1 + (x1 - x0));
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
				SetCS();
			}
		} else {
			WriteData(reinterpret_cast<uint8_t *>(s_FrameBuffer), nPixels * 2);
		}
	}

	void DrawPixel(const uint32_t x, const uint32_t y, const uint16_t nColour) {
		SetAddressWindow(x, y, x, y);
		WriteDataWord(nColour);
	}

	void DrawChar(uint32_t x0, uint32_t y0, const char nChar, sFONT *pFont, uint16_t nColourBackground, uint16_t nColourForeGround) {
		const auto x1 = x0 + pFont->Width - 1;
		const auto y1 = y0 + pFont->Height - 1;

		SetAddressWindow(x0, y0, x1, y1);

	    nColourForeGround = __builtin_bswap16(nColourForeGround);
	    nColourBackground = __builtin_bswap16(nColourBackground);

	    const auto nCharOffset = (nChar - ' ') * pFont->Height;
	    const auto *ptr = &pFont->table[nCharOffset];

//	    DEBUG_PRINTF("w=%u, h=%u, nCharOffset=%u", pFont->Width , pFont->Height, nCharOffset);

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

	/**
	 * Bresenham
	 */
	//TODO Can be optimized for horizontal and vertical lines
	void DrawLine(uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1, uint16_t nColour) {
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

private:
	virtual void SetAddressWindow(uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1)=0;

private:
	void SetCursor(const uint32_t x, const uint32_t y) {
		SetAddressWindow(x, y, x, y);
	}

	void FillFramebuffer(uint16_t nColour) {
		nColour = __builtin_bswap16(nColour);

		for (uint32_t i = 0; i < sizeof(s_FrameBuffer) / sizeof(s_FrameBuffer[0]); i++) {
			s_FrameBuffer[i] = nColour;
		}
	}

protected:
	uint32_t m_nWidth { config::WIDTH };
	uint32_t m_nHeight { config::HEIGHT };
	uint32_t m_nRotate { 0 };

#if !defined(SPI_LCD_FRAME_BUFFER_ROWS)
 static constexpr uint32_t FRAME_BUFFER_ROWS = 5;
#else
 static constexpr uint32_t FRAME_BUFFER_ROWS = SPI_LCD_FRAME_BUFFER_ROWS;
#endif

	static inline uint16_t s_FrameBuffer[config::WIDTH * FRAME_BUFFER_ROWS];
};

#endif /* SPI_PAINT_H */
