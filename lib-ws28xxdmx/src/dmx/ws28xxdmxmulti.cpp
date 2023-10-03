/**
 * @file ws28xxdmxmulti.cpp
 *
 */
/* Copyright (C) 2019-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#pragma GCC push_options
#pragma GCC optimize ("O3")
#pragma GCC optimize ("-funroll-loops")
#pragma GCC optimize ("-fprefetch-loop-arrays")

#include <cstdint>
#include <algorithm>
#include <cassert>

#include "ws28xxdmxmulti.h"
#include "ws28xxmulti.h"
#include "ws28xx.h"

#include "pixeldmxparams.h"
#include "pixeldmxconfiguration.h"

#include "debug.h"

namespace ws28xxdmxmulti {
#if !defined (CONFIG_PIXELDMX_MAX_PORTS)
# define CONFIG_PIXELDMX_MAX_PORTS	8
#endif
static constexpr auto MAX_PORTS = CONFIG_PIXELDMX_MAX_PORTS;
}  // namespace ws28xxdmxmulti

WS28xxDmxMulti::WS28xxDmxMulti(PixelDmxConfiguration& pixelDmxConfiguration): m_pixelDmxConfiguration(pixelDmxConfiguration) {
	DEBUG_ENTRY

	m_pixelDmxConfiguration.Validate(ws28xxdmxmulti::MAX_PORTS , m_nChannelsPerPixel, m_PortInfo);

	DEBUG_PRINTF("m_PortInfo.nProtocolPortIndexLast=%u", m_PortInfo.nProtocolPortIndexLast);

	m_pWS28xxMulti = new WS28xxMulti(pixelDmxConfiguration);
	assert(m_pWS28xxMulti != nullptr);

	m_pWS28xxMulti->Blackout();

	DEBUG_EXIT
}

WS28xxDmxMulti::~WS28xxDmxMulti() {
	delete m_pWS28xxMulti;
	m_pWS28xxMulti = nullptr;
}

void WS28xxDmxMulti::Start(const uint32_t nPortIndex) {
	DEBUG_PRINTF("%u", nPortIndex);

	if (m_bIsStarted == 0) {
		if (m_pPixelDmxHandler != nullptr) {
			m_pPixelDmxHandler->Start();
		}
	}
	m_bIsStarted |= (1U << nPortIndex);
}

void WS28xxDmxMulti::Stop(const uint32_t nPortIndex) {
	DEBUG_PRINTF("%u", nPortIndex);

	if (m_bIsStarted & (1U << nPortIndex)) {
		m_bIsStarted &= ~(1U << nPortIndex);
	}

	if (m_bIsStarted == 0)  {
		if (m_pPixelDmxHandler != nullptr) {
			m_pPixelDmxHandler->Stop();
		}
	}
}

void WS28xxDmxMulti::SetData(uint32_t nPortIndex, const uint8_t* pData, uint32_t nLength, const bool doUpdate) {
	assert(pData != nullptr);
	assert(nLength <= lightset::dmx::UNIVERSE_SIZE);

#if defined (NODE_DDP_DISPLAY)
	const auto nOutIndex = (nPortIndex / 4);
	const auto nSwitch = nPortIndex - (nOutIndex * 4);
#else
	const auto nUniverses = m_pixelDmxConfiguration.GetUniverses();
	const auto nOutIndex = (nPortIndex / nUniverses);
	const auto nSwitch = nPortIndex - (nOutIndex * nUniverses);
#endif

	const auto nGroups = m_pixelDmxConfiguration.GetGroups();
	const auto beginIndex = m_PortInfo.nBeginIndexPort[nSwitch];
	const auto endIndex = std::min(nGroups, (beginIndex + (nLength / m_nChannelsPerPixel)));

	while (m_pWS28xxMulti->IsUpdating()) {
		// wait for completion
	}

	uint32_t d = 0;

	const auto nGroupingCount = m_pixelDmxConfiguration.GetGroupingCount();

	if (m_nChannelsPerPixel == 3) {
		switch (m_pixelDmxConfiguration.GetMap()) {
		case pixel::Map::RGB:
			for (uint32_t j = beginIndex; (j < endIndex) && (d < nLength); j++) {
				auto const nPixelIndexStart = (j * nGroupingCount);
				for (uint32_t k = 0; k < nGroupingCount; k++) {
					m_pWS28xxMulti->SetPixel(nOutIndex, nPixelIndexStart + k, pData[d + 0], pData[d + 1], pData[d + 2]);
				}
				d = d + 3;
			}
			break;
		case pixel::Map::RBG:
			for (uint32_t j = beginIndex; (j < endIndex) && (d < nLength); j++) {
				auto const nPixelIndexStart = (j * nGroupingCount);
				for (uint32_t k = 0; k < nGroupingCount; k++) {
					m_pWS28xxMulti->SetPixel(nOutIndex, nPixelIndexStart + k, pData[d + 0], pData[d + 2], pData[d + 1]);
				}
				d = d + 3;
			}
			break;
		case pixel::Map::GRB:
			for (uint32_t j = beginIndex; (j < endIndex) && (d < nLength); j++) {
				auto const nPixelIndexStart = (j * nGroupingCount);
				for (uint32_t k = 0; k < nGroupingCount; k++) {
					m_pWS28xxMulti->SetPixel(nOutIndex, nPixelIndexStart + k, pData[d + 1], pData[d + 0], pData[d + 2]);
				}
				d = d + 3;
			}
			break;
		case pixel::Map::GBR:
			for (uint32_t j = beginIndex; (j < endIndex) && (d < nLength); j++) {
				auto const nPixelIndexStart = (j * nGroupingCount);
				for (uint32_t k = 0; k < nGroupingCount; k++) {
					m_pWS28xxMulti->SetPixel(nOutIndex, nPixelIndexStart + k, pData[d + 2], pData[d + 0], pData[d + 1]);
				}
				d = d + 3;
			}
			break;
		case pixel::Map::BRG:
			for (uint32_t j = beginIndex; (j < endIndex) && (d < nLength); j++) {
				auto const nPixelIndexStart = (j * nGroupingCount);
				for (uint32_t k = 0; k < nGroupingCount; k++) {
					m_pWS28xxMulti->SetPixel(nOutIndex, nPixelIndexStart + k, pData[d + 1], pData[d + 2], pData[d + 0]);
				}
				d = d + 3;
			}
			break;
		case pixel::Map::BGR:
			for (uint32_t j = beginIndex; (j < endIndex) && (d < nLength); j++) {
				auto const nPixelIndexStart = (j * nGroupingCount);
				for (uint32_t k = 0; k < nGroupingCount; k++) {
					m_pWS28xxMulti->SetPixel(nOutIndex, nPixelIndexStart + k, pData[d + 2], pData[d + 1], pData[d + 0]);
				}
				d = d + 3;
			}
			break;
		default:
			assert(0);
			__builtin_unreachable();
			break;
		}
	} else {
		assert(m_nChannelsPerPixel == 4);
		for (uint32_t j = beginIndex; (j < endIndex) && (d < nLength); j++) {
			auto const nPixelIndexStart = (j * nGroupingCount);
			for (uint32_t k = 0; k < nGroupingCount; k++) {
				m_pWS28xxMulti->SetPixel(nOutIndex, nPixelIndexStart + k, pData[d], pData[d + 1], pData[d + 2], pData[d + 3]);
			}
			d = d + 4;
		}
	}

	if ((doUpdate) && (nPortIndex == m_PortInfo.nProtocolPortIndexLast)) {
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

void WS28xxDmxMulti::FullOn() {
	while (m_pWS28xxMulti->IsUpdating()) {
		// wait for completion
	}

	m_pWS28xxMulti->FullOn();
}
