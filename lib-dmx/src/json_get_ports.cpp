/**
 * @file json_get_ports.cpp
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
#include "lightset.h"

namespace remoteconfig {
namespace dmx {
static uint32_t get_portstatus(const uint32_t nPortIndex, char *pOutBuffer, const uint32_t nOutBufferSize) {
	const auto direction = Dmx::Get()->GetPortDirection(nPortIndex) == ::dmx::PortDirection::INP ? ::lightset::PortDir::INPUT : ::lightset::PortDir::OUTPUT;
	auto nLength = static_cast<uint32_t>(snprintf(pOutBuffer, nOutBufferSize,
			"{\"port\":\"%c\",\"direction\":\"%s\"},",
			static_cast<char>('A' + nPortIndex),
			lightset::get_direction(direction)));

	return nLength;
}

uint32_t json_get_ports(char *pOutBuffer, const uint32_t nOutBufferSize) {
	pOutBuffer[0] = '[';
	uint32_t nLength = 1;

	for (uint32_t nPortIndex = 0; nPortIndex < ::dmx::config::max::PORTS; nPortIndex++) {
		nLength += get_portstatus(nPortIndex, &pOutBuffer[nLength], nOutBufferSize - nLength);
	}

	pOutBuffer[nLength - 1] = ']';

	return nLength;
}
}  // namespace dmx
}  // namespace remoteconfig
