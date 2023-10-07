/**
 * @file ltcoscserver.h
 *
 */
/* Copyright (C) 2019-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef OSCSERVER_H_
#define OSCSERVER_H_

#include <stdint.h>

#include "ltcdisplayrgb.h"
#include "network.h"

namespace ltcoscserver {
static constexpr auto PATH_LENGTH_MAX =128;
}  // namespace ltcoscserver

class LtcOscServer {
public:
	LtcOscServer();

	void Start();
	void Stop();
	void Print();

	void SetPortIncoming(uint16_t nPortIncoming) {
		m_nPortIncoming = nPortIncoming;
	}

	uint16_t GetPortIncoming() const {
		return m_nPortIncoming;
	}

	void Run() {
		const auto nBytesReceived = Network::Get()->RecvFrom(m_nHandle, const_cast<const void **>(reinterpret_cast<void **>(&m_pBuffer)), &m_nRemoteIp, &m_nRemotePort);

		if (__builtin_expect((nBytesReceived <= 4), 1)) {
			return;
		}

		HandleOscRequest(nBytesReceived);
	}

private:
	void HandleOscRequest(const uint16_t nBytesReceived);
	void SetWS28xxRGB(uint32_t nSize, ltcdisplayrgb::ColourIndex tIndex);

private:
	uint16_t m_nPortIncoming;
	int32_t m_nHandle { -1 };
	uint32_t m_nRemoteIp { 0 };
	uint16_t m_nRemotePort { 0 };
	char m_aPath[ltcoscserver::PATH_LENGTH_MAX];
	uint32_t m_nPathLength { 0 };
	char *m_pBuffer;
};

#endif /* OSCSERVER_H_ */
