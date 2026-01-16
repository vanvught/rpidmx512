/**
 * @file ltcdisplayparamsconst.h
 *
 */
/* Copyright (C) 2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef JSON_LTCDISPLAYPARAMSCONST_H_
#define JSON_LTCDISPLAYPARAMSCONST_H_

#include "json/json_key.h"

namespace json
{
struct LtcDisplayParamsConst
{
	static constexpr char kFileName[] = "ltcdisplay.json";
   
   	// OLED SSD1306 / SSD1311
	static constexpr json::SimpleKey kOledIntensity {
	    "oled_intensity",
	    14,
	    Fnv1a32("oled_intensity", 14)
	};
	
	// Rotary control
	static constexpr json::SimpleKey kRotaryFullstep {
	    "rotary_fullstep",
	    15,
	    Fnv1a32("rotary_fullstep", 15)
	};
	
	// MAX7219 7-segment / matrix
	static constexpr json::SimpleKey kMax7219Type {
	    "max7219_type",
	    12,
	    Fnv1a32("max7219_type", 12)
	};

	static constexpr json::SimpleKey kMax7219Intensity {
	    "max7219_intensity",
	    17,
	    Fnv1a32("max7219_intensity", 17)
	};
	
	// PixelOutput specific
	static constexpr json::SimpleKey kPixelType {
	    "pixel_type",
	    10,
	    Fnv1a32("pixel_type", 10)
	};
	
	// RGB panel specific
	static constexpr json::SimpleKey kInfoMsg {
	    "info_msg",
	    8,
	    Fnv1a32("info_msg", 8)
	};
};
} // namespace json

#endif  // JSON_LTCDISPLAYPARAMSCONST_H_
