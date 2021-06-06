/**
 * @file lightset.h
 *
 */
/* Copyright (C) 2016-2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <cstdint>

namespace lightset {
struct Dmx {
	static constexpr auto ADDRESS_INVALID = 0xFFFF;
	static constexpr auto START_ADDRESS_DEFAULT = 1;
	static constexpr auto UNIVERSE_SIZE = 512;
	static constexpr auto MAX_VALUE = 255;
};

struct SlotInfo {
	uint8_t nType;
	uint16_t nCategory;
};

// WiFi solutions only
enum class OutputType {
	DMX,
	SPI,
	MONITOR,
	UNDEFINED
};
}  // namespace lightset

class LightSetDisplay {
public:
	virtual ~LightSetDisplay() {}

	virtual void ShowDmxStartAddress()=0;
};

class LightSetHandler {
public:
	virtual ~LightSetHandler() {}

	virtual void Start()=0;
	virtual void Stop()=0;
};

class LightSet {
public:
	LightSet();
	virtual ~LightSet() {}

	virtual void Start(uint32_t nPortIndex)= 0;
	virtual void Stop(uint32_t nPortIndex)= 0;
	virtual void SetData(uint32_t nPortIndex, const uint8_t *pData, uint32_t nLength)= 0;
	// Optional
	virtual void Blackout(__attribute__((unused)) bool bBlackout) {}
	virtual void Print() {}
	// RDM Optional
	virtual bool SetDmxStartAddress(uint16_t nDmxStartAddress);
	virtual uint16_t GetDmxStartAddress();
	virtual uint16_t GetDmxFootprint();
	virtual bool GetSlotInfo(uint16_t nSlotOffset, lightset::SlotInfo &tSlotInfo);

#if defined (ESP8266)
	// WiFi solutions only
	static const char *GetOutputType(lightset::OutputType type);
	static lightset::OutputType GetOutputType(const char *sType);
#endif

	void SetLightSetDisplay(LightSetDisplay *pLightSetDisplay) {
		s_pLightSetDisplay = pLightSetDisplay;
	}

	void SetLightSetHandler(LightSetHandler *pLightSetHandler) {
		s_pLightSetHandler = pLightSetHandler;
	}

	static LightSet *Get() {
		return s_pThis;
	}

protected:
	static LightSetDisplay *s_pLightSetDisplay;
	static LightSetHandler *s_pLightSetHandler;

private:
	static LightSet *s_pThis;
};

#endif /* LIGHTSET_H_ */
