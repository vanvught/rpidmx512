/**
 * @file pixeldisplay7segment.h
 */
/*
 * Copyright (C) 2019 by hippy mailto:dmxout@gmail.com
 * Copyright (C) 2019-2026 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef PIXELDISPLAY7SEGMENT_H_
#define PIXELDISPLAY7SEGMENT_H_

#include <cstdint>

#include "pixel.h"
#include "pixeltype.h"

struct WS28xxDisplay7SegmentConfig {
	static constexpr uint32_t NUM_OF_DIGITS = 8;
	static constexpr uint32_t NUM_OF_COLONS = 3;

	static constexpr uint32_t SEGMENTS_PER_DIGIT = 7;	///< number of LEDs that make up one digit
	static constexpr uint32_t LEDS_PER_SEGMENT = 1;		///< number of LEDs that make up one segment
	static constexpr uint32_t LEDS_PER_COLON = 2;		///< number of LEDs that make up one colon

	static constexpr uint32_t LED_COUNT = ((WS28xxDisplay7SegmentConfig::LEDS_PER_SEGMENT * WS28xxDisplay7SegmentConfig::SEGMENTS_PER_DIGIT * WS28xxDisplay7SegmentConfig::NUM_OF_DIGITS)
			+ (WS28xxDisplay7SegmentConfig::LEDS_PER_COLON * WS28xxDisplay7SegmentConfig::NUM_OF_COLONS));
};

class WS28xxDisplay7Segment {
public:
	WS28xxDisplay7Segment(pixel::LedType led_type, pixel::LedMap led_map);
	~WS28xxDisplay7Segment();

	void WriteChar(char ch, uint32_t pos, uint8_t red, uint8_t green, uint8_t blue);
	void WriteColon(char ch, uint32_t pos, uint8_t red, uint8_t green, uint8_t blue);

	void WriteAll(const char *chars, uint8_t red, uint8_t green, uint8_t blue);

	void SetColonsOff();

	void Show();

private:
	void RenderSegment(bool on_off, uint32_t current_digit_base, uint32_t current_segment, uint8_t red, uint8_t green, uint8_t blue);

private:
	PixelOutput *m_pPixelOutput { nullptr };
};

#endif /* PIXELDISPLAY7SEGMENT_H_ */
