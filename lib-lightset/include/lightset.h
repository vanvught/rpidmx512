/**
 * @file lightset.h
 *
 */
/* Copyright (C) 2016-2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#ifndef LIGHTSET_H_
#define LIGHTSET_H_

#include <stdint.h>

struct TLightSetSlotInfo {
	uint8_t nType;
	uint16_t nCategory;
};

enum {
	DMX_ADDRESS_INVALID = 0xFFFF
};

class LightSet {
public:
	virtual ~LightSet(void);

	virtual void Start(uint8_t nPort)= 0;
	virtual void Stop(uint8_t nPort)= 0;

	virtual void SetData(uint8_t nPort, const uint8_t *, uint16_t)= 0;

public: // RDM Optional
	virtual bool SetDmxStartAddress(uint16_t nDmxStartAddress);
	virtual uint16_t GetDmxStartAddress(void);

	virtual uint16_t GetDmxFootprint(void);

	virtual bool GetSlotInfo(uint16_t nSlotOffset, struct TLightSetSlotInfo &tSlotInfo);
};

#endif /* LIGHTSET_H_ */
