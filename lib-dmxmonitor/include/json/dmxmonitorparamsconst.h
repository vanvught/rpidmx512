/**
 * @file dmxmonitorparamsconst.h
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

#ifndef JSON_DMXMONITORPARAMSCONST_H_
#define JSON_DMXMONITORPARAMSCONST_H_

#include "json/json_key.h"

namespace json {
struct DmxMonitorParamsConst {
    static constexpr char kFileName[] = "monitor.json";

    static constexpr auto kDmxStartAddress = json::MakeSimpleKey("dmx_start_address");
    static constexpr auto kDmxMaxChannels = json::MakeSimpleKey("dmx_max_channels");
    static constexpr auto kFormat = json::MakeSimpleKey("format");
};
} // namespace json

#endif // JSON_DMXMONITORPARAMSCONST_H_
