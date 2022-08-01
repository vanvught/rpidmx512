/**
 * @file pixeldmxstore.h
 *
 */
/* Copyright (C) 2019-2022 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef PIXELDMXSTORE_H_
#define PIXELDMXSTORE_H_

#include <cstdint>

class PixelDmxStore {
public:
	virtual ~PixelDmxStore() {
	}

	virtual void SaveType(uint8_t nType)=0;
	virtual void SaveCount(uint16_t nCount)=0;
	virtual void SaveGroupingCount(uint16_t nGroupingCount)=0;
	virtual void SaveMap(uint8_t nMap)=0;
	virtual void SaveTestPattern(uint8_t nTestPattern)=0;
	virtual void SaveDmxStartAddress(uint16_t nDmxStartAddress)=0;
};

#endif /* PIXELDMXSTORE_H_ */
