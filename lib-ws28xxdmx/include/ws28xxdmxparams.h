/**
 * @file devicesparams.h
 *
 */
/* Copyright (C) 2017-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"); to deal
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

#ifndef WS28XXDMXPARAMS_H_
#define WS28XXDMXPARAMS_H_

#include <stdint.h>

#include "ws28xx.h"
#include "ws28xxdmx.h"
#include "ws28xxdmxmulti.h"

#include "rgbmapping.h"

struct TWS28xxDmxParams {
    uint32_t nSetList;
	TWS28XXType tLedType;
	uint16_t nLedCount;
	uint16_t nDmxStartAddress;
	bool bLedGrouping;
	uint32_t nSpiSpeedHz;
	uint8_t nGlobalBrightness;
	uint8_t nActiveOutputs;
	bool bUseSI5351A;
	uint16_t nLedGroupCount;
	uint8_t nRgbMapping;
	uint8_t nLowCode;
	uint8_t nHighCode;
};

static_assert(sizeof(struct TWS28xxDmxParams) <= 64, "struct TWS28xxDmxParams is too large");

struct WS28xxDmxParamsMask {
	static constexpr auto LED_TYPE = (1U << 0);
	static constexpr auto LED_COUNT = (1U << 1);
	static constexpr auto DMX_START_ADDRESS = (1U << 2);
	static constexpr auto LED_GROUPING = (1U << 3);
	static constexpr auto SPI_SPEED = (1U << 4);
	static constexpr auto GLOBAL_BRIGHTNESS = (1U << 5);
	static constexpr auto ACTIVE_OUT = (1U << 6);
	static constexpr auto USE_SI5351A = (1U << 7);
	static constexpr auto LED_GROUP_COUNT = (1U << 8);
	static constexpr auto RGB_MAPPING = (1U << 9);
	static constexpr auto LOW_CODE = (1U << 10);
	static constexpr auto HIGH_CODE = (1U << 11);
};

class WS28xxDmxParamsStore {
public:
	virtual ~WS28xxDmxParamsStore() {
	}

	virtual void Update(const struct TWS28xxDmxParams *pWS28xxDmxParams)=0;
	virtual void Copy(struct TWS28xxDmxParams *pWS28xxDmxParams)=0;
};

class WS28xxDmxParams {
public:
	WS28xxDmxParams(WS28xxDmxParamsStore *pWS28xxParamsStore = nullptr);
	~WS28xxDmxParams();

	bool Load();
	void Load(const char *pBuffer, uint32_t nLength);

	void Builder(const struct TWS28xxDmxParams *ptWS28xxParams, char *pBuffer, uint32_t nLength, uint32_t &nSize);
	void Save(char *pBuffer, uint32_t nLength, uint32_t &nSize);

	void Set(WS28xxDmx *pWS28xxDmx);
	void Set(WS28xxDmxMulti *pWS28xxDmxMulti);

	void Dump();

	TWS28XXType GetLedType() const {
		return m_tWS28xxParams.tLedType;
	}

	uint16_t GetLedCount() const {
		return m_tWS28xxParams.nLedCount;
	}

	uint32_t GetClockSpeedHz() const {
		return m_tWS28xxParams.nSpiSpeedHz;
	}

	uint8_t GetGlobalBrightness() const {
		return m_tWS28xxParams.nGlobalBrightness;
	}

	uint16_t GetDmxStartAddress() const {
		return m_tWS28xxParams.nDmxStartAddress;
	}

	bool IsLedGrouping() const {
		return m_tWS28xxParams.bLedGrouping;
	}

	uint8_t GetActivePorts() const {
		return m_tWS28xxParams.nActiveOutputs;
	}

	bool UseSI5351A() const {
		return m_tWS28xxParams.bUseSI5351A;
	}

	uint16_t GetLedGroupCount() const {
		return m_tWS28xxParams.nLedGroupCount;
	}

	TRGBMapping GetRgbMapping() const {
		return static_cast<TRGBMapping>(m_tWS28xxParams.nRgbMapping);
	}

	float GetLowCode() {
		return WS28xx::ConvertTxH(m_tWS28xxParams.nLowCode);
	}

	float GetHighCode() {
		return WS28xx::ConvertTxH(m_tWS28xxParams.nHighCode);
	}

public:
	static void staticCallbackFunction(void *p, const char *s);

private:
    void callbackFunction(const char *pLine);
    bool isMaskSet(uint32_t nMask) const {
    	return (m_tWS28xxParams.nSetList & nMask) == nMask;
    }

private:
    WS28xxDmxParamsStore *m_pWS28xxParamsStore;
    struct TWS28xxDmxParams m_tWS28xxParams;
};

#endif /* WS28XXDMXPARAMS_H_ */
