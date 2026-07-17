/**
 * @file dmxledparamsconst.h
 */
/* Copyright (C) 2025-2026 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef JSON_DMXLEDPARAMSCONST_H_
#define JSON_DMXLEDPARAMSCONST_H_

#include "json/json_key.h"

namespace json {
struct DmxLedParamsConst {
    static constexpr char kFileName[] = "dmxled.json";

    static constexpr auto kType = json::MakeSimpleKey("type");
    static constexpr auto kMap = json::MakeSimpleKey("map");
    static constexpr auto kCount = json::MakeSimpleKey("count");
    static constexpr auto kGroupingCount = json::MakeSimpleKey("group_count");
    static constexpr auto kT0H = json::MakeSimpleKey("t0h");
    static constexpr auto kT1H = json::MakeSimpleKey("t1h");
    static constexpr auto kActiveOutputPorts = json::MakeSimpleKey("active_out");
    static constexpr auto kTestPattern = json::MakeSimpleKey("test_pattern");
    static constexpr auto kSpiSpeedHz = json::MakeSimpleKey("clock_speed_hz");
    static constexpr auto kGlobalBrightness = json::MakeSimpleKey("global_brightness");
    static constexpr auto kGammaCorrection = json::MakeSimpleKey("gamma_correction");
    static constexpr auto kGammaValue = json::MakeSimpleKey("gamma_value");
};
} // namespace json

#endif // JSON_DMXLEDPARAMSCONST_H_
