/**
 * @file e131paramsconst.h
 *
 */
/* Copyright (C) 2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#ifndef E131PARAMSCONST_H_
#define E131PARAMSCONST_H_

#include <stdint.h>

class E131ParamsConst {
public:
	alignas(uint32_t) static const char PARAMS_FILE_NAME[];
	alignas(uint32_t) static const char PARAMS_UNIVERSE_PORT[4][16];
	alignas(uint32_t) static const char PARAMS_MERGE_MODE[];
	alignas(uint32_t) static const char PARAMS_MERGE_MODE_PORT[4][18];
	alignas(uint32_t) static const char PARAMS_NETWORK_DATA_LOSS_TIMEOUT[];
	alignas(uint32_t) static const char PARAMS_DISABLE_MERGE_TIMEOUT[];
};

#endif /* E131PARAMSCONST_H_ */
