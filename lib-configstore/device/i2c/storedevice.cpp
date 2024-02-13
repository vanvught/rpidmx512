/**
 * @file storedevice.cpp
 *
 */
/* Copyright (C) 2022-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include "i2c/at24cxx.h"

#include "debug.h"

namespace storedevice {
#if !defined (CONFIG_FLASHROM_I2C_INDEX)
# define CONFIG_FLASHROM_I2C_INDEX	0
#endif
static constexpr uint8_t I2C_INDEX = CONFIG_FLASHROM_I2C_INDEX;
/* Backwards compatibility with SPI FLASH */
static constexpr auto FLASH_SECTOR_SIZE = 4096U;
static constexpr auto ROM_SIZE = 4096U;
}  // namespace storedevice

StoreDevice::StoreDevice() : AT24C32(storedevice::I2C_INDEX) {
	DEBUG_ENTRY

	m_IsDetected = AT24C32::IsConnected();

	if (!m_IsDetected) {
		printf("StoreDevice: No AT24C32 at %2x", AT24C32::GetAddress());
	} else {
		printf("StoreDevice: Detected AT24C32 with total %u bytes [%u kB]\n", GetSize(), GetSize() / 1024U);
	}

	DEBUG_EXIT
}

StoreDevice::~StoreDevice() {
	DEBUG_ENTRY

	DEBUG_EXIT
}

uint32_t StoreDevice::GetSize() const {
	return storedevice::ROM_SIZE;
}

uint32_t StoreDevice::GetSectorSize() const {
	return storedevice::FLASH_SECTOR_SIZE;
}

bool StoreDevice::Read(uint32_t nOffset, uint32_t nLength, uint8_t *pBuffer, storedevice::result& nResult) {
	DEBUG_ENTRY
	assert((nOffset + nLength) <= storedevice::ROM_SIZE);

	AT24C32::Read(nOffset, pBuffer, nLength);

	nResult = storedevice::result::OK;

	DEBUG_EXIT
	return true;
}

bool StoreDevice::Erase([[maybe_unused]] uint32_t nOffset, [[maybe_unused]] uint32_t nLength, storedevice::result& nResult) {
	DEBUG_ENTRY

	nResult = storedevice::result::OK;

	DEBUG_EXIT
	return true;
}

bool StoreDevice::Write(uint32_t nOffset, uint32_t nLength, const uint8_t *pBuffer, storedevice::result& nResult) {
	DEBUG_ENTRY
	assert((nOffset + nLength) <= ROM_SIZE);

	AT24C32::Write(nOffset, pBuffer, nLength);

	nResult = storedevice::result::OK;

	DEBUG_EXIT
	return true;
}
