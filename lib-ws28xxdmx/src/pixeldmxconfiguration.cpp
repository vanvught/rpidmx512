/**
 * @file pixeldmxconfiguration.cpp
 *
 */
/* Copyright (C) 2021-2022 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#if !defined(__clang__)	// Needed for compiling on MacOS
# pragma GCC push_options
# pragma GCC optimize ("Os")
#endif

#include <cstdint>
#include <algorithm>

#include "pixeldmxconfiguration.h"

#include "debug.h"

#if defined (NODE_ARTNET_MULTI)
# define NODE_ARTNET
#endif

using namespace pixel;

void PixelDmxConfiguration::Validate(uint32_t nPortsMax, uint32_t& nLedsPerPixel, pixeldmxconfiguration::PortInfo& portInfo) {
	DEBUG_ENTRY

	PixelConfiguration::Validate(nLedsPerPixel);

	if (!IsRTZProtocol()) {
		const auto type = GetType();
		if (!((type == Type::WS2801) || (type == Type::APA102) || (type == Type::SK9822))) {
			SetType(Type::WS2801);
		}

		PixelConfiguration::Validate(nLedsPerPixel);
	}

	if (GetType() == Type::SK6812W) {
		portInfo.nBeginIndexPortId1 = 128;
		portInfo.nBeginIndexPortId2 = 256;
		portInfo.nBeginIndexPortId3 = 384;
	} else {
		portInfo.nBeginIndexPortId1 = 170;
		portInfo.nBeginIndexPortId2 = 340;
		portInfo.nBeginIndexPortId3 = 510;
	}

	if ((m_nGroupingCount == 0) || (m_nGroupingCount > GetCount())) {
		m_nGroupingCount = GetCount();
	}

	m_nGroups = GetCount() / m_nGroupingCount;

	m_nOutputPorts = std::min(nPortsMax, m_nOutputPorts);
	m_nUniverses = (1U + (m_nGroups  / (1U + portInfo.nBeginIndexPortId1)));

	if (nPortsMax == 1) {
		portInfo.nProtocolPortIndexLast = (m_nGroups / (1U + portInfo.nBeginIndexPortId1));
	} else {
#if defined (NODE_ARTNET) || defined (NODE_DDP_DISPLAY)
		portInfo.nProtocolPortIndexLast = (((m_nOutputPorts - 1U) * 4U) + m_nUniverses - 1U);
#else
		portInfo.nProtocolPortIndexLast = ((m_nOutputPorts * m_nUniverses)  - 1U);
#endif
	}

	DEBUG_PRINTF("portInfo.nProtocolPortIndexLast=%u", portInfo.nProtocolPortIndexLast);
	DEBUG_EXIT
}

#include <cstdio>

void PixelDmxConfiguration::Print() {
	PixelConfiguration::Print();

	printf("Pixel DMX configuration\n");
	printf(" Outputs : %d\n", m_nOutputPorts);
	printf(" Grouping count : %d [Groups : %d]\n", m_nGroupingCount, m_nGroups);
}
