/**
 * @file json_get_portstatus.cpp
 *
 */
/* Copyright (C) 2023-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include "artnetnode.h"
#include "lightset.h"

namespace remoteconfig {
namespace rdm {
static uint32_t get_portstatus(const uint32_t nPortIndex, char *pOutBuffer, const uint32_t nOutBufferSize) {
	const auto direction = ArtNetNode::Get()->GetPortDirection(nPortIndex);
	const char *status;

	if (direction == lightset::PortDir::OUTPUT) {
		if (ArtNetNode::Get()->GetRdm(nPortIndex)) {
			if (ArtNetNode::Get()->GetRdmDiscovery(nPortIndex)) {
				bool bIsIncremental;
				if (ArtNetNode::Get()->RdmIsRunning(nPortIndex, bIsIncremental)) {
					status = bIsIncremental ? "Incremental" : "Full";
				} else {
					status = "Idle";
				}
			} else {
				status = "Disabled";
			}
		} else {
			return 0;
		}
	} else if (direction == lightset::PortDir::INPUT) {
		if (ArtNetNode::Get()->RdmGetUidCount(nPortIndex) != 0) {
			status = "TOD";
		} else {
			return 0;
		}
	} else {
		return 0;
	}

	auto nLength = static_cast<uint32_t>(snprintf(pOutBuffer, nOutBufferSize,
			"{\"port\":\"%c\",\"direction\":\"%s\",\"status\":\"%s\"},",
			static_cast<char>('A' + nPortIndex),
			lightset::get_direction(direction),
			status));

	return nLength;
}

uint32_t json_get_portstatus(char *pOutBuffer, const uint32_t nOutBufferSize) {
	pOutBuffer[0] = '[';
	uint32_t nLength = 1;

	for (uint32_t nPortIndex = 0; nPortIndex < artnetnode::MAX_PORTS; nPortIndex++) {
		nLength += get_portstatus(nPortIndex, &pOutBuffer[nLength], nOutBufferSize - nLength);
	}

	if (nLength == 1) {
		nLength++;
	}

	pOutBuffer[nLength - 1] = ']';

	return nLength;
}
}  // namespace rdm
}  // namespace remoteconfig
