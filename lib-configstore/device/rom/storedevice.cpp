/**
 * @file storedevice.cpp
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

#include "configstoredevice.h"
#include "flashcode.h"

#include "debug.h"

StoreDevice::StoreDevice() {
	DEBUG_ENTRY

	m_IsDetected = FlashCode::IsDetected();

	DEBUG_EXIT
}

StoreDevice::~StoreDevice() {
	DEBUG_ENTRY

	DEBUG_EXIT
}

uint32_t StoreDevice::GetSize() const {
	return FlashCode::GetSize();
}

uint32_t StoreDevice::GetSectorSize() const {
	return FlashCode::GetSectorSize();
}

bool StoreDevice::Read(uint32_t nOffset, uint32_t nLength, uint8_t *pBuffer, storedevice::result& nResult) {
	DEBUG_ENTRY

	flashcode::result flashromResult;
	const auto state = FlashCode::Read(nOffset, nLength,  pBuffer, flashromResult);

	nResult = static_cast<storedevice::result>(flashromResult);

	DEBUG_EXIT
	return state;
}

bool StoreDevice::Erase(uint32_t nOffset, uint32_t nLength, storedevice::result& nResult) {
	DEBUG_ENTRY

	flashcode::result flashromResult;
	const auto state = FlashCode::Erase(nOffset, nLength, flashromResult);

	nResult = static_cast<storedevice::result>(flashromResult);

	DEBUG_EXIT
	return state;
}

bool StoreDevice::Write(uint32_t nOffset, uint32_t nLength, const uint8_t *pBuffer, storedevice::result& nResult) {
	DEBUG_ENTRY

	flashcode::result flashromResult;
	const auto state = FlashCode::Write(nOffset, nLength, pBuffer, flashromResult);

	nResult = static_cast<storedevice::result>(flashromResult);

	DEBUG_EXIT
	return state;
}
