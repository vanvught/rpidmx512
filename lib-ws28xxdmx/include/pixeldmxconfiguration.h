/**
 * @file pixeldmxconfiguration.h
 *
 */
/* Copyright (C) 2021-2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef PIXELDMXCONFIGURATION_H_
#define PIXELDMXCONFIGURATION_H_

#include <cstdint>
#include <cstdio>
#include <algorithm>

#include "pixelconfiguration.h"

#include "debug.h"

namespace pixeldmxconfiguration {
struct PortInfo {
	uint16_t nBeginIndexPort[4];
	uint16_t nProtocolPortIndexLast;
};
}  // namespace pixeldmxconfiguration

class PixelDmxConfiguration: public PixelConfiguration {
public:
	void SetOutputPorts(const uint16_t nOutputPorts) {
		m_nOutputPorts = nOutputPorts;
	}

	uint32_t GetOutputPorts() const {
		return m_nOutputPorts;
	}

	void SetGroupingCount(const uint16_t nGroupingCount) {
		m_nGroupingCount = nGroupingCount;
	}

	uint32_t GetGroupingCount() const {
		return m_nGroupingCount;
	}

	uint32_t GetGroups() const {
		return m_nGroups;
	}

	uint32_t GetUniverses() const {
		return m_nUniverses;
	}

	void SetDmxStartAddress(const uint16_t nDmxStartAddress) {
		m_nDmxStartAddress = nDmxStartAddress;
	}

	uint32_t GetDmxStartAddress() const {
		return m_nDmxStartAddress;
	}

	uint32_t GetDmxFootprint() const {
		return m_nDmxFootprint;
	}

	void Validate(const uint32_t nPortsMax, uint32_t& nLedsPerPixel, pixeldmxconfiguration::PortInfo& portInfo) {
		DEBUG_ENTRY

		PixelConfiguration::Validate(nLedsPerPixel);

		if (!IsRTZProtocol()) {
			const auto type = GetType();
			if (!((type == pixel::Type::WS2801) || (type == pixel::Type::APA102) || (type == pixel::Type::SK9822))) {
				SetType(pixel::Type::WS2801);
			}

			PixelConfiguration::Validate(nLedsPerPixel);
		}

		portInfo.nBeginIndexPort[0] = 0;

		if (GetType() == pixel::Type::SK6812W) {
			portInfo.nBeginIndexPort[1] = 128;
			portInfo.nBeginIndexPort[2] = 256;
			portInfo.nBeginIndexPort[3] = 384;
		} else {
			portInfo.nBeginIndexPort[1] = 170;
			portInfo.nBeginIndexPort[2] = 340;
			portInfo.nBeginIndexPort[3] = 510;
		}

		if ((m_nGroupingCount == 0) || (m_nGroupingCount > GetCount())) {
			m_nGroupingCount = PixelConfiguration::GetCount();
		}

		m_nGroups = PixelConfiguration::GetCount() / m_nGroupingCount;
		m_nOutputPorts = std::min(nPortsMax, m_nOutputPorts);
		m_nUniverses = static_cast<uint16_t>(1U + (m_nGroups  / (1U + portInfo.nBeginIndexPort[1])));
		m_nDmxFootprint = static_cast<uint16_t>(nLedsPerPixel * m_nGroups);

		if (nPortsMax == 1) {
			portInfo.nProtocolPortIndexLast = static_cast<uint16_t>(m_nGroups / (1U + portInfo.nBeginIndexPort[1]));
		} else {
#if defined (NODE_DDP_DISPLAY)
			portInfo.nProtocolPortIndexLast = static_cast<uint16_t>(((m_nOutputPorts - 1U) * 4U) + m_nUniverses - 1U);
#else
			portInfo.nProtocolPortIndexLast = static_cast<uint16_t>((m_nOutputPorts * m_nUniverses)  - 1U);
#endif
		}

		DEBUG_PRINTF("portInfo.nProtocolPortIndexLast=%u", portInfo.nProtocolPortIndexLast);
		DEBUG_EXIT
	}

	void Print() {
		PixelConfiguration::Print();

		puts("Pixel DMX configuration");
		printf(" Outputs : %d\n", m_nOutputPorts);
		printf(" Grouping count : %d [Groups : %d]\n", m_nGroupingCount, m_nGroups);
	}

private:
	uint32_t m_nOutputPorts { 1 };
	uint32_t m_nGroupingCount { 1 };
	uint32_t m_nGroups { pixel::defaults::COUNT };
	uint32_t m_nUniverses;
	uint32_t m_nDmxStartAddress { 1 };
	uint32_t m_nDmxFootprint;
};

#endif /* PIXELDMXCONFIGURATION_H_ */
