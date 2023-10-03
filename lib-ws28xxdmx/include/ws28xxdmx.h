/**
 * @file ws28xxdmx.h
 *
 */
/* Copyright (C) 2016-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <cassert>

#include "lightset.h"

#include "ws28xx.h"
#include "pixeldmxconfiguration.h"
#include "pixelpatterns.h"
#include "pixeldmxstore.h"
#include "pixeldmxhandler.h"

class WS28xxDmx final: public LightSet {
public:
	WS28xxDmx(PixelDmxConfiguration& pixelDmxConfiguration);
	~WS28xxDmx() override;

	void Start(const uint32_t nPortIndex) override;
	void Stop(const uint32_t nPortIndex) override;

	void SetData(uint32_t nPortIndex, const uint8_t *pData, uint32_t nLength, const bool doUpdate = true) override;
	void Sync(__attribute__((unused)) const uint32_t nPortIndex) override {}
	void Sync(const bool doForce) override {
		if (__builtin_expect((!doForce), 1)) {
			assert(m_pWS28xx != nullptr);
			m_pWS28xx->Update();
		}
	}

#if defined (OUTPUT_HAVE_STYLESWITCH)
	void SetOutputStyle(__attribute__((unused)) const uint32_t nPortIndex, __attribute__((unused)) const lightset::OutputStyle outputStyle) override {}
	lightset::OutputStyle GetOutputStyle(__attribute__((unused)) const uint32_t nPortIndex) const override {
		return lightset::OutputStyle::DELTA;
	}
#endif

	void Blackout(bool bBlackout) override;
	void FullOn() override;

	void Print() override {
		m_pixelDmxConfiguration.Print();
	}

	void SetWS28xxDmxStore(PixelDmxStore *pWS28xxDmxStore) {
		m_pWS28xxDmxStore = pWS28xxDmxStore;
	}

	void SetPixelDmxHandler(PixelDmxHandler *pPixelDmxHandler) {
		m_pPixelDmxHandler = pPixelDmxHandler;
	}

	pixel::Type GetType() const {
		return m_pixelDmxConfiguration.GetType();
	}

	pixel::Map GetMap() const {
		return m_pixelDmxConfiguration.GetMap();
	}

	uint32_t GetCount() const {
		return m_pixelDmxConfiguration.GetCount();
	}

	uint32_t GetGroups() const {
		return m_pixelDmxConfiguration.GetGroups();
	}

	uint32_t GetGroupingCount() const {
		return m_pixelDmxConfiguration.GetGroupingCount();
	}

	uint32_t GetUniverses() const {
		return m_pixelDmxConfiguration.GetUniverses();
	}

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
	PixelDmxConfiguration m_pixelDmxConfiguration;
	pixeldmxconfiguration::PortInfo m_PortInfo;
	uint32_t m_nChannelsPerPixel;

	uint16_t m_nDmxStartAddress;
	uint16_t m_nDmxFootprint;

	WS28xx *m_pWS28xx { nullptr };
	PixelDmxStore *m_pWS28xxDmxStore { nullptr };
	PixelDmxHandler *m_pPixelDmxHandler { nullptr };

	bool m_bIsStarted { false };
	bool m_bBlackout { false };

	static WS28xxDmx *s_pThis;
};

#endif /* WS28XXDMX_H_ */
