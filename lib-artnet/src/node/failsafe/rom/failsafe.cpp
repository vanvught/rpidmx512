/**
 * @file failsafe.cpp
 *
 */
/* Copyright (C) 2022 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <cassert>

#include "artnetnode.h"
#include "artnetnodefailsafe.h"

#include "../lib-flashcode/include/flashcode.h"

#include "debug.h"

namespace artnetnode {

static bool isDetected;
static uint32_t nOffsetBase;

static bool is_detected() {
	DEBUG_ENTRY
	DEBUG_PRINTF("isDetected=%d", isDetected);

	if (!isDetected) {
		if (!FlashCode::Get()->IsDetected()) {
			DEBUG_EXIT
			return false;
		}

		const auto nEraseSize = FlashCode::Get()->GetSectorSize();
		assert(nEraseSize <= failsafe::BYTES_NEEDED);
		const auto nPages = 1 + failsafe::BYTES_NEEDED / nEraseSize;

		DEBUG_PRINTF("KB_NEEDED=%u, nEraseSize=%u, nPages=%u", failsafe::BYTES_NEEDED, nEraseSize, nPages);

		assert(((nPages + 1) * nEraseSize) <= FlashCode::Get()->GetSize());

		nOffsetBase = FlashCode::Get()->GetSize() - ((nPages + 1) * nEraseSize);

		DEBUG_PRINTF("nOffsetBase=%p", nOffsetBase);
	}

	DEBUG_EXIT
	return true;
}

void failsafe_write_start() {
	DEBUG_ENTRY
	DEBUG_PRINTF("isDetected=%d", isDetected);

	if (!is_detected()) {
		DEBUG_EXIT
		return;
	}

	flashcode::result nResult;
	uint32_t nTimeout = 0;

	while (!FlashCode::Get()->Erase(nOffsetBase, FlashCode::Get()->GetSectorSize(), nResult)) {
		nTimeout++;
	}

	isDetected = (nResult == flashcode::result::OK);

	DEBUG_PRINTF("nResult=%d, isDetected=%d, nTimeout=%u", nResult, isDetected, nTimeout);
	DEBUG_EXIT
}

void failsafe_write(uint32_t nPortIndex, const uint8_t *pData) {
	DEBUG_ENTRY
	assert(nPortIndex < artnetnode::MAX_PORTS);
	assert(pData != nullptr);

	if (!isDetected) {
		DEBUG_EXIT
		return;
	}

	const auto nOffset = nOffsetBase + (nPortIndex * lightset::dmx::UNIVERSE_SIZE);

	DEBUG_PRINTF("nOffsetBase=%p, nOffset=%p", nOffsetBase, nOffset);

	flashcode::result nResult;
	uint32_t nTimeout = 0;

	while (!FlashCode::Get()->Write(nOffset, lightset::dmx::UNIVERSE_SIZE, pData, nResult)) {
		nTimeout++;
	}

	DEBUG_PRINTF("nResult=%d, nTimeout=%u", nResult, nTimeout);

	assert(nResult == flashcode::result::OK);

	DEBUG_EXIT
}

void failsafe_write_end() {
	DEBUG_ENTRY

	// No code needed here

	DEBUG_EXIT
}


void failsafe_read_start() {
	DEBUG_ENTRY
	DEBUG_PRINTF("isDetected=%d", isDetected);

	if (!is_detected()) {
		DEBUG_EXIT
		return;
	}

	isDetected = true;

	DEBUG_EXIT
}

void failsafe_read(uint32_t nPortIndex, uint8_t *pData) {
	DEBUG_ENTRY
	assert(nPortIndex < artnetnode::MAX_PORTS);
	assert(pData != nullptr);

	if (!isDetected) {
		DEBUG_EXIT
		return;
	}

	const auto nOffset = nOffsetBase + (nPortIndex * lightset::dmx::UNIVERSE_SIZE);

	DEBUG_PRINTF("nOffsetBase=%p, nOffset=%p", nOffsetBase, nOffset);

	flashcode::result nResult;
	uint32_t nTimeout = 0;

	while (!FlashCode::Get()->Read(nOffset, lightset::dmx::UNIVERSE_SIZE, pData, nResult)) {
		nTimeout++;
	}

	DEBUG_PRINTF("nResult=%d, nTimeout=%u", nResult, nTimeout);

	assert(nResult == flashcode::result::OK);

	DEBUG_EXIT
}

void failsafe_read_end() {
	DEBUG_ENTRY

	// No code needed here

	DEBUG_EXIT
}
}  // namespace artnetnode
