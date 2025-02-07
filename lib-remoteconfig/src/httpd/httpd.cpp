/**
 * @file httpd.cpp
 *
 */
/* Copyright (C) 2021-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#if defined (DEBUG_HTTPD)
# undef NDEBUG
#endif

#include <cstring>
#include <cstdio>
#include <ctype.h>
#include <cassert>

#include "httpd/httpd.h"

#include "network.h"
#include "net/tcp.h"
#include "net/apps/mdns.h"

#include "../../lib-network/config/net_config.h"

HttpDaemon::HttpDaemon() {
	DEBUG_ENTRY

	assert(m_nHandle == -1);
	m_nHandle = net::tcp_begin(80, Input);
	assert(m_nHandle != -1);

	for (uint32_t nIndex = 0; nIndex < TCP_MAX_TCBS_ALLOWED; nIndex++) {
		new (&handleRequest[nIndex]) HttpDeamonHandleRequest(nIndex, m_nHandle);
	}

	mdns_service_record_add(nullptr, mdns::Services::HTTP);

	DEBUG_EXIT
}

HttpDaemon::~HttpDaemon() {
	DEBUG_ENTRY

	mdns_service_record_delete(mdns::Services::HTTP);

	for (uint32_t nIndex = 0; nIndex < TCP_MAX_TCBS_ALLOWED; nIndex++) {
		// Explicitly calling the destructor because objects were constructed with placement new.
		handleRequest[nIndex].~HttpDeamonHandleRequest();
	}

	net::tcp_end(m_nHandle);

	DEBUG_EXIT
}
