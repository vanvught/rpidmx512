/**
 * @file json_status.cpp
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
#include <cassert>

#include "artnetcontroller.h"
#include "artnet.h"
#include "network.h"

namespace remoteconfig {
namespace artnet {
namespace controller {
static uint32_t get_port(const struct ::artnet::NodeEntryUniverse *pArtNetNodeEntryUniverse, char *pOutBuffer, const uint32_t nOutBufferSize) {
	const auto nLength = static_cast<uint32_t>(snprintf(pOutBuffer, nOutBufferSize,
			"{\"name\":\"%s\",\"universe\":%u},",
			pArtNetNodeEntryUniverse->ShortName, pArtNetNodeEntryUniverse->nUniverse));

	if (nLength <= nOutBufferSize) {
		return nLength;
	}

	return 0;
}

static uint32_t get_entry(const uint32_t nIndex, char *pOutBuffer, const uint32_t nOutBufferSize) {
	const auto *pPollTable = ArtNetController::Get()->GetPollTable();
	auto nLength = static_cast<uint32_t>(snprintf(pOutBuffer, nOutBufferSize,
			"{\"name\":\"%s\",\"ip\":\"" IPSTR "\",\"mac\":\"" MACSTR "\",\"ports\":[",
			pPollTable[nIndex].LongName, IP2STR(pPollTable[nIndex].IPAddress), MAC2STR(pPollTable[nIndex].Mac)));

	for (uint32_t nUniverse = 0; nUniverse < pPollTable[nIndex].nUniversesCount; nUniverse++) {
		const auto *pArtNetNodeEntryUniverse = &pPollTable[nIndex].Universe[nUniverse];
		nLength += get_port(pArtNetNodeEntryUniverse, &pOutBuffer[nLength], nLength);
	}

	nLength--;
	nLength += static_cast<uint32_t>(snprintf(&pOutBuffer[nLength], nOutBufferSize - nLength, "]},"));

	if (nLength <= nOutBufferSize) {
		return nLength;
	}

	return 0;
}

uint32_t json_get_polltable(char *pOutBuffer, const uint32_t nOutBufferSize) {
	const auto nBufferSize = nOutBufferSize - 2U;
	pOutBuffer[0] = '[';

	auto nLength = 1U;

	for (uint32_t nIndex = 0; (nIndex < ArtNetController::Get()->GetPollTableEntries()) && (nLength < nOutBufferSize); nIndex++) {
		const auto nSize = nBufferSize - nLength;
		nLength += get_entry(nIndex, &pOutBuffer[nLength], nSize);
	}

	if (nLength != 1) {
		pOutBuffer[nLength - 1] = ']';
	} else {
		pOutBuffer[1] = ']';
		nLength = 2;
	}

	assert(nLength <= nOutBufferSize);
	return nLength;
}
}  // namespace controller
}  // namespace artnet
}  // namespace remoteconfig
