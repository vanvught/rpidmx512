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
#include "common/utils/utils_hash.h"

namespace json {
struct DmxLedParamsConst {
    static constexpr char kFileName[] = "dmxled.json";

    static constexpr json::SimpleKey kType{"type", 4, Fnv1a32("type", 4)};
    static constexpr json::SimpleKey kMap{"map", 3, Fnv1a32("map", 3)};
    static constexpr json::SimpleKey kCount{"count", 5, Fnv1a32("count", 5)};
    static constexpr json::SimpleKey kGroupingCount{"group_count", 11, Fnv1a32("group_count", 11)};
    static constexpr json::SimpleKey kT0H{"t0h", 3, Fnv1a32("t0h", 3)};

    static constexpr json::SimpleKey kT1H{"t1h", 3, Fnv1a32("t1h", 3)};
    static constexpr json::SimpleKey kActiveOutputPorts{"active_out", 10, Fnv1a32("active_out", 10)};
    static constexpr json::SimpleKey kTestPattern{"test_pattern", 12, Fnv1a32("test_pattern", 12)};
    static constexpr json::SimpleKey kSpiSpeedHz{"clock_speed_hz", 14, Fnv1a32("clock_speed_hz", 14)};
    static constexpr json::SimpleKey kGlobalBrightness{"global_brightness", 17, Fnv1a32("global_brightness", 17)};
    static constexpr json::SimpleKey kGammaCorrection{"gamma_correction", 16, Fnv1a32("gamma_correction", 16)};
    static constexpr json::SimpleKey kGammaValue{"gamma_value", 11, Fnv1a32("gamma_value", 11)};
};
} // namespace json

#endif // JSON_DMXLEDPARAMSCONST_H_
