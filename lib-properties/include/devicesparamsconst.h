/**
 * @file devicesparamsconst.h
 *
 */
/* Copyright (C) 2019-2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef DEVICESPARAMSCONST_H_
#define DEVICESPARAMSCONST_H_

struct DevicesParamsConst {
	static const char FILE_NAME[];

	static const char TYPE[];
	static const char MAP[];

	static const char LED_T0H[];
	static const char LED_T1H[];

	static const char COUNT[];
	static const char GROUPING_COUNT[];

	static const char SPI_SPEED_HZ[];

	static const char GLOBAL_BRIGHTNESS[];

	static const char ACTIVE_OUT[];

	static const char TEST_PATTERN[];

	static const char GAMMA_CORRECTION[];
	static const char GAMMA_VALUE[];
};

#endif /* DEVICESPARAMSCONST_H_ */
