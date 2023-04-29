/**
 * @file ddpdisplay.h
 *
 */
/* Copyright (C) 2021-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef DDPDISPLAY_H_
#define DDPDISPLAY_H_

#include <cstdint>
#include <algorithm>

#include "ddp.h"

#include "lightset.h"

#include "network.h"

#if !defined(LIGHTSET_PORTS)
# error LIGHTSET_PORTS is not defined
#endif

#if !defined (CONFIG_PIXELDMX_MAX_PORTS)
# error CONFIG_PIXELDMX_MAX_PORTS is not defined
#endif

namespace ddpdisplay {
namespace lightset {
static constexpr uint32_t MAX_PORTS = LIGHTSET_PORTS;
}  // namespace lightset
namespace configuration {
namespace pixel {
static constexpr uint32_t MAX_PORTS = CONFIG_PIXELDMX_MAX_PORTS;
}  // namespace pixel
namespace dmx {
#if defined OUTPUT_DMX_SEND_MULTI
 static constexpr uint32_t MAX_PORTS = 2;
#else
 static constexpr uint32_t MAX_PORTS = 0;
#endif
}  // namespace dmx
static constexpr uint32_t MAX_PORTS = configuration::pixel::MAX_PORTS + configuration::dmx::MAX_PORTS;
}  // namespace configuration
}  // namespace ddpdisplay

static_assert(ddpdisplay::lightset::MAX_PORTS == ddpdisplay::configuration::dmx::MAX_PORTS + ddpdisplay::configuration::pixel::MAX_PORTS * 4, "Configuration errror");

class DdpDisplay {
public:
	DdpDisplay();
	~DdpDisplay();

	void Start();
	void Stop();

	void Run();

	void Print();

	void SetCount(uint32_t nCount, uint32_t nChannelsPerPixel, uint32_t nActivePorts) {
		m_nCount = nCount;
		m_nStripDataLength = nCount * nChannelsPerPixel;
		m_nLightSetDataMaxLength = (nChannelsPerPixel == 4 ? 512U : 510U);
		m_nActivePorts = std::min(nActivePorts, ddpdisplay::configuration::pixel::MAX_PORTS);
	}

	uint32_t GetCount() const {
		return m_nCount;
	}

	uint32_t GetChannelsPerPixel() const {
		return m_nStripDataLength / m_nCount;
	}

	void SetOutput(LightSet *pLightSet) {
		m_pLightSet = pLightSet;
	}

	LightSet *GetOutput() const {
		return m_pLightSet;
	}

	static DdpDisplay* Get() {
		return s_pThis;
	}

private:
	void CalculateOffsets();
	void HandleQuery();
	void HandleData();

private:
	uint8_t m_macAddress[network::MAC_SIZE];
	int32_t m_nHandle { -1 };
	uint32_t m_nFromIp { 0 };
	uint32_t m_nCount { 0 };
	uint32_t m_nStripDataLength { 0 };
	uint32_t m_nLightSetDataMaxLength { 0 };
	uint32_t m_nActivePorts { 0 };

	LightSet *m_pLightSet { nullptr };

	ddp::Packet m_Packet;

	static uint32_t s_nLightsetPortLength[ddpdisplay::lightset::MAX_PORTS];
	static uint32_t s_nOffsetCompare[ddpdisplay::configuration::MAX_PORTS];

	static DdpDisplay *s_pThis;
};

#endif /* DDPDISPLAY_H_ */
