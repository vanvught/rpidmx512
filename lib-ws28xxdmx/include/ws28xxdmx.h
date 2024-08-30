/**
 * @file ws28xxdmx.h
 *
 */
/* Copyright (C) 2016-2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

class WS28xxDmx final: public LightSet {
public:
	WS28xxDmx();
	~WS28xxDmx() override;

	void Start(const uint32_t nPortIndex) override;
	void Stop(const uint32_t nPortIndex) override;

	void SetData(const uint32_t nPortIndex, const uint8_t *pData, uint32_t nLength, const bool doUpdate = true) override;
	void Sync([[maybe_unused]] const uint32_t nPortIndex) override {}
	void Sync() override {
		assert(m_pWS28xx != nullptr);
		m_pWS28xx->Update();
	}

#if defined (OUTPUT_HAVE_STYLESWITCH)
	void SetOutputStyle([[maybe_unused]] const uint32_t nPortIndex, [[maybe_unused]] const lightset::OutputStyle outputStyle) override {}
	lightset::OutputStyle GetOutputStyle([[maybe_unused]] const uint32_t nPortIndex) const override {
		return lightset::OutputStyle::DELTA;
	}
#endif

	void Blackout(bool bBlackout) override;
	void FullOn() override;

	void Print() override {
		PixelDmxConfiguration::Get().Print();
	}

// RDM
	bool SetDmxStartAddress(uint16_t nDmxStartAddress) override;

	uint16_t GetDmxStartAddress() override {
		return PixelDmxConfiguration::Get().GetDmxStartAddress();
	}

	uint16_t GetDmxFootprint() override {
		return PixelDmxConfiguration::Get().GetDmxFootprint();
	}

	bool GetSlotInfo(uint16_t nSlotOffset, lightset::SlotInfo &tSlotInfo) override;

	static WS28xxDmx *Get() {
		return s_pThis;
	}

private:
	WS28xx *m_pWS28xx { nullptr };

	bool m_bIsStarted { false };
	bool m_bBlackout { false };

	static WS28xxDmx *s_pThis;
};

#endif /* WS28XXDMX_H_ */
