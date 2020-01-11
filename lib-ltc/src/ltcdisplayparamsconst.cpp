/**
 * @file ltcdisplayparamsconst.cpp
 *
 */
/* Copyright (C) 2019-2020 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include "ltcdisplayparamsconst.h"
#include "ltcdisplayws28xx.h"

alignas(uint32_t) const char LtcDisplayParamsConst::FILE_NAME[] = "ldisplay.txt";

alignas(uint32_t) const char LtcDisplayParamsConst::MAX7219_TYPE[] = "max7219_type";
alignas(uint32_t) const char LtcDisplayParamsConst::MAX7219_INTENSITY[] = "max7219_intensity";

alignas(uint32_t) const char LtcDisplayParamsConst::WS28XX_TYPE[] = "ws28xx_type";
alignas(uint32_t) const char LtcDisplayParamsConst::WS28XX_INTENSITY[] = "ws28xx_intensity";
alignas(uint32_t) const char LtcDisplayParamsConst::WS28XX_COLON_BLINK_MODE[] = "ws28xx_colon_blink_mode";
alignas(uint32_t) const char LtcDisplayParamsConst::WS28XX_COLOUR[LTCDISPLAYWS28XX_COLOUR_INDEX_LAST][24] = { "ws28xx_colour_segment", "ws28xx_colour_colon", "ws28xx_colour_message" };
