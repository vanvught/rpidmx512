/**
 * @file pixeldmxconfiguration.h
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

#ifndef PIXELDMXCONFIGURATION_H_
#define PIXELDMXCONFIGURATION_H_

#include <cstdint>

#include "pixelconfiguration.h"

namespace pixeldmxconfiguration {
struct PortInfo {
	uint32_t nBeginIndexPort[4];
	uint32_t nProtocolPortIndexLast;
};
}  // namespace pixeldmxconfiguration

class PixelDmxConfiguration: public PixelConfiguration {
public:
	void SetOutputPorts(uint32_t nOutputPorts) {
		m_nOutputPorts = nOutputPorts;
	}

	uint32_t GetOutputPorts() const {
		return m_nOutputPorts;
	}

	void SetGroupingCount(uint16_t nGroupingCount) {
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

	void SetDmxStartAddress(uint16_t nDmxStartAddress) {
		m_nDmxStartAddress = nDmxStartAddress;
	}

	uint16_t GetDmxStartAddress() const {
		return m_nDmxStartAddress;
	}

	void Validate(uint32_t nPortsMax, uint32_t& nLedsPerPixel, pixeldmxconfiguration::PortInfo& portInfo);

	void Print();

private:
	uint32_t m_nOutputPorts { 1 };
	uint32_t m_nGroupingCount { 1 };
	uint32_t m_nGroups { pixel::defaults::COUNT };
	uint32_t m_nUniverses;
	uint16_t m_nDmxStartAddress { 1 };
};

#endif /* PIXELDMXCONFIGURATION_H_ */
