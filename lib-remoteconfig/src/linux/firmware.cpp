/**
 * @file firmware.cpp
 *
 */
/* Copyright (C) 2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifdef NDEBUG
# undef NDEBUG
#endif

#include <cstdint>
#include <cassert>
#include <zlib.h>

#include "firmware.h"
#include "ubootheader.h"
 #include "firmware/debug/debug_debug.h"

namespace firmware {
enum class State {
	IDLE, START, CONTINUE
};

static auto s_State = State::IDLE;
static uint32_t s_nCRC;

bool firmware_install_start(const uint8_t *pBuffer, const uint32_t nBufferSize) {
	DEBUG_ENTRY();
	DEBUG_PRINTF("Firmware: Buffer = %p, Buffer size = %u", reinterpret_cast<const void *>(pBuffer), nBufferSize);

	assert(s_State == State::IDLE);
	s_State = State::START;

	assert(sizeof(struct TImageHeader) <= nBufferSize);

	UBootHeader uBootHeader(pBuffer);
	uBootHeader.Dump();

	const auto isValid = uBootHeader.IsValid();
	DEBUG_PRINTF("Firmware is valid? %s", isValid ? "Yes" : "No");

	if (!isValid) {
		DEBUG_EXIT();
		return false;
	}

	const uint32_t nFirmwareChunk = nBufferSize - sizeof(struct TImageHeader);

	if (nFirmwareChunk > 0) {
		DEBUG_PRINTF("Firmware: Chunk = %u", nFirmwareChunk);

		const auto *pFirmware = pBuffer + sizeof(struct TImageHeader);

		s_nCRC = crc32(0, pFirmware, nFirmwareChunk);
	}

	DEBUG_EXIT();
	return true;
}

bool firmware_install_continue(const uint8_t *pBuffer, const uint32_t nBufferSize) {
	DEBUG_ENTRY();
	DEBUG_PRINTF("Firmware: Buffer = %p, Buffer size = %u", reinterpret_cast<const void *>(pBuffer), nBufferSize);

	assert((s_State == State::START) || (s_State == State::CONTINUE));
	s_State = State::CONTINUE;

	s_nCRC = crc32(s_nCRC, pBuffer, nBufferSize);

	DEBUG_EXIT();
	return true;
}

bool firmware_install_end(const uint8_t *pBuffer, const uint32_t nBufferSize) {
	DEBUG_ENTRY();
	DEBUG_PRINTF("Firmware: Buffer = %p, Buffer size = %u", reinterpret_cast<const void *>(pBuffer), nBufferSize);

	assert(s_State == State::CONTINUE);
	s_State = State::IDLE;

	s_nCRC = crc32(s_nCRC, pBuffer, nBufferSize);

	DEBUG_PRINTF("CRC: %x", s_nCRC);

	DEBUG_EXIT();
	return true;
}

}  // namespace firmware
