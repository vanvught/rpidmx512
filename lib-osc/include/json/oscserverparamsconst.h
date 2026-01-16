/**
 * @file oscserverparamsconst.h
 *
 */
/* Copyright (C) 2022-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef JSON_OSCSERVERPARAMSCONST_H_
#define JSON_OSCSERVERPARAMSCONST_H_

#include "json/json_key.h"

namespace json
{
struct OscServerParamsConst
{
    static constexpr char kFileName[] = "oscserver.json";

    static constexpr json::SimpleKey kPath
    {
		"path", 
		4, 
		Fnv1a32("path", 4)
	};

    static constexpr json::SimpleKey kPathInfo
    {
		"path_info", 
		9, 
		Fnv1a32("path_info", 9)
	};

    static constexpr json::SimpleKey kPathBlackout
    {
		"path_blackout", 
		13, 
		Fnv1a32("path_blackout", 13)
	};

    static constexpr json::SimpleKey kTransmission
    {
		"partial_transmission", 
		19, 
		Fnv1a32("partial_transmission", 19)
	};
};
} // namespace json

#endif  // JSON_OSCSERVERPARAMSCONST_H_
