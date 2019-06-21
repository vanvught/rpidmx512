/**
 * @file devicesparams.h
 *
 */
/* Copyright (C) 2017-2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#ifndef WS28XXDMXPARAMS_H_
#define WS28XXDMXPARAMS_H_

#include <stdint.h>
#include <stdbool.h>

#include "ws28xxdmx.h"
#include "ws28xxdmxmulti.h"

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
};

enum TWS28xxDmxParamsMask {
	WS28XXDMX_PARAMS_MASK_LED_TYPE = (1 << 0),
	WS28XXDMX_PARAMS_MASK_LED_COUNT = (1 << 1),
	WS28XXDMX_PARAMS_MASK_DMX_START_ADDRESS = (1 << 2),
	WS28XXDMX_PARAMS_MASK_LED_GROUPING = (1 << 3),
	WS28XXDMX_PARAMS_MASK_SPI_SPEED = (1 << 4),
	WS28XXDMX_PARAMS_MASK_GLOBAL_BRIGHTNESS = (1 << 5),
	WS28XXDMX_PARAMS_MASK_ACTIVE_OUT = (1 << 6),
	WS28XXDMX_PARAMS_MASK_USE_SI5351A = (1 << 7),
	WS28XXDMX_PARAMS_MASK_LED_GROUP_COUNT = (1 << 8)
};

class WS28xxDmxParamsStore {
public:
	virtual ~WS28xxDmxParamsStore(void);

	virtual void Update(const struct TWS28xxDmxParams *pWS28xxDmxParams)=0;
	virtual void Copy(struct TWS28xxDmxParams *pWS28xxDmxParams)=0;
};

class WS28xxDmxParams {
public:
	WS28xxDmxParams(WS28xxDmxParamsStore *pWS28xxParamsStore = 0);
	~WS28xxDmxParams(void);

	bool Load(void);
	void Load(const char *pBuffer, uint32_t nLength);

	bool Builder(const struct TWS28xxDmxParams *ptWS28xxParams, uint8_t *pBuffer, uint32_t nLength, uint32_t& nSize);
	bool Save(uint8_t *pBuffer, uint32_t nLength, uint32_t& nSize);

	void Set(WS28xxDmx *);
	void Set(WS28xxDmxMulti *pWS28xxDmxMulti);

	void Dump(void);

	TWS28XXType GetLedType(void) {
		return m_tWS28xxParams.tLedType;
	}

	uint16_t GetLedCount(void) {
		return m_tWS28xxParams.nLedCount;
	}

	uint32_t GetClockSpeedHz(void) {
		return m_tWS28xxParams.nSpiSpeedHz;
	}

	uint8_t GetGlobalBrightness(void) {
		return m_tWS28xxParams.nGlobalBrightness;
	}

	uint16_t GetDmxStartAddress(void) {
		return m_tWS28xxParams.nDmxStartAddress;
	}

	bool IsLedGrouping(void) {
		return m_tWS28xxParams.bLedGrouping;
	}

	uint8_t GetActivePorts(void){
		return m_tWS28xxParams.nActiveOutputs;
	}

	bool UseSI5351A(void) {
		return m_tWS28xxParams.bUseSI5351A;
	}

	uint16_t GetLedGroupCount(void) {
		return m_tWS28xxParams.nLedGroupCount;
	}

public:
	static const char *GetLedTypeString(TWS28XXType);
    static void staticCallbackFunction(void *p, const char *s);

private:
    void callbackFunction(const char *pLine);
    bool isMaskSet(uint32_t nMask) const;

private:
    WS28xxDmxParamsStore *m_pWS28xxParamsStore;
    struct TWS28xxDmxParams m_tWS28xxParams;
};

#endif /* WS28XXDMXPARAMS_H_ */
