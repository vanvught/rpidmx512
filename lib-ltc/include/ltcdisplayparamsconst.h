/**
 * @file ltcdisplayparamsconst.h
 *
 */
/* Copyright (C) 2019-2020 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef LTCDISPLAYPARAMSCONST_H_
#define LTCDISPLAYPARAMSCONST_H_

#include <cstdint>

#include "ltcdisplayrgb.h"

struct LtcDisplayParamsConst {
	static inline const char FILE_NAME[] = "ldisplay.txt";

	/**
	 * OLED SSD1306 / SSD1311
	 */

	static inline const char OLED_INTENSITY[] = "oled_intensity";

	/**
	 * Rotary control
	 */

	static inline const char ROTARY_FULLSTEP[] = "rotary_fullstep";

	/**
	 * MAX7219 7-segment / matrix
	 */

	static inline const char MAX7219_TYPE[] = "max7219_type";
	static inline const char MAX7219_INTENSITY[] = "max7219_intensity";

	/**
	 * RGB Display generic
	 */

	static inline const char INTENSITY[] = "intensity";
	static inline const char COLON_BLINK_MODE[] = "colon_blink_mode";
	static inline const char COLOUR[static_cast<uint32_t>(ltcdisplayrgb::ColourIndex::LAST)][24] =
		{ "colour_time",
		  "colour_colon",
		  "colour_message",
		  "colour_fps",		// RGB panel specific
		  "colour_info",	// RGB panel specific
		  "colour_source"	// RGB panel specific
		};

	/**
	 * WS28xx specific
	 */

	static inline const char WS28XX_TYPE[] = "ws28xx_type";

	/**
	 * RGB panel specific
	 */

	static inline const char INFO_MSG[] = "info_msg";
};

#endif /* LTCDISPLAYPARAMSCONST_H_ */
