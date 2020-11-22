/**
 * @file ws28xxdmx.h
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

#ifndef WS28XXDMX_H_
#define WS28XXDMX_H_

#include <stdint.h>

#include "lightset.h"

#include "ws28xx.h"
#include "ws28xxdmxstore.h"

class WS28xxDmx: public LightSet {
public:
	WS28xxDmx();
	~WS28xxDmx() override;

	void Start(uint8_t nPort = 0) override;
	void Stop(uint8_t nPort = 0) override;

	void SetData(uint8_t nPort, const uint8_t*, uint16_t) override;

	void Blackout(bool bBlackout);

	virtual void SetLEDType(TWS28XXType);
	TWS28XXType GetLEDType() {
		return m_tLedType;
	}

	void SetRgbMapping(TRGBMapping tRGBMapping) {
		m_tRGBMapping = tRGBMapping;
	}

	void SetLowCode(uint8_t nLowCode) {
		m_nLowCode = nLowCode;
	}

	void SetHighCode(uint8_t nHighCode) {
		m_nHighCode = nHighCode;
	}

	virtual void SetLEDCount(uint16_t);
	uint16_t GetLEDCount() {
		return m_nLedCount;
	}

	void SetClockSpeedHz(uint32_t nClockSpeedHz) {
		m_nClockSpeedHz = nClockSpeedHz;
	}

	uint32_t GetClockSpeedHz() const {
		return m_nClockSpeedHz;
	}

	void SetGlobalBrightness(uint8_t nGlobalBrightness) {
		m_nGlobalBrightness = nGlobalBrightness;
	}

	uint8_t GetGlobalBrightness() const {
		return m_nGlobalBrightness;
	}

	void SetWS28xxDmxStore(WS28xxDmxStore *pWS28xxDmxStore) {
		m_pWS28xxDmxStore = pWS28xxDmxStore;
	}

	void Print() override;

public: // RDM
	bool SetDmxStartAddress(uint16_t nDmxStartAddress) override;

	uint16_t GetDmxStartAddress() override {
		return m_nDmxStartAddress;
	}

	uint16_t GetDmxFootprint() override {
		return m_nDmxFootprint;
	}

	bool GetSlotInfo(uint16_t nSlotOffset, struct TLightSetSlotInfo &tSlotInfo) override;

private:
	void UpdateMembers();

protected:
	TWS28XXType m_tLedType{WS2812B};

	TRGBMapping m_tRGBMapping{RGB_MAPPING_UNDEFINED};
	uint8_t m_nLowCode{0};
	uint8_t m_nHighCode{0};

	uint16_t m_nLedCount{170};
	uint16_t m_nDmxStartAddress{DMX_START_ADDRESS_DEFAULT};
	uint16_t m_nDmxFootprint;

	WS28xx* m_pLEDStripe{nullptr};
	bool m_bIsStarted{false};
	bool m_bBlackout{false};

	WS28xxDmxStore *m_pWS28xxDmxStore{nullptr};

private:
	uint32_t m_nClockSpeedHz{0};
	uint8_t m_nGlobalBrightness{0xFF};
	uint32_t m_nBeginIndexPortId1{170};
	uint32_t m_nBeginIndexPortId2{340};
	uint32_t m_nBeginIndexPortId3{510};
	uint32_t m_nChannelsPerLed{3};

	uint32_t m_nPortIdLast{3};
};

#endif /* WS28XXDMX_H_ */
