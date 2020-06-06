/**
 * @file d8x7segment.h
 *
 */
/* Copyright (C) 2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef DEVICE_D8X7SEGMENT_H_
#define DEVICE_D8X7SEGMENT_H_

#include "max7219.h"

class Max72197Segment: public MAX7219 {
public:
	Max72197Segment(void) {

	}

	void Init(uint8_t nIntensity) {
		WriteRegister(max7219::reg::SHUTDOWN, max7219::reg::shutdown::NORMAL_OP);
		WriteRegister(max7219::reg::DISPLAY_TEST, 0);
		WriteRegister(max7219::reg::DECODE_MODE, max7219::reg::decode_mode::CODEB);
		WriteRegister(max7219::reg::SCAN_LIMIT, 7);

		WriteRegister(max7219::reg::INTENSITY, nIntensity & 0x0F);

		Cls();
	}

	void Cls(void) {
		uint32_t i = 8;

		do {
			WriteRegister(i, max7219::digit::BLANK);
		} while (--i > 0);
	}

};

#endif /* DEVICE_D8X7SEGMENT_H_ */
