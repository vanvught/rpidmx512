/**
 * @file storedevice.cpp
 *
 */
/* Copyright (C) 2022-2023 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include "configstoredevice.h"
#include "gd32.h"

#include "debug.h"

namespace storedevice {
/* Backwards compatibility with SPI FLASH */
static constexpr auto FLASH_SECTOR_SIZE = 4096U;
static constexpr auto BSRAM_SIZE = 4096U;
}  // namespace storedevice


using namespace storedevice;

StoreDevice::StoreDevice() {
	DEBUG_ENTRY

	m_IsDetected = true;

	printf("StoreDevice: BSRAM with total %d bytes [%d kB]\n", GetSize(), GetSize() / 1024U);
	DEBUG_EXIT
}

StoreDevice::~StoreDevice() {
	DEBUG_ENTRY

	DEBUG_EXIT
}

uint32_t StoreDevice::GetSize() const {
	return BSRAM_SIZE;
}

uint32_t StoreDevice::GetSectorSize() const {
	return FLASH_SECTOR_SIZE;
}

bool StoreDevice::Read(__attribute__((unused)) uint32_t nOffset, __attribute__((unused)) uint32_t nLength, __attribute__((unused)) uint8_t *pBuffer, storedevice::result& nResult) {
	DEBUG_ENTRY
	DEBUG_PRINTF("offset=%p[%d], len=%u[%d], data=%p[%d]", nOffset, (((uint32_t)(nOffset) & 0x3) == 0), nLength, (((uint32_t)(nLength) & 0x3) == 0), pBuffer, (((uint32_t)(pBuffer) & 0x3) == 0));
	assert((nOffset + nLength) <= BSRAM_SIZE);

	nResult = result::OK;

	DEBUG_EXIT
	return true;
}

bool StoreDevice::Erase(__attribute__((unused)) uint32_t nOffset, __attribute__((unused)) uint32_t nLength, storedevice::result& nResult) {
	DEBUG_ENTRY

	nResult = result::OK;

	DEBUG_EXIT
	return true;
}

bool StoreDevice::Write(__attribute__((unused)) uint32_t nOffset, __attribute__((unused)) uint32_t nLength, __attribute__((unused)) const uint8_t *pBuffer, storedevice::result& nResult) {
	DEBUG_ENTRY
	DEBUG_PRINTF("offset=%p[%d], len=%u[%d], data=%p[%d]", nOffset, (((uint32_t)(nOffset) & 0x3) == 0), nLength, (((uint32_t)(nLength) & 0x3) == 0), pBuffer, (((uint32_t)(pBuffer) & 0x3) == 0));
	assert((nOffset + nLength) <= BSRAM_SIZE);

	nResult = result::OK;

	DEBUG_EXIT
	return true;
}
