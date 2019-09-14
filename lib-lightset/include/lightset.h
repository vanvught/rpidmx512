/**
 * @file lightset.h
 *
 */
/* Copyright (C) 2016-2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

enum TLightSetDmx {
	DMX_ADDRESS_INVALID = 0xFFFF,
	DMX_START_ADDRESS_DEFAULT = 1,
	DMX_UNIVERSE_SIZE = 512
};

enum TLightSetOutputType {
	LIGHTSET_OUTPUT_TYPE_DMX,
	LIGHTSET_OUTPUT_TYPE_SPI,
	LIGHTSET_OUTPUT_TYPE_MONITOR,
	LIGHTSET_OUTPUT_TYPE_UNDEFINED
};

class LightSet {
public:
	LightSet(void);
	virtual ~LightSet(void);

	virtual void Start(uint8_t nPort)= 0;
	virtual void Stop(uint8_t nPort)= 0;

	virtual void SetData(uint8_t nPort, const uint8_t *pData, uint16_t nLength)= 0;

	virtual void Print(void);

public: // RDM Optional
	virtual bool SetDmxStartAddress(uint16_t nDmxStartAddress);
	virtual uint16_t GetDmxStartAddress(void);

	virtual uint16_t GetDmxFootprint(void);

	virtual bool GetSlotInfo(uint16_t nSlotOffset, struct TLightSetSlotInfo &tSlotInfo);

public: // WiFi solutions only
	static const char* GetOutputType(TLightSetOutputType type);
	static TLightSetOutputType GetOutputType(const char* sType);

public:
	static LightSet* Get(void) {
		return s_pThis;
	}

private:
	static LightSet *s_pThis;
};

#endif /* LIGHTSET_H_ */
