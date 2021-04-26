/**
 * @file ws28xxdmx.cpp
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

#include <stdint.h>
#include <algorithm>
#include <cassert>

#ifndef NDEBUG
#if (__linux__)
# include <stdio.h>
#endif
#endif

#include "ws28xxdmx.h"
#include "ws28xx.h"

#include "lightset.h"

#include "pixeldmxconfiguration.h"

#include "debug.h"

using namespace pixel;
using namespace lightset;

WS28xxDmx::WS28xxDmx(PixelDmxConfiguration& pixelDmxConfiguration) {
	DEBUG_ENTRY

	pixelDmxConfiguration.Validate(1 , m_nChannelsPerPixel, m_PortInfo, m_nGroups, m_nUniverses);

	DEBUG_PRINTF("m_nChannelsPerPixel=%u, m_nGroups=%u, m_nUniverses=%u", m_nChannelsPerPixel, m_nGroups, m_nUniverses);

	m_pWS28xx = new WS28xx(pixelDmxConfiguration);
	assert(m_pWS28xx != nullptr);

	m_pWS28xx->Blackout();

	m_nGroupingCount = pixelDmxConfiguration.GetGroupingCount();
	pixelDmxConfiguration.Dump();

	DEBUG_EXIT
}

WS28xxDmx::~WS28xxDmx() {
	delete m_pWS28xx;
	m_pWS28xx = nullptr;
}

void WS28xxDmx::Start(__attribute__((unused)) uint8_t nPort) {
	if (m_bIsStarted) {
		return;
	}

	m_bIsStarted = true;

	if (m_pLightSetHandler != nullptr) {
		m_pLightSetHandler->Start();
	}
}

void WS28xxDmx::Stop(__attribute__((unused)) uint8_t nPort) {
	if (!m_bIsStarted) {
		return;
	}

	m_bIsStarted = false;

	if (m_pWS28xx != nullptr) {
		while (m_pWS28xx->IsUpdating()) {
			// wait for completion
		}
		m_pWS28xx->Blackout();
	}

	if (m_pLightSetHandler != nullptr) {
		m_pLightSetHandler->Stop();
	}
}

void WS28xxDmx::SetData(uint8_t nPortId, const uint8_t *pData, uint16_t nLength) {
	assert(pData != nullptr);
	assert(nLength <= Dmx::UNIVERSE_SIZE);

	uint32_t d = 0;
	uint32_t beginIndex, endIndex;

	switch (nPortId & 0x03) {
	case 0:
		beginIndex = 0;
		endIndex = std::min(m_nGroups, (nLength / m_nChannelsPerPixel));
		if (m_nGroups < m_PortInfo.nBeginIndexPortId1) {
			d = static_cast<uint32_t>(m_nDmxStartAddress - 1);
		}
		break;
	case 1:
		beginIndex = m_PortInfo.nBeginIndexPortId1;
		endIndex = std::min(m_nGroups, (beginIndex + (nLength /  m_nChannelsPerPixel)));
		break;
	case 2:
		beginIndex = m_PortInfo.nBeginIndexPortId2;
		endIndex = std::min(m_nGroups, (beginIndex + (nLength /  m_nChannelsPerPixel)));
		break;
	case 3:
		beginIndex = m_PortInfo.nBeginIndexPortId3;
		endIndex = std::min(m_nGroups, (beginIndex + (nLength /  m_nChannelsPerPixel)));
		break;
	default:
		__builtin_unreachable();
		break;
	}

#ifndef NDEBUG
#if defined (__linux__)
	printf("%d-%d:%x %x %x-%d", nPortId, m_nDmxStartAddress, pData[0], pData[1], pData[2], nLength);
#endif
#endif

	while (m_pWS28xx->IsUpdating()) {
		// wait for completion
	}

	if (m_nChannelsPerPixel == 3) {
		for (uint32_t j = beginIndex; (j < endIndex) && (d < nLength); j++) {
			auto const nPixelIndexStart = j * m_nGroupingCount;
			__builtin_prefetch(&pData[d]);
			for (uint32_t k = 0; k < m_nGroupingCount; k++) {
				m_pWS28xx->SetPixel(nPixelIndexStart + k, pData[d], pData[d + 1], pData[d + 2]);
			}
			d = d + 3;
		}
	} else {
		for (uint32_t j = beginIndex; (j < endIndex) && (d < nLength); j++) {
			auto const nPixelIndexStart = j * m_nGroupingCount;
			__builtin_prefetch(&pData[d]);
			for (uint32_t k = 0; k < m_nGroupingCount; k++) {
				m_pWS28xx->SetPixel(nPixelIndexStart + k, pData[d], pData[d + 1], pData[d + 2], pData[d + 3]);
			}
			d = d + 4;
		}
	}

	if (nPortId == m_PortInfo.nProtocolPortIdLast) {
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

// DMX

bool WS28xxDmx::SetDmxStartAddress(uint16_t nDmxStartAddress) {
	assert((nDmxStartAddress != 0) && (nDmxStartAddress <= Dmx::UNIVERSE_SIZE));

	//FIXME Footprint

	if ((nDmxStartAddress != 0) && (nDmxStartAddress <= Dmx::UNIVERSE_SIZE)) {
		m_nDmxStartAddress = nDmxStartAddress;

		if (m_pWS28xxDmxStore != nullptr) {
			m_pWS28xxDmxStore->SaveDmxStartAddress(m_nDmxStartAddress);
		}

		if (m_pLightSetDisplay != nullptr) {
			m_pLightSetDisplay->ShowDmxStartAddress();
		}

		return true;
	}

	return false;
}

// RDM

#define MOD(a,b)	(static_cast<unsigned>(a) - b * (static_cast<unsigned>(a)/b))

bool WS28xxDmx::GetSlotInfo(uint16_t nSlotOffset, SlotInfo& tSlotInfo) {
	unsigned nIndex;

	if (nSlotOffset >  m_nDmxFootprint) {
		return false;
	}

	if (m_tLedType == Type::SK6812W) {
		nIndex = MOD(nSlotOffset, 4);
	} else {
		nIndex = MOD(nSlotOffset, 3);
	}

	tSlotInfo.nType = 0x00;	// ST_PRIMARY

	switch (nIndex) {
		case 0:
			tSlotInfo.nCategory = 0x0205; // SD_COLOR_ADD_RED
			break;
		case 1:
			tSlotInfo.nCategory = 0x0206; // SD_COLOR_ADD_GREEN
			break;
		case 2:
			tSlotInfo.nCategory = 0x0207; // SD_COLOR_ADD_BLUE
			break;
		case 3:
			tSlotInfo.nCategory = 0x0212; // SD_COLOR_ADD_WHITE
			break;
		default:
			break;
	}

	return true;
}

