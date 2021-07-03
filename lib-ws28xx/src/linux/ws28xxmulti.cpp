/**
 * @file ws28xxmulti8x.cpp
 *
 */
/* Copyright (C) 2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <cstring>
#include <cassert>

#include "ws28xxmulti.h"

#include "debug.h"

void WS28xxMulti::SetupBuffers() {
	DEBUG_ENTRY

	constexpr uint32_t nSize = 32 * 1024;

	m_pBuffer = new uint8_t[nSize];
	assert(m_pBuffer != 0);

	const uint32_t nSizeHalf = nSize / 2;
	assert(m_nBufSize <= nSizeHalf);

	if (m_nBufSize > nSizeHalf) {
		// FIXME Handle internal error
		return;
	}

	m_pBlackoutBuffer = m_pBuffer + (nSizeHalf & static_cast<uint32_t>(~3));

	memset(m_pBuffer, 0, m_nBufSize);
	memcpy(m_pBlackoutBuffer, m_pBuffer, m_nBufSize);

	DEBUG_PRINTF("nSize=%x, m_pBuffer=%p, m_pBlackoutBuffer=%p", nSize, reinterpret_cast<void *>(m_pBuffer), reinterpret_cast<void *>(m_pBlackoutBuffer));
	DEBUG_EXIT
}
