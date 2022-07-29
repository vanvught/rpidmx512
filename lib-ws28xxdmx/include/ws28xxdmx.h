/**
 * @file ws28xxdmx.h
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

#ifndef WS28XXDMX_H_
#define WS28XXDMX_H_

#include <cstdint>

#include "lightset.h"

#include "ws28xx.h"
#include "ws28xxdmxstore.h"

#include "pixeldmxconfiguration.h"
#include "pixelpatterns.h"

#include "pixeldmxhandler.h"

class WS28xxDmx final: public LightSet {
public:
	WS28xxDmx(PixelDmxConfiguration& pixelDmxConfiguration);
	~WS28xxDmx() override;

	void Initialize();

	void Start(uint32_t nPortIndex = 0) override;
	void Stop(uint32_t nPortIndex = 0) override;

	void SetData(uint32_t nPortIndex, const uint8_t *pData, uint32_t nLength) override;

	void Blackout(bool bBlackout) override;

	void SetWS28xxDmxStore(WS28xxDmxStore *pWS28xxDmxStore) {
		m_pWS28xxDmxStore = pWS28xxDmxStore;
	}

	void SetPixelDmxHandler(PixelDmxHandler *pPixelDmxHandler) {
		m_pPixelDmxHandler = pPixelDmxHandler;
	}

	uint32_t GetUniverses() const {
		return m_nUniverses;
	}

	pixel::Type GetType() const {
		return m_pWS28xx->GetType();
	}

	uint32_t GetCount() const {
		return m_nGroups * m_nGroupingCount;
	}

	pixel::Map GetMap() const {
		return m_pWS28xx->GetMap();
	}

	uint32_t GetGroups() const {
		return m_nGroups;
	}

	uint32_t GetGroupingCount() const {
		return m_nGroupingCount;
	}

	void Print() override;

// RDM
	bool SetDmxStartAddress(uint16_t nDmxStartAddress) override;

	uint16_t GetDmxStartAddress() override {
		return m_nDmxStartAddress;
	}

	uint16_t GetDmxFootprint() override {
		return m_nDmxFootprint;
	}

	bool GetSlotInfo(uint16_t nSlotOffset, lightset::SlotInfo &tSlotInfo) override;

	static WS28xxDmx *Get() {
		return s_pThis;
	}

private:
	pixeldmxconfiguration::PortInfo m_PortInfo;
	uint32_t m_nGroups;
	uint32_t m_nGroupingCount;
	uint32_t m_nUniverses;
	uint32_t m_nChannelsPerPixel;

	uint8_t m_nLowCode { 0 };
	uint8_t m_nHighCode { 0 };

	uint32_t m_nClockSpeedHz { 0 };
	uint8_t m_nGlobalBrightness { 0xFF };

	uint16_t m_nDmxStartAddress { lightset::Dmx::START_ADDRESS_DEFAULT };
	uint16_t m_nDmxFootprint { 170 * 3 };

	WS28xx *m_pWS28xx { nullptr };
	WS28xxDmxStore *m_pWS28xxDmxStore { nullptr };
	PixelDmxHandler *m_pPixelDmxHandler { nullptr };

	bool m_bIsStarted { false };
	bool m_bBlackout { false };

	static WS28xxDmx *s_pThis;
};

#endif /* WS28XXDMX_H_ */
