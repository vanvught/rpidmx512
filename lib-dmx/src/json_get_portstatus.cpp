/**
 * @file json_get_portstatus.cpp
 *
 */
/* Copyright (C) 2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include "dmx.h"
#include "dmxconst.h"

namespace remoteconfig {
namespace dmx {
uint32_t json_get_portstatus(const char cPort, char *pOutBuffer, const uint32_t nOutBufferSize) {
	const uint32_t nPortIndex = (cPort | 0x20) - 'a';

	if (nPortIndex < ::dmx::config::max::PORTS) {
		auto& statistics = Dmx::Get()->GetTotalStatistics(nPortIndex);
		auto nLength = static_cast<uint32_t>(snprintf(pOutBuffer, nOutBufferSize,
				"{\"port\":\"%c\","
				"\"dmx\":{\"sent\":\"%u\",\"received\":\"%u\"},"
				"\"rdm\":{\"sent\":{\"class\":\"%u\",\"discovery\":\"%u\"},\"received\":{\"good\":\"%u\",\"bad\":\"%u\",\"discovery\":\"%u\"}}}",
				static_cast<char>('A' + nPortIndex),
				static_cast<unsigned int>(statistics.Dmx.Sent),
				static_cast<unsigned int>(statistics.Dmx.Received),
				static_cast<unsigned int>(statistics.Rdm.Sent.Class),
				static_cast<unsigned int>(statistics.Rdm.Sent.DiscoveryResponse),
				static_cast<unsigned int>(statistics.Rdm.Received.Good),
				static_cast<unsigned int>(statistics.Rdm.Received.Bad),
				static_cast<unsigned int>(statistics.Rdm.Received.DiscoveryResponse)));

		return nLength;
	}

	return 0;
}
}  // namespace dmx
}  // namespace remoteconfig
