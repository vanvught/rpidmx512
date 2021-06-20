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

#include <cstdint>
#include <cstdio>
#include <algorithm>
#include <cassert>

#include "ws28xxdmxmulti.h"
#include "ws28xxmulti.h"
#include "ws28xxdmxparams.h"
#include "ws28xx.h"

#include "pixeldmxconfiguration.h"

#include "debug.h"

using namespace ws28xxdmxmulti;
using namespace pixel;
using namespace lightset;

static uint8_t s_StopBuffer[Dmx::UNIVERSE_SIZE] = {0};

WS28xxDmxMulti::WS28xxDmxMulti(PixelDmxConfiguration& pixelDmxConfiguration) {
	DEBUG_ENTRY

	pixelDmxConfiguration.Validate(8 , m_nChannelsPerPixel, m_PortInfo, m_nGroups, m_nUniverses);

	DEBUG_PRINTF("m_nChannelsPerPixel=%u, m_nGroups=%u, m_nUniverses=%u", m_nChannelsPerPixel, m_nGroups, m_nUniverses);

	m_pWS28xxMulti = new WS28xxMulti(pixelDmxConfiguration);
	assert(m_pWS28xxMulti != nullptr);

	m_pWS28xxMulti->Blackout();

	m_nGroupingCount = pixelDmxConfiguration.GetGroupingCount();
	m_nOutputPorts = pixelDmxConfiguration.GetOutputPorts();

	pixelDmxConfiguration.Dump();

	DEBUG_EXIT
}

WS28xxDmxMulti::~WS28xxDmxMulti() {
	delete m_pWS28xxMulti;
	m_pWS28xxMulti = nullptr;
}

void WS28xxDmxMulti::Start(uint32_t nPortIndex) {
	DEBUG_PRINTF("%u", nPortIndex);

	if (m_bIsStarted == 0) {
		if (m_pPixelDmxHandler != nullptr) {
			m_pPixelDmxHandler->Start();
		}
	}
	m_bIsStarted |= (1U << nPortIndex);
}

void WS28xxDmxMulti::Stop(uint32_t nPortIndex) {
	DEBUG_PRINTF("%u", nPortIndex);

	if (m_bIsStarted & (1U << nPortIndex)) {
		SetData(nPortIndex, s_StopBuffer, sizeof(s_StopBuffer));
		m_bIsStarted &= ~(1U << nPortIndex);
	}

	if (m_bIsStarted == 0)  {
		if (m_pPixelDmxHandler != nullptr) {
			m_pPixelDmxHandler->Stop();
		}
	}
}

void WS28xxDmxMulti::SetData(uint32_t nPortIndex, const uint8_t* pData, uint32_t nLength) {
	assert(pData != nullptr);
	assert(nLength <= Dmx::UNIVERSE_SIZE);

	uint32_t beginIndex, endIndex;

#if defined (NODE_ARTNET_MULTI)
	const auto nOutIndex = (nPortIndex / 4);
	const auto nSwitch = nPortIndex - (nOutIndex * 4);
#else
	const auto nOutIndex = (nPortIndex / m_nUniverses);
	const auto nSwitch = nPortIndex - (nOutIndex * m_nUniverses);
#endif

	switch (nSwitch) {
	case 0:
		beginIndex = 0;
		endIndex = std::min(m_nGroups, (nLength / m_nChannelsPerPixel));
		break;
	case 1:
		beginIndex = m_PortInfo.nBeginIndexPortId1;
		endIndex = std::min(m_nGroups, (beginIndex + (nLength / m_nChannelsPerPixel)));
		break;
	case 2:
		beginIndex = m_PortInfo.nBeginIndexPortId2;
		endIndex = std::min(m_nGroups, (beginIndex + (nLength / m_nChannelsPerPixel)));
		break;
	case 3:
		beginIndex = m_PortInfo.nBeginIndexPortId3;
		endIndex = std::min(m_nGroups, (beginIndex + (nLength / m_nChannelsPerPixel)));
		break;
	default:
		__builtin_unreachable();
		break;
	}

#if 0
	DEBUG_PRINTF("nPort=%d, nLength=%d, nOutIndex=%d, nSwitch=%d, beginIndex=%d, endIndex=%d",
			static_cast<int>(nPortIndex), static_cast<int>(nLength), static_cast<int>(nOutIndex),
			static_cast<int>(nSwitch), static_cast<int>(beginIndex), static_cast<int>(endIndex));
#endif

	while (m_pWS28xxMulti->IsUpdating()) {
		// wait for completion
	}

	uint32_t d = 0;

	if (m_nChannelsPerPixel == 3) {
		for (uint32_t j = beginIndex; (j < endIndex) && (d < nLength); j++) {
			auto const nPixelIndexStart = (j * m_nGroupingCount);
			__builtin_prefetch(&pData[d]);
			for (uint16_t k = 0; k < m_nGroupingCount; k++) {
				m_pWS28xxMulti->SetPixel(nOutIndex, nPixelIndexStart + k, pData[d], pData[d + 1], pData[d + 2]);
			}
			d = d + 3;
		}
	} else {
		assert(m_nChannelsPerPixel == 4);
		for (uint32_t j = beginIndex; (j < endIndex) && (d < nLength); j++) {
			auto const nPixelIndexStart = (j * m_nGroupingCount);
			__builtin_prefetch(&pData[d]);
			for (uint16_t k = 0; k < m_nGroupingCount; k++) {
				m_pWS28xxMulti->SetPixel(nOutIndex, nPixelIndexStart + k, pData[d], pData[d + 1], pData[d + 2], pData[d + 3]);
			}
			d = d + 4;
		}
	}

	if (nPortIndex == m_PortInfo.nProtocolPortIndexLast) {
		m_pWS28xxMulti->Update();
	}
}

void WS28xxDmxMulti::Blackout(bool bBlackout) {
	m_bBlackout = bBlackout;

	while (m_pWS28xxMulti->IsUpdating()) {
		// wait for completion
	}

	if (bBlackout) {
		m_pWS28xxMulti->Blackout();
	} else {
		m_pWS28xxMulti->Update();
	}
}

void WS28xxDmxMulti::Print() {
	m_pWS28xxMulti->Print();

	printf("Pixel DMX parameters\n");
	printf(" Outputs : %d\n", m_nOutputPorts);
	printf(" Grouping count : %d [Groups : %d]\n", m_nGroupingCount, m_nGroups);
}
