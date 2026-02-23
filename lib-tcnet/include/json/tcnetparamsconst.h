/**
 * @file tcnetparamsconst.h
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
 

#ifndef JSON_TCNETPARAMSCONST_H_
#define JSON_TCNETPARAMSCONST_H_

 #include "json/json_key.h"

namespace json
{
struct TcNetParamsConst
{
	static constexpr char kFileName[] = "tcnet.json";
   
	static constexpr json::SimpleKey kNodeName {
	    "node_name",
	    9,
	    Fnv1a32("node_name", 9)
	};
	
	static constexpr json::SimpleKey kLayer {
	    "layer",
	    5,
	    Fnv1a32("layer", 5)
	};
	
	static constexpr json::SimpleKey kTimecodeType {
	    "timecode_type",
	    13,
	    Fnv1a32("timecode_type", 13)
	};
	
	static constexpr json::SimpleKey kUseTimecode {
	    "use_timecode",
	    12,
	    Fnv1a32("use_timecode", 12)
	};
};
} // namespace json

#endif  // JSON_TCNETPARAMSCONST_H_
