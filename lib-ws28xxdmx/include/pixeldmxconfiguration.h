/**
 * @file pixeldmxconfiguration.h
 *
 */
/* Copyright (C) 2021-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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
/**
 * Static Local Variables:
 * Since C++11, the initialization of function-local static variables, is guaranteed to be thread-safe.
 * This means that even if multiple threads attempt to access Get() simultaneously,
 * the C++ runtime ensures that the instance is initialized only once.
 */

#ifndef PIXELDMXCONFIGURATION_H_
#define PIXELDMXCONFIGURATION_H_

#include <cstdint>
#include <cstdio>
#include <algorithm>
#include <cassert>

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
	PixelDmxConfiguration() {
		DEBUG_ENTRY

    	assert(s_pThis == nullptr);
    	s_pThis = this;

    	DEBUG_EXIT
	}

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

	pixeldmxconfiguration::PortInfo& GetPortInfo() {
		return m_portInfo;
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

	void Validate(const uint32_t nPortsMax) {
		DEBUG_ENTRY

		PixelConfiguration::Validate();

		if (!PixelConfiguration::IsRTZProtocol()) {
			if (!((PixelConfiguration::GetType() == pixel::Type::WS2801) || (PixelConfiguration::GetType() == pixel::Type::APA102) || (PixelConfiguration::GetType() == pixel::Type::SK9822))) {
				PixelConfiguration::SetType(pixel::Type::WS2801);
			}

			PixelConfiguration::Validate();
		}

		m_portInfo.nBeginIndexPort[0] = 0;

		if (PixelConfiguration::GetType() == pixel::Type::SK6812W) {
			m_portInfo.nBeginIndexPort[1] = 128;
			m_portInfo.nBeginIndexPort[2] = 256;
			m_portInfo.nBeginIndexPort[3] = 384;
		} else {
			m_portInfo.nBeginIndexPort[1] = 170;
			m_portInfo.nBeginIndexPort[2] = 340;
			m_portInfo.nBeginIndexPort[3] = 510;
		}

		if ((m_nGroupingCount == 0) || (m_nGroupingCount > PixelConfiguration::GetCount())) {
			m_nGroupingCount = PixelConfiguration::GetCount();
		}

		m_nGroups = PixelConfiguration::GetCount() / m_nGroupingCount;
		m_nOutputPorts = std::min(nPortsMax, m_nOutputPorts);
		m_nUniverses = (1U + (m_nGroups  / (1U + m_portInfo.nBeginIndexPort[1])));
		m_nDmxFootprint = PixelConfiguration::GetLedsPerPixel() * m_nGroups;

		if (nPortsMax == 1) {
			m_portInfo.nProtocolPortIndexLast = static_cast<uint16_t>(m_nGroups / (1U + m_portInfo.nBeginIndexPort[1]));
		} else {
#if defined (NODE_DDP_DISPLAY)
			m_portInfo.nProtocolPortIndexLast = static_cast<uint16_t>(((m_nOutputPorts - 1U) * 4U) + m_nUniverses - 1U);
#else
			m_portInfo.nProtocolPortIndexLast = static_cast<uint16_t>((m_nOutputPorts * m_nUniverses)  - 1U);
#endif
		}

		DEBUG_EXIT
	}

	void Print() {
		PixelConfiguration::Print();
		puts("Pixel DMX configuration");
		printf(" Outputs        : %u\n", m_nOutputPorts);
		printf(" Grouping count : %u [Groups : %u]\n", m_nGroupingCount, m_nGroups);
		printf(" Universes      : %u\n", m_nUniverses);
		printf(" DmxFootprint   : %u\n", m_nDmxFootprint);

#ifndef NDEBUG
		const auto& beginIndexPort = m_portInfo.nBeginIndexPort;
		printf(" %u:%u:%u:%u -> %u\n", beginIndexPort[0], beginIndexPort[1], beginIndexPort[2], beginIndexPort[3], m_portInfo.nProtocolPortIndexLast);
#endif
	}

	static PixelDmxConfiguration& Get() {
		assert(s_pThis != nullptr); // Ensure that s_pThis is valid
		return *s_pThis;
	}

private:
	uint32_t m_nOutputPorts { 1 };
	uint32_t m_nGroupingCount { 1 };
	uint32_t m_nGroups { pixel::defaults::COUNT };
	uint32_t m_nUniverses { 0 };
	uint32_t m_nDmxStartAddress { 1 };
	uint32_t m_nDmxFootprint { 0 };
	pixeldmxconfiguration::PortInfo m_portInfo;

	static inline PixelDmxConfiguration *s_pThis { nullptr };
};

#endif /* PIXELDMXCONFIGURATION_H_ */
