/**
 * @file lightset.h
 *
 */
/* Copyright (C) 2016-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include "lightsetdisplay.h"

#include "debug.h"

struct TLightSetSlotInfo {
	uint8_t nType;
	uint16_t nCategory;
};

enum TLightSetDmx {
	DMX_ADDRESS_INVALID = 0xFFFF,
	DMX_START_ADDRESS_DEFAULT = 1,
	DMX_UNIVERSE_SIZE = 512,
	DMX_MAX_VALUE = 255
};

enum TLightSetOutputType {
	LIGHTSET_OUTPUT_TYPE_DMX,
	LIGHTSET_OUTPUT_TYPE_SPI,
	LIGHTSET_OUTPUT_TYPE_MONITOR,
	LIGHTSET_OUTPUT_TYPE_UNDEFINED
};

class LightSet {
public:
	LightSet();
	virtual ~LightSet() {
	}

	virtual void Start(uint8_t nPort)= 0;
	virtual void Stop(uint8_t nPort)= 0;

	virtual void SetData(uint8_t nPort, const uint8_t *pData, uint16_t nLength)= 0;

	virtual void Print() {
	}

	void SetLightSetDisplay(LightSetDisplay *pLightSetDisplay) {
		DEBUG_ENTRY
		m_pLightSetDisplay = pLightSetDisplay;
		DEBUG_EXIT
	}

	// RDM Optional
	virtual bool SetDmxStartAddress(uint16_t nDmxStartAddress);
	virtual uint16_t GetDmxStartAddress();
	virtual uint16_t GetDmxFootprint();
	virtual bool GetSlotInfo(uint16_t nSlotOffset, struct TLightSetSlotInfo &tSlotInfo);

	// WiFi solutions only
	static const char *GetOutputType(TLightSetOutputType type);
	static TLightSetOutputType GetOutputType(const char *sType);

	static LightSet *Get() {
		return s_pThis;
	}

protected:
	LightSetDisplay *m_pLightSetDisplay{nullptr};

private:
	static LightSet *s_pThis;
};

#endif /* LIGHTSET_H_ */
