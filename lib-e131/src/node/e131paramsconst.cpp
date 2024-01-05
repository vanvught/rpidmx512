/**
 * @file e131paramsconst.cpp
 *
 */
/* Copyright (C) 2019-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include "e131paramsconst.h"
#include "e131params.h"

#if LIGHTSET_PORTS > 4
# define MAX_ARRAY 4
#else
# define MAX_ARRAY LIGHTSET_PORTS
#endif

const char E131ParamsConst::FILE_NAME[] = "e131.txt";

const char E131ParamsConst::PRIORITY[e131params::MAX_PORTS][18] {
	"priority_port_a",
#if MAX_ARRAY >= 2
	"priority_port_b",
#endif
#if MAX_ARRAY >= 3
	"priority_port_c",
#endif
#if MAX_ARRAY == 4
	"priority_port_d"
#endif
};

