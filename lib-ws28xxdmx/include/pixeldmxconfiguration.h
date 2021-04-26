/**
 * @file pixeldmxconfiguration.h
 *
 */
/* Copyright (C) 2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <stdint.h>

#include "pixelconfiguration.h"

namespace pixeldmxconfiguration {
struct PortInfo {
	uint32_t nBeginIndexPortId1;
	uint32_t nBeginIndexPortId2;
	uint32_t nBeginIndexPortId3;
	uint32_t nProtocolPortIdLast;
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

	void SetGroupingEnabled(bool isEnabled) {
		m_isGroupingEnabled = isEnabled;
	}

	bool GetGroupingEnabled() const {
		return m_isGroupingEnabled;
	}

	void SetGroupingCount(uint32_t nGroupingCount) {
		m_nGroupingCount = nGroupingCount;
	}

	uint32_t GetGroupingCount() const {
		return m_nGroupingCount;
	}

	void Validate(uint32_t nPortsMax, uint32_t& nLedsPerPixel, pixeldmxconfiguration::PortInfo& portInfo, uint32_t& nGroups, uint32_t& nUniverses);

	void Dump();

private:
	uint32_t m_nOutputPorts { 1 };
	bool m_isGroupingEnabled { false };
	uint32_t m_nGroupingCount { 1 };
};

#endif /* PIXELDMXCONFIGURATION_H_ */
