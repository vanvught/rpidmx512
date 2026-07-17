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

namespace json {
struct LtcDisplayParamsConst {
    static constexpr char kFileName[] = "ltcdisplay.json";

    // OLED SSD1306 / SSD1311
    static constexpr auto kOledIntensity = json::MakeSimpleKey("oled_intensity");
    // Rotary control
    static constexpr auto kRotaryFullstep = json::MakeSimpleKey("rotary_fullstep");
    // MAX7219 7-segment / matrix
    static constexpr auto kMax7219Type = json::MakeSimpleKey("max7219_type");
    static constexpr auto kMax7219Intensity = json::MakeSimpleKey("max7219_intensity");
    // PixelOutput specific
    static constexpr auto kPixelType = json::MakeSimpleKey("pixel_type");
    // RGB panel specific
    static constexpr auto kInfoMsg = json::MakeSimpleKey("info_msg");
};
} // namespace json

#endif // JSON_LTCDISPLAYPARAMSCONST_H_
