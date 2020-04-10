/**
 * @file spiflashstoreuuid.cpp
 *
 */
/* Copyright (C) 2019-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <uuid/uuid.h>

#include "spiflashstore.h"

//void SpiFlashStore::UuidUpdate(const uuid_t uuid) {
//	bool bIsChanged = false;
//
//	const uint8_t *src = (uint8_t *) uuid;
//	uint8_t *dst = (uint8_t *) &m_aSpiFlashData[16];
//
//	for (uint32_t i = 0; i < sizeof(uuid_t); i++) {
//		if (*src != *dst) {
//			bIsChanged = true;
//			*dst = *src;
//		}
//		dst++;
//		src++;
//	}
//
//	if (bIsChanged && (m_tState != STORE_STATE_ERASED)) {
//		m_tState = STORE_STATE_CHANGED;
//	}
//}
//
//void SpiFlashStore::UuidCopyTo(uuid_t uuid) {
//	const uint8_t *src = (uint8_t *) &m_aSpiFlashData[16];
//	uint8_t *dst = (uint8_t *) uuid;
//
//	for (uint32_t i = 0; i < sizeof(uuid_t); i++) {
//		*dst++ = *src++;
//	}
//}
