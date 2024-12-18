/**
 * @file storedevice.cpp
 *
 */
/* Copyright (C) 2022-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include "configstoredevice.h"
#include "spi/spi_flash.h"

#include "debug.h"

StoreDevice::StoreDevice() {
	DEBUG_ENTRY

	if (!spi_flash_probe()) {
		DEBUG_PUTS("No SPI flash chip");
	} else {
		printf("StoreDevice: %s sector size %u total %u bytes [%u kB]\n",
				spi_flash_get_name(),
				static_cast<unsigned int>(spi_flash_get_sector_size()),
				static_cast<unsigned int>(spi_flash_get_size()),
				static_cast<unsigned int>(spi_flash_get_size() / 1024U));
		m_IsDetected = true;
	}

	DEBUG_EXIT
}

StoreDevice::~StoreDevice() {
	DEBUG_ENTRY

	DEBUG_EXIT
}

uint32_t StoreDevice::GetSize() const {
	return spi_flash_get_size();
}

uint32_t StoreDevice::GetSectorSize() const {
	return spi_flash_get_sector_size();
}

bool StoreDevice::Read(uint32_t nOffset, uint32_t nLength, uint8_t *pBuffer, storedevice::result& nResult) {
	DEBUG_ENTRY

	nResult = spi_flash_cmd_read_fast(nOffset, nLength, pBuffer) ? storedevice::result::OK : storedevice::result::ERROR;

	DEBUG_PRINTF("nResult=%d", static_cast<int>(nResult));
	DEBUG_EXIT
	return true;
}

bool StoreDevice::Erase(uint32_t nOffset, uint32_t nLength, storedevice::result& nResult) {
	DEBUG_ENTRY

	nResult = spi_flash_cmd_erase(nOffset, nLength) ? storedevice::result::OK : storedevice::result::ERROR;

	DEBUG_PRINTF("nResult=%d", static_cast<int>(nResult));
	DEBUG_EXIT
	return true;
}

bool StoreDevice::Write(uint32_t nOffset, uint32_t nLength, const uint8_t *pBuffer, storedevice::result& nResult) {
	DEBUG_ENTRY

	nResult = spi_flash_cmd_write_multi(nOffset, nLength, pBuffer) ? storedevice::result::OK : storedevice::result::ERROR;

	DEBUG_PRINTF("nResult=%d", static_cast<int>(nResult));
	DEBUG_EXIT
	return true;
}
