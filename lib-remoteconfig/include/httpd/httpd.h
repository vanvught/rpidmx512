/**
 * @file httpd.h
 *
 */
/* Copyright (C) 2021-2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef HTTPD_HTTPD_H_
#define HTTPD_HTTPD_H_

#include <cstdint>

#include "http.h"
#include "httpdhandlerequest.h"

#include "network.h"

#include "../../lib-network/config/net_config.h"

class HttpDaemon {
public:
	HttpDaemon();
	~HttpDaemon();

	void Run() {
		uint32_t nConnectionHandle;
		const auto nBytesReceived = Network::Get()->TcpRead(m_nHandle, const_cast<const uint8_t **>(reinterpret_cast<uint8_t **>(&m_RequestHeaderResponse)), nConnectionHandle);

		if (__builtin_expect((nBytesReceived == 0), 1)) {
			return;
		}

		DEBUG_PRINTF("nConnectionHandle=%u", nConnectionHandle);

		pHandleRequest[nConnectionHandle]->HandleRequest(nBytesReceived, m_RequestHeaderResponse);
	}

private:
	HttpDeamonHandleRequest *pHandleRequest[TCP_MAX_TCBS_ALLOWED];
	int32_t m_nHandle { -1 };
	char *m_RequestHeaderResponse { nullptr };
};

#endif /* HTTPD_HTTPD_H_ */
