/**
 * networktcp.cpp
 *
 */
/* Copyright (C) 2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include "network.h"

#include "./../net/net.h"

#include "debug.h"

int32_t Network::TcpBegin(uint16_t nLocalPort) {
	DEBUG_ENTRY
	DEBUG_PRINTF("nLocalPort=%u", nLocalPort);

	const auto nHandle = tcp_begin(nLocalPort);

	DEBUG_PRINTF("nHandle=%d", nHandle);
	DEBUG_EXIT
	return nHandle;
}

uint16_t Network::TcpRead(const int32_t nHandle, const uint8_t **ppBuffer) {
	return tcp_read(nHandle, ppBuffer);
}

void Network::TcpWrite(const int32_t nHandle, const uint8_t *pBuffer, uint16_t nLength) {
	DEBUG_ENTRY
	DEBUG_PRINTF("nHandle=%d, pBuffer=%p, nLength=%u, doClose=%d", nHandle, pBuffer, nLength);

	tcp_write(nHandle, pBuffer, nLength);

	DEBUG_EXIT
}
