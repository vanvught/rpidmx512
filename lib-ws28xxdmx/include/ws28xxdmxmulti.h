/**
 * @file ws28xxdmxmulti.h
 *
 */
/* Copyright (C) 2019-2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef WS28XXDMXMULTI_H_
#define WS28XXDMXMULTI_H_

#include <cstdint>

#include "lightset.h"

#include "ws28xxmulti.h"

#include "pixeldmxconfiguration.h"
#include "pixelpatterns.h"

#include "pixeldmxhandler.h"

namespace ws28xxdmxmulti {

}  // namespace ws28xxdmxmulti

class WS28xxDmxMulti final: public LightSet {
public:
	WS28xxDmxMulti(PixelDmxConfiguration& pixelDmxConfiguration);
	~WS28xxDmxMulti() override;

	void Start(uint32_t nPortIndex) override;
	void Stop(uint32_t nPortIndex) override;

	void SetData(uint32_t nPortIndex, const uint8_t *pData, uint32_t nLength) override;

	void Blackout(bool bBlackout) override;

	uint32_t GetUniverses() const {
		return m_nUniverses;
	}

	uint32_t GetGroups() const {
		return m_nGroups;
	}

	uint32_t GetOutputPorts() const {
		return m_nOutputPorts;
	}

	void Print() override;

	void SetPixelDmxHandler(PixelDmxHandler *pPixelDmxHandler) {
		m_pPixelDmxHandler = pPixelDmxHandler;
	}

	// RDMNet LLRP Device Only
	bool SetDmxStartAddress(__attribute__((unused)) uint16_t nDmxStartAddress) override {
		return false;
	}

	uint16_t GetDmxStartAddress() override {
		return lightset::Dmx::ADDRESS_INVALID;
	}

	uint16_t GetDmxFootprint() override {
		return 0;
	}

private:
	pixeldmxconfiguration::PortInfo m_PortInfo;
	uint32_t m_nGroups;
	uint32_t m_nGroupingCount;
	uint32_t m_nUniverses;
	uint32_t m_nChannelsPerPixel;
	uint32_t m_nOutputPorts;

	WS28xxMulti *m_pWS28xxMulti { nullptr };
	PixelDmxHandler *m_pPixelDmxHandler { nullptr };

	uint32_t m_bIsStarted { 0 };
	bool m_bBlackout { false };
};

#endif /* WS28XXDMXMULTI_H_ */
