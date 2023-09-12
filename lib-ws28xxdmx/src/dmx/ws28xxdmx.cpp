/**
 * @file ws28xxdmx.cpp
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

#pragma GCC push_options
#pragma GCC optimize ("O3")
#pragma GCC optimize ("-funroll-loops")
#pragma GCC optimize ("-fprefetch-loop-arrays")

#include <cstdint>
#include <algorithm>
#include <cassert>

#include "ws28xxdmx.h"
#include "ws28xx.h"

#include "lightset.h"

#include "pixeldmxconfiguration.h"

#include "debug.h"

WS28xxDmx *WS28xxDmx::s_pThis;

WS28xxDmx::WS28xxDmx(PixelDmxConfiguration& pixelDmxConfiguration): m_pixelDmxConfiguration(pixelDmxConfiguration) {
	DEBUG_ENTRY

	assert(s_pThis == nullptr);
	s_pThis = this;

	m_pixelDmxConfiguration.Validate(1 , m_nChannelsPerPixel, m_PortInfo);

	m_pWS28xx = new WS28xx(m_pixelDmxConfiguration);
	assert(m_pWS28xx != nullptr);

	m_pWS28xx->Blackout();

	m_nDmxStartAddress = m_pixelDmxConfiguration.GetDmxStartAddress();
	m_nDmxFootprint = static_cast<uint16_t>(m_nChannelsPerPixel * m_pixelDmxConfiguration.GetGroups());

	DEBUG_EXIT
}

WS28xxDmx::~WS28xxDmx() {
	DEBUG_ENTRY

	delete m_pWS28xx;
	m_pWS28xx = nullptr;

	DEBUG_EXIT
}

void WS28xxDmx::Start(__attribute__((unused)) uint32_t nPortIndex) {
	if (m_bIsStarted) {
		return;
	}

	m_bIsStarted = true;

	if (m_pPixelDmxHandler != nullptr) {
		m_pPixelDmxHandler->Start();
	}
}

void WS28xxDmx::Stop(__attribute__((unused)) uint32_t nPortIndex) {
	if (!m_bIsStarted) {
		return;
	}

	m_bIsStarted = false;

	if (m_pPixelDmxHandler != nullptr) {
		m_pPixelDmxHandler->Stop();
	}
}

void WS28xxDmx::SetData(uint32_t nPortIndex, const uint8_t *pData, uint32_t nLength, const bool doUpdate) {
	assert(pData != nullptr);
	assert(nLength <= lightset::dmx::UNIVERSE_SIZE);

	if (m_pWS28xx->IsUpdating()) {
		return;
	}

	uint32_t d = 0;

	const auto nSwitch = nPortIndex & 0x03;
	const auto nGroups = m_pixelDmxConfiguration.GetGroups();
	const auto beginIndex = m_PortInfo.nBeginIndexPort[nSwitch];
	const auto endIndex = std::min(nGroups, (beginIndex + (nLength / m_nChannelsPerPixel)));

	if ((nSwitch == 0) && (nGroups < m_PortInfo.nBeginIndexPort[1])) {
		d = static_cast<uint32_t>(m_pixelDmxConfiguration.GetDmxStartAddress() - 1);
	}

	const auto nGroupingCount = m_pixelDmxConfiguration.GetGroupingCount();

	if (m_nChannelsPerPixel == 3) {
		switch (m_pixelDmxConfiguration.GetMap()) {
		case pixel::Map::RGB:
			for (uint32_t j = beginIndex; (j < endIndex) && (d < nLength); j++) {
				auto const nPixelIndexStart = (j * nGroupingCount);
				for (uint32_t k = 0; k < nGroupingCount; k++) {
					m_pWS28xx->SetPixel(nPixelIndexStart + k, pData[d + 0], pData[d + 1], pData[d + 2]);
				}
				d = d + 3;
			}
			break;
		case pixel::Map::RBG:
			for (uint32_t j = beginIndex; (j < endIndex) && (d < nLength); j++) {
				auto const nPixelIndexStart = (j * nGroupingCount);
				for (uint32_t k = 0; k < nGroupingCount; k++) {
					m_pWS28xx->SetPixel(nPixelIndexStart + k, pData[d + 0], pData[d + 2], pData[d + 1]);
				}
				d = d + 3;
			}
			break;
		case pixel::Map::GRB:
			for (uint32_t j = beginIndex; (j < endIndex) && (d < nLength); j++) {
				auto const nPixelIndexStart = (j * nGroupingCount);
				for (uint32_t k = 0; k < nGroupingCount; k++) {
					m_pWS28xx->SetPixel(nPixelIndexStart + k, pData[d + 1], pData[d + 0], pData[d + 2]);
				}
				d = d + 3;
			}
			break;
		case pixel::Map::GBR:
			for (uint32_t j = beginIndex; (j < endIndex) && (d < nLength); j++) {
				auto const nPixelIndexStart = (j * nGroupingCount);
				for (uint32_t k = 0; k < nGroupingCount; k++) {
					m_pWS28xx->SetPixel(nPixelIndexStart + k, pData[d + 2], pData[d + 0], pData[d + 1]);
				}
				d = d + 3;
			}
			break;
		case pixel::Map::BRG:
			for (uint32_t j = beginIndex; (j < endIndex) && (d < nLength); j++) {
				auto const nPixelIndexStart = (j * nGroupingCount);
				for (uint32_t k = 0; k < nGroupingCount; k++) {
					m_pWS28xx->SetPixel(nPixelIndexStart + k, pData[d + 1], pData[d + 2], pData[d + 0]);
				}
				d = d + 3;
			}
			break;
		case pixel::Map::BGR:
			for (uint32_t j = beginIndex; (j < endIndex) && (d < nLength); j++) {
				auto const nPixelIndexStart = (j * nGroupingCount);
				for (uint32_t k = 0; k < nGroupingCount; k++) {
					m_pWS28xx->SetPixel(nPixelIndexStart + k, pData[d + 2], pData[d + 1], pData[d + 0]);
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
		for (auto j = beginIndex; (j < endIndex) && (d < nLength); j++) {
			auto const nPixelIndexStart = (j * nGroupingCount);
			for (uint32_t k = 0; k < nGroupingCount; k++) {
				m_pWS28xx->SetPixel(nPixelIndexStart + k, pData[d], pData[d + 1], pData[d + 2], pData[d + 3]);
			}
			d = d + 4;
		}
	}

	if ((doUpdate) && (nPortIndex == m_PortInfo.nProtocolPortIndexLast)) {
		if (__builtin_expect((m_bBlackout), 0)) {
			return;
		}
		m_pWS28xx->Update();
	}
}

void WS28xxDmx::Blackout(bool bBlackout) {
	m_bBlackout = bBlackout;

	while (m_pWS28xx->IsUpdating()) {
		// wait for completion
	}

	if (bBlackout) {
		m_pWS28xx->Blackout();
	} else {
		m_pWS28xx->Update();
	}
}

void WS28xxDmx::FullOn() {
	while (m_pWS28xx->IsUpdating()) {
		// wait for completion
	}

	m_pWS28xx->FullOn();
}

// DMX

bool WS28xxDmx::SetDmxStartAddress(uint16_t nDmxStartAddress) {
	assert((nDmxStartAddress != 0) && (nDmxStartAddress <= lightset::dmx::UNIVERSE_SIZE));

	if (nDmxStartAddress == m_nDmxStartAddress) {
		return true;
	}

	if ((nDmxStartAddress + m_nDmxFootprint) > lightset::dmx::UNIVERSE_SIZE) {
		return false;
	}

	if ((nDmxStartAddress != 0) && (nDmxStartAddress <= lightset::dmx::UNIVERSE_SIZE)) {
		m_nDmxStartAddress = nDmxStartAddress;

		if (m_pWS28xxDmxStore != nullptr) {
			m_pWS28xxDmxStore->SaveDmxStartAddress(m_nDmxStartAddress);
		}

		return true;
	}

	return false;
}

// RDM

bool WS28xxDmx::GetSlotInfo(uint16_t nSlotOffset, lightset::SlotInfo& slotInfo) {
	if (nSlotOffset >  m_nDmxFootprint) {
		return false;
	}

	slotInfo.nType = 0x00;	// ST_PRIMARY

	switch (nSlotOffset % m_nChannelsPerPixel) {
		case 0:
			slotInfo.nCategory = 0x0205; // SD_COLOR_ADD_RED
			break;
		case 1:
			slotInfo.nCategory = 0x0206; // SD_COLOR_ADD_GREEN
			break;
		case 2:
			slotInfo.nCategory = 0x0207; // SD_COLOR_ADD_BLUE
			break;
		case 3:
			slotInfo.nCategory = 0x0212; // SD_COLOR_ADD_WHITE
			break;
		default:
			__builtin_unreachable();
			break;
	}

	return true;
}
