/**
 * @file paint.h
 *
 */
/* Copyright (C) 2022-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef SPI_PAINT_H_
#define SPI_PAINT_H_

#include <cstdint>
#include <cstdlib>
#include <cassert>

#include "spi/lcd_font.h"
#include "spi/spilcd.h"
#include "spi/config.h"
 #include "firmware/debug/debug_debug.h"

class Paint : public SpiLcd {
public:
 explicit Paint(uint32_t cs) : SpiLcd(cs)
 {
     DEBUG_ENTRY();
     DEBUG_EXIT();
 }
    virtual ~Paint() = default;

	uint32_t GetWidth() const {
		return width_;
	}

	uint32_t GetHeight() const {
		return height_;
	}

	void FillColour(uint16_t colour) {
		DEBUG_PRINTF("colour=0x%x", colour);

		SetAddressWindow(0, 0, width_ - 1, height_ - 1);

		FillFramebuffer(colour);

		ClearCS();
		SetDC();

		for (uint32_t i = 0; i < config::kHeight / kFrameBufferRows; i++) {
			WriteDataContinue(reinterpret_cast<uint8_t *>(s_frame_buffer), sizeof(s_frame_buffer));
		}

		SetCS();
	}

	void Fill(uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1, uint16_t colour) {
		if (!(x0 < width_ && (y0 < height_) && (x1 < width_) && (y1 < height_))) {
//			DEBUG_PRINTF("[%u:%u] %u:%u-%u:%u", width_, height_, x0, y0, x1, y1);
			return;
		}

		assert(x1 > x0);
		assert(y1 > y0);

		SetAddressWindow(x0, y0, x1, y1);

		FillFramebuffer(colour);

		auto pixels = (1 + (y1 - y0)) * (1 + (x1 - x0));
		const auto kBufferSize = sizeof(s_frame_buffer) / sizeof(s_frame_buffer[0]);

		if (pixels > kBufferSize) {
			WriteDataStart(reinterpret_cast<uint8_t *>(s_frame_buffer), sizeof(s_frame_buffer));

			pixels = pixels - kBufferSize;

			while (pixels > kBufferSize) {
				WriteDataContinue(reinterpret_cast<uint8_t *>(s_frame_buffer), sizeof(s_frame_buffer));
				pixels = pixels - kBufferSize;
			}

			if (pixels > 0) {
				WriteDataEnd(reinterpret_cast<uint8_t *>(s_frame_buffer), pixels * 2);
			} else {
				SetCS();
			}
		} else {
			WriteData(reinterpret_cast<uint8_t *>(s_frame_buffer), pixels * 2);
		}
	}

	void DrawPixel(uint32_t x, uint32_t y, uint16_t colour) {
		SetAddressWindow(x, y, x, y);
		WriteDataWord(colour);
	}

	void DrawChar(uint32_t x0, uint32_t y0, char c, sFONT *font, uint16_t colour_background, uint16_t colour_fore_ground) {
		const auto kX1 = x0 + font->kWidth - 1;
		const auto kY1 = y0 + font->kHeight - 1;

		SetAddressWindow(x0, y0, kX1, kY1);

	    colour_fore_ground = __builtin_bswap16(colour_fore_ground);
	    colour_background = __builtin_bswap16(colour_background);

	    const auto kCharOffset = (c - ' ') * font->kHeight;
	    const auto *ptr = &font->table[kCharOffset];

//	    DEBUG_PRINTF("w=%u, h=%u, nCharOffset=%u", font->kWidth , font->kHeight, nCharOffset);

	    uint32_t index = 0;

	    if (font->kWidth == 8) {
	    	for (auto page = 0; page < font->kHeight; page++) {
	    		auto line = *ptr++;

	    		for (auto column = 0; column < font->kWidth; column++) {
	    			if ((line & 0x80) != 0) {
	    				s_frame_buffer[index++] = colour_fore_ground;
	    			} else {
	    				s_frame_buffer[index++] = colour_background;
	    			}

	    			line = line << 1;
	    		}
	    	}
	    } else if (font->kWidth < 16) {
	    	for (auto page = 0; page < font->kHeight; page++) {
	    		auto line = *ptr++;

	    		for (auto column = 0; column < font->kWidth; column++) {
	    			if ((line & 0x8000) != 0) {
	    				s_frame_buffer[index++] = colour_fore_ground;
	    			} else {
	    				s_frame_buffer[index++] = colour_background;
	    			}

	    			line = line << 1;
	    		}
	    	}
	    } else {
	    	for (auto page = 0; page < font->kHeight; page++) {
	    		auto line = *ptr++;

	    		for (auto column = 0; column < font->kWidth; column++) {
	    			if ((line & 0x1) != 0) {
	    				s_frame_buffer[index++] = colour_fore_ground;
	    			} else {
	    				s_frame_buffer[index++] = colour_background;
	    			}

	    			line = line >> 1;
	    		}
	    	}
	    }

	    WriteData(reinterpret_cast<uint8_t *>(s_frame_buffer), index * 2);
	}

	/**
	 * Bresenham
	 */
	//TODO (a) Can be optimized for horizontal and vertical lines
	void DrawLine(uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1, uint16_t colour) {
		const auto kDx = abs(x1 - x0);
		const auto kDy = -abs(y1 - y0);

		const auto kSx = x0 < x1 ? 1 : -1;
		const auto kSy = y0 < y1 ? 1 : -1;

		auto error = kDx + kDy;

		while (true) {
			DrawPixel(x0, y0, colour);

			if ((x0 == x1) && (y0 == y1)) {
				break;
			}

			auto e2 = 2 * error;

			if (e2 >= kDy) {
				if (x0 == x1) {
					break;
				}
				error = error + kDy;
				x0 = x0 + kSx;
			}

			if (e2 <= kDx) {
				if (y0 == y1) {
					break;
				}
				error = error + kDx;
				y0 = y0 + kSy;
			}
		}
	}

private:
	virtual void SetAddressWindow(uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1)=0;

private:
	void SetCursor(uint32_t x, uint32_t y) {
		SetAddressWindow(x, y, x, y);
	}

	void FillFramebuffer(uint16_t colour) {
		colour = __builtin_bswap16(colour);

		for (uint32_t i = 0; i < sizeof(s_frame_buffer) / sizeof(s_frame_buffer[0]); i++) {
			s_frame_buffer[i] = colour;
		}
	}

protected:
	uint32_t width_ { config::kWidth };
	uint32_t height_ { config::kHeight };
	uint32_t rotate_ { 0 };

#if !defined(SPI_LCD_kFrameBufferRows)
 static constexpr uint32_t kFrameBufferRows = 5;
#else
 static constexpr uint32_t kFrameBufferRows = SPI_LCD_kFrameBufferRows;
#endif

	static inline uint16_t s_frame_buffer[config::kWidth * kFrameBufferRows];
};

#endif  // SPI_PAINT_H_
