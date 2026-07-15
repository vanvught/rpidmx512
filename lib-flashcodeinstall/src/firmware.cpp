/**
 * @file firmware.cpp
 *
 */
/* Copyright (C) 2025-2026 by Arjan van Vught mailto:info@gd32-dmx.org
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
	kIdle, kStart, kContinue
};

static auto s_State = State::kIdle;
static uint32_t s_nCRC;

bool firmware_install_start(const uint8_t *buffer, uint32_t buffer_size) {
	DEBUG_ENTRY();
	DEBUG_PRINTF("Firmware: Buffer = %p, Buffer size = %u", reinterpret_cast<const void *>(buffer), static_cast<unsigned>(buffer_size));

	assert(s_State == State::kIdle);
	s_State = State::kStart;

	assert(sizeof(struct TImageHeader) <= buffer_size);

	UBootHeader uboot_header(buffer);
	uboot_header.Dump();

	const auto kIsValid = uboot_header.IsValid();
	DEBUG_PRINTF("Firmware is valid? %s", kIsValid ? "Yes" : "No");

	if (!kIsValid) {
		DEBUG_EXIT();
		return false;
	}

	const uint32_t kFirmwareChunk = buffer_size - sizeof(struct TImageHeader);

	if (kFirmwareChunk > 0) {
		DEBUG_PRINTF("Firmware: Chunk = %u", static_cast<unsigned>(kFirmwareChunk));

		const auto *firmware = buffer + sizeof(struct TImageHeader);

		s_nCRC = crc32(0, firmware, kFirmwareChunk);
	}

	DEBUG_EXIT();
	return true;
}

bool firmware_install_continue(const uint8_t *buffer, uint32_t buffer_size) {
	DEBUG_ENTRY();
	DEBUG_PRINTF("Firmware: Buffer = %p, Buffer size = %u", reinterpret_cast<const void *>(buffer), static_cast<unsigned>(buffer_size));

	assert((s_State == State::kStart) || (s_State == State::kContinue));
	s_State = State::kContinue;

	s_nCRC = crc32(s_nCRC, buffer, buffer_size);

	DEBUG_EXIT();
	return true;
}

bool firmware_install_end(const uint8_t *buffer, uint32_t buffer_size) {
	DEBUG_ENTRY();
	DEBUG_PRINTF("Firmware: Buffer = %p, Buffer size = %u", reinterpret_cast<const void *>(buffer), static_cast<unsigned>(buffer_size));

	assert(s_State == State::kContinue);
	s_State = State::kIdle;

	s_nCRC = crc32(s_nCRC, buffer, buffer_size);

	DEBUG_PRINTF("CRC: %x", static_cast<unsigned>(s_nCRC));

	DEBUG_EXIT();
	return true;
}
}  // namespace firmware
