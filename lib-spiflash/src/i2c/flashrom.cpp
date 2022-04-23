/**
 * @file flashrom.cpp
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
#include <cstdio>
#include <cassert>

#include "flashrom.h"
#include "i2c/at24cxx.h"

#include "debug.h"

namespace flashrom {
/* Backwards compatibility with SPI FLASH */
static constexpr auto FLASH_SECTOR_SIZE = 4096U;
static constexpr auto ROM_SIZE = 4096U;
}  // namespace flashrom

using namespace flashrom;

FlashRom *FlashRom::s_pThis;

FlashRom::FlashRom(): AT24C32(7) {
	DEBUG_ENTRY
	assert(s_pThis == nullptr);
	s_pThis = this;

	m_IsDetected = AT24C32::IsConnected();

	if (!m_IsDetected) {
		DEBUG_PUTS("No AT24C32");
	} else {
		printf("AT24Cxx: Detected %s with total %d bytes [%d kB]\n", GetName(), GetSize(), GetSize() / 1024U);
	}

	DEBUG_EXIT
}

const char *FlashRom::GetName() const{
	return "AT24Cxx";
}

uint32_t FlashRom::GetSize() const {
	return ROM_SIZE;
}

uint32_t FlashRom::GetSectorSize() const {
	return FLASH_SECTOR_SIZE;
}

bool FlashRom::Read(uint32_t nOffset, uint32_t nLength, uint8_t *pBuffer, flashrom::result& nResult) {
	DEBUG_ENTRY
	assert((nOffset + nLength) <= ROM_SIZE);

	AT24C32::Read(nOffset, pBuffer, nLength);

	nResult = result::OK;

	DEBUG_EXIT
	return true;
}

bool FlashRom::Erase(__attribute__((unused)) uint32_t nOffset, __attribute__((unused)) uint32_t nLength, flashrom::result& nResult) {
	DEBUG_ENTRY

	nResult = result::OK;

	DEBUG_EXIT
	return true;
}

bool FlashRom::Write(uint32_t nOffset, uint32_t nLength, const uint8_t *pBuffer, flashrom::result& nResult) {
	DEBUG_ENTRY
	assert((nOffset + nLength) <= ROM_SIZE);

	AT24C32::Write(nOffset, pBuffer, nLength);

	nResult = result::OK;

	DEBUG_EXIT
	return true;
}
