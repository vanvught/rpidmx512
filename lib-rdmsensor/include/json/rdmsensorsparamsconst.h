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

namespace json
{
struct RdmSensorsParamsConst
{
	static constexpr char kFileName[] = "sensors.json";
   
	static constexpr json::SimpleKey kBH170 {
	    "bh1750",
	    6,
	    Fnv1a32("bh1750", 6)
	};
	
	static constexpr json::SimpleKey kHTU21D {
	    "htu21d",
	    6,
	    Fnv1a32("htu21d", 6)
	};
	
	static constexpr json::SimpleKey kINA219 {
	    "ina219",
	    6,
	    Fnv1a32("ina219", 6)
	};
	
	static constexpr json::SimpleKey kMCP9808 {
	    "mcp9808",
	    7,
	    Fnv1a32("mcp9808", 7)
	};
	
	static constexpr json::SimpleKey kSI7021 {
	    "si7021",
	    6,
	    Fnv1a32("si7021", 6)
	};
	
	static constexpr json::SimpleKey kMCP3424 {
	    "mcp3424",
	    7,
	    Fnv1a32("mcp3424", 7)
	};
};
} // namespace json

#endif  // JSON_RDMSENSORSPARAMSCONST_H_
