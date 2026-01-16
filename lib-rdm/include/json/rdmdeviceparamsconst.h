/**
 * @file rdmdeviceparamsconst.h
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
 

#ifndef JSON_RDMDEVICEPARAMSCONST_H_
#define JSON_RDMDEVICEPARAMSCONST_H_

#include "json/json_key.h"

namespace json
{
struct RdmDeviceParamsConst
{
	static constexpr char kFileName[] = "rdmdevice.json";
   
	static constexpr json::SimpleKey kLabel {
	    "label",
	    5,
	    Fnv1a32("label", 5)
	};
	
	static constexpr json::SimpleKey kProductCategory {
	    "product_category",
	    16,
	    Fnv1a32("product_category", 16)
	};
	
	static constexpr json::SimpleKey kProductDetail {
	    "product_detail",
	    14,
	    Fnv1a32("product_detail", 14)
	};
};
} // namespace json

#endif  // JSON_RDMDEVICEPARAMSCONST_H_
