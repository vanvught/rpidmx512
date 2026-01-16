/**
 * @file l6470paramsconst.h
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

#ifndef JSON_L6470PARAMSCONST_H_
#define JSON_L6470PARAMSCONST_H_

#include "json/json_key.h"

namespace json
{
struct L6470ParamsConst
{
	static constexpr json::SimpleKey kMinSpeed {
	    "l6470_min_speed",
	    15,
	    Fnv1a32("l6470_min_speed", 15)
	};


	static constexpr json::SimpleKey kMaxSpeed {
	    "l6470_max_speed",
	    15,
	    Fnv1a32("l6470_max_speed", 15)
	};

	static constexpr json::SimpleKey kAcc {
	    "l6470_acc",
	    9,
	    Fnv1a32("l6470_acc", 9)
	};
	
	static constexpr json::SimpleKey kDec {
	    "l6470_dec",
	    9,
	    Fnv1a32("l6470_dec", 9)
	};

	static constexpr json::SimpleKey kKvalHold {
	    "l6470_kval_hold",
	    15,
	    Fnv1a32("l6470_kval_hold", 15)
	};
	
	static constexpr json::SimpleKey kKvalRun {
	    "l6470_kval_run",
	    14,
	    Fnv1a32("l6470_kval_run", 14)
	};

	static constexpr json::SimpleKey kKvalAcc {
	    "l6470_kval_acc",
	    14,
	    Fnv1a32("l6470_kval_acc", 14)
	};

	static constexpr json::SimpleKey kKvalDec {
	    "l6470_kval_dec",
	    14,
	    Fnv1a32("l6470_kval_dec", 14)
	};
	
	static constexpr json::SimpleKey kMicroSteps {
	    "l6470_micro_steps",
	    17,
	    Fnv1a32("l6470_micro_steps", 17)
	};

};
} // namespace json

#endif  // JSON_L6470PARAMSCONST_H_
