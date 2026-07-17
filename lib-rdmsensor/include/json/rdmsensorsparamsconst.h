/**
 * @file rdmsensorsparamsconst.h
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

#ifndef JSON_RDMSENSORSPARAMSCONST_H_
#define JSON_RDMSENSORSPARAMSCONST_H_

#include "json/json_key.h"

namespace json {
struct RdmSensorsParamsConst {
    static constexpr char kFileName[] = "sensors.json";

    static constexpr auto kBH170 = json::MakeSimpleKey("bh1750");
    static constexpr auto kHTU21D = json::MakeSimpleKey("htu21d");
    static constexpr auto kINA219 = json::MakeSimpleKey("ina219");
    static constexpr auto kMCP9808 = json::MakeSimpleKey("mcp9808");
    static constexpr auto kSI7021 = json::MakeSimpleKey("si7021");
    static constexpr auto kMCP3424 = json::MakeSimpleKey("mcp3424");
};
} // namespace json

#endif // JSON_RDMSENSORSPARAMSCONST_H_
